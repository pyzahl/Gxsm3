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


#include <fcntl.h>
#include <sys/ioctl.h>

#include "glbvars.h"
#include "comedi_hwi.h"

// you may want to handle/emulate some DSP commands later...
#include "dsp-pci32/xsm/dpramdef.h"
#include "dsp-pci32/xsm/xsmcmd.h"

// enable debug:
#define	COMEDI_DEBUG(S) XSM_DEBUG (DBG_L4, "comedi_hwi_dev: " << S )

/* Construktor for Gxsm comedi support base class
 * ==================================================
 * - open device
 * - init things
 * - ...
 */
comedi_hwi_dev::comedi_hwi_dev(){
	COMEDI_DEBUG("open driver");
	max_points_per_line = 400000; // what ever you are willing to handle...
	// here you may want to open you comedi device(s) 
	// and do some basic initialisations if needed

// e.g. open like this, the device name in the Gxsm prefs is "xsmres.DSPDev"
//	comedi_drv = open(xsmres.DSPDev, O_RDWR);
}

/* Destructor
 * close device
 */
comedi_hwi_dev::~comedi_hwi_dev(){
	COMEDI_DEBUG("close driver");
//	close (comedi_drv);
}

void comedi_hwi_dev::ExecCmd(int Cmd){
	// Exec "DSP" command (additional stuff)

	// Put CMD to DSP for execution.
	// Wait until done and
	// call whenever you are waiting longer for sth.:
	// gapp->check_events(); // inbetween!
}

int comedi_hwi_dev::WaitExec(int data){
	return 0; // not needed today, always returns 0	
}

// you may want this later for mover and other tasks...
void comedi_hwi_dev::SetParameter(PARAMETER_SET &hps, int scanflg){
}

void comedi_hwi_dev::GetParameter(PARAMETER_SET &hps){
}

size_t comedi_hwi_dev::ReadData(void *buf, size_t count) { 
	// move data into buf
	// e.g.: return read(dsp_data, buf, count); 
	return count; // return num of data read form source successfully
}

int comedi_hwi_dev::ReadScanData(int y_index, int num_srcs, Mem2d *m[MAX_SRCS_CHANNELS]){
	static time_t t0 = 0; // only for demo
	int len = m[0]->GetNx();
	SHT *linebuffer = new SHT[len*num_srcs];

// create dummy data (need to read some stuff from driver/tmp buffer...)
// should use ReadData (see above) later!
// and should not block...

	COMEDI_DEBUG("ReadData:" << y_index);

	if (t0 == 0) t0 = time (NULL);
	int drift = (int) (time (NULL) - t0);
	if (y_index < 0) // 2D HS Area Capture/Scan
		for (int k=0; k<m[0]->GetNy (); ++k)
			for (int i=0; i<num_srcs; ++i){
				for (int j=0; j<len; ++j){
					double x = rx*Dx + j*Dx + drift*5;
					double y = ry*Dy + k*Dx;
					Transform(&x, &y);
					linebuffer[i*len + j] = (SHT)(1000.*sin(x/100.*2*M_PI)
								      *cos(k/100.*2*M_PI));
				}
				if (m[i])
					m[i]->PutDataLine (k, linebuffer+i*len);
			}
	else
		for (int i=0; i<num_srcs; ++i){
			for (int j=0; j<len; ++j){
				double x = rx*Dx + j*Dx + drift;
				double y = ry*Dy;
				Transform(&x, &y);
				linebuffer[i*len + j] = (SHT)(1000.*sin(x/100.*2*M_PI)
							      *cos(y_index/100.*2*M_PI));
			}
			if (m[i])
				m[i]->PutDataLine (y_index, linebuffer+i*len);
		}

// move data into mem2d objects -- it's done above already...
//	for (int i=0; i<num_srcs; ++i)
//		if (m[i])
//			m[i]->PutDataLine (y_index, linebuffer+i*len);
	
	delete[] linebuffer;
	return 0;
}

int comedi_hwi_dev::ReadProbeData(int nsrcs, int nprobe, int kx, int ky, Mem2d *m, double scale){
	// later
	return 1;
}


/*
 * provide some info about the connected hardware/driver/etc.
 */
gchar* comedi_hwi_dev::get_info(){
	return g_strdup("*--Gxsm Comedi HwI base class--*\n"
			"Comedi device: do not know\n"
			"*--Features--*\n"
			"SCAN: Yes\n"
			"PROBE: No\n"
			"ACPROBE: No\n"
			"*--EOF--*\n"
		); 
}
