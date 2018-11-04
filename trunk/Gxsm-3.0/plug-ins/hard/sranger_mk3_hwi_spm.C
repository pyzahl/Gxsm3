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

#include "sranger_mk3_hwi.h"

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
#define	SRANGER_DEBUG(S) XSM_DEBUG (DBG_L4, "sranger_mk3_hwi_spm: " << S )

extern GxsmPlugin sranger_mk2_hwi_pi;
extern DSPControl *DSPControlClass;
extern DSPMoverControl *DSPMoverClass;
extern DSPPACControl *DSPPACClass;

int gpio3_monitor_out = 0;
int gpio3_monitor_in  = 0;
int gpio3_monitor_dir = 0;

/* 
 * Init things
 */

sranger_mk3_hwi_spm::sranger_mk3_hwi_spm():sranger_mk3_hwi_dev(){
	SRANGER_DEBUG("Init Sranger SPM");
	ScanningFlg=0;
        tip_pos [0] = tip_pos[1] = 0.;
}

/*
 * Clean up
 */

sranger_mk3_hwi_spm::~sranger_mk3_hwi_spm(){
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

#if 0
#define CONV_16(X) X = int_2_sranger_int (X)
#define CONV_U16(X) X = uint_2_sranger_uint (X)
#define CONV_32(X) X = long_2_sranger_long (X)
#else
#define CONV_16(X) 
#define CONV_U16(X)
#define CONV_32(X) 
#endif

/*
 Real-Time Query of DSP signals/values, auto buffered
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

gint sranger_mk3_hwi_spm::RTQuery (const gchar *property, double &val1, double &val2, double &val3){
        const gint64 max_age = 20000; // 20ms
        static gint64 time_of_last_reading = 0; // abs time in us
	static gint init=FALSE; 
	static SIGNAL_MONITOR_MK3 dsp_sig_mon;
	static AREA_SCAN_MK3 dsp_scan;
	static PROBE_MK3 dsp_probe;
	static SPM_STATEMACHINE_MK3 dsp_statemachine;
	static gint ok=FALSE;
	static MOVE_OFFSET_MK3 dsp_move;
	static AUTOAPPROACH_MK3 dsp_autoapp;
        
	static gint         signal_monitor_index[] = { 0, 1, 2, 3,   4, 5, 6,
                                                       11, 12, 13, 14,
                                                       16, 17, 18,
                                                       19, 20, 21,
                                                       -1 };
	// ------------------------------------ Current, ..                        X-Scan   Y-Scan   Z-Scan
	static const gchar *signal_monitor_setup[] = { "MIX IN 0", "MIX IN 1", "MIX IN 2", "MIX IN 3", "Out 3", "Out 4", "Out 5",
                                                       "Z Servo Watch", "PLL Exci Frq LP", "signal AVG-256", "signal RMS-256",
                                                       "X Offset", "Y Offset", "Z Offset",
                                                       "Count Axis 0", "Count Axis 1", "Count Axis 2",
                                                       NULL };
	static gint   signal_id[NUM_MONITOR_SIGNALS];

	if (!init){ // setup signal monitor
		for (int i=0; signal_monitor_index[i] >= 0 && signal_monitor_setup[i]; ++i){
			signal_id [signal_monitor_index[i] ] = lookup_signal_by_name (signal_monitor_setup[i]);
			if (signal_id [signal_monitor_index[i]] >= 0){
				PI_DEBUG_GP (DBG_L1, "Info: RTQuery: setup signal monitor[%d] <== S%d (%s)\n", signal_monitor_index[i], signal_id[ signal_monitor_index[i]], lookup_signal_name_by_index (signal_id[ signal_monitor_index[i]]) );
				change_signal_input (signal_id [signal_monitor_index [i]], signal_monitor_index[i]); // adjust monitor setup for Gxsm needs -- not essential but expected by RTQuery/PanView
			} else {
				PI_DEBUG_GP (DBG_EVER, "Error: RTQuery: setup signal monitor[%d] <== S%d ((%s)) failed.\n", signal_monitor_index[i], signal_id[ signal_monitor_index[i]], signal_monitor_setup[i]);
			}
		}
		init = TRUE;
	}

        if ( (time_of_last_reading+max_age) < g_get_real_time () ){
                // read signal monitor array
                lseek (dsp_alternative, magic_data.signal_monitor, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
                sr_read  (dsp_alternative, &dsp_sig_mon, sizeof (dsp_sig_mon));
                time_of_last_reading = g_get_real_time ();
                
                for (int i=0; i<NUM_MONITOR_SIGNALS; ++i){
                        CONV_32 (dsp_sig_mon.signal[i]);
                }
        }

// get signal monitor signal and convert to defined base unit + GXSM level MaxVolt correction (1.00x or 1.04x)
#define S_MONV(I) (gapp->xsm->Inst->Dig2VoltOut(3276.7) * lookup_signal_scale_by_index (signal_id[I]) * dsp_sig_mon.signal[I])
#define S_MON(I)  (lookup_signal_scale_by_index (signal_id[I]) * dsp_sig_mon.signal[I])

	if (*property == 'z'){
		val1 =  gapp->xsm->Inst->VZ() * S_MONV (6);
		val2 =  gapp->xsm->Inst->VX() * S_MONV (4);
		val3 =  gapp->xsm->Inst->VY() * S_MONV (5);
		
		if (gapp->xsm->Inst->OffsetMode () == OFM_ANALOG_OFFSET_ADDING){
//			val1 +=  gapp->xsm->Inst->VZ0() * S_MONV (18);
			val2 +=  gapp->xsm->Inst->VX0() * S_MONV (16);
			val3 +=  gapp->xsm->Inst->VY0() * S_MONV (17);
		}
		ok=TRUE;
		return TRUE;
	}

	if (*property == 'o' || *property == 'O'){
		// read/convert and return offset
		// NEED to request 'z' property first, then this is valid and up-to-date!!!!
		if (gapp->xsm->Inst->OffsetMode () == OFM_ANALOG_OFFSET_ADDING){
			val1 =  gapp->xsm->Inst->VZ0() * S_MONV (18);
			val2 =  gapp->xsm->Inst->VX0() * S_MONV (16);
			val3 =  gapp->xsm->Inst->VY0() * S_MONV (17);
		} else {
			val1 =  gapp->xsm->Inst->VZ() * S_MONV (18);
			val2 =  gapp->xsm->Inst->VX() * S_MONV (16);
			val3 =  gapp->xsm->Inst->VY() * S_MONV (17);
		}
		
		return ok;
	}

        // ZXY in Angstroem
        if (*property == 'R'){
                // ZXY Volts after Piezoamp -- without analog offset -> Dig -> ZXY in Angstroem
		val1 = gapp->xsm->Inst->V2ZAng (gapp->xsm->Inst->VZ() * S_MONV (6));
		val2 = gapp->xsm->Inst->V2XAng (gapp->xsm->Inst->VX() * S_MONV (4));
                val3 = gapp->xsm->Inst->V2YAng (gapp->xsm->Inst->VY() * S_MONV (5));
		return TRUE;
        }

        if (*property == 'f'){
		// defaul assignment -- may be life adjusted.... FIX!!!
		
                // signal_id [12] = query_module_signal_input (12); // update real time -- TDB, expensive
		double qf = S_MON (12);
                int I=12;
                /*
                std::cout << "S12 = ["
                          << lookup_signal_name_by_index (signal_id[I])
                          << ", scale= " << lookup_signal_scale_by_index (signal_id[I])
                          << ", number= " << dsp_sig_mon.signal[I]
                          << " value=" << qf << lookup_signal_unit_by_index (signal_id[I])
                          << std::endl;
                */      
                if (qf > 100.)
                        val1 = qf - DSPPACClass->pll.Reference[0];
                else if (qf > 0.)
			val1 = -log (qf) / (2.*M_PI/75000.); // f0 in Hz
		else
			val1 = 75000.; // FBW
//		val2 = gapp->xsm->Inst->Dig2VoltIn (dsp_analog_in.adc[5]) / gapp->xsm->Inst->nAmpere2V(1.); // actual nA reading    xxxx V  * 0.1nA/V
		val2 = S_MON (13) / gapp->xsm->Inst->nAmpere2V(1.); // actual nA reading    xxxx V  * 0.1nA/V
		val3 = sqrt(S_MON (14)) / gapp->xsm->Inst->nAmpere2V(1.); // actual nA RMS reading    xxxx V  * 0.1nA/V
		return TRUE;
	}

	// DSP Status Indicators
	if (*property == 's'){

		lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp, &dsp_statemachine, sizeof (dsp_statemachine)); 
		CONV_32 (dsp_statemachine.mode);
		CONV_32 (dsp_statemachine.DataProcessTime);
		CONV_32 (dsp_statemachine.IdleTime);
		CONV_32 (dsp_statemachine.DataProcessTime_Peak);
		CONV_32 (dsp_statemachine.IdleTime_Peak);

		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp, &dsp_scan, sizeof (dsp_scan)); 
		CONV_32 (dsp_scan.pflg);

		lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read (dsp, &dsp_move, sizeof (dsp_move)); 
		CONV_32 (dsp_move.pflg);

		lseek (dsp, magic_data.probe, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp, &dsp_probe, sizeof (dsp_probe)); 
		CONV_32 (dsp_probe.pflg);

		lseek (dsp, magic_data.autoapproach, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp, &dsp_autoapp, sizeof (dsp_autoapp)); 
		CONV_32 (dsp_autoapp.pflg);

		const int z_servo_watch = 16; // default -- FIX!!!!

		// scan watchdog!
		if (ScanningFlg && dsp_scan.pflg==0) // stop/cancel scan+FIFO-read in progress on Gxsm level if other wise interruped/finsihed FIFO overrun, etc.
			EndScan2D();

		// Bit Coded Status:
		// 0x001: FB watch
		// 0x002,4: AREA-SCAN-MODE scanning, paused
		// 0x008: PROBE
		// 0x010: PLL
		// 0x020: Z_POS ADJUSTER
		val1 = (double)(0
				+ (dsp_sig_mon.signal[11] ? 1:0)
				+ (( dsp_scan.pflg & 3) << 1)
				+ (( dsp_probe.pflg & 1) << 3)
				+ (( dsp_move.pflg & 1) << 4)
				+ ((( dsp_statemachine.mode & MD_PLL)? 1:0) << 5)
				+ ((( dsp_statemachine.mode & MD_ZPOS_ADJUSTER)? 1:0) << 6)
				+ (( dsp_autoapp.pflg & 1) << 7)
			);

