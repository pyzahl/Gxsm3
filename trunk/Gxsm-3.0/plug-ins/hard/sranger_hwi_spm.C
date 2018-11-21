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

#include "sranger_hwi.h"

#include "dsp-pci32/xsm/xsmcmd.h"

// need some SRanger io-controls 
// HAS TO BE IDENTICAL TO THE DRIVER's FILE!
#include "../plug-ins/hard/modules/sranger_ioctl.h"

// important notice:
// ----------------------------------------------------------------------
// all numbers need to be byte swaped on i386 (no change on ppc!!) via 
// int_2_sranger_int() or long_2_sranger_long()
// before use from DSP and before writing back
// ----------------------------------------------------------------------

// enable debug:
#define	SRANGER_DEBUG(S) XSM_DEBUG (DBG_L4, "sranger_hwi_spm: " << S )

extern GxsmPlugin sranger_hwi_pi;
extern DSPControl *DSPControlClass;
extern DSPMoverControl *DSPMoverClass;

/* 
 * Init things
 */

sranger_hwi_spm::sranger_hwi_spm():sranger_hwi_dev(){
	SRANGER_DEBUG("Init Sranger SPM");
	ScanningFlg=0;	
}

/*
 * Clean up
 */

sranger_hwi_spm::~sranger_hwi_spm(){
	SRANGER_DEBUG("Finish Sranger SPM");
}

/* 
 * SRanger Read/Write procedure:

 int ret;
 ret=lseek (dsp, address, SRANGER_SEEK_DATA_SPACE);
 ret=read (dsp, data, length<<1); 
 ret=write (dsp, data, length<<1); 

 *
 */

#define CONV_16(X) X = int_2_sranger_int (X)
#define CONV_32(X) X = long_2_sranger_long (X)

gint sranger_hwi_spm::RTQuery (const gchar *property, double &val1, double &val2, double &val3){
	static ANALOG_VALUES    dsp_analog;
	static struct { DSP_INT adc[8]; } dsp_analog_in;
//	static SPM_PI_FEEDBACK dsp_feedback_watch;
	static MOVE_OFFSET dsp_move;
	static gint ok=FALSE;

	if (*property == 'z'){
		lseek (dsp_alternative, magic_data.AIC_in, SRANGER_SEEK_DATA_SPACE);
		sr_read (dsp_alternative, &dsp_analog, sizeof (dsp_analog));

		CONV_16 (dsp_analog.x_scan);
		CONV_16 (dsp_analog.y_scan);
		CONV_16 (dsp_analog.z_scan);
		
		CONV_16 (dsp_analog.x_offset);
		CONV_16 (dsp_analog.y_offset);
		CONV_16 (dsp_analog.z_offset);
		
		val1 =  gapp->xsm->Inst->VZ() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.z_scan);
		val2 =  gapp->xsm->Inst->VX() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.x_scan);
		val3 =  gapp->xsm->Inst->VY() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.y_scan);
		
		if (gapp->xsm->Inst->OffsetMode () == OFM_ANALOG_OFFSET_ADDING){
			val1 +=  gapp->xsm->Inst->VZ0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.z_offset);
			val2 +=  gapp->xsm->Inst->VX0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.x_offset);
			val3 +=  gapp->xsm->Inst->VY0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.y_offset);
		}
		return TRUE;
	}
