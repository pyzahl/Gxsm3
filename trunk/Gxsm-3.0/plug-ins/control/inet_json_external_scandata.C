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
% PlugInMenuPath: windows-sectionInet JSON Scan External Data
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
static void inet_json_external_scandata_show_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data); // show ScanControl Window
static void inet_json_external_scandata_SaveValues_callback ( gpointer );

// Fill in the GxsmPlugin Description here -- see also: Gxsm/src/plugin.h
GxsmPlugin inet_json_external_scandata_pi = {
	NULL,                   // filled in and used by Gxsm, don't touch !
	NULL,                   // filled in and used by Gxsm, don't touch !
	0,                      // filled in and used by Gxsm, don't touch !
	NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
	// filled in here by Gxsm on Plugin load, 
	// just after init() is called !!!
	"Inet_Json_External_Scandata",
	//  "-rhkspmHARD +spmHARD +STM +AFM +SARLS +SNOM",                // PlugIn's Categorie, set to NULL for all, I just don't want this always to be loaded!
	//"+ALL +noHARD +Demo-HwI:SPMHARD +SRanger:SPMHARD +SRangerMK2:SPMHARD +SRangerTest:SPMHARD +Innovative_DSP:SPMHARD +Innovative_DSP:SPAHARD +TC211-CCDHARD +video4linuxHARD +Comedi:SPMHARD +kmdsp:SPMHARD -LAN_RHK:SPMHARD",
	NULL,
  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
	g_strdup ("Inet JSON Scan Source"),
	"Percy Zahl",
	"windows-section", // Menu-path/section
	N_("Inet JSON External Scan Data"), // Menu Entry
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
        GtkWidget *textinput;
	
	GSList *EC_list=NULL;
	GSList **RemoteEntryList = new GSList *;
	*RemoteEntryList = NULL;

	XsmRescourceManager xrm("InetJsonScanControl");

	PI_DEBUG (DBG_L2, "inet_json_external_scandata Plugin : building interface" );

	Unity    = new UnitObj(" "," ");

        // Window Title
	AppWindowInit("Inet JSON External Scan Data Control for RP");

        bp = new BuildParam (v_grid);
        bp->set_no_spin (true);
        //bp->set_default_ec_change_notice_fkt (VObject::ec_properties_changed, this);

        bp->new_grid_with_frame ("Inet Setup");
        textinput = bp->grid_add_input ("RP URL");
        gtk_entry_set_text (GTK_ENTRY (textinput), "http://rp-f05603.local/pacpll/?type=run");

        bp->pop_grid ();
        bp->new_line ();

        bp->show_all ();
 
        // save List away...
	//g_object_set_data( G_OBJECT (window), "INETJSONSCANCONTROL_EC_list", EC_list);

        set_window_geometry ("spm-scan-control");
}

Inet_Json_External_Scandata::~Inet_Json_External_Scandata (){
       	delete Unity;
}

void Inet_Json_External_Scandata::update(){
	if (G_IS_OBJECT (window))
		g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (window), "INETJSONSCANCONTROL_EC_list"),
				(GFunc) App::update_ec, NULL);
}

// Menu Call Back Fkte
#if 0
/* stolen from app_remote.C */
static void via_remote_list_Check_ec(Gtk_EntryControl* ec, remote_args* ra){
	ec->CheckRemoteCmd (ra);
};
#endif