// "  DSP Load: %.1f" %(100.*SPM_STATEMACHINE[ii_statemachine_DataProcessTime]/(SPM_STATEMACHINE[ii_statemachine_DataProcessTime]+SPM_STATEMACHINE[ii_statemachine_IdleTime]))
// "  Peak: %.1f" %(100.*SPM_STATEMACHINE[ii_statemachine_DataProcessTime_Peak]/(SPM_STATEMACHINE[ii_statemachine_DataProcessTime_Peak]+SPM_STATEMACHINE[ii_statemachine_IdleTime_Peak]))
		val2 = (double)dsp_statemachine.DataProcessTime / (double)(dsp_statemachine.DataProcessTime + dsp_statemachine.IdleTime); // DSP Load
		val3 = (double)dsp_statemachine.DataProcessTime_Peak / (double)(dsp_statemachine.DataProcessTime_Peak + dsp_statemachine.IdleTime_Peak); // DSP Peak Load

                return TRUE;
	}

        if (*property == 'Z'){
                val1 = sranger_mk2_hwi_pi.app->xsm->Inst->Dig2ZA ((double)dsp_probe.Zpos / (double)(1<<16));
                        
		return TRUE;
        }
        
	// quasi GPIO monitor/mirror -- HIGH LEVEL!!!
	if (*property == 'i'){
#define REALTIME_GPIO_WATCH
#ifdef REALTIME_GPIO_WATCH
		static CR_GENERIC_IO_MK3 dsp_gpio;
		dsp_gpio.start = long_2_sranger_long(2); // MAKE DSP READ GPIO
		dsp_gpio.stop  = long_2_sranger_long(0);
		lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_gpio, 2*sizeof (gint32)); 
		int i=10;
		do {
			lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC); // READ RESULT
			sr_read (dsp, &dsp_gpio, sizeof (dsp_gpio)); 
			CONV_32 (dsp_gpio.start);
			if (i<10) usleep(10000);
		} while (dsp_gpio.start && --i);
		// WARNING: fix me !! issue w signed/unsigned bit 16 will not work rigth!!
		CONV_U16 (dsp_gpio.gpio_data_out);
		CONV_U16 (dsp_gpio.gpio_data_in);
		CONV_U16 (dsp_gpio.gpio_direction_bits);
		gpio3_monitor_out = dsp_gpio.gpio_data_out & DSPMoverClass->mover_param.GPIO_direction;
		gpio3_monitor_in  = dsp_gpio.gpio_data_in  & (~DSPMoverClass->mover_param.GPIO_direction);
		gpio3_monitor_dir = dsp_gpio.gpio_direction_bits;
#endif
		val1 = (double)gpio3_monitor_out;
		val2 = (double)gpio3_monitor_in;
		val3 = (double)gpio3_monitor_dir;
	}

        if (*property == 'A'){
                // Wave/Mover Axis Counts
		val1 = S_MON (19);
		val2 = S_MON (20);
                val3 = S_MON (21);
		return TRUE;
        }

        
//	printf ("ZXY: %g %g %g\n", val1, val2, val3);

//	val1 =  (double)dsp_analog_out.z_scan;
//	val2 =  (double)dsp_analog_out.x_scan;
//	val3 =  (double)dsp_analog_out.y_scan;

	return TRUE;
}

gint  sranger_mk3_hwi_spm::RTQuery (const gchar *property, gchar **val) { 
        /*
        // Query Signal Monitor.... Snn
        int I=12;
        double value = (lookup_signal_scale_by_index (signal_id[I]) * dsp_sig_mon.signal[I]);
        signal_id [12] = query_module_signal_input (12);
        std::cout << "S12 = ["
                  << lookup_signal_name_by_index (signal_id[I])
                  << ", scale= " << lookup_signal_scale_by_index (signal_id[I])
                  << ", number= " << dsp_sig_mon.signal[I]
                  << " value=" << value << lookup_signal_unit_by_index (signal_id[I])
                  << std::endl;
        
        // val1 = value - DSPPACClass->pll.Reference[0];
        */
        return TRUE; 
};

#define DSP32Qs15dot16TOV     (10.0/(32767.*(1<<16)))
#define DSP32Qs15dot16TO_Volt (10.0/(32767.*(1<<16)))
#define DSP32Qs15dot0TO_Volt  (10.0/32767.)
#define DSP32Qs23dot8TO_Volt  (10.0/(32767.*(1<<8)))


gint  sranger_mk3_hwi_spm::RTQuery (const gchar *property, int n, gfloat *data){
        const gint64 max_age = 20000; // 20ms
        static gint64 time_of_last_reading1 = 0; // abs time in us
        static gint64 time_of_last_reading2 = 0; // abs time in us
        static gint64 time_of_last_reading3 = 0; // abs time in us
        static gint64 time_of_last_reading4 = 0; // abs time in us
        static gint64 time_of_last_trg = 0; // abs time in us
        static gint s1ok=0, s2ok=0, s3ok=0, s4ok=0;

        // Trigger
        if ( property[0] == 'T' && (time_of_last_trg+max_age) < g_get_real_time () ){
                time_of_last_trg = g_get_real_time ();
                set_blcklen (n);
                s1ok=-1; s2ok=-1;
        }
        
        // Signal1
        if ( property[1] == '1' && ((time_of_last_reading1+max_age) < g_get_real_time () || s1ok)){
                time_of_last_reading1 = g_get_real_time ();

                double scale =  DSP32Qs23dot8TO_Volt; // assuming MIX_IN_0..3 withe 23Q8 scale for10V
                s1ok=read_pll_signal1 (data, n, scale, 0);
        }
        // Signal2
        if ( property[1] == '2' && ((time_of_last_reading2+max_age) < g_get_real_time () || s2ok)){
                time_of_last_reading2 = g_get_real_time ();

                double scale =  DSP32Qs15dot16TO_Volt;
                s2ok=read_pll_signal2 (data, n, scale, 0);
        }
        // Signal1 deci 256
        if ( property[1] == '3' && ((time_of_last_reading3+max_age) < g_get_real_time () || s3ok)){
                time_of_last_reading3 = g_get_real_time ();

                double scale =  DSP32Qs15dot16TO_Volt;
                s3ok=read_pll_signal1dec (data, n, scale, property[0] == 'R');
        }
        // Signal2 subsampled 256
        if ( property[1] == '4' && ((time_of_last_reading4+max_age) < g_get_real_time () || s4ok)){
                time_of_last_reading4 = g_get_real_time ();

                double scale =  DSP32Qs15dot16TO_Volt;
                s4ok=read_pll_signal2dec (data, n, scale, property[0] == 'R');
        }
        return 0;
}


void sranger_mk3_hwi_spm::SetMode(int mode){
	static SPM_STATEMACHINE_MK3 dsp_state;
	dsp_state.set_mode = long_2_sranger_long(mode);
	dsp_state.clr_mode = long_2_sranger_long(0);
	lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
}

