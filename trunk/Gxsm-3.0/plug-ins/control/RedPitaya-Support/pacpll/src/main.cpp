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

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <vector>
#include <sys/mman.h>
#include <fcntl.h>

#include "main.h"

// INSTALL:
// scp -r pacpll root@rp-f05603:/opt/redpitaya/www/apps/
// make clean; make INSTALL_DIR=/opt/redpitaya

// CHROME BROWSER NOTES: USER SCHIT-F5 to force reload of all data, else caches are fooling....

/*
 * RedPitaya A9 JSON Interface PARAMETERS and SIGNALS
 * ------------------------------------------------------------
 */



//Signal size
#define SIGNAL_SIZE_DEFAULT      1024
#define SIGNAL_UPDATE_INTERVAL     50


//Signal
// Block mode
CFloatSignal SIGNAL_CH1("SIGNAL_CH1", SIGNAL_SIZE_DEFAULT, 0.0f);
CFloatSignal SIGNAL_CH2("SIGNAL_CH2", SIGNAL_SIZE_DEFAULT, 0.0f);

// Slow from GPIO, stripe plotter mode
CFloatSignal SIGNAL_CH3("SIGNAL_CH3", SIGNAL_SIZE_DEFAULT, 0.0f);
std::vector<float> g_data_signal_ch3(SIGNAL_SIZE_DEFAULT);

CFloatSignal SIGNAL_CH4("SIGNAL_CH4", SIGNAL_SIZE_DEFAULT, 0.0f);
std::vector<float> g_data_signal_ch4(SIGNAL_SIZE_DEFAULT);

CFloatSignal SIGNAL_CH5("SIGNAL_CH5", SIGNAL_SIZE_DEFAULT, 0.0f);
std::vector<float> g_data_signal_ch5(SIGNAL_SIZE_DEFAULT);

CFloatSignal SIGNAL_FRQ("SIGNAL_FRQ", SIGNAL_SIZE_DEFAULT, 0.0f);
std::vector<float> g_data_signal_frq(SIGNAL_SIZE_DEFAULT);

CFloatSignal SIGNAL_TIME("SIGNAL_TIME", SIGNAL_SIZE_DEFAULT, 0.0f);
std::vector<float> g_data_signal_time(SIGNAL_SIZE_DEFAULT);

double signal_dc_measured = 0.0;

//Parameter
CIntParameter GAIN1("GAIN1", CBaseParameter::RW, 100, 0, 1, 10000);
CIntParameter GAIN2("GAIN2", CBaseParameter::RW, 100, 0, 1, 10000);
CIntParameter GAIN3("GAIN3", CBaseParameter::RW, 1000, 0, 1, 10000);
CIntParameter GAIN4("GAIN4", CBaseParameter::RW, 100, 0, 1, 10000);
CIntParameter GAIN5("GAIN5", CBaseParameter::RW, 100, 0, 1, 10000);

CIntParameter OPERATION("OPERATION", CBaseParameter::RW, 1, 0, 1, 10);
CIntParameter PACVERBOSE("PACVERBOSE", CBaseParameter::RW, 0, 0, 0, 10);

CIntParameter SHR_DEC_DATA("SHR_DEC_DATA", CBaseParameter::RW, 0, 0, 0, 24);

/*
 *  TRANSPORT FPGA MODULE to BRAM:
 *  S_AXIS1: M-AXIS-SIGNAL from LMS CH0 (input), CH1 (not used) -- pass through from ADC 14 bit
 *  S_AXIS2: M-AXIS-CONTROL Amplitude: output/monitor from Ampl. Controller (32 bit)
 *  S_AXIS3: M-AXIS-CONTROL Phase: output/monitor from Phase. Controller (48 bit)
 *  S_AXIS4: M-AXIS-CONTROL Phase pass, monitor Phase. from CORDIC ATAN X/Y (24 bit)
 *  S_AXIS5: M-AXIS-CONTROL Amplitude pass, monitor Amplitude from CORDIC SQRT (24 bit)
 *
 *  TRANSPORT MODE:
 *  BLOCK modes singel shot, fill after start, stop when full:
 *  0: S_AXIS1 CH1, CH2 decimated, aver aged (sum)
 *  1: S_AXIS4,5  decimated, averaged (sum)
 *  2: A_AXIS1 CH1 decimated, averaged (sum), ADDRreg
 *  3: TEST -- ch1n <= +/-1, ch2n <= +/-2;
 *  4: S_AXIS2,3 decimated64bit ==> Ampl 32bit , Frq-Flower 32bit;
 *  CONTINEOUS STEAM FIFO operation (loop, overwrite)
 *  ... modes to be added for FIFO contineous operation mode
 */

CIntParameter TRANSPORT_CH3("TRANSPORT_CH3", CBaseParameter::RW, 0, 0, 0, 13);
CIntParameter TRANSPORT_CH4("TRANSPORT_CH4", CBaseParameter::RW, 1, 0, 0, 13);
CIntParameter TRANSPORT_CH5("TRANSPORT_CH5", CBaseParameter::RW, 1, 0, 0, 13);
CIntParameter TRANSPORT_MODE("TRANSPORT_MODE", CBaseParameter::RW, 0, 0, 0, 32768);
CIntParameter TRANSPORT_DECIMATION("TRANSPORT_DECIMATION", CBaseParameter::RW, 2, 0, 1, 32768);

CIntParameter BRAM_WRITE_POS("BRAM_WRITE_POS", CBaseParameter::RW, 0, 0, -100, 1024);
CIntParameter BRAM_DEC_COUNT("BRAM_DEC_COUNT", CBaseParameter::RW, 0, 0, 0, 0xffffffff);

CFloatParameter DC_OFFSET("DC_OFFSET", CBaseParameter::RW, 0, 0, -1000.0, 1000.0); // mV

CDoubleParameter EXEC_MONITOR("EXEC_MONITOR", CBaseParameter::RW, 0, 0, -1000.0, 1000.0); // mV
CDoubleParameter DDS_FREQ_MONITOR("DDS_FREQ_MONITOR", CBaseParameter::RW, 0, 0, 0.0, 25e6); // Hz
CDoubleParameter PHASE_MONITOR("PHASE_MONITOR", CBaseParameter::RW, 0, 0, -180.0, 180.0); // deg
CDoubleParameter VOLUME_MONITOR("VOLUME_MONITOR", CBaseParameter::RW, 0, 0, -1000.0, 1000.0); // mV

CDoubleParameter CENTER_AMPLITUDE("CENTER_AMPLITUDE", CBaseParameter::RW, 0, 0, 0.0, 1000.0); // mV
CDoubleParameter CENTER_FREQUENCY("CENTER_FREQUENCY", CBaseParameter::RW, 0, 0, 0.0, 25e6); // Hz
CDoubleParameter CENTER_PHASE("CENTER_PHASE", CBaseParameter::RW, 0, 0, -180.0, 180.0); // deg

