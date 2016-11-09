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

/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* irnore this module for docuscan
% PlugInModuleIgnore
 */


#include <locale.h>
#include <libintl.h>


#include "glbvars.h"
#include "plug-ins/hard/modules/dsp.h"
#include <fcntl.h>
#include <sys/ioctl.h>

#include "dsp-pci32/xsm/dpramdef.h"
#include "dsp-pci32/xsm/xsmcmd.h"

#include "innovative_dsp_hwi.h"

/* ============================================================
 * Hardwareimplementation hilevel DSP:
 * Virtuelle Funktionen der Basisklasse werden überschrieben
 * ============================================================ 
 */

/* Konstruktor: initialisiert Hardware
 * ==================================================
 * - device öffnen
 */
innovative_dsp_hwi_dev::innovative_dsp_hwi_dev(int ver):XSM_Hardware(){
	EventCheckOn();

	dsp_cmd  = open(xsmres.DSPDev, O_RDWR);
	dsp_usr  = open(xsmres.DSPDev, O_RDWR);
	dsp_data = open(xsmres.DSPDev, O_RDWR);
	if(dsp_cmd <= 0){ 
		printf("open %s failed, err=%d\nPlease do \"insmod pci32, pc31, ccd, dspspaemu or dspbbspa...\" (you need root for this) !\n\n"
		       "Or if you want to go without hardware, you can temporary disable hardware access:\n use: \"gxsm -h no\"\n",
		       xsmres.DSPDev, dsp_cmd);
		exit(0);
	}
	if(dsp_data <= 0 || dsp_usr <= 0){
		printf("second open %s failed, err=%d\nsorry, something goes wrong !\n", xsmres.DSPDev, dsp_data);
		close(dsp_cmd);
		exit(0);
	}

	switch(ioctl(dsp_cmd, PCDSP_GETMODID)){
	case PCDSP_MODID_SIM: InfoString = g_strdup("Module is SIM"); break;
	case PCDSP_MODID_PC31: InfoString = g_strdup("Module is PC31"); break;
	case PCDSP_MODID_PCI32: InfoString = g_strdup("Module is PCI32"); break;
	default: 
		InfoString = g_strdup("Module Typ is unknown!");
	}
	/* SEM0: default, MBOX */
	ioctl(dsp_cmd, PCDSP_SEM1START, DSP_CTRL_REGION);
	ioctl(dsp_cmd, PCDSP_SEM1LEN,   DSP_CTRL_REG_LEN);

	ioctl(dsp_usr, PCDSP_SEM2START, DSP_USR_REGION);
	ioctl(dsp_usr, PCDSP_SEM2LEN,   DSP_USR_REG_LEN);

	ioctl(dsp_data, PCDSP_SEM3START, DSP_DATA_REGION);
	ioctl(dsp_data, PCDSP_SEM3LEN,   DSP_DATA_REG_LEN);

	lseek(dsp_cmd,  BYTSIZE(DSP_CTRL_REGION), SEEK_SET);
	lseek(dsp_usr,  BYTSIZE(DSP_USR_REGION),  SEEK_SET);
	lseek(dsp_data, BYTSIZE(DSP_DATA_REGION), SEEK_SET);

	in_use_count=0;

	// ask DSP for software version and config report
	g_free(get_DSP_softinfo ());
}

/* Destruktor:
 * ==================================================
 * Hardware "abtrennen"
 */
innovative_dsp_hwi_dev::~innovative_dsp_hwi_dev(){
	close(dsp_cmd);
	close(dsp_usr);
	close(dsp_data);
}

