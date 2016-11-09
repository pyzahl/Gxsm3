/* Gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Copyright (C) 1999,2000,2001 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * additional features: Farid El Gabaly <farid.elgabaly@uam.es>, Juan de la Figuera <juan.delafiguera@uam.es>
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


// "C++" headers
#include <iostream>

// system headers
#include <fcntl.h>
#include <sys/ioctl.h>

#include <locale.h>
#include <libintl.h>


// Gxsm headers
#include "glbvars.h"
#include "dsp-pci32/xsm/xsmcmd.h"
#include "LAN_rhk_hwi.h"


// use "gxsm3 --debug-level=5" (5 or higher) to enable debug!
#define	RHK_DEBUG(S) XSM_DEBUG (DBG_L4, S)

/*
 * Init things
 */

LAN_rhk_hwi_spm::LAN_rhk_hwi_spm():LAN_rhk_hwi_dev(){
	RHK_DEBUG("Init LAN-RHK SPM");
	ScanningFlg=0;
}

/*
 * Clean up
 */

LAN_rhk_hwi_spm::~LAN_rhk_hwi_spm(){
	RHK_DEBUG("Finish LAN-RHK SPM");
}

void LAN_rhk_hwi_spm::ExecCmd(int Cmd){

	char txtcmd[80];
	// Exec "DSP" command (additional stuff)
	RHK_DEBUG("Exec Cmd 0x" << std::hex << Cmd);
	if (Cmd==DSP_CMD_APPROCH)
		{
			sprintf(txtcmd, "check %6.0d\n",ScanData.hardpars.TIP_DUz);
			SendCommand(txtcmd);
		}
	if (Cmd==DSP_CMD_APPROCH_MOV_XP)
		{
			sprintf(txtcmd, "approach %6.0f on %6.0f %6.0f\n",ScanData.hardpars.MOV_Steps,
				3276.8*ScanData.hardpars.MOV_Ampl, ScanData.hardpars.MOV_Speed);
			SendCommand(txtcmd);
		}
	else if (Cmd==DSP_CMD_CLR_PA)
		{
			SendCommand("stop\n");
		}
	else if (Cmd==DSP_CMD_AFM_MOV_YP)
		{
			sprintf(txtcmd, "approach %6.0f off %6.0f %6.0f\n",ScanData.hardpars.MOV_Steps,
				3276.8*ScanData.hardpars.MOV_Ampl, ScanData.hardpars.MOV_Speed);
			SendCommand(txtcmd);
		}
	else if (Cmd==DSP_CMD_AFM_MOV_YM)
		{
			sprintf(txtcmd, "retract %6.0f  %6.0f %6.0f\n",ScanData.hardpars.MOV_Steps, 3276.8*ScanData.hardpars.MOV_Ampl, ScanData.hardpars.MOV_Speed);
			SendCommand(txtcmd);
		}
	else if (Cmd==DSP_CMD_MOVETO_X)
		{
			SendCommand("xy on\n");
		}
	else if (Cmd==DSP_CMD_MOVETO_Y)
		{
			SendCommand("xy off\n");
		}

	// Put CMD to DSP for execution.
	// Wait until done and
	// call whenever you are waiting longer for sth.:
	// gapp->check_events(); // inbetween!
}


void LAN_rhk_hwi_spm::PutParameter(void *par, int grp)
{
	/* Read both scan parameters and spm settings (RHK STM 100 has its own scan generator, so we
	   only need to READ the settings, we cannot set them. To be used with new RHKControl plugin... */

	/* Use PutParameter(dsp) to transfer dsp parameter to this plugin (for DSPmover control) */
	/* Use PutParameter(scandata,1) to synchronize the settings as read from the RHK */

	char buffer[18], d0,d1;
	short *reading;
	int ad_gain, mult;
	float line_rate;
	int temp, count;

		if (ScanningFlg) {RHK_DEBUG("Tried to get params while scanning!");
		return;
		}
	if (grp!=1) {
		if (par)
		memcpy(&(ScanData.hardpars),par, sizeof(ScanData.hardpars));
		RHK_DEBUG("Settings " << ScanData.hardpars.AFM_Steps << ScanData.hardpars.MOV_Steps);
		return;
	}
	if (par)
		memcpy(&ScanData,par, sizeof(ScanData));
	RHK_DEBUG("Settings " << ScanData.hardpars.AFM_Steps << ScanData.hardpars.MOV_Steps);
	SendCommand("read_settings\n");
	ReadData(buffer,sizeof(buffer));
	reading=(short *)buffer;
	d0=buffer[16];
	d1=buffer[17];
	ScanData.hardpars.UTunnel=reading[2];
	ScanData.hardpars.ITunnelSoll=reading[1];

	ScanData.s.rx=reading[6];
	ScanData.s.ry=reading[4];
	//scanPar.rz=reading[4];
	ScanData.s.tStart;
	ScanData.s.tEnd;
	ScanData.s.x0=reading[7];
	ScanData.s.y0=reading[5];

	line_rate = d0 & 0x0f;
	temp = 2 << (((d0 & 0x70) >> 4) - 1);
	ad_gain = (temp == 0) ? 1 : temp;
	temp = d1 & 3;
	mult = (int)pow (10.0, temp);
	ScanData.s.ny= 128 << ((d1 & 12) >> 2);
	ScanData.s.nx= ScanData.s.ny;
	line_rate *= mult;
	ScanData.s.rz=32768/ad_gain;
	RHK_DEBUG("Updating parameters xUT=" );

	if (par)
		memcpy(par,&ScanData,sizeof(ScanData));


}