// PAC CONFIGURATION
//./pacpll -m s -f 32766.0 -v .5 -t 0.0004 -V 3
//./pacpll -m t -f 32766.0 -v .5 -t 0.0004 -M 1 -s 2.0 -d 0.05 -u 150000  > data-tune
CDoubleParameter FREQUENCY_TUNE("FREQUENCY_TUNE", CBaseParameter::RW, 32766.0, 0, 1, 25e6); // Hz
CDoubleParameter FREQUENCY_MANUAL("FREQUENCY_MANUAL", CBaseParameter::RW, 32766.0, 0, 1, 25e6); // Hz
CDoubleParameter VOLUME_MANUAL("VOLUME_MANUAL", CBaseParameter::RW, 300.0, 0, 0.0, 1000.0); // mV
CDoubleParameter PACTAU("PACTAU", CBaseParameter::RW, 200.0, 0, 0.0, 60e6); // us


CDoubleParameter TUNE_SPAN("TUNE_SPAN", CBaseParameter::RW, 5.0, 0, 0.1, 1e6); // Hz
CDoubleParameter TUNE_DFREQ("TUNE_DFREQ", CBaseParameter::RW, 0.05, 0, 0.0001, 1000.); // Hz

// PLL CONFIGURATION
CBooleanParameter AMPLITUDE_CONTROLLER("AMPLITUDE_CONTROLLER", CBaseParameter::RW, false, 0);
CBooleanParameter PHASE_CONTROLLER("PHASE_CONTROLLER", CBaseParameter::RW, false, 0);


//void rp_PAC_set_phase_controller64 (double setpoint, double cp, double ci, double upper, double lower)
CDoubleParameter AMPLITUDE_FB_SETPOINT("AMPLITUDE_FB_SETPOINT", CBaseParameter::RW, 20, 0, 0, 1000); // mV
CDoubleParameter AMPLITUDE_FB_CP("AMPLITUDE_FB_CP", CBaseParameter::RW, 0, 0, -1000, 1000);
CDoubleParameter AMPLITUDE_FB_CI("AMPLITUDE_FB_CI", CBaseParameter::RW, 0, 0, -1000, 1000);
CDoubleParameter EXEC_FB_UPPER("EXEC_FB_UPPER", CBaseParameter::RW, 500, 0, 0, 1000); // mV
CDoubleParameter EXEC_FB_LOWER("EXEC_FB_LOWER", CBaseParameter::RW, 0, 0, -1000, 1000); // mV

CDoubleParameter PHASE_FB_SETPOINT("PHASE_FB_SETPOINT", CBaseParameter::RW, 0, 0, -180, 180); // deg
CDoubleParameter PHASE_FB_CP("PHASE_FB_CP", CBaseParameter::RW, 0, 0, -1000, 1000); 
CDoubleParameter PHASE_FB_CI("PHASE_FB_CI", CBaseParameter::RW, 0, 0, -1000, 1000);
CDoubleParameter FREQ_FB_UPPER("FREQ_FB_UPPER", CBaseParameter::RW, 32768.0, 0, 0, 25e6); // Hz
CDoubleParameter FREQ_FB_LOWER("FREQ_FB_LOWER", CBaseParameter::RW, 0.1, 0, 0, 25e6); // Hz


CStringParameter pacpll_text("PAC_TEXT", CBaseParameter::RW, "N/A                                    ", 40);

CIntParameter updatePeriod("PERIOD", CBaseParameter::RW, 200, 0, 0, 50000);
CIntParameter timeDelay("time_delay", CBaseParameter::RW, 50000, 0, 0, 100000000);
CFloatParameter cpuLoad("CPU_LOAD", CBaseParameter::RW, 0, 0, 0, 100);
CFloatParameter memoryFree ("FREE_RAM", CBaseParameter::RW, 0, 0, 0, 1e15);
CDoubleParameter counter("COUNTER", CBaseParameter::RW, 1, 0, 1e-12, 1e+12);


/*
 * RedPitaya A9 FPGA Link
 * ------------------------------------------------------------
 */

const char *FPGA_PACPLL_A9_name = "/dev/mem";
void *FPGA_PACPLL_cfg  = NULL;
void *FPGA_PACPLL_bram = NULL;
size_t FPGA_PACPLL_CFG_block_size = 0;
size_t FPGA_PACPLL_BRAM_block_size = 0;

int verbose = 0;

//fprintf(stderr, "");

/*
 * RedPitaya A9 FPGA Memory Mapping Init
 * ------------------------------------------------------------
 */

int rp_PAC_App_Init(){
        int fd;
        FPGA_PACPLL_CFG_block_size  = 7*sysconf (_SC_PAGESIZE);   // 1024bit CFG Register
        FPGA_PACPLL_BRAM_block_size = 2048*sysconf(_SC_PAGESIZE); // Dual Ported FPGA BRAM

        if ((fd = open (FPGA_PACPLL_A9_name, O_RDWR)) < 0) {
                perror ("open");
                return RP_EOOR;
        }

        FPGA_PACPLL_bram = mmap (NULL, FPGA_PACPLL_BRAM_block_size,
                                 PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40000000);

        if (FPGA_PACPLL_bram == MAP_FAILED)
                return RP_EOOR;
  
        if (verbose > 1) fprintf(stderr, "RP FPGA_PACPLL BRAM: mapped %08lx - %08lx.\n", (unsigned long)(0x43000000), (unsigned long)(0x43000000 + FPGA_PACPLL_BRAM_block_size));

        
        FPGA_PACPLL_cfg = mmap (NULL, FPGA_PACPLL_CFG_block_size,
                                PROT_READ|PROT_WRITE,  MAP_SHARED, fd, 0x42000000);

        if (FPGA_PACPLL_cfg == MAP_FAILED)
                return RP_EOOR;

        if (verbose > 1) fprintf(stderr, "RP FPGA_PACPLL CFG: mapped %08lx - %08lx.\n", (unsigned long)(0x42000000), (unsigned long)(0x42000000 + FPGA_PACPLL_CFG_block_size));

        return RP_OK;
}

void rp_PAC_App_Release(){
        munmap (FPGA_PACPLL_cfg, FPGA_PACPLL_CFG_block_size);
        munmap (FPGA_PACPLL_bram, FPGA_PACPLL_BRAM_block_size);
}


/*
 * RedPitaya A9 FPGA Control and Data Transfer
 * ------------------------------------------------------------
 */

#define QN(N) ((1<<(N))-1)
#define QN64(N) ((1LL<<(N))-1)

#define Q22 QN(22)
#define Q23 QN(23)
#define Q24 QN(24)
#define Q13 QN(13)
#define QLMS QN(22)
#define BITS_CORDICSQRT 24
#define BITS_CORDICATAN 24
#define QCORDICSQRT QN(23) // W24: 1Q23
#define QCORDICATAN QN(21) // W24: 3Q21
#define QCORDICSQRTFIR QN64(32-2)
// #define QCORDICATANFIR (QN(24-3)*1024)
#define QCORDICATANFIR QN(32-3)

#define BITS_AMPL_CONTROL   32
#define BITS_PLHASE_CONTROL 48

#define Q31 0x7FFFFFFF  // (1<<31)-1 ... ov in expression expansion
#define Q32 0xFFFFFFFF  // (1<<32)-1 ... ov in expression expansion
#define Q40 QN64(40)
#define Q36 QN64(36)
#define Q37 QN64(37)
#define Q47 QN64(47)
#define Q44 QN64(44)
#define Q48 QN64(48)

#define QEXEC   Q31
#define QAMCOEF Q22
#define QFREQ   Q47
#define QPHCOEF Q31

long double cpu_values[4] = {0, 0, 0, 0}; /* reading only user, nice, system, idle */


