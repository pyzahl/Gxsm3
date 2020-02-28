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
// cp ~/SVN/RedPitaya/RedPACPLL4mdc-SPI/RedPACPLL4mdc-SPI.runs/impl_1/system_wrapper.bit fpga.bit
// scp -r pacpll root@rp-f05603.local:/opt/redpitaya/www/apps/
// make clean; make INSTALL_DIR=/opt/redpitaya

// CHROME BROWSER NOTES: USER SCHIT-F5 to force reload of all data, else caches are fooling....

/*
 * RedPitaya A9 JSON Interface PARAMETERS and SIGNALS
 * ------------------------------------------------------------
 */

#define ADC_DECIMATING     1
#define ADC_SAMPLING_RATE (125e6/ADC_DECIMATING)
//#define ADC_SAMPLING_RATE (125e6/2)

//Signal size
#define SIGNAL_SIZE_DEFAULT       1024
#define TUNE_SIGNAL_SIZE_DEFAULT  1024
#define PARAMETER_UPDATE_INTERVAL 200 // ms
#define SIGNAL_UPDATE_INTERVAL    200 // ms

#define SIGNAL_SIZE_GPIOX 16

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

CFloatSignal SIGNAL_FRQ("SIGNAL_FRQ", TUNE_SIGNAL_SIZE_DEFAULT, 0.0f);
std::vector<float> g_data_signal_frq(TUNE_SIGNAL_SIZE_DEFAULT);

CFloatSignal SIGNAL_TUNE_PHASE("SIGNAL_TUNE_PHASE", TUNE_SIGNAL_SIZE_DEFAULT, 0.0f);
std::vector<float> g_data_signal_ch1pa(TUNE_SIGNAL_SIZE_DEFAULT); // only used in tune mode

CFloatSignal SIGNAL_TUNE_AMPL("SIGNAL_TUNE_AMPL", TUNE_SIGNAL_SIZE_DEFAULT, 0.0f);
std::vector<float> g_data_signal_ch2aa(TUNE_SIGNAL_SIZE_DEFAULT); // only used in tune mode

CFloatSignal SIGNAL_TIME("SIGNAL_TIME", SIGNAL_SIZE_DEFAULT, 0.0f);
std::vector<float> g_data_signal_time(SIGNAL_SIZE_DEFAULT);

CIntSignal SIGNAL_GPIOX("SIGNAL_GPIOX",  SIGNAL_SIZE_GPIOX, 0);
std::vector<int> g_data_signal_gpiox(SIGNAL_SIZE_GPIOX);

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
CDoubleParameter FREQUENCY_CENTER("FREQUENCY_CENTER", CBaseParameter::RW, 32766.0, 0, 1, 25e6); // Hz -- used for BRam and AUX data to remove offset, and scale
CDoubleParameter AUX_SCALE("AUX_SCALE", CBaseParameter::RW, 1.0, 0, -1e6, 1e6); // 1
CDoubleParameter VOLUME_MANUAL("VOLUME_MANUAL", CBaseParameter::RW, 300.0, 0, 0.0, 1000.0); // mV
CDoubleParameter PAC_DCTAU("PAC_DCTAU", CBaseParameter::RW, 10.0, 0, -1.0, 1e6); // ms ,negative value disables DC LMS and used manual DC 
CDoubleParameter PACTAU("PACTAU", CBaseParameter::RW, 10.0, 0, 0.0, 60e6); // in periods now -- 350us good value @ 30kHz
CDoubleParameter PACATAU("PACATAU", CBaseParameter::RW, 1.5, 0, 0.0, 60e6); // in periods now -- 50us good value @ 30kHz

CDoubleParameter QC_GAIN("QC_GAIN", CBaseParameter::RW, 0, 0, -1024.0, 1024.0); // gain factor
CDoubleParameter QC_PHASE("QC_PHASE", CBaseParameter::RW, 0, 0, 0.0, 360.0); // deg

CDoubleParameter TUNE_SPAN("TUNE_SPAN", CBaseParameter::RW, 5.0, 0, 0.1, 1e6); // Hz
CDoubleParameter TUNE_DFREQ("TUNE_DFREQ", CBaseParameter::RW, 0.05, 0, 0.0001, 1000.); // Hz

// PLL CONFIGURATION
CBooleanParameter SET_SINGLESHOT_TRANSPORT_TRIGGER("SET_SINGLESHOT_TRANSPORT_TRIGGER", CBaseParameter::RW, false, 0);
CBooleanParameter AMPLITUDE_CONTROLLER("AMPLITUDE_CONTROLLER", CBaseParameter::RW, false, 0);
CBooleanParameter PHASE_CONTROLLER("PHASE_CONTROLLER", CBaseParameter::RW, false, 0);
CBooleanParameter PHASE_UNWRAPPING_ALWAYS("PHASE_UNWRAPPING_ALWAYS", CBaseParameter::RW, false, 0);
CBooleanParameter QCONTROL("QCONTROL", CBaseParameter::RW, false, 0);
CBooleanParameter LCK_AMPLITUDE("LCK_AMPLITUDE", CBaseParameter::RW, false, 0);
CBooleanParameter LCK_PHASE("LCK_PHASE", CBaseParameter::RW, false, 0);

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

// PHASE Valid for PAC time constant set to 15us:
// Cp = 20*log10 ( 1.6575e-5*Fc )
// Ci = 20*log10 ( 1.7357e-10*Fc^2 )
// Where Fc is the desired bandwidth of the controller in Hz (the suggested range is between 1.5 Hz to 4.5kHz).

// AMPL
// Cp = 20*log10 (0.08045*Q Fc / Gain_res F0 )
// Ci = 20*log10 (8.4243e-7*Q Fc^2 /Gain_res F0 )
// Where :
//Gain res is the gain of the resonator at the resonance
//Q is the Q factor of the resonator
//F0 is the frequency at the resonance in Hz
//Fc is the desired bandwidth of the controller in Hz (the suggested range is between 1.5 Hz to 10Hz).

CStringParameter pacpll_text("PAC_TEXT", CBaseParameter::RW, "N/A                                    ", 40);

