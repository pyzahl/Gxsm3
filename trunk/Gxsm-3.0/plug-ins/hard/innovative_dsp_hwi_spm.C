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

#include "dsp-pci32/xsm/xsmcmd.h"
//#include "dsp-pci32/xsm/xsmconst.h"

#include "innovative_dsp_hwi.h"


#define CHECKDIFF(A,B) (fabs((A)-(B)) > 1e-8)

/* ============================================================
 * Hardwareimplementation hilevel DSP:
 * Virtuelle Funktionen der Basisklasse werden überschrieben
 * ============================================================ 
 */

innovative_dsp_hwi_spm::innovative_dsp_hwi_spm():innovative_dsp_hwi_dev(){
 ScanningFlg=0;
}

innovative_dsp_hwi_spm::~innovative_dsp_hwi_spm(){
}

/* Übergeordnete Parameterübergabefunktionen PC => PC31/DSP
 * ========================================================
 * virtual !
 */
void innovative_dsp_hwi_spm::PutParameter(void *src, int grp){ // Parameterstruktur kopieren
	if(src)
		memcpy(&dspPar, src, sizeof(DSP_Param));
	switch(ScanningFlg){
	case 0:
		DSP_FbWerte();
		DSP_SetTransferFkt();
		DSP_SetRotParam();
		DSP_SetMoveParam();
		DSP_SetAppWerte();
		break;
	case 1:
		{
			gchar *mt = g_strdup_printf ("U=%.3fV I=%.4fnA", dspPar.UTunnel, dspPar.ITunnelSoll);
			gapp->monitorcontrol->LogEvent ("DSP parameter changed", mt);
			g_free (mt);
		}
		ScanningFlg=2;
		break;
	case 3:
		DSP_FbWerte();
		DSP_SetMoveParam();
		break;
	}
}

void innovative_dsp_hwi_spm::StartScan2D(){
  ScanningFlg=1; KillFlg=FALSE;
}

void innovative_dsp_hwi_spm::EndScan2D(){ 
  if(ScanningFlg>1){
    ScanningFlg=0; 
    PutParameter(NULL);
  }
  else
    ScanningFlg=0; 
}

void innovative_dsp_hwi_spm::PauseScan2D(){
	// sth to put to DSP?
	if(ScanningFlg>1){
		ScanningFlg=3; 
		PutParameter(NULL);
		ScanningFlg=1;
	}
	ScanningFlg=0;
}
void innovative_dsp_hwi_spm::ResumeScan2D(){
	ScanningFlg=1;
}

void innovative_dsp_hwi_spm::MovetoXY(long x, long y){
	static long sxy=123456789;
	long newsum = x+y;
	if ( newsum != sxy){
		PARAMETER_SET hardpar;
		sxy = newsum;
		rx=x;
		ry=y;
		hardpar.N   = DSP_MOVETOXY_Y+1;
		hardpar.Cmd = DSP_CMD_MOVETO_XY;
		hardpar.hp[DSP_MOVETOXY_X].value = x;
		hardpar.hp[DSP_MOVETOXY_Y].value = y;
		SetParameter (hardpar);
	}
}

void innovative_dsp_hwi_spm::SetDxDy(int dx, int dy){
	Dx = dx; 
	Dy = dy;
	dspPar.LS_dnx = dx;
}
void innovative_dsp_hwi_spm::SetOffset(long x, long y){
	rotoffx = x; rotoffy = y;
	DSP_SetRotParam();
}
void innovative_dsp_hwi_spm::SetAlpha(double alpha){ 
	Alpha=M_PI*alpha/180.;
	rotmyy = rotmxx = cos(Alpha);
	rotmyx = -(rotmxy = sin(Alpha));
	DSP_SetRotParam();
}

void innovative_dsp_hwi_spm::SetNx(long nx){ 
	Nx=nx; 
	dspPar.LS_nx2scan = nx;
}

/* Parameterübergabefunktionen PC => PC31/DSP
 * ==================================================
 *
 */

