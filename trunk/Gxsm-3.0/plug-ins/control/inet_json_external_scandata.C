/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Plugin Name: inet_json_external_scandata.C
 * ========================================
 * 
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Percy Zahl <zahl@fkp.uni-hannover.de>
 * additional features: Andreas Klust <klust@fkp.uni-hannover.de>
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

/* Please do not change the Begin/End lines of this comment section!
 * this is a LaTeX style section used for auto generation of the PlugIn Manual 
 * Chapter. Add a complete PlugIn documentation inbetween the Begin/End marks!
 * All "% PlugInXXX" commentary tags are mandatory
 * All "% OptPlugInXXX" tags are optional
 * --------------------------------------------------------------------------------
% BeginPlugInDocuSection
% PlugInDocuCaption: Inet JSON Scan Data Control
% PlugInName: inet_json_external_scandata
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: windows-section Inet JSON Scan External Data
RP data streaming

% PlugInDescription

% PlugInUsage

% OptPlugInRefs

% OptPlugInNotes

% OptPlugInHints

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */


#include <gtk/gtk.h>
#include <gio/gio.h>
#include <libsoup/soup.h>
#include <zlib.h>

#include "config.h"
#include "gxsm/plugin.h"

#include "gxsm/unit.h"
#include "gxsm/pcs.h"
#include "gxsm/xsmtypes.h"
#include "gxsm/glbvars.h"
#include "gxsm/action_id.h"

#include "plug-ins/control/inet_json_external_scandata.h"

// Plugin Prototypes - default PlugIn functions
static void inet_json_external_scandata_init (void); // PlugIn init
static void inet_json_external_scandata_query (void); // PlugIn "self-install"
static void inet_json_external_scandata_about (void); // About
static void inet_json_external_scandata_configure (void); // Configure plugIn, called via PlugIn-Configurator
static void inet_json_external_scandata_cleanup (void); // called on PlugIn unload, should cleanup PlugIn rescources

// other PlugIn Functions and Callbacks (connected to Buttons, Toolbar, Menu)
static void inet_json_external_scandata_show_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data);
static void inet_json_external_scandata_SaveValues_callback ( gpointer );

// Fill in the GxsmPlugin Description here -- see also: Gxsm/src/plugin.h
GxsmPlugin inet_json_external_scandata_pi = {
	NULL,                   // filled in and used by Gxsm, don't touch !
	NULL,                   // filled in and used by Gxsm, don't touch !
	0,                      // filled in and used by Gxsm, don't touch !
	NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
	"Inet_Json_External_Scandata",
	NULL,
	NULL,
	"Percy Zahl",
	"windows-section", // Menu-path/section
	N_("Inet JSON RP"), // Menu Entry -- overridden my set-window-geometry() call automatism
	N_("Open Inet JSON External Scan Data Control Window"),
	"Inet JSON External Scan Data Control Window", // help text
	NULL,          // error msg, plugin may put error status msg here later
	NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
	inet_json_external_scandata_init,  
	inet_json_external_scandata_query,  
	// about-function, can be "NULL"
	// can be called by "Plugin Details"
	inet_json_external_scandata_about,
	// configure-function, can be "NULL"
	// can be called by "Plugin Details"
	inet_json_external_scandata_configure,
	// run-function, can be "NULL", if non-Zero and no query defined, 
	// it is called on menupath->"plugin"
	NULL,
	// cleanup-function, can be "NULL"
	// called if present at plugin removal
	inet_json_external_scandata_show_callback, // direct menu entry callback1 or NULL
	NULL, // direct menu entry callback2 or NULL

	inet_json_external_scandata_cleanup
};

// Text used in Aboutbox, please update!!
static const char *about_text = N_("Inet JSON External Scan Data Control Plugin\n\n"
                                   "This plugin manages externa Scan Data Sources.\n"
	);

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
	inet_json_external_scandata_pi.description = g_strdup_printf(N_("Gxsm inet_json_external_scandata plugin %s"), VERSION);
	return &inet_json_external_scandata_pi; 
}

// data passed to "idle" function call, used to refresh/draw while waiting for data
typedef struct {
	GSList *scan_list; // scans to update
	GFunc  UpdateFunc; // function to call for background updating
	gpointer data; // additional data (here: reference to the current Inet_Json_External_Scandata object)
} IdleRefreshFuncData;

Inet_Json_External_Scandata *inet_json_external_scandata = NULL;

// Query Function, installs Plugin's in File/Import and Export Menupaths!

#define REMOTE_PREFIX "INET_JSON_EX_"

static void inet_json_external_scandata_query(void)
{
	if(inet_json_external_scandata_pi.status) g_free(inet_json_external_scandata_pi.status); 
	inet_json_external_scandata_pi.status = g_strconcat (
                                                 N_("Plugin query has attached "),
                                                 inet_json_external_scandata_pi.name, 
                                                 N_(": File IO Filters are ready to use"),
                                                 NULL);

	PI_DEBUG (DBG_L2, "inet_json_external_scandata_query:new" );
	inet_json_external_scandata = new Inet_Json_External_Scandata;

	PI_DEBUG (DBG_L2, "inet_json_external_scandata_query:res" );
	
	inet_json_external_scandata_pi.app->ConnectPluginToCDFSaveEvent (inet_json_external_scandata_SaveValues_callback);
}

static void inet_json_external_scandata_SaveValues_callback ( gpointer gp_ncf ){

	//NcFile *ncf = (NcFile *) gp_ncf;
	//NcDim* spmscd  = ncf->add_dim("inet_json_external_scandata_dim", strlen(tmp));
	//NcVar* spmsc   = ncf->add_var("inet_json_external_scandata", ncChar, spmscd);
	//spmsc->add_att("long_name", "inet_json_external_scandata: scan direction");
	//spmsc->put(tmp, strlen(tmp));
	//g_free (tmp);
}


// 5.) Start here with the plugins code, vars def., etc.... here.
// ----------------------------------------------------------------------
//


// init-Function
static void inet_json_external_scandata_init(void)
{
  PI_DEBUG (DBG_L2, "inet_json_external_scandata Plugin Init" );
}

// about-Function
static void inet_json_external_scandata_about(void)
{
        const gchar *authors[] = { inet_json_external_scandata_pi.authors, NULL};
        gtk_show_about_dialog (NULL,
                               "program-name",  inet_json_external_scandata_pi.name,
                               "version", VERSION,
                               "license", GTK_LICENSE_GPL_3_0,
                               "comments", about_text,
                               "authors", authors,
                               NULL
                               );
}

// configure-Function
static void inet_json_external_scandata_configure(void)
{
	if(inet_json_external_scandata_pi.app)
		inet_json_external_scandata_pi.app->message("inet_json_external_scandata Plugin Configuration");
}

// cleanup-Function, make sure the Menustrings are matching those above!!!
static void inet_json_external_scandata_cleanup(void)
{
	// delete ...
	if( inet_json_external_scandata )
		delete inet_json_external_scandata ;

	if(inet_json_external_scandata_pi.status) g_free(inet_json_external_scandata_pi.status); 
}

static void inet_json_external_scandata_show_callback(GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	PI_DEBUG (DBG_L2, "inet_json_external_scandata Plugin : show" );
	if( inet_json_external_scandata )
		inet_json_external_scandata->show();
}