void innovative_dsp_hwi_dev::ExecCmd(int Cmd){ 
	unsigned long dspctrl, l;
	clock_t timeout;
	dspctrl = (unsigned long)Cmd;

	if(write(dsp_cmd, &dspctrl, BYTSIZE(DSP_CTRL_PARAM)))
		XSM_SHOW_ALERT(N_("Attention"), N_("ExecCmd: DSP write failed!"), " ", 0);

	timeout=clock()+DSP_TIMEOUT*CLOCKS_PER_SEC;
	while(!(l=ioctl(dsp_cmd, PCDSP_MBOX_WRITE_NOWAIT, dspctrl)) && timeout > clock())
		gapp->check_events();
	if(!l)
		XSM_SHOW_ALERT(N_("Attention"), N_("ExecCmd: Timeout, DSP write mbox failed!"), " ", 0);
}

int  innovative_dsp_hwi_dev::WaitExec(int data){
	return 0;
}

void innovative_dsp_hwi_dev::Evchk(){ gapp->check_events(); usleep(5000); };

void innovative_dsp_hwi_dev::NoEvchk(){ usleep(5000); }; // don´t waste power here !


// Parameterliste an DSP <FC>bergeben und Cmd absetzten
void innovative_dsp_hwi_dev::SetParameter(PARAMETER_SET &hps, int scanflg){
	static clock_t last_error_time = 0;
	static int error_count = 0;
	static int errmsg_disable = 0;
#define CHECK_ERR_DTIME { \
		if (last_error_time+5 > clock()) errmsg_disable = 1;\
		if (++error_count > 20) errmsg_disable = 1;\
 		last_error_time = clock(); \
                        }
#define SHOW_MSG(A,B) { \
		if (errmsg_disable) \
			std::cerr << "innovative_dsp_hwi_dev::SetParameter Error Message: (to many, redirected to console)" << std::endl \
			     << "--  " << A << std::endl \
			     << "--  " << B << std::endl; \
                else \
         	        gapp->alert (N_("Attention"), A, B, 0); \
	}
	clock_t timeout;
	unsigned long l;
	unsigned long dspctrl[DSP_CTRL_REG_LEN];
	union fl { float  f; unsigned long hl; } Fl;
       
	static int busy=0;
	XSM_DEBUG (DBG_L2, "Calling instance #"<< busy <<" of SetParameter.");
	busy++; // set mutex
	// simple mutex mechanism -- got some trouble with "recursive" calls
	int k=0;
	while (busy > 1) { 
		usleep (50000); //gapp->check_events(); 
		if (k==0)
		  XSM_DEBUG (DBG_L2, "SetParameter is busy [" << busy << "] - waiting.");
		if (++k > 20){
		  XSM_DEBUG (DBG_L2, "-- Giving up waiting on DSP and trying to force setting parameters. Releasing Mutex "<< busy<<" --");
		  --busy; // Releasing Mutex
		  break;
		}
	}
       
	dspctrl[DSP_CTRL_CMD] = (unsigned long)hps.Cmd;
	for(int i=0; i<hps.N; i++){
		Fl.f = hps.hp[i].value;
		dspctrl[i + DSP_CTRL_PARAM] = Fl.hl;
	}

	// set timeout
	timeout=clock()+DSP_TIMEOUT*CLOCKS_PER_SEC;

	// wait until last cmd finished ? (new here)
	while(!(l=ioctl(dsp_cmd, PCDSP_MBOX_EMPTY, FALSE)) && timeout > clock())
		(*EventCheck)();

	// write Cmd and parameters
	if(write(dsp_cmd, dspctrl, BYTSIZE(hps.N+DSP_CTRL_PARAM))){
		CHECK_ERR_DTIME;
		SHOW_MSG (N_("DSP write parameters failed !"),
			  N_("cause: error while writing to device.")
			  );
	}

	// send cmd (Mailbox write)
	if(scanflg){
		while(!(l=ioctl(dsp_cmd, PCDSP_MBOX_WRITE_NOWAIT,  hps.Cmd)) && timeout > clock())
			(*EventCheck)();
		if(!l){
			CHECK_ERR_DTIME;
			SHOW_MSG (N_("SetParameter: DSP write mbox failed."),
				  N_("cause: timeout, in scan, cmd=PCDSP_MBOX_WRITE_NOWAIT"));
		}

		// Do some drawing, view update, etc inbetween?
		CallIdleFunc();

		// wait for Cmd execute OK (Mailbox empty)
    
		int k=0;
		while(!(l=ioctl(dsp_cmd, PCDSP_MBOX_EMPTY, FALSE)) && timeout > clock()){
			if(k-- >= 0)
				NoEvchk();
			else
				(*EventCheck)();
			// scan stopped ?
			if(KillFlg){
			  XSM_DEBUG (DBG_L2, "Release Mutex "<<busy<<" due to KillFlag set.");
			  --busy; // release mutex
			}
			return;
		}
	}else{
		while(!(l=ioctl(dsp_cmd, PCDSP_MBOX_WRITE_NOWAIT, hps.Cmd)) && timeout > clock())
			Evchk();
		if(!l){
			CHECK_ERR_DTIME;
			SHOW_MSG (N_("SetParameter: DSP write mbox failed."),
				  N_("cause: timeout, no scan"));
		}
		// wait for Cmd execute OK (Mailbox empty)
		while(!(l=ioctl(dsp_cmd, PCDSP_MBOX_EMPTY, FALSE)) && timeout > clock())
			Evchk();
	}
	if(!l){
		gchar *tt=g_strdup_printf("timeout=%ds reached: waiting since %gs", 
					  DSP_TIMEOUT, (double)((clock()-timeout)/CLOCKS_PER_SEC));
		CHECK_ERR_DTIME;
		SHOW_MSG (N_("SetParameter: DSP wait for mbox empty failed. \n"
			     "--  cause: timeout; write data and cmd exec OK"),
			  tt);
		g_free(tt);
	}
	XSM_DEBUG (DBG_L2, "Release Mutex " <<busy<<" due to end of function call");
	--busy; //release mutex
}