CIntParameter parameter_updatePeriod("PARAMETER_PERIOD", CBaseParameter::RW, PARAMETER_UPDATE_INTERVAL, 0, 0, 50000);
CIntParameter signal_updatePeriod("SIGNAL_PERIOD", CBaseParameter::RW, SIGNAL_UPDATE_INTERVAL, 0, 0, 50000);
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
  
        if (verbose > 1) fprintf(stderr, "RP FPGA_PACPLL BRAM: mapped %08lx - %08lx.\n", (unsigned long)(0x40000000), (unsigned long)(0x40000000 + FPGA_PACPLL_BRAM_block_size));

        
        FPGA_PACPLL_cfg = mmap (NULL, FPGA_PACPLL_CFG_block_size,
                                PROT_READ|PROT_WRITE,  MAP_SHARED, fd, 0x42000000);

        if (FPGA_PACPLL_cfg == MAP_FAILED)
                return RP_EOOR;

        if (verbose > 1) fprintf(stderr, "RP FPGA_PACPLL CFG: mapped %08lx - %08lx.\n", (unsigned long)(0x42000000), (unsigned long)(0x42000000 + FPGA_PACPLL_CFG_block_size));

        srand(time(NULL));   // init random

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
#define Q16 QN(16)
#define Q15 QN(15)
#define Q13 QN(13)
#define Q12 QN(12)
#define Q10 QN(10)
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

        double fclk = ADC_SAMPLING_RATE / 32;
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
        double fclk = ADC_SAMPLING_RATE;
        return Q44*freq/fclk;
}

double dds_phaseinc_to_freq (unsigned long long ddsphaseincQ44){
        double fclk = ADC_SAMPLING_RATE;
        return fclk*(double)ddsphaseincQ44/(double)(Q44);
}