Inet_Json_External_Scandata::Inet_Json_External_Scandata ()
{
        GtkWidget *tmp;
        GtkWidget *wid;
	
	GSList *EC_R_list=NULL;
	GSList *EC_QC_list=NULL;

        debug_level = 0;
        input_rpaddress = NULL;
        text_status = NULL;
        streaming = 0;

        ch_freq = -1;
        ch_ampl = -1;
        deg_extend = 1;

        for (int i=0; i<5; ++i){
                scope_ac[i]=false;
                scope_dc_level[i]=0.;
                gain_scale[i] = 0.001; // 1000mV full scale
        }
        block_message = 0;
        
        /* create a new connection, init */

	listener=NULL;
        port=9002;

	session=NULL;
	msg=NULL;
	client=NULL;
	client_error=NULL;
	error=NULL;
        
	PI_DEBUG (DBG_L2, "inet_json_external_scandata Plugin : building interface" );

	Unity    = new UnitObj(" "," ");
	Hz       = new UnitObj("Hz","Hz");
	Deg      = new UnitObj(UTF8_DEGREE,"deg");
	VoltDeg  = new UnitObj("V/" UTF8_DEGREE, "V/deg");
	Volt     = new UnitObj("V","V");
	mVolt     = new UnitObj("mV","mV");
	VoltHz   = new UnitObj("mV/Hz","mV/Hz");
	dB       = new UnitObj("dB","dB");
	Time     = new UnitObj("s","s");
	mTime    = new LinUnit("ms", "ms", "Time");
	uTime    = new LinUnit(UTF8_MU "s", "us", "Time");

        // Window Title
	AppWindowInit("Inet JSON External Scan Data Control for High Speed RedPitaya PACPLL");

        gchar *gs_path = g_strdup_printf ("%s.inet_json_settings", GXSM_RES_BASE_PATH_DOT);
        inet_json_settings = g_settings_new (gs_path);

        bp = new BuildParam (v_grid);
        bp->set_no_spin (true);

        bp->set_pcs_remote_prefix ("rp-pacpll-");

        bp->set_input_width_chars (16);
        bp->set_default_ec_change_notice_fkt (NULL, NULL);

        bp->new_grid_with_frame ("RedPitaya PAC Setup");
        bp->set_input_nx (2);
        bp->grid_add_ec ("In1 Offset", mVolt, &parameters.dc_offset, -1000.0, 1000.0, "g", 0.1, 1., "DC-OFFSET");
        EC_R_list = g_slist_prepend( EC_R_list, bp->ec);
        bp->ec->Freeze ();
        bp->new_line ();
        parameters.pac_dctau = 10.0; // ms
        parameters.pactau = 40.0; // us
        parameters.pacatau = 30.0; // us
        parameters.frequency_manual = 32768.0; // Hz
        parameters.frequency_center = 32768.0; // Hz
        parameters.aux_scale = 0.011642; // 20Hz / V equivalent 
        parameters.volume_manual = 300.0; // mV
        parameters.qc_gain=0.0; // gain +/-1.0
        parameters.qc_phase=0.0; // deg
        bp->set_input_width_chars (8);
        bp->set_input_nx (1);
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::pac_tau_parameter_changed, this);
  	bp->grid_add_ec ("Tau DC", mTime, &parameters.pac_dctau, -1.0, 63e6, "6g", 10., 1., "PAC-DCTAU");
        bp->new_line ();
  	bp->grid_add_ec ("Tau PAC", uTime, &parameters.pactau, 0.0, 63e6, "6g", 10., 1., "PACTAU");
  	bp->grid_add_ec (NULL, uTime, &parameters.pacatau, 0.0, 63e6, "6g", 10., 1., "PACATAU");
        bp->new_line ();
        bp->set_no_spin (false);
        bp->set_input_nx (2);
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::qc_parameter_changed, this);
  	bp->grid_add_ec ("QControl",Deg, &parameters.qc_phase, 0.0, 360.0, "5g", 10., 1., "QC-PHASE");
        EC_QC_list = g_slist_prepend( EC_QC_list, bp->ec);
        bp->ec->Freeze ();
        bp->new_line ();
  	bp->grid_add_ec ("QC Gain", Unity, &parameters.qc_gain, -1.0, 1.0, "4g", 10., 1., "QC-GAIN");
        EC_QC_list = g_slist_prepend( EC_QC_list, bp->ec);
        bp->ec->Freeze ();
        bp->new_line ();
        bp->set_no_spin (false);
        bp->set_input_width_chars (12);
        bp->set_input_nx (2);
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::pac_frequency_parameter_changed, this);
  	bp->grid_add_ec ("Frequency", Hz, &parameters.frequency_manual, 0.0, 20e6, ".3lf", 0.1, 10., "FREQUENCY-MANUAL");
        bp->new_line ();
        bp->set_no_spin (true);
        bp->set_input_width_chars (8);
        bp->set_input_nx (1);
  	bp->grid_add_ec ("Center,Scale", Hz, &parameters.frequency_center, 0.0, 20e6, ".3lf", 0.1, 10., "FREQUENCY-CENTER");
  	bp->grid_add_ec (NULL, Unity, &parameters.aux_scale, -1e6, 1e6, ".6lf", 0.1, 10., "AUX-SCALE");
        bp->new_line ();
        bp->set_no_spin (false);
        bp->set_input_nx (2);
        bp->set_input_width_chars (12);
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::pac_volume_parameter_changed, this);
  	bp->grid_add_ec ("Volume", mVolt, &parameters.volume_manual, 0.0, 1000.0, "5g", 0.1, 1.0, "VOLUME-MANUAL");
        bp->new_line ();
        bp->set_no_spin (true);
        bp->set_input_width_chars (10);
        parameters.tune_dfreq = 0.1;
        parameters.tune_span = 50.0;
        bp->set_input_nx (1);
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::tune_parameter_changed, this);
  	bp->grid_add_ec ("Tune dF,Span", Hz, &parameters.tune_dfreq, 1e-4, 1e3, "g", 0.01, 0.1, "TUNE-DFREQ");
  	bp->grid_add_ec (NULL, Hz, &parameters.tune_span, 0.0, 1e6, "g", 0.1, 10., "TUNE-SPAN");

        bp->pop_grid ();
        bp->set_default_ec_change_notice_fkt (NULL, NULL);

        bp->new_grid_with_frame ("Amplitude Controller");
        bp->set_input_nx (3);
        bp->grid_add_ec ("Reading", mVolt, &parameters.volume_monitor, -1.0, 1.0, "g", 0.1, 1., "VOLUME-MONITOR");
        EC_R_list = g_slist_prepend( EC_R_list, bp->ec);
        bp->ec->Freeze ();
        bp->new_line ();
        parameters.amplitude_fb_setpoint = 20.0; // mV
        parameters.amplitude_fb_invert = 1.;
        parameters.amplitude_fb_cp_db = -25.;
        parameters.amplitude_fb_ci_db = -40.;
        parameters.exec_fb_upper = 300.0;
        parameters.exec_fb_lower = -300.0;
        bp->set_no_spin (false);
        bp->set_input_width_chars (10);

        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::amp_ctrl_parameter_changed, this);
        bp->grid_add_ec ("Setpoint", mVolt, &parameters.amplitude_fb_setpoint, 0.0, 1000.0, "5g", 0.1, 10.0, "AMPLITUDE-FB-SETPOINT");
        bp->new_line ();
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::amplitude_gain_changed, this);
        bp->grid_add_ec ("CP gain", dB, &parameters.amplitude_fb_cp_db, -200.0, 200.0, "g", 0.1, 1.0, "AMPLITUDE-FB-CP");
        bp->new_line ();
        bp->grid_add_ec ("CI gain", dB, &parameters.amplitude_fb_ci_db, -200.0, 200.0, "g", 0.1, 1.0, "AMPLITUDE-FB-CI");
        bp->new_line ();
        bp->set_no_spin (true);
        bp->set_input_width_chars (16);
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::amp_ctrl_parameter_changed, this);
        bp->set_input_nx (1);
        bp->set_input_width_chars (10);
        bp->grid_add_ec ("Limits", mVolt, &parameters.exec_fb_lower, -1000.0, 1000.0, "g", 1.0, 10.0, "EXEC-FB-LOWER");
        bp->grid_add_ec ("...", mVolt, &parameters.exec_fb_upper, 0.0, 1000.0, "g", 1.0, 10.0, "EXEC-FB-UPPER");
        bp->new_line ();
        bp->set_input_width_chars (16);
        bp->set_input_nx (3);
        bp->set_default_ec_change_notice_fkt (NULL, NULL);
        bp->grid_add_ec ("Exec Amp", mVolt, &parameters.exec_amplitude_monitor, -1000.0, 1000.0, "g", 0.1, 1., "EXEC-AMPLITUDE-MONITOR");
        EC_R_list = g_slist_prepend( EC_R_list, bp->ec);
        bp->ec->Freeze ();
        bp->new_line ();
        bp->set_input_nx (1);
        bp->grid_add_check_button ( N_("Enable"), "Enable Amplitude Controller", 2,
                                    G_CALLBACK (Inet_Json_External_Scandata::amplitude_controller), this);
        bp->grid_add_check_button ( N_("Invert"), "Invert Amplitude Controller Gain. Normally positive.", 2,
                                    G_CALLBACK (Inet_Json_External_Scandata::amplitude_controller_invert), this);

        bp->new_line ();
        bp->grid_add_check_button ( N_("Set Auto Trigger SS"), "Set Auto Trigger SS", 2,
                                    G_CALLBACK (Inet_Json_External_Scandata::set_ss_auto_trigger), this);
        bp->grid_add_check_button ( N_("QControl"), "QControl", 2,
                                    G_CALLBACK (Inet_Json_External_Scandata::qcontrol), this);
	g_object_set_data( G_OBJECT (bp->button), "QC_SETTINGS_list", EC_QC_list);

        bp->pop_grid ();

        bp->new_grid_with_frame ("Phase Controller");
        bp->set_input_nx (3);
        bp->grid_add_ec ("Reading", Deg, &parameters.phase_monitor, -180.0, 180.0, "g", 1., 10., "PHASE-MONITOR");
        EC_R_list = g_slist_prepend( EC_R_list, bp->ec);
        bp->ec->Freeze ();
        bp->new_line ();
        parameters.phase_fb_setpoint = 60.;
        parameters.phase_fb_invert = 1.;
        parameters.phase_fb_cp_db = -76.;
        parameters.phase_fb_ci_db = -143.;
        parameters.freq_fb_upper = 35000.;
        parameters.freq_fb_lower = 28000.;
        bp->set_no_spin (false);
        bp->set_input_width_chars (8);
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::phase_ctrl_parameter_changed, this);
        bp->grid_add_ec ("Setpoint", Deg, &parameters.phase_fb_setpoint, -180.0, 180.0, "5g", 0.1, 1.0, "PHASE-FB-SETPOINT");
        bp->new_line ();
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::phase_gain_changed, this);
        bp->grid_add_ec ("CP gain", dB, &parameters.phase_fb_cp_db, -200.0, 200.0, "g", 0.1, 1.0, "PHASE-FB-CP");
        bp->new_line ();
        bp->grid_add_ec ("CI gain", dB, &parameters.phase_fb_ci_db, -200.0, 200.0, "g", 0.1, 1.0, "PHASE-FB-CI");
        bp->new_line ();
        bp->set_no_spin (true);
        bp->set_input_width_chars (16);
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::phase_ctrl_parameter_changed, this);
        bp->set_input_width_chars (10);
        bp->set_input_nx (1);
        bp->grid_add_ec ("Limits", Hz, &parameters.freq_fb_lower, 0.0, 25e6, "g", 0.1, 1.0, "FREQ-FB-LOWER");
        bp->grid_add_ec ("...", Hz, &parameters.freq_fb_upper, 0.0, 25e6, "g", 0.1, 1.0, "FREQ-FB-UPPER");
        bp->new_line ();
        bp->set_input_width_chars (16);
        bp->set_input_nx (3);
        bp->set_default_ec_change_notice_fkt (NULL, NULL);
        bp->grid_add_ec ("DDS Freq", Hz, &parameters.dds_frequency_monitor, 0.0, 25e6, ".4lf", 0.1, 1., "DDS-FREQ-MONITOR");
        EC_R_list = g_slist_prepend( EC_R_list, bp->ec);
        input_ddsfreq=bp->ec;
        bp->ec->Freeze ();
        bp->new_line ();
        bp->set_input_nx (1);
        bp->grid_add_check_button ( N_("Enable"), "Enable Phase Controller", 2,
                                    G_CALLBACK (Inet_Json_External_Scandata::phase_controller), this);
        bp->grid_add_check_button ( N_("Invert"), "Invert Phase Controller Gain. Normally positive.", 2,
                                    G_CALLBACK (Inet_Json_External_Scandata::phase_controller_invert), this);
        bp->new_line ();
        bp->grid_add_check_button ( N_("Unwapping"), "Always unwrap phase/auto unwrap only if controller is enabled", 2,
                                    G_CALLBACK (Inet_Json_External_Scandata::phase_unwrapping_always), this);
        bp->pop_grid ();
        bp->new_line ();

        // ========================================
        
        bp->new_grid_with_frame ("Oscilloscope and Data Transfer Selection -- Experimental", 10);
        // OPERATION MODE
	wid = gtk_combo_box_text_new ();
        g_signal_connect (G_OBJECT (wid), "changed",
                          G_CALLBACK (Inet_Json_External_Scandata::choice_operation_callback),
                          this);
        bp->grid_add_widget (wid);

        const gchar *operation_modes[] = {
                "MANUAL",
                "MEASURE DC_OFFSET",
                "RUN SCOPE",
                "INIT BRAM TRANSPORT",
                "SINGLE SHOT",
                "START BRAM LOOP",
                "RUN TUNE",
                "RUN TUNE F",
                "RUN TUNE FF",
                NULL };

        // Init choicelist
	for(int i=0; operation_modes[i]; i++)
                gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), operation_modes[i], operation_modes[i]);

	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 2);

        // FPGA Update Period
	wid = gtk_combo_box_text_new ();
        g_signal_connect (G_OBJECT (wid), "changed",
                          G_CALLBACK (Inet_Json_External_Scandata::choice_update_period_callback),
                          this);
        bp->grid_add_widget (wid);

	const gchar *update_periods[] = {
                "* 3/131us",
                "  4/262us",
                "  6/1.05ms",
                " 10/16.8ms",
                " 14/--",
                " 16/--",
                " 20/--",
                " 24/--",
                NULL };
   
	// Init choicelist
	for(int i=0; update_periods[i]; i++)
                gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), update_periods[i], update_periods[i]);

	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 3);

        bp->grid_add_label (" Scale");
                
        // Scope Auto Set Modes
	wid = gtk_combo_box_text_new ();
        g_signal_connect (G_OBJECT (wid), "changed",
                          G_CALLBACK (Inet_Json_External_Scandata::choice_auto_set_callback),
                          this);
        bp->grid_add_widget (wid);

	const gchar *auto_set_modes[] = {
                "Auto Set CH1",
                "Auto Set CH2",
                "Auto Set CH3",
                "Auto Set CH4",
                "Auto Set CH5",
                "Default All=1V",
                "Default All=1x",
                "Default All=10x",
                "Manual",
                NULL };
   
	// Init choicelist
	for(int i=0; auto_set_modes[i]; i++)
                gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), auto_set_modes[i], auto_set_modes[i]);

	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 6);

        bp->new_line ();

        // BRAM TRANSPORT MODE BLOCK S1,S2
	wid = gtk_combo_box_text_new ();
        g_signal_connect (G_OBJECT (wid), "changed",
                          G_CALLBACK (Inet_Json_External_Scandata::choice_transport_ch12_callback),
                          this);
        bp->grid_add_widget (wid);

	const gchar *transport_modes[] = {
                "OFF: no plot",
                "IN1, IN2",
                "Phase, Ampl",
                "IN1, Ampl",
                "IN1, Phase",
                "Exec, Freq",
                "Ampl, Exec",
                "Phase, Freq",
                "M-DCiir, Ampl",
                "8BIT GPIO, px clk",
                NULL };
   
	// Init choicelist
	for(int i=0; transport_modes[i]; i++)
                gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), transport_modes[i], transport_modes[i]);

        channel_selections[5] = 1;
        channel_selections[6] = 1;
        channel_selections[0] = 1;
        channel_selections[1] = 1;
	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 1);

        // GPIO monitor selections -- full set, experimental
	const gchar *monitor_modes_gpio[] = {
                "OFF: no plot",
                "LMS Amplitude(A,B)",
                "LMS Phase(A,B)",
                "LMS A (Real)",
                "LMS B (Imag)",
                "SQRT Ampl Monitor",
                "ATAN Phase Monitor",
                "X5",
                "X6",
                "Exec Amplitude",
                "DDS Freq Monitor",
                "X3 M (LMS Input)",
                "X5 M1(LMS Input-DC)",
                "X11 BRAM WPOS",
                "X12 BRAM DEC",
                NULL };

        // CH3 from GPIO MONITOR</p>
	wid = gtk_combo_box_text_new ();
        g_signal_connect (G_OBJECT (wid), "changed",
                          G_CALLBACK (Inet_Json_External_Scandata::choice_transport_ch3_callback),
                          this);
        bp->grid_add_widget (wid);

	// Init choicelist
	for(int i=0; monitor_modes_gpio[i]; i++)
                gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), monitor_modes_gpio[i], monitor_modes_gpio[i]);

        channel_selections[2] = 0;
	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 0);

        // CH4 from GPIO MONITOR</p>
	wid = gtk_combo_box_text_new ();
        g_signal_connect (G_OBJECT (wid), "changed",
                          G_CALLBACK (Inet_Json_External_Scandata::choice_transport_ch4_callback),
                          this);
        bp->grid_add_widget (wid);

	// Init choicelist
	for(int i=0; monitor_modes_gpio[i]; i++)
                gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), monitor_modes_gpio[i], monitor_modes_gpio[i]);

        channel_selections[3] = 0;
	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 0);

        // CH5 from GPIO MONITOR</p>
	wid = gtk_combo_box_text_new ();
        g_signal_connect (G_OBJECT (wid), "changed",
                          G_CALLBACK (Inet_Json_External_Scandata::choice_transport_ch5_callback),
                          this);
        bp->grid_add_widget (wid);

	// Init choicelist
	for(int i=0; monitor_modes_gpio[i]; i++)
                gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), monitor_modes_gpio[i], monitor_modes_gpio[i]);

        channel_selections[4] = 0;
	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 0);


        // Scope Display
        bp->new_line ();
        signal_graph = gtk_image_new_from_surface (NULL);
        bp->grid_add_widget (signal_graph, 10);
        
        bp->new_line ();
        bp->grid_add_check_button ( N_("Ch1 AC"), "Remove Offset from Ch1", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::scope_ac_ch1_callback), this);
        bp->grid_add_check_button ( N_("Ch2 AC"), "Remove Offset from Ch2", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::scope_ac_ch2_callback), this);
        bp->grid_add_check_button ( N_("Ch3 AC"), "Remove Offset from Ch3", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::scope_ac_ch3_callback), this);

        
        // ========================================
        
        bp->pop_grid ();
        bp->new_line ();
        
        bp->new_grid_with_frame ("RedPitaya Web Socket Address for JSON talk", 10);

        bp->set_input_width_chars (25);
        input_rpaddress = bp->grid_add_input ("RedPitaya Address");

        g_settings_bind (inet_json_settings, "redpitay-address",
                         G_OBJECT (bp->input), "text",
                         G_SETTINGS_BIND_DEFAULT);
        gtk_widget_set_tooltip_text (input_rpaddress, "RedPitaya IP Address like rp-f05603.local or 130.199.123.123");
        //  "ws://rp-f05603.local:9002/"
        //gtk_entry_set_text (GTK_ENTRY (input_rpaddress), "http://rp-f05603.local/pacpll/?type=run");
        //gtk_entry_set_text (GTK_ENTRY (input_rpaddress), "130.199.243.200");
        //gtk_entry_set_text (GTK_ENTRY (input_rpaddress), "192.168.1.10");
        
        bp->grid_add_check_button ( N_("Connect"), "Check to initiate connection, uncheck to close connection.", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::connect_cb), this);
        bp->grid_add_check_button ( N_("Scope"), "Enable Scope", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::enable_scope), this);
        bp->grid_add_check_button ( N_("Debug"), "Enable debugging LV1.", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::dbg_l1), this);
        bp->grid_add_check_button ( N_("+"), "Debug LV2", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::dbg_l2), this);
        bp->grid_add_check_button ( N_("++"), "Debug LV4", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::dbg_l4), this);
        
        //bp->new_line ();
        //tmp=bp->grid_add_button ( N_("Read"), "TEST READ", 1,
        //                          G_CALLBACK (Inet_Json_External_Scandata::read_cb), this);
        //tmp=bp->grid_add_button ( N_("Write"), "TEST WRITE", 1,
        //                          G_CALLBACK (Inet_Json_External_Scandata::write_cb), this);

        bp->new_line ();
        bp->set_input_width_chars (80);
        red_pitaya_health = bp->grid_add_input ("RedPitaya Health",10);
        gtk_widget_set_sensitive (bp->input, FALSE);
        gtk_editable_set_editable (GTK_EDITABLE (bp->input), FALSE); 
        update_health ("Not connected.");
        bp->new_line ();

        text_status = gtk_text_view_new ();
 	gtk_text_view_set_editable (GTK_TEXT_VIEW (text_status), FALSE);
        //gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_status), GTK_WRAP_WORD_CHAR);
        GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
        //gtk_widget_set_size_request (scrolled_window, 200, 60);
        gtk_widget_set_hexpand (scrolled_window, TRUE);
        gtk_widget_set_vexpand (scrolled_window, TRUE);
        gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (text_status)) ;
        bp->grid_add_widget (scrolled_window, 10);

        // ========================================
        bp->pop_grid ();
        
        bp->show_all ();
 
        // save List away...
	//g_object_set_data( G_OBJECT (window), "INETJSONSCANCONTROL_EC_list", EC_list);

	g_object_set_data( G_OBJECT (window), "PAC_EC_READINGS_list", EC_R_list);

        set_window_geometry ("inet-json-rp-control"); // needs rescoure entry and defines window menu entry as geometry is managed

	inet_json_external_scandata_pi.app->RemoteEntryList = g_slist_concat (inet_json_external_scandata_pi.app->RemoteEntryList, bp->remote_list_ec);

        
        // hookup to scan start and stop
        inet_json_external_scandata_pi.app->ConnectPluginToStartScanEvent (Inet_Json_External_Scandata::scan_start_callback);
        inet_json_external_scandata_pi.app->ConnectPluginToStopScanEvent (Inet_Json_External_Scandata::scan_stop_callback);
}

