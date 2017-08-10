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


#include "glbvars.h"
#include "plug-ins/hard/modules/dsp.h"
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sranger_mk2_hwi.h"

#include "dsp-pci32/xsm/xsmcmd.h"

// need some SRanger io-controls 
// HAS TO BE IDENTICAL TO THE DRIVER's FILE!
#include "../plug-ins/hard/modules/sranger_mk23_ioctl.h"

// important notice:
// ----------------------------------------------------------------------
// all numbers need to be byte swaped on i386 (no change on ppc!!) via 
// int_2_sranger_int() or long_2_sranger_long()
// before use from DSP and before writing back
// ----------------------------------------------------------------------

// enable debug:
#define	SRANGER_DEBUG(S) XSM_DEBUG (DBG_L4, "sranger_mk2_hwi_spm: " << S )

extern GxsmPlugin sranger_mk2_hwi_pi;
extern DSPControl *DSPControlClass;
extern DSPMoverControl *DSPMoverClass;

int gpio_monitor_out = 0;
int gpio_monitor_in  = 0;
int gpio_monitor_dir = 0;

void dumpbuffer(unsigned char *buffer, int size, int i0){
        int i,j,in;
        in=size;
        for(i=i0; i<in; i+=16){
                printf("%06X:",i>>1);
                for(j=0; (j<16) && (i+j)<size; j++)
                        printf(" %02x",buffer[i+j]);
                printf("   ");
                for(j=0; (j<16) && (i+j)<size; j++)
                        if(isprint(buffer[j+i]))
                                printf("%c",buffer[i+j]);
                        else
                                printf(".");
                printf("\n");
        }
}


/* 
 * Init things
 */

sranger_mk2_hwi_spm::sranger_mk2_hwi_spm():sranger_mk2_hwi_dev(){
	SRANGER_DEBUG("Init Sranger SPM");
	ScanningFlg=0;	
}

/*
 * Clean up
 */

sranger_mk2_hwi_spm::~sranger_mk2_hwi_spm(){
	SRANGER_DEBUG("Finish Sranger SPM");
}

/* 
 * SRanger Read/Write procedure:

 int ret;
 ret=lseek (dsp, address, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
 ret=read (dsp, data, length<<1); 
 ret=write (dsp, data, length<<1); 

 *
 */

#define CONV_16(X) X = int_2_sranger_int (X)
#define CONV_U16(X) X = uint_2_sranger_uint (X)
#define CONV_32(X) X = long_2_sranger_long (X)

/*
 Real-Time Query of DSP signals/values, auto buffered for z,o,R
 Propertiy hash:      return val1, val2, val3:
 "z" :                ZS, XS, YS  with offset!! -- in volts after piezo amplifier
 "o" :                Z0, X0, Y0  offset -- in volts after piezo amplifier
 "R" :                expected Z, X, Y -- in Angstroem/base unit
 "f" :                dFreq, I-avg, I-RMS
 "s" :                DSP Statemachine Status Bits, DSP load, DSP load peak
 "Z" :                probe Z Position
 "i" :                GPIO (high level speudo monitor)
 "A" :                Mover/Wave axis counts 0,1,2 (X/Y/Z)
 */

gint sranger_mk2_hwi_spm::RTQuery (const gchar *property, double &val1, double &val2, double &val3){
        const gint64 max_age = 50000; // 50ms
        static gint64 time_of_last_xyz_reading = 0; // abs time in us
        static gint64 time_of_last_fb_reading = 0; // abs time in us
	static struct { DSP_INT x_offset, y_offset, z_offset, x_scan, y_scan, z_scan, bias, motor; } dsp_analog;
	static struct { DSP_INT adc[8]; } dsp_analog_in;
	static MOVE_OFFSET dsp_move;
	static AREA_SCAN dsp_scan;
	static PROBE dsp_probe;
	static SPM_PI_FEEDBACK dsp_feedback_watch;
	static SPM_STATEMACHINE dsp_statemachine;
        static AUTOAPPROACH dsp_aap;
	static gint ok=FALSE;

        // auto buffered
        if ( (*property == 'z' || *property == 'R' || *property == 'o') && (time_of_last_xyz_reading+max_age) < g_get_real_time () ){
		// read/convert 3D tip positon
		lseek (dsp_alternative, magic_data.AIC_out, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp_alternative, &dsp_analog, sizeof (dsp_analog));

		CONV_16 (dsp_analog.x_scan);
		CONV_16 (dsp_analog.y_scan);
		CONV_16 (dsp_analog.z_scan);
		
		CONV_16 (dsp_analog.x_offset);
		CONV_16 (dsp_analog.y_offset);
		CONV_16 (dsp_analog.z_offset);

                time_of_last_xyz_reading = g_get_real_time ();
        }
        
	if (*property == 'z'){

		val1 =  gapp->xsm->Inst->VZ() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.z_scan);
		val2 =  gapp->xsm->Inst->VX() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.x_scan);
		val3 =  gapp->xsm->Inst->VY() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.y_scan);
		
		if (gapp->xsm->Inst->OffsetMode () == OFM_ANALOG_OFFSET_ADDING){
//			val1 +=  gapp->xsm->Inst->VZ0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.z_offset);
			val2 +=  gapp->xsm->Inst->VX0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.x_offset);
			val3 +=  gapp->xsm->Inst->VY0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.y_offset);
		}
		ok=TRUE;
		return TRUE;
	}
        // ZXY in Angstroem
        if (*property == 'R'){
                // ZXY Volts after Piezoamp -- without analog offset -> Dig -> ZXY in Angstroem
		val1 = gapp->xsm->Inst->V2ZAng (gapp->xsm->Inst->VZ() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.z_scan));
		val2 = gapp->xsm->Inst->V2XAng (gapp->xsm->Inst->VX() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.x_scan));
                val3 = gapp->xsm->Inst->V2YAng (gapp->xsm->Inst->VY() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_analog.y_scan));
		return TRUE;
        }

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

        // unbuffered
	if (*property == 'O'){
		// read HR offset move position
		lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp, &dsp_move, sizeof (dsp_move)); 
