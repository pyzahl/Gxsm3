/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Copyright (C) 1999,2000,2001,2002,2003 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * additional features: Andreas Klust <klust@users.sf.net>
 * WWW Home: http://gxsm.sf.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

/* irnore this module for docuscan
% PlugInModuleIgnore
 */


#include <locale.h>
#include <libintl.h>
#include <time.h>


#include "glbvars.h"
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sranger_hwi.h"

typedef struct{
	DSP_UINT    r_position;	  /* read pos (Gxsm) (always in word size) (WO) by host =WO */
	DSP_UINT    w_position;   /* write pos (DSP) (always in word size) (RO) by host =RO */
	DSP_UINT    current_section_head_position; /* resync/verify and status info =RO */
	DSP_UINT    current_section_index; /* index of current section =RO */
	DSP_UINT    fill;	      /* filled indicator = max num to read =RO */
	DSP_UINT    stall;	      /* number of fifo stalls =RO */
	DSP_UINT    length;	      /* constant, buffer size =RO */
	DSP_UINT    p_buffer_base; /* pointer to external memory, buffer start =RO */
	DSP_UINT    p_buffer_w;    /* pointer to external memory =RO */
	DSP_UINT    p_buffer_l;
} DATA_FIFO_EXTERN_PCOPY;

typedef struct{
	DSP_LONG srcs;
	DSP_LONG n;
	DSP_LONG time;
	DSP_INT  x_offset;
	DSP_INT  y_offset;
	DSP_INT  z_offset;
	DSP_INT  x_scan;
	DSP_INT  y_scan;
	DSP_INT  z_scan;
	DSP_INT  u_scan;
	DSP_INT  section;
} PROBE_SECTION_HEADER;


// you may want to handle/emulate some DSP commands later...
#include "dsp-pci32/xsm/dpramdef.h"
#include "dsp-pci32/xsm/xsmcmd.h"


// need some SRanger io-controls 
// HAS TO BE IDENTICAL TO THE DRIVER's FILE!
#include "../plug-ins/hard/modules/sranger_ioctl.h"

// SRanger data exchange structs and consts
#include "SR-STD_spmcontrol/FB_spm_dataexchange.h" 

// enable debug:
#define	SRANGER_DEBUG(S) XSM_DEBUG (DBG_L4, S)
#define	SRANGER_ERROR(S) XSM_DEBUG_ERROR (DBG_L4, S)


#define SR_READFIFO_RESET -1
#define SR_EMPTY_PROBE_FIFO -2


extern DSPControl *DSPControlClass;
extern GxsmPlugin sranger_hwi_pi;

// some thread states

#define RET_FR_OK      0
#define RET_FR_ERROR   -1
#define RET_FR_WAIT    1
#define RET_FR_NOWAIT  2
#define RET_FR_FCT_END 3

#define FR_NO   0
#define FR_YES  1

#define FR_INIT   1
#define FR_FINISH 2
#define FR_FIFO_FORCE_RESET 3


gpointer FifoReadThread (void *ptr_sr);
gpointer ProbeFifoReadThread (void *ptr_sr);
gpointer ProbeFifoReadFunction (void *ptr_sr, int dspdev);



/* Construktor for Gxsm sranger support base class
 * ==================================================
 * - open device
 * - init things
 * - ...
 */