void sranger_mk3_hwi_spm::ClrMode(int mode){
	static SPM_STATEMACHINE_MK3 dsp_state;
	dsp_state.set_mode = long_2_sranger_long(0);
	dsp_state.clr_mode = long_2_sranger_long(mode);
	lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
}

gint sranger_mk3_hwi_spm::SetUserParam (gint n, const gchar *id, double value){
        if (id){
                if (! strcmp (id, "FB_ON"))
                        ExecCmd (DSP_CMD_START);
                else if (! strcmp (id, "FB_OFF"))
                        ExecCmd (DSP_CMD_HALT);
                else if (! strcmp (id, "ZADJ_ON"))
                        ExecCmd (DSP_CMD_ZPOS_ADJUST_ON);
                else if (! strcmp (id, "ZADJ_HOLD"))
                        ExecCmd (DSP_CMD_ZPOS_ADJUST_HOLD);
        }
        return 0;
}


void sranger_mk3_hwi_spm::ExecCmd(int Cmd){
	static int wave_form_address=0;
        static int last_GPIO_BITS=99999;
        
	// query wave form buffer -- shared w probe fifo!!! 
	if (!wave_form_address){
		static DATA_FIFO_EXTERN_MK3 fifo;
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
		static SPM_STATEMACHINE_MK3 dsp_state;
		dsp_state.set_mode = long_2_sranger_long(MD_PID); // enable feedback
		dsp_state.clr_mode = long_2_sranger_long(0);
		lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
		break;
	}
	case DSP_CMD_HALT: // disable Feedback
	{
		static SPM_STATEMACHINE_MK3 dsp_state;
		dsp_state.set_mode = long_2_sranger_long(0);
		dsp_state.clr_mode = long_2_sranger_long(MD_PID);
		lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
		break;
	}
	case DSP_CMD_ZPOS_ADJUST_ON: //
	{
		static SPM_STATEMACHINE_MK3 dsp_state;
		dsp_state.set_mode = long_2_sranger_long(MD_ZPOS_ADJUSTER);
		dsp_state.clr_mode = long_2_sranger_long(0);
		lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
		break;
	}
	case DSP_CMD_ZPOS_ADJUST_HOLD: //
	{
		static SPM_STATEMACHINE_MK3 dsp_state;
		dsp_state.set_mode = long_2_sranger_long(0);
		dsp_state.clr_mode = long_2_sranger_long(MD_ZPOS_ADJUSTER);
		lseek (dsp, magic_data.statemachine, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_state, MAX_WRITE_SPM_STATEMACHINE<<1); 
		break;
	}
	case DSP_CMD_APPROCH_MOV_XP: // auto approch "Mover" -- note ".._XP" is a historic ID (fix me!!) -- Z channel mapping is used
	{
                PI_DEBUG_GP (DBG_L2, "MOVER: Wave Command AutoApp\n");
		static AUTOAPPROACH_MK3 dsp_aap;
		dsp_aap.start = long_2_sranger_long(1);           /* Initiate =WO */
		dsp_aap.stop  = long_2_sranger_long(0);           /* Cancel   =WO */

                // create and download waveforms
                DSPMoverClass->create_waveform (DSPMoverClass->mover_param.AFM_Amp, DSPMoverClass->mover_param.AFM_Speed);
                lseek (dsp, wave_form_address, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
                sr_write (dsp, &DSPMoverClass->mover_param.MOV_waveform[0], DSPMoverClass->mover_param.MOV_wave_len<<1); 
                
                // wave play setup for auto approach
		// configure wave[0,1,...] out channel destination

                int channels=1;
                switch (DSPMoverClass->mover_param.MOV_waveform_id){
                case MOV_WAVE_BESOCKE:
                case MOV_WAVE_SINE: channels = 3; break;
                case MOV_WAVE_STEPPERMOTOR:
                case MOV_WAVE_KOALA: channels = 2; break;
                default: channels = 1; break;
                }
                
		dsp_aap.n_wave_channels    = long_2_sranger_long (channels); /* number wave channels -- up top 6, must match wave data */
                for (int i=0; i<channels; ++i)
                        dsp_aap.channel_mapping[i] = long_2_sranger_long (DSPMoverClass->mover_param.wave_out_channel_xyz[i][2]);

                if (DSPMoverClass->mover_param.MOV_waveform_id == MOV_WAVE_GPIO){
                        g_message ("MK3 AUTO APPROACH START in AAP_MOVER_PULSE mode");
                        dsp_aap.mover_mode = long_2_sranger_long (AAP_MOVER_AUTO_APP | AAP_MOVER_PULSE);
                } else {
                        g_message ("MK3 AUTO APPROACH START in AAP_MOVER_WAVE_PLAY mode");
                        dsp_aap.mover_mode = long_2_sranger_long (AAP_MOVER_AUTO_APP | AAP_MOVER_WAVE_PLAY);
                }
                //DSPMoverClass->mover_param.MOV_mode 
		//DSPMoverClass->mover_param.MOV_output 
                //DSPMoverClass->mover_param.inch_worm_phase > 0. ? AAP_MOVER_IWMODE : 0

		dsp_aap.max_wave_cycles = long_2_sranger_long (2*(int)DSPMoverClass->mover_param.AFM_Steps);     /* max number of repetitions */
		dsp_aap.wave_length     = long_2_sranger_long (DSPMoverClass->mover_param.MOV_wave_len); /* Length of Waveform -- total count all samples/channels */
		dsp_aap.wave_speed      = long_2_sranger_long (DSPMoverClass->mover_param.MOV_wave_speed_fac);     /* Wave Speed (hold number per step) */

                // auto app parameters
		dsp_aap.ci_retract  = float_2_sranger_q31 (0.01 * DSPMoverClass->mover_param.retract_ci / sranger_mk2_hwi_pi.app->xsm->Inst->VZ ()); // CI setting for reversing Z (retract)

                //**** dsp_feedback.ci = float_2_sranger_q31 (0.01 * z_servo[SERVO_CI] / sranger_mk2_hwi_pi.app->xsm->Inst->VZ ());
                dsp_aap.n_wait      = long_2_sranger_long((int)(DSPControlClass->frq_ref*1e-3*DSPMoverClass->mover_param.final_delay));                     /* delay inbetween cycels */
		dsp_aap.n_wait_fin  = long_2_sranger_long((int)(DSPControlClass->frq_ref*1e-3*DSPMoverClass->mover_param.max_settling_time));                     /* delay inbetween cycels */
                // counter setup -- optional, not essential for stepping, only for keeping a count for up to 3 axis
		dsp_aap.axis = int_2_sranger_int (2); // arbitrary assignmet for counter: 2=Z axis
		dsp_aap.dir  = int_2_sranger_int (1); // arbitrary direction, assume +1 for approaching direction -- only for counter increment/decement

		// std::cout << "Auto-Steps: " << dsp_aap.piezo_steps << " Amp=" << DSPMoverClass->mover_param.AFM_Amp << std::endl;
		lseek (dsp, magic_data.autoapproach, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_aap,  MAX_WRITE_AUTOAPPROACH<<1); 
		break;
	}
	case DSP_CMD_APPROCH: // auto approch "Slider"
		break;
	case DSP_CMD_CLR_PA: // Stop all
	{
		static AUTOAPPROACH_MK3 dsp_aap;
                dsp_aap.mover_mode = AAP_MOVER_STOP; /* reset wave 0 and 1 outputs */
		dsp_aap.start = long_2_sranger_long(0);           /* Initiate =WO */
		dsp_aap.stop  = long_2_sranger_long(1);           /* Cancel   =WO */
		lseek (dsp, magic_data.autoapproach, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_aap,  2*sizeof (gint32)); 
#if 0
                dsp_aap.mover_mode = AAP_MOVER_STOP; /* reset wave 0 and 1 outputs */
		dsp_aap.piezo_steps = long_2_sranger_long(1);     /* max number of repetitions */
		dsp_aap.u_piezo_amp = long_2_sranger_long(0); /* Amplitude, Peak2Peak */
		dsp_aap.start = long_2_sranger_long(1);           /* Initiate =WO */
		dsp_aap.stop  = long_2_sranger_long(0);           /* Cancel   =WO */
		lseek (dsp, magic_data.autoapproach, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_aap,  2*sizeof (gint32)); 
		dsp_aap.start = long_2_sranger_long(0);           /* Initiate =WO */
		dsp_aap.stop  = long_2_sranger_long(1);           /* Cancel   =WO */
		lseek (dsp, magic_data.autoapproach, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_aap,  2*sizeof (gint32)); 
#endif
		break;
	}
	case DSP_CMD_AFM_MOV_XM: // manual move X-
	case DSP_CMD_AFM_MOV_XP: // manual move X+
	case DSP_CMD_AFM_MOV_YM: // manual move Y-
	case DSP_CMD_AFM_MOV_YP: // manual move Y+
	case DSP_CMD_AFM_MOV_ZM: // manual move Z-
        case DSP_CMD_AFM_MOV_ZP: // manual move Z+
	{
                PI_DEBUG_GP (DBG_L2, "MOVER: Wave Command %d\n", Cmd);
                

		static AUTOAPPROACH_MK3 dsp_aap;
		dsp_aap.start = long_2_sranger_long(1);           /* Initiate =WO */
		dsp_aap.stop  = long_2_sranger_long(0);           /* Cancel   =WO */
		dsp_aap.n_wait_fin  = long_2_sranger_long (0);    /* just clear, olny autoapp */
                dsp_aap.ci_retract  = float_2_sranger_q31 (0);    /* just clear, olny autoapp */

                // create and download waveform(s) to DSP
                switch (Cmd){
                case DSP_CMD_AFM_MOV_XM:
                case DSP_CMD_AFM_MOV_YM:
                case DSP_CMD_AFM_MOV_ZM:
                        dsp_aap.dir  = int_2_sranger_int (-1); // arbitrary direction, assume -1 "left/up"
                        DSPMoverClass->create_waveform (DSPMoverClass->mover_param.AFM_Amp, -DSPMoverClass->mover_param.AFM_Speed);
                        break;
                case DSP_CMD_AFM_MOV_XP:
                case DSP_CMD_AFM_MOV_YP:
                case DSP_CMD_AFM_MOV_ZP:
                        dsp_aap.dir  = int_2_sranger_int (1); // arbitrary direction, assume +1 "rigth/down"
                        DSPMoverClass->create_waveform (DSPMoverClass->mover_param.AFM_Amp, DSPMoverClass->mover_param.AFM_Speed);
                        break;
                }
                lseek (dsp, wave_form_address, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
                sr_write (dsp, &DSPMoverClass->mover_param.MOV_waveform[0], DSPMoverClass->mover_param.MOV_wave_len<<1); 

                int channels=1;
                switch (DSPMoverClass->mover_param.MOV_waveform_id){
                case MOV_WAVE_BESOCKE:
                case MOV_WAVE_SINE: channels = 3; break;
                case MOV_WAVE_STEPPERMOTOR:
                case MOV_WAVE_KOALA: channels = 2; break;
                default: channels = 1; break;
                }
                
		dsp_aap.n_wave_channels    = long_2_sranger_long (channels); /* number wave channels -- up top 6, must match wave data */

                // wave play setup for auto approach
		// configure wave[0,1,...] out channel destination/mappings. Single wave channel here.
                switch (Cmd){
                case DSP_CMD_AFM_MOV_XM:
                case DSP_CMD_AFM_MOV_XP:
                        dsp_aap.axis = int_2_sranger_int (0); // arbitrary assignment for counter: 0=X axis  *** note so far also used in "rot" and auto folder for L/R buttons -- fixme!
                        for (int i=0; i<channels; ++i)
                                dsp_aap.channel_mapping[i] = long_2_sranger_long (DSPMoverClass->mover_param.wave_out_channel_xyz[i][0]);
                        break;
                case DSP_CMD_AFM_MOV_YM:
                case DSP_CMD_AFM_MOV_YP:
                        dsp_aap.axis = int_2_sranger_int (1); // arbitrary assignment for counter: 1=Y axis
                        for (int i=0; i<channels; ++i)
                                dsp_aap.channel_mapping[i] = long_2_sranger_long (DSPMoverClass->mover_param.wave_out_channel_xyz[i][1]);
                        break;
		case DSP_CMD_AFM_MOV_ZM:
                case DSP_CMD_AFM_MOV_ZP:
                        dsp_aap.axis = int_2_sranger_int (2); // arbitrary assignment for counter: 2=Z axis      
                        for (int i=0; i<channels; ++i)
                                dsp_aap.channel_mapping[i] = long_2_sranger_long (DSPMoverClass->mover_param.wave_out_channel_xyz[i][2]);
                        break;
                }
		// ... [0..5] (configure all needed channels!)

		dsp_aap.mover_mode = long_2_sranger_long (AAP_MOVER_WAVE_PLAY);

                PI_DEBUG_GP (DBG_L2, "max steps=%f \n", 
                                        DSPMoverClass->mover_param.AFM_Steps);
		dsp_aap.max_wave_cycles = long_2_sranger_long (2*(int)DSPMoverClass->mover_param.AFM_Steps);     /* max number of repetitions */
                PI_DEBUG_GP (DBG_L2, "max steps converted for dsp=%d \n", 
                                        dsp_aap.max_wave_cycles);
		dsp_aap.wave_length     = long_2_sranger_long (DSPMoverClass->mover_param.MOV_wave_len); /* Length of Waveform -- total count all samples/channels */
		dsp_aap.wave_speed      = long_2_sranger_long (DSPMoverClass->mover_param.MOV_wave_speed_fac);     /* Wave Speed (hold number per step) */

                // counter setup -- optional, not essential for stepping, only for keeping a count for up to 3 axis
                // setup above!
		// dsp_aap.axis = int_2_sranger_int (2); // arbitrary assignmet for counter: 2=Z axis
		// dsp_aap.dir  = int_2_sranger_int (1); // arbitrary direction, assume +1 for approaching direction -- only for counter increment/decrement

		dsp_aap.n_wait      = long_2_sranger_long (2);                         /* delay inbetween cycels */

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
		static MOVE_OFFSET_MK3 dsp_move;
		const double fract = 1<<16;
		// DIG / cycle * FRACT
		double mvspd_z = 0.;
		double steps = 0.;
		double delta = 0.;
		lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_read  (dsp, &dsp_move, sizeof (dsp_move)); 
		
		dsp_move.start = long_2_sranger_long (MODE_ZOFFSET_MOVE);
		
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
			dsp_move.start = long_2_sranger_long (MODE_ZOFFSET_AUTO_ADJUST);
			mvspd_z = fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_speed) / DSPControlClass->frq_ref;
			steps =  3.*fabs (fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_adjust) / mvspd_z);
			break;
		case DSP_CMD_Z0_CENTER:
			mvspd_z = fract * sranger_mk2_hwi_pi.app->xsm->Inst->Z0A2Dig (DSPMoverClass->Z0_speed) / DSPControlClass->frq_ref;
			CONV_32 (dsp_move.ZPos);
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
			static CR_OUT_PULSE_MK3 dsp_puls;
			static CR_GENERIC_IO_MK3 dsp_gpio;
			int pper = (int)round ( DSPControlClass->frq_ref*2.*DSPMoverClass->mover_param.AFM_Speed*1e-3 );
			int pon  = pper / 8 + 1; /* fixed on-off ratio */
			dsp_puls.start = long_2_sranger_long(0); // SETUP PARAMS -- just in case
			dsp_puls.stop  = long_2_sranger_long(1);
			dsp_puls.period   = long_2_sranger_long(pper);
			dsp_puls.duration = long_2_sranger_long(pon);
			dsp_puls.number   = long_2_sranger_long((int)DSPMoverClass->mover_param.AFM_Steps);     /* max number of repetitions */
			dsp_puls.on_bcode = uint_2_sranger_uint((int)(DSPMoverClass->mover_param.GPIO_on 
								   | DSPMoverClass->mover_param.AFM_GPIO_setting));     /* GPIO puls on bcode | GPIO_setting */
			dsp_puls.off_bcode = uint_2_sranger_uint((int)(DSPMoverClass->mover_param.GPIO_off 
								   | DSPMoverClass->mover_param.AFM_GPIO_setting));     /* GPIO puls off bcode | GPIO_setting */
			dsp_puls.reset_bcode = uint_2_sranger_uint((int)(DSPMoverClass->mover_param.GPIO_reset 
								     | DSPMoverClass->mover_param.AFM_GPIO_setting));     /* GPIO puls reset bcode | GPIO_setting */
			
			lseek (dsp, magic_data.CR_out_puls, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
			sr_write (dsp, &dsp_puls, MAX_WRITE_CR_OUT_PULS<<1); 

                        // GPIO UPDATE

			dsp_gpio.start = long_2_sranger_long(2); // READ GPIO COMMAND
			dsp_gpio.stop  = long_2_sranger_long(0);
			lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
			sr_write (dsp, &dsp_gpio, 4<<1); 

                        usleep ((useconds_t) (500) ); // give some time to read.

			lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
			sr_read (dsp, &dsp_gpio, sizeof (dsp_gpio));  // READ BACK

			CONV_U16 (dsp_gpio.gpio_data_out);
			CONV_U16 (dsp_gpio.gpio_data_in);
			CONV_U16 (dsp_gpio.gpio_direction_bits);
			gpio3_monitor_out = dsp_gpio.gpio_data_out & DSPMoverClass->mover_param.GPIO_direction;
			gpio3_monitor_in  = dsp_gpio.gpio_data_in  & (~DSPMoverClass->mover_param.GPIO_direction);
			gpio3_monitor_dir = dsp_gpio.gpio_direction_bits;

                        g_message ("Checking for GPIO changes...");
                        g_message ("GPIO: dsp_puls.period                       = %d", dsp_puls.period);
                        g_message ("GPIO: dsp_puls.duration                     = %d", dsp_puls.duration);
                        g_message ("GPIO: dsp_puls.number                       = %d", dsp_puls.number);
                        g_message ("GPIO: dsp_puls.on_bcode                     = %02X", dsp_puls.on_bcode);
                        g_message ("GPIO: dsp_puls.off_bcode                    = %02X", dsp_puls.off_bcode);
                        g_message ("GPIO: dsp_puls.reset_bcode                  = %02X", dsp_puls.reset_bcode);
                        g_message ("GPIO: dsp_gpio.gpio_data_out                = %02X", dsp_gpio.gpio_data_out);
                        g_message ("DSPMoverClass->mover_param.GPIO_reset       = %02X", DSPMoverClass->mover_param.GPIO_reset);
                        g_message ("DSPMoverClass->mover_param.AFM_GPIO_setting = %02X", DSPMoverClass->mover_param.AFM_GPIO_setting);

			if (last_GPIO_BITS != DSPMoverClass->mover_param.AFM_GPIO_setting || dsp_gpio.gpio_data_out != (DSPMoverClass->mover_param.GPIO_reset | DSPMoverClass->mover_param.AFM_GPIO_setting)) {
                                last_GPIO_BITS=DSPMoverClass->mover_param.AFM_GPIO_setting;

				// std::cout << "GPIO ist   = " << std::hex << dsp_gpio.gpio_data_out << " ::M= " << gpio3_monitor_out << std::dec << std::endl;
				// std::cout << "GPIO update= " << std::hex << DSPMoverClass->mover_param.AFM_GPIO_setting << std::dec << std::endl;
				// std::cout << "GPIO dir   = " << std::hex << DSPMoverClass->mover_param.GPIO_direction << std::dec << std::endl;

				dsp_gpio.start = long_2_sranger_long (3); // (RE)CONFIGURE GPIO DIRECTION
				dsp_gpio.stop  = long_2_sranger_long (0);
				dsp_gpio.gpio_data_in  = uint_2_sranger_uint (0);
				dsp_gpio.gpio_data_out = uint_2_sranger_uint ((int)(DSPMoverClass->mover_param.GPIO_reset 
										    | DSPMoverClass->mover_param.AFM_GPIO_setting));
				dsp_gpio.gpio_direction_bits = uint_2_sranger_uint (DSPMoverClass->mover_param.GPIO_direction);
				lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
				sr_write (dsp, &dsp_gpio, 8<<1); 

                                g_message ("GPIO: Direction Setup => %02x", dsp_gpio.gpio_direction_bits);
                                
				usleep ((useconds_t) (1000) ); // give some time to settle IO/Relais/etc...
				
				dsp_gpio.start = long_2_sranger_long (1); // WRITE GPIO
				dsp_gpio.stop  = long_2_sranger_long (0);
				dsp_gpio.gpio_direction_bits = uint_2_sranger_uint (0);
				dsp_gpio.gpio_data_in  = uint_2_sranger_uint (0);
				dsp_gpio.gpio_data_out = uint_2_sranger_uint ((int)(DSPMoverClass->mover_param.GPIO_reset 
										  | DSPMoverClass->mover_param.AFM_GPIO_setting));
				dsp_gpio.gpio_direction_bits = uint_2_sranger_uint (DSPMoverClass->mover_param.GPIO_direction);
				lseek (dsp, magic_data.CR_generic_io, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
				sr_write (dsp, &dsp_gpio, 8<<1); 

                                g_message ("GPIO: Pins          => %02x", dsp_gpio.gpio_data_out);
                                
				CONV_U16 (dsp_gpio.gpio_data_out);
				gpio3_monitor_out = dsp_gpio.gpio_data_out;

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

void sranger_mk3_hwi_spm::reset_scandata_fifo(int stall){
	static DATA_FIFO_MK3 dsp_fifo;
	// reset DSP fifo read position
	dsp_fifo.r_position = long_2_sranger_long(0);
	dsp_fifo.w_position = long_2_sranger_long(0);
	dsp_fifo.fill  = long_2_sranger_long(0);
	dsp_fifo.stall = long_2_sranger_long(stall); // unlock (0) or lock (1) scan now
	lseek (dsp, magic_data.datafifo, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_fifo, (MAX_WRITE_DATA_FIFO_SETUP)<<1);
}

void sranger_mk3_hwi_spm::tip_to_origin(double x, double y){
	AREA_SCAN_MK3 dsp_scan;
	PROBE_MK3 dsp_probe;

        if (! RTQuery_clear_to_move_tip ()){
                gapp->monitorcontrol->LogEvent ("MovetoSXY", "Instrument is busy with VP or conflciting task: skipping requst.", 3);
                g_warning ("sranger_mk3_hwi_spm::tip_to_origin -- Instrument is busy with VP or conflciting task. Skipping.");
                return;
        }
        
        // make sure no conflicts
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
        sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
        CONV_32 (dsp_scan.pflg);
        if (dsp_scan.pflg){
                gapp->monitorcontrol->LogEvent ("MovetoSXY", "tip_to_origin is busy (scanning/moving in progress): skipping.", 3);
                g_warning ("sranger_mk3_hwi_spm::tip_to_origin -- scanning/moving (busy)! [%x] -- skipping.", dsp_scan.pflg);
                return;
        }
        
	lseek (dsp, magic_data.probe, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
        sr_read  (dsp, &dsp_probe, sizeof (dsp_probe));
        CONV_32 (dsp_probe.pflg);
        if (dsp_probe.pflg){
                gapp->monitorcontrol->LogEvent ("MovetoSXY", "tip_to_origin is busy (probe active): skipping.", 3);
                g_warning ("sranger_mk3_hwi_spm::tip_to_origin -- probe active! [%x] -- skipping.", dsp_probe.pflg);
                return;
        }

	// get current position
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));

	reset_scandata_fifo ();

	// move tip from current position to Origin i.e. x,y
	dsp_scan.xyz_vec[i_X] = long_2_sranger_long (dsp_scan.xyz_vec[i_X]);
	dsp_scan.xyz_vec[i_Y] = long_2_sranger_long (dsp_scan.xyz_vec[i_Y]);
	SRANGER_DEBUG("SR:SCAN_XY last: " << (dsp_scan.xyz_vec[i_X]>>16) << ", " << (dsp_scan.xyz_vec[i_Y]>>16));
        //gapp->monitorcontrol->LogEvent ("MovetoSXY", "tip_to_origin is busy (probe active): skipping.", 3);

	double Mdx = x - (double)dsp_scan.xyz_vec[i_X];
	double Mdy = y - (double)dsp_scan.xyz_vec[i_Y];
	double mvspd = (1<<16) * sranger_mk2_hwi_pi.app->xsm->Inst->X0A2Dig (DSPControlClass->move_speed_x) 
		/ DSPControlClass->frq_ref;
	double steps = round (sqrt (Mdx*Mdx + Mdy*Mdy) / mvspd);
	dsp_scan.fm_dx = (long)round(Mdx/steps);
	dsp_scan.fm_dy = (long)round(Mdy/steps);
	dsp_scan.num_steps_move_xy = (long)steps;
	SRANGER_DEBUG("SR:SCAN_XY move: dXdY=" << Mdx <<", "<< Mdy << " #steps=" << steps);

	double zx_ratio = sranger_mk2_hwi_pi.app->xsm->Inst->Dig2XA (1) / sranger_mk2_hwi_pi.app->xsm->Inst->Dig2ZA (1);
	double zy_ratio = sranger_mk2_hwi_pi.app->xsm->Inst->Dig2YA (1) / sranger_mk2_hwi_pi.app->xsm->Inst->Dig2ZA (1);
	dsp_scan.fm_dz = 0;

	// setup scan for zero size scan and move to 0,0 in scan coord sys
	dsp_scan.fm_dx = long_2_sranger_long (dsp_scan.fm_dx);
	dsp_scan.fm_dy = long_2_sranger_long (dsp_scan.fm_dy);
	dsp_scan.num_steps_move_xy = long_2_sranger_long (dsp_scan.num_steps_move_xy);

	dsp_scan.start = long_2_sranger_long (-(AREA_SCAN_RUN|AREA_SCAN_START_NORMAL));
	dsp_scan.srcs_xp  = long_2_sranger_long(0);
	dsp_scan.srcs_xm  = long_2_sranger_long(0);
	dsp_scan.srcs_2nd_xp  = long_2_sranger_long (0);
	dsp_scan.srcs_2nd_xm  = long_2_sranger_long (0);
	dsp_scan.dnx_probe = long_2_sranger_long(-1);
	dsp_scan.nx_pre = long_2_sranger_long(0);
	dsp_scan.nx = long_2_sranger_long(0);
	dsp_scan.ny = long_2_sranger_long(0);
	dsp_scan.xyz_vec[i_X] = long_2_sranger_long (dsp_scan.xyz_vec[i_X]);
	dsp_scan.xyz_vec[i_Y] = long_2_sranger_long (dsp_scan.xyz_vec[i_Y]);

	// initiate "return to origin" "dummy" scan now
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_scan, (MAX_WRITE_SCAN)<<1);

	// verify Pos,
	// wait until ready
        gint i=250;
	do {
		usleep (20000); // give some time
		sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
		//		CallIdleFunc ();
	} while (dsp_scan.pflg && --i); // check complete and timeout ~5s

        if (!i)
                g_warning ("sranger_mk3_hwi_spm::tip_to_origin -- tip move timeout 5s exceeded.");

	dsp_scan.xyz_vec[i_X] = long_2_sranger_long (dsp_scan.xyz_vec[i_X]);
	dsp_scan.xyz_vec[i_Y] = long_2_sranger_long (dsp_scan.xyz_vec[i_Y]);
	SRANGER_DEBUG("SR:SCAN_XY fin: " << (dsp_scan.xyz_vec[i_X]>>16) << ", " << (dsp_scan.xyz_vec[i_Y]>>16));
}

// just note that we are scanning next...
void sranger_mk3_hwi_spm::StartScan2D(){
	DSPControlClass->StartScanPreCheck ();
	ScanningFlg=1; 
	KillFlg=FALSE; // if this gets TRUE while scanning, you should stopp it!!
}

// and its done now!
void sranger_mk3_hwi_spm::EndScan2D(){ 
	static AREA_SCAN_MK3 dsp_scan;
	ScanningFlg=0; 
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
	CONV_32 (dsp_scan.pflg);
	// cancel scanning?
	if (dsp_scan.pflg) {
		dsp_scan.start = long_2_sranger_long(0);
		dsp_scan.stop  = long_2_sranger_long(AREA_SCAN_STOP);
		dsp_scan.raster_b = long_2_sranger_long (0);
		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_scan, MAX_WRITE_SCAN<<1);
	}
	// wait until ready
        gint i=200;
	do {
		usleep (20000); // give some time
		sr_read (dsp, &dsp_scan, sizeof (dsp_scan));
		CONV_32 (dsp_scan.pflg);
                gapp->check_events ("Returning tip to home..."); // do not lock [added new]
	} while (dsp_scan.pflg && --i); // check complete and timeout ~4s

        if (!i)
                g_warning ("sranger_mk3_hwi_spm::EndScan2D -- timeout 4s exceeded.");

	// do return to center?
	if (DSPControlClass->center_return_flag){
                g_message ("sranger_mk3_hwi_spm::EndScan2D -- tip to orign/manual scan position [dig:%10.3f, %10.3f].", tip_pos[0]/(1<<16), tip_pos[1]/(1<<16));
                tip_to_origin (tip_pos[0], tip_pos[1]);
        }
}

// we are paused
void sranger_mk3_hwi_spm::PauseScan2D(){
	static AREA_SCAN_MK3 dsp_scan;
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
	CONV_32 (dsp_scan.pflg);

	// pause -- if scanning
	if (dsp_scan.pflg & AREA_SCAN_RUN) {
		dsp_scan.start = long_2_sranger_long(0);
		dsp_scan.stop  = long_2_sranger_long(AREA_SCAN_PAUSE); // PAUSE SCAN
		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_scan, 2*sizeof (gint32)); // only start/stop!
	}
//	ScanningFlg=1;
}

// and its going againg
void sranger_mk3_hwi_spm::ResumeScan2D(){
	static AREA_SCAN_MK3 dsp_scan;
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
	CONV_32 (dsp_scan.pflg);

	// resume scanning if paused
	if (dsp_scan.pflg & (AREA_SCAN_START_NORMAL|AREA_SCAN_START_FASTSCAN)) { // only if PAUSEd previously!
		dsp_scan.start = long_2_sranger_long(0);
		dsp_scan.stop  = long_2_sranger_long(AREA_SCAN_RESUME); // RESUME SCAN FROM PAUSE
		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_scan, 2*sizeof (gint32)); // only start/stop!
	}
//	ScanningFlg=1;
}