double dds_phaseinc_rel_to_freq (long long ddsphaseincQ44){
        double fclk = ADC_SAMPLING_RATE;
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
// Configure Control Switched: Loops Ampl and Phase On/Off, Unwrapping, QControl
void rp_PAC_configure_switches (int phase_ctrl, int am_ctrl, int phase_unwrap_always, int qcontrol, int lck_amp, int lck_phase){
        if (verbose > 2) fprintf(stderr, "##Configure loop controls: %x",  phase_ctrl ? 1:0 | am_ctrl ? 2:0); 
        set_gpio_cfgreg_int32 (PACPLL_CFG_CONTROL_LOOPS,
                               (phase_ctrl ? 1:0) | (am_ctrl ? 2:0) | (phase_unwrap_always ? 4:0) |
                               (qcontrol ? 8:0) |
                               (lck_amp ? 0x10:0)  | (lck_phase ? 0x20:0)
                               );
}


#define QCONTROL_CFG_GAIN_DELAY 29
// Configure Q-Control Logic build into Volume Adjuster
void rp_PAC_set_qcontrol (double gain, double phase){
        double samples_per_period = ADC_SAMPLING_RATE / FREQUENCY_MANUAL.Value ();
        int ndelay = int (samples_per_period * phase/360. + 0.5);

        if (ndelay < 0 || ndelay > 8192 || phase < 0.)
                ndelay = 0; // Q-Control disabled when delay == 0

        if (verbose > 2) fprintf(stderr, "##Configure: qcontrol= %d, %d\n", (int)(Q15*gain), ndelay); 

        ndelay = 8192-ndelay; // pre compute offset in ring buffer
        set_gpio_cfgreg_int32 (QCONTROL_CFG_GAIN_DELAY, ((int)(Q10 * gain)<<16) | ndelay );
}

#if 0
// tau from mu
double time_const(double fref, double mu){
        return -(1.0/fref) / log(1.0-mu);
}
// mu from tau
double mu_const(double fref, double tau){
        return 1.0-exp(-1.0/(fref*tau));
}
// -3dB cut off freq
double cut_off_freq_3db(double fref, double mu){
        return -(1.0/(fref*2.*M_PI)) * log(1.0-mu);
}

double mu_opt (double periods){
        double mu = 11.9464 / (6.46178 + periods);
        return mu > 1.0 ? 1.0 : mu;
}
#endif

#define PACPLL_CFG_PACTAU     4 // (actual Q22 mu)
#define PACPLL_CFG_PACATAU   27
#define PACPLL_CFG_PAC_DCTAU 28
// tau in s for dual PAC and auto DC offset
void rp_PAC_set_pactau (double tau, double atau, double dc_tau){
        if (verbose > 2) fprintf(stderr, "##Configure: tau= %g  Q22: %d\n", tau, (int)(Q22 * tau)); 

#if 1
        // in tau s (us) -> mu
        set_gpio_cfgreg_int32 (PACPLL_CFG_PACTAU, (int)(Q22/ADC_SAMPLING_RATE/(1e-6*tau))); // Q22 significant from top - tau for phase
        set_gpio_cfgreg_int32 (PACPLL_CFG_PACATAU, (int)(Q22/ADC_SAMPLING_RATE/(1e-6*atau))); // Q22 significant from top -- atau is tau for amplitude
#else
        // in peridos relative to reference base frequency. Silently limit mu to opt mu.
        // mu_opt = 11.9464 / (6.46178 + ADC_SAMPLE_FRQ/F_REF)
        double spp = ADC_SAMPLING_RATE / FREQUENCY_MANUAL.Value (); // samples per period
        double mu_fastest = mu_opt (spp);
        double mu = mu_const (ADC_SAMPLING_RATE, tau/FREQUENCY_MANUAL.Value ()); // mu from user tau measured in periods of f-reference
        if (verbose > 2) fprintf(stderr, "##Configure: pac PHtau   mu= %g, fastest=%g\n", mu, mu_fstest);
        if (mu > mu_fastest) mu=mu_fastest;
        set_gpio_cfgreg_int32 (PACPLL_CFG_PACTAU, (int)(Q22*mu)); // Q22 significant from top - tau for phase

        double mu = mu_const (ADC_SAMPLING_RATE, atau/FREQUENCY_MANUAL.Value ()); // mu from user tau measured in periods of f-reference
        if (verbose > 2) fprintf(stderr, "##Configure: pac AMPtau   mu= %g, fastest=%g\n", mu, mu_fstest);
        if (mu > mu_fastest) mu=mu_fastest;
        set_gpio_cfgreg_int32 (PACPLL_CFG_PACATAU, (int)(Q22*mu)); // Q22 significant from top -- atau is tau for amplitude
#endif
        // Q22 significant from top -- dc_tau is tau for DC FIR-IIR Filter on phase aligned decimated data:
        // at 4x Freq Ref sampling rate. Moving averaging FIR sampling at past 4 zero crossing of Sin Cos ref passed on to IIR with tau
        if (dc_tau > 0.)
                set_gpio_cfgreg_int32 (PACPLL_CFG_PAC_DCTAU, (int)(Q31/(4.*FREQUENCY_MANUAL.Value ())/dc_tau));
        else if (dc_tau < 0.)
                set_gpio_cfgreg_uint32 (PACPLL_CFG_PAC_DCTAU, 0xffffffff); // disable -- use manaul DC, set below
        else
                set_gpio_cfgreg_uint32 (PACPLL_CFG_PAC_DCTAU, 0); // freeze
}

#define PACPLL_CFG_DC_OFFSET 5
// Set "manual" DC offset used if dc_tau (see above) signum bit is set (neg).
void rp_PAC_set_dcoff (double dc){
        if (verbose > 2) fprintf(stderr, "##Configure: dc= %g  Q22: %d\n", dc, (int)(Q22 * dc)); 
        set_gpio_cfgreg_int32 (PACPLL_CFG_DC_OFFSET, (int)(Q22 * dc));
}

// measure DC, update manual offset
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

// update/follow (slow IIR) DC offset
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

// Controller Topology:
/*
  // IP Configuration
  //                                  DEFAULTS   PHASE   AMPL         
    parameter AXIS_TDATA_WIDTH =            32,  24      24    // INPUT AXIS DATA WIDTH
    parameter M_AXIS_CONTROL_TDATA_WIDTH =  32,  48      16    // SERVO CONTROL DATA WIDTH OF AXIS
    parameter CONTROL_WIDTH =               32,  44      16    // SERVO CONTROL DATA WIDTH
    parameter M_AXIS_CONTROL2_TDATA_WIDTH = 32,  48      32    // INTERNAL CONTROl DATA WIDTH MAPPED TO AXIS FOR READOUT not including extend
    parameter CONTROL2_WIDTH =              50,  75      56    // INTERNAL CONTROl DATA WIDTH not including extend **** COEFQ+AXIS_TDATA_WIDTH == CONTROL2_WIDTH
    parameter CONTROL2_OUT_WIDTH =          32,  44      32    // max passed outside control width, must be <= CONTROL2_WIDTH
    parameter COEF_WIDTH =                  32,  32      32    // CP, CI WIDTH
    parameter QIN =                         22,  22      22    // Q In Signal
    parameter QCOEF =                       31,  31      31    // Q CP, CI's
    parameter QCONTROL =                    31,  31      31    // Q Controlvalue
    parameter CEXTEND =                      4,   1       4    // room for saturation check
    parameter DEXTEND =                      1,   1       1    // data, erorr extend
    parameter AMCONTROL_ALLOW_NEG_SPECIAL =  1    0       1    // Special Ampl. Control Mode
  // ********************

  if (AMCONTROL_ALLOW_NEG_SPECIAL)
     if (error_next > $signed(0) && control_next < $signed(0)) // auto reset condition for amplitude control to preven negative phase, but allow active "damping"
          control      <= 0;
          controlint   <= 0;

  ... check limits and limit to upper and lower, set limiter status indicators;

  m = AXI-axi_input;   // [AXIS_TDATA_WIDTH-1:0]
  error = setpoint - m;

  if (enable)
    controlint_next <= controlint + ci*error; // saturation via extended range and limiter // Q64.. += Q31 x Q22 ==== AXIS_TDATA_WIDTH + COEF_WIDTH
    control_next    <= controlint + cp*error; // 
  else
    controlint_next <= reset;
    control_next    <= reset;

  *************************
  assign M_AXIS_CONTROL_tdata   = {control[CONTROL2_WIDTH+CEXTEND-1], control[CONTROL2_WIDTH-2:CONTROL2_WIDTH-CONTROL_WIDTH]}; // strip extension
  assign M_AXIS_CONTROL2_tdata  = {control[CONTROL2_WIDTH+CEXTEND-1], control[CONTROL2_WIDTH-2:CONTROL2_WIDTH-CONTROL2_OUT_WIDTH]};

  assign mon_signal  = {m[AXIS_TDATA_WIDTH+DEXTEND-1], m[AXIS_TDATA_WIDTH-2:0]};
  assign mon_error   = {error[AXIS_TDATA_WIDTH+DEXTEND-1], error[AXIS_TDATA_WIDTH-2:0]};
  assign mon_control = {control[CONTROL2_WIDTH+CEXTEND-1], control[CONTROL2_WIDTH-2:CONTROL2_WIDTH-32]};
  assign mon_control_lower32 = {{control[CONTROL2_WIDTH-32-1 : (CONTROL2_WIDTH>=64? CONTROL2_WIDTH-32-1-31:0)]}, {(CONTROL2_WIDTH>=64?0:(64-CONTROL2_WIDTH)){1'b0}}}; // signed, lower 31
  *************************
 */


// Configure Controllers

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

// Amplitude Controller
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

// Phase Controller
// CONTROL[75] OUT[44] : [75-1-1:75-44]=43+1   m[24]  x  c[32]  = 56 M: 24{Q32},  P: 44{Q14}
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

        set_gpio_cfgreg_int32 (PACPLL_CFG_PHASE_CONTROLLER + PACPLL_CFG_SET,   ((int)(QCORDICATAN * setpoint))); // <<(32-BITS_CORDICATAN));
        set_gpio_cfgreg_int32 (PACPLL_CFG_PHASE_CONTROLLER + PACPLL_CFG_CP,    (long long)(QPHCOEF * cp)); // {32}   22+1 bit error, 32bit CP,CI << 31 --  m[24]  x  cp|ci[32]  = 56 M: 24{Q32},  P: 44{Q14}, top S43
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
#define PACPLL_CFG_TRANSPORT_AUX_CENTER      18  // 18,19
#define PACPLL_CFG_TRANSPORT_INIT            16  // Bit 4=1
#define PACPLL_CFG_TRANSPORT_SINGLE          2   // Bit 1=1
#define PACPLL_CFG_TRANSPORT_LOOP            0   // Bit 1=0
#define PACPLL_CFG_TRANSPORT_START           1   // Bit 0=1

// Configure BRAM Data Transport Mode
void rp_PAC_configure_transport (int control, int shr_dec_data, int nsamples, int decimation, int channel_select, double scale, double center){
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
        // AUX scale, center
        set_gpio_cfgreg_int32 (PACPLL_CFG_TRANSPORT_AUX_SCALE, (int)round(Q15*scale));
        set_gpio_cfgreg_int48 (PACPLL_CFG_TRANSPORT_AUX_CENTER, (unsigned long long)round (dds_phaseinc (center))); // => 44bit phase
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
 * reading_vector[6] := x5 M-DC_LMS
 * reading_vector[7] := x6
 * reading_vector[8] := x7 Exec Amp Mon
 * reading_vector[9] := DDS Freq
 * reading_vector[10]:= x3 Monitor (In0 Signal, LMS inpu)
 * reading_vector[11]:= x3
 * reading_vector[12]:= x11
 * reading_vector[13]:= x12
 */

#define READING_MAX_VALUES 14

// Get all GPIO mapped data / system state snapshot
void rp_PAC_get_single_reading (double reading_vector[READING_MAX_VALUES]){
        int x,y,xx7,xx8,xx9,uix;
        double a,b,v,p,x3,x4,x5,x6,x7,x8,x9,x10,qca,pfpga,x11,x12;
        
        SIGNAL_GPIOX[0] = x = read_gpio_reg_int32 (1,0); // GPIO X1 : LMS A (cfg + 0x1000)
        a=(double)x / QLMS;
        SIGNAL_GPIOX[1] = x = read_gpio_reg_int32 (1,1); // GPIO X2 : LMS B (cfg + 0x1008)
        b=(double)x / QLMS;
        v=sqrt (a*a+b*b);
        p=atan2 ((a-b),(a+b));
        
        SIGNAL_GPIOX[2] = x = read_gpio_reg_int32 (2,0); // GPIO X3 : LMS DBG1 :: M (LMS input Signal) (cfg + 0x2000) ===> used for DC OFFSET calculation
        x3=(double)x / QLMS;
        SIGNAL_GPIOX[3] = x = read_gpio_reg_int32 (2,1); // GPIO X4 : CORDIC SQRT (AM2=A^2+B^2) = Amplitude Monitor
        x4=(double)x / QCORDICSQRT;
        //x4=(double)x / QEXEC;
        SIGNAL_GPIOX[4] = x = read_gpio_reg_int32 (3,0); // GPIO X5 : MDC
        x5=(double)x / QLMS; // MDC    /QCORDICSQRTFIR;
        SIGNAL_GPIOX[5] = x = read_gpio_reg_int32 (3,1); // GPIO X6 : XXX
        x6=(double)x / QCORDICATANFIR;
        SIGNAL_GPIOX[6] = xx7 = x = read_gpio_reg_int32 (4,0); // GPIO X7 : Exec Ampl Control Signal (signed)
        x7=(double)x / QEXEC;
        SIGNAL_GPIOX[7] = xx8 = x = read_gpio_reg_int32 (4,1); // GPIO X8 : DDS Phase Inc (Freq.) upper 32 bits of 44 (signed)
        SIGNAL_GPIOX[8] = xx9 = x = read_gpio_reg_int32 (5,0); // GPIO X9 : DDS Phase Inc (Freq.) lower 32 bits of 44 (signed)
        x9=(double)x / QLMS;
        SIGNAL_GPIOX[9] = uix = read_gpio_reg_uint32 (5,1); // GPIO X10: CORDIC ATAN(X/Y) = Phase Monitor
        x10 = (double)uix / QCORDICATAN; // ATAN 24bit 3Q21 
        //x10=(qca=(double)x / QCORDICATAN/M_PI*180.) - 90. - (x8<0? 230:0.); // ???? WHY NOT 180 ????
        //x10 /= 180.; // for testing 100 ==== 180
                  
        //pfpga=atan2 (x8,x7)/M_PI*180. - 90.;
        //pfpga /= 180.;
        SIGNAL_GPIOX[10] = y=x = read_gpio_reg_int32 (6,0); // GPIO X11 : Transport WPos
        x11=(double)(x&0xffff);
        SIGNAL_GPIOX[11] = x = read_gpio_reg_int32 (6,1); // GPIO X12 : Transport Dbg
        x12=(double)((x>>16)&0xffff);

        SIGNAL_GPIOX[12] = x = read_gpio_reg_int32 (7,0); // GPIO X13 : Transport Dbg
        SIGNAL_GPIOX[13] = x = read_gpio_reg_int32 (7,1); // GPIO X14 : Transport Dbg
        SIGNAL_GPIOX[14] = x = read_gpio_reg_int32 (8,0); // GPIO X15 : Transport Dbg
        SIGNAL_GPIOX[15] = x = read_gpio_reg_int32 (8,1); // GPIO X16 : Transport Dbg

        
        // LMS Detector Readings and double precision conversions
        reading_vector[0] = v*1000.; // LMS Amplitude (Volume) in mVolts (from A,B)
        reading_vector[1] = p/M_PI*180.; // LMS Phase (from A,B)
        reading_vector[2] = a; // LMS A (Real)
        reading_vector[3] = b; // LMS B (Imag)

        // FPGA CORDIC based convertions
        reading_vector[4] = x4*1000.;  // FPGA CORDIC (SQRT) Amplitude Monitor in mVolt
        reading_vector[5] = x10/M_PI*180.; // FPGA CORDIC (ATAN) Phase Monitor

        reading_vector[6] = x5*1000.;  // X5
        reading_vector[7] = x6;  // X4

        reading_vector[8] = x7*1000.0;  // Exec Ampl Control Signal (signed) in mV
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
        double reading_vector[READING_MAX_VALUES];
        int status[3];

        if (SET_SINGLESHOT_TRANSPORT_TRIGGER.Value ()){

                //while (!bram_status(status)); // block

                // trigger single shot
                rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,
                                            SHR_DEC_DATA.Value (),  1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
                if (verbose > 0) fprintf(stderr, "OnNewParams: OP=4 Single Shot BRAM Transport -- trigger before controller adjust\n");
                rp_PAC_get_single_reading (reading_vector);
                if (verbose == 1) fprintf(stderr, "1BRAM T start:\n");
                rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START|PACPLL_CFG_TRANSPORT_SINGLE,
                                            SHR_DEC_DATA.Value (), 1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
        }

        if (OPERATION.Value() < 6)
                rp_PAC_adjust_dds (FREQUENCY_MANUAL.Value());
        rp_PAC_set_volume (VOLUME_MANUAL.Value() / 1000.); // mV -> V
        rp_PAC_set_pactau (PACTAU.Value(), PACATAU.Value(), PAC_DCTAU.Value() * 1e-3); // periods, periods, ms -> s

        rp_PAC_set_qcontrol (QC_GAIN.Value (), QC_PHASE.Value ());

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
        
        rp_PAC_configure_switches (PHASE_CONTROLLER.Value ()?1:0, AMPLITUDE_CONTROLLER.Value ()?1:0, PHASE_UNWRAPPING_ALWAYS.Value ()?1:0, QCONTROL.Value ()?1:0, LCK_AMPLITUDE.Value ()?1:0, LCK_PHASE.Value ()?1:0);
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
        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,
                                    SHR_DEC_DATA.Value (), 1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START|PACPLL_CFG_TRANSPORT_LOOP,
                                    SHR_DEC_DATA.Value (), 1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
        
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



void read_bram (int n, int dec, int t_mode, double gain1, double gain2){
        int k;
        size_t i = 12;
        size_t N = 8*n;
        
        switch (t_mode){
        case 0: // AXIS1xy :  IN1, IN2
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // IN1 (14)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // IN2 (14)
                        SIGNAL_CH1[k] = (float)ix32*gain1/Q13*1000.;
                        SIGNAL_CH2[k] = (float)iy32*gain2/Q13*1000.;
                }
                break;
        case 1: // AXIS45 :  CORDIC SQRT, ATAN
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Phase (24)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Ampl (24)
                        SIGNAL_CH1[k] = (float)ix32/QCORDICATAN*180./M_PI;
                        SIGNAL_CH2[k] = (float)iy32/QCORDICSQRT*1000.;
                }
                break;
        case 2:
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // IN1 (14)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Ampl (24)
                        SIGNAL_CH1[k] = (float)ix32*gain1/Q13*1000.;
                        SIGNAL_CH2[k] = (float)iy32/QCORDICSQRT*1000.;
                }
                break;
        case 3:
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // IN1 (14)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Phase (24)
                        SIGNAL_CH1[k]  = (float)ix32*gain1/Q13*1000.;
                        SIGNAL_CH2[k]  = (float)iy32/QCORDICATAN*180./M_PI;
                }
                break;
        case 4:
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Ampl Exec (32)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Freq (48)-Lower(48)
                        SIGNAL_CH1[k]  = (float)ix32 / QEXEC * 1000.;
                        SIGNAL_CH2[k]  = (float)dds_phaseinc_rel_to_freq ((double)iy32); // correct to 44
                }
                break;
        case 5:
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Ampl (24)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Ampl Exec (32)
                        SIGNAL_CH1[k]  = (float)ix32/QCORDICSQRT*1000.;
                        SIGNAL_CH2[k]  = (float)iy32 / QEXEC * 1000.;
                }
                break;
        case 6:
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Phase (24)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Freq (48)-Lower(48)
                        SIGNAL_CH1[k]  = (float)ix32/QCORDICATAN*180./M_PI;
                        SIGNAL_CH2[k]  = (float)dds_phaseinc_rel_to_freq ((double)iy32); // correct to 44
                }
                break;
        case 7:
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // M-DC_iir (32)
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Ampl (24)
                        SIGNAL_CH1[k] = (float)ix32/QLMS*1000.; // mV
                        SIGNAL_CH2[k] = (float)iy32/QCORDICSQRT*1000.; // mV
                }
                break;
        default :
                for (k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                        int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // direct data
                        int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // direct data
                        SIGNAL_CH1[k]  = (float)ix32;
                        SIGNAL_CH2[k]  = (float)iy32;
                }
                break;
        }
}