//		int_2_sranger_int (dsp_move.pflg)
		CONV_32 (dsp_move.xyz_vec[i_X]); // 16.16 position
		CONV_32 (dsp_move.xyz_vec[i_Y]);
		CONV_32 (dsp_move.xyz_vec[i_Z]);
		CONV_16 (dsp_move.pflg);
		if (gapp->xsm->Inst->OffsetMode () == OFM_ANALOG_OFFSET_ADDING){
			val1 =  gapp->xsm->Inst->VZ0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.xyz_vec[i_Z]/65536.);
			val2 =  gapp->xsm->Inst->VX0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.xyz_vec[i_X]/65536.);
			val3 =  gapp->xsm->Inst->VY0() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.xyz_vec[i_Y]/65536.);
		} else {
			val1 =  gapp->xsm->Inst->VZ() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.xyz_vec[i_Z]/65536.);
			val2 =  gapp->xsm->Inst->VX() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.xyz_vec[i_X]/65536.);
			val3 =  gapp->xsm->Inst->VY() * gapp->xsm->Inst->Dig2VoltOut((double)dsp_move.xyz_vec[i_Y]/65536.);
		}
		return ok;
	}

        // unbuffered
        if ( (*property == 'f') && (time_of_last_fb_reading+max_age) < g_get_real_time () ){
#if 0
		lseek (dsp_alternative, magic_data.AIC_in, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		read  (dsp_alternative, &dsp_analog_in, sizeof (dsp_analog_in));

		// from DSP to PC
		for (int i=0; i<8; ++i)
			CONV_16 (dsp_analog_in.adc[i]);
#endif
		// read IIR life conditions
		lseek (dsp_alternative, magic_data.feedback, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp_alternative, &dsp_feedback_watch, sizeof (dsp_feedback_watch));

		CONV_16 (dsp_feedback_watch.q_factor15);
		CONV_16 (dsp_feedback_watch.I_cross);
		CONV_16 (dsp_feedback_watch.I_offset);
		CONV_32 (dsp_feedback_watch.I_avg);
		CONV_32 (dsp_feedback_watch.I_rms);
		CONV_16 (dsp_feedback_watch.watch);

                time_of_last_fb_reading = g_get_real_time ();
        }

	if (*property == 'f'){
		if (dsp_feedback_watch.q_factor15 > 0 && dsp_feedback_watch.I_cross > 0)
			val1 = -log ((double)dsp_feedback_watch.q_factor15 / 32767.) / (2.*M_PI/75000.); // f0 in Hz
		else
			val1 = 75000.; // FBW
//		val2 = gapp->xsm->Inst->Dig2VoltIn (dsp_analog_in.adc[5]) / gapp->xsm->Inst->nAmpere2V(1.); // actual nA reading    xxxx V  * 0.1nA/V
		val2 = (double)dsp_feedback_watch.I_avg/256. * gapp->xsm->Inst->Dig2VoltIn (1) / gapp->xsm->Inst->nAmpere2V(1.); // actual nA reading    xxxx V  * 0.1nA/V
		val3 = sqrt((double)dsp_feedback_watch.I_rms/256.) * gapp->xsm->Inst->Dig2VoltIn (1) / gapp->xsm->Inst->nAmpere2V(1.); // actual nA RMS reading    xxxx V  * 0.1nA/V
		return TRUE;
	}

	// DSP Status Indicators
	if (*property == 's'){
		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp, &dsp_statemachine, sizeof (dsp_statemachine)); 
		CONV_16 (dsp_statemachine.mode);
		CONV_16 (dsp_statemachine.DataProcessTime);
		CONV_16 (dsp_statemachine.IdleTime);
		CONV_16 (dsp_statemachine.DataProcessTime_Peak);
		CONV_16 (dsp_statemachine.IdleTime_Peak);

		sr_read  (dsp, &dsp_scan, sizeof (dsp_scan)); 
		CONV_16 (dsp_scan.pflg);

		lseek (dsp, magic_data.probe, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp, &dsp_probe, sizeof (dsp_probe)); 
		CONV_16 (dsp_probe.pflg);

		// scan watchdog!
		if (ScanningFlg && dsp_scan.pflg==0) // stop/cancel scan+FIFO-read in progress on Gxsm level if other wise interruped/finsihed FIFO overrun, etc.
		  ;	//	EndScan2D();

		// Bit Coded Status:
		// 1: FB watch
		// 2,4: AREA-SCAN-MODE scanning, paused
		// 8: PROBE
		val1 = (double)(0
				+ (dsp_feedback_watch.watch ? 1:0)
				+ (( dsp_scan.pflg & 3) << 1)
				+ ((dsp_probe.pflg & 1) << 3)
				+ (( dsp_move.pflg & 1) << 4)
			);

// "  DSP Load: %.1f" %(100.*SPM_STATEMACHINE[ii_statemachine_DataProcessTime]/(SPM_STATEMACHINE[ii_statemachine_DataProcessTime]+SPM_STATEMACHINE[ii_statemachine_IdleTime]))
// "  Peak: %.1f" %(100.*SPM_STATEMACHINE[ii_statemachine_DataProcessTime_Peak]/(SPM_STATEMACHINE[ii_statemachine_DataProcessTime_Peak]+SPM_STATEMACHINE[ii_statemachine_IdleTime_Peak]))
		val2 = (double)dsp_statemachine.DataProcessTime / (double)(dsp_statemachine.DataProcessTime + dsp_statemachine.IdleTime); // DSP Load
		val3 = (double)dsp_statemachine.DataProcessTime_Peak / (double)(dsp_statemachine.DataProcessTime_Peak + dsp_statemachine.IdleTime_Peak); // DSP Peak Load
		
		return TRUE;
	}

	// quasi GPIO monitor/mirror -- HIGH LEVEL!!!
	if (*property == 'i'){
#define REALTIME_GPIO_WATCH
#ifdef REALTIME_GPIO_WATCH
		static CR_GENERIC_IO dsp_gpio;
		dsp_gpio.start = int_2_sranger_int(2); // MAKE DSP READ GPIO
		dsp_gpio.stop  = int_2_sranger_int(0);
		lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_gpio, 2<<1); 
		int i=10;
		do {
			lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC); // READ RESULT
			sr_read (dsp, &dsp_gpio, sizeof (dsp_gpio)); 
			CONV_U16 (dsp_gpio.start);
			if (i<10) usleep(10000);
		} while (dsp_gpio.start && --i);
		// WARNING: fix me !! issue w signed/unsigned bit 16 will not work rigth!!
		CONV_U16 (dsp_gpio.gpio_data_out);
		CONV_U16 (dsp_gpio.gpio_data_in);
		CONV_U16 (dsp_gpio.gpio_direction_bits);
		gpio_monitor_out = dsp_gpio.gpio_data_out & DSPMoverClass->mover_param.GPIO_direction;
		gpio_monitor_in  = dsp_gpio.gpio_data_in  & (~DSPMoverClass->mover_param.GPIO_direction);
		gpio_monitor_dir = dsp_gpio.gpio_direction_bits;
#endif
		val1 = (double)gpio_monitor_out;
		val2 = (double)gpio_monitor_in;
		val3 = (double)gpio_monitor_dir;
	}

        // unbuffered, so far only user input/button press triggered, no need to buffer
        if ( *property == 'A' ){    // ... && (time_of_last_xyz_reading+max_age) < g_get_real_time () ){
                // read DSP level wave/mover axis counts
		lseek (dsp_alternative, magic_data.autoapproach, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp_alternative, &dsp_aap, sizeof (dsp_aap));
		CONV_16 (dsp_aap.count_axis[0]);
		CONV_16 (dsp_aap.count_axis[1]);
		CONV_16 (dsp_aap.count_axis[2]);
                val1 = (double)dsp_aap.count_axis[0];
                val2 = (double)dsp_aap.count_axis[1];
                val3 = (double)dsp_aap.count_axis[2];
        }
        
//	printf ("ZXY: %g %g %g\n", val1, val2, val3);

//	val1 =  (double)dsp_analog.z_scan;
//	val2 =  (double)dsp_analog.x_scan;
//	val3 =  (double)dsp_analog.y_scan;

	return TRUE;
}

void sranger_mk2_hwi_spm::SetMode(int mode){
	static SPM_STATEMACHINE dsp_state;
	dsp_state.set_mode = int_2_sranger_int(mode);
	dsp_state.clr_mode = int_2_sranger_int(0);
	lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
}