void innovative_dsp_hwi_spm::DSP_FbWerte(){
  static double sum=1234e100;
  double nowsum;
  nowsum = dspPar.UTunnel+dspPar.ITunnelSoll
    +dspPar.CP+dspPar.CI+dspPar.CS+dspPar.fb_frq
    +dspPar.fir_fg+dspPar.SetPoint;
  if(CHECKDIFF(sum, nowsum)){
    PARAMETER_SET hardpar;
    XSM_DEBUG (DBG_L4, "DSP_FbWerte d:" << (sum - nowsum));
    sum = nowsum;

    hardpar.N   = DSP_CS+1;
    hardpar.Cmd = DSP_CMD_SET_WERTE;
    hardpar.hp[DSP_UTUNNEL].value = dspPar.UTunnel;
    hardpar.hp[DSP_ITUNNEL].value = gapp->xsm->Inst->nAmpere2V(dspPar.ITunnelSoll);
    hardpar.hp[DSP_CP     ].value = dspPar.CP;
    hardpar.hp[DSP_CI     ].value = dspPar.CI;
    hardpar.hp[DSP_CD     ].value = gapp->xsm->Inst->nNewton2V(dspPar.SetPoint);
    hardpar.hp[DSP_FB_FRQ ].value = dspPar.fb_frq;
    hardpar.hp[DSP_FIR_FG ].value = dspPar.fir_fg;
    hardpar.hp[DSP_CS     ].value = dspPar.CS;
    SetParameter(hardpar);
    SetParameter(hardpar); // some trouble with pc31, doing twice helps.. :=( !!
  }
}

void innovative_dsp_hwi_spm::DSP_SetTransferFkt(){
  static double sum=1234e100;
  double nowsum;
  nowsum = dspPar.LogOffset+dspPar.LogSkl+dspPar.LinLog;
  if(CHECKDIFF(sum ,nowsum)){
    PARAMETER_SET hardpar;
    XSM_DEBUG (DBG_L4, "DSP_SetTransferFkt");
    sum = nowsum;
    hardpar.N   = DSP_LIN_LOG+1;
    hardpar.Cmd = DSP_CMD_SET_TRANSFER_FKT;
    hardpar.hp[DSP_UTUNNEL].value = dspPar.UTunnel;
    hardpar.hp[DSP_LOGOFF ].value = dspPar.LogOffset;
    hardpar.hp[DSP_LOGSKL ].value = dspPar.LogSkl;
    hardpar.hp[DSP_LIN_LOG].value = dspPar.LinLog;
    SetParameter(hardpar);
  }
}

void innovative_dsp_hwi_spm::DSP_SetRotParam(){
  static double sum=1234e100;
  double nowsum;
  nowsum = Alpha+rotoffx+rotoffy;
  if(CHECKDIFF(sum ,nowsum)){
    PARAMETER_SET hardpar;
    XSM_DEBUG (DBG_L4, "DSP_SetRotParam d:" << (sum - nowsum));
    sum = nowsum;
    hardpar.N   = DSP_ROTOFFY+1;
    hardpar.Cmd = DSP_CMD_ROTPARAM;
    hardpar.hp[DSP_ROTXX  ].value = rotmxx;
    hardpar.hp[DSP_ROTXY  ].value = rotmxy;
    hardpar.hp[DSP_ROTYX  ].value = rotmyx;
    hardpar.hp[DSP_ROTYY  ].value = rotmyy;
    hardpar.hp[DSP_ROTOFFX].value = rotoffx;
    hardpar.hp[DSP_ROTOFFY].value = rotoffy;
    SetParameter(hardpar);
  }
}

void innovative_dsp_hwi_spm::DSP_SetMoveParam(){
  static double sum=1234e100;
  double nowsum;
  nowsum = dspPar.MV_stepsize+dspPar.MV_nRegel;
  if(CHECKDIFF(sum ,nowsum)){
    PARAMETER_SET hardpar;
    XSM_DEBUG (DBG_L4, "DSP_SetMoveParam");
    sum = nowsum;
    hardpar.N   = DSP_MVNREGEL+1;
    hardpar.Cmd = DSP_CMD_MOVETO_PARAM;
    hardpar.hp[DSP_MVSTEPSZ].value = dspPar.MV_stepsize;
    hardpar.hp[DSP_MVNREGEL].value = dspPar.MV_nRegel;
    SetParameter(hardpar);
  }
}

