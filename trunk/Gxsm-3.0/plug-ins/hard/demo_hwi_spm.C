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

#include "demo_hwi.h"

#include "dsp-pci32/xsm/xsmcmd.h"

// important notice:
// ----------------------------------------------------------------------
// all numbers need to be byte swaped on i386 (no change on ppc!!) via 
// int_2_demo_int() or long_2_demo_long()
// before use from DSP and before writing back
// ----------------------------------------------------------------------

// enable debug:
#define	DEMO_DEBUG(S) XSM_DEBUG (DBG_L4, "demo_hwi_spm: " << S )

extern GxsmPlugin demo_hwi_pi;
extern Demo_SPM_Control *Demo_SPM_ControlClass;

/* 
 * Init things
 */

demo_hwi_spm::demo_hwi_spm():demo_hwi_dev(){
	DEMO_DEBUG("Init Demo SPM");
	ScanningFlg=0;	

	x_scan = y_scan = z_scan = 0.;
	x_offset = y_offset = z_offset = 0.;
	bias = 2.0;  current = 0.1; setpoint=0.1; scan_speed = 1000.;
}	

/*
 * Clean up
 */

demo_hwi_spm::~demo_hwi_spm(){
	DEMO_DEBUG("Finish Demo SPM");
}

gint demo_hwi_spm::RTQuery (gchar *property, double &val1, double &val2, double &val3){

	val1 =  gapp->xsm->Inst->VZ() * gapp->xsm->Inst->Dig2VoltOut(z_scan);
	val2 =  gapp->xsm->Inst->VX() * gapp->xsm->Inst->Dig2VoltOut(x_scan);
	val3 =  gapp->xsm->Inst->VY() * gapp->xsm->Inst->Dig2VoltOut(y_scan);

	if (gapp->xsm->Inst->OffsetMode () == OFM_ANALOG_OFFSET_ADDING){
		val1 +=  gapp->xsm->Inst->VZ0() * gapp->xsm->Inst->Dig2VoltOut(z_offset);
		val2 +=  gapp->xsm->Inst->VX0() * gapp->xsm->Inst->Dig2VoltOut(x_offset);
		val3 +=  gapp->xsm->Inst->VY0() * gapp->xsm->Inst->Dig2VoltOut(y_offset);
	}

	return TRUE;
}

void demo_hwi_spm::ExecCmd(int Cmd){
	// ----------------------------------------------------------------------
	// all numbers need to be byte swaped on i386 (no change on ppc!!) via 
	// int_2_demo_int() or long_2_demo_long()
	// before use from DSP and before writing back
	// ----------------------------------------------------------------------

	switch (Cmd){
	case DSP_CMD_START: // enable Feedback
	{
		static SPM_STATEMACHINE dsp_state;
		if (IS_AFM_CTRL){ // AFM
			// ensure use of linear feedback mode
			;
		} else { // STM, ...
			// ensure use of logarithm feedback mode
			;
		}
		// set mode, enable feedback now
		;
		break;
	}
	case DSP_CMD_HALT: // disable Feedback
	{
		// stop/halt feedback now
		break;
	}
	case DSP_CMD_APPROCH_MOV_XP: // auto approch "Mover"
	{
		// mover/coarse motion requests...
		;
		break;
	}
	case DSP_CMD_APPROCH: // auto approch "Slider"
		// start auto app.
		;
		break;
	case DSP_CMD_CLR_PA: // Stop all
	{
		// stopp all coarse motions now - emergency stop!!
		;
		break;
	}
	case DSP_CMD_AFM_MOV_XM: // manual move X-
	case DSP_CMD_AFM_MOV_XP: // manual move X+
	case DSP_CMD_AFM_MOV_YM: // manual move Y-
	case DSP_CMD_AFM_MOV_YP: // manual move Y+
	{
		// X/Y coarse motions...
		break;
	}

//		case DSP_CMD_:
//				break;
	default: break;
	}
}

void demo_hwi_spm::reset_scandata_fifo(int stall){
	// recover fifo...
}

void demo_hwi_spm::tip_to_origin(double x, double y){
	static AREA_SCAN dsp_scan;

	// get current position
	;

	reset_scandata_fifo ();

	// move tip from current position to Origin i.e. x,y
	;
	x_scan = x; y_scan = y;
}

// just note that we are scanning next...
void demo_hwi_spm::StartScan2D(){
	Demo_SPM_ControlClass->StartScanPreCheck ();
	ScanningFlg=1; 
	KillFlg=FALSE; // if this gets TRUE while scanning, you should stopp it!!
}

// and its done now!
void demo_hwi_spm::EndScan2D(){ 
	static AREA_SCAN dsp_scan;
	ScanningFlg=0; 
	// cancel scanning?
	;
	// wait until ready
	;
	// do return to center?
	if (Demo_SPM_ControlClass->center_return_flag)
		tip_to_origin ();
}

