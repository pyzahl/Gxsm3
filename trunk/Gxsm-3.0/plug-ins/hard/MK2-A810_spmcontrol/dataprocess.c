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
/* 20040820 CVS fix2 */

/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/*
 *
 *	dataprocess.c
 *
 *	This is the ISR of the DMA. This ISR is called at each new sample.
 */


/* ============================================================
    ** Recorder / Scope functionality **
    from Alex:
    I made the record function for SR2. The execution time depends on the buffers position. 
    For instance, if both buffers are in the SDRAM, it means 44 DSP cycles by write in SDRAM 
    plus the overhead (23 DSP cycles): 88+23=111 cycles.
 
    Here is the definition that you have to add in your C file:
 
   Definition of the record signal function defined in the assembler file
   This function records in Signal1() and Signal2() if blcklen is not equal to -1
   pSignal1 and pSignal2 pointers are used to determine the recorded signals
  ==============================================================
 */


// Definition of the record signal functions defined in the assembler file
// This functions record in Signalx if blcklen16/32 is not equal to -1
// pSignal1 and pSignal2 pointers are used to determine the recorded signals
 
extern void RecSignalsASM16();
extern void RecSignalsASM32();
 

#include "FB_spm_dataexchange.h"
#include "dataprocess.h"  
#include "mul32.h"

/* local used variables for automatic offset compensation */
long    memb[8];        /* 32 bit variable used to calculate the input0 offset*/
int     blcklen;	/* Block length, used for the offset measurement*/
int     IdleTimeTmp;
long    tmpsum;

// RMS buffer
#define RMS_N2           8
#define RMS_N            (1 << RMS_N2)
int     rms_pipi = 0;       /* Pipe index */
int     rms_I_pipe[RMS_N] = 
{ 
	0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0, 
	0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0, 
	0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0, 
	0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
};
long    rms_I2_pipe[RMS_N] = 
{ 
	0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L, 0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L, 
	0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L, 0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L, 
	0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L, 0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L, 
	0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L, 0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0L,  0L,0L,0L,0L, 0L,0L,0L,0
};

// 8x digital sigma-delta over sampling using bit0 of 16 -> gain of 2 bits resoultion (16+2=18)
#define SIGMA_DELTA_LEN   8
int     sigma_delta_index = 0;
long    zpos_xymult = 0L;
long    s_xymult = 0L;
long    s_xymult_prev = 0L;
long    d_tmp=0L;
int     tmp_hrbits = 0;

//#define SIGMA_DELTA_HR_MAKS_FAST
#ifdef SIGMA_DELTA_HR_MAKS_FAST  // Fast HR Matrix -- good idea, but performance/distorsion issues with the A810 due to the high dynamic demands
int     sigma_delta_hr_mask[SIGMA_DELTA_LEN * SIGMA_DELTA_LEN] = { 
	0, 0, 0, 0,  0, 0, 0, 0,
	0, 0, 0, 0,  1, 0, 0, 0,
	0, 0, 0, 1,  0, 0, 0, 1,
	0, 0, 1, 0,  0, 1, 0, 1,

	0, 1, 0, 1,  0, 1, 0, 1,
	0, 1, 1, 0,  1, 1, 0, 1,
	1, 0, 1, 1,  1, 0, 1, 1,
	1, 1, 1, 1,  0, 1, 1, 1
};
#else // Slow HR Matrix -- actually better analog performance
int     sigma_delta_hr_mask[SIGMA_DELTA_LEN * SIGMA_DELTA_LEN] = { 
	0, 0, 0, 0,  0, 0, 0, 0,
	1, 0, 0, 0,  0, 0, 0, 0,
	1, 1, 0, 0,  0, 0, 0, 0,
	1, 1, 1, 0,  0, 0, 0, 0,

	1, 1, 1, 1,  0, 0, 0, 0,
	1, 1, 1, 1,  1, 0, 0, 0,
	1, 1, 1, 1,  1, 1, 0, 0,
	1, 1, 1, 1,  1, 1, 1, 0
};
#endif

// for(hrb=0; hrb<8; ++hrb) {for(i=0; i<8; ++i) printf ("%d, ", hrb>i?1:0 ); printf("\n");

