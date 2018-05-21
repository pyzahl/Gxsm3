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
	
	GSList *EC_list=NULL;
	GSList **RemoteEntryList = new GSList *;
	*RemoteEntryList = NULL;
	GSList *EC_R_list=NULL;

        debug_level = 1;
        input_rpaddress = NULL;
        text_status = NULL;

        ch_freq = -1;
        ch_ampl = -1;

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
	uTime    = new LinUnit(UTF8_MU "s", "us", "Time", 1e-6);

        // Window Title
	AppWindowInit("Inet JSON External Scan Data Control for High Speed RedPitaya PACPLL");

        bp = new BuildParam (v_grid);
        bp->set_no_spin (true);

        bp->set_pcs_remote_prefix ("rp-pacpll-");

        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::parameter_changed, this);
        bp->set_input_width_chars (16);

        bp->new_grid_with_frame ("RedPitaya PAC Setup");
        bp->grid_add_ec ("Reading", Hz, &parameters.dds_frequency_monitor, 0.0, 25e6, "g", 0.1, 1., "DDS-FREQ-MONITOR");
        EC_R_list = g_slist_prepend( EC_R_list, bp->ec);
        bp->ec->Freeze ();
        bp->new_line ();
        bp->grid_add_ec ("Offset", mVolt, &parameters.dc_offset, -1.0, 1.0, "g", 0.1, 1., "DC-OFFSET");
        EC_R_list = g_slist_prepend( EC_R_list, bp->ec);
        bp->ec->Freeze ();
        bp->new_line ();
        parameters.pactau = 0.0002;
        parameters.frequency = 32768.0;
        parameters.volume = 0.1;
  	bp->grid_add_ec ("Tau PAC", Time, &parameters.pactau, 0.0, 63.0, "g", 0.1, 1., "PACTAU");
        bp->new_line ();
        bp->set_no_spin (false);
        bp->set_input_width_chars (10);
  	bp->grid_add_ec ("Frequency", Hz, &parameters.frequency, 0.0, 20e6, "g", 0.1, 100., "FREQUENCY");
        bp->new_line ();
  	bp->grid_add_ec ("Volume", Volt, &parameters.volume, 0.0, 1.0, "g", 0.01, 0.1, "VOLUME");
        bp->new_line ();
        bp->set_no_spin (true);
        bp->set_input_width_chars (16);
        parameters.tune_dfreq = 0.1;
        parameters.tune_span = 50.0;
  	bp->grid_add_ec ("Tune dFreq", Hz, &parameters.tune_dfreq, 1e-4, 1e3, "g", 0.01, 0.1, "TUNE-DFREQ");
        bp->new_line ();
  	bp->grid_add_ec ("Tune Span", Hz, &parameters.tune_span, 0.0, 1e6, "g", 0.1, 10., "TUNE-SPAN");

        bp->pop_grid ();

        bp->new_grid_with_frame ("Amplitude Controller");
        bp->grid_add_ec ("Reading", mVolt, &parameters.volume_monitor, -1.0, 1.0, "g", 0.1, 1., "VOLUME-MONITOR");
        EC_R_list = g_slist_prepend( EC_R_list, bp->ec);
        bp->ec->Freeze ();
        bp->new_line ();
        parameters.amplitude_fb_setpoint = 0.02;
        parameters.amplitude_fb_cp_db = -50.;
        parameters.amplitude_fb_ci_db = -60.;
        parameters.exec_fb_upper = 0.2;
        parameters.exec_fb_lower = -0.1;
        bp->set_no_spin (false);
        bp->set_input_width_chars (8);

        bp->grid_add_ec ("Setpoint", Volt, &parameters.amplitude_fb_setpoint, 0.0, 1.0, "g", 0.1, 1.0, "AMPLITUDE-FB-SETPOINT");
        bp->new_line ();
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::amplitude_gain_changed, this);
        bp->grid_add_ec ("CP gain", dB, &parameters.amplitude_fb_cp_db, -200.0, 200.0, "g", 0.1, 1.0, "AMPLITUDE-FB-CP");
        bp->new_line ();
        bp->grid_add_ec ("CI gain", dB, &parameters.amplitude_fb_ci_db, -200.0, 200.0, "g", 0.1, 1.0, "AMPLITUDE-FB-CI");
        bp->new_line ();
        bp->set_no_spin (true);
        bp->set_input_width_chars (16);
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::parameter_changed, this);
        bp->grid_add_ec ("Upper Limit", Volt, &parameters.exec_fb_upper, 0.0, 1.0, "g", 0.1, 1.0, "EXEC-FB-UPPER");
        bp->new_line ();
        bp->grid_add_ec ("Lower Limit", Volt, &parameters.exec_fb_lower, -1.0, 1.0, "g", 0.1, 1.0, "EXEC-FB-LOWER");
        bp->new_line ();
        bp->grid_add_check_button ( N_("Enable"), "Enable Amplitude Controller", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::amplitude_controller), this);
        bp->grid_add_check_button ( N_("Invert"), "Invert Amplitude Controller Gain", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::amplitude_controller_invert), this);


        bp->pop_grid ();

        bp->new_grid_with_frame ("Phase Controller");
        bp->grid_add_ec ("Reading", Deg, &parameters.phase_monitor, -1.0, 1.0, "g", 0.1, 1., "PHASE-MONITOR");
        EC_R_list = g_slist_prepend( EC_R_list, bp->ec);
        bp->ec->Freeze ();
        bp->new_line ();
        parameters.phase_fb_setpoint = 50.;
        parameters.phase_fb_cp_db = -180.;
        parameters.phase_fb_ci_db = -180.;
        parameters.freq_fb_upper = 33000.;
        parameters.freq_fb_lower = 30000.;
        bp->set_no_spin (false);
        bp->set_input_width_chars (8);
        bp->grid_add_ec ("Setpoint", Deg, &parameters.phase_fb_setpoint, -180.0, 180.0, "g", 0.1, 1.0, "PHASE-FB-SETPOINT");
        bp->new_line ();
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::phase_gain_changed, this);
        bp->grid_add_ec ("CP gain", dB, &parameters.phase_fb_cp_db, -200.0, 200.0, "g", 0.1, 1.0, "PHASE-FB-CP");
        bp->new_line ();
        bp->grid_add_ec ("CI gain", dB, &parameters.phase_fb_ci_db, -200.0, 200.0, "g", 0.1, 1.0, "PHASE-FB-CI");
        bp->new_line ();
        bp->set_no_spin (true);
        bp->set_input_width_chars (16);
        bp->set_default_ec_change_notice_fkt (Inet_Json_External_Scandata::parameter_changed, this);
        bp->grid_add_ec ("Upper Limit", Deg, &parameters.freq_fb_upper, 0.0, 25e6, "g", 0.1, 1.0, "FREQ-FB-UPPER");
        bp->new_line ();
        bp->grid_add_ec ("Lower Limit", Deg, &parameters.freq_fb_lower, 0.0, 25e6, "g", 0.1, 1.0, "FREQ-FB-LOWER");
        bp->new_line ();
        bp->grid_add_check_button ( N_("Enable"), "Enable Phase Controller", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::phase_controller), this);
        bp->grid_add_check_button ( N_("Invert"), "Invert Phase Controller Gain", 1,
                                    G_CALLBACK (Inet_Json_External_Scandata::amplitude_controller_invert), this);

        bp->pop_grid ();

        bp->new_line ();
        bp->new_grid_with_frame ("RedPitaya Web Socket Address for JSON talk", 10);

        bp->set_input_width_chars (25);
        input_rpaddress = bp->grid_add_input ("RedPitaya Address");
        gtk_widget_set_tooltip_text (input_rpaddress, "RedPitaya IP Address like rp-f05603.local or 130.199.123.123");
        //  "ws://rp-f05603.local:9002/"
        //gtk_entry_set_text (GTK_ENTRY (input_rpaddress), "http://rp-f05603.local/pacpll/?type=run");
        //gtk_entry_set_text (GTK_ENTRY (input_rpaddress), "130.199.243.200");
        gtk_entry_set_text (GTK_ENTRY (input_rpaddress), "192.168.1.10");
        
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

        
        bp->pop_grid ();
        bp->new_line ();

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
                "START BRAM TRANSPORT",
                "READ BRAM",
                "TUNE",
                NULL };

        // Init choicelist
	for(int i=0; operation_modes[i]; i++)
                gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), operation_modes[i], operation_modes[i]);

	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 2);

        // BRAM TRANSPORT MODE BLOCK S1,S2
	wid = gtk_combo_box_text_new ();
        g_signal_connect (G_OBJECT (wid), "changed",
                          G_CALLBACK (Inet_Json_External_Scandata::choice_transport_callback),
                          this);
        bp->grid_add_widget (wid);

	const gchar *transport_modes[] = {
                "0: IN1, IN2",
                "1: PHASE, AMPL",
                "2: IN1, AMPL (AC)",
                "3: IN2, PHASE (AC)",
                "4: Exec,Freq",
                "5: FIR Ampl,Phase",
                "6: RP DIGITAL IN 0,1 counts",
                "7: RESET Counts",
                "8: RESET",
                NULL };
   
	// Init choicelist
	for(int i=0; transport_modes[i]; i++)
                gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), transport_modes[i], transport_modes[i]);

	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 0);

        // Scop Auto Set Modes
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
                "Default All=0.1",
                "Default All=1",
                "Default All=10",
                "Manual",
                NULL };
   
	// Init choicelist
	for(int i=0; auto_set_modes[i]; i++)
                gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), auto_set_modes[i], auto_set_modes[i]);

	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 6);

        
        bp->new_line ();

        // GPIO monitor selections -- full set, experimental
	const gchar *monitor_modes_gpio[] = {
                "LMS Amplitude from A,B",
                "LMS Phase from A,B",
                "LMS A",
                "LMS B",
                "FPGA CORDIC Amplitude Monitor",
                "FPGA CORDIC Phase Monitor",
                "FIR passed CORDIC Amplitude Monitor",
                "FIR passed CORDIC Phase Monitor",
                "LMS X",
                "LMS Y",
                "LMS M (LMS Input Signal)",
                "LMS M1 (LMS Input Signal-DC)",
                "Phase from X,Y",
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

	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 1);

        // CH5 from GPIO MONITOR</p>
	wid = gtk_combo_box_text_new ();
        g_signal_connect (G_OBJECT (wid), "changed",
                          G_CALLBACK (Inet_Json_External_Scandata::choice_transport_ch5_callback),
                          this);
        bp->grid_add_widget (wid);

	// Init choicelist
	for(int i=0; monitor_modes_gpio[i]; i++)
                gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), monitor_modes_gpio[i], monitor_modes_gpio[i]);

	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), 5);

                                     
        bp->new_line ();
        signal_graph = gtk_image_new_from_surface (NULL);
        bp->grid_add_widget (signal_graph, 10);
        
        bp->show_all ();
 
        // save List away...
	//g_object_set_data( G_OBJECT (window), "INETJSONSCANCONTROL_EC_list", EC_list);

	g_object_set_data( G_OBJECT (window), "PAC_EC_READINGS_list", EC_R_list);

        set_window_geometry ("inet-json-rp-control"); // needs rescoure entry and defines window menu entry as geometry is managed

        // hookup to scan start and stop
        inet_json_external_scandata_pi.app->ConnectPluginToStartScanEvent (Inet_Json_External_Scandata::scan_start_callback);
        inet_json_external_scandata_pi.app->ConnectPluginToStopScanEvent (Inet_Json_External_Scandata::scan_stop_callback);
}