int initiate_bram_write_measurements(){
        int ret;
        int status[3];
        double reading_vector[READING_MAX_VALUES];
        int decs[] = { 16, 14, 14, 16 };

        rp_PAC_get_single_reading (reading_vector);
        rp_PAC_get_single_reading (reading_vector);
        ret =  bram_status(status);
        if (ret){
                if (verbose == 1) fprintf(stderr, "BRAM T init:\n");
                rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,
                                            decs[OPERATION.Value ()-6],  1024, 1<<decs[OPERATION.Value ()-6], 1, AUX_SCALE.Value (), FREQUENCY_CENTER.Value()); // Phase, Ampl
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
                if (verbose == 1) fprintf(stderr, "BRAM T start:\n");
                rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START|PACPLL_CFG_TRANSPORT_SINGLE,
                                            decs[OPERATION.Value ()-6],  1024, 1<<decs[OPERATION.Value ()-6], 1, AUX_SCALE.Value (), FREQUENCY_CENTER.Value()); // Phase, Ampl
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
        }
        BRAM_WRITE_POS.Value () = status[0];
        BRAM_DEC_COUNT.Value () = status[1];
        return ret;
}

void read_phase_ampl_buffer_avg (double &ampl, double &phase, int initiate){
        ampl=0.;
        phase=0.;
        size_t i = 12;
        size_t N = 8*SIGNAL_SIZE_DEFAULT;

        if (initiate){
                int timeout = 100; while ( !initiate_bram_write_measurements () && --timeout>0) usleep (1000);
        }
        
        // wait for buffer full
        int status[3];
        int max_try=100; while ( !bram_status(status) && --max_try>0) usleep (1000);

        for (int k=0; i<N && k < SIGNAL_SIZE_DEFAULT; ++k){
                int32_t ix32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Phase (24)
                int32_t iy32 = *((int32_t *)((uint8_t*)FPGA_PACPLL_bram+i)); i+=4; // Ampl (24)
                phase += (SIGNAL_CH1[k] = (double)ix32/QCORDICATAN/M_PI*180.); // PLL Phase deg
                ampl  += (SIGNAL_CH2[k] = (double)iy32/QCORDICSQRT*1000.); // // Resonator Amplitude Signal Monitor in mV
        }
        phase /= SIGNAL_SIZE_DEFAULT;
        ampl  /= SIGNAL_SIZE_DEFAULT;
}

