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

/* ignore this module for docuscan
% PlugInModuleIgnore
*/



#include <locale.h>
#include <libintl.h>

#include <time.h>

#include "glbvars.h"
#include "plug-ins/hard/modules/dsp.h"
#include <fcntl.h>
#include <sys/ioctl.h>

#include "gxsm/action_id.h"

#include "dsp-pci32/xsm/xsmcmd.h"

#include "sranger_hwi_control.h"
#include "sranger_hwi.h"
#include "../plug-ins/hard/modules/sranger_ioctl.h"


extern GxsmPlugin sranger_hwi_pi;
extern sranger_hwi_dev *sranger_hwi_hardware;



#define CONV_16(X) X = sranger_hwi_hardware->int_2_sranger_int (X)
#define CONV_32(X) X = sranger_hwi_hardware->long_2_sranger_long (X)

 	
void DSPControl::conv_dsp_probe (){
	CONV_16 (dsp_probe.start);
	CONV_16 (dsp_probe.AC_amp);
	CONV_16 (dsp_probe.AC_frq);
	CONV_16 (dsp_probe.AC_phaseA);
	CONV_16 (dsp_probe.AC_phaseB);
	CONV_16 (dsp_probe.AC_nAve);
	CONV_32 (dsp_probe.Zpos);
//	CONV_16 (dsp_probe.);
//	CONV_32 (dsp_probe.);
	CONV_16 (dsp_probe.vector_head);

}

void DSPControl::read_dsp_probe (){
	if (!sranger_hwi_hardware) return; 

	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.probe, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_read  (sranger_hwi_hardware->dsp, &dsp_probe, sizeof (dsp_probe)); 

	// from DSP to PC
	conv_dsp_probe ();

	// update, reconvert
	AC_amp = gapp->xsm->Inst->Dig2VoltOut ((double)dsp_probe.AC_amp) * gapp->xsm->Inst->BiasGainV2V ();
	AC_frq = dsp_probe.AC_frq;
	if (AC_frq < 32)
		AC_frq = 1.+22000.*AC_frq/128.;

	AC_phaseA = (double)dsp_probe.AC_phaseA/16.;
	AC_phaseB = (double)dsp_probe.AC_phaseB/16.;
	AC_lockin_avg_cycels = dsp_probe.AC_nAve;

	update ();
}


// some needfull macros to get some readable code
#define CONST_DSP_F16 65536.
#define VOLT2AIC(U)   (int)(gapp->xsm->Inst->VoltOut2Dig (gapp->xsm->Inst->BiasV2V (U)))
#define DVOLT2AIC(U)  (int)(gapp->xsm->Inst->VoltOut2Dig ((U)/gapp->xsm->Inst->BiasGainV2V ()))



// make automatic n and dnx from float number of steps, keep n below 1000.
void DSPControl::make_auto_n_vector_elments (double fnum){
	dsp_vector.n = 1;
	dsp_vector.dnx = 0;
	if (fnum >= 1.){
		if (fnum <= 1000.){ // automatic n ramp limiter
			dsp_vector.n = (DSP_LONG)round (fnum);
			dsp_vector.dnx = 0;
		} else if (fnum <= 10000.){
			dsp_vector.n = (DSP_LONG)round (fnum/10.);
			dsp_vector.dnx = 10;
		} else if (fnum <= 100000.){
			dsp_vector.n = (DSP_LONG)round (fnum/100.);
			dsp_vector.dnx = 100;
		} else if (fnum <= 1000000.){
			dsp_vector.n = (DSP_LONG)round (fnum/1000.);
			dsp_vector.dnx = 1000;
		} else{
			dsp_vector.n = (DSP_LONG)round (fnum/10000.);
			dsp_vector.dnx = 10000;
		}
	}
	++dsp_vector.n;
}

// make IV and dz (optional) vector from U_initial, U_final, dZ, n points and V-slope
// Options:
// Ramp-Mode: MAKE_VEC_FLAG_RAMP, auto n computation
// FixV-Mode: MAKE_VEC_FLAG_VHOLD, fix bias, only compute "speed" by Ui,Uf,slope
double DSPControl::make_Vdz_vector (double Ui, double Uf, double dZ, int n, double slope, int source, int options, double long &duration, make_vector_flags flags){
	double dv = fabs (Uf - Ui);
	if (flags & MAKE_VEC_FLAG_RAMP || n < 2)
		make_auto_n_vector_elments (dv/slope*frq_ref);
	else {
		dsp_vector.n = n; // number of data points
//		++dsp_vector.n;
		dsp_vector.dnx = abs((DSP_LONG)round ((Uf - Ui)*frq_ref/(slope*dsp_vector.n))); // number of steps between data points
	}
	double steps = (double)(dsp_vector.n) * (double)(dsp_vector.dnx+1);	//total number of steps

	duration += (double long) steps;
	dsp_vector.srcs = source & 0xffff; // source channel coding
	dsp_vector.options = options;
	dsp_vector.repetitions = 0;
	dsp_vector.ptr_next = 0x0;
	dsp_vector.ptr_final = flags & MAKE_VEC_FLAG_END ? 0:1; // VPC relative branch to next vector
	dsp_vector.f_du = flags & MAKE_VEC_FLAG_VHOLD ? 0 : (DSP_LONG)round (CONST_DSP_F16*gapp->xsm->Inst->VoltOut2Dig ((Uf-Ui)/gapp->xsm->Inst->BiasGainV2V ())/(steps));
	dsp_vector.f_dz = (DSP_LONG)round (CONST_DSP_F16*gapp->xsm->Inst->ZA2Dig (dZ/(steps)));
	dsp_vector.f_dx = 0;
	dsp_vector.f_dy = 0;
	dsp_vector.f_dx0 = 0;
	dsp_vector.f_dy0 = 0;
	dsp_vector.f_dphi = 0;
	return gapp->xsm->Inst->V2BiasV (gapp->xsm->Inst->Dig2VoltOut (VOLT2AIC(Ui) + (double)dsp_vector.f_du*steps/CONST_DSP_F16));
}	

// Copy of Vdz above, but the du steps were used for dx0
double DSPControl::make_Vdx0_vector (double Ui, double Uf, double dZ, int n, double slope, int source, int options, double long &duration, make_vector_flags flags
){
        double dv = fabs (Uf - Ui);
        if (flags & MAKE_VEC_FLAG_RAMP || n < 2)
                make_auto_n_vector_elments (dv/slope*frq_ref);
        else {
                dsp_vector.n = n; // number of data points
//              ++dsp_vector.n;
                dsp_vector.dnx = abs((DSP_LONG)round ((Uf - Ui)*frq_ref/(slope*dsp_vector.n))); // number of steps between data points
        }
        double steps = (double)(dsp_vector.n) * (double)(dsp_vector.dnx+1);     //total number of steps

        duration += (double long) steps;
        dsp_vector.srcs = source & 0xffff; // source channel coding
        dsp_vector.options = options;
	dsp_vector.repetitions = 0;
	dsp_vector.ptr_next = 0x0;
        dsp_vector.ptr_final = flags & MAKE_VEC_FLAG_END ? 0:1; // VPC relative branch to next vector
        dsp_vector.f_dx0 = flags & MAKE_VEC_FLAG_VHOLD ? 0 : (DSP_LONG)round (CONST_DSP_F16*gapp->xsm->Inst->VoltOut2Dig ((Uf-Ui)/gapp->xsm->Inst->BiasGainV2V ())/(steps)); // !!!!!!x
        dsp_vector.f_dz = (DSP_LONG)round (CONST_DSP_F16*gapp->xsm->Inst->ZA2Dig (dZ/(steps)));
        dsp_vector.f_dx = 0;
        dsp_vector.f_dy = 0;
        dsp_vector.f_du = 0;
        dsp_vector.f_dy0 = 0;
        dsp_vector.f_dphi = 0;
        return gapp->xsm->Inst->V2BiasV (gapp->xsm->Inst->Dig2VoltOut (VOLT2AIC(Ui) + (double)dsp_vector.f_du*steps/CONST_DSP_F16));
}       

