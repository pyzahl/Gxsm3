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
#include "plug-ins/hard/modules/dsp.h"

#include "dsp-pci32/xsm/xsmcmd.h"

#include "comedi_hwi.h"

// enable debug:
#define	COMEDI_DEBUG(S) XSM_DEBUG (DBG_L4, "comedi_hwi_spm: " << S )

/* 
 * Init things
 */

comedi_hwi_spm::comedi_hwi_spm():comedi_hwi_dev(){
	COMEDI_DEBUG("Init Comedi SPM");
	ScanningFlg=0;	
}

/*
 * Clean up
 */

comedi_hwi_spm::~comedi_hwi_spm(){
	COMEDI_DEBUG("Finish Comedi SPM");
}

void comedi_hwi_spm::PutParameter(void *src, int grp){ // Parameterstruktur kopieren
	static double V=1;
	if(src)
		memcpy(&dspPar, src, sizeof(DSP_Param));
	
	/* handle this dspPar in some way:
	   dspPar.UTunnel       <- I guess you need only to set U (you may say V) here!
	   dspPar.ITunnelSoll
	   dspPar.CP
	   dspPar.CI
	   dspPar.SetPoint
	*/
	COMEDI_DEBUG("PutParameter: (only if changed!)");

	if (V != dspPar.UTunnel){
		COMEDI_DEBUG("  new V=" << V);
	}
}

// just note that we are scanning next...
void comedi_hwi_spm::StartScan2D(){
	ScanningFlg=1; 
	KillFlg=FALSE; // if this gets TRUE while scanning, you should stopp it!!
}

// and its done now!
void comedi_hwi_spm::EndScan2D(){ 
	ScanningFlg=0; 
}

// we are paused
void comedi_hwi_spm::PauseScan2D(){
	ScanningFlg=0;
}

// and its going againg
void comedi_hwi_spm::ResumeScan2D(){
	ScanningFlg=1;
}

// X position tip, relative to Offset,
// but in rotated cooerd sys!
// Note: X is always the fast scanning direction!!
void comedi_hwi_spm::MovetoXY(long x, long y){
	// this is how I pass params to the DSP
	// you could do sth. else here!
// use the rot matrix (rotmxx/xy/yx/yy/rotoffx/rotoffy) !!
/*
	PARAMETER_SET hardpar;
	rx=x;
	hardpar.N   = DSP_MOVETOX+1;
	hardpar.Cmd = DSP_CMD_MOVETO_X;
	hardpar.hp[DSP_MOVETOX ].value = x;
	SetParameter(hardpar);
*/
	COMEDI_DEBUG("MovetoX: " << x); rx=x;

// use the rot matrix (rotmxx/xy/yx/yy/rotoffx/rotoffy) !!
/*
	PARAMETER_SET hardpar;
	ry=y;
	hardpar.N   = DSP_MOVETOY+1;
	hardpar.Cmd = DSP_CMD_MOVETO_Y;
	hardpar.hp[DSP_MOVETOY ].value = y;
	SetParameter(hardpar);
*/
	COMEDI_DEBUG("MovetoY: " << y); ry=y;
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void comedi_hwi_spm::SetDxDy(int dx, int dy){
	Dx = dx; 
	Dy = dy;
	dspPar.LS_dnx = dx;
	COMEDI_DEBUG("SetDxSy: " << dx << ", " << dy);
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void comedi_hwi_spm::SetOffset(long x, long y){
	rotoffx = x; rotoffy = y;
	COMEDI_DEBUG("SetOffset: " << x << ", " << y);
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void comedi_hwi_spm::SetAlpha(double alpha){ 
	// calculate rot matrix
	Alpha=M_PI*alpha/180.;
	rotmyy = rotmxx = cos(Alpha);
	rotmyx = -(rotmxy = sin(Alpha));
	COMEDI_DEBUG("SetAlpha: " << alpha);
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void comedi_hwi_spm::SetNx(long nx){ 
	Nx=nx; 
	dspPar.LS_nx2scan = nx;
	COMEDI_DEBUG("Setnx: " << nx);
}


/*
 * Do ScanLine in multiple Channels Mode
 *
 * yindex:  Index of Line
 * xdir:    Scanning Direction (1:+X / -1:-X) eg. for/back scan
 * lsscrs:  LineScan Sources, MUX/Channel Configuration
 * PC31 has can handle 2 Channels of 16bit data simultan, using 4-fold Mux each
 *      additional the PID-Outputvalue can be used and the PID Src is set by MUXA
 *      it may be possible to switch MUXB while scanning -- test it !!!
 * bit     0 1 2 3  4 5 6 7  8 9 10 11  12 13 14 15 ...
 * PC31:   ==PID==  =MUX-A=  ==MUX-B==  
 * PCI32:  ==PID==  A======  B========  C==== D====
 *    eg.  Z-Value  Fz/I     Fric...
 * example:
 * value   1 0 0 0  1 0 0 0  1 0  0  0  0...
 * buffers B0       B1       B2
 * 3 buffers used for transfer (B0:Z, B1:Force, B2:Friction)
 *
 * Mob:     List of MemObjs. to store Data
 *
 * with maximized DPRAM usage: [DPRAM] [DSPMEM1] [DSPMEM2] ... (Blocks of same size)
 *                              B0,B1,  B2, B3,   B4,..
 * nach DSP_CMD_SWAPDPRAM:      B2,B3,  -------   B4,..
 * nach DSP_CMD_SWAPDPRAM:      B4,..   -------   -------
 * ...
 * 
 */

void comedi_hwi_spm::ScanLineM(int yindex, int xdir, int lssrcs, Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0 ){

		if (yindex < 0){ // HS capture (-1) / init (-2) ?
				if (yindex != -1) return; // XP/XM init cycle
				return; // ... or run 2D HS caprture
		}

// this is how I pass parameters to the DSP
// you may use thoes otherwise
/*
	PARAMETER_SET hardpar;

	hardpar.N   = DSP_LSYINDEX+1;
	hardpar.Cmd = DSP_CMD_LINESCAN;
	hardpar.hp[DSP_LSNX       ].value = dspPar.LS_nx2scan;
	if(xdir<0) // Rück - Scan (-)
		hardpar.hp[DSP_LSDNX      ].value = -dspPar.LS_dnx;
	else    // Hin  - Scan (+)
		hardpar.hp[DSP_LSDNX      ].value = dspPar.LS_dnx;
	hardpar.hp[DSP_LSSTEPSZ   ].value = dspPar.LS_stepsize;
	hardpar.hp[DSP_LSNREGEL   ].value = dspPar.LS_nRegel;
	hardpar.hp[DSP_LSNAVE     ].value = dspPar.LS_nAve;
	hardpar.hp[DSP_LSINTAVE   ].value = dspPar.LS_IntAve;
	hardpar.hp[DSP_LSNXPRE    ].value = dspPar.LS_nx_pre;
	hardpar.hp[DSP_LSSRCS     ].value = lssrcs; // Codierte MUX/Srcs Configuration
	hardpar.hp[DSP_LSYINDEX   ].value = yindex; // for verify only

	// waits until done!!! But calls the check event func for non blocking!
	SetParameter(hardpar, TRUE);
*/
	// call this while waiting for background data updates on screen...
	CallIdleFunc ();

	// calc # of srcs
	int num_srcs = 0;
	int bi = 0;
	do{
		if(lssrcs & (1<<bi++))
			++num_srcs;
	}while(bi<16);

	ReadScanData (yindex, num_srcs, Mob);
}
