/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Hardware Interface Plugin
 * ==============================
 * hacked form Percy Zahls' demo_hwi* files
 * 
 * Author: Marcello Carla' <carla@fi.infn.it>
 * 
 * Copyright (C) 2008 Percy Zahl, Marcello Carla'
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
 * All "% OptPlugInXXX" tags are optional and can be removed or commented in
 * ---------------------------------------------------------------------------
% BeginPlugInDocuSection
% PlugInDocuCaption: Interface to the spm2 scan device
% PlugInName: kmdsp_hwi
% PlugInAuthor: Marcello Carla'
% PlugInAuthorEmail: carla@fi.infn.it
% PlugInMenuPath: Tools/SR-DSP Control

% PlugInDescription

The spm2 package provides the control of a Scanning Probe Microscope
through one or two common use DAQ boards (http://spm.polosci.unifi.it)
and a software simulated DSP. This module supports the use of the spm2
package from within the GXSM program.

% EndPlugInDocuSection
* ----------------------------------------------------------------------------
*/

#include <sys/ioctl.h>

#include "config.h"
#include "gxsm3/plugin.h"
#include "gxsm3/xsmhard.h"
#include "gxsm3/glbvars.h"

// Define HwI PlugIn reference name here,
// this is what is listed later within "Preferenced Dialog"
// i.e. the string selected for "Hardware/Card"!
#define THIS_HWI_PLUGIN_NAME "kmdsp:SPM"

// Plugin Prototypes
static void kmdsp_hwi_init( void );
static void kmdsp_hwi_about( void );
static void kmdsp_hwi_configure( void );
static void kmdsp_hwi_query( void );
static void kmdsp_hwi_cleanup( void );

// Fill in the GxsmPlugin Description here
GxsmPlugin kmdsp_hwi_pi = {
	NULL,                   // filled in and used by Gxsm, don't touch !
	NULL,                   // filled in and used by Gxsm, don't touch !
	0,                      // filled in and used by Gxsm, don't touch !
	NULL,                   // The Gxsm-App Class Ref.pointer (called
        // "gapp" in Gxsm) is filled in here by Gxsm on Plugin load, 
	// just after init() is called !!!
	// -------------------------------------------------------------------
	// Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
	"kmdsp_hwi-"
	"HW-INT-1S-SHORT",
	// Plugin's Category - used to autodecide on Pluginloading or ignoring
	// In this case of Hardware-Interface-Plugin here is the
        // interface-name required
	// this is the string selected for "Hardware/Card"!
	THIS_HWI_PLUGIN_NAME,
	// Description, is shown by PluginViewer
        // (Plugin: listplugin, Tools->Plugin Details)
	g_strdup ("spm2 hardware interface."),
	// Author(s)
	"Marcello Carla'",
	// Menupath to position where it is appendet to -- not used by HwI PIs
	N_("Hardware/"),
	// Menuentry -- not used by HwI PIs
	N_(THIS_HWI_PLUGIN_NAME"-HwI"),
	// help text shown on menu
	N_("This is the "THIS_HWI_PLUGIN_NAME" - GXSM Hardware Interface."),
	// more info...
	"N/A",
	NULL,          // error msg, plugin may put error status msg here later
	NULL,          // Plugin Status, managed by Gxsm, plugin may
        // manipulate it too
	// init-function pointer, can be "NULL", 
	// called if present at plugin load
	kmdsp_hwi_init,  
	// query-function pointer, can be "NULL", 
	// called if present after plugin init to let plugin manage
        // it install itself
	kmdsp_hwi_query, // query can be used (otherwise set to NULL)
        // to install
	// additional control dialog in the GXSM menu
	// about-function, can be "NULL"
	// can be called by "Plugin Details"
	kmdsp_hwi_about,
	// configure-function, can be "NULL"
	// can be called by "Plugin Details"
	kmdsp_hwi_configure,
	// run-function, can be "NULL", if non-Zero and no query defined, 
	// it is called on menupath->"plugin"
	NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
	// cleanup-function, can be "NULL"
	// called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

	kmdsp_hwi_cleanup
};


// Text used in Aboutbox, please update!!
static const char *about_text = N_("GXSM kmdsp_hwi Plugin\n\nInterface for "
                                   "hardware handled by the spm2 package.");

/* Here we go... */

#include "kmdsp_hwi.h"
#include "kmdsp_hwi_control.h"

/*
 * PI global
 */
// name of the currently in GXSM configured HwI (Hardware/Card)
gchar *kmdsp_hwi_configure_string = NULL;
// instance of the HwI derived XSM_Hardware class
kmdsp_hwi_dev *kmdsp_hwi_hardware = NULL;

gchar *kmdsp_SPM_Control_menupath  = g_strdup("windows-section");
gchar *kmdsp_SPM_Control_menuentry = g_strdup(N_("kmdsp SPM Control"));
gchar *kmdsp_SPM_Control_menuhelp  = g_strdup(N_("open the SPM control window"));

kmdsp_SPM_Control *kmdsp_SPM_ControlClass = NULL;

static void kmdsp_SPM_Control_show_callback ( GtkWidget*, void* );
static void DSPMover_show_callback ( GtkWidget*, void* );

static void kmdsp_SPM_Control_StartScan_callback ( gpointer );

static void kmdsp_SPM_Control_SaveValues_callback ( gpointer );
static void kmdsp_SPM_Control_LoadValues_callback ( gpointer );

/* 
 * PI essential members
 */

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used
// to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){
        kmdsp_hwi_pi.description =
                g_strdup_printf(N_("GXSM HwI kmdsp_hwi plugin %s"), VERSION);
	return &kmdsp_hwi_pi; 
}