void sranger_mk2_hwi_spm::ClrMode(int mode){
	static SPM_STATEMACHINE dsp_state;
	dsp_state.set_mode = int_2_sranger_int(0);
	dsp_state.clr_mode = int_2_sranger_int(mode);
	lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
}

void sranger_mk2_hwi_spm::ExecCmd(int Cmd){
	static int wave_form_address=0;

	// query wave form buffer -- shared w probe fifo!!! 
	if (!wave_form_address){
		static DATA_FIFO_EXTERN fifo;
		lseek (dsp, magic_data.probedatafifo, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp, &fifo, sizeof (fifo));
		check_and_swap (fifo.buffer_base);
		wave_form_address=(int)fifo.buffer_base;
	}

	// ----------------------------------------------------------------------
	// all numbers need to be byte swaped on i386 (no change on ppc!!) via 
	// int_2_sranger_int() or long_2_sranger_long()
	// before use from DSP and before writing back
	// ----------------------------------------------------------------------

	switch (Cmd){
	case DSP_CMD_START: // enable Feedback
	{
		static SPM_STATEMACHINE dsp_state;
		dsp_state.set_mode = int_2_sranger_int(MD_PID); // enable feedback
		dsp_state.clr_mode = int_2_sranger_int(0);
		lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
		break;
	}
	case DSP_CMD_HALT: // disable Feedback
	{
		static SPM_STATEMACHINE dsp_state;
		dsp_state.set_mode = int_2_sranger_int(0);
		dsp_state.clr_mode = int_2_sranger_int(MD_PID);
		lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
		break;
	}
	case DSP_CMD_APPROCH_MOV_XP: // auto approach "Mover"
	{
		static AUTOAPPROACH dsp_aap;
		dsp_aap.start = int_2_sranger_int(1);           /* Initiate =WO */
		dsp_aap.stop  = int_2_sranger_int(0);           /* Cancel   =WO */

		if (DSPMoverClass->mover_param.MOV_mode & AAP_MOVER_WAVE){
			// create, convert and download waveform(s) to DSP
			DSPMoverClass->create_waveform (DSPMoverClass->mover_param.AFM_Amp, DSPMoverClass->mover_param.AFM_Speed);

			for (int i=0; i<DSPMoverClass->mover_param.MOV_wave_len; ++i)
				DSPMoverClass->mover_param.MOV_waveform[i] = int_2_sranger_int (DSPMoverClass->mover_param.MOV_waveform[i]);

			lseek (dsp, wave_form_address, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
			sr_write (dsp, &DSPMoverClass->mover_param.MOV_waveform[0], DSPMoverClass->mover_param.MOV_wave_len<<1); 
		}

                gint channels = 1;
                switch (DSPMoverClass->mover_param.MOV_waveform_id){
                case MOV_WAVE_BESOCKE:
                case MOV_WAVE_SINE: channels = 3; break;
                case MOV_WAVE_KOALA: channels = 2; break;
                default: channels = 1; break;
                }
                // wave play setup for auto approach

                // counter setup -- optional, not essential for stepping, only for keeping a count for up to 3 axis
		dsp_aap.dir  = int_2_sranger_int (1); // arbitrary direction, assume +1 for approaching direction -- only for counter increment/decement
                dsp_aap.axis = int_2_sranger_int (2); // arbitrary assignmet for counter: 2=Z axis        

		// configure wave[0,1,...] out channel destination
		dsp_aap.n_wave_channels    = int_2_sranger_int (channels); /* number wave channels -- up top 6, must match wave data */
                for (int i=0; i<channels; ++i) // multi channel wave -- test on "X"
                        dsp_aap.channel_mapping[i] = long_2_sranger_long (DSPMoverClass->mover_param.wave_out_channel_dsp[i]);
		// ... [5] (configure all channels!)

		dsp_aap.mover_mode = int_2_sranger_int (AAP_MOVER_AUTO_APP | AAP_MOVER_WAVE_PLAY);
                //DSPMoverClass->mover_param.MOV_mode 
		//DSPMoverClass->mover_param.MOV_output 
                //DSPMoverClass->mover_param.inch_worm_phase > 0. ? AAP_MOVER_IWMODE : 0

		dsp_aap.max_wave_cycles = int_2_sranger_int (2*(int)DSPMoverClass->mover_param.AFM_Steps);     /* max number of repetitions */
		dsp_aap.wave_length     = int_2_sranger_int (DSPMoverClass->mover_param.MOV_wave_len); /* Length of Waveform -- total count all samples/channels */
		dsp_aap.wave_speed      = int_2_sranger_int (DSPMoverClass->mover_param.MOV_wave_speed_fac);     /* Wave Speed (hold number per step) */

                // auto app parameters
		dsp_aap.ci_retract  = float_2_sranger_q15 (0.1 * DSPMoverClass->mover_param.retract_ci / sranger_mk2_hwi_pi.app->xsm->Inst->VZ ()); // CI setting for reversing Z (retract)
                //**** dsp_feedback.ci = float_2_sranger_q15 (0.01 * z_servo[SERVO_CI] / sranger_mk2_hwi_pi.app->xsm->Inst->VZ ());
                dsp_aap.n_wait      = int_2_sranger_int((int)(DSPControlClass->frq_ref*1e-3*DSPMoverClass->mover_param.final_delay));                     /* delay inbetween cycels */
		dsp_aap.n_wait_fin  = long_2_sranger_long((int)(DSPControlClass->frq_ref*1e-3*DSPMoverClass->mover_param.max_settling_time));                     /* delay inbetween cycels */

                lseek (dsp, magic_data.autoapproach, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
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
		lseek (dsp, magic_data.autoapproach, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_aap,  2<<1); 
		break;
	}
	case DSP_CMD_AFM_MOV_XM: // manual move X-
	case DSP_CMD_AFM_MOV_XP: // manual move X+
	case DSP_CMD_AFM_MOV_YM: // manual move Y-
	case DSP_CMD_AFM_MOV_YP: // manual move Y+
        case DSP_CMD_AFM_MOV_ZM: // manual move Z-
        case DSP_CMD_AFM_MOV_ZP: // manual move Z+
	{
		static AUTOAPPROACH dsp_aap;
		dsp_aap.start = int_2_sranger_int(1);           /* Initiate =WO */
		dsp_aap.stop  = int_2_sranger_int(0);           /* Cancel   =WO */

		if (DSPMoverClass->mover_param.MOV_mode & AAP_MOVER_WAVE){
			// create, convert and download waveform(s) to DSP
                        switch (Cmd){
                        case DSP_CMD_AFM_MOV_XM:
                        case DSP_CMD_AFM_MOV_YM:
                        case DSP_CMD_AFM_MOV_ZM:
                                dsp_aap.dir  = int_2_sranger_int (-1); // arbitrary direction, assume -1 "left/down"
                                DSPMoverClass->create_waveform (DSPMoverClass->mover_param.AFM_Amp, -DSPMoverClass->mover_param.AFM_Speed);
                                break;
                        case DSP_CMD_AFM_MOV_XP:
                        case DSP_CMD_AFM_MOV_YP:
                        case DSP_CMD_AFM_MOV_ZP:
                                dsp_aap.dir  = int_2_sranger_int (1); // arbitrary direction, assume +1 "right/up"
                                DSPMoverClass->create_waveform (DSPMoverClass->mover_param.AFM_Amp, DSPMoverClass->mover_param.AFM_Speed);
                                break;
                        }
			
			for (int i=0; i<DSPMoverClass->mover_param.MOV_wave_len; ++i)
				DSPMoverClass->mover_param.MOV_waveform[i] = int_2_sranger_int (DSPMoverClass->mover_param.MOV_waveform[i]);

			lseek (dsp, wave_form_address, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
			sr_write (dsp, &DSPMoverClass->mover_param.MOV_waveform[0], DSPMoverClass->mover_param.MOV_wave_len<<1); 
		}
                
                gint channels = 1;
                switch (DSPMoverClass->mover_param.MOV_waveform_id){
                case MOV_WAVE_BESOCKE:
                case MOV_WAVE_SINE: channels = 3; break;
                case MOV_WAVE_KOALA: channels = 2; break;
                default: channels = 1; break;
                }
                // wave play setup for auto approach
		// configure wave[0,1,...] out channel destination
		dsp_aap.n_wave_channels    = int_2_sranger_int (channels); /* number wave channels -- up top 6, must match wave data */
                switch (Cmd){
                case DSP_CMD_AFM_MOV_XM:
                case DSP_CMD_AFM_MOV_XP:
                        dsp_aap.axis = int_2_sranger_int (0); // arbitrary assignment for counter: 0=X axis        
                        for (int i=0; i<channels; ++i) // multi channel wave -- test on "X"
                                dsp_aap.channel_mapping[i] = long_2_sranger_long (DSPMoverClass->mover_param.wave_out_channel_dsp[i]);
                        break;
                case DSP_CMD_AFM_MOV_YM:
                case DSP_CMD_AFM_MOV_YP:
                        dsp_aap.axis = int_2_sranger_int (1); // arbitrary assignment for counter: 1=Y axis        
                        for (int i=0; i<channels; ++i)
                                dsp_aap.channel_mapping[i] = long_2_sranger_long (DSPMoverClass->mover_param.wave_out_channel_dsp[i]);
                        break;                
                case DSP_CMD_AFM_MOV_ZM:
                case DSP_CMD_AFM_MOV_ZP:
                        dsp_aap.axis = int_2_sranger_int (2); // arbitrary assignment for counter: 2=Z axis      
                        for (int i=0; i<channels; ++i) 
                                dsp_aap.channel_mapping[i] = long_2_sranger_long (DSPMoverClass->mover_param.wave_out_channel_dsp[i]);
                        break;
		}
		// ... [0..5] (configure all needed channels!)

		dsp_aap.mover_mode = int_2_sranger_int (AAP_MOVER_WAVE_PLAY);
                //DSPMoverClass->mover_param.MOV_mode 
		//DSPMoverClass->mover_param.MOV_output 
                //DSPMoverClass->mover_param.inch_worm_phase > 0. ? AAP_MOVER_IWMODE : 0

		dsp_aap.max_wave_cycles = int_2_sranger_int (2*(int)DSPMoverClass->mover_param.AFM_Steps);     /* max number of repetitions */
		dsp_aap.wave_length     = int_2_sranger_int (DSPMoverClass->mover_param.MOV_wave_len); /* Length of Waveform -- total count all samples/channels */
		dsp_aap.wave_speed      = int_2_sranger_int (DSPMoverClass->mover_param.MOV_wave_speed_fac);     /* Wave Speed (hold number per step) */

                dsp_aap.n_wait      = int_2_sranger_int (2);  /* delay inbetween cycels */
		dsp_aap.ci_retract  = float_2_sranger_q15 (0); // just clear -- CI setting for reversing Z 
                // counter setup -- optional, not essential for stepping, only for keeping a count for up to 3 axis
                // set above
		// dsp_aap.axis = int_2_sranger_int (2); // arbitrary assignmet for counter: 2=Z axis
		// dsp_aap.dir  = int_2_sranger_int (1); // arbitrary direction, assume +1 for approaching direction -- only for counter increment/decement

		lseek (dsp, magic_data.autoapproach, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_aap,  MAX_WRITE_AUTOAPPROACH<<1); 
		break;
	}

	case DSP_CMD_Z0_STOP:
	case DSP_CMD_Z0_P:
	case DSP_CMD_Z0_M:
	case DSP_CMD_Z0_AUTO:
	case DSP_CMD_Z0_CENTER:
	case DSP_CMD_Z0_GOTO:
	{
		static MOVE_OFFSET dsp_move;
		const double fract = 1<<16;
		// DIG / cycle * FRACT
		double mvspd_z = 0.;
		double steps = 0.;
		double delta = 0.;
		lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp, &dsp_move, sizeof (dsp_move)); 
		
		dsp_move.start = int_2_sranger_int (MODE_ZOFFSET_MOVE);
		
		switch (Cmd){
		case DSP_CMD_Z0_STOP: break;
		case DSP_CMD_Z0_P:
			mvspd_z = fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_speed) / DSPControlClass->frq_ref;
			steps =  fabs (fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_adjust) / mvspd_z);
			break;
		case DSP_CMD_Z0_M:
			mvspd_z = -fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_speed) / DSPControlClass->frq_ref;
			steps =  fabs (fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_adjust) / mvspd_z);
			break;
		case DSP_CMD_Z0_AUTO:
			dsp_move.start = int_2_sranger_int (MODE_ZOFFSET_AUTO_ADJUST);
			mvspd_z = fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_speed) / DSPControlClass->frq_ref;
			steps =  3.*fabs (fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_adjust) / mvspd_z);
			break;
		case DSP_CMD_Z0_CENTER:
			mvspd_z = fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_speed) / DSPControlClass->frq_ref;
			CONV_32 (dsp_move.xyz_vec[i_Z]);
			steps =  fabs ((double)dsp_move.xyz_vec[i_Z] / mvspd_z);
			mvspd_z = fabs (mvspd_z) * (dsp_move.xyz_vec[i_Z] > 0 ? -1.:1.);
			break;
		case DSP_CMD_Z0_GOTO:
			mvspd_z = fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_speed) / DSPControlClass->frq_ref;
			CONV_32 (dsp_move.xyz_vec[i_Z]);
			delta = fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_goto) - (double)dsp_move.xyz_vec[i_Z];
			steps =  fabs (delta / mvspd_z);
			mvspd_z = fabs (mvspd_z) * (delta > 0 ? 1.:-1.);
			break;
		}

		dsp_move.f_d_xyz_vec[i_Z] = long_2_sranger_long ((long)round(mvspd_z));
		dsp_move.num_steps = long_2_sranger_long ((long)round(steps));
		
		lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_move, MAX_WRITE_MOVE<<1);	

		break;
	}
	case DSP_CMD_GPIO_SETUP:
	{	if (DSPMoverClass->mover_param.GPIO_direction) {
			static CR_OUT_PULSE dsp_puls;
			static CR_GENERIC_IO dsp_gpio;
			int pper = (int)round ( DSPControlClass->frq_ref*2.*DSPMoverClass->mover_param.AFM_Speed*1e-3 );
			int pon  = pper / 8 + 1; /* fixed on-off ratio */
			dsp_puls.start = int_2_sranger_int(0); // SETUP PARAMS -- just in case
			dsp_puls.stop  = int_2_sranger_int(1);
			dsp_puls.period   = int_2_sranger_int(pper);
			dsp_puls.duration = int_2_sranger_int(pon);
			dsp_puls.number   = int_2_sranger_int((int)DSPMoverClass->mover_param.AFM_Steps);     /* max number of repetitions */
			dsp_puls.on_bcode = uint_2_sranger_uint((int)(DSPMoverClass->mover_param.GPIO_on 
								   | DSPMoverClass->mover_param.AFM_GPIO_setting));     /* GPIO puls on bcode | GPIO_setting */
			dsp_puls.off_bcode = uint_2_sranger_uint((int)(DSPMoverClass->mover_param.GPIO_off 
								   | DSPMoverClass->mover_param.AFM_GPIO_setting));     /* GPIO puls off bcode | GPIO_setting */
			dsp_puls.reset_bcode = uint_2_sranger_uint((int)(DSPMoverClass->mover_param.GPIO_reset 
								     | DSPMoverClass->mover_param.AFM_GPIO_setting));     /* GPIO puls reset bcode | GPIO_setting */
			
			lseek (dsp, magic_data.CR_out_puls, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
			sr_write (dsp, &dsp_puls, MAX_WRITE_CR_OUT_PULS<<1); 


			dsp_gpio.start = int_2_sranger_int(2); // READ GPIO
			dsp_gpio.stop  = int_2_sranger_int(0);
			lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
			sr_write (dsp, &dsp_gpio, 2<<1); 

			lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
			sr_read (dsp, &dsp_gpio, sizeof (dsp_gpio)); 

			// FIXED!!! -- use CONV_U16. WARNING: fix me !! issue w signed/unsigned bit 16 will not work rigth!!
			CONV_U16 (dsp_gpio.gpio_data_out);
			CONV_U16 (dsp_gpio.gpio_data_in);
			CONV_U16 (dsp_gpio.gpio_direction_bits);
			gpio_monitor_out = dsp_gpio.gpio_data_out & DSPMoverClass->mover_param.GPIO_direction;
			gpio_monitor_in  = dsp_gpio.gpio_data_in  & (~DSPMoverClass->mover_param.GPIO_direction);
			gpio_monitor_dir = dsp_gpio.gpio_direction_bits;

			if (dsp_gpio.gpio_data_out != (DSPMoverClass->mover_param.GPIO_reset | DSPMoverClass->mover_param.AFM_GPIO_setting)) {

//				std::cout << "GPIO ist   = " << std::hex << dsp_gpio.gpio_data_out << " ::M= " << gpio_monitor_out << std::dec << std::endl;
//				std::cout << "GPIO update= " << std::hex << DSPMoverClass->mover_param.AFM_GPIO_setting << std::dec << std::endl;
//				std::cout << "GPIO dir   = " << std::hex << DSPMoverClass->mover_param.GPIO_direction << std::dec << std::endl;

				dsp_gpio.start = int_2_sranger_int(3); // (RE)CONFIGURE GPIO DIRECTION
				dsp_gpio.stop  = int_2_sranger_int(0);
				dsp_gpio.gpio_direction_bits = uint_2_sranger_uint(0);
				dsp_gpio.gpio_data_in  = uint_2_sranger_uint(0);
				dsp_gpio.gpio_data_out = uint_2_sranger_uint((DSPMoverClass->mover_param.GPIO_reset 
									      | DSPMoverClass->mover_param.AFM_GPIO_setting));
				dsp_gpio.gpio_direction_bits = uint_2_sranger_uint(DSPMoverClass->mover_param.GPIO_direction);
				lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
				sr_write (dsp, &dsp_gpio, 5<<1); 

				usleep ((useconds_t) (5000) ); // give some time to settle IO/Relais/etc...
				
				dsp_gpio.start = int_2_sranger_int(1); // WRITE GPIO
				dsp_gpio.stop  = int_2_sranger_int(0);
				dsp_gpio.gpio_direction_bits = uint_2_sranger_uint(0);
				dsp_gpio.gpio_data_in  = uint_2_sranger_uint(0);
				dsp_gpio.gpio_data_out = uint_2_sranger_uint((DSPMoverClass->mover_param.GPIO_reset 
									      | DSPMoverClass->mover_param.AFM_GPIO_setting));
				dsp_gpio.gpio_direction_bits = uint_2_sranger_uint(DSPMoverClass->mover_param.GPIO_direction);
				lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
				sr_write (dsp, &dsp_gpio, 5<<1); 

				CONV_U16 (dsp_gpio.gpio_data_out);
				gpio_monitor_out = dsp_gpio.gpio_data_out;
				
				usleep ((useconds_t) (500 * DSPMoverClass->mover_param.GPIO_delay) ); // give some time to settle IO/Relais/etc...
				usleep ((useconds_t) (500 * DSPMoverClass->mover_param.GPIO_delay) ); // ... delay is in ms
			}
		}
		break;
	}

//		case DSP_CMD_:
//				break;
	default: break;
	}
}