Inet_Json_External_Scandata::~Inet_Json_External_Scandata (){
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

	PI_DEBUG (DBG_L2, "setup_scan[" << ch << " ]: scantitle done: " << gapp->xsm->scan[ch]->data.ui.type ); 

	g_free (scantitle);
	gapp->xsm->scan[ch]->draw ();

	return 0;
}



void Inet_Json_External_Scandata::parameter_changed (Param_Control* pcs, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;

        self->write_parameter ("PACTAU", self->parameters.pactau);
        self->write_parameter ("FREQUENCY", self->parameters.frequency);
        self->write_parameter ("VOLUME", self->parameters.volume);
        self->write_parameter ("TUNE-DFREQ", self->parameters.tune_dfreq);
        self->write_parameter ("TUNE-SPAN", self->parameters.tune_span);
        self->write_parameter ("AMPLITUDE-FB-SETPOINT", self->parameters.amplitude_fb_setpoint);
        self->write_parameter ("EXEC-FB-UPPER", self->parameters.exec_fb_upper);
        self->write_parameter ("EXEC-FB-LOWER", self->parameters.exec_fb_lower);
        self->write_parameter ("PHASE-FB-SETPOINT", self->parameters.phase_fb_setpoint);
        self->write_parameter ("FREQ-FB-UPPER", self->parameters.freq_fb_upper);
        self->write_parameter ("FREQ-FB-LOWER", self->parameters.freq_fb_lower);
        //self->write_parameter ("", self->parameters.);
}