inline void set_gpio_cfgreg_int32 (int cfg_slot, int value){
        size_t off = cfg_slot * 4;

        *((int32_t *)((uint8_t*)FPGA_PACPLL_cfg + off)) = value;

        if (verbose > 2) fprintf(stderr, "set_gpio32[CFG%d] int32 %04x = %08x %d\n", cfg_slot, off, value, value);
}

inline void set_gpio_cfgreg_uint32 (int cfg_slot, unsigned int value){
        size_t off = cfg_slot * 4;

        *((int32_t *)((uint8_t*)FPGA_PACPLL_cfg + off)) = value;

        if (verbose > 2) fprintf(stderr, "set_gpio32[CFG%d] uint32 %04x = %08x %d\n", cfg_slot, off, value, value);
}

inline void set_gpio_cfgreg_int64 (int cfg_slot, long long value){
        size_t off_lo = cfg_slot * 4;
        size_t off_hi = off_lo + 4;
        unsigned long long uv = (unsigned long long)value;
        if (verbose > 2) fprintf(stderr, "set_gpio64[CFG%d] int64 %04x = %08x %d\n", cfg_slot, off_lo, value, value);
        *((int32_t *)((uint8_t*)FPGA_PACPLL_cfg + off_lo)) = uv & 0xffffffff;
        *((int32_t *)((uint8_t*)FPGA_PACPLL_cfg + off_hi)) = uv >> 32;
}

 
// 48bit in 64cfg reg from top -- eventual precision bits=0
inline void set_gpio_cfgreg_int48 (int cfg_slot, unsigned long long value){
        set_gpio_cfgreg_int64 (cfg_slot, value << 16);
}
 

inline int32_t read_gpio_reg_int32_t (int gpio_block, int pos){
        size_t offset = gpio_block * 0x1000 + pos * 8;

        return *((int32_t *)((uint8_t*)FPGA_PACPLL_cfg + offset)); 
}

inline int read_gpio_reg_int32 (int gpio_block, int pos){
        size_t offset = gpio_block * 0x1000 + pos * 8;
        int x = *((int32_t *)((uint8_t*)FPGA_PACPLL_cfg + offset));
        return x;
}

inline unsigned int read_gpio_reg_uint32 (int gpio_block, int pos){
        size_t offset = gpio_block * 0x1000 + pos * 8;
        unsigned int x = *((int32_t *)((uint8_t*)FPGA_PACPLL_cfg + offset));
        return x;
}

 
#if 0
/*
 * SINE_SDB64 IP -- currently NOT IN USE!!!
 */
#define PACPLL_CFG_DSRE 0
#define PACPLL_CFG_DSIM 1
void rp_PAC_adjust_sdb64 (double hz){
        if (verbose > 2) fprintf(stderr, "RP FPGA_PACPLL CFG: mapped %08lx - %08lx.\n", (unsigned long)(0x42000000), (unsigned long)(0x42000000 + FPGA_PACPLL_CFG_block_size));

        double fclk = 125e6 / 32;
        double dphi = 2.*M_PI*(hz/fclk);
        double x;
        if (verbose > 1) fprintf(stderr, "## Adjust: f= %12.4f Hz\n", hz);

        set_gpio_cfgreg_int32 (PACPLL_CFG_DSRE, (int) (round (x = Q31 * cos (dphi))));
        if (verbose > 1) fprintf(stderr, "#dsRe= %12f\n", x); 
        set_gpio_cfgreg_int32 (PACPLL_CFG_DSIM, (int) (round (x = Q31 * sin (dphi))));
        if (verbose > 1) fprintf(stderr, "#dsIm= %12f\n", x); 

}
#endif

// Q44: 32766.0000 Hz -> phase_inc=4611404543  0000000112dc72ff
double dds_phaseinc (double freq){
        double fclk = 125e6;
        return Q44*freq/fclk;
}

double dds_phaseinc_to_freq (unsigned long long ddsphaseincQ44){
        double fclk = 125e6;
        return fclk*(double)ddsphaseincQ44/(double)(Q44);
}

#define PACPLL_CFG_DDS_PHASEINC 0
//#define PACPLL_CFG_DDS_PHASEINC_LO 0
//#define PACPLL_CFG_DDS_PHASEINC_HI 1
void rp_PAC_adjust_dds (double freq){
        // 44 Bit Phase, using 48bit tdata
        unsigned long long phase_inc = (unsigned long long)round (dds_phaseinc (freq));
        //        unsigned int lo32, hi32;
        if (verbose > 2) fprintf(stderr, "##Adjust: f= %12.4f Hz -> Q44 phase_inc=%lld  %016llx\n", freq, phase_inc, phase_inc);

        set_gpio_cfgreg_int48 (PACPLL_CFG_DDS_PHASEINC, phase_inc);
}

#define PACPLL_CFG_VOLUME_SINE 2
void rp_PAC_set_volume (double volume){
        if (verbose > 2) fprintf(stderr, "##Configure: volume= %g V\n", volume); 
        set_gpio_cfgreg_int32 (PACPLL_CFG_VOLUME_SINE, (int)(Q31 * volume));
}

#define PACPLL_CFG_CONTROL_LOOPS 3
void rp_PAC_configure_loops (int phase_ctrl, int am_ctrl){
        if (verbose > 2) fprintf(stderr, "##Configure loop controls: %x",  phase_ctrl ? 1:0 | am_ctrl ? 2:0); 
        set_gpio_cfgreg_int32 (PACPLL_CFG_CONTROL_LOOPS, (phase_ctrl ? 1:0) | (am_ctrl ? 2:0));
}

#define PACPLL_CFG_PACTAU 4
void rp_PAC_set_pactau (double tau){
        if (verbose > 2) fprintf(stderr, "##Configure: tau= %g  Q22: %d\n", tau, (int)(Q22 * tau)); 
        set_gpio_cfgreg_int32 (PACPLL_CFG_PACTAU, (int)(Q22 * tau)); // Q22 significant from top
}

#define PACPLL_CFG_DC_OFFSET 5
void rp_PAC_set_dcoff (double dc){
        if (verbose > 2) fprintf(stderr, "##Configure: dc= %g  Q22: %d\n", dc, (int)(Q22 * dc)); 
        set_gpio_cfgreg_int32 (PACPLL_CFG_DC_OFFSET, (int)(Q22 * dc));
}

void rp_PAC_auto_dc_offset_adjust (){
        double dc = 0.0;
        int x,i,k;
        double x1;

        rp_PAC_set_dcoff (0.0);
        usleep(10000);
        
        for (k=i=0; i<300000; ++i){
                if (i%777 == 0) usleep(1000);
                x = read_gpio_reg_int32 (2,0);
                x1=(double)x / QLMS;
                if (fabs(x1) < 0.5){
                        dc += x1;
                        ++k;
                }
        }
        dc /= k;
        if (verbose > 1) fprintf(stderr, "RP PACPLL DC-Offset = %10.6f  {%d}\n", dc, k);
        rp_PAC_set_dcoff (dc);

        signal_dc_measured = dc;
}