sranger_hwi_dev::sranger_hwi_dev(){
	SRANGER_DEBUG("open driver");
	AIC_max_points = 1<<15; // SR-AIC resolution is limiting...
	fifo_read_thread = NULL;
	probe_fifo_read_thread = NULL;
	probe_fifo_thread_active = FALSE;
	thread_dsp = 0;
	productid = g_strdup ("not yet identified");
	swap_flg = 0;
	magic_data.magic = 0; // set to zero, this means all data is invalid!
	dsp_alternative = 0;

	if((dsp = open (xsmres.DSPDev, O_RDWR)) <= 0){
		SRANGER_ERROR(
			"Can´t open <" << xsmres.DSPDev << ">, reason: " << strerror(errno) << std::endl
			<< "please make sure:" << std::endl
			<< "-> that the device exists and has proper access rights" << std::endl
			<< "-> that the kernel module loaded" << std::endl
			<< "-> the USB connection to SignalRanger" << std::endl
			<< "-> that the SR-SP2 is powered on");
		productid=g_strdup_printf ("Device used: %s\n\n"
					   "Make sure:\n"
					   "-> that the device exists and has proper access rights\n"
					   "-> that the kernel module loaded (try 'dmesg')\n"
					   "-> the USB connection to SignalRanger is OK\n"
					   "-> that the SR-SP2 is powered on\n"
					   "Start 'gxsm3 -h no' to change the device path/name.",
					   xsmres.DSPDev);
		gapp->alert (N_("No Hardware"), N_("Open Device failed."), productid, 1);
		dsp = 0;
		exit (-1);
		return;
	}
	else{
		int ret;
		unsigned int vendor, product;
		if ((ret=ioctl(dsp, SRANGER_IOCTL_VENDOR, (unsigned long)&vendor)) < 0){
			SRANGER_ERROR(strerror(ret) << " cannot query VENDOR" << std::endl
				      << "Device: " << xsmres.DSPDev);
			g_free (productid);
			productid=g_strdup_printf ("Device used: %s\n Start 'gxsm3 -h no' to correct the problem.", xsmres.DSPDev);
			gapp->alert (N_("Unkonwn Hardware"), N_("Query Vendor ID failed."), productid, 1);
			close (dsp);
			dsp = 0;
			exit (-1);
			return;
		}
		if ((ret=ioctl(dsp, SRANGER_IOCTL_PRODUCT, (unsigned long)&product)) < 0){
			SRANGER_ERROR(strerror(ret) << " cannot query PRODUCT" << std::endl
				      << "Device: " << xsmres.DSPDev);
			g_free (productid);
			productid=g_strdup_printf ("Device used: %s\n Start 'gxsm3 -h no' to correct the problem.", xsmres.DSPDev);
			gapp->alert (N_("Unkonwn Hardware"), N_("Query Product ID failed."), productid, 1);
			close (dsp);
			dsp = 0;
			exit (-1);
			return;
		}
		if (vendor == 0x0a59){
			g_free (productid);
			if (product == 0x0101)
				productid=g_strdup ("Vendor/Product: B.Paillard, Signal Ranger STD");
			else if (product == 0x0103)
				productid=g_strdup ("Vendor/Product: B.Paillard, Signal Ranger SP2");
			else{
				productid=g_strdup ("Vendor/Product: B.Paillard, unkown version!");
				gapp->alert (N_("Unkonwn Hardware detected"), N_("No Signal Ranger found."), productid, 1);
				close (dsp);
				dsp=0;
				exit (-1);
				return;
			}
				
			// now read magic struct data
			lseek (dsp, FB_SPM_MAGIC_ADR, SRANGER_SEEK_DATA_SPACE);
			sr_read (dsp, &magic_data, sizeof (magic_data)); 
				
			swap_flg = 0;
			if (magic_data.magic != FB_SPM_MAGIC){
				swap (&magic_data.magic);
				if (magic_data.magic != FB_SPM_MAGIC){
					SRANGER_ERROR ("DSP SPM soft cannot be identified: Magic unkown");
					productid=g_strdup_printf ("Bad Magic: %04x\n"
								   "Please launch the correct DSP software before restarting GXSM.",
								   magic_data.magic);
					gapp->alert (N_("Wrong DSP magic"), N_("DSP software was not identified."), productid, 1);
					close (dsp);
					dsp=0;
					magic_data.magic = 0; // set to zero, this means all data is invalid!
					exit (-1);
					return;
				}
				// no swapp all to fix endianess
				swap_flg = 1;
				swap (&magic_data.version);
				swap (&magic_data.year);
				swap (&magic_data.mmdd);
				swap (&magic_data.dsp_soft_id);
				swap (&magic_data.statemachine);
				swap (&magic_data.AIC_in);
				swap (&magic_data.AIC_out);
				swap (&magic_data.AIC_reg);
				swap (&magic_data.feedback);
				swap (&magic_data.analog);
				swap (&magic_data.scan);
				swap (&magic_data.move);
				swap (&magic_data.probe);
				swap (&magic_data.autoapproach);
				swap (&magic_data.datafifo);
				swap (&magic_data.probedatafifo);
				swap (&magic_data.scan_event_trigger);
				swap (&magic_data.dfm_fuzzymix);
				swap (&magic_data.CR_out_puls);
				swap (&magic_data.external);
				swap (&magic_data.CR_generic_io);
			}
			SRANGER_DEBUG ("SRanger, FB_SPM soft: Magic data OK");
				
			if (FB_SPM_SOFT_ID != magic_data.dsp_soft_id){
				gchar *details = g_strdup_printf (
					"Bad SR DSP Software ID %4X found.\n"
					"SR DSP Software ID %4X expected.\n"
					"This non SPM DSP code will not work for GSXM, cannot proceed."
					"Please launch the correct DSP software before restarting GXSM.",
					(unsigned int)magic_data.dsp_soft_id,
					(unsigned int)FB_SPM_SOFT_ID);
				SRANGER_DEBUG ("Signal Ranger FB_SPM soft Version mismatch\n" << details);
				gapp->alert (N_("Warning"), N_("Signal Ranger FB_SPM software version mismatch detected!"), details, 1);
				g_free (details);
				close (dsp);
				dsp=0;
				magic_data.magic = 0; // set to zero, this means all data is invalid!
				exit (-1);
				return;
			}
				
			if (FB_SPM_VERSION != magic_data.version || 
			    FB_SPM_SOFT_ID != magic_data.dsp_soft_id){
				gchar *details = g_strdup_printf(
					"Detected SRanger DSP Software Version: %x.%02x\n"
					"GXSM was build for DSP Software Version: %x.%02x\n\n"
					"Note: This may cause incompatility problems and unpredictable toubles,\n"
					"however, trying to proceed in case you know what you are doing.",
					magic_data.version >> 8, 
					magic_data.version & 0xff,
					FB_SPM_VERSION >> 8, 
					FB_SPM_VERSION & 0xff)
				SRANGER_DEBUG ("Signal Ranger FB_SPM soft Version mismatch\n" << details);
				gapp->alert (N_("Warning"), N_("Signal Ranger FB_SPM software version mismatch detected!"), details, 1);
				g_free (details);
			}
				
			SRANGER_DEBUG ("ProductId:" << productid);

			// open some more DSP connections, used by threads
			if((thread_dsp = open (xsmres.DSPDev, O_RDWR)) <= 0){
				SRANGER_ERROR ("cannot open thread SR connection, trying to continue...");
				thread_dsp = 0;
			}
			// testing...
			if((probe_thread_dsp = open (xsmres.DSPDev, O_RDWR)) <= 0){
				SRANGER_ERROR ("cannot open probe thread SR connection, trying to continue...");
				probe_thread_dsp = 0;
			}
			if((dsp_alternative = open (xsmres.DSPDev, O_RDWR)) <= 0){
				SRANGER_ERROR ("cannot open alternative SR connection, trying to continue...");
				dsp_alternative = 0;
			}
		}else{
			SRANGER_ERROR ("unkown vendor, exiting");
			close (dsp);
			dsp = 0;
			return;
		}
	}
}

/* Destructor
 * close device
 */
sranger_hwi_dev::~sranger_hwi_dev(){
	SRANGER_DEBUG ("closing connection to SRanger driver");
	close (dsp);
	close (thread_dsp);
	close (probe_thread_dsp);
	close (dsp_alternative);
}

// data translation helpers

void sranger_hwi_dev::swap (guint16 *addr){
	guint16 temp1, temp2;
	temp1 = temp2 = *addr;
	*addr = ((temp2 & 0xFF) << 8) | ((temp1 >> 8) & 0xFF);
}

void sranger_hwi_dev::swap (gint16 *addr){
	guint16 temp1, temp2;
	temp1 = temp2 = *addr;
	*addr = ((temp2 & 0xFF) << 8) | ((temp1 >> 8) & 0xFF);
}

void sranger_hwi_dev::swap (gint32 *addr){
	gint32 temp1, temp2, temp3, temp4;

	temp1 = (*addr)       & 0xFF;
	temp2 = (*addr >> 8)  & 0xFF;
	temp3 = (*addr >> 16) & 0xFF;
	temp4 = (*addr >> 24) & 0xFF;

	*addr = (temp1 << 24) | (temp2 << 16) | (temp3 << 8) | temp4;
}