void sranger_mk3_hwi_spm::UpdateScanGainMirror (){
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
        // Update XYZ Scan Gains: bitcoded -/8/8/8 (0..255)x -- not yet used and fixed set to 10x (0x000a0a0a) -- also:	DSP_SIG Offset XYZ_gain
        dsp_scan.xyz_gain = long_2_sranger_long ((long)( 0
                                                         | ((((long)round(gapp->xsm->Inst->VX()))&0xff) << 16)
                                                         | ((((long)round(gapp->xsm->Inst->VY()))&0xff) << 8)
                                                         | ((((long)round(gapp->xsm->Inst->VZ()))&0xff))));

	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_scan, (MAX_WRITE_SCAN)<<1);

        lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read (dsp, &dsp_move, sizeof (dsp_move)); 
        // Update XYZ Offset Gains: bitcoded -/8/8/8 (0..255)x -- not yet used and fixed set to 10x (0x000a0a0a) -- also:	DSP_SIG Offset XYZ_gain
        dsp_move.xyz_gain = long_2_sranger_long ((long)( 0
                                                         | ((((long)round(gapp->xsm->Inst->VX0()))&0xff) << 16)
                                                         | ((((long)round(gapp->xsm->Inst->VY0()))&0xff) << 8)
                                                         | ((((long)round(gapp->xsm->Inst->VZ0()))&0xff))));

	lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_move, MAX_WRITE_MOVE<<1);
}

