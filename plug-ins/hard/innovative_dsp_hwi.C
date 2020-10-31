/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Hardware Interface Plugin Name: innovative_dsp_hwi.C
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
% PlugInDocuCaption: Innovative DSP Hardware Interface for PC31 and PCI32 (OBSOLETE or to be ported)
% PlugInName: innovative_dsp_hwi
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: Hardware/Innovative_DSP:SPM:SPA-HwI

% PlugInDescription
This provides the Innovative DSP hardware interface for GXSM.
Supported are PC31 and PCI32.

% PlugInUsage
Set the \GxsmPref{Hardware}{Card} to ''Innovative\_Dsp:SPM'' or
''Innovative\_Dsp::SPA''.

%% OptPlugInSources

%% OptPlugInDest

% OptPlugInNote
This is an experimental code since GXSM2 and HwI. Use GXSM-1 (''Gxsm''
CVS branch) for a stable version.

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */

#include <sys/ioctl.h>

#include "config.h"
#include "gxsm3/plugin.h"
#include "gxsm3/xsmhard.h"

// Define HwI PlugIn reference name here, this is what is listed later within "Preferenced Dialog"
// i.e. the string selected for "Hardware/Card"!

#define THIS_HWI_PLUGIN_NAME "Innovative_DSP:SPM:SPA"

// Plugin Prototypes
static void innovative_dsp_hwi_init( void );
static void innovative_dsp_hwi_about( void );
static void innovative_dsp_hwi_configure( void );
static void innovative_dsp_hwi_cleanup( void );

// Fill in the GxsmPlugin Description here
GxsmPlugin innovative_dsp_hwi_pi = {
  NULL,                   // filled in and used by Gxsm, don't touch !
  NULL,                   // filled in and used by Gxsm, don't touch !
  0,                      // filled in and used by Gxsm, don't touch !
  NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
                          // filled in here by Gxsm on Plugin load, 
                          // just after init() is called !!!
  // ----------------------------------------------------------------------
  // Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
  "innovative_dsp_hwi-"
  "HW-INT-1S-SHORT",
  // Plugin's Category - used to autodecide on Pluginloading or ignoring
  // In this case of Hardware-Interface-Plugin here is the interface-name required
  // this is the string selected for "Hardware/Card"!
  THIS_HWI_PLUGIN_NAME,
  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
  "Innovative_Dsp hardware interface.",
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
  innovative_dsp_hwi_init,  
  // query-function pointer, can be "NULL", 
  // called if present after plugin init to let plugin manage it install itself
  NULL, // query should be "NULL" for Gxsm-Math-Plugin !!!
  // about-function, can be "NULL"
  // can be called by "Plugin Details"
  innovative_dsp_hwi_about,
  // configure-function, can be "NULL"
  // can be called by "Plugin Details"
  innovative_dsp_hwi_configure,
  // run-function, can be "NULL", if non-Zero and no query defined, 
  // it is called on menupath->"plugin"
  NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
  // cleanup-function, can be "NULL"
  // called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

  innovative_dsp_hwi_cleanup
};


// Text used in Aboutbox, please update!!
static const char *about_text = N_("GXSM innovative_dsp_hwi Plugin\n\n"
                                   "Innovative_Dsp Hardware Interface for SPM and SPA-LEED.");

/* Here we go... */

#include "innovative_dsp_hwi.h"

/*
 * PI global
 */

innovative_dsp_hwi_dev *innovative_dsp_hwi_hardware = NULL;

/* 
 * PI essential members
 */

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
  innovative_dsp_hwi_pi.description = g_strdup_printf(N_("GXSM HwI innovative_dsp_hwi plugin %s"), VERSION);
  return &innovative_dsp_hwi_pi; 
}

// Symbol "get_gxsm_hwi_hardware_class" is resolved by dlsym from Gxsm for all HwI type PIs, 
// Essential Plugin Function!!
XSM_Hardware *get_gxsm_hwi_hardware_class ( void *data ) {

// THIS_HWI_PLUGIN_NAME is "Innovative_DSP:SPM:SPA"

	if ( !strcmp ((char*)data, "Innovative_DSP:SPM") )
		return (innovative_dsp_hwi_hardware = new innovative_dsp_hwi_spm ());

	if ( !strcmp ((char*)data, "Innovative_DSP:SPA") )
		return (innovative_dsp_hwi_hardware = new innovative_dsp_hwi_spa ());

	return NULL;
}

// init-Function
static void innovative_dsp_hwi_init(void)
{
	PI_DEBUG (DBG_L2, "innovative_dsp_hwi Plugin Init");
	innovative_dsp_hwi_hardware = NULL; // not yet selected interface type
 }

// about-Function
static void innovative_dsp_hwi_about(void)
{
	const gchar *authors[] = { innovative_dsp_hwi_pi.authors, NULL};
	gtk_show_about_dialog (NULL, "program-name",  innovative_dsp_hwi_pi.name,
					"version", VERSION,
					  "license", GTK_LICENSE_GPL_3_0,
					  "comments", about_text,
					  "authors", authors,
					  NULL
				));
}

// configure-Function
static void innovative_dsp_hwi_configure(void)
{
	if(innovative_dsp_hwi_pi.app)
		innovative_dsp_hwi_pi.app->message("innovative_dsp_hwi Plugin Configuration");
}

// cleanup-Function
static void innovative_dsp_hwi_cleanup(void)
{
	PI_DEBUG (DBG_L2, "innovative_dsp_hwi Plugin Cleanup");
	delete innovative_dsp_hwi_hardware;
	innovative_dsp_hwi_hardware = NULL;
}