gint32 sranger_hwi_dev::float_2_sranger_q15 (double x){
	if (x >= 1.)
		return 32767;
	if (x <= -1.)
		return -32766;
	
	return (gint32)(x*32767.);
}

gint32 sranger_hwi_dev::int_2_sranger_int (gint32 x){
	gint16 tmp = x > 32767 ? 32767 : x < -32766 ? -32766 : x; // saturate
	if (swap_flg)
		swap (&tmp);
	return (gint32)tmp;
}

gint32 sranger_hwi_dev::long_2_sranger_long (gint32 x){
	if (swap_flg)
		swap (&x);
	return x;
}



// Image Data FIFO read thread section
// ==================================================

// start_fifo_read:
// Data Transfer Setup:
// Prepare and Fork Image Data FIFO read thread

int sranger_hwi_dev::start_fifo_read (int y_start, 
				  int num_srcs0, int num_srcs1, int num_srcs2, int num_srcs3, 
				  Mem2d **Mob0, Mem2d **Mob1, Mem2d **Mob2, Mem2d **Mob3){
	if (num_srcs0 || num_srcs1 || num_srcs2 || num_srcs3){
		fifo_data_num_srcs[0] = num_srcs0;
		fifo_data_num_srcs[1] = num_srcs1;
		fifo_data_num_srcs[2] = num_srcs2;
		fifo_data_num_srcs[3] = num_srcs3;
		fifo_data_Mobp[0] = Mob0;
		fifo_data_Mobp[1] = Mob1;
		fifo_data_Mobp[2] = Mob2;
		fifo_data_Mobp[3] = Mob3;
		fifo_data_y_index = y_start; // if > 0, scan dir is "bottom-up"
		fifo_read_thread = g_thread_new ("FifoReadThread", FifoReadThread, this);

		if ((DSPControlClass->Source & 0xffff) && DSPControlClass->probe_trigger_raster_points > 0){
			DSPControlClass->probe_trigger_single_shot = 0;
			ReadProbeFifo (thread_dsp, FR_INIT); // init
		}
	}
	else{
		if (DSPControlClass->Source & 0xffff)
			probe_fifo_read_thread = g_thread_new ("ProbeFifoReadThread", ProbeFifoReadThread, this);
	}

	return 0;
}

// FifoReadThread:
// Image Data FIFO read thread

gpointer FifoReadThread (void *ptr_sr){
	sranger_hwi_dev *sr = (sranger_hwi_dev*)ptr_sr;
	int ny = sr->fifo_data_Mobp[sr->fifo_data_num_srcs[0] ? 0:1][0]->GetNy();

	SRANGER_DEBUG ("Starting Fifo Read, reading " << ny << " lines, " << "y_index: " << sr->fifo_data_y_index);

	// This delay is to avoid some not yet exploited initial fifo-reading delay and unnecessary early fifo overflow - PZ
//	usleep(2000000); // wait for GUI stuff to get ready, empiric -- 4s

	sr->ReadLineFromFifo (SR_READFIFO_RESET); // init read fifo status

	if (sr->fifo_data_y_index == 0){ // top-down
		for (int yi=0; yi < ny; ++yi)
			if (sr->ReadLineFromFifo (yi))
				break;
	}else{ // bottom-up
		for (int yi=ny-1; yi >= 0; --yi)
			if (sr->ReadLineFromFifo (yi)) 
				break;
	}

	sr->ReadLineFromFifo (SR_EMPTY_PROBE_FIFO); // finish reading all FIFO's (probe may have to be emptied)

	SRANGER_DEBUG ("Fifo Read Done.");

	return NULL;
}