void rp_PAC_auto_dc_offset_correct (){
        int i,x;
        double x1;
        double q=0.0000001;
        double q1=1.0-q;
        for (i=0; i<30000; ++i){
                if (i%777 == 0) usleep(1000);
                x = read_gpio_reg_int32 (2,0);
                x1=(double)x / QLMS;
                if (fabs(x1) < 0.1)
                        signal_dc_measured = q1*signal_dc_measured + q*x1;
        }
        rp_PAC_set_dcoff (signal_dc_measured);
}


// +10 AM Controller
// +20 PHASE Controller
// +0 Set, +2 CP, +4 CI, +6 UPPER, +8 LOWER
#define PACPLL_CFG_PHASE_CONTROLLER 10
#define PACPLL_CFG_AMPLITUDE_CONTROLLER 20
#define PACPLL_CFG_SET   0
#define PACPLL_CFG_CP    1
#define PACPLL_CFG_CI    2
#define PACPLL_CFG_UPPER 3 // 3,4 64bit
#define PACPLL_CFG_LOWER 5 // 5,6 64bit

// AMPL from CORDIC: 24bit Q23 -- QCORDICSQRT
void rp_PAC_set_amplitude_controller (double setpoint, double cp, double ci, double upper, double lower){
        if (verbose > 2) fprintf(stderr, "##Configure Controller: set= %g  Q22: %d    cp=%g ci=%g upper=%g lower=%g\n", setpoint, (int)(Q22 * setpoint), cp, ci, upper, lower); 
        /*
        double Q = pll.Qfactor;     // Q-factor
        double F0 = pll.FSineHz; // Res. Freq
        double Fc = pll.auto_set_BW_Amp; // 1.5 .. 10Hz
        double gainres = pll.GainRes;
        double cp = 20. * log10 (0.08045   * Q*Fc / (gainres*F0));
        double ci = 20. * log10 (8.4243e-7 * Q*Fc*Fc / (gainres*F0));

        write_pll_variable32 (dsp_pll.icoef_Amp, pll.signum_ci_Amp * CPN(29)*pow (10.,pll.ci_gain_Amp/20.));
        // = ISign * CPN(29)*pow(10.,Igain/20.);
		
        write_pll_variable32 (dsp_pll.pcoef_Amp, pll.signum_cp_Amp * CPN(29)*pow (10.,pll.cp_gain_Amp/20.));
        // = PSign * CPN(29)*pow(10.,Pgain/20.);
        */
        set_gpio_cfgreg_int32 (PACPLL_CFG_AMPLITUDE_CONTROLLER + PACPLL_CFG_SET,   ((int)(QCORDICSQRT * setpoint))<<(32-BITS_CORDICSQRT));
        set_gpio_cfgreg_int32 (PACPLL_CFG_AMPLITUDE_CONTROLLER + PACPLL_CFG_CP,    ((int)(QAMCOEF * cp)));
        set_gpio_cfgreg_int32 (PACPLL_CFG_AMPLITUDE_CONTROLLER + PACPLL_CFG_CI,    ((int)(QAMCOEF * ci)));
        set_gpio_cfgreg_int32 (PACPLL_CFG_AMPLITUDE_CONTROLLER + PACPLL_CFG_UPPER, ((int)(QEXEC * upper)));
        set_gpio_cfgreg_int32 (PACPLL_CFG_AMPLITUDE_CONTROLLER + PACPLL_CFG_LOWER, ((int)(QEXEC * lower)));
}

void rp_PAC_set_phase_controller (double setpoint, double cp, double ci, double upper, double lower){
        if (verbose > 2) fprintf(stderr, "##Configure Controller: set= %g  Q22: %d    cp=%g ci=%g upper=%g lower=%g\n", setpoint, (int)(Q22 * setpoint), cp, ci, upper, lower); 

        /*
        double cp = 20. * log10 (1.6575e-5  * pll.auto_set_BW_Phase);
        double ci = 20. * log10 (1.7357e-10 * pll.auto_set_BW_Phase*pll.auto_set_BW_Phase);

        write_pll_variable32 (dsp_pll.icoef_Phase, pll.signum_ci_Phase * CPN(29)*pow (10.,pll.ci_gain_Phase/20.));
        // = ISign * CPN(29)*pow(10.,Igain/20.);
	
        write_pll_variable32 (dsp_pll.pcoef_Phase, pll.signum_cp_Phase * CPN(29)*pow (10.,pll.cp_gain_Phase/20.));
        // = PSign * CPN(29)*pow(10.,Pgain/20.);
        */

        set_gpio_cfgreg_int32 (PACPLL_CFG_PHASE_CONTROLLER + PACPLL_CFG_SET,   ((int)(QCORDICATAN * setpoint))<<(32-BITS_CORDICATAN));
        set_gpio_cfgreg_int32 (PACPLL_CFG_PHASE_CONTROLLER + PACPLL_CFG_CP,    (long long)(QPHCOEF * cp)); // 22+1 bit error, 32bit CP,CI << 31 
        set_gpio_cfgreg_int32 (PACPLL_CFG_PHASE_CONTROLLER + PACPLL_CFG_CI,    (long long)(QPHCOEF * ci));
        set_gpio_cfgreg_int48 (PACPLL_CFG_PHASE_CONTROLLER + PACPLL_CFG_UPPER, (unsigned long long)round (dds_phaseinc (upper))); // => 44bit phase
        set_gpio_cfgreg_int48 (PACPLL_CFG_PHASE_CONTROLLER + PACPLL_CFG_LOWER, (unsigned long long)round (dds_phaseinc (lower)));
}
/*
        OPERATION::
        start   <= operation[0:0]; // Trigger start for single shot
        init    <= operation[7:4]; // reset/reinit
        shr_dec_data <= operation[31:8]; // DEC data shr for ch1..4

 */
#define PACPLL_CFG_TRANSPORT_CONTROL         6
#define PACPLL_CFG_TRANSPORT_SAMPLES         7
#define PACPLL_CFG_TRANSPORT_DECIMATION      8
#define PACPLL_CFG_TRANSPORT_CHANNEL_SELECT  9
#define PACPLL_CFG_TRANSPORT_AUX_SCALE       17
#define PACPLL_CFG_TRANSPORT_INIT            (1<<4)
#define PACPLL_CFG_TRANSPORT_START           1
#define PACPLL_CFG_TRANSPORT_LOOP            3
#define PACPLL_CFG_TRANSPORT_XXXXX           0

void rp_PAC_configure_transport (int control, int shr_dec_data, int nsamples, int decimation, int channel_select){
        if (verbose > 2) fprintf(stderr, "##Configure transport: 0x%02x, dec=%d, M=%d\n",  control, decimation, channel_select); 
        set_gpio_cfgreg_uint32 (PACPLL_CFG_TRANSPORT_CONTROL,
                                (control & 0xff)
                                | (((shr_dec_data >= 0 && shr_dec_data <= 24) ? shr_dec_data : 0) << 8)
                                );
        set_gpio_cfgreg_int32 (PACPLL_CFG_TRANSPORT_SAMPLES, nsamples);
        if (decimation < 8)
                decimation = 8;
        set_gpio_cfgreg_int32 (PACPLL_CFG_TRANSPORT_DECIMATION, decimation);
        set_gpio_cfgreg_int32 (PACPLL_CFG_TRANSPORT_CHANNEL_SELECT, channel_select);
        // AUX scale, Q15, in top 32
        set_gpio_cfgreg_int32 (PACPLL_CFG_TRANSPORT_AUX_SCALE, (32767)<<16);
}

