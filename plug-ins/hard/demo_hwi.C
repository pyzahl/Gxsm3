/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Hardware Interface Plugin Name: demo_hwi.C
 * ===============================================
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
 * All "% OptPlugInXXX" tags are optional and can be removed or commented in
 * --------------------------------------------------------------------------------
% BeginPlugInDocuSection
% PlugInDocuCaption: Demo Hardware Interface
% PlugInName: demo_hwi
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: Tools/SR-DSP Control

% PlugInDescription
This provides a demonstartion hardware interface (HwI)
for GXSM.  It contains all hardware close and specific settings and
controls for feedback, scanning, all kind of probing (spectroscopy and
manipulations).

All functions are simulated.

% EndPlugInDocuSection
* -------------------------------------------------------------------------------- 
*/

#include <sys/ioctl.h>

#include "config.h"
#include "gxsm/plugin.h"
#include "gxsm/xsmhard.h"
#include "gxsm/glbvars.h"

// Define HwI PlugIn reference name here, this is what is listed later within "Preferenced Dialog"
// i.e. the string selected for "Hardware/Card"!
#define THIS_HWI_PLUGIN_NAME "Demo-HwI:SPM"

// Plugin Prototypes
static void demo_hwi_init( void );
static void demo_hwi_about( void );
static void demo_hwi_configure( void );
static void demo_hwi_query( void );
static void demo_hwi_cleanup( void );

// Fill in the GxsmPlugin Description here
GxsmPlugin demo_hwi_pi = {
	NULL,                   // filled in and used by Gxsm, don't touch !
	NULL,                   // filled in and used by Gxsm, don't touch !
	0,                      // filled in and used by Gxsm, don't touch !
	NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
	// filled in here by Gxsm on Plugin load, 
	// just after init() is called !!!
	// ----------------------------------------------------------------------
	// Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
	"demo_hwi-"
	"HW-INT-1S-SHORT",
	// Plugin's Category - used to autodecide on Pluginloading or ignoring
	// In this case of Hardware-Interface-Plugin here is the interface-name required
	// this is the string selected for "Hardware/Card"!
	THIS_HWI_PLUGIN_NAME,
	// Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
	g_strdup ("Demo hardware interface."),
	// Author(s)
	"Percy Zahl",
	// Menupath to position where it is appendet to -- not used by HwI PIs
	N_("Hardware/"),
	// Menuentry -- not used by HwI PIs
	N_(THIS_HWI_PLUGIN_NAME"-HwI"),
	// help text shown on menu
	N_("This is the "THIS_HWI_PLUGIN_NAME" - GXSM Hardware Interface."),
	// more info...
	"N/A",
	NULL,          // error msg, plugin may put error status msg here later
	NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
	// init-function pointer, can be "NULL", 
	// called if present at plugin load
	demo_hwi_init,  
	// query-function pointer, can be "NULL", 
	// called if present after plugin init to let plugin manage it install itself
	demo_hwi_query, // query can be used (otherwise set to NULL) to install
	// additional control dialog in the GXSM menu
	// about-function, can be "NULL"
	// can be called by "Plugin Details"
	demo_hwi_about,
	// configure-function, can be "NULL"
	// can be called by "Plugin Details"
	demo_hwi_configure,
	// run-function, can be "NULL", if non-Zero and no query defined, 
	// it is called on menupath->"plugin"
	NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
	// cleanup-function, can be "NULL"
	// called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

	demo_hwi_cleanup
};


// Text used in Aboutbox, please update!!
static const char *about_text = N_("GXSM demo_hwi Plugin\n\n"
                                   "Demonstration Hardware Interface for SPM.");

/* Here we go... */

#include "demo_hwi.h"
#include "demo_hwi_control.h"

/*
 * PI global
 */

// #define PI_DEBUG(L, DBGTXT) std::cout << "** (" << __FILE__ << ": " << __FUNCTION__ << ") Gxsm-PI-DEBUG-MESSAGE **: " << std::endl << " - " << DBGTXT << std::endl

gchar *demo_hwi_configure_string = NULL;   // name of the currently in GXSM configured HwI (Hardware/Card)
demo_hwi_dev *demo_hwi_hardware = NULL; // instance of the HwI derived XSM_Hardware class

const gchar *Demo_SPM_Control_menupath  = "windows-section";
const gchar *Demo_SPM_Control_menuentry = N_("Demo SPM Control");
const gchar *Demo_SPM_Control_menuhelp  = N_("open the SPM control window");

Demo_SPM_Control *Demo_SPM_ControlClass = NULL;

static void Demo_SPM_Control_show_callback ( GtkWidget*, void* );
static void DSPMover_show_callback ( GtkWidget*, void* );

static void Demo_SPM_Control_StartScan_callback ( gpointer );

static void Demo_SPM_Control_SaveValues_callback ( gpointer );
static void Demo_SPM_Control_LoadValues_callback ( gpointer );



/* 
 * PI essential members
 */

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
	demo_hwi_pi.description = g_strdup_printf(N_("GXSM HwI demo_hwi plugin %s"), VERSION);
	return &demo_hwi_pi; 
}