// Parameter von DSP anfordern
void innovative_dsp_hwi_dev::GetParameter(PARAMETER_SET &hps){
	unsigned long dspctrl[DSP_CTRL_REG_LEN];
	union fl { float  f; unsigned long hl; } Fl;
  
	// read parameter-area
	if(read(dsp_cmd, dspctrl, BYTSIZE(hps.N+DSP_CTRL_PARAM)))
		gapp->alert("Achtung", "DSP read parameters failed !", " ", 0);

	for(int i=0; i<hps.N; i++){
		Fl.hl = dspctrl[i + DSP_CTRL_PARAM];
		hps.hp[i].value = Fl.f;
	}
}

gchar* innovative_dsp_hwi_dev::get_DSP_softinfo (){
	PARAMETER_SET hardpar;
	#define TMPSIZE 1000
	gchar infotxt[TMPSIZE+1];
	int i;

	scan_data_mode = SCAN_DATA_SWAP_SHORT;
	prb_data_mode = PRB_SHORT;
	max_points_per_line = DSP_DATA_REG_LEN<<1;

	XSM_DEBUG (DBG_L2, "innovative_dsp_hwi_dev::get_DSP_softinfo -- Asking DSP to report software info." );

	hardpar.N   = 1;
	hardpar.Cmd = DSP_CMD_GETINFO;
	hardpar.hp[0].value = 0.;
	SetParameter(hardpar, FALSE);
	
	long infobuffer[DSP_DATA_REG_LEN];

	// read info data buffer
	ReadData(infobuffer, DSP_DATA_REG_LEN*sizeof(long));

	memset (infotxt, 0, sizeof(infotxt));
	// check for alignement, DSP has long, kernel doese char
	if (!strncmp ((char*)infobuffer, "*123-567-9abXdef", 16)){
		XSM_DEBUG(DBG_L2, "++ KERNEL-EMU-DETECTED" );
		strcpy (infotxt, 
			"*--XSM-DSP-SOFT-INFO--*\n"
			"--*Kernel-Emu-Detected*--\n");
		memcpy ((void*)&infotxt[strlen(infotxt)], (void*)&infobuffer[92/4], 
			MIN ((strlen ((char*)&infobuffer[92/4])-1), (TMPSIZE-100)));
	}else{
		XSM_DEBUG(DBG_L2, "-- DSP-W-LONGMODE-DPRAM-SUGGESTED" );
		for(i=0; i<TMPSIZE && infobuffer[i]!='@'; ++i)
			infotxt[i] = (gchar)infobuffer[i];
		infotxt[i]=0;
	}
	if (strncmp (infotxt, "*--XSM-DSP-SOFT-INFO--*", 23)){
		XSM_DEBUG(DBG_L2, "-- AUTOMATIC DEVICE IDENTIFY FAILED" );
		infobuffer[24] = 0;
		gchar *tmp = g_strdup_printf ("Sorry, this DSP software does not support reports!\n"
					      "Identify String found:\n"
					      "<%s>\n",
					      (char*)infobuffer);
		XSM_DEBUG_PLAIN (DBG_L2, tmp);
		return  (tmp);
	}
	XSM_DEBUG(DBG_L2, "++ AUTOMATIC DEVICE IDENTIFY OK, REPORT:" );
	XSM_DEBUG_PLAIN (DBG_L2, infotxt );
	XSM_DEBUG(DBG_L2, "** END OF REPORT" );

	// now check some options:
	if (strstr (infotxt, "ProbeDataMode: short")){
		prb_data_mode = PRB_SHORT;
		XSM_DEBUG(DBG_L2, "DSP ProbeData Mode is SHORT." );
	} else if (strstr (infotxt, "ProbeDataMode: float")){
		prb_data_mode = PRB_FLOAT;
		XSM_DEBUG(DBG_L2, "DSP ProbeData Mode is FLOAT." );
	} else
		XSM_DEBUG(DBG_L2, "DSP ProbeData Mode not reported, using SHORT." );

	if (strstr (infotxt, "DPRAMDATAMOVE: yes")){
		scan_data_mode = SCAN_DATA_MOVE2DPRAM_SHORT;
		XSM_DEBUG(DBG_L2, "Scan Data Mode is MOVE2DPRAM." );
	} else if (strstr (infotxt, "DPRAMSWAP: yes")){
		scan_data_mode = SCAN_DATA_SWAP_SHORT;
		XSM_DEBUG(DBG_L2, "Scan Data Mode is SWAP (deprecated mode)." );
	} else
		XSM_DEBUG(DBG_L2, "Scan Data ModeDSP not reported, using old SWAP method." );

	gchar *dpl;
	if ((dpl = strstr (infotxt, "DATABUFFERLEN: "))){
		max_points_per_line = atoi (dpl + 15)<<1;
		XSM_DEBUG(DBG_L2, "Found DATABUFFERLEN, setting max points per line to " << max_points_per_line );
	}

	return (g_strdup (infotxt));
}