void Inet_Json_External_Scandata::set_gain_defaults (){
        write_parameter ("GAIN1", 100.);
        write_parameter ("SHR_CH1", 0.);
        write_parameter ("GAIN2", 100.);
        write_parameter ("SHR_CH1", 0.);
        write_parameter ("GAIN3", 100.);
        write_parameter ("GAIN4", 100.);
        write_parameter ("SHR_CH34", 0.);
        write_parameter ("GAIN5", 100.);
        write_parameter ("PACVERBOSE", 0);
        write_parameter ("TRANSPORT_DECIMATION", 2);
        write_parameter ("TRANSPORT_MODE", 0);
        write_parameter ("OPERATION", 2);
}

void Inet_Json_External_Scandata::choice_operation_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->write_parameter ("OPERATION", gtk_combo_box_get_active (GTK_COMBO_BOX (widget)));
}

void Inet_Json_External_Scandata::choice_transport_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->write_parameter ("TRANSPORT_MODE", gtk_combo_box_get_active (GTK_COMBO_BOX (widget)));
}

void Inet_Json_External_Scandata::choice_auto_set_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        int m=gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
        if (m>=0 && m<5)
                self->gain_scale[m] = -1.; // recalculate
        else {
                m -= 5;
                double s[] = { 0.1, 1.0, 10.0 };
                if (m < 3) 
                        for (int i=0; i<5; ++i)
                                self->gain_scale[i] = s[m]; // Fixed
        }
}