/*
 * Get Single Direct FPGA Reading via GPIO
 * ========================================
 * reading_vector[0] := LMS Amplitude estimation (from A,B)
 * reading_vector[1] := LMS Phase estimation (from A,B)
 * reading_vector[2] := LMS A
 * reading_vector[3] := LMS B
 * reading_vector[4] := FPGA CORDIC Amplitude Monitor
 * reading_vector[5] := FPGA CORDIC Phase Monitor
 * reading_vector[6] := x5
 * reading_vector[7] := x6
 * reading_vector[8] := x7 Exec Amp Mon
 * reading_vector[9] := DDS Freq
 * reading_vector[10]:= x3 Monitor (In0 Signal, LMS inpu)
 * reading_vector[11]:= x3
 * reading_vector[12]:= x11
 * reading_vector[13]:= x12
 */

#define READING_MAX_VALUES 14

void rp_PAC_get_single_reading (double reading_vector[READING_MAX_VALUES]){
        int x,y,xx7,xx8,xx9,uix;
        double a,b,v,p,x3,x4,x5,x6,x7,x8,x9,x10,qca,pfpga,x11,x12;
        
        x = read_gpio_reg_int32 (1,0); // GPIO X1 : LMS A (cfg + 0x1000)
        a=(double)x / QLMS;
        x = read_gpio_reg_int32 (1,1); // GPIO X2 : LMS B (cfg + 0x1008)
        b=(double)x / QLMS;
        v=sqrt (a*a+b*b);
        p=atan2 ((a-b),(a+b))/M_PI*180. - 90.;
        p/=180.; // for testing
        
        x = read_gpio_reg_int32 (2,0); // GPIO X3 : LMS DBG1 :: M (LMS input Signal) (cfg + 0x2000) ===> used for DC OFFSET calculation
        x3=(double)x / QLMS;
        x = read_gpio_reg_int32 (2,1); // GPIO X4 : CORDIC SQRT (AM2=A^2+B^2) = Amplitude Monitor
        x4=(double)x / QCORDICSQRT;
        //x4=(double)x / QEXEC;
        x = read_gpio_reg_int32 (3,0); // GPIO X5 : XXX -- CORDIC SQRT = Amplitude after FIR -- experimental, removed FIR
        x5=(double)x / QCORDICSQRTFIR;
        x = read_gpio_reg_int32 (3,1); // GPIO X6 : XXX --- CORDIC ATAN(X/Y) = Phase after FIR -- experimental, removed FIR
        x6=(double)x / QCORDICATANFIR;
        xx7 = x = read_gpio_reg_int32 (4,0); // GPIO X7 : Exec Ampl Control Signal (signed)
        x7=(double)x / QEXEC;
        xx8 = x = read_gpio_reg_int32 (4,1); // GPIO X8 : DDS Phase Inc (Freq.) upper 32 bits of 44 (signed)
        xx9 = x = read_gpio_reg_int32 (5,0); // GPIO X9 : DDS Phase Inc (Freq.) lower 32 bits of 44 (signed)
        x9=(double)x / QLMS;
        uix = read_gpio_reg_uint32 (5,1); // GPIO X10: CORDIC ATAN(X/Y) = Phase Monitor
        x10 = (double)uix / QCORDICATAN; // ATAN 24bit 3Q21 
        //x10=(qca=(double)x / QCORDICATAN/M_PI*180.) - 90. - (x8<0? 230:0.); // ???? WHY NOT 180 ????
        //x10 /= 180.; // for testing 100 ==== 180
                  
        //pfpga=atan2 (x8,x7)/M_PI*180. - 90.;
        //pfpga /= 180.;
        y=x = read_gpio_reg_int32 (6,0); // GPIO X11 : Transport WPos
        x11=(double)(x&0xffff);
        x = read_gpio_reg_int32 (6,1); // GPIO X12 : Transport Dbg
        x12=(double)((x>>16)&0xffff);

        // LMS Detector Readings and double precision conversions
        reading_vector[0] = v; // LMS Amplitude (Volume) in Volts (from A,B)
        reading_vector[1] = p; // LMS Phase (from A,B)
        reading_vector[2] = a; // LMS A (Real)
        reading_vector[3] = b; // LMS B (Imag)

        // FPGA CORDIC based convertions
        reading_vector[4] = x4;  // FPGA CORDIC (SQRT) Amplitude Monitor
        reading_vector[5] = x10; // FPGA CORDIC (ATAN) Phase Monitor

        reading_vector[6] = x5;  // FPGA CORDIC FIR Amplitude
        reading_vector[7] = x6;  // FPGA CORDIC FIR Phase !! <-CHECK FPGA CONFIG

        reading_vector[8] = x7;  // Exec Ampl Control Signal (signed)
        reading_vector[9] = dds_phaseinc_to_freq(((long long)xx8<<(44-32)) + ((long long)xx9>>(64-44)));  // DDS Phase Inc (Freq.) upper 32 bits of 44 (signed)

        reading_vector[10] = x3; // M (LMS input Signal)
        reading_vector[11] = x3; // M1 (LMS input Signal-DC), tests...

        reading_vector[12] = x11; // test (bram write pos from data transport)
        reading_vector[13] = x12; // test (bram debug from data transport)

        // x11: bram write pos (y)
        // x12: transport dbg  (x)
        // assign writeposition = { {(32-BRAM_ADDR_WIDTH){1'b0}}, int_addrA_reg };
        // assign debug = { {(2){1'b0}}, operation[12:8],  operation[7:0],     {(5){1'b0}}, finished_state,trigger,running,   {(1){1'b0}}, bramwr_sms, {(1){1'b0}}, dec_sms };
        //start   <= operation[0:0]; // Trigger start for single shot
        //init    <= operation[7:4]; // reset/reinit
        //ch1_shr <= operation[12:8]; // DEC data shr ch1,..4
        //ch2_shr <= operation[20:16];
        //ch3_shr <= operation[28:24];
        //ch4_shr <= operation[28:24];
        if (verbose == 1) fprintf(stderr, "PAC TRANSPORT DBG: BRAM WritePos=%08X DBG=%08x DECSMS=%d BRAMSMS=%d run=%d trig=%d fin=%d op=%d ops=%d opo=%d ch1shr=%d\n",
                                  y, x,
                                  x&0x7, (x>>4)&7,
                                  (x>>8)&1, (x>>9)&1, (x>>10)&1,
                                  (x>>16)&0xf, (x>>16)&1, (x>>20)&15, (x>>24)&0x63
                                  );
        else if (verbose > 4) fprintf(stderr, "PAC DBG: A:%12f  B:%12f X1:%12f X2:%12f X3:%12f X4:%12f X5:%12f X6:%12f X7:%12f X8:%12f X9:%12f X10:%12f X11:%12f X12:%12f\n", a,b,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12);
        else if (verbose > 3) fprintf(stderr, "PAC READING: Ampl=%10.4f V Phase=%10.4f deg a=%10.4f b=%10.4f  FPGA: %10.4f %10.4f FIR: %10.4f %10.4f  M %10.4f V  pfpga=%10.4f\n", v,p, a,b, x4, x10, x5, x6, x3, pfpga);
}
        

