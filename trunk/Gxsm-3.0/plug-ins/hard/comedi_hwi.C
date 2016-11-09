/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Hardware Interface Plugin Name: comedi_hwi.C
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
% PlugInModuleIgnore

% BeginPlugInDocuSection
% PlugInDocuCaption: Comedi Demonstration/Experimental Hardware Interface (OBSOLETE)
% PlugInName: comedi_hwi
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: Hardware/Comedi:SPM-HwI

% PlugInDescription
This provides a dummy data simulating hardware interface for GXSM. For
demonstartive purpose only.

% PlugInUsage
Set the \GxsmPref{Hardware}{Card} to ''Comedi:SPM''.

%% OptPlugInSources

%% OptPlugInDest

% OptPlugInNote
For experimental and demonstrational/template purpose only.

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */

#include <sys/ioctl.h>

#include "config.h"
#include "gxsm/plugin.h"
#include "gxsm/xsmhard.h"

// Define HwI PlugIn reference name here, this is what is listed later within "Preferenced Dialog"
// i.e. the string selected for "Hardware/Card"!
#define THIS_HWI_PLUGIN_NAME "Comedi:SPM"

// Plugin Prototypes
static void comedi_hwi_init( void );
static void comedi_hwi_about( void );
static void comedi_hwi_configure( void );
static void comedi_hwi_cleanup( void );

// Fill in the GxsmPlugin Description here
GxsmPlugin comedi_hwi_pi = {
  NULL,                   // filled in and used by Gxsm, don't touch !
  NULL,                   // filled in and used by Gxsm, don't touch !
  0,                      // filled in and used by Gxsm, don't touch !
  NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
                          // filled in here by Gxsm on Plugin load, 
                          // just after init() is called !!!
  // ----------------------------------------------------------------------
  // Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
  "comedi_hwi-"
  "HW-INT-1S-SHORT",
  // Plugin's Category - used to autodecide on Pluginloading or ignoring
  // In this case of Hardware-Interface-Plugin here is the interface-name required
  // this is the string selected for "Hardware/Card"!
  THIS_HWI_PLUGIN_NAME,
  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
  "Comedi hardware interface.",
  // Author(s)
  "Percy Zahl",
  // Menupath to position where it is appendet to -- not used by HwI PIs
  N_("Hardware/"),
  // Menuentry -- not used by HwI PIs
  N_(THIS_HWI_PLUGIN_NAME"-HwI"),
  // help text shown on menu
  N_("This is the "THIS_HWI_PLUGIN_NAME" - GXSM Hardware Interface"),
  // more info...
  "N/A",
  NULL,          // error msg, plugin may put error status msg here later
  NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
  // init-function pointer, can be "NULL", 
  // called if present at plugin load
  comedi_hwi_init,  
  // query-function pointer, can be "NULL", 
  // called if present after plugin init to let plugin manage it install itself
  NULL, // query should be "NULL" for Gxsm-Math-Plugin !!!
  // about-function, can be "NULL"
  // can be called by "Plugin Details"
  comedi_hwi_about,
  // configure-function, can be "NULL"
  // can be called by "Plugin Details"
  comedi_hwi_configure,
  // run-function, can be "NULL", if non-Zero and no query defined, 
  // it is called on menupath->"plugin"
  NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
  // cleanup-function, can be "NULL"
  // called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

  comedi_hwi_cleanup
};


// Text used in Aboutbox, please update!!
static const char *about_text = N_("GXSM comedi_hwi Plugin\n\n"
                                   "Comedi Hardware Interface for SPM.");

/* Here we go... */

#include "comedi_hwi.h"

/*
 * PI global
 */

comedi_hwi_dev *comedi_hwi_hardware = NULL;

/* 
 * PI essential members
 */

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
  comedi_hwi_pi.description = g_strdup_printf(N_("GXSM HwI comedi_hwi plugin %s"), VERSION);
  return &comedi_hwi_pi; 
}

// Symbol "get_gxsm_hwi_hardware_class" is resolved by dlsym from Gxsm for all HwI type PIs, 
// Essential Plugin Function!!
XSM_Hardware *get_gxsm_hwi_hardware_class ( void *data ) {
	return comedi_hwi_hardware;
}

// init-Function
static void comedi_hwi_init(void)
{
	PI_DEBUG (DBG_L2, "comedi_hwi Plugin Init");
	comedi_hwi_hardware = new comedi_hwi_spm ();
 }

// about-Function
static void comedi_hwi_about(void)
{
	const gchar *authors[] = { comedi_hwi_pi.authors, NULL};
	gtk_show_about_dialog (NULL, "program-name",  comedi_hwi_pi.name,
					"version", VERSION,
					  "license", GTK_LICENSE_GPL_3_0,
					  "comments", about_text,
					  "authors", authors,
					  NULL
				));
}

// configure-Function
static void comedi_hwi_configure(void)
{
	if(comedi_hwi_pi.app)
		comedi_hwi_pi.app->message("comedi_hwi Plugin Configuration");
}

// cleanup-Function
static void comedi_hwi_cleanup(void)
{
	PI_DEBUG (DBG_L2, "comedi_hwi Plugin Cleanup");
	delete comedi_hwi_hardware;
	comedi_hwi_hardware = NULL;
}