void Inet_Json_External_Scandata::choice_transport_ch3_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->write_parameter ("TRANSPORT_CH3", gtk_combo_box_get_active (GTK_COMBO_BOX (widget)));
}

void Inet_Json_External_Scandata::choice_transport_ch4_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->write_parameter ("TRANSPORT_CH4", gtk_combo_box_get_active (GTK_COMBO_BOX (widget)));
}

void Inet_Json_External_Scandata::choice_transport_ch5_callback (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->write_parameter ("TRANSPORT_CH5", gtk_combo_box_get_active (GTK_COMBO_BOX (widget)));
}

void Inet_Json_External_Scandata::amplitude_gain_changed (Param_Control* pcs, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->write_parameter ("AMPLITUDE_FB_CP",
                               self->parameters.amplitude_fb_cp = self->parameters.amplitude_fb_invert
                               * pow (10., self->parameters.amplitude_fb_cp_db/20.));
        self->write_parameter ("AMPLITUDE_FB_CI",
                               self->parameters.amplitude_fb_ci = self->parameters.amplitude_fb_invert
                               * pow (10., self->parameters.amplitude_fb_ci_db/20.));
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
        self->write_parameter ("PHASE_FB_CP",
                               self->parameters.phase_fb_cp = self->parameters.phase_fb_invert
                               * pow (10., self->parameters.phase_fb_cp_db/20.));
        self->write_parameter ("PHASE_FB_CI",
                               self->parameters.phase_fb_ci = self->parameters.phase_fb_invert
                               * pow (10., self->parameters.phase_fb_ci_db/20.));
}

