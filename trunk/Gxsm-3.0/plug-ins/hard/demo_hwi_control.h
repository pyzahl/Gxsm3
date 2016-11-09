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

#ifndef __DEMO_HWI_CONTROL_H
#define __DEMO_HWI_CONTROL_H

#include "gxsm/app_profile.h"
#include "SR-STD_spmcontrol/FB_spm_dataexchange.h" // Demo data exchange structs and consts

typedef enum { DestDSP, DestHwI } Destination;

typedef union {
        struct { unsigned char ch, x, y, z; } s;
        unsigned long   l;
} AmpIndex;

typedef struct{
	/* Mover Control */
	double MOV_Ampl, MOV_Steps, MOV_WavePeriod;
	
#define DSP_AFMMOV_MODES 4
	double AFM_Amp, AFM_WavePeriod, AFM_Steps;  /* STM/AFM Besocke Kontrolle -- current */
	double AFM_usrAmp[DSP_AFMMOV_MODES];  
	double AFM_usrWavePeriod[DSP_AFMMOV_MODES];
	double AFM_usrSteps[DSP_AFMMOV_MODES]; /* Parameter Memory for different Mover/Slider Modes */

#define MOV_MAXWAVELEN     4096
#define MOV_WAVE_SAWTOOTH  0
#define MOV_WAVE_SINE      1
#define MOV_WAVE_CYCLO     2
#define MOV_WAVE_CYCLO_PL  3
#define MOV_WAVE_CYCLO_MI  4
#define MOV_WAVE_CYCLO_IPL 5
#define MOV_WAVE_CYCLO_IMI 6
#define MOV_WAVE_USER      7
#define MOV_WAVE_USER_TTL  8
#define MOV_WAVE_LAST      9
	int MOV_output, MOV_mode, MOV_waveform_id;
	int MOV_wave_len, MOV_wave_speed;
	short MOV_waveform[MOV_MAXWAVELEN];
	double final_delay;
} Mover_Param;

typedef enum { PV_MODE_NONE, PV_MODE_IV, PV_MODE_FZ, PV_MODE_PL, PV_MODE_LP, PV_MODE_SP, PV_MODE_TS, PV_MODE_LM, PV_MODE_AC, PV_MODE_AX } pv_mode;
typedef enum { MAKE_VEC_FLAG_NORMAL=0, MAKE_VEC_FLAG_VHOLD=1, MAKE_VEC_FLAG_RAMP=2, MAKE_VEC_FLAG_END=4 } make_vector_flags;

 
class Demo_SPM_Control : public AppBase{
	friend class demo_hwi_dev;

#define DSP_FB_ON  1
#define DSP_FB_OFF 0

#define FLAG_FB_ON       0x01 // FB on
#define FLAG_DUAL        0x02 // Dual Data
#define FLAG_SHOW_RAMP   0x04 // show ramp data
#define FLAG_INTEGRATE   0x08 // integrate and normalize date of all AIC data point inbetween

#define FLAG_AUTO_SAVE   0x01 // auto save
#define FLAG_AUTO_PLOT   0x02 // auto plot

 public:
        Demo_SPM_Control ();
        virtual ~Demo_SPM_Control();

	void AddProbeModes (GtkWidget *notebook);

        void save_values (NcFile *ncf);
        void load_values (NcFile *ncf);

	void recalculate_dsp_scan_speed_parameters (AREA_SCAN &dsp_scan_par);
	void recalculate_dsp_scan_slope_parameters (AREA_SCAN &dsp_scan_par);
        void update();
        void updateDSP(int FbFlg=-1);
	void update_trigger (gboolean flg);

	double GetUserParam (gint n, gchar *id);
	gint SetUserParam (gint n, gchar *id, double value);