// ReadLineFromFifo:
// read scan line from FIFO -- wait for data, and empty FIFO as quick as possible, 
// sort data chunks away int scan-mem2d objects
int sranger_hwi_dev::ReadLineFromFifo (int y_index){
	static int len[4] = { 0,0,0,0 };
	static int total_len = 0;
	static int fifo_reads = 0;
	static DATA_FIFO dsp_fifo;
	static int max_fill = 0;
	int xi;
	static int readfifo_status = 0;

//		SRANGER_DEBUG ("ReadData: yindex=" << y_index << "---------------------------");

//	std::cout << "ReadLineFromFifo:" << y_index << std::endl;
		
	// initiate and unlock scan process now!
	if (y_index == SR_READFIFO_RESET){
		readfifo_status = 0;
		return 0;
	}

	// finish reading all FIFO's (probe may have to be emptied)
	if (y_index == SR_EMPTY_PROBE_FIFO){
		// check probe fifo
		if (DSPControlClass->probe_trigger_raster_points > 0){
			for (int i=0; i<10; ++i){ // about 1/4s finish time
				// free some cpu time now
				usleep(25000);
				ProbeFifoReadFunction (this, thread_dsp);
			}
		}
		return 0;
	}

	if (!readfifo_status){
		fifo_reads = 0; max_fill = 0;
		total_len = 0;
		for (int dir=0; dir < 4; ++dir){ // number of data chunks per scanline and direction
			if (!fifo_data_num_srcs[dir]) 
				len[dir] = 0;
			else
				len[dir] = fifo_data_Mobp[dir][0]->GetNx();
			total_len += len[dir];
//			std::cout << "Dir:" << dir << " L: " << len[dir] << std::endl;
		}

		lseek (thread_dsp, magic_data.datafifo, SRANGER_SEEK_DATA_SPACE);
		sr_read (thread_dsp, &dsp_fifo, (MAX_WRITE_DATA_FIFO+3)<<1);
		dsp_fifo.stall = 0; // unlock scanning
		check_and_swap (dsp_fifo.stall);
		sr_write (thread_dsp, &dsp_fifo, (MAX_WRITE_DATA_FIFO+3)<<1);
		// to PC format
		check_and_swap (dsp_fifo.r_position);
		check_and_swap (dsp_fifo.w_position);
		readfifo_status = 1;
	}

	{
		int maxnum_w = MAX (MAX (MAX (fifo_data_num_srcs[0]&0x0f, fifo_data_num_srcs[1]&0x0f), fifo_data_num_srcs[2]&0x0f), fifo_data_num_srcs[3]&0x0f);
		int maxnum_l = MAX (MAX (MAX ((fifo_data_num_srcs[0]&0xf0)>>4, (fifo_data_num_srcs[1]&0xf0)>>4), (fifo_data_num_srcs[2]&0xf0)>>4), (fifo_data_num_srcs[3]&0xf0)>>4);
		SHT *linebuffer_w = new SHT[len[0]*maxnum_w+1];
		LNG *linebuffer_l = new LNG[len[0]*maxnum_l+1];
		for (int dir = 0; dir < 4 && ScanningFlg; ++dir){
			if (!fifo_data_num_srcs[dir]) continue;
			xi = 0;
			do{
				// check probe fifo
				if (DSPControlClass->probe_trigger_raster_points > 0)
					ProbeFifoReadFunction (this, thread_dsp);

				// transfer data from DSP fifo into local fifo buffer -- empty DSP fifo
				if (abs (dsp_fifo.r_position - dsp_fifo.w_position) < (DATAFIFO_LENGTH/16)){
					// to DSP format
					check_and_swap (dsp_fifo.r_position);
					check_and_swap (dsp_fifo.w_position);

					// set DSP fifo read position
					if (DSPControlClass->probe_trigger_raster_points > 0)
						lseek (thread_dsp, magic_data.datafifo, SRANGER_SEEK_DATA_SPACE); // added for mixed probe/scan data read
					sr_write (thread_dsp, &dsp_fifo, MAX_WRITE_DATA_FIFO<<1);
//								SRANGER_DEBUG ("fifo buffer read #" << fifo_reads << " w-r: " << dsp_fifo.w_position - dsp_fifo.r_position << "  dir: " << dir);
					// Get all and empty fifo
					// lseek (thread_dsp, magic_data.datafifo, SRANGER_SEEK_DATA_SPACE);
					sr_read  (thread_dsp, &dsp_fifo, sizeof (dsp_fifo));
					++fifo_reads;
								
					// to PC format
					check_and_swap (dsp_fifo.r_position);
					check_and_swap (dsp_fifo.w_position);
								
					// calc and update Fifo stats
					{
						check_and_swap (dsp_fifo.length); // to PC
						dsp_fifo.fill = dsp_fifo.w_position - dsp_fifo.r_position;
						if (dsp_fifo.fill < 0) 	dsp_fifo.fill += dsp_fifo.length;
						if (dsp_fifo.fill > max_fill){
							max_fill = 	dsp_fifo.fill;
						}
						// calc fill percentage
						dsp_fifo.fill = (int)(100*(double)dsp_fifo.fill/(double)dsp_fifo.length);
						g_free (AddStatusString);
						AddStatusString = g_strdup_printf ("Fifo: %d%% [%d%%]", 
										   dsp_fifo.fill, 
										   (int)(100*(double)max_fill/(double)dsp_fifo.length));
						check_and_swap (dsp_fifo.length); // to DSP
					}
								

					// free some cpu time now
					usleep(25000);
				}
						
				// transfer and convert data from fifo buffer
				dsp_fifo.r_position += FifoRead ((int)dsp_fifo.r_position, (int)dsp_fifo.w_position,
								 xi, fifo_data_num_srcs[dir], len[dir], 
								 linebuffer_w, linebuffer_l, dsp_fifo.buffer.w, dsp_fifo.buffer.l);
				dsp_fifo.r_position &= DATAFIFO_MASK;
						
			} while (xi < len[dir] && ScanningFlg);
				
			// skip if scan was canceled
			if (xi >= len[dir] && ScanningFlg){
				// read data into linebuffer
				int num_w = fifo_data_num_srcs[dir]&0x0f;
				int num_l = (fifo_data_num_srcs[dir]&0xf0)>>4;
				for (int i=0; i<num_w; ++i){
					if (fifo_data_Mobp[dir][i])
						fifo_data_Mobp[dir][i]->PutDataLine (y_index, linebuffer_w+i*len[dir]);
				}
				for (int i=0; i<num_l; ++i){
					if (fifo_data_Mobp[dir][i+num_w])
						fifo_data_Mobp[dir][i+num_w]->PutDataLine (y_index, linebuffer_l+i*len[dir]);
				}
			}
		}

		delete[] linebuffer_w;
		delete[] linebuffer_l;
	}

	fifo_data_y_index = y_index;

	// check probe fifo again
	if (DSPControlClass->probe_trigger_raster_points > 0)
		ProbeFifoReadFunction (this, thread_dsp);

	return (ScanningFlg) ? 0:1;
}

// FifoRead:
// Read data from FIFO ring buffer (ring) now in host memory and convert and sort into temporary secondary buffer (linear)
// Short-Type Data
int sranger_hwi_dev::FifoRead (int start, int end, int &xi, int num_srcs, int len, SHT *buffer_w, LNG *buffer_l , SHT *fifo_w, LNG *fifo_l){
	int count=0;
	int num_w = num_srcs&0x0f;
	int num_l = (num_srcs&0xf0)>>4;
	int block_len = num_w + 2*num_l;
	while (end < start) end += DATAFIFO_LENGTH;

//	std::cout << "FIFO read: " << start << ":" << end << " -> [" << len << "] " << num_w <<"W " << num_l << "L" << std::endl;

	if (swap_flg){
		for (int fi=start; fi<end; fi+=block_len){
			for (int i=0; i<num_w; ++i){
				int q=(fi+i)&DATAFIFO_MASK;
				swap (&fifo_w[q]);
				buffer_w[xi+i*len] = fifo_w[q]-1;
				++count;
			}
			if (num_l){
				if ((num_w+fi)&1) 
					++count, ++fi; // force even address if long words following
				for (int i=0; i<num_l; ++i){
					int q = ((fi+num_w + 2*i)&DATAFIFO_MASK) >> 1;
					swap (&fifo_l[q]);
					buffer_l[xi+i*len] = fifo_l[q];
					count += 2;
				}
			}
			if( ++xi == len) break;
		}	
	} else {
		for (int fi=start; fi<end; fi+=block_len){
			for (int i=0; i<num_w; ++i){
				buffer_w[xi+i*len] = fifo_w[(fi+i)&DATAFIFO_MASK]-1;
				++count;
			}
			if (num_l){
				if ((fi+num_w)&1) 
					++count, ++fi; // force even address if long words following
				for (int i=0; i<num_l; ++i){
					int q = ((fi+num_w + 2*i)&DATAFIFO_MASK) >> 1;
					buffer_l[xi+i*len] = fifo_l[q];
					count += 2;
				}
			}
			if( ++xi == len) break;
		}	
	}
	return count;
}