#define HR_OUT(CH, VALUE32) tmp_hrbits = _lsshl (VALUE32, -13) & 7; AIC_OUT(CH) = _sadd (_lsshl (VALUE32, -16), sigma_delta_hr_mask[(tmp_hrbits<<3) + sigma_delta_index])



int     sdt=0;
int     sdt_16=0;

unsigned short  QEP_cnt_old[2];
unsigned long   SUM_QEP_cnt_Diff[2];
long   DSP_time;

int     gate_cnt = 0;
int     gate_cnt_multiplier = 0;
int     gate_cnt_1 = 0;
int     feedback_hold = 0; /* Probe controlled Feedback-Hold flag */
int     pipi = 0;       /* Pipe index */
int     scnt = 0;       /* Slow summing count */
int     mout6_bias   = 0; /* Holds original bias value, while mout6 gets offset adjusted */
int     mout_orig[3] = {0,0,0}; /* Holds original value, while mout0..2 gets offset adjusted */

/* IIR tmp */

long   Q30L   = 1073741823L;
long   Q15L   = 32767L;
int    Q15    = 32767;
int    AbsIn  = 0;

/* RANDOM GEN */

long randomnum = 1L;


/* externals of SPM control */

extern SPM_STATEMACHINE state;
extern SPM_PI_FEEDBACK  feedback;
extern EXT_CONTROL      external;
extern FEEDBACK_MIXER   feedback_mixer;
extern ANALOG_VALUES    analog;
extern MOVE_OFFSET      move;
extern AREA_SCAN        scan;
extern AUTOAPPROACH     autoapp;
extern PROBE            probe;
extern CR_OUT_PULSE     CR_out_pulse;
extern CR_GENERIC_IO    CR_generic_io;
extern RECORDER         recorder;

extern int AS_ch2nd_constheight_enabled; /* const H mode flg of areascan process */
extern struct aicregdef    aicreg;

extern long PRB_ACPhaseA32;

long xy_vec[4];
long mm_vec[4];
long result_vec[4];

#define BIAS_ADJUST_STEP 4
#define Z_ADJUST_STEP   0x200L


/* Auxillary Random Number Generator */

#define RNG_a 16807         /* multiplier */
#define RNG_m 2147483647L   /* 2**31 - 1 */
#define RNG_q 127773L       /* m div a */
#define RNG_r 2836          /* m mod a */

inline void generate_nextlongrand (){
      unsigned long lo, hi;

      lo = _lsmpy (RNG_a, randomnum & 0xFFFF);
      hi = _lsmpy (RNG_a, (unsigned long)randomnum >> 16);
      lo += (hi & 0x7FFF) << 16;
      if (lo > RNG_m){
            lo &= RNG_m;
            ++lo;
      }
      lo += hi >> 15;
      if (lo > RNG_m){
            lo &= RNG_m;
            ++lo;
      }
      randomnum = (long)lo;
}


/* smoothly adjust bias - make sure |analog_bias| < 32766-BIAS_ADJUST_STEP !!  */
inline void run_bias_adjust (){
	if (analog.out[ANALOG_BIAS]-BIAS_ADJUST_STEP > AIC_OUT(6)){
		AIC_OUT(6) += BIAS_ADJUST_STEP;
	}else{	
		if (analog.out[ANALOG_BIAS]+BIAS_ADJUST_STEP < AIC_OUT(6))
			AIC_OUT(6) -= BIAS_ADJUST_STEP;
		else	
			AIC_OUT(6) = analog.out[ANALOG_BIAS];
	}
}

/* smoothly brings Zpos back to zero in case VP left it non zero at finish */
inline void run_Zpos_adjust (){
#if 0
	probe.Zpos = 0;
#else
	if (probe.Zpos > 0)
//		probe.Zpos -= Z_ADJUST_STEP;
		probe.Zpos = _lsadd (probe.Zpos, -Z_ADJUST_STEP);
	else
		if (probe.Zpos < 0)
//			probe.Zpos += Z_ADJUST_STEP;
			probe.Zpos = _lsadd (probe.Zpos, Z_ADJUST_STEP);
		else
			probe.Zpos = 0L;
#endif
}


/* This is ISR is called on each new sample.  The "mode"/statevariable
 * should be initialized with AIC_OFFSET_COMPENSATION before starting
 * the AIC/DMA isr, this assures the correct offset initialization and
 * runs the automatic offset compensation on startup.
 */