        static void ChangedNotify(Param_Control* pcs, gpointer data);
        static int ChangedAction(GtkWidget *widget, Demo_SPM_Control *dspc);
        static int feedback_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
        static int se_auto_trigger_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
        static int choice_Ampl_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
        static int auto_probe_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int LockIn_exec_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int LockIn_read_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int LockIn_write_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_exec_IV_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_write_IV_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_exec_FZ_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_write_FZ_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_exec_PL_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_write_PL_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_exec_LP_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_write_LP_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_exec_SP_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_write_SP_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_exec_TS_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_write_TS_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_exec_LM_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_write_LM_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_exec_AX_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_write_AX_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_exec_RF_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
//	static int Probing_eventcheck_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_graph_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_save_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
	static int Probing_abort_callback(GtkWidget *widget, Demo_SPM_Control *dspc);
        static int change_source_callback (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_AC_option_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_AC_auto_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_IV_option_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_IV_auto_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_FZ_option_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_FZ_auto_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_PL_option_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_PL_auto_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_LP_option_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_LP_auto_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_SP_option_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_SP_auto_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_TS_option_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_TS_auto_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_LM_option_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_LM_auto_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_AX_option_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int callback_change_AX_auto_flags (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int DSP_expert_callback (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int DSP_guru_callback (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int DSP_cret_callback (GtkWidget *widget, Demo_SPM_Control *dspc);
	static int DSP_slope_callback (GtkWidget *widget, Demo_SPM_Control *dspc);

	void StartScanPreCheck ();
	pv_mode write_vector_mode;

	void store_values ();

	double frq_ref;           //!< Frquency Reference: i.e. feedback, scan, ... dataaq. sampling rate
	int    feedback_flag;
	// -- section ANALOG_VALUES --
	double bias;              //!< Bias (usually applied to the sample)
	// -- section SPM_PI_FEEDBACK --
	double current_set_point; //!< Current Setpoint (STM; log mode)
	double voltage_set_point; //!< Universal SetPoint (AFM,...; lin mode)
	double usr_cp, usr_ci;    //!< Feedback: Const Proportional, Const Integral [user visible values]

	// -- section SCAN_EVENTS_TRIGGER
	double trigger_bias_setpoint_xp[8];
	double trigger_bias_setpoint_xm[8];

	// -- section AREA_SCAN --
	double dynamic_zoom;      //!< on-the-fly zooming, 1 = normal
	double area_slope_x;      //!< slope compensation in X, in scan coordinate system (possibly rotated) -- applied before, but by feedback
	double area_slope_y;      //!< slope compensation in Y, in scan coordinate system (possibly rotated) -- applied before, but by feedback
	int    area_slope_compensation_flag; //!< enable/disable slope compensation
	int    center_return_flag; //!< enable/disable return to center after scan
	double move_speed_x;      //!< in DAC (AIC) units per second, GXSM core computes from A/s using X-gain and Ang/DAC...
	double scan_speed_x_requested;      //!< in DAC (AIC) units per second - requested
	double scan_speed_x;      //!< in DAC (AIC) units per second - best match
	Gtk_EntryControl *scan_speed_ec;
	double gain_ratio;        //!< may be used later for compensation of direction dependence of speed in case of different X/Y gains. 
	                          //!< i.e. it is gain-X/gain-Y
	int pre_points;           //!< pre point before scan line data is taken (experimental)

	// UserEvent sensitive:
	double ue_bias;
	double ue_current_set_point;
	double ue_voltage_set_point;
	double ue_usr_cp, ue_usr_ci;
	double ue_scan_speed_x_r;
	double ue_scan_speed_x;
	double ue_slope_x;
	double ue_slope_y;
	double ue_slope_flg;

	double volt_points[10];
	int    num_points[10];

	// LockIn
	double AC_amp, AC_frq, AC_phaseA, AC_phaseB;
	double AC_phase_span, AC_phase_slope, AC_final_delay;
	int AC_points;
	int AC_repetitions;
	int AC_lockin_avg_cycels;
	int AC_option_flags;
	int AC_auto_flags;
	GtkWidget *AC_status;

	// Probing
	int probe_trigger_raster_points_user;
	int probe_trigger_raster_points;
	int probe_trigger_single_shot;
	int Source, XSource, PSource;
	int probe_ready;
	gchar *probe_fname;
	int probe_findex;

	// STS (I-V)
	void make_auto_n_vector_elments (double fnum);
	double make_Vdz_vector (double Ui, double Uf, double dZ, int n, double slope, int source, int options, double long &duration, make_vector_flags flags);
	double make_Vdx0_vector (double Ui, double Uf, double dZ, int n, double slope, int source, int options, double long &duration, make_vector_flags flags);
	double make_dx0_vector (double X0i, double X0f, int n, double slope, int source, int options, double long &duration, make_vector_flags flags);
	double make_ZXYramp_vector (double dZ, double dX, double dY, int n, double slope, int source, int options, double long &duration, make_vector_flags flags);
	double make_phase_vector (double dPhi, int n, double slope, int source, int options, double long &duration, make_vector_flags flags);
	double make_delay_vector (double delay, int source, int options, double long &duration, make_vector_flags flags, int points=0);
	void append_null_vector (int options, int index);
	double IV_start, IV_end, IV_slope, IV_slope_ramp, IV_final_delay, IV_recover_delay;
	double IV_dz;
	int    IV_points;
	int    IV_repetitions;
	int    IVdz_repetitions;
	int    IV_option_flags;
	int    IV_auto_flags;
	GtkWidget *IV_status;

	// FZ (Force-Z(Distance))
	double FZ_start, FZ_end, FZ_slope, FZ_slope_ramp, FZ_final_delay;
	int    FZ_points;
	int    FZ_repetitions;
	int    FZ_option_flags;
	int    FZ_auto_flags;
	GtkWidget *FZ_status;

	// PL (Puls)
	double PL_duration, PL_slope, PL_volt, PL_final_delay;
	int    PL_repetitions;
	int    PL_option_flags;
	int    PL_auto_flags;
	GtkWidget *PL_status;

	// LP (Laserpuls)
	double LP_duration, LP_slope, LP_volt, LP_final_delay, LP_FZ_end, LP_triggertime;
	int    LP_repetitions;
	int    LP_option_flags;
	int    LP_auto_flags;
	GtkWidget *LP_status;

	// SP (Special/Slow Puls)
	double SP_duration, SP_ramptime, SP_volt, SP_final_delay, SP_flag_volt;
	int    SP_repetitions;
	int    SP_option_flags;
	int    SP_auto_flags;
	GtkWidget *SP_status;

	// TS (Time Spectroscopy)
	double TS_duration, TS_points;
	int    TS_repetitions;
	int    TS_option_flags;
	int    TS_auto_flags;
	GtkWidget *TS_status;

	// LM (Lateral Manipulation)
	double LM_dx, LM_dy, LM_dz, LM_slope, LM_final_delay;
	int    LM_repetitions;
	int    LM_points;
	int    LM_option_flags;
	int    LM_auto_flags;
	GtkWidget *LM_status;

	// AX (Auxillary Probe -- QMA, CMA, etc.)
	double AX_to_volt, AX_resolution, AX_gain, AX_gatetime;
	double AX_start, AX_end, AX_slope, AX_slope_ramp, AX_final_delay;
	int    AX_points;
	int    AX_repetitions;
	int    AX_option_flags;
	int    AX_auto_flags;
	GtkWidget *AX_status;

	// -- Profile Displays
	int last_probe_data_index;

	// dynamic temporary probe data storage
	GSList *probedata_list;
	int num_probe_events;
	// -- Array of full expanded probe data set
#define NUM_PROBEDATA_ARRAYS 26
	GArray *garray_probedata[NUM_PROBEDATA_ARRAYS];
	int current_probe_data_index;
	int nun_valid_data_sections;
#define PROBEDATA_ARRAY_INDEX 0 // Array [0] holds the probe index over all sections
#define PROBEDATA_ARRAY_TIME  1 // Array [1] holds the time
#define PROBEDATA_ARRAY_X0    2 // Array [2] holds X-Offset
#define PROBEDATA_ARRAY_Y0    3 // Array [3] holds Y-Offset
#define PROBEDATA_ARRAY_PHI   4 // Array [4] holds Z-Offset
#define PROBEDATA_ARRAY_XS    5 // Array [5] holds X-Scan
#define PROBEDATA_ARRAY_YS    6 // Array [6] holds Y-Scan
#define PROBEDATA_ARRAY_ZS    7 // Array [7] holds Z-Scan
#define PROBEDATA_ARRAY_U     8 // Array [8] holds U (Bias)
#define PROBEDATA_ARRAY_SEC   9 // Array [9] holds Section Index
#define PROBEDATA_ARRAY_AIC5OUT_ZMON 10 // Array [10] holds ZMON (AIC5 out)
#define PROBEDATA_ARRAY_AIC6OUT_UMON 11 // Array [11] holds UMON (AIC6 out)
#define PROBEDATA_ARRAY_AIC5_FBS     12 // Array [12] holds FBS (Feedback Source, i.e. I, df, force, ...)
#define PROBEDATA_ARRAY_AIC0         13 // Array [13] holds AIC0 in
#define PROBEDATA_ARRAY_AIC1         14 // Array [14] holds AIC1 in
#define PROBEDATA_ARRAY_AIC2         15 // Array [15] holds AIC2 in
#define PROBEDATA_ARRAY_AIC3         16 // Array [16] holds AIC3 in
#define PROBEDATA_ARRAY_AIC4         17 // Array [17] holds AIC4 in
#define PROBEDATA_ARRAY_AIC6         18 // Array [18] holds AIC6 in (not used yet)
#define PROBEDATA_ARRAY_AIC7         19 // Array [19] holds AIC7 in (not used yet)
#define PROBEDATA_ARRAY_LCK0         20 // Array [20] holds LockIn0st
#define PROBEDATA_ARRAY_LCK1A        21 // Array [21] holds LockIn1st
#define PROBEDATA_ARRAY_LCK1B        22 // Array [22] holds LockIn22st
#define PROBEDATA_ARRAY_LCK2A        23 // Array [23] holds LockIn1st
#define PROBEDATA_ARRAY_LCK2B        24 // Array [24] holds LockIn22st
#define PROBEDATA_ARRAY_COUNT        25 // Array [25] holds Count

#define PROBEDATA_ARRAY_END          PROBEDATA_ARRAY_COUNT // last element number

	int    current_auto_flags;
	int    raster_auto_flags;
	GtkWidget *save_button;

 protected:
	void write_dsp_probe (int start=0, pv_mode pvm=PV_MODE_NONE){}; // dummy
	
 private:
	SPM_STATEMACHINE dsp_state;
	SPM_PI_FEEDBACK  dsp_feedback;
	ANALOG_VALUES    dsp_analog;
	AREA_SCAN        dsp_scan;
	SCAN_EVENT_TRIGGER dsp_scan_event_trigger;
	PROBE            dsp_probe;
	DATA_FIFO        dsp_fifo;
	CR_OUT_PULSE     dsp_cr_out_pulse;  // IO puls engine
	CR_GENERIC_IO    dsp_cr_generic_io; // IO and Counter control
	PROBE_VECTOR     dsp_vector;

	#define MAX_PV 50
	PROBE_VECTOR     dsp_vector_list[MAX_PV]; // copy for GXSM internal use only

	GSList *RemoteEntryList;
	GSList *FreezeEntryList;
        UnitObj *Unity, *Volt, *Angstroem, *Current, *SetPtUnit, *Speed, *PhiSpeed, *Frq, *Deg, *Vslope, *Time, *msTime, *TimeUms, *minTime;
	
	int expert_mode, guru_mode;
};


class DSPMoverControl : public AppBase{
public:
	DSPMoverControl();
	virtual ~DSPMoverControl();

	void update();
	void updateDSP(int sliderno=-1);
	static void ExecCmd(int cmd);
	static void ChangedNotify(Param_Control* pcs, gpointer data);
	static int config_mode (GtkWidget *widget, DSPMoverControl *dspc);
	static int config_waveform (GtkWidget *widget, DSPMoverControl *dspc);
	static int config_output (GtkWidget *widget, DSPMoverControl *dspc);
	static int CmdAction(GtkWidget *widget, DSPMoverControl *dspc);
	static int StopAction(GtkWidget *widget, DSPMoverControl *dspc);

	void create_waveform (double amp, double duration);
	Mover_Param mover_param;

private:
	void create_folder();

	UnitObj *Unity, *Volt, *Time, *Length;
};

#endif