// ==================================================
//
// Probe Data FIFO read thread section
//
// ==================================================
 
// FIFO watch verbosity...
# define LOGMSGS0(X) std::cout << X
//# define LOGMSGS0(X)

//# define LOGMSGS(X) std::cout << X
# define LOGMSGS(X)

//# define LOGMSGS2(X) std::cout << X
# define LOGMSGS2(X)

// ProbeFifoReadThread:
// Independent ProbeFifoRead Thread (manual probe)
gpointer ProbeFifoReadThread (void *ptr_sr){
	int finish_flag=FALSE;
	sranger_hwi_dev *sr = (sranger_hwi_dev*)ptr_sr;

	if (sr->probe_fifo_thread_active){
		LOGMSGS ( "ProbeFifoReadThread ERROR: Attempt to start again while in progress! [#" << sr->probe_fifo_thread_active << "]" << std::endl);
		return NULL;
	}
	sr->probe_fifo_thread_active++;
	DSPControlClass->probe_ready = FALSE;

	if (DSPControlClass->probe_trigger_single_shot)
		 finish_flag=TRUE;

	clock_t timeout = clock() + (clock_t)(CLOCKS_PER_SEC*(0.5+sr->probe_time_estimate/22000.));

	LOGMSGS ( "ProbeFifoReadThread START  " << (DSPControlClass->probe_trigger_single_shot ? "Single":"Multiple") << "-VP[#" 
		  << sr->probe_fifo_thread_active << "] Timeout is set to:" 
		  << (timeout-clock()) << "Clks (incl. 0.5s Reserve) (" << ((double)sr->probe_time_estimate/22000.) << "s)" << std::endl);

	sr->ReadProbeFifo (sr->probe_thread_dsp, FR_INIT); // init

	int i=1;
	while (sr->is_scanning () || finish_flag){
		if (DSPControlClass->current_auto_flags & FLAG_AUTO_PLOT)
			DSPControlClass->Probing_graph_update_thread_safe ();
			//DSPControlClass->Probing_graph_callback (NULL, DSPControlClass, 0);
		++i;
		switch (sr->ReadProbeFifo (sr->probe_thread_dsp)){
		case RET_FR_NOWAIT:
			LOGMSGS2 ( ":NOWAIT:" << i << " TmoClk=" << (timeout-clock()) << std::endl);
			continue;
		case RET_FR_WAIT:
			LOGMSGS ( ":WAIT:" << i << " TmoClk=" << (timeout-clock()) << std::endl);
			usleep(50000);
			if (finish_flag && clock() > timeout)
			    goto error;
			continue;
		case RET_FR_OK:
			LOGMSGS2 ( ":OK:" << i << " TmoClk=" << (timeout-clock()) << std::endl);
			continue;
		case RET_FR_ERROR:
			LOGMSGS ( ":ERROR:" << i << " TmoClk=" << (timeout-clock()) << std::endl);
			goto error;
		case RET_FR_FCT_END: 
			LOGMSGS ( ":FCT_END:" << i << " TmoClk=" << (timeout-clock()) << std::endl);
			if (finish_flag){
				if (DSPControlClass->current_auto_flags & FLAG_AUTO_PLOT)
                                        DSPControlClass->Probing_graph_update_thread_safe (1);
                                        //DSPControlClass->Probing_graph_callback (NULL, DSPControlClass, 1);
				if (DSPControlClass->current_auto_flags & FLAG_AUTO_SAVE)
					DSPControlClass->Probing_save_callback (NULL, DSPControlClass);

				goto finish;
			}
			DSPControlClass->push_probedata_arrays ();
			DSPControlClass->init_probedata_arrays ();

			// reset timeout
//			timeout = clock() + (clock_t)(CLOCKS_PER_SEC*(0.5+sr->probe_time_estimate/22000.));
			continue;
		}
		LOGMSGS ( ":FIFO THREAD ERROR DETECTION:" << i << " TmoClk=" << (timeout-clock()) << std::endl);
		goto error;
	}
finish:
	LOGMSGS ( "ProbeFifoReadThread DONE  Single-VP[#" << sr->probe_fifo_thread_active << "] Timeout left (positive is OK):" << (timeout-clock()) << std::endl);

error:
	sr->ReadProbeFifo (sr->probe_thread_dsp, FR_FINISH); // finish

	--sr->probe_fifo_thread_active;
	DSPControlClass->probe_ready = TRUE;

	return NULL;
}

// ProbeFifoReadFunction:
// inlineable ProbeFifoRead Function -- similar to the thread, is called/inlined by/into the Image Data Thread
gpointer ProbeFifoReadFunction (void *ptr_sr, int dspdev){
	sranger_hwi_dev *sr = (sranger_hwi_dev*)ptr_sr;

	while (sr->is_scanning ()){
		switch (sr->ReadProbeFifo (dspdev)){
		case RET_FR_NOWAIT:
			continue;
		case RET_FR_WAIT:
			return NULL;
		case RET_FR_OK:
			continue;
		case RET_FR_ERROR:
			goto error;
		case RET_FR_FCT_END: 
			if (DSPControlClass->probedata_length () > 0){
				DSPControlClass->push_probedata_arrays ();
				DSPControlClass->init_probedata_arrays ();
				LOGMSGS ( "ProbeFifoReadFunction -- Pushed Probe Data on Stack" << std::endl);
			}
			continue;
		}
		goto error;
	}
error:
	return NULL;
}