size_t innovative_dsp_hwi_dev::ReadData(void *buf, size_t count) { 
	// may be adding auto-swap-mode later... PZ
	return read(dsp_data, buf, count); 
}

int innovative_dsp_hwi_dev::ReadScanData(int y_index, int num_srcs, Mem2d *m[MAX_SRCS_CHANNELS]){
	int len = m[0]->GetNx();
	if (scan_data_mode == SCAN_DATA_MOVE2DPRAM_SHORT){
		if( len & 1 ) ++len; // is always even!
		int mv_buf_len  = len*num_srcs; // max bufer size for data move
		if (y_index == -1) // get all HS_areascan data!
			mv_buf_len *= m[0]->GetNy();
		int dataset_len = mv_buf_len;   // total dataset buffer size
		if (mv_buf_len > (DSP_DATA_REG_LEN<<1))
			mv_buf_len = (DSP_DATA_REG_LEN<<1); 
		SHT *linebuffer = new SHT[dataset_len];
		// move full dataset to buffer first
		int mv_len = mv_buf_len;
		int offset = 0;
		do{
			ReadData (&linebuffer[offset], mv_len*sizeof(SHT));
			offset += mv_buf_len;
			if (offset < dataset_len){
				PARAMETER_SET hardpar;
				hardpar.N   = DSP_LSSRCS+1;
				hardpar.Cmd = DSP_CMD_MOVE_DATA2DPRAM;
				hardpar.hp[DSP_MVD_CMD_START].value = offset>>1; // index in DPRAM dwords
				hardpar.hp[DSP_MVD_CMD_LEN  ].value = (mv_len = ((dataset_len-offset) > mv_buf_len 
										 ? mv_buf_len : dataset_len-offset)) >> 1;
				SetParameter (hardpar, TRUE);
			} else break;
		}while (1);

		int y = y_index == -1 ? 0:y_index;
		for (int offset=0; y < m[0]->GetNy(); ++y){
			for (int i=0; i<num_srcs; ++i)
				if (m[i])
					m[i]->PutDataLine (y, linebuffer+ i*len + offset);
			if (y_index == -1)
				offset += len*num_srcs;
			else
				break; // single line mode
		}

		delete[] linebuffer;
	}else{
		if (y_index < 0) return 0; // HS Capture not supported!
		// Transfer Data, using old SWAP method
		SHT linebuffer[DSP_DATA_REG_LEN<<1]; // Max Size for LineData "at once"
		size_t elem_size=sizeof(SHT);
		size_t bsz = (len*elem_size);
		int i = 0;
		int n = 0;
		do{
			size_t sz = 0;
			int bin = 0;
			while (n < num_srcs && (sz+bsz) < (DSP_DATA_REG_LEN<<2)){
				sz += bsz;
				++n;
				++bin;
			}
			if (sz > 0){
				// read data buffer(s)
				ReadData(linebuffer, sz);
				int j = 0;
				do{
					m[i++]->PutDataLine (y_index, linebuffer+(j++)*len);
				}while(--bin && (i < MAX_SRCS_CHANNELS) ? m[i] != NULL : FALSE);
			}
			else 
				break;
			// Swap DPRAM Buffers...
			ExecCmd(DSP_CMD_SWAPDPRAM);
		}while ((i < MAX_SRCS_CHANNELS) ? m[i]!=NULL : FALSE);
	}
	return 0;
}