//	val1 =  (double)dsp_analog.z_scan;
//	val2 =  (double)dsp_analog.x_scan;
//	val3 =  (double)dsp_analog.y_scan;
	if (*property == 'o'){
		// read/convert and return offset
		// NEED to request 'z' property first, then this is valid and up-to-date!!!!
		if (gapp->xsm->Inst->OffsetMode () == OFM_ANALOG_OFFSET_ADDING){
			val1 =  gapp->xsm->Inst->VZ0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.z_offset);
			val2 =  gapp->xsm->Inst->VX0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.x_offset);
			val3 =  gapp->xsm->Inst->VY0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.y_offset);
		} else {
			val1 =  gapp->xsm->Inst->VZ() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.z_offset);
			val2 =  gapp->xsm->Inst->VX() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.x_offset);
			val3 =  gapp->xsm->Inst->VY() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.y_offset);
		}
		
		return ok;
	}

	if (*property == 'O'){
		// read HR offset move position
		lseek (dsp, magic_data.move, SRANGER_SEEK_DATA_SPACE);
		sr_read (dsp, &dsp_move, sizeof (dsp_move)); 
//		int_2_sranger_int (dsp_move.pflg)
		CONV_32 (dsp_move.XPos); // 16.16 position
		CONV_32 (dsp_move.YPos);
		CONV_32 (dsp_move.ZPos);
		if (gapp->xsm->Inst->OffsetMode () == OFM_ANALOG_OFFSET_ADDING){
			val1 =  gapp->xsm->Inst->VZ0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.ZPos/65536.);
			val2 =  gapp->xsm->Inst->VX0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.XPos/65536.);
			val3 =  gapp->xsm->Inst->VY0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.YPos/65536.);
		} else {
			val1 =  gapp->xsm->Inst->VZ() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.ZPos/65536.);
			val2 =  gapp->xsm->Inst->VX() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.XPos/65536.);
			val3 =  gapp->xsm->Inst->VY() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.YPos/65536.);
		}
		return ok;
	}

	if (*property == 'f'){
		lseek (dsp_alternative, magic_data.AIC_in, SRANGER_SEEK_DATA_SPACE);
		sr_read (dsp_alternative, &dsp_analog_in, sizeof (dsp_analog_in));

		// from DSP to PC
		for (int i=0; i<8; ++i)
			CONV_16 (dsp_analog_in.adc[i]);

//		lseek (dsp_alternative, magic_data.feedback, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
//		sr_read (dsp_alternative, &dsp_feedback_watch, sizeof (dsp_feedback_watch));

//		CONV_16 (dsp_feedback_watch.q_factor15);

//		if (dsp_feedback_watch.q_factor15 > 0)
//			val1 = -log ((double)dsp_feedback_watch.q_factor15 / 32767.) / (2.*M_PI/75000.); // f0 in Hz
//		else
//			val1 = 75000.;
//		val1 = -log (dsp_feedback_watch.q_factor15 / 32767.) / (2.*M_PI/75000.); // f0 in Hz
		val1 = 22000.; // fixed
		
		val2 = gapp->xsm->Inst->Dig2VoltIn (dsp_analog_in.adc[5]) / gapp->xsm->Inst->nAmpere2V(1.); // actual nA reading
		val3 = -1.; // N/A
		return TRUE;
	}

	return TRUE;
}