// ReadProbeFifo:
// read from probe FIFO, this engine needs to be called several times from master thread/function
int sranger_hwi_dev::ReadProbeFifo (int dspdev, int control){
	int pvd_blk_size=0;
	static double pv[9];
	static int last = 0;
	static int last_read_end = 0;
	static DATA_FIFO_EXTERN_PCOPY fifo;
	static PROBE_SECTION_HEADER section_header;
	static int next_header = 0;
	static int number_channels = 0;
	static int data_index = 0;
	static int end_read = 0;
	static int data_block_size=0;
	static int need_fct = FR_YES;  // need fifo control
	static int need_hdr = FR_YES;  // need header
	static int need_data = FR_YES; // need data
	static int ch_lut[32];
	static int ch_msk[]  = { 0x0000001, 0x0000002,   0x0000010, 0x0000020, 0x0000040, 0x0000080,   0x0000100, 0x0000200, 0x0000400, 0x0000800,
				 0x0000008, 0x0001000, 0x0002000, 0x0004000, 0x0008000,   0x0000004,   0x0000000 };
	static int ch_size[] = {    2, 2,    2, 2, 2, 2,   2, 2, 2, 2,    4,   4,  4,  4,   4,   4,  0 };
	const char *ch_header[] = {"Zmon-AIC5Out", "Umon-AIC6Out", "AIC5-I", "AIC0", "AIC1", "AIC2", "AIC3", "AIC4", "AIC6", "AIC7",
				    "LockIn0", "LockIn1stA", "LockIn1stB", "LockIn2ndA", "LockIn2ndB", "Count", NULL };
	static short data[EXTERN_PROBEDATAFIFO_LENGTH];
	static double dataexpanded[16];
#ifdef LOGMSGS0
	static double dbg0=0., dbg1=0.;
	static int dbgi0=0;
#endif

	switch (control){
	case FR_FIFO_FORCE_RESET: // not used normally -- FIFO is reset by DSP at probe_init (single probe)
		fifo.r_position = 0;
		fifo.w_position = 0;
		check_and_swap (fifo.r_position);
		check_and_swap (fifo.w_position);
		lseek (dspdev, magic_data.probedatafifo, SRANGER_SEEK_DATA_SPACE);
		sr_write (dspdev, &fifo, 2*sizeof(DSP_INT)); // reset positions now to prevent reading old/last dataset before DSP starts init/putting data
		return RET_FR_OK;

	case FR_INIT:
		last = 0;
		next_header = 0;
		number_channels = 0;
		data_index = 0;
		last_read_end = 0;

		need_fct  = FR_YES;
		need_hdr  = FR_YES;
		need_data = FR_YES;

		DSPControlClass->init_probedata_arrays ();
		for (int i=0; i<16; dataexpanded[i++]=0.);

		LOGMSGS0 ( std::endl << "************** PROBE FIFO-READ INIT **************" << std::endl);
		LOGMSGS ( "FR::INIT-OK." << std::endl);
		return RET_FR_OK; // init OK.

	case FR_FINISH:
		LOGMSGS ( "FR::FINISH-OK." << std::endl);
		return RET_FR_OK; // finish OK.

	default: break;
	}

	if (need_fct){ // read and check fifo control?
		LOGMSGS2 ( "FR::NEED_FCT, last: 0x"  << std::hex << last << std::dec << std::endl);

		lseek (dspdev, magic_data.probedatafifo, SRANGER_SEEK_DATA_SPACE);
		sr_read  (dspdev, &fifo, sizeof (fifo));
		check_and_swap (fifo.w_position);
		check_and_swap (fifo.current_section_head_position);
		check_and_swap (fifo.current_section_index);
		check_and_swap (fifo.p_buffer_base);
		end_read = fifo.w_position >= last ? fifo.w_position : EXTERN_PROBEDATAFIFO_LENGTH;

		// check for new data
		if ((end_read - last) < 1)
			return RET_FR_WAIT;
		else {
			need_fct  = FR_NO;
			need_data = FR_YES;
		}
	}

	if (need_data){ // read full FIFO block
		LOGMSGS ( "FR::NEED_DATA" << std::endl);

		int database = (int)fifo.p_buffer_base;
		int dataleft = end_read;
		int position = 0;
		if (fifo.w_position > last_read_end){
//			database += last_read_end;
//			dataleft -= last_read_end;
		}
		for (; dataleft > 0; database += 0x4000, dataleft -= 0x4000, position += 0x4000){
			LOGMSGS ( "FR::NEED_DATA: B::0x" <<  std::hex << database <<  std::dec << std::endl);
			lseek (dspdev, database, SRANGER_SEEK_DATA_SPACE);
			sr_read  (dspdev, &data[position], (dataleft >= 0x4000 ? 0x4000 : dataleft)<<1);
		}
		last_read_end = end_read;
			
		need_data = FR_NO;
	}

	if (need_hdr){ // we have enough data if control gets here!
		LOGMSGS ( "FR::NEED_HDR" << std::endl);

		// check for FIFO loop
		if (last > (EXTERN_PROBEDATAFIFO_LENGTH - EXTERN_PROBEDATA_MAX_LEFT)){
			LOGMSGS0 ( "FR:FIFO LOOP DETECTED -- FR::NEED_HDR ** Data @ " 
				   << "0x" << std::hex << last
				   << " -2 :[" << (*((DSP_LONG*)&data[last-2]))
				   << " " << (*((DSP_LONG*)&data[last]))
				   << " " << (*((DSP_LONG*)&data[last+2]))
				   << " " << (*((DSP_LONG*)&data[last+4]))
				   << " " << (*((DSP_LONG*)&data[last+6])) 
				   << "] : FIFO LOOP MARK " << std::dec << ( *((DSP_LONG*)&data[last]) == 0 ? "OK":"ERROR")
				   << std::endl);
			next_header -= last;
			last = 0;
			end_read = fifo.w_position;
		}

		if (((fifo.w_position - last) < (sizeof (section_header)>>1))){
			need_fct  = FR_YES;
			return RET_FR_WAIT;
		}

		memcpy ((void*)&(section_header.srcs), (void*)(&data[next_header]), sizeof (section_header));
		check_and_swap (section_header.srcs);
		check_and_swap (section_header.n);
		check_and_swap (section_header.time);
		check_and_swap (section_header.x_offset);
		check_and_swap (section_header.y_offset);
		check_and_swap (section_header.z_offset);
		check_and_swap (section_header.x_scan);
		check_and_swap (section_header.y_scan);
		check_and_swap (section_header.z_scan);
		check_and_swap (section_header.u_scan);
		check_and_swap (section_header.section);
		
		// set vector of expanded data array representation, section start
		pv[0] = section_header.time;
		pv[1] = section_header.x_offset;
		pv[2] = section_header.y_offset;
		pv[3] = section_header.z_offset;
		pv[4] = section_header.x_scan;
		pv[5] = section_header.y_scan;
		pv[6] = section_header.z_scan;
		pv[7] = section_header.u_scan;
		pv[8] = section_header.section;

		LOGMSGS ( "FR::NEED_HDR -- got HDR @ 0x" << std::hex << next_header << std::dec 
			  << " section: " << section_header.section
			  << " time: " << section_header.time 
			  << " XYZ: " << section_header.x_scan << ", " << section_header.y_scan  << ", " << section_header.z_scan 
			  << std::endl);

		// validate header -- if stupid things are happening, values are messed up so check:
		if (section_header.time < 0 || section_header.section < 0 || section_header.section > 50 || section_header.n < 0 || section_header.n > 10000){
			LOGMSGS0 ( "************** FIFO READ ERROR DETECTED: bad section header **************" << std::endl);
			LOGMSGS0 ( "==> read bad HDR @ 0x" << std::hex << next_header << std::dec 
				   << " last: 0x" << std::hex << last << std::dec 
				   << " section: " << section_header.section
				   << " time: " << section_header.time 
				   << " XYZ: " << section_header.x_scan << ", " << section_header.y_scan  << ", " << section_header.z_scan 
				   << std::endl);
			LOGMSGS0 ( "Last Sec: " << dbg0 << "Last srcs: " << dbg1 << " last last: 0x" << std::hex << dbgi0 << std::dec << std::endl);
			// baild out and try recovery
			goto auto_recover_and_debug;
		}
#ifdef LOGMSGS0
		dbg0 = section_header.section;
		dbg1 = section_header.srcs;
		dbgi0 = last;
#endif

		need_hdr = FR_NO;

		// analyze header and setup channel lookup table
		number_channels=0;
		last += sizeof (section_header) >> 1;
		next_header += sizeof (section_header) >> 1;

		data_block_size = 0;
		if (section_header.srcs){
			LOGMSGS ( "FR::NEED_HDR: decoding DATA_SRCS to read..." << std::endl);
			for (int i = 0; ch_msk[i] && i < 18; ++i){
				if (section_header.srcs & ch_msk[i]){
					ch_lut[number_channels] = i;
					data_block_size += ch_size[i];
					++number_channels;
				}
				if (i == 10 && (data_block_size>>1) & 1){ // adjust for even LONG position
					data_block_size += 2;
					ch_lut[number_channels] = -1; // dummy fill
					++number_channels;
				}
					    
			}
			next_header += section_header.n * (data_block_size >> 1);
			data_index = 0;

		} else {
			LOGMSGS ( "FR::NEED_HDR: DATA_SRCS ZERO." << std::endl);

			if (section_header.n == 0){
				LOGMSGS ( "FR::NEED_HDR: DATA_N ZERO -> END PROBE OK." << std::endl);
				DSPControlClass->probe_trigger_single_shot = 0; // if single shot, stop reading next
				number_channels = 0;
				data_index = 0;
				need_hdr = FR_YES;
				return RET_FR_FCT_END;
			} else {
				LOGMSGS ( "FR::NEED_HDR: no data in this section." << std::endl);
			}

			LOGMSGS ( "FR::NEED_HDR: need next hrd!" << std::endl);
			need_hdr = FR_YES;
			return RET_FR_NOWAIT;
		}
	}

	// now read/convert all available/valid data from fifo block we just copied
	pvd_blk_size = data_block_size >> 1; // data block size in "word"-indices
	LOGMSGS ( "FR::DATA-cvt:"
		  << " pvd_blk-sz="  << pvd_blk_size 
		  << " last=0x"      << std::hex << last << std::dec
		  << " next_header=0x" << std::hex << next_header << std::dec
		  << " end_read=0x"  << std::hex << end_read << " EPL:0x" << EXTERN_PROBEDATAFIFO_LENGTH << std::dec 
		  << " data_index="  << data_index << std::dec
		  << std::endl);

	for (int element=0; last < end_read; ++last, ++data_index){

		// got all data, at next header?
		if (last == next_header){
			LOGMSGS ( "FR:NEXT HDR EXPECTED" << std::endl);
			need_hdr = FR_YES;
			return RET_FR_NOWAIT;
		}
		if (last > next_header){
			LOGMSGS0 ( "FR:ERROR:: =====> MISSED NEXT HDR?"
				   << " last: 0x" << std::hex << last
				   << " next_header: 0x" << std::hex << next_header
				   << std::endl);
			goto auto_recover_and_debug;
		}

		int channel = ch_lut[element++];
		if (channel >= 0){ // only if valid (skip possible dummy (2) fillings)
			// check for long data (4)
			if (ch_size[channel] == 4){
				LNG *tmp = (LNG*)&data[last];
				check_and_swap (*tmp);
				dataexpanded[channel] = (double) (*tmp);
				++last; ++data_index;
			} else { // normal data (2)
				check_and_swap (data[last]);
				dataexpanded[channel] = (double) data[last];
			}
		}

		// add vector and data to expanded data array representation
		if ((data_index % pvd_blk_size) == (pvd_blk_size-1)){
			if (data_index >= pvd_blk_size) // skip to next vector
				DSPControlClass->add_probevector ();
			else // set vector as previously read from section header (pv[]) at sec start
				DSPControlClass->set_probevector (pv);
			// add full data vector
			DSPControlClass->add_probedata (dataexpanded);
			element = 0;

			// check for FIFO loop now
			if (last > (EXTERN_PROBEDATAFIFO_LENGTH - EXTERN_PROBEDATA_MAX_LEFT)){
				++last;
				LOGMSGS0 ( "FR:FIFO LOOP DETECTED ** Data @ " 
					   << "0x" << std::hex << last
					   << " -2 :[" << (*((DSP_LONG*)&data[last-2]))
					   << " " << (*((DSP_LONG*)&data[last]))
					   << " " << (*((DSP_LONG*)&data[last+2]))
					   << " " << (*((DSP_LONG*)&data[last+4]))
					   << " " << (*((DSP_LONG*)&data[last+6])) 
					   << "] : FIFO LOOP MARK " << std::dec << ( *((DSP_LONG*)&data[last]) == 0 ? "OK":"ERROR")
					   << std::endl);
				next_header -= last;
				last = -1; // compensate for ++last at of for(;;)!
				end_read = fifo.w_position;
			}
		}
	}

	LOGMSGS ( "FR:FIFO NEED FCT" << std::endl);
	need_fct = FR_YES;

	return RET_FR_WAIT;


// emergency bailout and auto recovery, FIFO restart
auto_recover_and_debug:

#ifdef LOGMSGS0
	LOGMSGS0 ( "************** -- FIFO DEBUG -- **************" << std::endl);
	LOGMSGS0 ( "LastArdess: " << "0x" << std::hex << last << std::dec << std::endl);
	for (int adr=last-8; adr < last+32; adr+=8){
		while (adr < 0) ++adr;
		LOGMSGS0 ("0x" << std::hex << adr << "::"
			  << " " << (*((DSP_LONG*)&data[adr]))
			  << " " << (*((DSP_LONG*)&data[adr+2]))
			  << " " << (*((DSP_LONG*)&data[adr+4]))
			  << " " << (*((DSP_LONG*)&data[adr+6]))
			  << std::dec << std::endl);
	}
	LOGMSGS0 ( "************** TRYING AUTO RECOVERY **************" << std::endl);
	LOGMSGS0 ( "***** -- STOP * RESET FIFO * START PROBE -- ******" << std::endl);
#endif

	// STOP PROBE
	// reset positions now to prevent reading old/last dataset before DSP starts init/putting data
	DSP_INT start_stop[2] = { 0, 1 };
	lseek (dspdev, magic_data.probe, SRANGER_SEEK_DATA_SPACE);
	sr_write (dspdev, &start_stop, 2*sizeof(DSP_INT));

	// RESET FIFO
	// reset positions now to prevent reading old/last dataset before DSP starts init/putting data
	fifo.r_position = 0;
	fifo.w_position = 0;
	check_and_swap (fifo.r_position);
	check_and_swap (fifo.w_position);
	lseek (dspdev, magic_data.probedatafifo, SRANGER_SEEK_DATA_SPACE);
	sr_write (dspdev, &fifo, 2*sizeof(DSP_INT));

	// START PROBE
	// reset positions now to prevent reading old/last dataset before DSP starts init/putting data
	start_stop[0] = 1;
	start_stop[1] = 0;
	lseek (dspdev, magic_data.probe, SRANGER_SEEK_DATA_SPACE);
	sr_write (dspdev, &start_stop, 2*sizeof(DSP_INT));

	// reset all internal and partial reinit FIFO read thread
	last = 0;
	next_header = 0;
	number_channels = 0;
	data_index = 0;
	last_read_end = 0;
	need_fct  = FR_YES;
	need_hdr  = FR_YES;
	need_data = FR_YES;

	// and start over on next call
	return RET_FR_WAIT;
}