void sranger_mk2_hwi_spm::reset_scandata_fifo(int stall){
	static DATA_FIFO dsp_fifo;
	// reset DSP fifo read position
	dsp_fifo.r_position = int_2_sranger_int(0);
	dsp_fifo.w_position = int_2_sranger_int(0);
	dsp_fifo.fill  = int_2_sranger_int(0);
	dsp_fifo.stall = int_2_sranger_int(stall); // unlock (0) or lock (1) scan now
	lseek (dsp, magic_data.datafifo, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_fifo, (MAX_WRITE_DATA_FIFO+3)<<1);
}

void sranger_mk2_hwi_spm::tip_to_origin(double x, double y){
        AREA_SCAN dsp_scan;
        PROBE dsp_probe;

        // make sure no conflicts
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
        sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
        CONV_32 (dsp_scan.pflg);
        if (dsp_scan.pflg){
                gapp->monitorcontrol->LogEvent ("MovetoSXY", "tip_to_origin is busy (scanning/moving in progress): skipping.", 3);
                g_warning ("sranger_mk2_hwi_spm::tip_to_origin -- scanning!  [%x] -- skipping.", dsp_scan.pflg);
                return;
        }
        
	lseek (dsp, magic_data.probe, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
        sr_read  (dsp, &dsp_probe, sizeof (dsp_probe));
        CONV_32 (dsp_probe.pflg);
        if (dsp_probe.pflg){
                gapp->monitorcontrol->LogEvent ("MovetoSXY", "tip_to_origin is busy (probe active): skipping.", 3);
                g_warning ("sranger_mk2_hwi_spm::tip_to_origin -- probe active!  [%x] -- skipping.", dsp_probe.pflg);
                return;
        }

	// get current position
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));

	reset_scandata_fifo ();

	// move tip from current position to Origin i.e. x,y
	dsp_scan.xyz_vec[i_X] = long_2_sranger_long (dsp_scan.xyz_vec[i_X]);
	dsp_scan.xyz_vec[i_Y] = long_2_sranger_long (dsp_scan.xyz_vec[i_Y]);
	SRANGER_DEBUG("SR:EndScan2D last XYPos: " << (dsp_scan.xyz_vec[i_X]>>16) << ", " << (dsp_scan.xyz_vec[i_Y]>>16));

	double Mdx = x - (double)dsp_scan.xyz_vec[i_X];
	double Mdy = y - (double)dsp_scan.xyz_vec[i_Y];
	double mvspd = (1<<16) * sranger_mk2_hwi_pi.app->xsm->Inst->XA2Dig (DSPControlClass->move_speed_x) 
		/ DSPControlClass->frq_ref;
	double steps = round (sqrt (Mdx*Mdx + Mdy*Mdy) / mvspd);
	dsp_scan.fm_dx = (long)round(Mdx/steps);
	dsp_scan.fm_dy = (long)round(Mdy/steps);
	dsp_scan.num_steps_move_xy = (long)steps;

	double zx_ratio = sranger_mk2_hwi_pi.app->xsm->Inst->Dig2XA (1) / sranger_mk2_hwi_pi.app->xsm->Inst->Dig2ZA (1);
	double zy_ratio = sranger_mk2_hwi_pi.app->xsm->Inst->Dig2YA (1) / sranger_mk2_hwi_pi.app->xsm->Inst->Dig2ZA (1);

	// obsolete --
	dsp_scan.fm_dzxy = long_2_sranger_long ((long)round (zx_ratio * (double)dsp_scan.fm_dx * DSPControlClass->area_slope_x
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
	dsp_scan.xyz_vec[i_X] = long_2_sranger_long (dsp_scan.xyz_vec[i_X]);
	dsp_scan.xyz_vec[i_Y] = long_2_sranger_long (dsp_scan.xyz_vec[i_Y]);

	// initiate "return to origin" "dummy" scan now
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_scan, (MAX_WRITE_SCAN)<<1);

	// verify Pos,
	// wait until ready
        int i=250;
	do {
		usleep (20000); // release cpu time
		CallIdleFunc ();
		// pop all remaining events
		DSPControlClass->Probing_eventcheck_callback (NULL, DSPControlClass);

		sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
	} while (dsp_scan.pflg && --i); // check complete or timeout ~5s

        if (!i)
                g_warning ("sranger_mk2_hwi_spm::tip_to_origin -- tip move timeout 5s exceeded.");

	dsp_scan.xyz_vec[i_X] = long_2_sranger_long (dsp_scan.xyz_vec[i_X]);
	dsp_scan.xyz_vec[i_Y] = long_2_sranger_long (dsp_scan.xyz_vec[i_Y]);
	SRANGER_DEBUG("SR:EndScan2D return XYPos: " << (dsp_scan.xyz_vec[i_X]>>16) << ", " << (dsp_scan.xyz_vec[i_Y]>>16));
}