void Inet_Json_External_Scandata::phase_controller_invert (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->parameters.phase_fb_invert = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) ? -1.:1.;
        self->phase_gain_changed (NULL, self);
}

void Inet_Json_External_Scandata::phase_controller (GtkWidget *widget, Inet_Json_External_Scandata *self){
        self->write_parameter ("PHASE_CONTROLLER", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
        self->parameters.phase_controller = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

void Inet_Json_External_Scandata::update(){
	if (G_IS_OBJECT (window))
		g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (window), "INETJSONSCANCONTROL_EC_list"),
				(GFunc) App::update_ec, NULL);
}

void Inet_Json_External_Scandata::update_monitoring_parameters(){

        // mirror global parameters to private
        parameters.dds_frequency_monitor = pacpll_parameters.dds_frequency_monitor;
        parameters.dc_offset = pacpll_parameters.dc_offset;
        parameters.volume_monitor = pacpll_parameters.volume_monitor;
        parameters.phase_monitor = pacpll_parameters.phase_monitor;
        parameters.cpu_load = pacpll_parameters.cpu_load;
        parameters.free_ram = pacpll_parameters.free_ram;
        parameters.counter = pacpll_parameters.counter;

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

void Inet_Json_External_Scandata::connect_cb (GtkWidget *widget, Inet_Json_External_Scandata *self){
        if (!self->text_status) return;
        if (!self->input_rpaddress) return;
        self->debug_log (gtk_entry_get_text (GTK_ENTRY (self->input_rpaddress)));

        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))){
                self->status_append ("Connecting to RedPitaya...\n");

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
                        return;
                } else {
                        gchar *buffer = g_new0 (gchar, 100);
                        gssize num = g_input_stream_read (istream,
                                                          (void *)buffer,
                                                          100,
                                                          NULL,
                                                          &self->error);   
                        if (self->error != NULL) {
                                g_warning (self->error->message);
                                self->status_append (self->error->message);
                                self->status_append ("\n ");
                                g_free (buffer);
                                return;
                        } else {
                                self->status_append ("Response: ");
                                self->status_append (buffer);
                                self->status_append ("\n ");
                        }
                        g_free (buffer);
                }

                // then connect to NGNIX WebSocket on RP
                self->status_append ("2. Connecting to NGNIX RedPitaya PACPLL WebSocket...\n");
                gchar *url = g_strdup_printf ("ws://%s:%u", gtk_entry_get_text (GTK_ENTRY (self->input_rpaddress)), self->port);
                self->status_append (url);
                self->status_append ("\n");
                g_message ("Connecting to: %s", url);
                
                self->msg = soup_message_new ("GET", url);
                g_free (url);
                g_message ("soup_message_new - OK");
                soup_session_websocket_connect_async (self->session, self->msg, // SoupSession *session, SoupMessage *msg,
                                                      NULL, NULL, // const char *origin, char **protocols,
                                                      NULL, Inet_Json_External_Scandata::got_client_connection, self); // GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data
                g_message ("soup_session_websocket_connect_async - OK");
        } else {
                // tear down connection
                self->status_append ("Dissconnecting...\n ");

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
                self->set_gain_defaults ();

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
        }

	g_bytes_unref (message);
}

void Inet_Json_External_Scandata::on_closed (SoupWebsocketConnection *ws, gpointer user_data){
        Inet_Json_External_Scandata *self = (Inet_Json_External_Scandata *)user_data;
        self->status_append ("WebSocket connection externally closed.\n");
}

void Inet_Json_External_Scandata::json_parse_message (const char *json_string){
        jsmn_parser p;
        jsmntok_t tok[10000]; /* We expect no more than 5000 tokens, signal array is 1024 * 5*/

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
                dump_parameters ();
}