// Copy of Vdz above, but the du steps were used for dx0
double DSPControl::make_dx0_vector (double X0i, double X0f, int n, double slope, int source, int options, double long &duration, make_vector_flags flags
){
        double dv = fabs (X0f - X0i);
        if (flags & MAKE_VEC_FLAG_RAMP || n < 2)
                make_auto_n_vector_elments (dv/slope*frq_ref);
        else {
                dsp_vector.n = n; // number of data points
//              ++dsp_vector.n;
                dsp_vector.dnx = abs((DSP_LONG)round ((X0f - X0i)*frq_ref/(slope*dsp_vector.n))); // number of steps between data points
        }
        double steps = (double)(dsp_vector.n) * (double)(dsp_vector.dnx+1);     //total number of steps

        duration += (double long) steps;
        dsp_vector.srcs = source & 0xffff; // source channel coding
        dsp_vector.options = options;
	dsp_vector.repetitions = 0;
	dsp_vector.ptr_next = 0x0;
        dsp_vector.ptr_final = flags & MAKE_VEC_FLAG_END ? 0:1; // VPC relative branch to next vector
        dsp_vector.f_dx0 = flags & MAKE_VEC_FLAG_VHOLD ? 0 : (DSP_LONG)round (CONST_DSP_F16*gapp->xsm->Inst->VoltOut2Dig (X0f-X0i)/(steps));
        dsp_vector.f_dz = 0;
        dsp_vector.f_dx = 0;
        dsp_vector.f_dy = 0;
        dsp_vector.f_du = 0;
        dsp_vector.f_dy0 = 0;
        dsp_vector.f_dphi = 0;
        return gapp->xsm->Inst->V2BiasV (gapp->xsm->Inst->Dig2VoltOut (VOLT2AIC(X0i) + (double)dsp_vector.f_dx0*steps/CONST_DSP_F16));
}       

// make dZ/dX/dY vector from n point (if > 2, else automatic n) and (dX,dY,dZ) slope
double DSPControl::make_ZXYramp_vector (double dZ, double dX, double dY, int n, double slope, int source, int options, double long &duration, make_vector_flags flags){
	double dr = sqrt(dZ*dZ + dX*dX + dY*dY);

	if (flags & MAKE_VEC_FLAG_RAMP || n<2)
		make_auto_n_vector_elments (dr/slope*frq_ref);
	else {
		dsp_vector.n = n;
		dsp_vector.dnx = (DSP_LONG)round ( fabs (dr*frq_ref/(slope*dsp_vector.n)));
		++dsp_vector.n;
	}
	double steps = (double)(dsp_vector.n) * (double)(dsp_vector.dnx+1);

	duration += (double long) steps;
	dsp_vector.srcs = source & 0xffff;
	dsp_vector.options = options;
	dsp_vector.ptr_fb = 0;
	dsp_vector.repetitions = 0;
	dsp_vector.ptr_next = 0x0;
	dsp_vector.ptr_final = flags & MAKE_VEC_FLAG_END ? 0:1; // VPC relative branch to next vector
	dsp_vector.f_du = 0;
	dsp_vector.f_dx = (DSP_LONG)round (dsp_vector.n > 1 ? (CONST_DSP_F16*gapp->xsm->Inst->XA2Dig (dX) / steps) : 0);
	dsp_vector.f_dy = (DSP_LONG)round (dsp_vector.n > 1 ? (CONST_DSP_F16*gapp->xsm->Inst->YA2Dig (dY) / steps) : 0);
	dsp_vector.f_dz = (DSP_LONG)round (dsp_vector.n > 1 ? (CONST_DSP_F16*gapp->xsm->Inst->ZA2Dig (dZ) / steps) : 0);
	dsp_vector.f_dx0 = 0;
	dsp_vector.f_dy0 = 0;
	dsp_vector.f_dphi = 0;

	return gapp->xsm->Inst->Dig2ZA ((long)round ((double)dsp_vector.f_dz*steps/CONST_DSP_F16));
}




// make dPhi vector from n point (if > 2, else automatic n) and dPhi slope
double DSPControl::make_phase_vector (double dPhi, int n, double slope, int source, int options, double long &duration, make_vector_flags flags){
	double dr = dPhi*16.;
	slope *= 16.;

	if (flags & MAKE_VEC_FLAG_RAMP || n<2)
		make_auto_n_vector_elments (dr/slope*frq_ref);
	else {
		dsp_vector.n = n;
		dsp_vector.dnx = (DSP_LONG)round ( fabs (dr*frq_ref/(slope*dsp_vector.n)));
		++dsp_vector.n;
	}
	double steps = (double)(dsp_vector.n) * (double)(dsp_vector.dnx+1);

	duration += (double long) steps;
	dsp_vector.srcs = source & 0xffff;
	dsp_vector.options = options;
	dsp_vector.ptr_fb = 0;
	dsp_vector.repetitions = 0;
	dsp_vector.ptr_next = 0x0;
	dsp_vector.ptr_final = flags & MAKE_VEC_FLAG_END ? 0:1; // VPC relative branch to next vector
	dsp_vector.f_du = 0;
	dsp_vector.f_dx = 0;
	dsp_vector.f_dy = 0;
	dsp_vector.f_dz = 0;
	dsp_vector.f_dx0 = 0;
	dsp_vector.f_dy0 = 0;
	dsp_vector.f_dphi = (DSP_LONG)round (dsp_vector.n > 1 ? (CONST_DSP_F16 * dr / steps) : 0);

	return round ((double)dsp_vector.f_dphi*steps/CONST_DSP_F16/16.);
}