// we are paused
void demo_hwi_spm::PauseScan2D(){
	ScanningFlg=0;
}

// and its going againg
void demo_hwi_spm::ResumeScan2D(){
	ScanningFlg=1;
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void demo_hwi_spm::SetOffset(double x, double y){
	static double old_x=123456789., old_y=123456789.;
	static MOVE_OFFSET dsp_move;
	double dx,dy,steps;
	const double fract = 1<<16;

	if (old_x == x && old_y == y) return;

	DEMO_DEBUG("SetOffset: " << x << ", " << y);
	old_x = x; old_y = y;
	
	x_offset = x; y_offset = y;	
	rotoffx = x; rotoffy = y;

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
 * SR:     
 *    eg.  Z-Value  Fz/I     Fric...
 * example:
 * value   1 0 0 0  1 0 0 0  1 0  0  0  0...
 * buffers B0       B1       B2
 * 3 buffers used for transfer (B0:Z, B1:Force, B2:Friction)
 *
 * Mob:     List of MemObjs. to store Data
 *
 * 
 */

void demo_hwi_spm::ScanLineM(int yindex, int xdir, int lssrcs, Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0 ){
	static Mem2d **Mob_dir[4];
	static long srcs_dir[4];
	static int nsrcs_dir[4];
	static int ydir=0;
	static int running = FALSE;
	static AREA_SCAN dsp_scan;
	
	int num_srcs_w = 0; // #words
	int num_srcs_l = 0; // #long words
	int bi = 0;
	// find # of srcs_w (16 bit data) 0x0001, 0x0010..0x0800 (Bits 0, 4,5,6,7, 8,9,10,11)
	do{
		if(lssrcs & (1<<bi++))
			++num_srcs_w;
	}while(bi<12);
	// find # of srcs_l (32 bit data) 0x1000..0x8000 (Bits 12,13,14,15)
	do{
		if(lssrcs & (1<<bi++))
			++num_srcs_l;
	}while(bi<16);
	
	int num_srcs = (num_srcs_l<<4) | num_srcs_w;
	
	// ix0 not used yet, not subscan
	if (yindex == -2 && xdir == 1){ // first init step of XP (->)
		// cancel any running scan now
		;
		
		// and start fifo read thread, it releases the
		// scan-lock (dspfifo.stall flag) and it terminates if it is
		// done or scan stop is requested!
		for (int i=0; i<4; ++i){
			srcs_dir[i] = nsrcs_dir[i] = 0;
			Mob_dir[i] = NULL;
		}
		srcs_dir[0]  = lssrcs;
		nsrcs_dir[0] = num_srcs;
		Mob_dir[0]   = Mob;
		running= FALSE;
		return;
	}
	if (yindex == -2 && xdir == -1){ // second init step of XM (<-)
		srcs_dir[1]  = lssrcs;
		nsrcs_dir[1] = num_srcs;
		Mob_dir[1]   = Mob;
		return;
	}
	if (yindex == -2 && xdir == 2){ // ... init step of 2ND_XP (2>)
		srcs_dir[2]  = lssrcs;
		nsrcs_dir[2] = num_srcs;
		Mob_dir[2]   = Mob;
		return;
	}
	if (yindex == -2 && xdir == -2){ // ... init step of 2ND_XM (<2)
		srcs_dir[3]  = lssrcs;
		nsrcs_dir[3] = num_srcs;
		Mob_dir[3]   = Mob;
		return;
	}

	if (! running && yindex >= 0){ // now do final scan setup and send scan setup, start reading data fifo
		running = TRUE;
		
		// get current params and position
		// Note: current Xpos/Ypos should be 0/0, so we can change the rot.matrix now!
		// ------> Demo::EndScan2D always returns to 0/0 <------
		;
		
		// check for new rotation, set if needed
		;
		
		// setup scan
		;

#if 0		
		// 1st XP scan here
		//   enable do probe at XP?
		if (Demo_SPM_ControlClass->Source && Demo_SPM_ControlClass->probe_trigger_raster_points > 0)
			dsp_scan.srcs_xp  = long_2_demo_long (srcs_dir[0] | 0x0008);
		else
			dsp_scan.srcs_xp  = long_2_demo_long (srcs_dir[0]);
		
		dsp_scan.srcs_xm  = long_2_demo_long (srcs_dir[1]);
		
		// 2nd scan, setup later, see below
		dsp_scan.srcs_2nd_xp  = long_2_demo_long (srcs_dir[2]);
		dsp_scan.srcs_2nd_xm  = long_2_demo_long (srcs_dir[3]);

		// --- DISABLED ---
		dsp_scan.Zoff_2nd_xp  = int_2_demo_int (0); // init to zero, set later
		dsp_scan.Zoff_2nd_xm  = int_2_demo_int (0); // init to zero, set later

		// enable probe?
		if (Demo_SPM_ControlClass->Source && Demo_SPM_ControlClass->probe_trigger_raster_points){
			if (Demo_SPM_ControlClass->Source && Demo_SPM_ControlClass->probe_trigger_raster_points > 0){
				dsp_scan.dnx_probe = int_2_demo_int (Demo_SPM_ControlClass->probe_trigger_raster_points);
				dsp_scan.raster_a = int_2_demo_int ((int)(1.+ceil((Demo_SPM_ControlClass->probe_trigger_raster_points-1.)/2)));
				dsp_scan.raster_b = int_2_demo_int ((int)(1.+floor((Demo_SPM_ControlClass->probe_trigger_raster_points-1.)/2)));
			} else {
				dsp_scan.dnx_probe = int_2_demo_int ((int)fabs((double)Demo_SPM_ControlClass->probe_trigger_raster_points));
				dsp_scan.raster_a = int_2_demo_int (0);
				dsp_scan.raster_b = int_2_demo_int (0);
			}
		}
		else{
			dsp_scan.dnx_probe = int_2_demo_int (-1); // not yet, auto probe trigger
			dsp_scan.raster_a = int_2_demo_int (0);
			dsp_scan.raster_b = int_2_demo_int (0);
		}
		
		dsp_scan.nx = Nx; // num datapoints in X to take
		dsp_scan.ny = Ny-1; // num datapoints in Y to take

		ydir = yindex > 0 ? -1 : 1;
	
		Demo_SPM_ControlClass->recalculate_dsp_scan_speed_parameters (dsp_scan);
#endif

		// start scan DSP generator and data aquisition thread now
		;
	}

	// call this while waiting for background data updates on screen...
	//		ReadScanData (yindex, num_srcs, Mob); has moved into the thread now

#if 0
	// wait for data, updated display, data move is done in background by the fifo read thread
	do {
		usleep (20000); // release cpu time
		gapp->check_events (); // do not lock
//		Demo_SPM_ControlClass->Probing_eventcheck_callback (NULL, Demo_SPM_ControlClass);
		if (ydir > 0 && yindex <= fifo_data_y_index) break;
		if (ydir < 0 && yindex >= fifo_data_y_index) break;

	} while (ScanningFlg);
#endif


// simple version:
	double x,y,z;
	Mem2d Line(Mob[0], Nx, 1);

	if (yindex <= 3){
		g_print ("ScanLineM: y=%d  rx=%ld ry=%ld  xdir=%d  Nx=%ld  Ny=%ld\n", yindex, rx, ry, xdir, Nx, Ny);
		g_print ("Dx, Dy:   %ld, %ld DAC steps inbetween\n", Dx, Dy);
		g_print ("Bias:     %g\n", Demo_SPM_ControlClass->bias);
		g_print ("CP, CI:   %g, %g\n", Demo_SPM_ControlClass->usr_cp, Demo_SPM_ControlClass->usr_ci);
		g_print ("Setpoint: %g\n", Demo_SPM_ControlClass->current_set_point);
		g_print ("Sc Speed: %g\n", Demo_SPM_ControlClass->scan_speed_x_requested);
	}

	if (yindex >= 0){
		double g1[2] = { 1.0000000, 0.0000 };
		double g2[2] = { 0.8660254, 0.5000 };

		for(int i=0; i<Nx; i++){
			double l1,l2, sz1, sz2;
			x = i*Dx*xdir + rx;
			y = ry;
			Transform(&x, &y);

			// x,y in DAC positions
			// now go back to real word for model to lattic indices

			x = gapp->xsm->Inst->Dig2XA ((long)x) / 3.4;
			y = gapp->xsm->Inst->Dig2YA ((long)y) / 3.4;

		        l1 = x*g1[0] + y*g1[1];
			l2 = x*g2[0] + y*g2[1];

			sz1 = remainder (l1, 7.0);
			sz2 = remainder (l2, 7.0);

			gboolean kh = fabs(sz1) < 0.5 && fabs(sz2) < 0.5 ? TRUE : FALSE;

			z = kh ? 50.*Demo_SPM_ControlClass->bias : (fabs (l1-round (l1)) < 0.2 && fabs (l2-round(l2)) < 0.2) ? 100. : 0.;

			Line.PutDataPkt (z, i, 0);
		}

		CallIdleFunc ();

		int i=0;
		do{
			//    Mob[i]->PutDataLine(yindex, (void*)dummy);
			Mob[i]->CopyFrom (&Line, 0, 0, ix0, yindex, Nx);
		}while ((++i < MAX_SRCS_CHANNELS) ? Mob[i]!=NULL : FALSE);
	}



	// update line, only if not canceled
	if (ScanningFlg)
		CallIdleFunc ();

	// pop all remaining events
//	Demo_SPM_ControlClass->Probing_eventcheck_callback (NULL, Demo_SPM_ControlClass);
}