Inet_Json_External_Scandata::~Inet_Json_External_Scandata (){
	delete mTime;
	delete uTime;
	delete Time;
	delete dB;
	delete VoltHz;
	delete Volt;
	delete mVolt;
	delete VoltDeg;
	delete Deg;
	delete Hz;
	delete Unity;
}

void Inet_Json_External_Scandata::scan_start_callback (gpointer user_data){
        //Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        Inet_Json_External_Scandata *self = inet_json_external_scandata;
        self->ch_freq = -1;
        self->ch_ampl = -1;
        self->streaming = 1;
        self->operation_mode = 0;
        g_message ("Inet_Json_External_Scandata::scan_start_callback");
        if ((self->ch_freq=gapp->xsm->FindChan(xsmres.extchno[0])) >= 0)
                self->setup_scan (self->ch_freq, "X+", "Ext1-Freq", "Hz", "Freq", 1.0);
        if ((self->ch_ampl=gapp->xsm->FindChan(xsmres.extchno[1])) >= 0)
                self->setup_scan (self->ch_ampl, "X+", "Ext1-Ampl", "V", "Ampl", 1.0);
}

void Inet_Json_External_Scandata::scan_stop_callback (gpointer user_data){
        //Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        Inet_Json_External_Scandata *self = inet_json_external_scandata;
        self->ch_freq = -1;
        self->ch_ampl = -1;
        self->streaming = 0;
        g_message ("Inet_Json_External_Scandata::scan_stop_callback");
}