// ==================================================
// obsolete

int sranger_hwi_dev::ReadScanData(int y_index, int num_srcs, Mem2d *m[MAX_SRCS_CHANNELS]){
	return 0; // done by thread
}

int sranger_hwi_dev::ReadProbeData(int nsrcs, int nprobe, int kx, int ky, Mem2d *m, double scale){
	// this function is obsolete since vector probe
	return 1;
}

/*
 * provide some info about the connected hardware/driver/etc.
 */
gchar* sranger_hwi_dev::get_info(){
	static gchar *magic_info = NULL;
	if (productid){
		gchar *details = NULL;
		if (FB_SPM_VERSION != magic_data.version){
			details = g_strdup_printf(
				"WARNING: Signal Ranger FB_SPM soft Version mismatch detected:\n"
				"Detected SRanger DSP Software Version: %x.%02x\n"
				"Detected SRanger DSP Software ID: %04X\n"
				"GXSM was build for DSP Software Version: %x.%02x\n"
				"And DSP Software ID %4X is recommended.\n"
				"This may cause incompatility problems, trying to proceed.\n",
				magic_data.version >> 8, 
				magic_data.version & 0xff,
				magic_data.dsp_soft_id,
				FB_SPM_VERSION >> 8, 
				FB_SPM_VERSION & 0xff,
				FB_SPM_SOFT_ID);
		} else {
			details = g_strdup_printf("Errors/Warnings: none\n");
		}
		g_free (magic_info); // g_free irgnors NULL ptr!
		magic_info = g_strdup_printf (
			"Magic....... : %04X\n"
			"Version..... : %04X\n"
			"Date........ : %04X%04X\n"
			"*-- DSP Control Struct Locations --*\n"
			"statemachine : %04x\n"
			"feedback.... : %04x\n"
			"dfm_fuzzymix.: %04x\n"
			"analog...... : %04x\n"
			"move........ : %04x\n"
			"scan........ : %04x\n"
			"probe....... : %04x\n"
			"autoapproach : %04x\n"
			"datafifo.... : %04x\n"
			"probedatafifo: %04x\n"
			"------------------------------------\n"
			"%s",
			magic_data.magic,
			magic_data.version,
			magic_data.year,
			magic_data.mmdd,
			magic_data.statemachine,
			magic_data.feedback,
			magic_data.dfm_fuzzymix,
			magic_data.analog,
			magic_data.move,
			magic_data.scan,
			magic_data.probe,
			magic_data.autoapproach,
			magic_data.datafifo,
			magic_data.probedatafifo,
			details
			);
		g_free (details);
		return g_strconcat (
			"*--GXSM Sranger HwI base class--*\n",
			"Sranger device: ", productid, "\n",
			"*--Features--*\n",
			FB_SPM_FEATURES,
			"*--Magic Info--*\n",
			magic_info,
			"*--EOF--*\n",
			NULL
			); 
	}
	else
		return g_strdup("*--GXSM Sranger HwI base class--*\n"
				"Sranger device not connected\n"
				"or not supported.");
}

double sranger_hwi_dev::GetUserParam (gint n, gchar *id){
	return DSPControlClass->GetUserParam (n, id);
}

gint sranger_hwi_dev::SetUserParam (gint n, gchar *id, double value){
	return DSPControlClass->SetUserParam (n, id, value);
}