void run_tune_op (int clear_tune_data, double epsilon){
        double reading_vector[READING_MAX_VALUES];
        int status[3];
        int ch;
        static int dir=1;
        static double f=0.;
        static double tune_amp_max=0.;
        static double tune_phase=0.;
        static double tune_fcenter=0.;
        double ampl, phase;
        static int    i_prev = TUNE_SIGNAL_SIZE_DEFAULT/2;
        static double f_prev = 0.;
        
        static int reps[] = { 1, 5, 25, 6 };
        int waitus[] = { 100000, 80000, 40000, 40000 };
        static int i0 = TUNE_SIGNAL_SIZE_DEFAULT/2;
        double df = TUNE_SPAN.Value ()/(TUNE_SIGNAL_SIZE_DEFAULT-1);
        double ampl_prev=0.;
        double phase_prev=0.;

        // zero buffers, reset state
        if (clear_tune_data){
                for (int i = 0; i < TUNE_SIGNAL_SIZE_DEFAULT; i++){
                        SIGNAL_TUNE_PHASE[i] = 0.;
                        SIGNAL_TUNE_AMPL[i]  = 0.;
                        SIGNAL_FRQ[i] = -TUNE_SPAN.Value ()/2 + df*i;
                           
                        g_data_signal_ch1pa.erase (g_data_signal_ch1pa.begin());
                        g_data_signal_ch1pa.push_back (0.0);
                
                        g_data_signal_ch2aa.erase (g_data_signal_ch2aa.begin());
                        g_data_signal_ch2aa.push_back (0.0);
                
                        g_data_signal_frq.erase (g_data_signal_frq.begin());
                        g_data_signal_frq.push_back (0.0);
                }
                f = 0.; // f = -TUNE_SPAN.Value ()/4.;
                dir = 1;
                i0 = TUNE_SIGNAL_SIZE_DEFAULT/2;
                i_prev = TUNE_SIGNAL_SIZE_DEFAULT/2;
                f_prev = 0.;

                rp_PAC_adjust_dds (FREQUENCY_MANUAL.Value() + f);
                FREQUENCY_TUNE.Value() = f;
                usleep (25000); // min recover time
                read_phase_ampl_buffer_avg (ampl, phase, 1);
                ampl_prev  = ampl;
                phase_prev = phase;
                
                CENTER_FREQUENCY.Value () = tune_fcenter;
                CENTER_PHASE.Value () = tune_phase;
                CENTER_AMPLITUDE.Value () = tune_amp_max;
                tune_amp_max=0.;
        }

        double s = i0 > TUNE_SIGNAL_SIZE_DEFAULT/2 ? 1:-1.;
        double x = (double)(i0-TUNE_SIGNAL_SIZE_DEFAULT/2); x *= x; x /= TUNE_SIGNAL_SIZE_DEFAULT/2; x *= s;
        int k = 0;
        for (int ti = 0; ti < reps[OPERATION.Value ()-6]; ++ti){
                read_phase_ampl_buffer_avg (ampl, phase, clear_tune_data);

                // wait for stable reading
                int max_try = 30;
                while (fabs(ampl-ampl_prev) > epsilon*ampl && --max_try>0){
                        read_phase_ampl_buffer_avg (ampl, phase, 1);
                        ampl_prev  = ampl;
                        phase_prev = phase;
                }
                ampl_prev  = ampl;
                phase_prev = phase;

                
                if (OPERATION.Value () == 9){
                        // TUNE mode: RS (Random Search)
                        // if stable or last rep, continue to near by point
                        k = (int)(128.*(double)rand () / RAND_MAX) - 64;
                        int i = TUNE_SIGNAL_SIZE_DEFAULT/2 + (int)x + k;
                        if (i < 0) i=0;
                        if (i >= TUNE_SIGNAL_SIZE_DEFAULT) i=TUNE_SIGNAL_SIZE_DEFAULT-1;

                        f = -TUNE_SPAN.Value ()/2 + df*i;

                        if (SIGNAL_TUNE_PHASE[i_prev] != 0.0){
                                SIGNAL_TUNE_PHASE[i_prev] += phase;
                                SIGNAL_TUNE_PHASE[i_prev] *= 0.5;
                        } else
                                SIGNAL_TUNE_PHASE[i_prev] = phase;
                        if (SIGNAL_TUNE_AMPL[i_prev]  != 0.0){
                                SIGNAL_TUNE_AMPL[i_prev] += ampl;
                                SIGNAL_TUNE_AMPL[i_prev] *= 0.5;
                        } else
                                SIGNAL_TUNE_AMPL[i_prev] = ampl;
                        SIGNAL_FRQ[i_prev] = f_prev;
                        i_prev = i;
                        f_prev = f;
                } else {
                        // TUNE mode linear rocking sweep loop

                        g_data_signal_ch1pa.erase (g_data_signal_ch1pa.begin());
                        g_data_signal_ch1pa.push_back (phase);
                
                        g_data_signal_ch2aa.erase (g_data_signal_ch2aa.begin());
                        g_data_signal_ch2aa.push_back (ampl);
                
                        g_data_signal_frq.erase (g_data_signal_frq.begin());
                        g_data_signal_frq.push_back (f);

                        if (f < TUNE_SPAN.Value ()/2 && dir == 1)
                                f += TUNE_DFREQ.Value ();
                        else if (dir == 1){
                                dir = -1;
                                CENTER_FREQUENCY.Value () = tune_fcenter;
                                CENTER_PHASE.Value () = tune_phase;
                                CENTER_AMPLITUDE.Value () = tune_amp_max;
                                tune_amp_max=0.;
                        }
                        if (f > -TUNE_SPAN.Value ()/2 && dir == -1)
                                f -= TUNE_DFREQ.Value ();
                        else if (dir == -1){
                                dir = 1;
                                CENTER_FREQUENCY.Value () = tune_fcenter;
                                CENTER_PHASE.Value () = tune_phase;
                                CENTER_AMPLITUDE.Value () = tune_amp_max;
                                tune_amp_max=0.;
                        }
                }
                // next
                rp_PAC_adjust_dds (FREQUENCY_MANUAL.Value() + f);
                FREQUENCY_TUNE.Value() = f;
                usleep (25000); // min recover time

                int timeout = 100; while ( !initiate_bram_write_measurements () && --timeout>0) usleep (1000);

                if (reps[OPERATION.Value ()-6] > 1)
                        usleep (waitus[OPERATION.Value ()-6]);
                
                if (ampl > tune_amp_max){
                        tune_amp_max = ampl;
                        tune_phase = phase;
                        tune_fcenter = FREQUENCY_MANUAL.Value() + f;
                }

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

                for (int i = 0; i < SIGNAL_SIZE_DEFAULT; i++){
                        SIGNAL_CH3[i] = g_data_signal_ch3[i];
                        SIGNAL_CH4[i] = g_data_signal_ch4[i];
                        SIGNAL_CH5[i] = g_data_signal_ch5[i];
                }

                if (OPERATION.Value () == 9){
                        ; // set directly at index above in RS mode
                } else {
                        for (int i = 0; i < TUNE_SIGNAL_SIZE_DEFAULT; i++){
                                SIGNAL_TUNE_PHASE[i] = g_data_signal_ch1pa[i];
                                SIGNAL_TUNE_AMPL[i]  = g_data_signal_ch2aa[i];
                                SIGNAL_FRQ[i] = g_data_signal_frq[i];
                        }
                }
        }
        i0 = (TUNE_SIGNAL_SIZE_DEFAULT-1)*(double)rand () / RAND_MAX;
}

