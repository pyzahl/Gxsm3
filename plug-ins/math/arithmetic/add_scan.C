/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * plugin_helper reports your answers as
author		=Percy Zahl
email	        	=zahl@users.sf.net
pluginname		=add_scan
pluginmenuentry 	=Add X
menupath		=Math/Arithmetic/
entryplace		=Arithmetic
shortentryplace	=AR
abouttext		=adds the active to X channel
smallhelp		=add actibe and X
longhelp		=This is a detailed help for my Plugin.
 * 
 * Gxsm Plugin Name: add_scan.C
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
 * All "% OptPlugInXXX" tags are optional and can be removed or commented in
 * --------------------------------------------------------------------------------
% BeginPlugInDocuSection
% PlugInDocuCaption: Add two scans
% PlugInName: add_scan
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: Math/Arithmetic/Add X

% PlugInDescription
Adds the Z-values of two scan.

% PlugInUsage
Call \GxsmMenu{Math/Arithmetic/Add X}.

% OptPlugInSources
The active channel is added to the X channel.

%% OptPlugInObjects
%A optional rectangle is used for data extraction...

% OptPlugInDest
The computation result is placed into an existing math channel, else
into a new created math channel. The result is of type \GxsmEmph{float}.

% OptPlugInNotes
Both scans are required to have the same size in pixels.

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */

#include <gtk/gtk.h>
#include "config.h"
#include "gxsm3/plugin.h"

// Plugin Prototypes
static void add_scan_init( void );
static void add_scan_about( void );
static void add_scan_configure( void );
static void add_scan_cleanup( void );

// Define Type of math plugin here, only one line should be commented in!!
// #define GXSM_ONE_SRC_PLUGIN__DEF
#define GXSM_TWO_SRC_PLUGIN__DEF

// Math-Run-Function, use only one of (automatically done :=)
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
// "OneSrc" Prototype
 static gboolean add_scan_run( Scan *Src, Scan *Dest );
#else
// "TwoSrc" Prototype
 static gboolean add_scan_run( Scan *Src1, Scan *Src2, Scan *Dest );
#endif

// Fill in the GxsmPlugin Description here
GxsmPlugin add_scan_pi = {
  NULL,                   // filled in and used by Gxsm, don't touch !
  NULL,                   // filled in and used by Gxsm, don't touch !
  0,                      // filled in and used by Gxsm, don't touch !
  NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
                          // filled in here by Gxsm on Plugin load, 
                          // just after init() is called !!!
  // ----------------------------------------------------------------------
  // Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
  "add_scan-"
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
  "M1S"
#else
  "M2S"
#endif
  "-AR",
  // Plugin's Category - used to autodecide on Pluginloading or ignoring
  // NULL: load, else
  // example: "+noHARD +STM +AFM"
  // load only, if "+noHARD: no hardware" and Instrument is STM or AFM
  // +/-xxxHARD und (+/-INST or ...)
  NULL,
  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
  g_strdup ("This is a detailed help for my Plugin."), 
  // Author(s)
  "Percy Zahl",
  // Menupath to position where it is appendet to
  "math-arithmetic-section",
  // Menuentry
  N_("Add X"),
  // help text shown on menu
  N_("adds active and X"),
  // more info...
  "adds active and X.",
  NULL,          // error msg, plugin may put error status msg here later
  NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
  // init-function pointer, can be "NULL", 
  // called if present at plugin load
  add_scan_init,  
  // query-function pointer, can be "NULL", 
  // called if present after plugin init to let plugin manage it install itself
  NULL, // query should be "NULL" for Gxsm-Math-Plugin !!!
  // about-function, can be "NULL"
  // can be called by "Plugin Details"
  add_scan_about,
  // configure-function, can be "NULL"
  // can be called by "Plugin Details"
  add_scan_configure,
  // run-function, can be "NULL", if non-Zero and no query defined, 
  // it is called on menupath->"plugin"
  NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
  // cleanup-function, can be "NULL"
  // called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

  add_scan_cleanup
};

// special math Plugin-Strucure, use
// GxsmMathOneSrcPlugin add_scan_m1s_pi -> "OneSrcMath"
// GxsmMathTwoSrcPlugin add_scan_m2s_pi -> "TwoSrcMath"
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
 GxsmMathOneSrcPlugin add_scan_m1s_pi
#else
 GxsmMathTwoSrcPlugin add_scan_m2s_pi
#endif
 = {
   // math-function to run, see prototype(s) above!!
   add_scan_run
 };

// Text used in Aboutbox, please update!!
static const char *about_text = N_("Gxsm add_scan Plugin\n\n"
                                   "adds the active and X channel");

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
  add_scan_pi.description = g_strdup_printf(N_("Gxsm MathOneArg add_scan plugin %s"), VERSION);
  return &add_scan_pi; 
}

// Symbol "get_gxsm_math_one|two_src_plugin_info" is resolved by dlsym from Gxsm, 
// used to find out which Math Type the Plugin is!! 
// Essential Plugin Function!!
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
GxsmMathOneSrcPlugin *get_gxsm_math_one_src_plugin_info( void ) {
  return &add_scan_m1s_pi; 
}
#else
GxsmMathTwoSrcPlugin *get_gxsm_math_two_src_plugin_info( void ) { 
  return &add_scan_m2s_pi; 
}
#endif

/* Here we go... */
// init-Function
static void add_scan_init(void)
{
  PI_DEBUG (DBG_L2, "add_scan Plugin Init");
}

// about-Function
static void add_scan_about(void)
{
  const gchar *authors[] = { add_scan_pi.authors, NULL};
  gtk_show_about_dialog (NULL, 
			 "program-name",  add_scan_pi.name,
			 "version", VERSION,
			 "license", GTK_LICENSE_GPL_3_0,
			 "comments", about_text,
			 "authors", authors,
			 NULL
			 );
}

// configure-Function
static void add_scan_configure(void)
{
  if(add_scan_pi.app)
    add_scan_pi.app->message("add_scan Plugin Configuration");
}

// cleanup-Function
static void add_scan_cleanup(void)
{
  PI_DEBUG (DBG_L2, "add_scan Plugin Cleanup");
}

// run-Function
static gboolean add_scan_run(Scan *Src1, Scan *Src2, Scan *Dest)
{
	if(Src1->data.s.nx != Src2->data.s.nx || Src1->data.s.ny != Src2->data.s.ny)
		return MATH_SELECTIONERR;
	
	for(int v=0; v<Dest->mem2d->GetNv () && v<Src1->mem2d->GetNv () && v<Src2->mem2d->GetNv (); ++v)
		for(int line=0; line<Dest->mem2d->GetNy (); ++line)
			for(int col=0; col<Dest->mem2d->GetNx (); ++col)
				Dest->mem2d->PutDataPkt(
					Src1->mem2d->GetDataPkt (col, line, v)
					+ Src2->mem2d->GetDataPkt (col, line, v),
					col, line, v);
	
	return MATH_OK;
}