int Inet_Json_External_Scandata::setup_scan (int ch, 
				 const gchar *titleprefix, 
				 const gchar *name,
				 const gchar *unit,
				 const gchar *label,
				 double d2u
	){
	// did this scan already exists?
	if ( ! gapp->xsm->scan[ch]){ // make a new one ?
		gapp->xsm->scan[ch] = gapp->xsm->NewScan (gapp->xsm->ChannelView[ch], 
							  gapp->xsm->data.display.ViewFlg, 
							  ch, 
							  &gapp->xsm->data);
		// Error ?
		if (!gapp->xsm->scan[ch]){
			XSM_SHOW_ALERT (ERR_SORRY, ERR_NOMEM,"",1);
			return FALSE;
		}
	}


	Mem2d *m=gapp->xsm->scan[ch]->mem2d;
        m->Resize (m->GetNx (), m->GetNy (), m->GetNv (), ZD_DOUBLE, false); // multilayerinfo=clean
	
	// Setup correct Z unit
	UnitObj *u = gapp->xsm->MakeUnit (unit, label);
	gapp->xsm->scan[ch]->data.SetZUnit (u);
	delete u;
		
        gapp->xsm->scan[ch]->create (TRUE, FALSE, strchr (titleprefix, '-') ? -1.:1., gapp->xsm->hardware->IsFastScan ());

	// setup dz from instrument definition or propagated via signal definition
	if (fabs (d2u) > 0.)
		gapp->xsm->scan[ch]->data.s.dz = d2u;
	else
		gapp->xsm->scan[ch]->data.s.dz = gapp->xsm->Inst->ZResolution (unit);
	
	// set scan title, name, ... and draw it!

	gchar *scantitle = NULL;
	if (!gapp->xsm->GetMasterScan ()){
		gapp->xsm->SetMasterScan (gapp->xsm->scan[ch]);
		scantitle = g_strdup_printf ("M %s %s", titleprefix, name);
	} else {
		scantitle = g_strdup_printf ("%s %s", titleprefix, name);
	}
	gapp->xsm->scan[ch]->data.ui.SetName (scantitle);
	gapp->xsm->scan[ch]->data.ui.SetOriginalName ("unknown");
	gapp->xsm->scan[ch]->data.ui.SetTitle (scantitle);
	gapp->xsm->scan[ch]->data.ui.SetType (scantitle);
	gapp->xsm->scan[ch]->data.s.xdir = strchr (titleprefix, '-') ? -1.:1.;
	gapp->xsm->scan[ch]->data.s.ydir = gapp->xsm->data.s.ydir;

        streampos=x=y=0; // assume top down full size
        
	PI_DEBUG (DBG_L2, "setup_scan[" << ch << " ]: scantitle done: " << gapp->xsm->scan[ch]->data.ui.type ); 

	g_free (scantitle);
	gapp->xsm->scan[ch]->draw ();

	return 0;
}


void Inet_Json_External_Scandata::pac_tau_parameter_changed (Param_Control* pcs, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->write_parameter ("PAC_DCTAU", self->parameters.pac_dctau);
        self->write_parameter ("PACTAU", self->parameters.pactau);
        self->write_parameter ("PACATAU", self->parameters.pacatau);
}

void Inet_Json_External_Scandata::pac_frequency_parameter_changed (Param_Control* pcs, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->write_parameter ("FREQUENCY_MANUAL", self->parameters.frequency_manual);
        self->write_parameter ("FREQUENCY_CENTER", self->parameters.frequency_center);
        self->write_parameter ("AUX_SCALE", self->parameters.aux_scale);
}

void Inet_Json_External_Scandata::pac_volume_parameter_changed (Param_Control* pcs, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->write_parameter ("VOLUME_MANUAL", self->parameters.volume_manual);
}

static void freeze_ec(Gtk_EntryControl* ec, gpointer data){ ec->Freeze (); };
static void thaw_ec(Gtk_EntryControl* ec, gpointer data){ ec->Thaw (); };


void Inet_Json_External_Scandata::qcontrol (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->write_parameter ("QCONTROL", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
        self->parameters.qcontrol = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

        if (self->parameters.qcontrol)
                g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (widget), "QC_SETTINGS_list"),
				(GFunc) thaw_ec, NULL);
        else
                g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (widget), "QC_SETTINGS_list"),
				(GFunc) freeze_ec, NULL);
}

void Inet_Json_External_Scandata::qc_parameter_changed (Param_Control* pcs, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->write_parameter ("QC_GAIN", self->parameters.qc_gain);
        self->write_parameter ("QC_PHASE", self->parameters.qc_phase);
}

void Inet_Json_External_Scandata::tune_parameter_changed (Param_Control* pcs, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->write_parameter ("TUNE_DFREQ", self->parameters.tune_dfreq);
        self->write_parameter ("TUNE_SPAN", self->parameters.tune_span);
}

void Inet_Json_External_Scandata::amp_ctrl_parameter_changed (Param_Control* pcs, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->write_parameter ("AMPLITUDE_FB_SETPOINT", self->parameters.amplitude_fb_setpoint);
        self->write_parameter ("EXEC_FB_UPPER", self->parameters.exec_fb_upper);
        self->write_parameter ("EXEC_FB_LOWER", self->parameters.exec_fb_lower);
}

void Inet_Json_External_Scandata::phase_ctrl_parameter_changed (Param_Control* pcs, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->write_parameter ("PHASE_FB_SETPOINT", self->parameters.phase_fb_setpoint);
        self->write_parameter ("FREQ_FB_UPPER", self->parameters.freq_fb_upper);
        self->write_parameter ("FREQ_FB_LOWER", self->parameters.freq_fb_lower);
}

void Inet_Json_External_Scandata::send_all_parameters (){
        write_parameter ("GAIN1", 1.);
        write_parameter ("GAIN2", 1.);
        write_parameter ("GAIN3", 1.);
        write_parameter ("GAIN4", 1.);
        write_parameter ("GAIN5", 1.);
        write_parameter ("SHR_DEC_DATA", 4.);
        write_parameter ("PACVERBOSE", 0);
        write_parameter ("TRANSPORT_DECIMATION", 16);
        write_parameter ("TRANSPORT_MODE", 0);
        write_parameter ("OPERATION", 2);
        pac_tau_parameter_changed (NULL, this);
        pac_frequency_parameter_changed (NULL, this);
        pac_volume_parameter_changed (NULL, this);
        qc_parameter_changed (NULL, this);
        tune_parameter_changed (NULL, this);
        amp_ctrl_parameter_changed (NULL, this);
        amplitude_gain_changed (NULL, this);
        phase_ctrl_parameter_changed (NULL, this);
        phase_gain_changed (NULL, this);
}

void Inet_Json_External_Scandata::choice_operation_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->operation_mode = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
        self->write_parameter ("OPERATION", self->operation_mode);
}