void UpdateSignals(void)
{
        static int clear_tune_data=1;
        double reading_vector[READING_MAX_VALUES];
        int ch;
        int status[3];
        int n=1024;
        
        if (verbose > 3) fprintf(stderr, "UpdateSignals()\n");

        // Scope, Tune in single shot mode
        if ( OPERATION.Value () == 2){
                if (verbose > 3) fprintf(stderr, "re-arm read BRAM writer\n");
                if (verbose == 1) fprintf(stderr, "BRAM read:\n");
                read_bram (SIGNAL_SIZE_DEFAULT, TRANSPORT_DECIMATION.Value (),  TRANSPORT_MODE.Value (), GAIN1.Value (), GAIN2.Value ());
                rp_PAC_get_single_reading (reading_vector);
                rp_PAC_get_single_reading (reading_vector);
                if (bram_status(status) && !SET_SINGLESHOT_TRANSPORT_TRIGGER.Value ()){
                        if (verbose == 1) fprintf(stderr, "BRAM T init:\n");
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,
                                                    SHR_DEC_DATA.Value (), n, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                        if (verbose == 1) fprintf(stderr, "BRAM T start:\n");
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START|PACPLL_CFG_TRANSPORT_SINGLE,
                                                    SHR_DEC_DATA.Value (), n, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                }
                BRAM_WRITE_POS.Value () = status[0];
                BRAM_DEC_COUNT.Value () = status[1];
        }

        // LOOP READ (FIFO MODE)
        if ( OPERATION.Value () == 5 ){
                int n=1024;
                // udpate
                rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START|PACPLL_CFG_TRANSPORT_LOOP,
                                            SHR_DEC_DATA.Value (), n, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
                bram_status(status);
                read_bram (SIGNAL_SIZE_DEFAULT, TRANSPORT_DECIMATION.Value (),  TRANSPORT_MODE.Value (), GAIN1.Value (), GAIN2.Value ());
                rp_PAC_get_single_reading (reading_vector);
                BRAM_WRITE_POS.Value () = status[0];
                BRAM_DEC_COUNT.Value () = status[1];
        }
        
        // TUNE MODE
        if ( OPERATION.Value () >= 6 && OPERATION.Value () <= 9){
                run_tune_op (clear_tune_data, 0.02); // 2% error
                rp_PAC_get_single_reading (reading_vector);
                clear_tune_data = 0; // after completed / mode switch zero tune data buffers
        } else {
                clear_tune_data = 1;
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
                }
        }
        
        if (verbose > 3) fprintf(stderr, "UpdateSignal complete.\n");

        VOLUME_MONITOR.Value ()   = reading_vector[4]; // Resonator Amplitude Signal Monitor in mV
        PHASE_MONITOR.Value ()    = reading_vector[5]; // PLL Phase in deg
        EXEC_MONITOR.Value ()     = reading_vector[8]; // Exec Amplitude Monitor in mV
        DDS_FREQ_MONITOR.Value () = reading_vector[9]; // DDS Freq Monitor in Hz
        
        rp_PAC_auto_dc_offset_correct ();
}