interrupt void dataprocess()
{
	int i, k;
	asm_read_time ();

	/* Load DataTime */
	state.DataProcessTime = (int)DSP_time;

	/* Compute IdleTime */
	IdleTimeTmp   -= state.DataProcessTime;
	state.IdleTime = IdleTimeTmp;


	if (sigma_delta_index & 1){

#ifdef USE_ANALOG_16 // if special build for MK2-Analog_16
		analog.counter[0] = 0L; // Counter/Timer support not yet available via FPGA
		analog.counter[1] = 0L; // Counter/Timer support not yet available via FPGA

#else // default: MK2-A810

		/* Automatic Aligned Gateing by scan or probe process */
		asm_counter_accumulate (); /* accumulate events in counters */
			
		/* always handle Counter_1 -- 16it gatetime only (fast only, < 800ms @ 75 kHz) */
		if (++gate_cnt_1 >= CR_generic_io.gatetime_1){
			gate_cnt_1 = 0;
			CR_generic_io.count_1 = analog.counter[1];
			analog.counter[1] = 0L;
				
			if (CR_generic_io.count_1 > CR_generic_io.peak_count_1) 
				CR_generic_io.peak_count_1 = CR_generic_io.count_1;
		}
	
		if (!(scan.pflg || probe.pflg) && CR_generic_io.pflg) {
			/* stand alone Rate Meter Mode else gating managed via spm_areascan or _probe module */
				
			/* handle Counter_0 -- 32it gatetime (very wide range) */
			if (gate_cnt_multiplier >= CR_generic_io.gatetime_h_0){
				if (++gate_cnt >= CR_generic_io.gatetime_l_0){
					gate_cnt = 0;
					gate_cnt_multiplier = 0;
						
					CR_generic_io.count_0 = analog.counter[0];
					analog.counter[0] = 0L;
						
					if (CR_generic_io.count_0 > CR_generic_io.peak_count_0) 
						CR_generic_io.peak_count_0 = CR_generic_io.count_0;
				}
			} else { // this makes up a 32bit counter...
				if (++gate_cnt == 0)
					++gate_cnt_multiplier;
			}
		}
#endif		
	}

	// always compute IIR/filters/RM, applies also to STS sources with FB OFF now!
	if (feedback.I_cross > 0){
	        // run IIR self adaptive filter on DAC0
	        feedback.I_fbw = AIC_IN(0);
		AbsIn = _abss(AIC_IN(0));
		feedback.q_factor15 = _lssub (Q15L, _smac (feedback.cb_Ic, AbsIn, Q15) / _sadd (AbsIn, feedback.I_cross));
		feedback.zxx = feedback.q_factor15;
		if (feedback.q_factor15 < feedback.ca_q15)
		        feedback.q_factor15 = feedback.ca_q15;
		feedback.I_iir = _smac ( _lsmpy ( _ssub (Q15, feedback.q_factor15), AIC_IN(0)),  feedback.q_factor15, _lsshl (feedback.I_iir, -16));
		AIC_IN(0) = _lsshl (feedback.I_iir, -16);
	}
						
	// Average and RMS computations
	feedback.I_avg = _lsadd (feedback.I_avg, AIC_IN(0));
	feedback.I_avg = _lssub (feedback.I_avg, rms_I_pipe[rms_pipi]);
	rms_I_pipe[rms_pipi] = AIC_IN(0);
	
	tmpsum = _lssub (AIC_IN(0), _lsshl (feedback.I_avg, -RMS_N2));
	tmpsum = _abss(tmpsum);
	if (tmpsum > 2047) // need to limit to avoid overflow of sum
	        tmpsum = 2047;
	tmpsum *= tmpsum;
	feedback.I_rms = _lsadd (feedback.I_rms, tmpsum);
	feedback.I_rms = _lssub (feedback.I_rms, rms_I2_pipe[rms_pipi]);
	rms_I2_pipe[rms_pipi]  = tmpsum;
	
	if (++rms_pipi == RMS_N) 
	        rms_pipi = 0;
						
        #define mi i
	for (mi=0; mi<4; ++mi){
	        // MIXER CHANNEL mi
	        if (feedback_mixer.mode[mi] & 0x10) // negate flag set?
		        AIC_IN(mi) = -AIC_IN(mi);
	}

	if (sigma_delta_index & 1){
		/* Offset Move task ? */
		if (move.pflg)
			run_offset_move ();
			
		/* Area Scan task ?
		 * the feedback task needs to be enabled to see the effect
		 * --> can set CI/CP to small values to "contineously" disable it!
		 */
		if (scan.pflg == 1)
			if (!scan.raster_b || !probe.pflg) // pause scan in raster_b is set and probe is going.
				run_area_scan ();

		// reset FB watch -- see below
		feedback.watch = 0; // for actual watch fb activity
	} else {
		
		/* Vector Probe task ?  (Bias may be changed here) */
		feedback_hold = 0;
		if (probe.pflg){
			run_probe ();
			if (probe.vector)
				if (probe.vector->options & VP_FEEDBACK_HOLD)
					feedback_hold = 1;
		} else {
			/* (re) Adjust Bias ? */
//			probe.Upos = _lsshl (analog.out[ANALOG_BIAS], 16);
			probe.Upos = (long)analog.out[ANALOG_BIAS] << 16;
				
			if (AIC_OUT(6) != analog.out[ANALOG_BIAS])
				run_bias_adjust ();
			if (probe.Zpos)
				run_Zpos_adjust ();
		}
	}

	/* "virtual" DSP based differential input for IN0 and IN4 as reference remapped on IN0 */
			
	if (state.mode & MD_DIFFIN0M4){
		AIC_IN(0) = _ssub (AIC_IN(0), AIC_IN(4));
	}

	/* FeedBack (FB) task ?  (evaluates several conditions and runs in various modes) */

	if (!feedback_hold && sigma_delta_index & 1){ /* may be freezed by probing process */
		/* Let's check out, if the feedback loop is really running. */
		feedback_hold = 1; 

//#define EN_CODE_EXTERNAL_FEEDBACK_CTRL
#ifdef EN_CODE_EXTERNAL_FEEDBACK_CTRL
		/* check if PID flag, EXTFB flag (+ treshold check...) are set */
		if ((state.mode & (MD_EXTFB | MD_PID)) == (MD_EXTFB | MD_PID) ?
		    (*(((int*)&iobuf)+external.FB_control_channel) <= external.FB_control_treshold) : state.mode & MD_PID){
#else
		if (state.mode & MD_PID){
#endif
			if ( !(AS_ch2nd_constheight_enabled && scan.pflg)){ // skip in 2nd const height mode!
				feedback_hold = 0; 

				// Feedback Mixer -- data transform and delta computation, summing
				feedback_mixer.delta = 0L;

				for (mi=0; mi<4; ++mi){
 					// MIXER CHANNEL mi
					switch (feedback_mixer.mode[mi]){
					case 3: // LOG
						feedback_mixer.x = AIC_IN(mi);
						asm_calc_mix_log ();
						feedback_mixer.delta = _smac(feedback_mixer.delta, _ssub (feedback_mixer.lnx, feedback_mixer.setpoint_log[mi]), feedback_mixer.gain[mi]);
						break;
					case 1: // LIN
						feedback_mixer.x = AIC_IN(mi);
						feedback_mixer.delta = _smac(feedback_mixer.delta, _ssub (feedback_mixer.x, feedback_mixer.setpoint[mi]), feedback_mixer.gain[mi]);
						break;
					case 9: // FUZZY
						if (AIC_IN(mi) > feedback_mixer.level[mi]){
							feedback_mixer.x = _ssub (AIC_IN(mi), feedback_mixer.level[mi]);
							feedback_mixer.delta = _smac(feedback_mixer.delta, _ssub (feedback_mixer.x, feedback_mixer.setpoint[mi]), feedback_mixer.gain[mi]);
						}
						break;
					case 11: // FUZZY LOG
						if (AIC_IN(mi) > feedback_mixer.level[mi]){
							feedback_mixer.x = _ssub (AIC_IN(mi), feedback_mixer.level[mi]);
							asm_calc_mix_log ();
							feedback_mixer.delta = _smac(feedback_mixer.delta, _ssub (feedback_mixer.lnx, feedback_mixer.setpoint[mi]), feedback_mixer.gain[mi]);
						}
						break;
					default: break; // OFF
					}
				}

				// run plain feedback from delta directly
				feedback.delta =  _lsshl (feedback_mixer.delta, -15);
				asm_feedback_from_delta ();
				feedback.watch = 1; // OK, we did it! -- for actual watch fb activity
			}
		} else {
			if (state.mode & MD_ZTRAN) // use external Z, inverted
				feedback.z = AIC_OUT(6) = (-AIC_IN(7));
		}
	}

	/* external watch task ? -- pass variable out on channel 7 -- no FIR */
	if (state.mode & MD_WATCH)
	{
		switch(external.watch_value){
			//default: AIC_OUT(7) = 0;
#ifdef EN_CODE_EXTERNAL_FEEDBACK_CTRL
		case EXTERNAL_FEEDBACK:
			AIC_OUT(7) = _sadd(feedback_hold * _ssub(external.watch_max, external.watch_min), external.watch_min);
			break;
			// AIC_OUT(7) = feedback_hold * (external.watch_max - external.watch_min) + external.watch_min;
#endif
		case EXTERNAL_OFFSET_X:
			AIC_OUT(7) = analog.out[ANALOG_X0_AUX];
			break;
#if 0
		case EXTERNAL_LOG:
			AIC_OUT(7) = feedback_mixer.lnx;
			break;
#endif
#if 0
		case EXTERNAL_DELTA:
			AIC_OUT(7) = feedback.delta;
			break;
#endif
		case EXTERNAL_PRB_SINE:
			AIC_OUT(7) = PRB_ref1stA;
			break;
		case EXTERNAL_PRB_PHASE:
			AIC_OUT(7) = PRB_modindex > (probe.LockInRefSinLenA>>1) ? 16384 : 0;
			break;
		case EXTERNAL_BIAS_ADD:
			AIC_OUT(6) = _lsshl (_smac (probe.Upos, AIC_IN(7), probe.AC_amp), -16);
			break;
		case EXTERNAL_EN_MOTOR:
			AIC_OUT(7) = analog.out[ANALOG_MOTOR];
			break;
//#define EN_CODE_EXTERNAL_FB2
#ifdef EN_CODE_EXTERNAL_FB2
// note: may break things here... max code size exceeded eventually depending on other code sections enabled
		case EXTERNAL_EN_PHASE:
			AIC_OUT(7) = _sadd (_lsshl (PRB_ACPhaseA32, -16), analog.out[ANALOG_MOTOR]);
			break;
		case EXTERNAL_PID:
			// run 2nd PID feedback in AIC 7 in/out
			tmpsum = AIC_IN(7);
			tmpsum = _ssub (external.fb2_set, tmpsum);
			external.fb2_delta2 = _ssub (external.fb2_delta, tmpsum);
			external.fb2_delta  = tmpsum;
			tmpsum = external.fb2_i_sum = _smac (external.fb2_i_sum, external.fb2_ci, external.fb2_delta);
			tmpsum = _smac (tmpsum, external.fb2_cp, external.fb2_delta);
			tmpsum = _smac (tmpsum, external.fb2_cd, external.fb2_delta2);
			HR_OUT (7, tmpsum);
			break;
#endif
		}
	}
	else AIC_OUT(7) = 0;


// NOW OUTPUT HR SIGNALS ON XYZ-Offset and XYZ-Scan -- do not touch Bias OUT(6) and Motor OUT(7) here -- handled directly previously.
// note: OUT(0-5) get overridden below by coarse/mover actions if requeste!!!

/* HR sigma-delta data processing (if enabled) -- turn off via adjusting sigma_delta_hr_mask to all 0 */

// do scan coordinate rotation transformation:
	if ( !(state.mode & MD_XXA))
	{
		xy_vec[2] = xy_vec[0] = scan.xyz_vec[i_X];
		xy_vec[3] = xy_vec[1] = scan.xyz_vec[i_Y];
		mul32 (xy_vec, scan.rotm, result_vec, 4);
		scan.xy_r_vec[i_X] = _lsadd (result_vec[0], result_vec[1]);
		scan.xy_r_vec[i_Y] = _lsadd (result_vec[2], result_vec[3]);
	} else {
		scan.xy_r_vec[i_X] = scan.xyz_vec[i_X];
		scan.xy_r_vec[i_Y] = scan.xyz_vec[i_Y];
	}

// XY-Offset and XY-Scan output -- separated or combined as life configuraion:
	if (state.mode & MD_OFFSETADDING){
		for (i=0; i<=i_Y; ++i)
			result_vec[i] = _lsadd (move.xyz_vec[i], scan.xy_r_vec[i]);
		
		HR_OUT (3, result_vec[i_X]);
		HR_OUT (4, result_vec[i_Y]);
	} else {
		HR_OUT (0, move.xyz_vec[i_X]);
		HR_OUT (1, move.xyz_vec[i_Y]);
		HR_OUT (3, scan.xy_r_vec[i_X]);
		HR_OUT (4, scan.xy_r_vec[i_Y]);
	}


// Z-Offset output
//---- slope add X*mx + Y*my
//#ifdef DSP_Z0_SLOPE_COMPENSATION
// limit dz add from xy-mult to say 10x scan.fm_dz0x+y, feedback like adjust if "diff" to far off from sudden slope change 
//	zpos_xymult = move.ZPos + scan.XposR * scan.fm_dz0x +  scan.YposR * scan.fm_dz0y ;
// make sure a smooth adjust -- if slope parameters get changed, need to prevent a jump.

	if ( !(state.mode & MD_XXB))
	{
		mul32 (scan.xy_r_vec, scan.fm_dz0_xy_vec, result_vec, 2);
		s_xymult = _lsadd (result_vec[i_X], result_vec[i_Y]);
		
		d_tmp = _lssub (s_xymult, s_xymult_prev);
		if (d_tmp > scan.z_slope_max) // limit up
			s_xymult_prev = _lsadd (s_xymult_prev, scan.z_slope_max);
		else if (d_tmp < -scan.z_slope_max) // limit dn
			s_xymult_prev = _lsadd (s_xymult_prev, -scan.z_slope_max);
		else s_xymult_prev = s_xymult; // normally this should do it
		
		zpos_xymult = _lsadd (move.xyz_vec[i_Z], s_xymult_prev);
		HR_OUT (2, zpos_xymult);
	} else {
		HR_OUT (2, move.xyz_vec[i_Z]);
	}
//#endif


// FEEDBACK OUT -- calculate Z-Scan output:

	d_tmp = _lssub (feedback.z32neg, probe.Zpos);
	HR_OUT (5, d_tmp);

	sigma_delta_index = (++sigma_delta_index) & 7;

// ========== END HR processing ================

// Auxillary outputs, option, coarse moves overrides, etc.

	if (state.mode & MD_NOISE){
		generate_nextlongrand ();
		AIC_OUT(6) = _lsshl (_smac (probe.Upos, randomnum, probe.AC_amp), -16);
	}

	/* Run Autoapproch/Movercontrol task ? */
	if (autoapp.pflg){
		if (autoapp.pflg==2)
			autoapp.pflg = 0;
		else
			run_autoapp ();
		
		for (i=0; i<autoapp.n_wave_channels; ++i){
			k = autoapp.channel_mapping[i] & 7;
			if (autoapp.channel_mapping[i] & AAP_MOVER_SIGNAL_ADD)
				AIC_OUT (k) = _sadd (AIC_OUT (k), analog.out[k]);
			else
				AIC_OUT (k) = analog.out[k];
		}
	} else 
		/* Run CoolRunner Out puls(es) task ? */
		if (CR_out_pulse.pflg)
			run_CR_out_pulse ();

	/* hooks to universal recorder */
	if (recorder.pflg){
		switch (recorder.pflg){
		case 1: RecSignalsASM16(); break;
		case 2: RecSignalsASM32(); break;
		}
	}

	/* increment DSP blink statemaschine's counter and time reference */
	++state.BLK_count;

	/* -- end of all data processing, preformance statistics update now -- */
	
	asm_read_time ();

	/* Load IdleTime */
	IdleTimeTmp = (int)DSP_time;

	/* Compute DataTime */
	state.DataProcessTime -= IdleTimeTmp;


	/* Load Peak Detection */
	if (abs(state.DataProcessTime) > abs(state.DataProcessTime_Peak)){
		 state.DataProcessTime_Peak = state.DataProcessTime;
		 state.IdleTime_Peak = state.IdleTime;
	}
}