// just note that we are scanning next...
void LAN_rhk_hwi_spm::StartScan2D(){
	ScanningFlg=1;
	KillFlg=FALSE; // if this gets TRUE while scanning, you should stopp it!!
	//	SendCommand("start\n");
	RHK_DEBUG("Start Scan 2D");
}

// and its done now!
void LAN_rhk_hwi_spm::EndScan2D(){
	ScanningFlg=0;
	SendCommand("reset\n");
	RHK_DEBUG("End Scan 2D");
}

// we are paused
void LAN_rhk_hwi_spm::PauseScan2D(){
	ScanningFlg=0;
	SendCommand("stop\n");
	RHK_DEBUG("Pause Scan 2D");
}

// and its going againg
void LAN_rhk_hwi_spm::ResumeScan2D(){
	SendCommand("start\n");
	RHK_DEBUG("Resume Scan 2D");
	ScanningFlg=1;
}

// X position tip, relative to Offset,
// but in rotated cooerd sys!
// Note: X is always the fast scanning direction!!
void LAN_rhk_hwi_spm::MovetoX(long x){
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
	//RHK_DEBUG("MovetoX: " << x); rx=x;
}

// Y position tip, relative to Offset,
// but in rotated cooerd sys!
void LAN_rhk_hwi_spm::MovetoY(long y){
	// use the rot matrix (rotmxx/xy/yx/yy/rotoffx/rotoffy) !!
	/*
	  PARAMETER_SET hardpar;
	  ry=y;
	  hardpar.N   = DSP_MOVETOY+1;
	  hardpar.Cmd = DSP_CMD_MOVETO_Y;
	  hardpar.hp[DSP_MOVETOY ].value = y;
	  SetParameter(hardpar);
	*/
	//RHK_DEBUG("MovetoY: " << y); ry=y;
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void LAN_rhk_hwi_spm::SetDxDy(int dx, int dy){
	Dx = dx; 
	Dy = dy;
	//dspPar.LS_dnx = dx;
	RHK_DEBUG("SetDxSy: " << dx << ", " << dy);
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void LAN_rhk_hwi_spm::SetOffset(long x, long y){
	rotoffx = x; rotoffy = y;
	RHK_DEBUG("SetOffset: " << x << ", " << y);
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void LAN_rhk_hwi_spm::SetAlpha(double alpha){ 
	// calculate rot matrix
	Alpha=M_PI*alpha/180.;
	rotmyy = rotmxx = cos(Alpha);
	rotmyx = -(rotmxy = sin(Alpha));
	RHK_DEBUG("SetAlpha: " << alpha);
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void LAN_rhk_hwi_spm::SetNx(long nx){ 
	Nx=nx; 
	//dspPar.LS_nx2scan = nx;
	RHK_DEBUG("Setnx: " << nx);
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

void LAN_rhk_hwi_spm::ScanLineM(int yindex, int xdir, int lssrcs, Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0 ){
	char txt[80];
	int count;
	int channel; // Only one for now...

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
	if (yindex<1) {
		if (lssrcs & 0x01)  channel=0;
		else if (lssrcs & 0x02) channel=1;
		else if (lssrcs & 0x04) channel=2;
		else if (lssrcs & 0x08) channel=3;
		else if (lssrcs & 0x08) channel=3;
		else if (lssrcs & 0x10) channel=4;
		else if (lssrcs & 0x20) channel=5;
		else if (lssrcs & 0x40) channel=6;
		else if (lssrcs & 0x80) channel=7;

		RHK_DEBUG("Selecting " << lssrcs <<  " and so, channel " << channel );
		count=sprintf(txt, "channel %d\n\0",channel);
		SendCommand(txt);
		SendCommand("start\n");
	}
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
