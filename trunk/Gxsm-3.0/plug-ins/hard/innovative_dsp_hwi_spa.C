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

#include "dsp-pci32/spa/spacmd.h"
#include "innovative_dsp_hwi.h"

/* ============================================================
 * Hardwareimplementation hilevel DSP:
 * Virtuelle Funktionen der Basisklasse werden überschrieben
 * ============================================================ 
 */

innovative_dsp_hwi_spa::innovative_dsp_hwi_spa():innovative_dsp_hwi_dev(){
  ScanningFlg=0;
}

innovative_dsp_hwi_spa::~innovative_dsp_hwi_spa(){
}

/* Übergeordnete Parameterübergabefunktionen PC => PC31/DSP
 * ========================================================
 * virtual !
 */
void innovative_dsp_hwi_spa::PutParameter(void *src, int grp){ // Parameterstruktur kopieren
  if(src)
    memcpy(&dspPar, src, sizeof(DSP_Param));
  switch(ScanningFlg){
  case 0:
    DSP_SpaWerte();
    break;
  case 1:
    ScanningFlg=2;
    break;
  case 3:
    DSP_SpaWerte();
    break;
  }
}

void innovative_dsp_hwi_spa::StoreParameter(){
}

void innovative_dsp_hwi_spa::RestoreParameter(){
  DSP_SpaWerte(TRUE);
}


void innovative_dsp_hwi_spa::StartScan2D(){
  ScanningFlg=1; KillFlg=FALSE;
}

void innovative_dsp_hwi_spa::EndScan2D(){ 
  if(ScanningFlg>1){
    ScanningFlg=0; 
    PutParameter(NULL);
  }
  else
    ScanningFlg=0; 
}

void innovative_dsp_hwi_spa::MovetoXY(long x, long y){
  rx=x;
  ry=y;
}
void innovative_dsp_hwi_spa::SetDxDy(int dx, int dy){
  Dx = dx; 
  Dy = dy;
  dspPar.LS_dnx = dx;
}
void innovative_dsp_hwi_spa::SetOffset(long x, long y){
  rotoffx = x; rotoffy = y;
  DSP_SpaWerte();
}
void innovative_dsp_hwi_spa::SetAlpha(double alpha){ 
  Alpha=M_PI*alpha/180.;
  rotmyy = rotmxx = cos(Alpha);
  rotmyx = -(rotmxy = sin(Alpha));
  DSP_SpaWerte();
}

void innovative_dsp_hwi_spa::SetNx(long nx){ 
  Nx=nx; 
  dspPar.LS_nx2scan = nx;
}

/* Parameterübergabefunktionen PC => PC31/DSP
 * ==================================================
 *
 */

void innovative_dsp_hwi_spa::DSP_SpaWerte(int flg){
  static double sum=1234e100;
  double nowsum;
  nowsum = rotoffx+rotoffx+dspPar.SPA_Length+dspPar.LS_nx2scan+Alpha+dspPar.SPA_Gatetime+dspPar.SPA_EnergyVolt;
  if(sum != nowsum || flg){
    PARAMETER_SET hardpar;
    sum = nowsum;
    XSM_DEBUG (DBG_L4, "DSP_SpaWerte");

    hardpar.N   = DSP_MYY+1;
    hardpar.Cmd = DSP_CMD_SCAN_PARAM;
    hardpar.hp[DSP_X0   ].value = rotoffx;
    hardpar.hp[DSP_Y0   ].value = rotoffy;
    hardpar.hp[DSP_len  ].value = dspPar.SPA_Length;
    hardpar.hp[DSP_N    ].value = dspPar.LS_nx2scan;
    hardpar.hp[DSP_alpha].value = Alpha;
    hardpar.hp[DSP_ms   ].value = 1000.*dspPar.SPA_Gatetime;
    hardpar.hp[DSP_E    ].value = dspPar.SPA_EnergyVolt;
    hardpar.hp[DSP_MXX  ].value = cos (Alpha);
    hardpar.hp[DSP_MXY  ].value = sin (Alpha);
    hardpar.hp[DSP_MYX  ].value = -sin (Alpha);
    hardpar.hp[DSP_MYY  ].value = cos (Alpha);
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
 */
void innovative_dsp_hwi_spa::ScanLineM(int yindex, int xdir, int lssrcs, Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0){
	if (yindex < 0) return;

	int i, bi, bi2, bin;
	size_t elem_size=sizeof(long);
	size_t bsz;
	size_t sz;
	static long linebuffer[DSP_DATA_REG_LEN]; // Max Size for LineData "at once"
	PARAMETER_SET hardpar;

	hardpar.N   = DSP_E+1;
	hardpar.Cmd = DSP_CMD_SCAN_START;
	hardpar.hp[DSP_Y0   ].value = ry;
	hardpar.hp[DSP_len  ].value = dspPar.SPA_Length; // !!! LS_dnx*dspPar.LS_nx2scan;
	hardpar.hp[DSP_E    ].value = dspPar.SPA_EnergyVolt;
	SetParameter(hardpar, TRUE);

	// Setup MUX, #-Buffers
	// 

	/* Daten übertragen */
	bsz=(dspPar.LS_nx2scan*elem_size);
	bi=i=0;
	do{
		bin=sz=0;
		do{
			if(lssrcs & (1<<bi++)){
				sz += bsz; ++bin;
			}
			if((sz+bsz) > (DSP_DATA_REG_LEN<<2)) 
				break;
		}while(bi<16);
		if(bin){
			// read data buffer(s)
			ReadData(linebuffer, sz);
			bi2=0;
			do{
				//	XSM_DEBUG(DBG_L2, "PDL:" << i << " " << bi2 << " " << bsz );
				Mob[i++]->PutDataLine(yindex, linebuffer+(bi2++)*dspPar.LS_nx2scan, scanmode);
			}while(--bin && (i < MAX_SRCS_CHANNELS) ? Mob[i]!=NULL : FALSE);
		}
		else 
			break;
		// Swap DPRAM Buffers...
		ExecCmd(DSP_CMD_SWAPDPRAM);
	}while((i < MAX_SRCS_CHANNELS) ? Mob[i]!=NULL : FALSE);
}