void innovative_dsp_hwi_spm::DSP_SetAppWerte(){
  static double sum=1234e100, sum2=1234e100;
  double nowsum, nowsum2;
  nowsum = dspPar.AFM_Amp+dspPar.AFM_Speed+dspPar.AFM_Steps;
  if(CHECKDIFF(sum, nowsum)){
    PARAMETER_SET hardpar;
    XSM_DEBUG (DBG_L4, "DSP_SetAppWerte AMP-SPEED-STEPS d:" << (sum - nowsum));
    sum = nowsum;
    XSM_DEBUG (DBG_L4, " Amp:" << dspPar.AFM_Amp << " Spd: " << dspPar.AFM_Speed << "#:" << dspPar.AFM_Steps);
    hardpar.N   = DSP_AFM_SLIDER_STEPS+1;
    hardpar.Cmd = DSP_CMD_AFM_SLIDER_PARAM;
    hardpar.hp[DSP_AFM_SLIDER_AMP  ].value = dspPar.AFM_Amp;
    hardpar.hp[DSP_AFM_SLIDER_SPEED].value = dspPar.AFM_Speed;
    hardpar.hp[DSP_AFM_SLIDER_STEPS].value = dspPar.AFM_Steps;
    SetParameter(hardpar);
  }
  nowsum2 = dspPar.TIP_nSteps+dspPar.TIP_Delay+dspPar.TIP_DUz+dspPar.TIP_DUzRev;
  if(CHECKDIFF(sum2, nowsum2)){
    PARAMETER_SET hardpar;
    XSM_DEBUG (DBG_L4, "DSP_SetAppWerte TIPAPPPARAM");
    sum2 = nowsum2;
    XSM_DEBUG (DBG_L4, " nSteps:" << dspPar.TIP_nSteps);
    hardpar.N   = DSP_TIPDUZREV+1;
    hardpar.Cmd = DSP_CMD_APPROCH_PARAM;
    hardpar.hp[DSP_TIPNSTEPS ].value = dspPar.TIP_nSteps;
    hardpar.hp[DSP_TIPNWARTE ].value = dspPar.TIP_Delay;
    hardpar.hp[DSP_TIPDUZ    ].value = dspPar.TIP_DUz;
    hardpar.hp[DSP_TIPDUZREV ].value = dspPar.TIP_DUzRev;
    SetParameter(hardpar);
  }
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
 * if yindex == -1, then a HS Capture (DSP_CMD_2D_HS_AREASCAN) is requested!
 */
void innovative_dsp_hwi_spm::ScanLineM(int yindex, int xdir, int lssrcs, Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0 ){
  PARAMETER_SET hardpar;

  if(ScanningFlg>1){
    ScanningFlg=3; 
    PutParameter(NULL);
    ScanningFlg=1;
  }

  if (yindex >= 0){
	  hardpar.N   = DSP_LSYINDEX+1;
	  hardpar.Cmd = DSP_CMD_LINESCAN;
  } else {
	  if (yindex != -1) return;
	  hardpar.N   = DSP_AS_DNY+1;
	  hardpar.Cmd = DSP_CMD_2D_HS_AREASCAN;
	  hardpar.hp[DSP_AS_NY2SCAN ].value = Mob[0]->GetNy();
	  hardpar.hp[DSP_AS_DNY     ].value = dspPar.LS_dnx; // only same step size!!!!!
  }
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
  SetParameter(hardpar, TRUE);

  // calc # of srcs
  int num_srcs = 0;
  int bi = 0;
  do{
	  if(lssrcs & (1<<bi++))
		  ++num_srcs;
  }while(bi<16);

  ReadScanData (yindex, num_srcs, Mob);

  if(ScanningFlg>1){
    ScanningFlg=3; 
    PutParameter(NULL);
    ScanningFlg=1;
  }
}