// Symbol "get_gxsm_hwi_hardware_class" is resolved by dlsym from Gxsm for all HwI type PIs, 
// Essential Plugin Function!!
XSM_Hardware *get_gxsm_hwi_hardware_class ( void *data ) {
	PI_DEBUG (DBG_L2, "demo_hwi HardwareInterface Init");
	demo_hwi_configure_string = g_strdup ((gchar*)data);
	demo_hwi_hardware = new demo_hwi_spm ();
	return demo_hwi_hardware;
}

// init-Function
static void demo_hwi_init(void)
{
	PI_DEBUG (DBG_L2, "demo_hwi Plugin Init");
	demo_hwi_hardware = NULL;
}

// about-Function
static void demo_hwi_about(void)
{
	const gchar *authors[] = { demo_hwi_pi.authors, NULL};
	gtk_show_about_dialog (NULL, "program-name",  demo_hwi_pi.name,
					"version", VERSION,
					  "license", GTK_LICENSE_GPL_3_0,
					  "comments", about_text,
					  "authors", authors,
					  NULL
				));
}

// configure-Function
static void demo_hwi_configure(void)
{
	PI_DEBUG (DBG_L2, "demo_hwi Plugin HwI-Configure");
	if(demo_hwi_pi.app)
		demo_hwi_pi.app->message("demo_hwi Plugin Configuration");
}

// query-Function
static void demo_hwi_query(void)
{
	g_print ("SR-HwI::demo_hwi_query:: <%s>\n",demo_hwi_configure_string);
	PI_DEBUG (DBG_L2, "demo_hwi Plugin Query: " << demo_hwi_configure_string);


	static GnomeUIInfo menuinfo[] = { 
		{ GNOME_APP_UI_ITEM, 
		  Demo_SPM_Control_menuentry, Demo_SPM_Control_menuhelp,
		  (gpointer) Demo_SPM_Control_show_callback, NULL,
		  NULL, GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BLANK, 
		  0, GDK_CONTROL_MASK, NULL },

		GNOMEUIINFO_END
	};

	gnome_app_insert_menus
		( GNOME_APP(demo_hwi_pi.app->getApp()), 
		  Demo_SPM_Control_menupath, menuinfo );

//	SR DSP Control Window
// ==================================================
	Demo_SPM_ControlClass = new Demo_SPM_Control;
	demo_hwi_pi.app->ConnectPluginToStartScanEvent (Demo_SPM_Control_StartScan_callback);

	g_print ("SR-HwI::demo_hwi_query:: ConnectPluginToCDFSaveEvent\n");
	// connect to GXSM nc-fileio
	demo_hwi_pi.app->ConnectPluginToCDFSaveEvent (Demo_SPM_Control_SaveValues_callback);
	demo_hwi_pi.app->ConnectPluginToCDFLoadEvent (Demo_SPM_Control_LoadValues_callback);

}

static void Demo_SPM_Control_show_callback( GtkWidget* widget, void* data){
	if ( Demo_SPM_ControlClass )
		Demo_SPM_ControlClass->show();
}

static void Demo_SPM_Control_StartScan_callback( gpointer ){
//	g_print ("SR-HwI::Demo_SPM_Control_StartScan_callback");
	if ( Demo_SPM_ControlClass )
		Demo_SPM_ControlClass->update();
}

static void Demo_SPM_Control_SaveValues_callback ( gpointer ncf ){
//	g_print ("SR-HwI::SPControl_SaveValues_callback\n");
	if ( Demo_SPM_ControlClass )
		Demo_SPM_ControlClass->save_values ((NcFile *) ncf);
}

static void Demo_SPM_Control_LoadValues_callback ( gpointer ncf ){
//	g_print ("SR-HwI::SPControl_LoadValues_callback\n");
	if ( Demo_SPM_ControlClass )
		Demo_SPM_ControlClass->load_values ((NcFile *) ncf);
}

// cleanup-Function
static void demo_hwi_cleanup(void)
{
	g_print ("SR-HwI::demo_hwi_cleanup -- Plugin Cleanup, --Menu\n");
	PI_DEBUG (DBG_L2, "demo_hwi Plugin Cleanup");

	gchar *mp = g_strconcat(Demo_SPM_Control_menupath, Demo_SPM_Control_menuentry, NULL);
	gnome_app_remove_menus (GNOME_APP( demo_hwi_pi.app->getApp() ), mp, 1);
	g_free(mp);

	// delete ...
	g_print ("SR-HwI::demo_hwi_cleanup -- Plugin Cleanup --DSPCoCl\n");
	if( Demo_SPM_ControlClass )
		delete Demo_SPM_ControlClass ;
	Demo_SPM_ControlClass = NULL;

	g_print ("SR-HwI::demo_hwi_cleanup -- Plugin Cleanup --sr_hwi\n");
	if (demo_hwi_hardware)
		delete demo_hwi_hardware;
	demo_hwi_hardware = NULL;

	g_print ("SR-HwI::demo_hwi_cleanup -- Plugin Cleanup --Info\n");
	g_free (demo_hwi_configure_string);
	demo_hwi_configure_string = NULL;

	g_print ("SR-HwI::demo_hwi_cleanup -- Plugin Cleanup done.\n");
}