void Inet_Json_External_Scandata::write_parameter (const gchar *paramater_id, double value){
        //soup_websocket_connection_send_text (self->client, "text");
        //soup_websocket_connection_send_binary (self->client, gconstpointer data, gsize length);
        //soup_websocket_connection_send_text (client, "{ \"parameters\":{\"GAIN1\":{\"value\":200.0}}}");

        if (client){
                gchar *json_string = g_strdup_printf ("{ \"parameters\":{\"%s\":{\"value\":%12g}}}", paramater_id, value);
                soup_websocket_connection_send_text (client, json_string);
                if  (debug_level > 1)
                        g_print (json_string);
                g_free (json_string);
        }
}

void Inet_Json_External_Scandata::write_parameter (const gchar *paramater_id, int value){
        if (client){
                gchar *json_string = g_strdup_printf ("{ \"parameters\":{\"%s\":{\"value\":%d}}}", paramater_id, value);
                soup_websocket_connection_send_text (client, json_string);
                if  (debug_level > 1)
                        g_print (json_string);
                g_free (json_string);
        }
}


void Inet_Json_External_Scandata::status_append (const gchar *msg){

	GtkTextBuffer *console_buf;
	GtkTextView *textview;
	GString *output;
	GtkTextMark *end_mark;
        GtkTextIter start_iter, end_trim_iter, end_iter;
        gint lines, max_lines=20*debug_level;

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
        if (ch_freq >= 0){
        }
        if (ch_ampl >= 0){
        }
}

void Inet_Json_External_Scandata::update_graph (){
        int n=1024;
        int h=256;
        if (!run_scope)
                h=2;
        double xs = 0.5;
        double yr = h/2;
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

                double avg=0.;
                char *valuestring;
                cairo_item_text *reading = new cairo_item_text ();
                reading->set_font_face_size ("Ununtu", 12.);
                reading->set_anchor (CAIRO_ANCHOR_W);
                cairo_item_path *wave = new cairo_item_path (n);
                wave->set_line_width (1.0);
                CAIRO_BASIC_COLOR_IDS color[] = { CAIRO_COLOR_RED_ID, CAIRO_COLOR_GREEN_ID, CAIRO_COLOR_CYAN_ID, CAIRO_COLOR_YELLOW_ID, CAIRO_COLOR_BLUE_ID };
                double *signal[] = { pacpll_signals.signal_ch1, pacpll_signals.signal_ch2, pacpll_signals.signal_ch3, pacpll_signals.signal_ch4, pacpll_signals.signal_ch5 };
                double min,max,s;
                for (int ch=0; ch<5; ++ch){
                        wave->set_stroke_rgba (BasicColors[color[ch]]);
                        min=max=signal[ch][512];
                        for (int k=0; k<n; ++k){
                                s=signal[ch][k];
                                if (s>max) max=s;
                                if (s<min) min=s;
                                wave->set_xy_fast (k,xs*k,-yr*(gain_scale[ch]>0.?gain_scale[ch]:1.)*s/100.);
                        }
                        if (gain_scale[ch] < 0.)
                                gain_scale[ch] = 70. / (0.0001 + (fabs(max) > fabs(min) ? fabs(max) : fabs(min)));
                        wave->draw (cr);

                        for (int i=1023-100; i<1023; ++i) avg+=signal[ch][i]; avg /= 100.;
                        valuestring = g_strdup_printf ("Ch%d %12.5f [x %g]", ch+1, avg/100., gain_scale[ch]);
                        reading->set_stroke_rgba (BasicColors[color[ch]]);
                        reading->set_text (10, -(110-14*ch), valuestring);
                        g_free (valuestring);
                        reading->draw (cr);
                }
                delete wave;
                delete reading;
        }
        cairo_destroy (cr);
        
        gtk_image_set_from_surface (GTK_IMAGE (signal_graph), surface);
        //cairo_surface_finish (surface);
}
