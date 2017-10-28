/* SRanger and Gxsm - Gnome X Scanning Microscopy Project
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * DSP tools for Linux
 *
 * Copyright (C) 1999,2000,2001,2002 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * WWW Home:
 * DSP part:  http://sranger.sf.net
 * Gxsm part: http://gxsm.sf.net
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

#include "FB_spm_statemaschine.h"
#include "FB_spm_analog.h"
#include "dataprocess.h"
#include "ReadWrite_GPIO.h"

#ifdef WATCH_ENABLE
extern WATCH            watch;
#endif

extern SPM_STATEMACHINE state;
extern SPM_PI_FEEDBACK  feedback;
extern FEEDBACK_MIXER   feedback_mixer;
extern ANALOG_VALUES    analog;
extern MOVE_OFFSET      move;
extern AREA_SCAN        scan;
extern PROBE            probe;
extern AUTOAPPROACH     autoapp;
extern CR_OUT_PULSE     CR_out_pulse;
extern CR_GENERIC_IO    CR_generic_io;
extern A810_CONFIG      a810_config;
extern DATA_SYNC_IO     data_sync_io;

extern	void asm_calc_mix_log ();

/*
 *	DSP idle loop, runs for ever, returns never !!!!
 *  ============================================================
 *	Main of the DSP State Maschine
 *      - State Status "STMMode", heartbeat
 *      - manage process commands via state, 
 *        this may change the state
 *      - enable/disable of all tasks
 */

int setpoint_old = 0;
int mix_setpoint_old[4] = {0,0,0,0};

int AIC_stop_mode = 0;
int sleepcount = 0;

void dsp_idle_loop (void){
	int i;
	extern unsigned short FreqDiv;
	extern unsigned short ADCRange;
	extern unsigned short QEP_ON;

	/* configure GPIO all in for startup safety -- this is ans should be default power-up, just to make sure again */
	i = 0x0000; WR_GPIO (GPIO_Dir_0, &i, 1);

	for(;;){	/* forever !!! */

		/* Sate Machines heartbeat... */
		if (state.BLK_count >= state.BLK_Ncount){
			state.BLK_count=0L;
			++state.DSP_time;
			if (++state.DSP_tens == 10){
				state.mode ^= MD_BLK;
				state.DSP_tens = 0;
			}
		}

		/* state change request? */
		if (state.set_mode){
				state.mode |= state.set_mode;
				state.set_mode = 0;
		}
		if (state.clr_mode){
				state.mode &= ~state.clr_mode;
				state.clr_mode = 0;
		}

		/* PID-feedback on/off only via flag MD_PID -- nothing to do here */

		if (feedback_mixer.exec){
			asm_calc_mix_log ();
			feedback_mixer.exec = 0;
		}

		for (i=0; i<4; ++i)
			if (feedback_mixer.setpoint[i] != mix_setpoint_old[i]){
				feedback_mixer.x = feedback_mixer.setpoint[i];
				asm_calc_mix_log ();
				feedback_mixer.setpoint_log[i] = feedback_mixer.lnx;
				mix_setpoint_old[i] = feedback_mixer.setpoint[i];
				WATCH16(i,feedback_mixer.setpoint_log[i]);
			}

		/* Start Offset Moveto ? */
		if (move.start && !autoapp.pflg)
			init_offset_move ();

		/* Start/Stop/Pause/Resume Area Scan ? */
		if (scan.start && !autoapp.pflg)
			init_area_scan ();

		switch (scan.stop){
		case 0: break;
		case 1: scan.stop = 0; scan.pflg = 0; break; // Stop/Cancel/Abort scan
		case 2: scan.stop = 0; scan.pflg = 2; break; // Pause Scan
		case 4: if (scan.pflg == 2) { scan.stop = 0; scan.pflg = 1; } break; // Resume Scan from Pause, else ignore
		default: break;
		}

		/* Start Probe ? */
		if (probe.start && !probe.pflg && !autoapp.pflg){
			init_probe_fifo (); // reset probe fifo!
			init_probe ();
		}
		if (probe.stop)
			stop_probe ();

		/* Start Autoapproach/run Mover ? */
		if (autoapp.start && !probe.pflg && !scan.pflg)
				init_autoapp ();
		if (autoapp.stop)
				stop_autoapp ();

		/* Start CoolRunner IO pulse ? */
		if (CR_out_pulse.start)
			init_CR_out_pulse ();
		if (CR_out_pulse.stop)
			stop_CR_out_pulse ();

		/* Do CoolRunner generic IO ? */
		if (CR_generic_io.start){
			switch (CR_generic_io.start){
			case 1: WR_GPIO (GPIO_Data_0, &CR_generic_io.gpio_data_out, 1); break; // write port
			case 2: WR_GPIO (GPIO_Data_0, &CR_generic_io.gpio_data_in, 0); break; // read port
			case 3: WR_GPIO (GPIO_Dir_0, &CR_generic_io.gpio_direction_bits, 1); break; // reconfigure port
			default: break;
			}
			CR_generic_io.start=0;
		}

		if (data_sync_io.pflg || (CR_generic_io.gpio_direction_bits & 0x0100))
			if (data_sync_io.tick){
				data_sync_io.gpiow_bits =
					(CR_generic_io.gpio_data_out & 0xff)
					| ((data_sync_io.xyit[0] & 1)<<8)
					| ((data_sync_io.xyit[1] & 1)<<9)
					| ((data_sync_io.xyit[2] & 1)<<10);
				WR_GPIO (GPIO_Data_0, &data_sync_io.gpiow_bits, 1); // write port
				data_sync_io.tick=0;
				//data_sync_io.last_xyt[0] = data_sync_io.xyt[0];
			}
		

#ifdef AIC_STOPMODE_ENABLE

		if (state.mode & MD_AIC_STOP){
			if (!AIC_stop_mode){
				AIC_stop_mode = 1;
				sleepcount = 0;

				/* Stop Analog810 -- Out/In may be undefined while stopped */
				stop_Analog810 ();
			}
			if (++sleepcount > 50){
				sleepcount = 0;
				dataprocess();
			}
		} else {
			if (AIC_stop_mode){
				AIC_stop_mode = 0;

				/* ReInit and Start Analog810 */
				FreqDiv   = a810_config.freq_div;     // default: =10 75kHz *** 5 ... 65536  150kHz at N=5   Fs = Clk_in/(N * 200), Clk_in=150MHz
				ADCRange  = a810_config.adc_range;    // default: =0 0: +/-10V, 1:+/-5V
				QEP_ON    = a810_config.qep_on;       // default: =1 (on) manage QEP counters
				start_Analog810 ();
			}
		}
#endif
		
	} /* repeat idle loop forever... */
}