// just note that we are scanning next...
void sranger_mk2_hwi_spm::StartScan2D(){
	DSPControlClass->StartScanPreCheck ();
	ScanningFlg=1; 
	KillFlg=FALSE; // if this gets TRUE while scanning, you should stopp it!!
}

// and its done now!
void sranger_mk2_hwi_spm::EndScan2D(){ 
	static AREA_SCAN dsp_scan;
	ScanningFlg=0; 
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
	CONV_16 (dsp_scan.pflg);
	// cancel scanning?
	if (dsp_scan.pflg) {
		dsp_scan.start = int_2_sranger_int(0);
		dsp_scan.stop  = int_2_sranger_int(1);
		dsp_scan.raster_b = int_2_sranger_int (0);
		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_scan, MAX_WRITE_SCAN<<1);
	}
	// wait until ready
        gint i=200;
	do {
		usleep (20000); // release cpu time
		CallIdleFunc ();
		// pop all remaining events
		DSPControlClass->Probing_eventcheck_callback (NULL, DSPControlClass);

		sr_read (dsp, &dsp_scan, sizeof (dsp_scan));
		CONV_16 (dsp_scan.pflg);
	} while (dsp_scan.pflg && --i); // check complete and timeout ~4s

        if (!i)
                g_warning ("sranger_mk2_hwi_spm::EndScan2D -- timeout 4s exceeded.");

	// do return to center?
	if (DSPControlClass->center_return_flag){
                g_message ("sranger_mk2_hwi_spm::EndScan2D -- tip to orign/manual scan position [dig:%10.3f, %10.3f].", tip_pos[0]/(1<<16), tip_pos[1]/(1<<16));
                tip_to_origin (tip_pos[0], tip_pos[1]);
        }
}