int innovative_dsp_hwi_dev::ReadProbeData(int nsrcs, int nprobe, int kx, int ky, Mem2d *m, double scale){
	if (prb_data_mode == PRB_SHORT){
		size_t data_set_size =  nprobe * nsrcs * sizeof(SHT);
		if (data_set_size >= (DSP_DATA_REG_LEN<<2))
			return 0;
		SHT linebuffer[DSP_DATA_REG_LEN<<1];
		ReadData(linebuffer, data_set_size);

		if (kx == -1 && ky == -1)
			for (int i=0; i < nsrcs; ++i)
				for (int k=0; k < nprobe; ++k)
					m->PutDataPkt (((double)linebuffer[k+i*nprobe]) * scale, 
						       k, i);
		else
			for (int k=0; k < nprobe; ++k)
				m->PutDataPkt (((double)linebuffer[k]) * scale, 
					       kx, ky, k);
		return 1;

	} else if (prb_data_mode == PRB_FLOAT){
		size_t data_set_size =  nprobe * nsrcs * sizeof(float);
		if (data_set_size >= (DSP_DATA_REG_LEN<<2))
			return 0;
		float linebuffer[DSP_DATA_REG_LEN<<2];
		ReadData(linebuffer, data_set_size);

		if (kx == -1 && ky == -1)
			for (int i=0; i < nsrcs; ++i)
				for (int k=0; k < nprobe; ++k)
					m->PutDataPkt (((double)linebuffer[k+i*nprobe]) * scale, 
						       k, i);
		else
			for (int k=0; k < nprobe; ++k)
				m->PutDataPkt (((double)linebuffer[k]) * scale, 
					       kx, ky, k);
		return 1;
	} else return 0;
}

/* nur für DSP */

/* ENDE MODUL DSPCOM */