// this does almost the same as the XSM_Hardware base class would do, 
// but you may want to do sth. yourself here
void sranger_mk3_hwi_spm::SetOffset(double x, double y){
	static double old_x=123456789, old_y=123456789;
	static MOVE_OFFSET_MK3 dsp_move;
	double dx,dy,steps;
	const double fract = 1<<16;

	if (DSPControlClass->ldc_flag) return; // ignore if any LDC (Linear Offset Compensation is in active!) 

        double vs=gapp->xsm->Inst->VX()+gapp->xsm->Inst->VY()+gapp->xsm->Inst->VZ();
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
		if (long_2_sranger_long (dsp_move.pflg)){
			if (long_2_sranger_long (dsp_move.num_steps)){
				usleep (50000);
			}
		}

	} while (long_2_sranger_long (dsp_move.pflg) && long_2_sranger_long (dsp_move.num_steps) > 0); // wait if there is a move in progress
#endif
	// convert -- only on i386 necessary
	dsp_move.start = long_2_sranger_long (1);
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

void sranger_mk3_hwi_spm::MovetoXY(double x, double y){
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

/* perform a delta move XYZ aktion within the scan-coordinate system (rotated by alpha) with limits */
void sranger_mk3_hwi_spm::Move_delta_XYZ (double dx, double dy, double dz, Mem2d *Mob[MAX_SRCS_CHANNELS], const gchar *options) {
  	static AREA_SCAN_MK3 dsp_scan;
        static int busy=0;
        
        if (busy)
                return;       

        busy=1;
        
	// get current position
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));

	reset_scandata_fifo ();

	// move tip from current position to Origin i.e. x,y
	dsp_scan.xyz_vec[i_X] = long_2_sranger_long (dsp_scan.xyz_vec[i_X]);
	dsp_scan.xyz_vec[i_Y] = long_2_sranger_long (dsp_scan.xyz_vec[i_Y]);
	SRANGER_DEBUG("SR:EndScan2D last XYPos: " << (dsp_scan.xyz_vec[i_X]>>16) << ", " << (dsp_scan.xyz_vec[i_Y]>>16));

        const double Q16 = 1<<16;
	double Mdx = Q16 * dx;
	double Mdy = Q16 * dy;
	double Mdz = Q16 * dz;
	double mvspd = Q16 * sranger_mk2_hwi_pi.app->xsm->Inst->X0A2Dig (DSPControlClass->move_speed_x) / DSPControlClass->frq_ref;
	double steps = round (sqrt (Mdx*Mdx + Mdy*Mdy + Mdz*Mdz) / mvspd);
	dsp_scan.fm_dx = (long)round(Mdx/steps);
	dsp_scan.fm_dy = (long)round(Mdy/steps);
	dsp_scan.fm_dz = (long)round(Mdz/steps);
	dsp_scan.num_steps_move_xy = (long)steps;

	double zx_ratio = sranger_mk2_hwi_pi.app->xsm->Inst->Dig2XA (1) / sranger_mk2_hwi_pi.app->xsm->Inst->Dig2ZA (1);
	double zy_ratio = sranger_mk2_hwi_pi.app->xsm->Inst->Dig2YA (1) / sranger_mk2_hwi_pi.app->xsm->Inst->Dig2ZA (1);

	// setup scan for zero size scan and move to 0,0 in scan coord sys
	dsp_scan.fm_dx = long_2_sranger_long (dsp_scan.fm_dx);
	dsp_scan.fm_dy = long_2_sranger_long (dsp_scan.fm_dy);
	dsp_scan.fm_dz = long_2_sranger_long (dsp_scan.fm_dz);
	dsp_scan.num_steps_move_xy = long_2_sranger_long (dsp_scan.num_steps_move_xy);

        dsp_scan.start = long_2_sranger_long (-(AREA_SCAN_RUN|AREA_SCAN_START_NORMAL));
        
	dsp_scan.srcs_xp  = long_2_sranger_long(0);
	dsp_scan.srcs_xm  = long_2_sranger_long(0);
	dsp_scan.srcs_2nd_xp  = long_2_sranger_long (0);
	dsp_scan.srcs_2nd_xm  = long_2_sranger_long (0);
	dsp_scan.dnx_probe = long_2_sranger_long(-1);
	dsp_scan.nx_pre = long_2_sranger_long(0);
	dsp_scan.nx = long_2_sranger_long(0); // NO ACTUAL SCAN -- just pre move!
	dsp_scan.ny = long_2_sranger_long(0); // NO ACTUAL SCAN!
	dsp_scan.xyz_vec[i_X] = long_2_sranger_long (dsp_scan.xyz_vec[i_X]);
	dsp_scan.xyz_vec[i_Y] = long_2_sranger_long (dsp_scan.xyz_vec[i_Y]);

	// initiate "return to origin" "dummy" scan now
	lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_write (dsp, &dsp_scan, (MAX_WRITE_SCAN)<<1);

	// verify Pos,
	// wait until ready
        gint i=2000;
	do {
		usleep (5000); // give some time
		sr_read  (dsp, &dsp_scan, sizeof (dsp_scan));
                CallIdleFunc ();
	} while (dsp_scan.pflg && --i); // with timeout ~5s

        if (!i)
                g_warning ("sranger_mk3_hwi_spm::Move_delta_XYZ -- timeout 5s exceeded.");
        
        dsp_scan.xyz_vec[i_X] = long_2_sranger_long (dsp_scan.xyz_vec[i_X]);
        dsp_scan.xyz_vec[i_Y] = long_2_sranger_long (dsp_scan.xyz_vec[i_Y]);

        busy=0;
        
}