// we are paused
void sranger_mk2_hwi_spm::PauseScan2D(){
	static AREA_SCAN dsp_scan;
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
	CONV_16 (dsp_scan.pflg);

	// pause -- if scanning
	if (dsp_scan.pflg == 1) {
		dsp_scan.start = int_2_sranger_int(0);
		dsp_scan.stop  = int_2_sranger_int(2); // PAUSE SCAN
		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_scan, 2<<1); // only start/stop!
	}
//	ScanningFlg=1;
}

// and its going againg
void sranger_mk2_hwi_spm::ResumeScan2D(){
	static AREA_SCAN dsp_scan;
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
	CONV_16 (dsp_scan.pflg);

	// resume scanning if paused
	if (dsp_scan.pflg == 2) { // only if PAUSEd previously!
		dsp_scan.start = int_2_sranger_int(0);
		dsp_scan.stop  = int_2_sranger_int(4); // RESUME SCAN FROM PAUSE
		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_scan, 2<<1); // only start/stop!
	}
//	ScanningFlg=1;
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void sranger_mk2_hwi_spm::SetOffset(double x, double y){
	static double old_x=123456789., old_y=123456789.;
	static MOVE_OFFSET dsp_move;
	double dx,dy,steps;
	const double fract = 1<<16;

	if (DSPControlClass->ldc_flag) return; // ignore if any LDC (Linear Offset Compensation is in active!) 

	if (old_x == x && old_y == y) return;

	SRANGER_DEBUG("SetOffset: " << x << ", " << y);
	old_x = x; old_y = y;
	
#if 1
	lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read (dsp, &dsp_move, sizeof (dsp_move)); 
#else
	do {
		lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read (dsp, &dsp_move, sizeof (dsp_move)); 
		if (int_2_sranger_int (dsp_move.pflg)){
			if (long_2_sranger_long (dsp_move.num_steps)){
				usleep (50000);
			}
		}
//		printf("\nMOVE:\n");
//		dumpbuffer ((unsigned char*)(&dsp_move), sizeof (dsp_move), 0);
//		printf("X,Ynew       %04x,     %04x\n", int_2_sranger_int (dsp_move.Xnew), int_2_sranger_int (dsp_move.Ynew));
//		printf("f_dx,y   %08x, %08x\n", long_2_sranger_long (dsp_move.f_dx), long_2_sranger_long (dsp_move.f_d_xyz_vec[i_Y]));
//		printf("#steps   %08x\n", long_2_sranger_long (dsp_move.num_steps));
//		printf("X,Ypos   %08x, %08x\n", long_2_sranger_long (dsp_move.xyz_vec[i_X]), long_2_sranger_long (dsp_move.xyz_vec[i_Y]));

	} while (int_2_sranger_int (dsp_move.pflg) && long_2_sranger_long (dsp_move.num_steps) > 0); // wait if there is a move in progress
#endif

	dsp_move.start = int_2_sranger_int (1);
	dsp_move.xy_new_vec[i_X]  = long_2_sranger_long ((long)round(x*(1<<16)));
	dsp_move.xy_new_vec[i_Y]  = long_2_sranger_long ((long)round(y*(1<<16)));

//      compute difference from current to new and necessary steps
	dx = ((double)x * fract) - (double)long_2_sranger_long (dsp_move.xyz_vec[i_X]);
	dy = ((double)y * fract) - (double)long_2_sranger_long (dsp_move.xyz_vec[i_Y]);
	double mvspd = fract * sranger_mk2_hwi_pi.app->xsm->Inst->X0A2Dig (DSPControlClass->move_speed_x) / DSPControlClass->frq_ref;
	steps = round (sqrt (dx*dx + dy*dy) / mvspd);
	dsp_move.f_d_xyz_vec[i_X] = long_2_sranger_long ((long)round(dx/steps));
	dsp_move.f_d_xyz_vec[i_Y] = long_2_sranger_long ((long)round(dy/steps));
	dsp_move.num_steps = long_2_sranger_long ((long)steps);


//	printf("SOLL: f_dx,y   %08x, %08x   #Steps: %08x\n", (long)round(dx/steps), (long)round(dy/steps), (long)steps);
	
	lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_move, MAX_WRITE_MOVE<<1);

}