void sranger_hwi_spm::ExecCmd(int Cmd){
	// ----------------------------------------------------------------------
	// all numbers need to be byte swaped on i386 (no change on ppc!!) via 
	// int_2_sranger_int() or long_2_sranger_long()
	// before use from DSP and before writing back
	// ----------------------------------------------------------------------

	switch (Cmd){
	case DSP_CMD_START: // enable Feedback
	{
		static SPM_STATEMACHINE dsp_state;
		if (IS_AFM_CTRL){ // AFM
			// ensure use of linear feedback mode
			dsp_state.set_mode = int_2_sranger_int(0);
			dsp_state.clr_mode = int_2_sranger_int(MD_LOG);
		} else { // STM, ...
			// ensure use of logarithm feedback mode
			dsp_state.set_mode = int_2_sranger_int(MD_LOG);
			dsp_state.clr_mode = int_2_sranger_int(0);
		}
		lseek (dsp, magic_data.statemachine, SRANGER_SEEK_DATA_SPACE);
		sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 

		dsp_state.set_mode = int_2_sranger_int(MD_PID); // enable feedback
		dsp_state.clr_mode = int_2_sranger_int(0);
		lseek (dsp, magic_data.statemachine, SRANGER_SEEK_DATA_SPACE);
		sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
		break;
	}
	case DSP_CMD_HALT: // disable Feedback
	{
		static SPM_STATEMACHINE dsp_state;
		dsp_state.set_mode = int_2_sranger_int(0);
		dsp_state.clr_mode = int_2_sranger_int(MD_PID);
		lseek (dsp, magic_data.statemachine, SRANGER_SEEK_DATA_SPACE);
		sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
		break;
	}
	case DSP_CMD_APPROCH_MOV_XP: // auto approch "Mover"
	{
		if (DSPMoverClass->mover_param.MOV_mode & AAP_MOVER_WAVE){
			// scale waveform
			// download waveform
			DSPMoverClass->create_waveform (DSPMoverClass->mover_param.AFM_Amp, DSPMoverClass->mover_param.AFM_WavePeriod);
			lseek (dsp, EXTERN_DATA_FIFO_ADDRESS, SRANGER_SEEK_DATA_SPACE);
			sr_write (dsp, &DSPMoverClass->mover_param.MOV_waveform[0], DSPMoverClass->mover_param.MOV_wave_len<<1); 
		}
		static AUTOAPPROACH dsp_aap;
		dsp_aap.start = int_2_sranger_int(1);           /* Initiate =WO */
		dsp_aap.stop  = int_2_sranger_int(0);           /* Cancel   =WO */
		dsp_aap.mover_mode = int_2_sranger_int(AAP_MOVER_XP_AUTO_APP | AAP_MOVER_XP | DSPMoverClass->mover_param.MOV_mode | DSPMoverClass->mover_param.MOV_output);
		dsp_aap.piezo_steps = int_2_sranger_int((int)DSPMoverClass->mover_param.AFM_Steps);     /* max number of repetitions */
		dsp_aap.n_wait      = int_2_sranger_int((int)(22100.0*1e-3*DSPMoverClass->mover_param.final_delay));                     /* delay inbetween cycels */
		dsp_aap.u_piezo_amp = int_2_sranger_int((int)(32767*DSPMoverClass->mover_param.AFM_Amp/5)); /* Amplitude, Peak2Peak */
		if (DSPMoverClass->mover_param.MOV_mode & AAP_MOVER_WAVE){
			dsp_aap.u_piezo_max = int_2_sranger_int(DSPMoverClass->mover_param.MOV_wave_len); /* Length of Waveform*/
			dsp_aap.piezo_speed = int_2_sranger_int(DSPMoverClass->mover_param.MOV_wave_speed);     /* Wave Speed (hold number per step) */
		}else{
			dsp_aap.u_piezo_max = int_2_sranger_int((int)(32767*DSPMoverClass->mover_param.AFM_Amp/5)/2); /* Amplitude, Peak */
			dsp_aap.piezo_speed = (int)DSPMoverClass->mover_param.AFM_WavePeriod;     /* Speed */
			dsp_aap.piezo_speed = int_2_sranger_int(dsp_aap.piezo_speed >= 1 ? dsp_aap.piezo_speed : 1);
		}
		lseek (dsp, magic_data.autoapproach, SRANGER_SEEK_DATA_SPACE);
		sr_write (dsp, &dsp_aap,  MAX_WRITE_AUTOAPPROACH<<1); 
		break;
	}
	case DSP_CMD_APPROCH: // auto approch "Slider"
		break;
	case DSP_CMD_CLR_PA: // Stop all
	{
		static AUTOAPPROACH dsp_aap;
		dsp_aap.start = int_2_sranger_int(0);           /* Initiate =WO */
		dsp_aap.stop  = int_2_sranger_int(1);           /* Cancel   =WO */
		lseek (dsp, magic_data.autoapproach, SRANGER_SEEK_DATA_SPACE);
		sr_write (dsp, &dsp_aap,  2<<1); 
		break;
	}
	case DSP_CMD_AFM_MOV_XM: // manual move X-
	case DSP_CMD_AFM_MOV_XP: // manual move X+
	case DSP_CMD_AFM_MOV_YM: // manual move Y-
	case DSP_CMD_AFM_MOV_YP: // manual move Y+
	{
		if (DSPMoverClass->mover_param.MOV_mode & AAP_MOVER_WAVE){
			// download waveform
			// scale waveform
			DSPMoverClass->create_waveform (DSPMoverClass->mover_param.AFM_Amp, DSPMoverClass->mover_param.AFM_WavePeriod);
			lseek (dsp, EXTERN_DATA_FIFO_ADDRESS, SRANGER_SEEK_DATA_SPACE);
			sr_write (dsp, &DSPMoverClass->mover_param.MOV_waveform[0], DSPMoverClass->mover_param.MOV_wave_len<<1); 
		}
		static AUTOAPPROACH dsp_aap;
		dsp_aap.start = int_2_sranger_int(1);           /* Initiate =WO */
		dsp_aap.stop  = int_2_sranger_int(0);           /* Cancel   =WO */
		switch (Cmd){
		case DSP_CMD_AFM_MOV_XM: dsp_aap.mover_mode = AAP_MOVER_XM; break;
		case DSP_CMD_AFM_MOV_XP: dsp_aap.mover_mode = AAP_MOVER_XP; break;
		case DSP_CMD_AFM_MOV_YM: dsp_aap.mover_mode = AAP_MOVER_YM; break;
		case DSP_CMD_AFM_MOV_YP: dsp_aap.mover_mode = AAP_MOVER_YP; break;
		}

		dsp_aap.mover_mode = int_2_sranger_int(dsp_aap.mover_mode | DSPMoverClass->mover_param.MOV_mode | DSPMoverClass->mover_param.MOV_output);

		dsp_aap.piezo_steps = int_2_sranger_int((int)DSPMoverClass->mover_param.AFM_Steps);     /* max number of repetitions */
		dsp_aap.n_wait      = int_2_sranger_int(2);                         /* delay inbetween cycels */
		dsp_aap.u_piezo_amp = int_2_sranger_int((int)(32767*DSPMoverClass->mover_param.AFM_Amp/5)); /* Amplitude, Peak2Peak */
		if (DSPMoverClass->mover_param.MOV_mode & AAP_MOVER_WAVE){
			dsp_aap.u_piezo_max = int_2_sranger_int(DSPMoverClass->mover_param.MOV_wave_len); /* Length of Waveform*/
			dsp_aap.piezo_speed = int_2_sranger_int(DSPMoverClass->mover_param.MOV_wave_speed);     /* Wave Speed (hold number per step) */
		}else{
			dsp_aap.u_piezo_max = int_2_sranger_int((int)(32767*DSPMoverClass->mover_param.AFM_Amp/5)/2); /* Amplitude, Peak */
			dsp_aap.piezo_speed = (int)DSPMoverClass->mover_param.AFM_WavePeriod;     /* Speed */
			dsp_aap.piezo_speed = int_2_sranger_int(dsp_aap.piezo_speed >= 1 ? dsp_aap.piezo_speed : 1);
		}
		lseek (dsp, magic_data.autoapproach, SRANGER_SEEK_DATA_SPACE);
		sr_write (dsp, &dsp_aap,  MAX_WRITE_AUTOAPPROACH<<1); 
		break;
	}

//		case DSP_CMD_:
//				break;
	default: break;
	}
}