// Make a delay Vector
double DSPControl::make_delay_vector (double delay, int source, int options, double long &duration, make_vector_flags flags, int points){

	if (points > 0){
		double rnum = delay*frq_ref;
		dsp_vector.n = points;
		dsp_vector.dnx = (DSP_LONG)round (rnum / points);
	} else
		make_auto_n_vector_elments (delay*frq_ref);
	duration += (double long) (dsp_vector.n)*(dsp_vector.dnx+1);
	dsp_vector.srcs = source & 0xffff;
	dsp_vector.options = options;
	dsp_vector.repetitions = 0; // number of repetitions, not used yet
	dsp_vector.ptr_next = 0x0;  // pointer to next vector -- not used, only for loops
	dsp_vector.ptr_final = flags & MAKE_VEC_FLAG_END ? 0:1;   // VPC relative branch to next vector
	dsp_vector.f_du = 0;
	dsp_vector.f_dz = 0;
	dsp_vector.f_dx = 0; // x stepwidth, not used for probing
	dsp_vector.f_dy = 0; // y stepwidth, not used for probing
	dsp_vector.f_dx0 = 0; // x0 stepwidth, not used for probing
	dsp_vector.f_dy0 = 0; // y0 stepwidth, not used for probing
	dsp_vector.f_dphi = 0; // z0 stepwidth, not used for probing
	return (double)((long)(dsp_vector.n)*(long)(dsp_vector.dnx+1))/frq_ref;
}

// Make Vector Table End
void DSPControl::append_null_vector (int options, int index){
	// NULL vector -- just to clean vector table
	dsp_vector.n = 0;
	dsp_vector.dnx = 0;
	dsp_vector.srcs = 0x0000;
	dsp_vector.options = options;
	dsp_vector.repetitions = 0; // number of repetitions
	dsp_vector.ptr_next = 0;  // END
	dsp_vector.ptr_final= 0;  // END
	dsp_vector.f_dx = 0;
	dsp_vector.f_dy = 0;
	dsp_vector.f_dz = 0;
	// append 4 NULL-Vectors, just to clean up the end.
	write_dsp_vector (index);
	write_dsp_vector (index+1);
	write_dsp_vector (index+2);
	write_dsp_vector (index+3);
}