void Inet_Json_External_Scandata::choice_update_period_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->data_shr_max = -1;
        switch (gtk_combo_box_get_active (GTK_COMBO_BOX (widget))){
        case 0: self->write_parameter ("SIGNAL_PERIOD",  20);
                self->data_shr_max = -1;
                break;
        case 1: self->write_parameter ("SIGNAL_PERIOD",  50);
                self->data_shr_max = 4;
                break;
        case 2: self->write_parameter ("SIGNAL_PERIOD", 100);
                self->write_parameter ("PARAMETER_PERIOD",  100);
                self->data_shr_max = 6;
                break;
        case 3: self->write_parameter ("SIGNAL_PERIOD", 200);
                self->write_parameter ("PARAMETER_PERIOD",  200);
                self->data_shr_max = 10;
                break;
        case 4: self->write_parameter ("SIGNAL_PERIOD", 500);
                self->write_parameter ("PARAMETER_PERIOD",  500);
                self->data_shr_max = 14;
                break;
        case 5: self->write_parameter ("SIGNAL_PERIOD",1000);
                self->data_shr_max = 16;
                break;
        case 6: self->write_parameter ("SIGNAL_PERIOD",10000);
                self->data_shr_max = 20;
                break;
        case 7: self->write_parameter ("SIGNAL_PERIOD",50000);
                self->data_shr_max = 24;
                break;
        default: self->write_parameter ("SIGNAL_PERIOD", 200); break;
        }
        if (self->data_shr_max > 0){
                self->write_parameter ("SHR_DEC_DATA", self->data_shr_max);
                self->write_parameter ("TRANSPORT_DECIMATION", 1<<self->data_shr_max);
        } else {
                self->write_parameter ("SHR_DEC_DATA", self->data_shr_max);
                self->write_parameter ("TRANSPORT_DECIMATION", 1<<self->data_shr_max);
        }
}

void Inet_Json_External_Scandata::choice_transport_ch12_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->channel_selections[0] = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
        self->channel_selections[1] = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
        if (gtk_combo_box_get_active (GTK_COMBO_BOX (widget)) > 0)
                self->write_parameter ("TRANSPORT_MODE", self->transport=gtk_combo_box_get_active (GTK_COMBO_BOX (widget))-1);
}

void Inet_Json_External_Scandata::choice_auto_set_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        int m=gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
        if (m>=0 && m<5)
                self->gain_scale[m] = -1.; // recalculate
        else {
                m -= 5;
                double s[] = { 0.01, 1.0, 10.0 };
                if (m < 3) 
                        for (int i=0; i<5; ++i)
                                self->gain_scale[i] = s[m]; // Fixed
        }
}

void Inet_Json_External_Scandata::choice_transport_ch3_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->channel_selections[2] = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
        if (gtk_combo_box_get_active (GTK_COMBO_BOX (widget)) > 0)
                self->write_parameter ("TRANSPORT_CH3", gtk_combo_box_get_active (GTK_COMBO_BOX (widget))-1);
}

void Inet_Json_External_Scandata::choice_transport_ch4_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->channel_selections[3] = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
        if (gtk_combo_box_get_active (GTK_COMBO_BOX (widget)) > 0)
                self->write_parameter ("TRANSPORT_CH4", gtk_combo_box_get_active (GTK_COMBO_BOX (widget))-1);
}

void Inet_Json_External_Scandata::choice_transport_ch5_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->channel_selections[4] = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
        if (gtk_combo_box_get_active (GTK_COMBO_BOX (widget)) > 0)
                self->write_parameter ("TRANSPORT_CH5", gtk_combo_box_get_active (GTK_COMBO_BOX (widget))-1);
}

void Inet_Json_External_Scandata::set_ss_auto_trigger (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->write_parameter ("SET_SINGLESHOT_TRANSPORT_TRIGGER", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
}

void Inet_Json_External_Scandata::amplitude_gain_changed (Param_Control* pcs, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->parameters.amplitude_fb_cp = self->parameters.amplitude_fb_invert * pow (10., self->parameters.amplitude_fb_cp_db/20.);
        self->parameters.amplitude_fb_ci = self->parameters.amplitude_fb_invert * pow (10., self->parameters.amplitude_fb_ci_db/20.);
        self->write_parameter ("AMPLITUDE_FB_CP", self->parameters.amplitude_fb_cp);
        self->write_parameter ("AMPLITUDE_FB_CI", self->parameters.amplitude_fb_ci);
}

void Inet_Json_External_Scandata::amplitude_controller_invert (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->parameters.amplitude_fb_invert = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? -1.:1.;
        self->amplitude_gain_changed (NULL, self);
}

void Inet_Json_External_Scandata::amplitude_controller (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->write_parameter ("AMPLITUDE_CONTROLLER", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
        self->parameters.amplitude_controller = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void Inet_Json_External_Scandata::phase_gain_changed (Param_Control* pcs, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->parameters.phase_fb_cp = self->parameters.phase_fb_invert * pow (10., self->parameters.phase_fb_cp_db/20.);
        self->parameters.phase_fb_ci = self->parameters.phase_fb_invert * pow (10., self->parameters.phase_fb_ci_db/20.);
        self->write_parameter ("PHASE_FB_CP", self->parameters.phase_fb_cp);
        self->write_parameter ("PHASE_FB_CI", self->parameters.phase_fb_ci);
}

void Inet_Json_External_Scandata::phase_controller_invert (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->parameters.phase_fb_invert = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? -1.:1.;
        self->phase_gain_changed (NULL, self);
}

void Inet_Json_External_Scandata::phase_controller (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->write_parameter ("PHASE_CONTROLLER", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
        self->parameters.phase_controller = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void Inet_Json_External_Scandata::phase_unwrapping_always (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->write_parameter ("PHASE_UNWRAPPING_ALWAYS", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
        self->parameters.phase_unwrapping_always = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void Inet_Json_External_Scandata::update(){
	if (G_IS_OBJECT (window))
		g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (window), "INETJSONSCANCONTROL_EC_list"),
				(GFunc) App::update_ec, NULL);
}

void Inet_Json_External_Scandata::update_monitoring_parameters(){

        // mirror global parameters to private
        parameters.dc_offset = pacpll_parameters.dc_offset;
        parameters.exec_amplitude_monitor = pacpll_parameters.exec_amplitude_monitor;
        parameters.dds_frequency_monitor = pacpll_parameters.dds_frequency_monitor;
        parameters.volume_monitor = pacpll_parameters.volume_monitor;
        parameters.phase_monitor = pacpll_parameters.phase_monitor;
        parameters.cpu_load = pacpll_parameters.cpu_load;
        parameters.free_ram = pacpll_parameters.free_ram;
        parameters.counter = pacpll_parameters.counter;

        gchar *delta_freq_info = g_strdup_printf ("[%g]", parameters.dds_frequency_monitor - parameters.frequency_center);
        input_ddsfreq->set_info (delta_freq_info);
        g_free (delta_freq_info);
        if (G_IS_OBJECT (window))
		g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (window), "PAC_EC_READINGS_list"),
				(GFunc) App::update_ec, NULL);
}

void Inet_Json_External_Scandata::enable_scope (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->run_scope = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void Inet_Json_External_Scandata::dbg_l1 (GtkWidget *widget, Inet_Json_External_Scandata *self){
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
                self->debug_level |= 1;
        else
                self->debug_level &= ~1;
}
void Inet_Json_External_Scandata::dbg_l2 (GtkWidget *widget, Inet_Json_External_Scandata *self){
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
                self->debug_level |= 2;
        else
                self->debug_level &= ~2;
}
void Inet_Json_External_Scandata::dbg_l4 (GtkWidget *widget, Inet_Json_External_Scandata *self){
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
                self->debug_level |= 4;
        else
                self->debug_level &= ~4;
}


void Inet_Json_External_Scandata::scope_ac_ch1_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->scope_ac[0] = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}
void Inet_Json_External_Scandata::scope_ac_ch2_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->scope_ac[1] = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}
void Inet_Json_External_Scandata::scope_ac_ch3_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->scope_ac[2] = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}


void Inet_Json_External_Scandata::connect_cb (GtkWidget *widget, Inet_Json_External_Scandata *self){
        if (!self->text_status) return;
        if (!self->input_rpaddress) return;
        self->debug_log (gtk_entry_get_text (GTK_ENTRY (self->input_rpaddress)));

        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))){
                self->status_append ("Connecting to RedPitaya...\n");

                self->update_health ("Connecting...");

                // new soup session
                self->session = soup_session_new ();

                // request to fire up RedPitaya PACPLLL NGNIX server
                gchar *urlstart = g_strdup_printf ("http://%s/bazaar?start=pacpll", gtk_entry_get_text (GTK_ENTRY (self->input_rpaddress)));
                self->status_append ("1. Requesting NGNIX RedPitaya PACPLL Server Startup:\n");
                self->status_append (urlstart);
                self->status_append ("\n ");
                self->msg = soup_message_new ("GET", urlstart);
                g_free (urlstart);
                GInputStream *istream = soup_session_send (self->session, self->msg, NULL, &self->error);

                if (self->error != NULL) {
                        g_warning (self->error->message);
                        self->status_append (self->error->message);
                        self->status_append ("\n ");
                        self->update_health (self->error->message);
                        return;
                } else {
                        gchar *buffer = g_new0 (gchar, 100);
                        gssize num = g_input_stream_read (istream,
                                                          (void *)buffer,
                                                          100,
                                                          NULL,
                                                          &self->error);   
                        if (self->error != NULL) {
                                self->update_health (self->error->message);
                                g_warning (self->error->message);
                                self->status_append (self->error->message);
                                self->status_append ("\n ");
                                g_free (buffer);
                                return;
                        } else {
                                self->status_append ("Response: ");
                                self->status_append (buffer);
                                self->status_append ("\n ");
                                self->update_health (buffer);
                        }
                        g_free (buffer);
                }

                // then connect to NGNIX WebSocket on RP
                self->status_append ("2. Connecting to NGNIX RedPitaya PACPLL WebSocket...\n");
                gchar *url = g_strdup_printf ("ws://%s:%u", gtk_entry_get_text (GTK_ENTRY (self->input_rpaddress)), self->port);
                self->status_append (url);
                self->status_append ("\n");
                // g_message ("Connecting to: %s", url);
                
                self->msg = soup_message_new ("GET", url);
                g_free (url);
                // g_message ("soup_message_new - OK");
                soup_session_websocket_connect_async (self->session, self->msg, // SoupSession *session, SoupMessage *msg,
                                                      NULL, NULL, // const char *origin, char **protocols,
                                                      NULL, Inet_Json_External_Scandata::got_client_connection, self); // GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data
                // g_message ("soup_session_websocket_connect_async - OK");
        } else {
                // tear down connection
                self->status_append ("Dissconnecting...\n ");
                self->update_health ("Dissconnected");

                //g_clear_object (&self->listener);
                g_clear_object (&self->client);
                g_clear_error (&self->client_error);
                g_clear_error (&self->error);
        }
}

