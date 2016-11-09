/* Gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Gxsm Hardware Interface Plugin
 * ==============================
 * hacked form Percy Zahls' demo_hwi* files
 * 
 * Author: Marcello Carla' <carla@fi.infn.it>
 * 
 * Copyright (C) 2008 Percy Zahl, Marcello Carla'
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
#include <time.h>


#include "glbvars.h"
#include <fcntl.h>
#include <sys/ioctl.h>

#include "kmdsp_hwi.h"

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


// enable debug:
#define	KMDSP_DEBUG(S) XSM_DEBUG (DBG_L4, S)
#define	KMDSP_ERROR(S) XSM_DEBUG_ERROR (DBG_L4, S)


#define SR_READFIFO_RESET -1
#define SR_EMPTY_PROBE_FIFO -2


extern kmdsp_SPM_Control *kmdsp_SPM_ControlClass;
extern GxsmPlugin kmdsp_hwi_pi;

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



/* Construktor for Gxsm kmdsp support base class
 * ==================================================
 * - open device
 * - init things
 * - ...
 */
kmdsp_hwi_dev::kmdsp_hwi_dev(){
	KMDSP_DEBUG("open driver");
	DAC_max_points = 1<<15; // DAC X/Y resolution is limiting max scan points...

	dsp = 0;

	// open device
	if((dsp = open (xsmres.DSPDev, O_RDWR)) <= 0){

                printf ("KMDSP,,%s,,%s - Failed opening device %s. Make sure the\n", __FILE__, __FUNCTION__, xsmres.DSPDev);
                printf ("KMDSP,,%s,,%s - spm module is loaded and the /dev/spm/scan\n", __FILE__, __FUNCTION__);
                printf ("KMDSP,,%s,,%s - device has proper access rights.\n", __FILE__, __FUNCTION__);
                exit (-1);

	} else {
		// do hardware setup and checks version control, etc....
	}
}

/* Destructor
 * close device
 */
kmdsp_hwi_dev::~kmdsp_hwi_dev(){
	KMDSP_DEBUG ("closing connection to spm driver");
	if (dsp)
		close (dsp);
}

// ==================================================
// obsolete

int kmdsp_hwi_dev::ReadScanData(int y_index, int num_srcs, Mem2d *m[MAX_SRCS_CHANNELS]){
	return 0; // done by thread
}

int kmdsp_hwi_dev::ReadProbeData(int nsrcs, int nprobe, int kx, int ky, Mem2d *m, double scale){
	// this function is obsolete since vector probe
	return 1;
}

/*
 * provide some info about the connected hardware/driver/etc.
 */
gchar* kmdsp_hwi_dev::get_info(){
	return g_strdup("*--GXSM kmdsp HwI class--*\n"
			"Handler for the spm module\n"
			"for kernel space software DSP");
}

double kmdsp_hwi_dev::GetUserParam (gint n, gchar *id){
	return kmdsp_SPM_ControlClass->GetUserParam (n, id);
}

gint kmdsp_hwi_dev::SetUserParam (gint n, gchar *id, double value){
	return kmdsp_SPM_ControlClass->SetUserParam (n, id, value);
}