void sranger_mk2_hwi_spm::MovetoXY(double x, double y){
	static double old_x=0, old_y=0;

	// only if not scan in progress!
	if (ScanningFlg == 0){ 
		if (x != old_x || y != old_y){
                        gchar *tmp=g_strdup_printf ("%10.4f %10.4f requested", x/(1<<16), y/(1<<16));
                        gapp->monitorcontrol->LogEvent ("MovetoSXY", tmp, 3);
                        g_free (tmp);
		        const double Q16 = 1<<16;
			old_x = x;
			old_y = y;
                        tip_pos[0] =  x * Q16;
                        tip_pos[1] =  y * Q16;
			tip_to_origin (tip_pos[0], tip_pos[1]);
		}
	}
}

void sranger_mk2_hwi_spm::set_ldc (double dxdt, double dydt, double dzdt){;
	static MOVE_OFFSET dsp_move;
	const double fract = 1<<16;
	lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read (dsp, &dsp_move, sizeof (dsp_move)); 
	
	dsp_move.start = int_2_sranger_int (2);

	// DIG / cycle * FRACT
	double mvspd_x = fract * sranger_mk2_hwi_pi.app->xsm->Inst->X0A2Dig (dxdt) / DSPControlClass->frq_ref;
	double mvspd_y = fract * sranger_mk2_hwi_pi.app->xsm->Inst->Y0A2Dig (dydt) / DSPControlClass->frq_ref;
	double mvspd_z = fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (dzdt) / DSPControlClass->frq_ref;
	int delay = 1;

	// auto tune precision
	while ((fabs (mvspd_x) > 1e-7 && fabs (mvspd_x) < 256.)
	       || (fabs (mvspd_y) > 1e-7 && fabs (mvspd_y) < 256.)
	       || (fabs (mvspd_z) > 1e-7 && fabs (mvspd_z) < 256.)){
		mvspd_x *= 2.;
		mvspd_y *= 2.;
		mvspd_z *= 2.;
		delay *= 2;
	}
	dsp_move.f_d_xyz_vec[i_X] = long_2_sranger_long ((long)round(mvspd_x));
	dsp_move.f_d_xyz_vec[i_Y] = long_2_sranger_long ((long)round(mvspd_y));
	dsp_move.f_d_xyz_vec[i_Z] = long_2_sranger_long ((long)round(mvspd_z));
	dsp_move.num_steps = long_2_sranger_long ((long)(delay - 1)); // steps - 1 --> one step per cycle
	
	lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
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

void sranger_mk2_hwi_spm::ScanLineM(int yindex, int xdir, int lssrcs, Mem2d *Mob[MAX_SRCS_CHANNELS], int ixy_sub[4]){
	static Mem2d **Mob_dir[4];
	static long srcs_dir[4];
	static int nsrcs_dir[4];
	static int ydir=0;
	static int running = FALSE;
	static AREA_SCAN dsp_scan;
	static double us_per_line;
	
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
		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read (dsp, &dsp_scan, sizeof (dsp_scan));
		// cancel scanning?
		if (dsp_scan.pflg) {
			dsp_scan.start = int_2_sranger_int (0);
			dsp_scan.stop  = int_2_sranger_int (1);
			lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
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
		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read (dsp, &dsp_scan, sizeof (dsp_scan));
		
		gint32 mxx,mxy,myx,myy;
		PI_DEBUG (DBG_L2, "Scan Rotation-Matrix: [[" << rotmxx << ", " << rotmxy << "], [" << rotmyx << ", " << rotmyy << "]]");
		// rotation matrix in Q31
		mxx = long_2_sranger_long (float_2_sranger_q31 (rotmxx));
		mxy = long_2_sranger_long (float_2_sranger_q31 (rotmxy));
		myx = long_2_sranger_long (float_2_sranger_q31 (rotmyx));
		myy = long_2_sranger_long (float_2_sranger_q31 (rotmyy));
		PI_DEBUG (DBG_L9, "ROTM - SR swapped: [[" << std::hex << mxx << ", " << mxy << "], [" << myx << ", " << myy << "]]" << std::dec);
		
		// check for new rotation
		if (dsp_scan.rotm[0] != mxx || dsp_scan.rotm[1] != mxy || dsp_scan.rotm[2] != myx || dsp_scan.rotm[3] != myy){
			// move to origin (rotation invariant point) before any rotation matrix changes!!
                        g_message ("sranger_mk2_hwi_spm::ScanLineM -- setup, rotation matrix changed: tip to orign forced for rotation invariant point.");
			tip_to_origin ();
			// reread position
			lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
			sr_read (dsp, &dsp_scan, sizeof (dsp_scan));
			// set new rotation matrix
			dsp_scan.rotm[0] = mxx;
			dsp_scan.rotm[1] = mxy;
			dsp_scan.rotm[2] = myx;
			dsp_scan.rotm[3] = myy;
		}
		
		// convert
		dsp_scan.xyz_vec[i_X] = long_2_sranger_long (dsp_scan.xyz_vec[i_X]);
		dsp_scan.xyz_vec[i_Y] = long_2_sranger_long (dsp_scan.xyz_vec[i_Y]);
		
		SRANGER_DEBUG("SR:Start/Rot XYPos: " << (dsp_scan.xyz_vec[i_X]>>16) << ", " << (dsp_scan.xyz_vec[i_Y]>>16));

		// setup scan, added forward_slow_down mode
#if 0
		if (GTK_TOGGLE_BUTTON(DSPControlClass->FastScan_status)->active) 
			dsp_scan.start = int_2_sranger_int (2);
		else
			dsp_scan.start = int_2_sranger_int (1);
#endif
                if (DSPControlClass->scan_forward_slow_down > 1)
                        dsp_scan.start = int_2_sranger_int (DSPControlClass->scan_forward_slow_down);
                else
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
		dsp_scan.Zoff_2nd_xp  = int_2_sranger_int (sranger_mk2_hwi_pi.app->xsm->Inst->ZA2Dig (DSPControlClass->x2nd_Zoff)); // init to zero, set later  x2nd_Zoff
		dsp_scan.Zoff_2nd_xm  = int_2_sranger_int (sranger_mk2_hwi_pi.app->xsm->Inst->ZA2Dig (DSPControlClass->x2nd_Zoff)); // init to zero, set later  x2nd_Zoff

		// enable probe?
		if (DSPControlClass->Source && DSPControlClass->probe_trigger_raster_points){
			if (DSPControlClass->Source && DSPControlClass->probe_trigger_raster_points > 0){
				dsp_scan.dnx_probe = int_2_sranger_int (DSPControlClass->probe_trigger_raster_points);
				dsp_scan.raster_a = int_2_sranger_int ((int)(1.+ceil((DSPControlClass->probe_trigger_raster_points-1.)/2)));
//				dsp_scan.raster_b = int_2_sranger_int ((int)(1.+floor((DSPControlClass->probe_trigger_raster_points-1.)/2)));
				dsp_scan.raster_b = int_2_sranger_int (DSPControlClass->probe_and_wait);
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
	
		recalculate_dsp_scan_speed_parameters (); // adjusts dsp_scan.dnx, ....!

		// some thing wicked is going on -- getting screwed valued returned in "dsp_scan.fs_dx & dy"
		PI_DEBUG_GP (DBG_L4, "twFSX*** %d \n", tmp_fs_dx);
		PI_DEBUG_GP (DBG_L4, "twFSY*** %d \n", tmp_fs_dy);
		PI_DEBUG_GP (DBG_L4, "twDNX*** %d \n", tmp_dnx);
		PI_DEBUG_GP (DBG_L4, "twDNY*** %d \n", tmp_dny);
		PI_DEBUG_GP (DBG_L4, "twNXP*** %d \n", tmp_nx_pre);
		
		dsp_scan.fs_dx  = (DSP_LONG)tmp_fs_dx;
		dsp_scan.fs_dy  = (DSP_LONG)tmp_fs_dy;
		dsp_scan.dnx    = (DSP_INT)tmp_dnx;
		dsp_scan.dny    = (DSP_INT)tmp_dny;
		dsp_scan.nx_pre = (DSP_INT)tmp_nx_pre;
		// ----

		PI_DEBUG_GP (DBG_L4, "FSX*** %d \n", dsp_scan.fs_dx);
		PI_DEBUG_GP (DBG_L4, "FSY*** %d \n", dsp_scan.fs_dy);
		PI_DEBUG_GP (DBG_L4, "DNX*** %d \n", dsp_scan.dnx);
		PI_DEBUG_GP (DBG_L4, "DNY*** %d \n", dsp_scan.dny);
		PI_DEBUG_GP (DBG_L4, "NXP*** %d \n", dsp_scan.nx_pre);

		const double fract = 1<<16;
		// from current position to Origin/Start
		// init .dsp_scan for initial move to scan start pos
		// ++++ plus offset by sub_scan settings via ixy_sub matrix[4] := { xoff, xn, yoff, yn}
		double Mdx = -(((double)Nx+1.)/2.-ixy_sub[0])*dsp_scan.dnx*dsp_scan.fs_dx - (double)dsp_scan.xyz_vec[i_X];
		double Mdy =  (((double)Ny-1.)/2.-ixy_sub[2])*dsp_scan.dny*dsp_scan.fs_dy - (double)dsp_scan.xyz_vec[i_Y];
                subscan_data_y_index_offset = ixy_sub[2];

		//### double Mdx = -(double)(Nx+1)*dsp_scan.dnx*dsp_scan.fs_dx/2. - (double)dsp_scan.xyz_vec[i_X];
		//### double Mdy =  (double)(Ny-1)*dsp_scan.dny*dsp_scan.fs_dy/2. - (double)dsp_scan.xyz_vec[i_Y];
		double mvspd = fract * sranger_mk2_hwi_pi.app->xsm->Inst->XA2Dig (DSPControlClass->move_speed_x) / DSPControlClass->frq_ref;
		double steps = round (sqrt (Mdx*Mdx + Mdy*Mdy) / mvspd);
		dsp_scan.fm_dx = (long)round(Mdx/steps);
		dsp_scan.fm_dy = (long)round(Mdy/steps);
		dsp_scan.num_steps_move_xy = (long)steps;

		recalculate_dsp_scan_slope_parameters (); // adjusts dsp_scan.fs_dx, dsp_scan.fs_dy, dsp_scan.fm_dz0x, dsp_scan.fm_dz0y);
		gapp->xsm->data.s.pixeltime = (double)dsp_scan.dnx/DSPControlClass->frq_ref;

		// convert to DSP
		if (ixy_sub[1] > 0)
                        dsp_scan.nx = int_2_sranger_int (ixy_sub[1]); // num datapoints in X to take
                else
                        dsp_scan.nx = int_2_sranger_int (Nx); // num datapoints in X to take

		if (ixy_sub[3] > 0)
                        dsp_scan.ny = int_2_sranger_int (ixy_sub[3]-1); // num datapoints in Y to take
                else
                        dsp_scan.ny = int_2_sranger_int (Ny-1); // num datapoints in Y to take

		dsp_scan.fs_dx = long_2_sranger_long (dsp_scan.fs_dx);
		dsp_scan.fs_dy = long_2_sranger_long (dsp_scan.fs_dy);
		dsp_scan.dnx = int_2_sranger_int (dsp_scan.dnx);
		dsp_scan.dny = int_2_sranger_int (dsp_scan.dny);
		dsp_scan.nx_pre = int_2_sranger_int(dsp_scan.nx_pre);

		dsp_scan.fm_dx = long_2_sranger_long (dsp_scan.fm_dx);
		dsp_scan.fm_dy = long_2_sranger_long (dsp_scan.fm_dy);
		dsp_scan.num_steps_move_xy = long_2_sranger_long (dsp_scan.num_steps_move_xy);

		dsp_scan.fm_dzxy = long_2_sranger_long (dsp_scan.fm_dzxy);
				
		// initiate scan now, this starts a 2D scan process!!!
		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_scan, (MAX_WRITE_SCAN)<<1); // write ix,iy - only here

		start_fifo_read (yindex, nsrcs_dir[0], nsrcs_dir[1], nsrcs_dir[2], nsrcs_dir[3], Mob_dir[0], Mob_dir[1], Mob_dir[2], Mob_dir[3]);
	}

	// call this while waiting for background data updates on screen...
	//		ReadScanData (yindex, num_srcs, Mob); has moved into the thread now

	us_per_line = dsp_scan.dnx*dsp_scan.nx/DSPControlClass->frq_ref*1e6;
	// wait for data, updated display, data move is done in background by the fifo read thread
	do {
	        usleep ((int)us_per_line < 20000 ? (int)us_per_line : 20000); // release cpu time
		gapp->check_events (); // do not lock
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