void Inet_Json_External_Scandata::got_client_connection (GObject *object, GAsyncResult *result, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        g_message ("got_client_connection");

	self->client = soup_session_websocket_connect_finish (SOUP_SESSION (object), result, &self->client_error);
        if (self->client_error != NULL) {
                self->status_append ("Connection failed: ");
                self->status_append (self->client_error->message);
                self->status_append ("\n");
                g_message (self->client_error->message);
        } else {
                self->status_append ("RedPitaya WebSocket Connected!\n ");
		g_signal_connect(self->client, "closed",  G_CALLBACK(Inet_Json_External_Scandata::on_closed),  self);
		g_signal_connect(self->client, "message", G_CALLBACK(Inet_Json_External_Scandata::on_message), self);
		//g_signal_connect(connection, "closing", G_CALLBACK(on_closing_send_message), message);
                self->send_all_parameters ();
        }
}

void Inet_Json_External_Scandata::on_message(SoupWebsocketConnection *ws,
                                             SoupWebsocketDataType type,
                                             GBytes *message,
                                             gpointer user_data)
{
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
	gconstpointer contents;
	gsize len;
        gchar *tmp;
        
        self->debug_log ("WebSocket message received.");
        
	if (type == SOUP_WEBSOCKET_DATA_TEXT) {
		contents = g_bytes_get_data (message, &len);
		self->status_append ("WEBSOCKET_DATA_TEXT\n");
		self->status_append ((gchar*)contents);
		self->status_append ("\n");
                g_message ((gchar*)contents);
	} else if (type == SOUP_WEBSOCKET_DATA_BINARY) {
		contents = g_bytes_get_data (message, &len);

                tmp = g_strdup_printf ("WEBSOCKET_DATA_BINARY NGNIX JSON ZBytes: %d", len);
                self->debug_log (tmp);
                g_free (tmp);

#if 0
                // dump to file
                FILE *f;
                f = fopen ("/tmp/gxsm-rp-json.gz","wb");
                fwrite (contents, len, 1, f);
                fclose (f);
                // -----------
                //$ pzahl@phenom:~$ zcat /tmp/gxsm-rp-json.gz 
                //{"parameters":{"DC_OFFSET":{"value":-18.508743,"min":-1000,"max":1000,"access_mode":0,"fpga_update":0},"CPU_LOAD":{"value":5.660378,"min":0,"max":100,"access_mode":0,"fpga_update":0},"COUNTER":{"value":4,"min":0,"max":1000000000000,"access_mode":0,"fpga_update":0}}}pzahl@phenom:~$ 
                //$ pzahl@phenom:~$ file /tmp/gxsm-rp-json.gz 
                // /tmp/gxsm-rp-json.gz: gzip compressed data, max speed, from FAT filesystem (MS-DOS, OS/2, NT)
                // GZIP:  zlib.MAX_WBITS|16
#endif
                self->debug_log ("Uncompressing...");
                gsize size=len*100+1000;
                gchar *json_buffer = g_new0 (gchar, size);

                // inflate buffer into json_buffer
                z_stream zInfo ={0};
                zInfo.total_in  = zInfo.avail_in  = len;
                zInfo.total_out = zInfo.avail_out = size;
                zInfo.next_in  = (Bytef*)contents;
                zInfo.next_out = (Bytef*)json_buffer;
      
                int ret= -1;
                ret = inflateInit2 (&zInfo, MAX_WBITS + 16);
                if ( ret == Z_OK ) {
                        ret = inflate( &zInfo, Z_FINISH );     // zlib function
                        // inflate() returns
                        // Z_OK if some progress has been made (more input processed or more output produced),
                        // Z_STREAM_END if the end of the compressed data has been reached and all uncompressed output has been produced,
                        // Z_NEED_DICT if a preset dictionary is needed at this point,
                        // Z_DATA_ERROR if the input data was corrupted (input stream not conforming to the zlib format or incorrect check value, in which case strm->msg points to a string with a more specific error),
                        // Z_STREAM_ERROR if the stream structure was inconsistent (for example next_in or next_out was Z_NULL, or the state was inadvertently written over by the application),
                        // Z_MEM_ERROR if there was not enough memory,
                        // Z_BUF_ERROR if no progress was possible or if there was not enough room in the output buffer when Z_FINISH is used. Note that Z_BUF_ERROR is not fatal, and inflate() can be called again with more input and more output space to continue decompressing. If
                        // Z_DATA_ERROR is returned, the application may then call inflateSync() to look for a good compression block if a partial recovery of the data is to be attempted. 
                        switch ( ret ){
                        case Z_STREAM_END:
                                tmp = NULL;
                                if (self->debug_level > 2)
                                        tmp = g_strdup_printf ("Z_STREAM_END out = %d, in = %d, ratio=%g\n",zInfo.total_out, zInfo.total_in, (double)zInfo.total_out / (double)zInfo.total_in);
                                break;
                        case Z_OK:
                                tmp = g_strdup_printf ("Z_OK out = %d, in = %d\n",zInfo.total_out, zInfo.total_in); break;
                        case Z_NEED_DICT:
                                tmp = g_strdup_printf ("Z_NEED_DICT out = %d, in = %d\n",zInfo.total_out, zInfo.total_in); break;
                        case Z_DATA_ERROR:
                                self->status_append (zInfo.msg);
                                tmp = g_strdup_printf ("\nZ_DATA_ERROR out = %d, in = %d\n",zInfo.total_out, zInfo.total_in); break; 
                                break;
                        case Z_STREAM_ERROR:
                                tmp = g_strdup_printf ("Z_STREAM_ERROR out = %d\n",zInfo.total_out); break;
                        case Z_MEM_ERROR:
                                tmp = g_strdup_printf ("Z_MEM_ERROR out = %d, in = %d\n",zInfo.total_out, zInfo.total_in); break;
                        case Z_BUF_ERROR:
                                tmp = g_strdup_printf ("Z_BUF_ERROR out = %d, in = %d  ratio=%g\n",zInfo.total_out, zInfo.total_in, (double)zInfo.total_out / (double)zInfo.total_in); break;
                        default:
                                tmp = g_strdup_printf ("ERROR ?? inflate result = %d,  out = %d, in = %d\n",ret,zInfo.total_out, zInfo.total_in); break;
                        }
                        self->status_append (tmp);
                        g_free (tmp);
                }
                inflateEnd( &zInfo );   // zlib function
                if (self->debug_level > 0){
                        self->status_append (json_buffer);
                        self->status_append ("\n");
                }

                self->json_parse_message (json_buffer);

                g_free (json_buffer);
                
                self->update_monitoring_parameters();
                self->update_graph ();
                self->stream_data ();
                self->update_health ();
        }

	g_bytes_unref (message);
}

void Inet_Json_External_Scandata::on_closed (SoupWebsocketConnection *ws, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->status_append ("WebSocket connection externally closed.\n");
}

void Inet_Json_External_Scandata::json_parse_message (const char *json_string){
        jsmn_parser p;
        jsmntok_t tok[10000]; /* We expect no more than 10000 tokens, signal array is 1024 * 5*/

        // typial data messages:
        // {"signals":{"SIGNAL_CH3":{"size":1024,"value":[0,0,...,0.543632,0.550415]},"SIGNAL_CH4":{"size":1024,"value":[0,0,... ,-94.156487]},"SIGNAL_CH5":{"size":1024,"value":[0,0,.. ,-91.376022,-94.156487]}}}
        // {"parameters":{"DC_OFFSET":{"value":-18.591045,"min":-1000,"max":1000,"access_mode":0,"fpga_update":0},"COUNTER":{"value":2.4,"min":0,"max":1000000000000,"access_mode":0,"fpga_update":0}}}

        jsmn_init(&p);
        int ret = jsmn_parse(&p, json_string, strlen(json_string), tok, sizeof(tok)/sizeof(tok[0]));
        if (ret < 0) {
                g_warning ("JSON PARSER:  Failed to parse JSON: %d\n%s\n", ret, json_string);
                return;
        }
        /* Assume the top-level element is an object */
        if (ret < 1 || tok[0].type != JSMN_OBJECT) {
                g_warning("JSON PARSER:  Object expected\n");
                return;
        }

#if 0
        json_dump (json_string, tok, p.toknext, 0);
#endif

        json_fetch (json_string, tok, p.toknext, 0);
        if  (debug_level > 1)
                dump_parameters (debug_level);
}



void Inet_Json_External_Scandata::write_parameter (const gchar *paramater_id, double value){
        //soup_websocket_connection_send_text (self->client, "text");
        //soup_websocket_connection_send_binary (self->client, gconstpointer data, gsize length);
        //soup_websocket_connection_send_text (client, "{ \"parameters\":{\"GAIN1\":{\"value\":200.0}}}");

        if (client){
                gchar *json_string = g_strdup_printf ("{ \"parameters\":{\"%s\":{\"value\":%g}}}", paramater_id, value);
                soup_websocket_connection_send_text (client, json_string);
                if  (debug_level > 0)
                        g_print ("%s\n",json_string);
                g_free (json_string);
        }
}