void sranger_hwi_spm::reset_scandata_fifo(int stall){
	static DATA_FIFO dsp_fifo;
	// reset DSP fifo read position
	dsp_fifo.r_position = int_2_sranger_int(0);
	dsp_fifo.w_position = int_2_sranger_int(0);
	dsp_fifo.fill  = int_2_sranger_int(0);
	dsp_fifo.stall = int_2_sranger_int(stall); // unlock (0) or lock (1) scan now
	lseek (dsp, magic_data.datafifo, SRANGER_SEEK_DATA_SPACE);
	sr_write (dsp, &dsp_fifo, (MAX_WRITE_DATA_FIFO+3)<<1);
}

void sranger_hwi_spm::tip_to_origin(double x, double y){
	static AREA_SCAN dsp_scan;

	// get current position
	lseek (dsp, magic_data.scan, SRANGER_SEEK_DATA_SPACE);
	sr_read (dsp, &dsp_scan, sizeof (dsp_scan));

	reset_scandata_fifo ();

	// move tip from current position to Origin i.e. x,y
	dsp_scan.Xpos = long_2_sranger_long (dsp_scan.Xpos);
	dsp_scan.Ypos = long_2_sranger_long (dsp_scan.Ypos);
	SRANGER_DEBUG("SR:EndScan2D last XYPos: " << (dsp_scan.Xpos>>16) << ", " << (dsp_scan.Ypos>>16));

	double Mdx = x - (double)dsp_scan.Xpos;
	double Mdy = y - (double)dsp_scan.Ypos;
	double mvspd = (1<<16) * sranger_hwi_pi.app->xsm->Inst->XA2Dig (DSPControlClass->move_speed_x) 
		/ DSPControlClass->frq_ref;
	double steps = round (sqrt (Mdx*Mdx + Mdy*Mdy) / mvspd);
	dsp_scan.fm_dx = (DSP_LONG)round(Mdx/steps);
	dsp_scan.fm_dy = (DSP_LONG)round(Mdy/steps);
	dsp_scan.num_steps_move_xy = (DSP_LONG)steps;

	double zx_ratio = sranger_hwi_pi.app->xsm->Inst->Dig2XA (1) / sranger_hwi_pi.app->xsm->Inst->Dig2ZA (1);
	double zy_ratio = sranger_hwi_pi.app->xsm->Inst->Dig2YA (1) / sranger_hwi_pi.app->xsm->Inst->Dig2ZA (1);
	dsp_scan.fm_dzxy = long_2_sranger_long ((DSP_LONG)round (zx_ratio * (double)dsp_scan.fm_dx * DSPControlClass->area_slope_x
						           + zy_ratio * (double)dsp_scan.fm_dy * DSPControlClass->area_slope_y));

	// setup scan for zero size scan and move to 0,0 in scan coord sys
	dsp_scan.fm_dx = long_2_sranger_long (dsp_scan.fm_dx);
	dsp_scan.fm_dy = long_2_sranger_long (dsp_scan.fm_dy);
	dsp_scan.num_steps_move_xy = long_2_sranger_long (dsp_scan.num_steps_move_xy);

	dsp_scan.start = int_2_sranger_int(1);
	dsp_scan.srcs_xp  = long_2_sranger_long(0);
	dsp_scan.srcs_xm  = long_2_sranger_long(0);
	dsp_scan.srcs_2nd_xp  = long_2_sranger_long (0);
	dsp_scan.srcs_2nd_xm  = long_2_sranger_long (0);
	dsp_scan.dnx_probe = int_2_sranger_int(-1);
	dsp_scan.nx_pre = int_2_sranger_int(0);
	dsp_scan.nx = int_2_sranger_int(0);
	dsp_scan.ny = int_2_sranger_int(0);
	dsp_scan.Xpos = long_2_sranger_long (dsp_scan.Xpos);
	dsp_scan.Ypos = long_2_sranger_long (dsp_scan.Ypos);

	// initiate "return to origin" "dummy" scan now
	lseek (dsp, magic_data.scan, SRANGER_SEEK_DATA_SPACE);
	sr_write (dsp, &dsp_scan, (MAX_WRITE_SCAN)<<1);

	// verify Pos,
	// wait until ready
	do {
		usleep (100000); // give some time
		sr_read (dsp, &dsp_scan, sizeof (dsp_scan));
	} while (dsp_scan.pflg);
	dsp_scan.Xpos = long_2_sranger_long (dsp_scan.Xpos);
	dsp_scan.Ypos = long_2_sranger_long (dsp_scan.Ypos);
	SRANGER_DEBUG("SR:EndScan2D return XYPos: " << (dsp_scan.Xpos>>16) << ", " << (dsp_scan.Ypos>>16));
}