void sranger_mk3_hwi_spm::set_ldc (double dxdt, double dydt, double dzdt){;
	static MOVE_OFFSET_MK3 dsp_move;
	const double fract = 1<<16;
	lseek (dsp, magic_data.move, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
	sr_read (dsp, &dsp_move, sizeof (dsp_move)); 
	
	dsp_move.start = long_2_sranger_long (2);

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

void sranger_mk3_hwi_spm::ScanLineM(int yindex, int xdir, int lssrcs, Mem2d *Mob[MAX_SRCS_CHANNELS], int ixy_sub[4]){
	static Mem2d **Mob_dir[4];
	static long srcs_dir[4];
	static int nsrcs_dir[4];
	static int ydir=0;
	static int running = FALSE;
	static AREA_SCAN_MK3 dsp_scan;
	static double us_per_line;
	
	int num_srcs_w = 0; // #words
	int num_srcs_l = 0; // #long words
	int bi = 0;
	// find # of srcs_w (16 bit data, 4-11:ovs32, norm  -> float) 0x0001, 0x0010..0x0800 (Bits 0, 4,5,6,7, 8,9,10,11)
	do{
		if(lssrcs & (1<<bi++))
			++num_srcs_w;
	}while(bi<12);
	// find # of srcs_l (32 bit data -> float ) 0x1000..0x8000 (Bits 12,13,14,15)
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
			dsp_scan.start = long_2_sranger_long (0);
			dsp_scan.stop  = long_2_sranger_long (AREA_SCAN_STOP);
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
		PI_DEBUG (DBG_L2, "Scan Rotation Matrix: [[" << rotmxx << ", " << rotmxy << "], [" << rotmyx << ", " << rotmyy << "]]");
		// rotation matrix in Q31
		mxx = long_2_sranger_long (float_2_sranger_q31 (rotmxx));
		mxy = long_2_sranger_long (float_2_sranger_q31 (rotmxy));
		myx = long_2_sranger_long (float_2_sranger_q31 (rotmyx));
		myy = long_2_sranger_long (float_2_sranger_q31 (rotmyy));
		PI_DEBUG (DBG_L9, "ROTM - SR swapped: [[" << std::hex << mxx << ", " << mxy << "], [" << myx << ", " << myy << "]]" << std::dec );

		// check for new rotation
		if (dsp_scan.rotm[0] != mxx || dsp_scan.rotm[1] != mxy || dsp_scan.rotm[2] != myx || dsp_scan.rotm[3] != myy){
			// move to origin (rotation invariant point) before any rotation matrix changes!!
                        g_message ("sranger_mk3_hwi_spm::ScanLineM -- setup, rotation matrix changed: tip to orign forced for rotation invariant point.");
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

		// setup scan
                if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(DSPControlClass->FastScan_status))) 
			dsp_scan.start = long_2_sranger_long (-(AREA_SCAN_RUN|AREA_SCAN_START_FASTSCAN));
		else
                        if (DSPControlClass->scan_forward_slow_down > 1)
                                dsp_scan.start = long_2_sranger_long (DSPControlClass->scan_forward_slow_down);
                        else
                                dsp_scan.start = long_2_sranger_long (-(AREA_SCAN_RUN|AREA_SCAN_START_NORMAL));
		
		// 1st XP scan here
		dsp_scan.srcs_xp  = long_2_sranger_long (srcs_dir[0]);
		dsp_scan.srcs_xm  = long_2_sranger_long (srcs_dir[1]);
		
		// 2nd scan, setup later, see below
		dsp_scan.srcs_2nd_xp  = long_2_sranger_long (srcs_dir[2]);
		dsp_scan.srcs_2nd_xm  = long_2_sranger_long (srcs_dir[3]);

		// --- DISABLED ---
//		dsp_scan.Zoff_2nd_xp  = long_2_sranger_long (sranger_mk2_hwi_pi.app->xsm->Inst->ZA2Dig (DSPControlClass->x2nd_Zoff)); // init to zero, set later  x2nd_Zoff
//		dsp_scan.Zoff_2nd_xm  = long_2_sranger_long (sranger_mk2_hwi_pi.app->xsm->Inst->ZA2Dig (DSPControlClass->x2nd_Zoff)); // init to zero, set later  x2nd_Zoff

		// enable probe?
		if (DSPControlClass->probe_trigger_raster_points){
			if (DSPControlClass->probe_trigger_raster_points > 0){
				dsp_scan.dnx_probe = long_2_sranger_long (DSPControlClass->probe_trigger_raster_points);
				dsp_scan.raster_a = long_2_sranger_long ((int)(1.+ceil((DSPControlClass->probe_trigger_raster_points-1.)/2)));
//				dsp_scan.raster_b = long_2_sranger_long ((int)(1.+floor((DSPControlClass->probe_trigger_raster_points-1.)/2)));
				dsp_scan.raster_b = long_2_sranger_long (DSPControlClass->probe_and_wait);
			} else {
				dsp_scan.dnx_probe = long_2_sranger_long ((int)fabs((double)DSPControlClass->probe_trigger_raster_points));
				dsp_scan.raster_a = long_2_sranger_long (0);
				dsp_scan.raster_b = long_2_sranger_long (0);
			}
		}
		else{
			dsp_scan.dnx_probe = long_2_sranger_long (-1); // not yet, auto probe trigger
			dsp_scan.raster_a = long_2_sranger_long (0);
			dsp_scan.raster_b = long_2_sranger_long (0);
		}
		
		dsp_scan.nx = Nx; // num datapoints in X to take
		dsp_scan.ny = Ny-1; // num datapoints in Y to take

		ydir = yindex > 0 ? -1 : 1;
	
		// speed
		recalculate_dsp_scan_speed_parameters (); // adjusts dsp_scan.dnx, ....!

		// some thing wicked is going on -- getting screwed valued returned in "dsp_scan.fs_dx & dy"
		PI_DEBUG_GP (DBG_L5, "sc.FSX** %d \n", dsp_scan.fs_dx);
		PI_DEBUG_GP (DBG_L5, "twFSX*** %d \n", tmp_fs_dx);
		PI_DEBUG_GP (DBG_L5, "twFSY*** %d \n", tmp_fs_dy);
		PI_DEBUG_GP (DBG_L5, "twDNX*** %d \n", tmp_dnx);
		PI_DEBUG_GP (DBG_L5, "twDNY*** %d \n", tmp_dny);
		PI_DEBUG_GP (DBG_L5, "twNXP*** %d \n", tmp_nx_pre);
		
		dsp_scan.fs_dx  = (DSP_INT32)tmp_fs_dx;
		dsp_scan.fs_dy  = (DSP_INT32)tmp_fs_dy;
		dsp_scan.dnx    = (DSP_INT32)tmp_dnx;
		dsp_scan.dny    = (DSP_INT32)tmp_dny;
		dsp_scan.nx_pre = (DSP_INT32)tmp_nx_pre;
		dsp_scan.Zoff_2nd_xp = (DSP_INT32)tmp_sRe;
		dsp_scan.Zoff_2nd_xm = (DSP_INT32)tmp_sIm;
		// ----

		PI_DEBUG_GP (DBG_L5, "FSX*** %d \n", dsp_scan.fs_dx);
		PI_DEBUG_GP (DBG_L5, "FSY*** %d \n", dsp_scan.fs_dy);
		PI_DEBUG_GP (DBG_L5, "DNX*** %d \n", dsp_scan.dnx);
		PI_DEBUG_GP (DBG_L5, "DNY*** %d \n", dsp_scan.dny);
		PI_DEBUG_GP (DBG_L5, "Zoff_xp/Re*** %d \n", dsp_scan.Zoff_2nd_xp);
		PI_DEBUG_GP (DBG_L5, "Zoff_xm/Im*** %d \n", dsp_scan.Zoff_2nd_xm);
		PI_DEBUG_GP (DBG_L5, "NXP*** %d \n", dsp_scan.nx_pre);

		// mirror gain -- may be hooked into by Smart Piezo Amp or anything else!
		dsp_scan.xyz_gain = ( 
				     ((((int)round( sranger_mk2_hwi_pi.app->xsm->Inst->VZ ())) & 0xff) << 16 ) |
				     ((((int)round( sranger_mk2_hwi_pi.app->xsm->Inst->VY ())) & 0xff) <<  8 ) |
				     ((((int)round( sranger_mk2_hwi_pi.app->xsm->Inst->VX ())) & 0xff))      );
		
		dsp_move.xyz_gain = ( 
				     ((((int)round( sranger_mk2_hwi_pi.app->xsm->Inst->VZ0 ())) & 0xff) << 16 ) |
				     ((((int)round( sranger_mk2_hwi_pi.app->xsm->Inst->VY0 ())) & 0xff) <<  8 ) |
				     ((((int)round( sranger_mk2_hwi_pi.app->xsm->Inst->VX0 ())) & 0xff))      );
		// not yet transferred to DSP.

		PI_DEBUG_GP (DBG_L5, "XYZ gain code:*Sc 0x%08x  Mv: 0x%08x \n", dsp_scan.xyz_gain, dsp_move.xyz_gain);

		const double fract = 1<<16;
		// from current position to Origin/Start
		// init .dsp_scan for initial move to scan start pos
		// ++++ plus offset by sub_scan settings via ixy_sub matrix[4] := { xoff, xn, yoff, yn}
		double Mdx = -(((double)Nx+1.)/2.-ixy_sub[0])*dsp_scan.dnx*dsp_scan.fs_dx - (double)dsp_scan.xyz_vec[i_X];
		double Mdy =  (((double)Ny-1.)/2.-ixy_sub[2])*dsp_scan.dny*dsp_scan.fs_dy - (double)dsp_scan.xyz_vec[i_Y];
                subscan_data_y_index_offset = ixy_sub[2];
		//### double Mdx = -(double)(Nx+1)*dsp_scan.dnx*dsp_scan.fs_dx/2. - (double)dsp_scan.xyz_vec[i_X];
		//### double Mdy =  (double)(Ny-1)*dsp_scan.dny*dsp_scan.fs_dy/2. - (double)dsp_scan.xyz_vec[i_Y];
		double mvspd = fract * sranger_mk2_hwi_pi.app->xsm->Inst->X0A2Dig (DSPControlClass->move_speed_x) / DSPControlClass->frq_ref;
		double steps = round (sqrt (Mdx*Mdx + Mdy*Mdy) / mvspd);
		dsp_scan.fm_dx = (long)round(Mdx/steps);
		dsp_scan.fm_dy = (long)round(Mdy/steps);
		dsp_scan.num_steps_move_xy = (long)steps;

		// slope
		recalculate_dsp_scan_slope_parameters (); // adjusts dsp_scan.fm_dzx, dsp_scan.fm_dzy, dsp_scan.fm_dzxy, dsp_scan.fs_dx, dsp_scan.fs_dy, dsp_scan.fm_dx, dsp_scan.fm_dy);
		gapp->xsm->data.s.pixeltime = (double)dsp_scan.dnx/DSPControlClass->frq_ref;

		// convert to DSP
		//dsp_scan.nx = long_2_sranger_long (Nx); // num datapoints in X to take
		//dsp_scan.ny = long_2_sranger_long (Ny-1); // num datapoints in Y to take
		if (ixy_sub[1] > 0)
                        dsp_scan.nx = long_2_sranger_long (ixy_sub[1]); // num datapoints in X to take
                else
                        dsp_scan.nx = long_2_sranger_long (Nx); // num datapoints in X to take

		if (ixy_sub[3] > 0)
                        dsp_scan.ny = long_2_sranger_long (ixy_sub[3]-1); // num datapoints in Y to take
                else
                        dsp_scan.ny = long_2_sranger_long (Ny-1); // num datapoints in Y to take

		dsp_scan.fs_dx = long_2_sranger_long (dsp_scan.fs_dx);
		dsp_scan.fs_dy = long_2_sranger_long (dsp_scan.fs_dy);
		dsp_scan.dnx = long_2_sranger_long (dsp_scan.dnx);
		dsp_scan.dny = long_2_sranger_long (dsp_scan.dny);
		dsp_scan.nx_pre = long_2_sranger_long(dsp_scan.nx_pre);

		dsp_scan.fm_dx = long_2_sranger_long (dsp_scan.fm_dx);
		dsp_scan.fm_dy = long_2_sranger_long (dsp_scan.fm_dy);
		dsp_scan.num_steps_move_xy = long_2_sranger_long (dsp_scan.num_steps_move_xy);

		dsp_scan.Zoff_2nd_xp  = long_2_sranger_long (dsp_scan.Zoff_2nd_xp);
		dsp_scan.Zoff_2nd_xm  = long_2_sranger_long (dsp_scan.Zoff_2nd_xm);

		dsp_scan.fm_dz = long_2_sranger_long (dsp_scan.fm_dz);
				
		dsp_scan.xyz_gain = long_2_sranger_long (dsp_scan.xyz_gain);
		dsp_move.xyz_gain = long_2_sranger_long (dsp_move.xyz_gain); // *** not yet moved to DSP, no yet needed -- but in place

		// initiate scan now, this starts a 2D scan process!!!
		lseek (dsp, magic_data.scan, SRANGER_MK23_SEEK_DATA_SPACE | SRANGER_MK23_SEEK_ATOMIC);
		sr_write (dsp, &dsp_scan, (MAX_WRITE_SCAN)<<1); // write ix,iy - only here
		start_fifo_read (yindex, nsrcs_dir[0], nsrcs_dir[1], nsrcs_dir[2], nsrcs_dir[3], Mob_dir[0], Mob_dir[1], Mob_dir[2], Mob_dir[3]);
	}

	// call this while waiting for background data updates on screen...
	//		ReadScanData (yindex, num_srcs, Mob); has moved into the thread now

	us_per_line = dsp_scan.dnx*dsp_scan.nx/(2.*DSPControlClass->frq_ref)*1e6;
	// wait for data, updated display, data move is done in background by the fifo read thread
	do {
	        usleep ((int)us_per_line < 10000 ? (int)us_per_line : 10000); // release cpu time
		gapp->check_events_self (); // do not lock, quiete
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