void Inet_Json_External_Scandata::write_parameter (const gchar *paramater_id, int value){
        if (client){
                gchar *json_string = g_strdup_printf ("{ \"parameters\":{\"%s\":{\"value\":%d}}}", paramater_id, value);
                soup_websocket_connection_send_text (client, json_string);
                if  (debug_level > 0)
                        g_print ("%s\n",json_string);
                g_free (json_string);
        }
}

void Inet_Json_External_Scandata::update_health (const gchar *msg){
        if (msg){
                gtk_entry_set_text (GTK_ENTRY (red_pitaya_health), msg);
        } else {
                gchar *health_string = g_strdup_printf ("CPU: %3.0f%% Free: %6.1f MB #: %g", pacpll_parameters.cpu_load, pacpll_parameters.free_ram/1024/1024, pacpll_parameters.counter);
                gtk_entry_set_text (GTK_ENTRY (red_pitaya_health), health_string);
                g_free (health_string);
        }
}

void Inet_Json_External_Scandata::status_append (const gchar *msg){

	GtkTextBuffer *console_buf;
	GtkTextView *textview;
	GString *output;
	GtkTextMark *end_mark;
        GtkTextIter start_iter, end_trim_iter, end_iter;
        gint lines, max_lines=20*debug_level+10;

	if (!msg) {
		if (debug_level > 4)
                        g_warning("No message to append");
		return;
	}

	// read string which contain last command output
	console_buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text_status));
	gtk_text_buffer_get_bounds (console_buf, &start_iter, &end_iter);

	// get output widget content
	output = g_string_new (gtk_text_buffer_get_text (console_buf,
                                                         &start_iter, &end_iter,
                                                         FALSE));

	// append input line
	output = g_string_append (output, msg);
	gtk_text_buffer_set_text (console_buf, output->str, -1);
	g_string_free (output, TRUE);

        gtk_text_buffer_get_start_iter (console_buf, &start_iter);
        lines = gtk_text_buffer_get_line_count (console_buf);
        if (lines > max_lines){
                gtk_text_buffer_get_iter_at_line_index (console_buf, &end_trim_iter, lines-max_lines, 0);
                gtk_text_buffer_delete (console_buf,  &start_iter,  &end_trim_iter);
        }
        
	// scroll to end
	gtk_text_buffer_get_end_iter (console_buf, &end_iter);
	end_mark = gtk_text_buffer_create_mark (console_buf, "cursor", &end_iter,
                                                FALSE);
	g_object_ref (end_mark);
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (text_status),
                                      end_mark, 0.0, FALSE, 0.0, 0.0);
	g_object_unref (end_mark);
}

void Inet_Json_External_Scandata::stream_data (){
        int deci=16;
        int n=4;
        if (data_shr_max > 2) { //if (ch_freq >= 0 || ch_ampl >= 0){
                double decimation = 125e6 * gapp->xsm->hardware->GetScanrate ();
                deci = (gint64)decimation;
                if (deci > 2){
                        for (n=0; deci; ++n) deci >>= 1;
                        --n;
                }
                if (n>data_shr_max) n=data_shr_max; // limit to 24. Note: (32 bits - 8 control, may shorten control, only 3 needed)
                deci = 1<<n;
                //g_print ("Scan Pixel rate is %g s/pix -> Decimation %g -> %d n=%d\n", gapp->xsm->hardware->GetScanrate (), decimation, deci, n);
        }
        if (deci != data_decimation){
                data_decimation = deci;
                data_shr = n;
                write_parameter ("SHR_DEC_DATA", data_shr);
                write_parameter ("TRANSPORT_DECIMATION", data_decimation);
        }

        if (ch_freq >= 0 || ch_ampl >= 0){
                if (x < gapp->xsm->scan[ch_freq]->mem2d->GetNx () &&
                    y < gapp->xsm->scan[ch_freq]->mem2d->GetNy ()){
                        for (int i=0; i<1000; ++i) {
                                if (ch_freq >= 0)
                                        gapp->xsm->scan[ch_freq]->mem2d->PutDataPkt (parameters.freq_fb_lower + pacpll_signals.signal_ch2[i], x,y);
                                if (ch_ampl >= 0)
                                        gapp->xsm->scan[ch_ampl]->mem2d->PutDataPkt (pacpll_signals.signal_ch1[i], x,y);
                                ++x;
                                if (x >= gapp->xsm->scan[ch_freq]->mem2d->GetNx ()) {x=0; ++y; };
                                if (y >= gapp->xsm->scan[ch_freq]->mem2d->GetNy ()) break;
                        }
                        streampos += 1024;
                        gapp->xsm->scan[ch_freq]->draw ();
                }
        }
}