// just note that we are scanning next...
void sranger_hwi_spm::StartScan2D(){
	DSPControlClass->StartScanPreCheck ();
	ScanningFlg=1; 
	KillFlg=FALSE; // if this gets TRUE while scanning, you should stopp it!!
}

// and its done now!
void sranger_hwi_spm::EndScan2D(){ 
	static AREA_SCAN dsp_scan;
	ScanningFlg=0; 
	lseek (dsp, magic_data.scan, SRANGER_SEEK_DATA_SPACE);
	sr_read (dsp, &dsp_scan, sizeof (dsp_scan));
	// cancel scanning?
	if (dsp_scan.pflg) {
		dsp_scan.start = int_2_sranger_int(0);
		dsp_scan.stop  = int_2_sranger_int(1);
		lseek (dsp, magic_data.scan, SRANGER_SEEK_DATA_SPACE);
		sr_write (dsp, &dsp_scan, MAX_WRITE_SCAN<<1);
	}
	// wait until ready
	do {
		usleep (20000); // release cpu time
		gapp->check_events ("Returning tip to home..."); // do not lock
		DSPControlClass->Probing_eventcheck_callback (NULL, DSPControlClass);
		sr_read (dsp, &dsp_scan, sizeof (dsp_scan));
	} while (dsp_scan.pflg);

	// do return to center?
	if (DSPControlClass->center_return_flag)
		tip_to_origin ();

	DSPControlClass->EndScanCheck ();
}