int bram_status(int bram_status[3]){
        int x11 = read_gpio_reg_int32 (6,0); // GPIO X11 : Transport DecCnt, 0,0, Wpos
        int x12 = read_gpio_reg_int32 (6,1); // GPIO X12 : Tranmsport Dbg
        bram_status[0] = x11 & 0xffff; // GPIO X11 : Transport WPos
        bram_status[1] = (x11>>16) & 0xffff; // GPIO X11 : Transport DecCount
        bram_status[2] = x12;          // GPIO X12 : Transport Dbg
        return ((bram_status[2]>>10) & 1)  &&  ((bram_status[2]>>8) & 1); // finished (finished and run set)
}


/*
 * RedPitaya A9 JSON Interface
 * ------------------------------------------------------------
 */



// Generator config
void set_PAC_config()
{
        if (OPERATION.Value() != 6)
                rp_PAC_adjust_dds (FREQUENCY_MANUAL.Value());
        rp_PAC_set_volume (VOLUME_MANUAL.Value() / 1000.); // mV -> V
        rp_PAC_set_pactau (PACTAU.Value() * 1e-6); // us -> s

        rp_PAC_set_amplitude_controller (
                                         AMPLITUDE_FB_SETPOINT.Value ()/1000., // mv to V
                                         AMPLITUDE_FB_CP.Value (),
                                         AMPLITUDE_FB_CI.Value (),
                                         EXEC_FB_UPPER.Value ()/1000., // mV -> +/-1V
                                         EXEC_FB_LOWER.Value ()/1000.
                                         );

        rp_PAC_set_phase_controller (
                                     PHASE_FB_SETPOINT.Value ()/180.*M_PI, // Phase
                                     PHASE_FB_CP.Value (),
                                     PHASE_FB_CI.Value (),
                                     FREQ_FB_UPPER.Value (), // Hz
                                     FREQ_FB_LOWER.Value ()
                                     );
        
        rp_PAC_configure_loops (PHASE_CONTROLLER.Value ()?1:0, AMPLITUDE_CONTROLLER.Value ()?1:0);
}




const char *rp_app_desc(void)
{
        return (const char *)"Red Pitaya PACPLL.\n";
}


int rp_app_init(void)
{
        double reading_vector[READING_MAX_VALUES];

        fprintf(stderr, "Loading PACPLL application\n");

        // Initialization of PAC api
        if (rp_PAC_App_Init() != RP_OK) 
                {
                        fprintf(stderr, "Red Pitaya PACPLL API init failed!\n");
                        return EXIT_FAILURE;
                }
        else fprintf(stderr, "Red Pitaya PACPLL API init success!\n");

        rp_PAC_auto_dc_offset_adjust ();

        //Set signal update interval
        CDataManager::GetInstance()->SetSignalInterval(SIGNAL_UPDATE_INTERVAL);

        // Init PAC
        set_PAC_config();

        rp_PAC_get_single_reading (reading_vector);

        // init block transport for scope
        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,  SHR_DEC_DATA.Value (), 1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value ());
        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START, SHR_DEC_DATA.Value (), 1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value ());
        
        return 0;
}


int rp_app_exit(void)
{
        fprintf(stderr, "Unloading Red Pitaya PACPLL application\n");

        rp_PAC_App_Release();
        
        return 0;
}


int rp_set_params(rp_app_params_t *p, int len)
{
        return 0;
}


int rp_get_params(rp_app_params_t **p)
{
        return 0;
}


int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
        return 0;
}


//READ
//return *((volatile uint32_t *) ( ((uint8_t*)map) + offset ));

//WRITE
//*((volatile uint32_t *) ( ((uint8_t*)map)+ offset )) = value;

/*

                       0: // S_AXIS1: 32: {16: ADC-IN1, 16: ADC-IN2 }
                        begin
                            if (S_AXIS1_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS1_tdata[15:0]);          // IN1 (16bit data, 14bit ADC) => 32
                                ch2n <= $signed(S_AXIS1_tdata[16+15:16+0]);    // IN2 (16bit data, 14bit ADC) => 32
                                dec_sms_next <= 3'd3;
                            end
                        end
                        1:
                        begin
                            if (S_AXIS4_tvalid && S_AXIS5_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS4_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Phase (24) =>  32
                                ch2n <= $signed(S_AXIS5_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                dec_sms_next <= 3'd3;
                            end
                        end
                        2:
                        begin
                            if (S_AXIS1_tvalid && S_AXIS5_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS1_tdata[15:0]);                    // IN1 Signal with
                                ch2n <= $signed(S_AXIS5_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                dec_sms_next <= 3'd3;
                            end
                        end
                        3:
                        begin
                            if (S_AXIS1_tvalid && S_AXIS4_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS1_tdata[15:0]);                    // IN1 Signal with
                                ch2n <= $signed(S_AXIS4_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Phase (24) =>  32
                                dec_sms_next <= 3'd3;
                            end
                        end
                        4:
                        begin
                            if (S_AXIS2_tvalid && S_AXIS3_tvalid)
                            begin
                                ch1n <= S_AXIS2_tdata[SAXIS_2_DATA_WIDTH-1:0];                 // Amplitude Exec (32) =>  64 sum
                                ch2n <= (S_AXIS3_tdata[SAXIS_3_DATA_WIDTH-1:0] - axis3_lower); // Freq (48) - Lower (48) =>  64 sum
                                dec_sms_next <= 3'd3;
                            end
                        end
                        5:
                        begin
                            if (S_AXIS7_tvalid && S_AXIS8_tvalid)
                            begin
                                ch1n <= S_AXIS7_tdata[SAXIS_78_DATA_WIDTH-1:0]; // FIR Ampl
                                ch2n <= S_AXIS8_tdata[SAXIS_78_DATA_WIDTH-1:0]; // FIR Phase
                                dec_sms_next <= 3'd3;
                            end
                        end
                        6:
                        begin
                            ch1n <= ch1 + mk3_pixel_clock; // keep counting!
                            ch2n <= ch1 + mk3_line_clock;
                            dec_sms_next <= 3'd3;
                        end
                        7:
                        begin
                            ch1 <= 0; // just set zero!
                            ch2 <= 0;
                            dec_sms_next <= 3'd3;
                        end
                        8:
                        begin
                            ch1n <= rp_digital_in;
                            ch2n <= mk3_pixel_clock;
                            dec_sms_next <= 3'd3;
                        end
                    endcase

 */