// Symbol "get_gxsm_hwi_hardware_class" is resolved by dlsym from
// Gxsm for all HwI type PIs, 
// Essential Plugin Function!!
XSM_Hardware *get_gxsm_hwi_hardware_class ( void *data ) {
	kmdsp_hwi_configure_string = g_strdup ((gchar*)data);
	kmdsp_hwi_hardware = new kmdsp_hwi_spm ();
	return kmdsp_hwi_hardware;
}

// init-Function
static void kmdsp_hwi_init(void)
{
	kmdsp_hwi_hardware = NULL;
}

// about-Function
static void kmdsp_hwi_about(void)
{
	const gchar *authors[] = { kmdsp_hwi_pi.authors, NULL};
	gtk_show_about_dialog (NULL, 
			       "program-name",  kmdsp_hwi_pi.name,
			       "version", VERSION,
			       N_("(C) 2008 Marcello Carla'"),
			       "comments", about_text,
			       "authors", authors,
			       NULL
			       );
}

// configure-Function
static void kmdsp_hwi_configure(void)
{
	if(kmdsp_hwi_pi.app)
		kmdsp_hwi_pi.app->message("kmdsp_hwi Plugin Configuration");
}

// query-Function
static void kmdsp_hwi_query(void)
{
	g_print ("SR-HwI::kmdsp_hwi_query:: <%s>\n",
                 kmdsp_hwi_configure_string);

	static GnomeUIInfo menuinfo[] = { 
		{ GNOME_APP_UI_ITEM, 
		  kmdsp_SPM_Control_menuentry, kmdsp_SPM_Control_menuhelp,
		  (gpointer) kmdsp_SPM_Control_show_callback, NULL,
		  NULL, GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BLANK, 
		  0, GDK_CONTROL_MASK, NULL },

		GNOMEUIINFO_END
	};

	gnome_app_insert_menus
		( GNOME_APP(kmdsp_hwi_pi.app->getApp()), 
		  kmdsp_SPM_Control_menupath, menuinfo );

//	SR DSP Control Window
// ==================================================
	kmdsp_SPM_ControlClass = new kmdsp_SPM_Control;
	kmdsp_hwi_pi.app->ConnectPluginToStartScanEvent(
                kmdsp_SPM_Control_StartScan_callback);

	g_print ("SR-HwI::kmdsp_hwi_query:: ConnectPluginToCDFSaveEvent\n");
	// connect to GXSM nc-fileio
	kmdsp_hwi_pi.app->ConnectPluginToCDFSaveEvent (
                kmdsp_SPM_Control_SaveValues_callback);
	kmdsp_hwi_pi.app->ConnectPluginToCDFLoadEvent (
                kmdsp_SPM_Control_LoadValues_callback);
}

static void kmdsp_SPM_Control_show_callback( GtkWidget* widget, void* data){
	if ( kmdsp_SPM_ControlClass )
		kmdsp_SPM_ControlClass->show();
}

static void kmdsp_SPM_Control_StartScan_callback( gpointer ){
//	g_print ("SR-HwI::kmdsp_SPM_Control_StartScan_callback");
	if ( kmdsp_SPM_ControlClass )
		kmdsp_SPM_ControlClass->update();
}

static void kmdsp_SPM_Control_SaveValues_callback ( gpointer ncf ){
//	g_print ("SR-HwI::SPControl_SaveValues_callback\n");
	if ( kmdsp_SPM_ControlClass )
		kmdsp_SPM_ControlClass->save_values ((NcFile *) ncf);
}

static void kmdsp_SPM_Control_LoadValues_callback ( gpointer ncf ){
//	g_print ("SR-HwI::SPControl_LoadValues_callback\n");
	if ( kmdsp_SPM_ControlClass )
		kmdsp_SPM_ControlClass->load_values ((NcFile *) ncf);
}

// cleanup-Function
static void kmdsp_hwi_cleanup(void)
{
	g_print ("SR-HwI::kmdsp_hwi_cleanup -- Plugin Cleanup, --Menu\n");

	gchar *mp = g_strconcat(kmdsp_SPM_Control_menupath,
                                kmdsp_SPM_Control_menuentry, NULL);
	gnome_app_remove_menus (GNOME_APP( kmdsp_hwi_pi.app->getApp()), mp, 1);
	g_free(mp);

	// delete ...
	g_print ("SR-HwI::kmdsp_hwi_cleanup -- Plugin Cleanup --DSPCoCl\n");
	if( kmdsp_SPM_ControlClass )
		delete kmdsp_SPM_ControlClass ;
	kmdsp_SPM_ControlClass = NULL;

	g_print ("SR-HwI::kmdsp_hwi_cleanup -- Plugin Cleanup --sr_hwi\n");
	if (kmdsp_hwi_hardware)
		delete kmdsp_hwi_hardware;
	kmdsp_hwi_hardware = NULL;

	g_print ("SR-HwI::kmdsp_hwi_cleanup -- Plugin Cleanup --Info\n");
	g_free (kmdsp_hwi_configure_string);
	kmdsp_hwi_configure_string = NULL;

	g_print ("SR-HwI::kmdsp_hwi_cleanup -- Plugin Cleanup done.\n");
}