void UpdateParams(void){
        if (verbose > 3) fprintf(stderr, "UpdateParams()\n");
	CDataManager::GetInstance()->SetParamInterval (parameter_updatePeriod.Value());
	CDataManager::GetInstance()->SetSignalInterval (signal_updatePeriod.Value());

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

        counter.Value()=counter.Value()+(double)parameter_updatePeriod.Value()/1000.0;

        if (verbose > 3) fprintf(stderr, "UpdateParams: text update\n");
        pacpll_text.Value() = "Hello this is the RP PACPLL Server.    ";

        if (counter.Value()>30000) {
                counter.Value()=0;
        }
        if (verbose > 3) fprintf(stderr, "UpdateParams complete.\n");
}


void OnNewParams(void)
{
        static int ppv=0;
        static int spv=0;
        static int operation=0;
        double reading_vector[READING_MAX_VALUES];

#if 0
        
        if (ppv == 0) { ppv=parameter_updatePeriod.Value(); parameter_updatePeriod.Update (); }
        if (spv == 0) { spv=signal_updatePeriod.Value(); signal_updatePeriod.Update (); }
        
        if (verbose > 3) fprintf(stderr, "OnNewParams()\n");

        if (ppv != parameter_updatePeriod.Value ()){
                CDataManager::GetInstance()->SetParamInterval (parameter_updatePeriod.Value());
                ppv = parameter_updatePeriod.Value ();
        }
        if (spv != signal_updatePeriod.Value ()){
                CDataManager::GetInstance()->SetSignalInterval (signal_updatePeriod.Value());
                spv = signal_updatePeriod.Value ();
        }
#endif
        
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
        FREQUENCY_CENTER.Update ();
        AUX_SCALE.Update ();
        VOLUME_MANUAL.Update ();
        PACTAU.Update ();
        PACATAU.Update ();
        PAC_DCTAU.Update ();

        QC_GAIN.Update ();
        QC_PHASE.Update ();
        QCONTROL.Update ();
        
        PHASE_CONTROLLER.Update ();
        AMPLITUDE_CONTROLLER.Update ();
        SET_SINGLESHOT_TRANSPORT_TRIGGER.Update ();
        PHASE_UNWRAPPING_ALWAYS.Update ();
        
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

        if ( OPERATION.Value () > 0 && OPERATION.Value () != operation ){
                operation = OPERATION.Value ();
                switch (OPERATION.Value ()){
                case 1:
                        rp_PAC_auto_dc_offset_adjust ();
                        if (verbose > 0) fprintf(stderr, "OnNewParams: OP=1 Auto Offset Run, DC=%f mV\n", 1000.*signal_dc_measured);
                        pacpll_text.Value() = "Auto Offset completed.                 ";
                        break;
                case 2: // Scope
                        if (verbose > 0) fprintf(stderr, "OnNewParams: OP=5 Start-Finish, Repeat Hilevel BRAM Transport (Scope/Tune)\n");
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,
                                                    SHR_DEC_DATA.Value (),  1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                        if (verbose == 1) fprintf(stderr, "1BRAM read:\n");
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START|PACPLL_CFG_TRANSPORT_SINGLE,
                                                    SHR_DEC_DATA.Value (), 1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
                        break;
                case 6: // Tune
                        if (verbose > 0) fprintf(stderr, "OnNewParams: OP=5 Start-Finish, Repeat Hilevel BRAM Transport (Scope/Tune)\n");
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,
                                                    16,  1024, 65536, 1, AUX_SCALE.Value (), FREQUENCY_CENTER.Value()); // Phase, Ampl
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                        if (verbose == 1) fprintf(stderr, "1BRAM read:\n");
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START|PACPLL_CFG_TRANSPORT_SINGLE,
                                                    16,  1024, 65536, 1, AUX_SCALE.Value (), FREQUENCY_CENTER.Value()); // Phase, Ampl
                        break;
                case 7: // Tune fast
                        if (verbose > 0) fprintf(stderr, "OnNewParams: OP=5 Start-Finish, Repeat Hilevel BRAM Transport (Scope/Tune)\n");
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,
                                                    16,  1024, 65536, 1, AUX_SCALE.Value (), FREQUENCY_CENTER.Value()); // Phase, Ampl
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                        if (verbose == 1) fprintf(stderr, "1BRAM read:\n");
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START|PACPLL_CFG_TRANSPORT_SINGLE,
                                                    16,  1024, 65536, 1, AUX_SCALE.Value (), FREQUENCY_CENTER.Value()); // Phase, Ampl
                        break;
                case 8: // Tune very fast
                        if (verbose > 0) fprintf(stderr, "OnNewParams: OP=5 Start-Finish, Repeat Hilevel BRAM Transport (Scope/Tune)\n");
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,
                                                    12,  1024, 4096, 1, AUX_SCALE.Value (), FREQUENCY_CENTER.Value()); // Phase, Ampl
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                        if (verbose == 1) fprintf(stderr, "1BRAM read:\n");
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START|PACPLL_CFG_TRANSPORT_SINGLE,
                                                    12,  1024, 4096, 1, AUX_SCALE.Value (), FREQUENCY_CENTER.Value()); // Phase, Ampl
                        break;
                case 3:
                        if (verbose > 0) fprintf(stderr, "OnNewParams: OP=3 Init/ResetStart BRAM Transport\n");
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                        if (verbose == 1) fprintf(stderr, "1BRAM T init:\n");
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,
                                                    SHR_DEC_DATA.Value (),  1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
                        rp_PAC_get_single_reading (reading_vector);
                        break;
                case 4:
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,
                                                    SHR_DEC_DATA.Value (),  1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                        if (verbose > 0) fprintf(stderr, "OnNewParams: OP=4 Single Shot BRAM Transport -- test\n");
                        rp_PAC_get_single_reading (reading_vector);
                        if (verbose == 1) fprintf(stderr, "1BRAM T start:\n");
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START|PACPLL_CFG_TRANSPORT_SINGLE,
                                                    SHR_DEC_DATA.Value (), 1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
                        break;
                case 5:
                        if (verbose > 0) fprintf(stderr, "OnNewParams: OP=5 Start Loop BRAM Transport for loop/FIFO mode\n");
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_INIT,
                                                    SHR_DEC_DATA.Value (),  1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                        rp_PAC_get_single_reading (reading_vector);
                        if (verbose == 1) fprintf(stderr, "1BRAM read:\n");
                        rp_PAC_configure_transport (PACPLL_CFG_TRANSPORT_START|PACPLL_CFG_TRANSPORT_LOOP,
                                                    SHR_DEC_DATA.Value (), 1024, TRANSPORT_DECIMATION.Value (), TRANSPORT_MODE.Value (), AUX_SCALE.Value (), FREQUENCY_CENTER.Value());
                        break;
                }
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