// we are paused
void sranger_hwi_spm::PauseScan2D(){
	ScanningFlg=0;
}

// and its going againg
void sranger_hwi_spm::ResumeScan2D(){
	ScanningFlg=1;
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void sranger_hwi_spm::SetOffset(double x, double y){
	static double old_x=123456789, old_y=123456789;
	static MOVE_OFFSET dsp_move;
	double dx,dy,steps;
	const double fract = 1<<16;

	if (old_x == x && old_y == y) return;

	SRANGER_DEBUG("SetOffset: " << x << ", " << y);
	old_x = x; old_y = y;
	
	do {
		lseek (dsp, magic_data.move, SRANGER_SEEK_DATA_SPACE);
		sr_read (dsp, &dsp_move, sizeof (dsp_move)); 
		if (int_2_sranger_int (dsp_move.pflg)){
			if (long_2_sranger_long (dsp_move.num_steps)){
				usleep (50000);
			}
		}
	} while (int_2_sranger_int (dsp_move.pflg) && long_2_sranger_long (dsp_move.num_steps)); // wait if there is a move in progress

	// convert -- only on i386 necessary
	dsp_move.start = int_2_sranger_int (1);
	dsp_move.Xnew  = int_2_sranger_int ((DSP_INT)round(x));
	dsp_move.Ynew  = int_2_sranger_int ((DSP_INT)round(y));

	dx = ((double)x * fract) - (double)long_2_sranger_long (dsp_move.XPos);
	dy = ((double)y * fract) - (double)long_2_sranger_long (dsp_move.YPos);
	double mvspd = fract * sranger_hwi_pi.app->xsm->Inst->X0A2Dig (DSPControlClass->move_speed_x) / DSPControlClass->frq_ref;
	steps = round (sqrt (dx*dx + dy*dy) / mvspd);
	dsp_move.f_dx = long_2_sranger_long ((DSP_LONG)round(dx/steps));
	dsp_move.f_dy = long_2_sranger_long ((DSP_LONG)round(dy/steps));
	dsp_move.num_steps = long_2_sranger_long ((DSP_LONG)steps);
	
	lseek (dsp, magic_data.move, SRANGER_SEEK_DATA_SPACE);
	sr_write (dsp, &dsp_move, MAX_WRITE_MOVE<<1);
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

void sranger_hwi_spm::ScanLineM(int yindex, int xdir, int lssrcs, Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0 ){
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
		lseek (dsp, magic_data.scan, SRANGER_SEEK_DATA_SPACE);
		sr_read (dsp, &dsp_scan, sizeof (dsp_scan));
		// cancel scanning?
		if (dsp_scan.pflg) {
			dsp_scan.start = int_2_sranger_int (0);
			dsp_scan.stop  = int_2_sranger_int (1);
			lseek (dsp, magic_data.scan, SRANGER_SEEK_DATA_SPACE);
			sr_write (dsp, &dsp_scan, MAX_WRITE_SCAN<<1);
			usleep (50000); // give some time to stop
		}
		
		// reset DSP fifo read position
		reset_scandata_fifo (1); // lock scan now, released at start of fifo read thread
		
		// clean up any old vector probe data
		DSPControlClass->free_probedata_arrays ();
		
		
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
		// ------> SRanger::EndScan2D always returns to 0/0 <------
		lseek (dsp, magic_data.scan, SRANGER_SEEK_DATA_SPACE);
		sr_read (dsp, &dsp_scan, sizeof (dsp_scan));
		
		int mxx,mxy,myx,myy;
		// rotation matrix -- where is the round() function gone in math.h ???
		mxx = int_2_sranger_int ((int)(rotmxx*(1<<15)));
		mxy = int_2_sranger_int ((int)(rotmxy*(1<<15)));
		myx = int_2_sranger_int ((int)(rotmyx*(1<<15)));
		myy = int_2_sranger_int ((int)(rotmyy*(1<<15)));
		
		// check for new rotation
		if (dsp_scan.rotxx != mxx || dsp_scan.rotxy != mxy || dsp_scan.rotyx != myx || dsp_scan.rotyy != myy){
			// move to origin (rotation invariant point) before any rotation matrix changes!!
			tip_to_origin ();
			// reread position
			lseek (dsp, magic_data.scan, SRANGER_SEEK_DATA_SPACE);
			sr_read (dsp, &dsp_scan, sizeof (dsp_scan));
			// set new rotation matrix
			dsp_scan.rotxx = mxx;
			dsp_scan.rotxy = mxy;
			dsp_scan.rotyx = myx;
			dsp_scan.rotyy = myy;
		}
		
		// convert
		dsp_scan.Xpos = long_2_sranger_long (dsp_scan.Xpos);
		dsp_scan.Ypos = long_2_sranger_long (dsp_scan.Ypos);
		
		SRANGER_DEBUG("SR:Start/Rot XYPos: " << (dsp_scan.Xpos>>16) << ", " << (dsp_scan.Ypos>>16));

		// setup scan
		dsp_scan.start = int_2_sranger_int (1);
		
		// 1st XP scan here
		//   enable do probe at XP?
		if (DSPControlClass->Source && DSPControlClass->probe_trigger_raster_points > 0)
			dsp_scan.srcs_xp  = long_2_sranger_long (srcs_dir[0] | 0x0008);
		else
			dsp_scan.srcs_xp  = long_2_sranger_long (srcs_dir[0]);
		
		dsp_scan.srcs_xm  = long_2_sranger_long (srcs_dir[1]);
		
		// 2nd scan, setup later, see below
		dsp_scan.srcs_2nd_xp  = long_2_sranger_long (srcs_dir[2]);
		dsp_scan.srcs_2nd_xm  = long_2_sranger_long (srcs_dir[3]);

		// --- DISABLED ---
		dsp_scan.Zoff_2nd_xp  = int_2_sranger_int (0); // init to zero, set later
		dsp_scan.Zoff_2nd_xm  = int_2_sranger_int (0); // init to zero, set later

		// enable probe?
		if (DSPControlClass->Source && DSPControlClass->probe_trigger_raster_points){
			if (DSPControlClass->Source && DSPControlClass->probe_trigger_raster_points > 0){
				dsp_scan.dnx_probe = int_2_sranger_int (DSPControlClass->probe_trigger_raster_points);
				dsp_scan.raster_a = int_2_sranger_int ((int)(1.+ceil((DSPControlClass->probe_trigger_raster_points-1.)/2)));
				dsp_scan.raster_b = int_2_sranger_int ((int)(1.+floor((DSPControlClass->probe_trigger_raster_points-1.)/2)));
			} else {
				dsp_scan.dnx_probe = int_2_sranger_int ((int)fabs((double)DSPControlClass->probe_trigger_raster_points));
				dsp_scan.raster_a = int_2_sranger_int (0);
				dsp_scan.raster_b = int_2_sranger_int (0);
			}
		}
		else{
			dsp_scan.dnx_probe = int_2_sranger_int (-1); // not yet, auto probe trigger
			dsp_scan.raster_a = int_2_sranger_int (0);
			dsp_scan.raster_b = int_2_sranger_int (0);
		}
		
		dsp_scan.nx = Nx; // num datapoints in X to take
		dsp_scan.ny = Ny-1; // num datapoints in Y to take

		ydir = yindex > 0 ? -1 : 1;
	
		DSPControlClass->recalculate_dsp_scan_speed_parameters (dsp_scan);

		const double fract = 1<<16;
		// from current position to Origin/Start
		// init .dsp_scan for initial move to scan start pos
		double Mdx = -(double)(Nx+1)*dsp_scan.dnx*dsp_scan.fs_dx/2. - (double)dsp_scan.Xpos;
		double Mdy =  (double)(Ny-1)*dsp_scan.dny*dsp_scan.fs_dy/2. - (double)dsp_scan.Ypos;
		double mvspd = fract * sranger_hwi_pi.app->xsm->Inst->XA2Dig (DSPControlClass->move_speed_x) / DSPControlClass->frq_ref;
		double steps = round (sqrt (Mdx*Mdx + Mdy*Mdy) / mvspd);
		dsp_scan.fm_dx = (DSP_LONG)round(Mdx/steps);
		dsp_scan.fm_dy = (DSP_LONG)round(Mdy/steps);
		dsp_scan.num_steps_move_xy = (DSP_LONG)steps;

		DSPControlClass->recalculate_dsp_scan_slope_parameters (dsp_scan);

		// convert to DSP
		dsp_scan.nx = int_2_sranger_int (Nx); // num datapoints in X to take
		dsp_scan.ny = int_2_sranger_int (Ny-1); // num datapoints in Y to take

		dsp_scan.fs_dx = long_2_sranger_long (dsp_scan.fs_dx);
		dsp_scan.fs_dy = long_2_sranger_long (dsp_scan.fs_dy);
		dsp_scan.dnx = int_2_sranger_int (dsp_scan.dnx);
		dsp_scan.dny = int_2_sranger_int (dsp_scan.dny);
		dsp_scan.nx_pre = int_2_sranger_int(dsp_scan.nx_pre);

		dsp_scan.fm_dx = long_2_sranger_long (dsp_scan.fm_dx);
		dsp_scan.fm_dy = long_2_sranger_long (dsp_scan.fm_dy);
		dsp_scan.num_steps_move_xy = long_2_sranger_long (dsp_scan.num_steps_move_xy);

		dsp_scan.fm_dzx  = long_2_sranger_long (dsp_scan.fm_dzx);
		dsp_scan.fm_dzy  = long_2_sranger_long (dsp_scan.fm_dzy);
		dsp_scan.fm_dzxy = long_2_sranger_long (dsp_scan.fm_dzxy);
				
		// initiate scan now, this starts a 2D scan process!!!
		lseek (dsp, magic_data.scan, SRANGER_SEEK_DATA_SPACE);
		sr_write (dsp, &dsp_scan, (MAX_WRITE_SCAN)<<1); // write ix,iy - only here

		start_fifo_read (yindex, nsrcs_dir[0], nsrcs_dir[1], nsrcs_dir[2], nsrcs_dir[3], Mob_dir[0], Mob_dir[1], Mob_dir[2], Mob_dir[3]);
	}

	// call this while waiting for background data updates on screen...
	//		ReadScanData (yindex, num_srcs, Mob); has moved into the thread now

	// wait for data, updated display, data move is done in background by the fifo read thread
	do {
		usleep (20000); // release cpu time
		gapp->check_events_self(); // do not lock, quite
		DSPControlClass->Probing_eventcheck_callback (NULL, DSPControlClass);
		if (ydir > 0 && yindex <= fifo_data_y_index) break;
		if (ydir < 0 && yindex >= fifo_data_y_index) break;

	} while (ScanningFlg);

	// update line, only if not canceled
	if (ScanningFlg)
		CallIdleFunc ();

	// pop all remaining events
	DSPControlClass->Probing_eventcheck_callback (NULL, DSPControlClass);
}