// Create Vector Table form Mode (pvm=PV_MODE_XXXXX) and Execute if requested (start=TRUE) or write only
void DSPControl::write_dsp_probe (int start, pv_mode pvm){
	int options=0;
	int ramp_sources=0x000;
	int ramp_points;
	int recover_options=0;
	int vector_index = 0;
	double long vp_duration = 0;
	double dU_IV=0.;
	double dU_step=0.;
	int vpci;

	if (!sranger_hwi_hardware) return; 

	if (!start) // reset 
		probe_trigger_single_shot = 0;

	switch (pvm){
	case PV_MODE_NONE: // write dummy delay and NULL Vector
		options      = (PL_option_flags & FLAG_FB_ON     ? 0      : VP_FEEDBACK_HOLD)
			     | (PL_option_flags & FLAG_INTEGRATE ? VP_AIC_INTEGRATE : 0);
		ramp_sources = PL_option_flags & FLAG_SHOW_RAMP ? Source : 0x000;
		recover_options = 0;

		make_delay_vector (0.1, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);
		make_delay_vector (0.1, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_END);
		write_dsp_vector (vector_index++);
		append_null_vector (options, vector_index);
		sranger_hwi_hardware->probe_time_estimate = (int)vp_duration; // used for timeout check
		break;

	case PV_MODE_IV: // ------------ Special Vector Setup for IV Probes "Probe ala STM"
		options      = (IV_option_flags & FLAG_FB_ON     ? 0      : VP_FEEDBACK_HOLD)
			     | (IV_option_flags & FLAG_INTEGRATE ? VP_AIC_INTEGRATE : 0);
		ramp_sources = IV_option_flags & FLAG_SHOW_RAMP ? Source : 0x000;
		recover_options = 0; // FeedBack On for recovery!

		vpci = vector_index;
		// do we need to split up if crossing zero?
		if (fabs (IV_dz) > 0.001 && ((IV_end < 0. && IV_start > 0.) || (IV_end > 0. && IV_start < 0.)) && (bias != 0.)){
			int vpc=0;
			// compute sections   
			// z = z0 + dz * ( 1 - | U / Bias | )
			// dz_bi := dz*(1-|Ui/Bias|) - 0
			// dz_i0 := dz - dz_bi
			// dz_0f := dz*(1-|Uf/Bias|) - (dz_bi + dz_i0)
			double ui,u0,uf;
			double dz_bi, dz_i0, dz_0f;
			int n12, n23;
			ui = IV_start; 
			u0 = 0.;
			uf = IV_end;
			n12 = (int)(round ((double)IV_points*fabs (ui/(uf-ui))));
			n23 = (int)(round ((double)IV_points*fabs (uf/(uf-ui))));

			dz_bi = IV_dz*(1.-fabs (ui/bias));
			dz_i0 = IV_dz - dz_bi;
			dz_0f = IV_dz*(1.-fabs (uf/bias)) - (dz_bi + dz_i0);

			make_Vdz_vector (bias, ui, dz_bi, -1, IV_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
			write_dsp_vector (vector_index++);

			make_Vdz_vector (ui, u0, dz_i0, n12, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			write_dsp_vector (vector_index++);

			make_Vdz_vector (u0, uf, dz_0f, n23, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			write_dsp_vector (vector_index++);

			dU_IV = uf-ui; dU_step = dU_IV/IV_points;

			if (IV_option_flags & FLAG_DUAL) {
				// run also reverse probe ramp in dual mode
				make_Vdz_vector (uf, u0, -dz_0f, n23, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				write_dsp_vector (vector_index++);

				make_Vdz_vector (u0, ui, -dz_i0, n12, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				write_dsp_vector (vector_index++);

				// Ramp back to given bias voltage   
				make_Vdz_vector (ui, bias, -dz_bi, -1, IV_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
				write_dsp_vector (vector_index++);

			} else {
				make_Vdz_vector (uf, bias, -(dz_bi+dz_i0+dz_0f), -1, IV_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
				write_dsp_vector (vector_index++);
			}

			if (IV_repetitions > 1){
				// Final vector, gives the IVC some time to recover   
				make_delay_vector (IV_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				write_dsp_vector (vector_index++);
				make_delay_vector (IV_recover_delay, ramp_sources, recover_options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				dsp_vector.repetitions = IV_repetitions-1;
				dsp_vector.ptr_next = -vector_index; // go to start
				write_dsp_vector (vector_index++);
			} else {
				make_delay_vector (IV_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				write_dsp_vector (vector_index++);
				make_delay_vector (IV_recover_delay, ramp_sources, recover_options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				write_dsp_vector (vector_index++);
			}
			// add automatic conductivity measurement rho(Z) -- HOLD Bias fixed now!
			if (IVdz_repetitions > 0){
				// don't know the reason, but the following delay vector is needed to separate dI/dU and dI/dz
				make_delay_vector (0., ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				write_dsp_vector (vector_index++);

				// in case of rep > 1 the DSP will jump back to this point
				vpc = vector_index;
	
				make_Vdz_vector (bias, ui, dz_bi, -1, IV_slope_ramp, ramp_sources, options, vp_duration, (make_vector_flags)(MAKE_VEC_FLAG_RAMP | MAKE_VEC_FLAG_VHOLD));
				write_dsp_vector (vector_index++);
	
				make_Vdz_vector (ui, u0, dz_i0, n12, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_VHOLD);
				write_dsp_vector (vector_index++);
	
				make_Vdz_vector (u0, uf, dz_0f, n23, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_VHOLD);
				write_dsp_vector (vector_index++);
	
				if (IV_option_flags & FLAG_DUAL) {
					make_Vdz_vector (uf, u0, -dz_0f, n23, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_VHOLD);
					write_dsp_vector (vector_index++);
	
					make_Vdz_vector (u0, ui, -dz_i0, n12, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_VHOLD);
					write_dsp_vector (vector_index++);
	
					make_Vdz_vector (ui, bias, -dz_bi, -1, IV_slope_ramp, ramp_sources, options, vp_duration, (make_vector_flags)(MAKE_VEC_FLAG_RAMP | MAKE_VEC_FLAG_VHOLD));
					write_dsp_vector (vector_index++);
				} else {
					make_Vdz_vector (uf, bias, -(dz_bi+dz_i0+dz_0f), -1, IV_slope_ramp, ramp_sources, options, vp_duration, (make_vector_flags)(MAKE_VEC_FLAG_RAMP | MAKE_VEC_FLAG_VHOLD));
					write_dsp_vector (vector_index++);
				}
	
				if (IVdz_repetitions > 1){
					// Final vector, gives the IVC some time to recover   
					make_delay_vector (IV_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
					write_dsp_vector (vector_index++);
	
					make_delay_vector (IV_recover_delay, ramp_sources, recover_options, vp_duration, MAKE_VEC_FLAG_NORMAL);
					dsp_vector.repetitions = IVdz_repetitions-1;
					dsp_vector.ptr_next = -(vector_index-vpc); // go to rho start
					write_dsp_vector (vector_index++);
				} else {
					make_delay_vector (IV_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
					write_dsp_vector (vector_index++);
					make_delay_vector (IV_recover_delay, ramp_sources, recover_options, vp_duration, MAKE_VEC_FLAG_NORMAL);
					write_dsp_vector (vector_index++);
				}
			}

		} else {
			// compute sections   
			// z = z0 + dz * ( 1 - | U / Bias | )
			// dz_bi := dz*(1-|Ui/Bias|) - 0
			double ui,uf;
			double dz_bi, dz_if;
			int vpc=0;

			ui = IV_start; 
			uf = IV_end;
			if (bias != 0.){
				dz_bi = IV_dz*(1.-fabs (ui/bias));
				dz_if = IV_dz*(1.-fabs (uf/bias)) - dz_bi;
			} else {
				dz_bi = dz_if = 0.;
			}

			dU_IV   = gapp->xsm->Inst->V2BiasV (gapp->xsm->Inst->Dig2VoltOut ((long double)dsp_vector.f_du*(long double)(dsp_vector.n-1)*(long double)(dsp_vector.dnx ? dsp_vector.dnx+1 : 1)/CONST_DSP_F16));
			dU_step = gapp->xsm->Inst->V2BiasV (gapp->xsm->Inst->Dig2VoltOut ((long double)dsp_vector.f_du*(long double)(dsp_vector.dnx ? dsp_vector.dnx+1 : 1)/CONST_DSP_F16));
			

			make_Vdz_vector (bias, ui, dz_bi, -1, IV_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
			write_dsp_vector (vector_index++);

			make_Vdz_vector (ui, uf, dz_if, IV_points, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			write_dsp_vector (vector_index++);
			
			// add vector for reverse return ramp? -- Force return path if dz != 0
			if (IV_option_flags & FLAG_DUAL) {
				// run also reverse probe ramp in dual mode
				make_Vdz_vector (uf, ui, -dz_if, IV_points, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				write_dsp_vector (vector_index++);

				// Ramp back to given bias voltage   
				make_Vdz_vector (ui, bias, -dz_bi, -1, IV_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
				write_dsp_vector (vector_index++);
			} else {
				// Ramp back to given bias voltage   
				make_Vdz_vector (uf, bias, -(dz_if+dz_bi), -1, IV_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
				write_dsp_vector (vector_index++);
			}
			if (IV_repetitions > 1){
				// Final vector, gives the IVC some time to recover   
				make_delay_vector (IV_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				write_dsp_vector (vector_index++);

				make_delay_vector (IV_recover_delay, ramp_sources, recover_options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				dsp_vector.repetitions = IV_repetitions-1;
				dsp_vector.ptr_next = -vector_index; // go to start
				write_dsp_vector (vector_index++);
			} else {
				make_delay_vector (IV_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				write_dsp_vector (vector_index++);
				make_delay_vector (IV_recover_delay, ramp_sources, recover_options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				write_dsp_vector (vector_index++);
			}

			// add automatic conductivity measurement rho(Z) -- HOLD Bias fixed now!
			if ((IVdz_repetitions > 0) && (fabs (IV_dz) > 0.001) && (bias != 0.)){
				// don't know the reason, but the following delay vector is needed to separate dI/dU and dI/dz
				make_delay_vector (0., ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
				write_dsp_vector (vector_index++);

				// in case of rep > 1 the DSP will jump back to this point
				vpc = vector_index;
	
				make_Vdz_vector (bias, ui, dz_bi, -1, IV_slope_ramp, ramp_sources, options, vp_duration, (make_vector_flags)(MAKE_VEC_FLAG_RAMP | MAKE_VEC_FLAG_VHOLD));
				write_dsp_vector (vector_index++);

				make_Vdz_vector (ui, uf, dz_if, IV_points, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_VHOLD);
				write_dsp_vector (vector_index++);
			
				if (IV_option_flags & FLAG_DUAL) {
					make_Vdz_vector (uf, ui, -dz_if, IV_points, IV_slope, Source, options, vp_duration, MAKE_VEC_FLAG_VHOLD);
					write_dsp_vector (vector_index++);

					// Ramp back to given bias voltage   
					make_Vdz_vector (ui, bias, -dz_bi, -1, IV_slope_ramp, ramp_sources, options, vp_duration, (make_vector_flags)(MAKE_VEC_FLAG_RAMP | MAKE_VEC_FLAG_VHOLD));
					write_dsp_vector (vector_index++);
				} else {
					make_Vdz_vector (uf, bias, -(dz_if+dz_bi), -1, IV_slope_ramp, ramp_sources, options, vp_duration, (make_vector_flags)(MAKE_VEC_FLAG_RAMP | MAKE_VEC_FLAG_VHOLD));
					write_dsp_vector (vector_index++);
				}
	
				if (IVdz_repetitions > 1 ){
					// Final vector, gives the IVC some time to recover   
					make_delay_vector (IV_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
					write_dsp_vector (vector_index++);
	
					make_delay_vector (IV_recover_delay, ramp_sources, recover_options, vp_duration, MAKE_VEC_FLAG_NORMAL);
					dsp_vector.repetitions = IVdz_repetitions-1;
					dsp_vector.ptr_next = -(vector_index-vpc); // go to rho start
					write_dsp_vector (vector_index++);
				} else {
					make_delay_vector (IV_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
					write_dsp_vector (vector_index++);
					make_delay_vector (IV_recover_delay, ramp_sources, recover_options, vp_duration, MAKE_VEC_FLAG_NORMAL);
					write_dsp_vector (vector_index++);
				}
			}
		}

		// Step and Repeat along Line defined by dx dy -- if dxy points > 1, auto return?
		if (IV_dxy_points > 1){

			// Move probe to next position and setup auto repeat!
			make_ZXYramp_vector (0., IV_dx/(IV_dxy_points-1), IV_dy/(IV_dxy_points-1), 100, IV_dxy_slope, 
					     ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
			write_dsp_vector (vector_index++);
			make_delay_vector (IV_dxy_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
			dsp_vector.repetitions = IV_dxy_points-1;
			dsp_vector.ptr_next = -(vector_index-vpci); // go to initial IV start for full repeat
			write_dsp_vector (vector_index++);

			// add vector for full reverse return path -- YES!, always auto return!
			make_ZXYramp_vector (0., 
					     -IV_dx*(IV_dxy_points)/(IV_dxy_points-1.), 
					     -IV_dy*(IV_dxy_points)/(IV_dxy_points-1.), 100, IV_dxy_slope, 
					     ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
			write_dsp_vector (vector_index++);
		}

		// Final vector
		make_delay_vector (0., ramp_sources, options, vp_duration, MAKE_VEC_FLAG_END);
		write_dsp_vector (vector_index++);

		append_null_vector (options, vector_index);

		sranger_hwi_hardware->probe_time_estimate = (int)vp_duration; // used for timeout check


		{
			gchar *info=NULL;
			int warn_flag=FALSE;
			if (probe_trigger_raster_points_user > 0 && write_vector_mode != PV_MODE_NONE){
				double T_probe_cycle   = 1e3 * (double)vp_duration/frq_ref; // Time of full probe cycle in ms
				double T_raster2raster = 1e3 * gapp->xsm->data.s.rx / (gapp->xsm->data.s.nx/probe_trigger_raster_points_user) / scan_speed_x; // Time inbetween raster points in ms
				info = g_strdup_printf ("Tp=%.2f ms, Tr=%.2f ms, Td=%.2f ms", T_probe_cycle, T_raster2raster, T_raster2raster - T_probe_cycle);

				if (T_raster2raster <= T_probe_cycle){
					warn_flag=TRUE;
					GtkWidget *dialog = gtk_message_dialog_new (NULL,
										    GTK_DIALOG_DESTROY_WITH_PARENT,
										    GTK_MESSAGE_WARNING,
										    GTK_BUTTONS_CLOSE,
										    "The probing at each raster point lasts to long:\n"
										    "Time of one probe cycle is %.2f ms\n"
										    "and\n"
										    "Time from raster to raster point is %.2f ms.\n\n --- FYI: ---\n"
										    "# Raster points per line: %d\n"
										    "Time inbetween single scan points: %.2f ms",
										    T_probe_cycle, T_raster2raster, 
										    gapp->xsm->data.s.nx/probe_trigger_raster_points_user,
										    1e3 * gapp->xsm->data.s.rx / gapp->xsm->data.s.nx / scan_speed_x
										    );
					g_signal_connect_swapped (G_OBJECT (dialog), "response",
								  G_CALLBACK (gtk_widget_destroy),
								  G_OBJECT (dialog));
					gtk_widget_show (dialog);
				}	
			} else
				info = g_strdup_printf ("Tp=%.2f ms, dU=%.3f V, dUs=%.2f mV, O*0x%02x S*0x%06x", 
							1e3*(double)vp_duration/frq_ref, dU_IV, dU_step*1e3, options, Source
					);
			if (IV_status){
				GtkStyle *style;
				GdkColor ct, cbg;
				gtk_entry_set_text (GTK_ENTRY (IV_status), info);
				style = gtk_style_copy (gtk_widget_get_style(IV_status));
				if (warn_flag){
					ct.red = 0xffff;
					ct.green = 0x0;
					ct.blue = 0x0;
					cbg.red = 0xffff;
					cbg.green = 0x9999;
					cbg.blue = 0xffff;
				}else{
					ct.red = 0x0;
					ct.green = 0x0;
					ct.blue = 0x0;
					cbg.red = 0xeeee;
					cbg.green = 0xdddd;
					cbg.blue = 0xdddd;
				}
				// GTK3QQQ ARGRRRRRR 
				// gdk_color_alloc (gtk_widget_get_colormap(IV_status), &ct);
				// gdk_color_alloc (gtk_widget_get_colormap(IV_status), &cbg);
				style->text[GTK_STATE_NORMAL] = ct;
				style->bg[GTK_STATE_NORMAL] = cbg;
				gtk_widget_set_style(IV_status, style);
			}
			g_free (info);
		}

		break;


	case PV_MODE_FZ: // ------------ Special Vector Setup for FZ (Force(or what ever!!)-Distance) "Probe ala AFM"
		options      = (FZ_option_flags & FLAG_FB_ON     ? 0      : VP_FEEDBACK_HOLD)
			     | (FZ_option_flags & FLAG_INTEGRATE ? VP_AIC_INTEGRATE : 0);
		ramp_sources = FZ_option_flags & FLAG_SHOW_RAMP ? Source : 0x000;

		// Ramp to initial Z from "current Z"
		make_ZXYramp_vector (FZ_start, 0., 0., -1, FZ_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
		write_dsp_vector (vector_index++);

		// Ramp to final Z
		make_ZXYramp_vector (FZ_end - FZ_start, 0., 0., FZ_points, FZ_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		// add vector for reverse return ramp?
		if (FZ_option_flags & FLAG_DUAL) {
			// Ramp to initil Z
			make_ZXYramp_vector (FZ_start - FZ_end, 0., 0., FZ_points, FZ_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			write_dsp_vector (vector_index++);

			// Ramp back to "current Z"
			make_ZXYramp_vector (-FZ_start, 0., 0., -1, FZ_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
			write_dsp_vector (vector_index++);
		} else {
			// Ramp back to "current Z"
			make_ZXYramp_vector (-FZ_end, 0., 0., -1, FZ_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
			write_dsp_vector (vector_index++);
		}

		// Final vector, gives the IVC some time to recover   
		make_delay_vector (FZ_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_END);
		write_dsp_vector (vector_index++);

		append_null_vector (options, vector_index);

		sranger_hwi_hardware->probe_time_estimate = (int)vp_duration; // used for timeout check

		{
			gchar *info=NULL;
			if (probe_trigger_raster_points_user > 0 && write_vector_mode != PV_MODE_NONE){
				double T_probe_cycle   = 1e3 * (double)vp_duration/frq_ref; // Time of full probe cycle in ms
				double T_raster2raster = 1e3 * gapp->xsm->data.s.rx / (gapp->xsm->data.s.nx/probe_trigger_raster_points_user) / scan_speed_x; // Time inbetween raster points in ms
				info = g_strdup_printf ("Tp=%.2f ms, Tr=%.2f ms, Td=%.2f ms", T_probe_cycle, T_raster2raster, T_raster2raster - T_probe_cycle);

				if (T_raster2raster <= T_probe_cycle){
					GtkWidget *dialog = gtk_message_dialog_new (NULL,
										    GTK_DIALOG_DESTROY_WITH_PARENT,
										    GTK_MESSAGE_WARNING,
										    GTK_BUTTONS_CLOSE,
										    "The probing a each raster point lasts to long:\n"
										    "T probe cycle is %.2f ms\n"
										    "and\n"
										    "T raster to raster point is %.2f ms.",
										    T_probe_cycle, T_raster2raster
										    );
					g_signal_connect_swapped (G_OBJECT (dialog), "response",
								  G_CALLBACK (gtk_widget_destroy),
								  G_OBJECT (dialog));
					gtk_widget_show (dialog);
				}
			} else
				info = g_strdup_printf ("Tp=%.2f ms", 
							1e3*(double)vp_duration/frq_ref
					);
			if (FZ_status)
				gtk_entry_set_text (GTK_ENTRY (FZ_status), info);
			g_free (info);
		}

		break;



	case PV_MODE_PL: // ------------ Special Vector Setup for PL (Puls)
		options      = (PL_option_flags & FLAG_FB_ON     ? 0      : VP_FEEDBACK_HOLD)
			     | (PL_option_flags & FLAG_INTEGRATE ? VP_AIC_INTEGRATE : 0);
		ramp_sources = PL_option_flags & FLAG_SHOW_RAMP ? Source : 0x000;

		if (PL_slope < 0.1)
			ramp_points = (int)(10./PL_slope);
		else
			ramp_points = 10;

		make_Vdz_vector (bias, bias+PL_volt, 0., ramp_points, PL_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_delay_vector (PL_duration * 1e-3, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_Vdz_vector (bias+PL_volt, bias, 0., ramp_points, PL_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);
			
		make_delay_vector (PL_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		dsp_vector.repetitions = PL_repetitions-1;
		dsp_vector.ptr_next = -3; // go to start
		write_dsp_vector (vector_index++);

		make_delay_vector (PL_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_END);
		write_dsp_vector (vector_index++);
		append_null_vector (options, vector_index);

		sranger_hwi_hardware->probe_time_estimate = (int)vp_duration; // used for timeout check

		{
			gchar *info=NULL;
			info = g_strdup_printf ("T_total=%.2f ms, O*0x%02x S*0x%06x", 
						1e3*(double)vp_duration/frq_ref, options, Source
				);
			if (PL_status)
				gtk_entry_set_text (GTK_ENTRY (PL_status), info);
			g_free (info);
		}
		break;

	case PV_MODE_SP: // ------------ Special Vector Setup for Slow PL (Puls) + Flag
		options      = (SP_option_flags & FLAG_FB_ON     ? 0      : VP_FEEDBACK_HOLD)
			     | (SP_option_flags & FLAG_INTEGRATE ? VP_AIC_INTEGRATE : 0);
		ramp_sources = SP_option_flags & FLAG_SHOW_RAMP ? Source : 0x000;

//	double SP_duration, SP_ramptime, SP_volt, SP_final_delay, SP_flag_on_volt, SP_flag_off_volt;


		ramp_points = (int)(100.*SP_ramptime);

		make_dx0_vector (0., SP_flag_volt, 10, SP_flag_volt/0.1, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_delay_vector (SP_final_delay * 60., ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_dx0_vector (SP_flag_volt, 0., 10, SP_flag_volt/0.1, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);


		make_Vdz_vector (bias, bias+SP_volt, 0., ramp_points, SP_volt/(SP_ramptime*60.) , Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_delay_vector (SP_duration * 60., Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_Vdz_vector (bias+SP_volt, bias, 0., ramp_points, SP_volt/(SP_ramptime*60.), Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);
			
		make_dx0_vector (0., SP_flag_volt, 10, SP_flag_volt/0.1, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_delay_vector (SP_final_delay * 60., ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		dsp_vector.repetitions = SP_repetitions-1;
		dsp_vector.ptr_next = -5; // go to start
		write_dsp_vector (vector_index++);


		make_dx0_vector (SP_flag_volt, 0., 10, SP_flag_volt/0.1, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_delay_vector (SP_final_delay * 60., ramp_sources, options, vp_duration, MAKE_VEC_FLAG_END);
		write_dsp_vector (vector_index++);
		append_null_vector (options, vector_index);

		sranger_hwi_hardware->probe_time_estimate = (int)vp_duration; // used for timeout check

		{
			gchar *info=NULL;
			info = g_strdup_printf ("T_total=%.2f ms, O*0x%02x S*0x%06x", 
						1e3*(double)vp_duration/frq_ref, options, Source
				);
			if (SP_status)
				gtk_entry_set_text (GTK_ENTRY (SP_status), info);
			g_free (info);
		}
		break;

	case PV_MODE_LP: // ------------ Special Vector Setup for LP (Laserpuls)
		options      = (LP_option_flags & FLAG_FB_ON     ? 0      : VP_FEEDBACK_HOLD)
			     | (LP_option_flags & FLAG_INTEGRATE ? VP_AIC_INTEGRATE : 0);
		ramp_sources = LP_option_flags & FLAG_SHOW_RAMP ? Source : 0x000;

		recover_options=0;

		ramp_points = 10;

		make_delay_vector (LP_duration * 1e-3, Source, recover_options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_ZXYramp_vector (LP_FZ_end, 0., 0., ramp_points, LP_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_Vdx0_vector (bias, bias+LP_volt, 0., ramp_points, LP_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_delay_vector (LP_triggertime * 1e-3, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_Vdx0_vector (bias+LP_volt, bias, 0., ramp_points, LP_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);
			
		make_delay_vector (LP_final_delay * 1e-3, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		make_ZXYramp_vector (-LP_FZ_end, 0., 0., ramp_points, LP_slope, Source, recover_options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		dsp_vector.repetitions = LP_repetitions-1;
		dsp_vector.ptr_next = -6; // go to start
		write_dsp_vector (vector_index++);

		make_delay_vector (LP_final_delay * 1e-3, ramp_sources, recover_options, vp_duration, MAKE_VEC_FLAG_END);
		write_dsp_vector (vector_index++);
		append_null_vector (options, vector_index);

		sranger_hwi_hardware->probe_time_estimate = (int)vp_duration; // used for timeout check

                {
                        gchar *info=NULL;
                        info = g_strdup_printf ("T_total=%.2f ms, O*0x%02x S*0x%06x", 
                                                1e3*(double)vp_duration/frq_ref, options, Source
                                );
                        if (LP_status)
                                gtk_entry_set_text (GTK_ENTRY (LP_status), info);
                        g_free (info);
                }
                break;
		

	case PV_MODE_TS: // ------------ Special Vector Setup for TS (Time Spectroscopy)
		options      = (TS_option_flags & FLAG_FB_ON     ? 0      : VP_FEEDBACK_HOLD)
			     | (TS_option_flags & FLAG_INTEGRATE ? VP_AIC_INTEGRATE : 0);
		ramp_sources = TS_option_flags & FLAG_SHOW_RAMP ? Source : 0x000;

		make_delay_vector (TS_duration * 1e-3, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL, (int)TS_points);
		write_dsp_vector (vector_index++);

		dsp_vector.repetitions = TS_repetitions-1;
		dsp_vector.ptr_next = -1; // go to start
		write_dsp_vector (vector_index++);

		append_null_vector (options, vector_index);

		sranger_hwi_hardware->probe_time_estimate = (int)vp_duration; // used for timeout check

		{
			gchar *info=NULL;
			info = g_strdup_printf ("T_total=%.2f ms, O*0x%02x S*0x%06x", 
						1e3*(double)vp_duration/frq_ref, options, Source
				);
			if (TS_status)
				gtk_entry_set_text (GTK_ENTRY (TS_status), info);
			g_free (info);
		}
		break;


	case PV_MODE_LM: // ------------ Special Vector Setup for LM (lat manipulation)
		options      = (LM_option_flags & FLAG_FB_ON     ? 0      : VP_FEEDBACK_HOLD)
			     | (LM_option_flags & FLAG_INTEGRATE ? VP_AIC_INTEGRATE : 0);
		ramp_sources = LM_option_flags & FLAG_SHOW_RAMP ? Source : 0x000;

		// Ramp to initial Z from "current Z"
		make_ZXYramp_vector (LM_dz, LM_dx, LM_dy, LM_points, LM_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		// add vector for reverse return path?
		if (LM_option_flags & FLAG_DUAL) {
			make_ZXYramp_vector (-LM_dz, -LM_dx, -LM_dy, LM_points, LM_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			write_dsp_vector (vector_index++);
		}

		make_delay_vector (LM_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_END);
		write_dsp_vector (vector_index++);

		append_null_vector (options, vector_index);

		sranger_hwi_hardware->probe_time_estimate = (int)vp_duration; // used for timeout check

		{
			gchar *info = g_strdup_printf ("T=%.2f ms", 1e3*(double)vp_duration/frq_ref);
			if (LM_status)
				gtk_entry_set_text (GTK_ENTRY (LM_status), info);
			g_free (info);
		}
		break;

	case PV_MODE_AC: // ------------ Special Vector Setup for AC (phase probe)
		options      = (AC_option_flags & FLAG_FB_ON     ? 0      : VP_FEEDBACK_HOLD)
			     | (AC_option_flags & FLAG_INTEGRATE ? VP_AIC_INTEGRATE : 0);
		ramp_sources = AC_option_flags & FLAG_SHOW_RAMP ? Source : 0x000;

		if (AC_option_flags & FLAG_DUAL) {
			make_phase_vector (AC_phase_span, AC_points, AC_phase_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			write_dsp_vector (vector_index++);

			make_delay_vector (AC_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			write_dsp_vector (vector_index++);

			make_phase_vector (-AC_phase_span, AC_points, AC_phase_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			write_dsp_vector (vector_index++);

			make_delay_vector (AC_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			dsp_vector.repetitions = AC_repetitions-1;
			dsp_vector.ptr_next = -3; // go to start
			write_dsp_vector (vector_index++);
		} else {
			make_phase_vector (AC_phase_span, AC_points, AC_phase_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			write_dsp_vector (vector_index++);

			make_delay_vector (AC_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			dsp_vector.repetitions = AC_repetitions-1;
			dsp_vector.ptr_next = -1; // go to start
			write_dsp_vector (vector_index++);
		}

		make_delay_vector (AC_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_END);
		write_dsp_vector (vector_index++);
		append_null_vector (options, vector_index);

		sranger_hwi_hardware->probe_time_estimate = (int)vp_duration; // used for timeout check

		{
			gchar *info=NULL;
			info = g_strdup_printf ("T_total=%.2f ms, O*0x%02x S*0x%06x", 
						1e3*(double)vp_duration/frq_ref, options, Source
				);
			if (AC_status)
				gtk_entry_set_text (GTK_ENTRY (AC_status), info);
			g_free (info);
		}
		break;

	case PV_MODE_AX: // ------------ Special Vector Setup for AX (auxillary probe, QMA, CMA, ...)
		options      = (AX_option_flags & FLAG_FB_ON     ? 0      : VP_FEEDBACK_HOLD)
			     | (AX_option_flags & FLAG_INTEGRATE ? VP_AIC_INTEGRATE : 0);
		ramp_sources = AX_option_flags & FLAG_SHOW_RAMP ? Source : 0x000;

		make_Vdz_vector (bias, AX_start, 0., -1, AX_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
		write_dsp_vector (vector_index++);

		AX_slope = fabs(AX_end-AX_start)/(AX_gatetime*AX_points);

		make_Vdz_vector (AX_start, AX_end, 0., AX_points, AX_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		write_dsp_vector (vector_index++);

		// add vector for reverse return path?
		if (AX_option_flags & FLAG_DUAL) {
			make_Vdz_vector (AX_end, AX_start, 0., AX_points, AX_slope, Source, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			write_dsp_vector (vector_index++);

			make_Vdz_vector (AX_start, bias, 0., -1, AX_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_RAMP);
			write_dsp_vector (vector_index++);
		} else {
			make_Vdz_vector (AX_end, bias, 0., -1, AX_slope_ramp, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
			write_dsp_vector (vector_index++);
		}
			
		make_delay_vector (AX_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_NORMAL);
		dsp_vector.repetitions = AX_repetitions-1;
		dsp_vector.ptr_next = -3; // go to start
		write_dsp_vector (vector_index++);

		make_delay_vector (AX_final_delay, ramp_sources, options, vp_duration, MAKE_VEC_FLAG_END);
		write_dsp_vector (vector_index++);
		append_null_vector (options, vector_index);

		sranger_hwi_hardware->probe_time_estimate = (int)vp_duration; // used for timeout check

		{
			gchar *info = g_strdup_printf ("T=%.2f ms, Slope: %.4f V/s", 1e3*(double)vp_duration/frq_ref, AX_slope);
			if (AX_status)
				gtk_entry_set_text (GTK_ENTRY (AX_status), info);
			g_free (info);
		}
		break;
	}

	if (start){
                g_message ("Executing Vector Probe Now!");
		gapp->monitorcontrol->LogEvent ("VectorProbe", "Execute");
	}

	// --------------------------------------------------

	// now write probe structure, this may starts probe if "start" is true

	// update all from DSP
	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.probe, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_read  (sranger_hwi_hardware->dsp, &dsp_probe, sizeof (dsp_probe)); 
	// from DSP to PC
	conv_dsp_probe ();
	
	// update with changed user parameters
	dsp_probe.start = start ? 1:0;
	dsp_probe.AC_amp = DVOLT2AIC (AC_amp);
	dsp_probe.AC_frq = (int) (AC_frq);
	dsp_probe.AC_phaseA = (int)round(AC_phaseA*16.);
	dsp_probe.AC_phaseB = (int)round(AC_phaseB*16.);
	dsp_probe.AC_nAve = AC_lockin_avg_cycels;
	dsp_probe.Zpos = 0;

	// from PC to DSP
	conv_dsp_probe ();
	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.probe, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_write  (sranger_hwi_hardware->dsp, &dsp_probe,  MAX_WRITE_PROBE<<1); 
	// from DSP to PC
	conv_dsp_probe ();

	// read back
	read_dsp_probe ();
	// Update EC's
	update();
}

void DSPControl::conv_dsp_vector (){
	CONV_32 (dsp_vector.n);
	CONV_32 (dsp_vector.dnx);  
	CONV_32 (dsp_vector.srcs);
	CONV_32 (dsp_vector.options);
	CONV_16 (dsp_vector.ptr_fb);
	CONV_16 (dsp_vector.repetitions);
	CONV_16 (dsp_vector.i);
	CONV_16 (dsp_vector.j);
	CONV_16 (dsp_vector.ptr_next);
	CONV_16 (dsp_vector.ptr_final);
	CONV_32 (dsp_vector.f_du);
	CONV_32 (dsp_vector.f_dx);
	CONV_32 (dsp_vector.f_dy);
	CONV_32 (dsp_vector.f_dz);
	CONV_32 (dsp_vector.f_dx0);
	CONV_32 (dsp_vector.f_dy0);
	CONV_32 (dsp_vector.f_dphi);
}

void DSPControl::write_dsp_vector (int index){
        if (!sranger_hwi_hardware) return; 

	// update all from DSP
	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.probe, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_read  (sranger_hwi_hardware->dsp, &dsp_probe, sizeof (dsp_probe)); 
	// from DSP to PC
	conv_dsp_probe ();

	if (dsp_probe.vector_head < EXTERN_PROBE_VECTOR_HEAD_DEFAULT || index > 40 || index < 0){
		sranger_hwi_pi.app->message("Error writing Probe Vector:\n Bad vector address, aborting request.");
		return;
	}

	// check count ranges
	if (dsp_vector.dnx < 0 || dsp_vector.dnx > 32767 || dsp_vector.n < 0 || dsp_vector.n > 32767){
		GtkWidget *dialog = gtk_message_dialog_new (NULL,
							    GTK_DIALOG_DESTROY_WITH_PARENT,
							    GTK_MESSAGE_WARNING,
							    GTK_BUTTONS_CLOSE,
							    "Probe Vector [pc%02d] not acceptable:\n"
							    "n   = %6d   [0..32767]\n"
							    "dnx = %6d   [0..32767]\n"
							    "ATTENTION: May result in unpredictable probe actions.\n"
							    "AUTO-LIMITING FOR SAFETY.\n"
							    "Hint: adjust slope/speed/points.. to make fit.",
							    index, dsp_vector.n, dsp_vector.dnx  
			);
		g_signal_connect_swapped (G_OBJECT (dialog), "response",
					  G_CALLBACK (gtk_widget_destroy),
					  G_OBJECT (dialog));
		gtk_widget_show (dialog);

		if (dsp_vector.dnx < 0) dsp_vector.dnx = 0;
		if (dsp_vector.dnx > 32767) dsp_vector.dnx = 32767;
		if (dsp_vector.n < 0) dsp_vector.n = 0;
		if (dsp_vector.n > 32767) dsp_vector.n = 32767;

	}

	// setup VPC essentials
	dsp_vector.i = dsp_vector.repetitions; // preload repetitions counter now! (if now already done)
	dsp_vector.j = 0; // not yet used -- XXXX

	// update GXSM's internal copy of vector list
	dsp_vector_list[index].n = dsp_vector.n;
	dsp_vector_list[index].dnx = dsp_vector.dnx;
	dsp_vector_list[index].srcs = dsp_vector.srcs;
	dsp_vector_list[index].options = dsp_vector.options;
	dsp_vector_list[index].ptr_fb = dsp_vector.ptr_fb;
	dsp_vector_list[index].repetitions = dsp_vector.repetitions;
	dsp_vector_list[index].i = dsp_vector.i;
	dsp_vector_list[index].j = dsp_vector.j;
	dsp_vector_list[index].ptr_next = dsp_vector.ptr_next;
	dsp_vector_list[index].ptr_final = dsp_vector.ptr_final;
	dsp_vector_list[index].f_du = dsp_vector.f_du;
	dsp_vector_list[index].f_dx = dsp_vector.f_dx;
	dsp_vector_list[index].f_dy = dsp_vector.f_dy;
	dsp_vector_list[index].f_dz = dsp_vector.f_dz;
	dsp_vector_list[index].f_dx0 = dsp_vector.f_dx0;
	dsp_vector_list[index].f_dy0 = dsp_vector.f_dy0;
	dsp_vector_list[index].f_dphi = dsp_vector.f_dphi;

	{ 
		double mVf = 10000. / (65536. * 32768.);
		gchar *pvi = g_strdup_printf ("ProbeVector[pc%02d]", index);
		gchar *pvd = g_strdup_printf ("(n:%05d, dnx:%05d, 0x%04x, 0x%04x, r:%4d, pc:%d, f:%d),"
					      "(dU:%6.4f mV, dxzy:%6.4f, %6.4f, %6.4f mV, dxy0:%6.4f, %6.4f mV, dP:%.4f)", 
					      dsp_vector.n, dsp_vector.dnx,
					      dsp_vector.srcs, dsp_vector.options,
					      dsp_vector.repetitions,
					      dsp_vector.ptr_next, dsp_vector.ptr_final,
					      mVf * dsp_vector.f_du, mVf * dsp_vector.f_dx, mVf * dsp_vector.f_dy, mVf * dsp_vector.f_dz,
					      mVf * dsp_vector.f_dx0, mVf * dsp_vector.f_dy0,
					      dsp_vector.f_dphi / CONST_DSP_F16
			);

		gapp->monitorcontrol->LogEvent (pvi, pvd);
		g_free (pvi);
		g_free (pvd);
	}
	// from PC to to DSP format
        conv_dsp_vector();
	lseek (sranger_hwi_hardware->dsp, dsp_probe.vector_head + index*SIZE_OF_PROBE_VECTOR, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_write  (sranger_hwi_hardware->dsp, &dsp_vector, SIZE_OF_PROBE_VECTOR<<1);

	// from DSP to PC
        conv_dsp_vector();
}

void DSPControl::read_dsp_vector (int index){
        if (!sranger_hwi_hardware) return; 

	// update all from DSP
	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.probe, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_read  (sranger_hwi_hardware->dsp, &dsp_probe, sizeof (dsp_probe)); 
	// from DSP to PC
	conv_dsp_probe ();

	if (dsp_probe.vector_head < EXTERN_PROBE_VECTOR_HEAD_DEFAULT || index > 40 || index < 0){
		sranger_hwi_pi.app->message("Error reading Probe Vector:\n Bad vector address, aborting request.");
		return;
	}

	lseek (sranger_hwi_hardware->dsp, dsp_probe.vector_head + index*SIZE_OF_PROBE_VECTOR, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_read  (sranger_hwi_hardware->dsp, &dsp_vector, SIZE_OF_PROBE_VECTOR<<1);

	// from DSP to PC
        conv_dsp_vector();
}


