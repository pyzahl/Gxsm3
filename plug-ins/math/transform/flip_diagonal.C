/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * plugin_helper reports your answers as
author		=Percy Zahl
email	        	=zahl@users.sf.net
pluginname		=flip_diagonal
pluginmenuentry 	=Flip Diagonal
menupath		=Math/Transformations/
entryplace		=Transformations
shortentryplace	=TR
abouttext		=Flips an image along it's diagonale.
smallhelp		=Flips an image diagonal
longhelp		=This is a detailed help for my Plugin.
 * 
 * Gxsm Plugin Name: flip_diagonal.C
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
% PlugInDocuCaption: Flip diagonal
% PlugInName: flip_diagonal
% PlugInAuthor: A. Klust, P. Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: Math/Transformations/Flip Diagonal

% PlugInDescription
Flips an image along it's diagonale.

% PlugInUsage
Call \GxsmMenu{Math/Transformations/Flip Diagonal}.

% OptPlugInSources
The active channel is used as data source.

%% OptPlugInObjects
%A optional rectangle is used for data extraction...

%% OptPlugInDest
%The computation result is placed into an existing math channel, else into a new created math channel.

%% OptPlugInConfig
%describe the configuration options of your plug in here!

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */

#include <gtk/gtk.h>
#include "config.h"
#include "gxsm3/plugin.h"

// Plugin Prototypes
static void flip_diagonal_init( void );
static void flip_diagonal_about( void );
static void flip_diagonal_configure( void );
static void flip_diagonal_cleanup( void );

// Define Type of math plugin here, only one line should be commented in!!
#define GXSM_ONE_SRC_PLUGIN__DEF
// #define GXSM_TWO_SRC_PLUGIN__DEF

// Math-Run-Function, use only one of (automatically done :=)
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
// "OneSrc" Prototype
static gboolean flip_diagonal_run( Scan *Src, Scan *Dest );
#else
// "TwoSrc" Prototype
static gboolean flip_diagonal_run( Scan *Src1, Scan *Src2, Scan *Dest );
#endif

// Fill in the GxsmPlugin Description here
GxsmPlugin flip_diagonal_pi = {
	NULL,                   // filled in and used by Gxsm, don't touch !
	NULL,                   // filled in and used by Gxsm, don't touch !
	0,                      // filled in and used by Gxsm, don't touch !
	NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
	// filled in here by Gxsm on Plugin load, 
	// just after init() is called !!!
	// ----------------------------------------------------------------------
	// Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
	"flip_diagonal-"
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
	"M1S"
#else
	"M2S"
#endif
	"-TR",
	// Plugin's Category - used to autodecide on Pluginloading or ignoring
	// NULL: load, else
	// example: "+noHARD +STM +AFM"
	// load only, if "+noHARD: no hardware" and Instrument is STM or AFM
	// +/-xxxHARD und (+/-INST or ...)
	NULL,
	// Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
	g_strdup("This is a detailed help for my Plugin."),                   
	// Author(s)
	"Percy Zahl",
	// Menupath to position where it is appendet to
	"math-transformations-section",
	// Menuentry
	N_("Flip Diagonal"),
	// help text shown on menu
	N_("This is a detailed help for my Plugin."),
	// more info...
	"Flips an image diagonal",
	NULL,          // error msg, plugin may put error status msg here later
	NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
	// init-function pointer, can be "NULL", 
	// called if present at plugin load
	flip_diagonal_init,  
	// query-function pointer, can be "NULL", 
	// called if present after plugin init to let plugin manage it install itself
	NULL, // query should be "NULL" for Gxsm-Math-Plugin !!!
	// about-function, can be "NULL"
	// can be called by "Plugin Details"
	flip_diagonal_about,
	// configure-function, can be "NULL"
	// can be called by "Plugin Details"
	flip_diagonal_configure,
	// run-function, can be "NULL", if non-Zero and no query defined, 
	// it is called on menupath->"plugin"
	NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
	// cleanup-function, can be "NULL"
	// called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

	flip_diagonal_cleanup
};

// special math Plugin-Strucure, use
// GxsmMathOneSrcPlugin flip_diagonal_m1s_pi -> "OneSrcMath"
// GxsmMathTwoSrcPlugin flip_diagonal_m2s_pi -> "TwoSrcMath"
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
GxsmMathOneSrcPlugin flip_diagonal_m1s_pi
#else
GxsmMathTwoSrcPlugin flip_diagonal_m2s_pi
#endif
= {
	// math-function to run, see prototype(s) above!!
	flip_diagonal_run
};

// Text used in Aboutbox, please update!!
static const char *about_text = N_("Gxsm flip_diagonal Plugin\n\n"
                                   "Flips an image along it's diagonale.");

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
	flip_diagonal_pi.description = g_strdup_printf(N_("Gxsm MathOneArg flip_diagonal plugin %s"), VERSION);
	return &flip_diagonal_pi; 
}

// Symbol "get_gxsm_math_one|two_src_plugin_info" is resolved by dlsym from Gxsm, 
// used to find out which Math Type the Plugin is!! 
// Essential Plugin Function!!
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
GxsmMathOneSrcPlugin *get_gxsm_math_one_src_for_all_vt_plugin_info( void ) {
	return &flip_diagonal_m1s_pi; 
}
#else
GxsmMathTwoSrcPlugin *get_gxsm_math_two_src_plugin_info( void ) { 
	return &flip_diagonal_m2s_pi; 
}
#endif

/* Here we go... */
// init-Function
static void flip_diagonal_init(void)
{
	PI_DEBUG (DBG_L2, "flip_diagonal Plugin Init");
}

// about-Function
static void flip_diagonal_about(void)
{
	const gchar *authors[] = { flip_diagonal_pi.authors, NULL};
	gtk_show_about_dialog (NULL,
			       "program-name",  flip_diagonal_pi.name,
			       "version", VERSION,
			       "license", GTK_LICENSE_GPL_3_0,
			       "comments", about_text,
			       "authors", authors,
			       NULL
			       );
}

// configure-Function
static void flip_diagonal_configure(void)
{
	if(flip_diagonal_pi.app)
		flip_diagonal_pi.app->message("flip_diagonal Plugin Configuration");
}

// cleanup-Function
static void flip_diagonal_cleanup(void)
{
	PI_DEBUG (DBG_L2, "flip_diagonal Plugin Cleanup");
}

// run-Function
static gboolean flip_diagonal_run(Scan *Src, Scan *Dest)
{
	if (!Src || !Dest)
		return 0;

	Dest->mem2d->Resize (Src->mem2d->GetNy (), Src->mem2d->GetNx ());
	Dest->data.s.nx = Dest->mem2d->GetNx ();
	Dest->data.s.ny = Dest->mem2d->GetNy ();
	Dest->data.s.rx = Src->data.s.ry;
	Dest->data.s.dx = Src->data.s.dy;
	Dest->data.s.ry = Src->data.s.rx;
	Dest->data.s.dy = Src->data.s.dx;

	for (int col = 0; col < Dest->mem2d->GetNx (); col++)
		for (int line = 0; line < Dest->mem2d->GetNy (); line++)
			Dest->mem2d->PutDataPkt (Src->mem2d->GetDataPkt (line, col), 
						 col, line);

	return MATH_OK;
}