void read_bram (int n, int dec, int t_mode, double gain1, double gain2){
        int k;
        size_t i = 12;
        size_t N = 8*n;
        static float dc=0.;
        float val;
        float dc_new=0.;
        
        switch (t_mode){
        case 0: // AXIS1xy :  IN1, IN2
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // IN1 (14)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // IN2 (14)
                        // printf ("%d %8.6f %8.6f\n", i>>3, (double)ix32/Q22, (double)ix32/Q22);
                        SIGNAL_CH1[k] = (float)ix32*gain1/Q13;
                        SIGNAL_CH2[k] = (float)iy32*gain2/Q13;
                }
                break;
        case 1: // AXIS45 :  CORDIC SQRT, ATAN
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Phase (24)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Ampl (24)
                        SIGNAL_CH1[k] = (float)ix32*gain1/QCORDICATAN;
                        SIGNAL_CH2[k] = (float)iy32*gain2/QCORDICSQRT;
                }
                break;
        case 2: // IN1, ADDR
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // IN1 (14)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Ampl (24)
                        SIGNAL_CH1[k] = (float)ix32*gain1/Q13;
                        val  = (float)iy32*gain2/QCORDICSQRT;
                        dc_new += val;
                        SIGNAL_CH2[k] = val-dc;
                }
                dc = dc_new/SIGNAL_SIZE_DEFAULT;
                break;
        case 3:
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // IN1 (14)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Phase (24)
                        SIGNAL_CH1[k]  = (float)ix32*gain1;
                        val  = (float)iy32*gain2/QCORDICATAN;
                        dc_new += val;
                        SIGNAL_CH2[k]  = val-dc;
                }
                dc = dc_new/SIGNAL_SIZE_DEFAULT;
                break;
        case 4:
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Ampl Exec (32)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Freq (48)-Lower(48)
                        SIGNAL_CH1[k]  = (float)ix32 / QEXEC;
                        SIGNAL_CH2[k]  = (float)dds_phaseinc_to_freq ((double)iy32); // correct to 44
                }
                break;
        case 5:
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // FIR Ampl (experimental)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // FIR Phase (experimental)
                        SIGNAL_CH1[k]  = (float)ix32*gain1/QCORDICSQRT;
                        SIGNAL_CH2[k]  = (float)iy32*gain2/QCORDICATAN;
                }
                break;
        case 6:
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // pixel clock counter, keeps counting, reset via mode 7
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // line clock counter, ..."" "" ""
                        SIGNAL_CH1[k]  = (float)ix32*gain1;
                        SIGNAL_CH2[k]  = (float)iy32*gain2;
                }
                break;
        default : break;
        }
}



void UpdateSignals(void)
{
        static int state=0;
        static int dir=1;
        static double f=0.;
        static double tune_amp_max=0.;
        static double tune_phase=0.;
        static double tune_fcenter=0.;
        double ampl, phase;
        double reading_vector[READING_MAX_VALUES];
        int ch;
        int status[3];
        
        if (verbose > 3) fprintf(stderr, "UpdateSignals()\n");

        if ( OPERATION.Value () == 2 || OPERATION.Value () == 6){
                int n=1024;
                if (verbose > 3) fprintf(stderr, "re-arm read BRAM writer\n");
                if (verbose == 1) fprintf(stderr, "BRAM read:\n");
                read_bram (SIGNAL_SIZE_DEFAULT, TRANSPORT_DECIMATION.Value (),  TRANSPORT_MODE.Value (), GAIN1.Value (), GAIN2.Value ());
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
                if (bram_status(status)){
                        if (verbose == 1) fprintf(stderr, "BRAM T init:\n");
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,  SHR_DEC_DATA.Value (), n, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value ());
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                        if (verbose == 1) fprintf(stderr, "BRAM T start:\n");
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START, SHR_DEC_DATA.Value (), n, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value ());
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                }
                BRAM_WRITE_POS.Value () = status[0];
                BRAM_DEC_COUNT.Value () = status[1];
        }

        if ( OPERATION.Value () == 5 ){ // LOOP READ (FIFO MODE)
                int n=1024;
                bram_status(status);
                read_bram (SIGNAL_SIZE_DEFAULT, TRANSPORT_DECIMATION.Value (),  TRANSPORT_MODE.Value (), GAIN1.Value (), GAIN2.Value ());
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
                BRAM_WRITE_POS.Value () = status[0];
                BRAM_DEC_COUNT.Value () = status[1];
        }
        
        // TUNE MODE
        if ( OPERATION.Value () == 6 ){
                
                ampl = reading_vector[4] * 1000.; // Resonator Amplitude Signal Monitor in mV
                phase = reading_vector[5] * 180./M_PI; // PLL Phase deg

                if (ampl > tune_amp_max){
                        tune_amp_max = ampl;
                        tune_phase = phase;
                        tune_fcenter = FREQUENCY_MANUAL.Value() + f;
                }
                
                if (f < TUNE_SPAN.Value ()/2 && dir == 1)
                        f += TUNE_DFREQ.Value ();
                if (f > TUNE_SPAN.Value ()/2 && dir == 1){
                        dir = -1;
                        CENTER_FREQUENCY.Value () = tune_fcenter;
                        CENTER_PHASE.Value () = tune_phase;
                        CENTER_AMPLITUDE.Value () = tune_amp_max;
                        tune_amp_max=0.;
                }
                if (f > -TUNE_SPAN.Value ()/2 && dir == -1)
                        f -= TUNE_DFREQ.Value ();
                if (f < -TUNE_SPAN.Value ()/2 && dir == -1){
                        dir = 1;
                        CENTER_FREQUENCY.Value () = tune_fcenter;
                        CENTER_PHASE.Value () = tune_phase;
                        CENTER_AMPLITUDE.Value () = tune_amp_max;
                        tune_amp_max=0.;
                }
                rp_PAC_adjust_dds (FREQUENCY_MANUAL.Value() + f);
                FREQUENCY_TUNE.Value() = f;
        } else {
                f = 0.; dir = 1;
        }
        g_data_signal_frq.erase (g_data_signal_frq.begin());
        g_data_signal_frq.push_back (f);
        
        if (verbose > 3) fprintf(stderr, "UpdateSignals get GPIO reading:\n");

        rp_PAC_get_single_reading (reading_vector);

        // Slow GPIO MONITOR in strip plotter mode
        // Push it to vector
        ch = TRANSPORT_CH3.Value ();
        if (verbose > 3) fprintf(stderr, "UpdateSignals: CH3=%d \n", ch);
        g_data_signal_ch3.erase (g_data_signal_ch3.begin());
        g_data_signal_ch3.push_back (reading_vector[ch >=0 && ch < READING_MAX_VALUES ? ch : 0] * GAIN3.Value ());

        ch = TRANSPORT_CH4.Value ();
        if (verbose > 3) fprintf(stderr, "UpdateSignals: CH4=%d \n", ch);
        g_data_signal_ch4.erase (g_data_signal_ch4.begin());
        g_data_signal_ch4.push_back (reading_vector[ch >=0 && ch < READING_MAX_VALUES ? ch : 1] * GAIN4.Value ());

        ch = TRANSPORT_CH5.Value ();
        if (verbose > 3) fprintf(stderr, "UpdateSignals: CH5=%d \n", ch);
        g_data_signal_ch5.erase (g_data_signal_ch5.begin());
        g_data_signal_ch5.push_back (reading_vector[ch >=0 && ch < READING_MAX_VALUES ? ch : 1] * GAIN5.Value ());

        // Copy data to signals
        if (verbose > 3) fprintf(stderr, "UpdateSignals copy signals\n");
        for (int i = 0; i < SIGNAL_SIZE_DEFAULT; i++){
                SIGNAL_CH3[i] = g_data_signal_ch3[i];
                SIGNAL_CH4[i] = g_data_signal_ch4[i];
                SIGNAL_CH5[i] = g_data_signal_ch5[i];
                if (OPERATION.Value () == 6)
                        SIGNAL_FRQ[i] = g_data_signal_frq[i];
        }
        
        if (verbose > 3) fprintf(stderr, "UpdateSignal complete.\n");

        VOLUME_MONITOR.Value ()   = reading_vector[4] * 1000.; // Resonator Amplitude Signal Monitor in mV
        PHASE_MONITOR.Value ()    = reading_vector[5] * 180./M_PI; // PLL Phase deg
        EXEC_MONITOR.Value ()     = reading_vector[8] * 1000.; // Exec Amplitude Monitor in mV
        DDS_FREQ_MONITOR.Value () = reading_vector[9]; // DDS Freq Monitor in Hz
        
        rp_PAC_auto_dc_offset_correct ();
}