void Inet_Json_External_Scandata::update_graph (){
        int n=1023;
        int h=256;
        if (!run_scope)
                h=2;
        double xs = 0.5;
        double x0 = xs*n/2;
        double yr = h/2;
        double y_hi  = yr*0.95;
        double dB_hi   =  0.0;
        double dB_mags =  4.0;
        cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, n/2, h);
        cairo_t *cr = cairo_create (surface);
        if (run_scope){
                cairo_translate (cr, 0., yr);
                cairo_scale (cr, 1., 1.);
                cairo_save (cr);
                cairo_item_rectangle *paper = new cairo_item_rectangle (0., -128., 512., 128.);
                paper->set_line_width (0.2);
                paper->set_stroke_rgba (CAIRO_COLOR_GREY1);
                paper->set_fill_rgba (CAIRO_COLOR_BLACK);
                paper->draw (cr);
                delete paper;
                //cairo_item_segments *grid = new cairo_item_segments (44);
                
                double avg=0.;
                double avg10=0.;
                char *valuestring;
                cairo_item_text *reading = new cairo_item_text ();
                reading->set_font_face_size ("Ununtu", 10.);
                reading->set_anchor (CAIRO_ANCHOR_W);
                cairo_item_path *wave = new cairo_item_path (n);
                wave->set_line_width (1.0);
                const gchar *ch_id[] = { "Ch1", "Ch2", "Ch3", "Ch4", "Ch5", "Phase", "Ampl" };
                CAIRO_BASIC_COLOR_IDS color[] = { CAIRO_COLOR_RED_ID, CAIRO_COLOR_GREEN_ID, CAIRO_COLOR_CYAN_ID, CAIRO_COLOR_YELLOW_ID, CAIRO_COLOR_BLUE_ID, CAIRO_COLOR_RED_ID, CAIRO_COLOR_GREEN_ID  };
                double *signal[] = { pacpll_signals.signal_ch1, pacpll_signals.signal_ch2, pacpll_signals.signal_ch3, pacpll_signals.signal_ch4, pacpll_signals.signal_ch5, // 0...4 CH1..5
                                     pacpll_signals.signal_phase, pacpll_signals.signal_ampl  }; // 5,6 PHASE, AMPL in Tune Mode, averaged from burst
                double x,xf,min,max,s,ydb,yph;
                if (operation_mode >= 6 && operation_mode <= 8)
                        operation_mode = 6; // TUNE
                int ch_last=(operation_mode == 6) ? 7 : 5;
                for (int ch=0; ch<ch_last; ++ch){
                        int part_i0=0;
                        int part_pos=1;

                        if (operation_mode == 6 && (ch == 0 || ch == 1))
                                if (ch == 0)
                                        wave->set_stroke_rgba (1.,0.,0.,0.4);
                                else
                                        wave->set_stroke_rgba (0.,1.,0.,0.4);
                        else
                                wave->set_stroke_rgba (BasicColors[color[ch]]);
                        min=max=signal[ch][512];
                        for (int k=0; k<n; ++k){
                                s=signal[ch][k];
                                if (s>max) max=s;
                                if (s<min) min=s;
                                if (ch<5)
                                        if (scope_ac[ch])
                                                s -= scope_dc_level[ch];
                                
                                xf = 250.+480.*pacpll_signals.signal_frq[k]/parameters.tune_span; // tune plot, freq x
                                x  = xs*k; // time plot
                                x = (operation_mode == 6 && ch > 1) ? xf : x;
                                
                                if (operation_mode == 6 && (ch == 6 || ch == 1)){
                                        // 0..-60dB range, 1mV:-60dB (center), 0dB:1000mV (top)
                                        wave->set_xy_fast (k, ch == 1 ? x:xf, ydb=db_to_y (dB_from_mV (s), dB_hi, y_hi, dB_mags));

                                } else if (operation_mode == 6 && ((ch == 5 || ch == 0) // phase data always scale +/-180deg
                                                                   || ( ch == 2 && (channel_selections[2]==2 || channel_selections[2]==6))
                                                                   || ( ch == 3 && (channel_selections[3]==2 || channel_selections[3]==6))
                                                                   || ( ch == 4 && (channel_selections[4]==2 || channel_selections[4]==6)))
                                           ){
                                        // -180..180 deg
                                        wave->set_xy_fast (k, ch == 0 ? x:xf, yph=deg_to_y (s, y_hi));
                                } else{
                                        if (ch > 1 && (channel_selections[ch]==2 || channel_selections[ch]==6)) // Phase
                                                wave->set_xy_fast (k, x, deg_to_y (s, y_hi));
                                        else if ((   ch == 0 && channel_selections[0]==2) // Phase
                                                 || (ch == 1 && channel_selections[0]==4)
                                                 || (ch == 0 && channel_selections[0]==7)
                                                 )
                                                wave->set_xy_fast (k, x, deg_to_y (s, y_hi));
                                        
                                        else if ((   ch >  1 && (channel_selections[ch]==1 || channel_selections[ch]==5 || channel_selections[ch]==9)) // Amplitude
                                                 || (ch == 1 && channel_selections[0]==2) // Amplitude
                                                 || (ch == 1 && channel_selections[0]==3)
                                                 || (ch == 0 && channel_selections[0]==6)
                                                 )
                                                wave->set_xy_fast (k, x, db_to_y (dB_from_mV (s), dB_hi, y_hi, dB_mags));
                                        
                                        else if ((   ch == 0 && (channel_selections[0]==5)) // || channel_selections[0]==1)){ // Exec Amplitude
                                                 || (ch == 1 && (channel_selections[0]==6))
                                                 ){
                                                wave->set_xy_fast (k, x, db_to_y (dB_from_mV (s), dB_hi, y_hi, dB_mags));
                                                if (k>2 && s<0. && part_pos){
                                                        wave->set_stroke_rgba (CAIRO_COLOR_MAGENTA_ID);
                                                        wave->draw_partial (cr, part_i0, k-1); part_i0=k; part_pos=0;
                                                }
                                                if (k>2 && s>0. && !part_pos){
                                                        wave->set_stroke_rgba (BasicColors[color[ch]]);
                                                        wave->draw_partial (cr, part_i0, k-1); part_i0=k; part_pos=1;
                                                }
                                        }
                                        else 
                                                wave->set_xy_fast (k, x,-yr*(gain_scale[ch]>0.?gain_scale[ch]:1.)*s);
                                }
                        }
                        if (s>0. && part_i0>0){
                                wave->set_stroke_rgba (BasicColors[color[ch]]);
                                wave->draw_partial (cr, part_i0, n-2);
                        }
                        if (s<0. && part_i0>0){
                                wave->set_stroke_rgba (CAIRO_COLOR_MAGENTA_ID);
                                wave->draw_partial (cr, part_i0, n-2);
                        }
                        if (channel_selections[ch] && part_i0==0)
                                wave->draw (cr);

                        if (ch<6){
                                scope_dc_level[ch] = 0.5*(min+max);

                                if (gain_scale[ch] < 0.)
                                        if (scope_ac[ch]){
                                                min -= scope_dc_level[ch];
                                                max -= scope_dc_level[ch];
                                                gain_scale[ch] = 0.7 / (0.0001 + (fabs(max) > fabs(min) ? fabs(max) : fabs(min)));
                                        } else
                                                gain_scale[ch] = 0.7 / (0.0001 + (fabs(max) > fabs(min) ? fabs(max) : fabs(min)));
                        }
                                           
                        if (operation_mode != 6 && channel_selections[ch]){
                                avg=avg10=0.;
                                for (int i=1023-100; i<1023; ++i) avg+=signal[ch][i]; avg/=100.;
                                for (int i=1023-10; i<1023; ++i) avg10+=signal[ch][i]; avg10/=10.;
                                valuestring = g_strdup_printf ("%s %g %12.5f [x %g] %g %g {%g}", ch_id[ch], avg10, avg, gain_scale[ch], min, max, max-min);
                                reading->set_stroke_rgba (BasicColors[color[ch]]);
                                reading->set_text (10, -(110-14*ch), valuestring);
                                g_free (valuestring);
                                reading->draw (cr);
                        }
                        
                }
                if (pacpll_parameters.bram_write_pos >= 0 && pacpll_parameters.bram_write_pos <= 1024){
                        cairo_item_segments *cursors = new cairo_item_segments (2);
                        cursors->set_line_width (0.5);
                        cursors->set_stroke_rgba (CAIRO_COLOR_WHITE);
                        cursors->set_xy_fast (0,pacpll_parameters.bram_write_pos,-80.);
                        cursors->set_xy_fast (1,pacpll_parameters.bram_write_pos,80.);
                        cursors->draw (cr);
                        g_free (cursors);
                }

                if (operation_mode < 6){
                        valuestring = g_strdup_printf ("Dec=%d [>>%d] wp#{%d,%d}", data_decimation, data_shr, pacpll_parameters.bram_write_pos, pacpll_parameters.bram_dec_count);
                        reading->set_stroke_rgba (CAIRO_COLOR_WHITE);
                        reading->set_text (10, -(110-14*6), valuestring);
                        g_free (valuestring);
                        reading->draw (cr);
                }

                // tick marks dB
                for (int db=(int)dB_hi; db >= -20*dB_mags; db -= 10){
                        valuestring = g_strdup_printf ("%4d dB", db);
                        reading->set_stroke_rgba (CAIRO_COLOR_GREEN);
                        reading->set_text (440,  db_to_y ((double)db, dB_hi, y_hi, dB_mags), valuestring);
                        g_free (valuestring);
                        reading->draw (cr);
                }
                // ticks deg
                for (int deg=-180*deg_extend; deg <= 180*deg_extend; deg += 30*deg_extend){
                        valuestring = g_strdup_printf ("%d" UTF8_DEGREE, deg);
                        reading->set_stroke_rgba (CAIRO_COLOR_RED);
                        reading->set_text (480, deg_to_y (deg, y_hi), valuestring);
                        g_free (valuestring);
                        reading->draw (cr);
                }
                cairo_item_segments *cursors = new cairo_item_segments (2);
                cursors->set_line_width (0.5);
                cursors->set_stroke_rgba (CAIRO_COLOR_WHITE);
                // coord cross
                cursors->set_xy_fast (0,250.+480.*(-0.5),0.);
                cursors->set_xy_fast (1,250.+480.*(0.5),0.);
                cursors->draw (cr);
                cursors->set_xy_fast (0,250.,y_hi);
                cursors->set_xy_fast (1,250.,-y_hi);
                cursors->draw (cr);

                // phase setpoint
                cursors->set_stroke_rgba (1.,0.,0.,0.5);
                cursors->set_xy_fast (0,250.+480.*(-0.5), deg_to_y (parameters.phase_fb_setpoint, y_hi));
                cursors->set_xy_fast (1,250.+480.*(0.5), deg_to_y (parameters.phase_fb_setpoint, y_hi));
                cursors->draw (cr);

                // phase setpoint
                cursors->set_stroke_rgba (0.,1.,0.,0.5);
                cursors->set_xy_fast (0,250.+480.*(-0.5), db_to_y (dB_from_mV (parameters.amplitude_fb_setpoint), dB_hi, y_hi, dB_mags));
                cursors->set_xy_fast (1,250.+480.*(0.5), db_to_y (dB_from_mV (parameters.amplitude_fb_setpoint), dB_hi, y_hi, dB_mags));
                cursors->draw (cr);

               
                if (operation_mode == 6){ // tune info
                        if (debug_level > 0)
                                g_print ("Tune: %g Hz,  %g mV,  %g dB, %g deg\n",
                                         pacpll_signals.signal_frq[n-1],
                                         s,
                                         -20.*log (fabs(s)),
                                         pacpll_signals.signal_ch1[n-1]
                                         );

                        // current pos marks
                        cursors->set_stroke_rgba (CAIRO_COLOR_YELLOW);
                        x = 250.+480.*pacpll_signals.signal_frq[n-1]/parameters.tune_span;
                        cursors->set_xy_fast (0,x,ydb-20.);
                        cursors->set_xy_fast (1,x,ydb+20.);
                        cursors->draw (cr);
                        cursors->set_xy_fast (0,x,yph-20);
                        cursors->set_xy_fast (1,x,yph+20);
                        cursors->draw (cr);

                        cursors->set_stroke_rgba (CAIRO_COLOR_GREEN);
                        x = 250.+480.*(pacpll_parameters.center_frequency-pacpll_parameters.frequency_manual)/parameters.tune_span;
                        ydb=-y_hi*(20.*log10 (fabs (pacpll_parameters.center_amplitude)))/60.;
                        cursors->set_xy_fast (0,x,ydb-50.);
                        cursors->set_xy_fast (1,x,ydb+50.);
                        cursors->draw (cr);

                        cursors->set_stroke_rgba (CAIRO_COLOR_RED);
                        x = 250.+480.*(pacpll_parameters.center_frequency-pacpll_parameters.frequency_manual)/parameters.tune_span;
                        ydb=-y_hi*pacpll_parameters.center_phase/180.;
                        cursors->set_xy_fast (0,x-50,ydb);
                        cursors->set_xy_fast (1,x+50,ydb);
                        cursors->draw (cr);

                        g_free (cursors);

                        valuestring = g_strdup_printf ("Phase: %g deg", pacpll_signals.signal_phase[n-1]);
                        reading->set_stroke_rgba (CAIRO_COLOR_RED);
                        reading->set_text (10, -(110), valuestring);
                        g_free (valuestring);
                        reading->draw (cr);
                        valuestring = g_strdup_printf ("Amplitude: %g dB (%g mV)", dB_from_mV (pacpll_signals.signal_ampl[n-1]), pacpll_signals.signal_ampl[n-1] );
                        reading->set_stroke_rgba (CAIRO_COLOR_GREEN);
                        reading->set_text (10, -(110-14), valuestring);
                        g_free (valuestring);
                        reading->draw (cr);
                        
                        valuestring = g_strdup_printf ("Tuning: last peak @ %g mV  %.4f Hz  %g deg",
                                                       pacpll_parameters.center_amplitude,
                                                       pacpll_parameters.center_frequency,
                                                       pacpll_parameters.center_phase
                                                       );
                        reading->set_stroke_rgba (CAIRO_COLOR_WHITE);
                        reading->set_text (10, (110+14*0), valuestring);
                        g_free (valuestring);
                        reading->draw (cr);
                } else if (transport == 0){ // add polar plot for CH1,2 as XY
                        wave->set_stroke_rgba (CAIRO_COLOR_MAGENTA);
                        for (int k=0; k<n; ++k)
                                wave->set_xy_fast (k,n/4-yr*gain_scale[0]*signal[0][k],-yr*gain_scale[1]*signal[1][k]);
                        wave->draw (cr);
                }
                delete wave;
                delete reading;
        } else {
                deg_extend = 1;
        }
        
        cairo_destroy (cr);
        
        gtk_image_set_from_surface (GTK_IMAGE (signal_graph), surface);
        //cairo_surface_finish (surface);
}