void UpdateParams(void){
        if (verbose > 3) fprintf(stderr, "UpdateParams()\n");
	CDataManager::GetInstance()->SetParamInterval(updatePeriod.Value());
	CDataManager::GetInstance()->SetSignalInterval(updatePeriod.Value());

        DC_OFFSET.Value() = (float)(1000.*signal_dc_measured); // mV
   	
        FILE *fp = fopen("/proc/stat","r");
        if(fp) {
                long double a[4];
                int ret=fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
                fclose(fp);

                if (ret == 4){
                        long double divider = ((a[0]+a[1]+a[2]+a[3]) - (cpu_values[0]+cpu_values[1]+cpu_values[2]+cpu_values[3]));
                        long double loadavg = 100;
                        if(divider > 0.01) {
                                loadavg = ((a[0]+a[1]+a[2]) - (cpu_values[0]+cpu_values[1]+cpu_values[2])) / divider;
                        }
                        cpuLoad.Value() = (float)(loadavg * 100);
                        cpu_values[0]=a[0];cpu_values[1]=a[1];cpu_values[2]=a[2];cpu_values[3]=a[3];
                } else {
                       cpuLoad.Value() = (float)(-1); // ERROR EVALUATING LOAD
                }
                
        }
    
        struct sysinfo memInfo;
        sysinfo (&memInfo);
        memoryFree.Value() = (float)memInfo.freeram;

        counter.Value()=counter.Value()+(double)updatePeriod.Value()/1000.0;

        if (verbose > 3) fprintf(stderr, "UpdateParams: text update\n");
        pacpll_text.Value() = "Hello this is the RP PACPLL Server.    ";

        if (counter.Value()>1000) {
                counter.Value()=0;
        }
        if (verbose > 3) fprintf(stderr, "UpdateParams complete.\n");
}


void OnNewParams(void)
{
        if (verbose > 3) fprintf(stderr, "OnNewParams()\n");
        PACVERBOSE.Update ();
        OPERATION.Update ();

        GAIN1.Update ();
        GAIN2.Update ();
        GAIN3.Update ();
        GAIN4.Update ();
        GAIN5.Update ();
        SHR_DEC_DATA.Update ();
        TUNE_SPAN.Update ();
        TUNE_DFREQ.Update ();
        
        FREQUENCY_MANUAL.Update ();
        VOLUME_MANUAL.Update ();
        PACTAU.Update ();
        PHASE_CONTROLLER.Update ();
        AMPLITUDE_CONTROLLER.Update ();

        AMPLITUDE_FB_SETPOINT.Update ();
        AMPLITUDE_FB_CP.Update ();
        AMPLITUDE_FB_CI.Update ();
        EXEC_FB_UPPER.Update ();
        EXEC_FB_LOWER.Update ();

        PHASE_FB_SETPOINT.Update ();
        PHASE_FB_CP.Update ();
        PHASE_FB_CI.Update ();
        FREQ_FB_UPPER.Update ();
        FREQ_FB_LOWER.Update ();
        
        if (verbose > 3) fprintf(stderr, "OnNewParams: verbose=%d\n", PACVERBOSE.Value ());
        verbose = PACVERBOSE.Value ();

        TRANSPORT_DECIMATION.Update ();
        TRANSPORT_MODE.Update ();
        TRANSPORT_CH3.Update ();
        TRANSPORT_CH4.Update ();
        TRANSPORT_CH5.Update ();

        /*
        if (verbose > 3) fprintf(stderr,
                                 "NewParams: v:%d o:%d g[%d %d %d %d]\n"
                                 "   f:%f v:%f tau:%f\n"
                                 "   pc:%d ac:%d\n"
                                 "   td:%d tm:%d tc3:%d tc4:%d\n",
                                 PACVERBOSE.Value (),
                                 OPERATION.Value (),
                                 
                                 GAIN1.Value (),
                                 GAIN2.Value (),
                                 GAIN3.Value (),
                                 GAIN4.Value (),
        
                                 FREQUENCY_MANUAL.Value (),
                                 VOLUME_MANUAL.Value (),
                                 PACTAU.Value (),
                                 
                                 PHASE_CONTROLLER.Value (),
                                 AMPLITUDE_CONTROLLER.Value (),

                                 TRANSPORT_DECIMATION.Value (),
                                 TRANSPORT_MODE.Value (),
                                 TRANSPORT_CH3.Value (),
                                 TRANSPORT_CH4.Value ()
                                 );             
        */
        if ( OPERATION.Value () == 1 ){
                
                rp_PAC_auto_dc_offset_adjust ();
                OPERATION.Value () = 0;
                if (verbose > 0) fprintf(stderr, "OnNewParams: OP=1 Auto Offset Run, DC=%f mV\n", 1000.*signal_dc_measured);
                pacpll_text.Value() = "Auto Offset completed.                 ";
        }
        if ( OPERATION.Value () == 3 ){
                double reading_vector[READING_MAX_VALUES];
                OPERATION.Value () = 0;
                if (verbose > 0) fprintf(stderr, "OnNewParams: OP=3 Init BRAM Transport\n");
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
                if (verbose == 1) fprintf(stderr, "1BRAM T init:\n");
                rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT, SHR_DEC_DATA.Value (),  1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value ());
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
        }
        
        if ( OPERATION.Value () == 4 ){
                double reading_vector[READING_MAX_VALUES];
                OPERATION.Value () = 0;
                if (verbose > 0) fprintf(stderr, "OnNewParams: OP=4 Start BRAM Transport\n");
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
                if (verbose == 1) fprintf(stderr, "1BRAM T start:\n");
                rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START, SHR_DEC_DATA.Value (), 1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value ());
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
        }
        
        if ( OPERATION.Value () == 5 ){
                double reading_vector[READING_MAX_VALUES];
                OPERATION.Value () = 0;
                if (verbose > 0) fprintf(stderr, "OnNewParams: OP=5 Read BRAM\n");
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
                if (verbose == 1) fprintf(stderr, "1BRAM read:\n");
                rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_LOOP, SHR_DEC_DATA.Value (), 1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value ());
                read_bram (SIGNAL_SIZE_DEFAULT, TRANSPORT_DECIMATION.Value (),  TRANSPORT_MODE.Value (), GAIN1.Value (), GAIN2.Value ());
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
        }
        
        if (verbose > 3) fprintf(stderr, "OnNewParams: set_PAC_config\n");
        set_PAC_config();
        if (verbose > 3) fprintf(stderr, "OnNewParams done.\n");
}


void OnNewSignals(void){
        if (verbose > 3) fprintf(stderr, "OnNewSignals()\n");
	// do something
	CDataManager::GetInstance()->UpdateAllSignals();
        if (verbose > 3) fprintf(stderr, "OnNewSignals done.\n");
}


void PostUpdateSignals(void){
        if (verbose > 3) fprintf(stderr, "PostUpdateSignals()\n");
        if (verbose > 3) fprintf(stderr, "PostUpdateSignals done.\n");
}

// END.
