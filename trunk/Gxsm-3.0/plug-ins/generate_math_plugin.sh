#!/bin/sh

# /* Gxsm - Gnome X Scanning Microscopy
# * universal STM/AFM/SARLS/SPALEED/... controlling and
# * data analysis software
# * 
# * Copyright (C) 1999,2000,2001 Stefan Schroeder, Percy Zahl
# *
# * Authors: Stefan Schroeder, Percy Zahl <zahl@users.sf.net>
# * additional features: Andreas Klust <klust@users.sf.net>
# * WWW Home: http://gxsm.sf.net
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
# */
#
# /* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

#
# plugin helper must be located in the plug-ins directory for operation.
# it is a tool for writing gxsm-plugin and is based on vorlage.C
#

# set default values
AUTHOR=$USERNAME
EMAIL=""
PLUGINNAME=mynewplugin
PLUGINMENUENTRY="My new Plugin"
ENTRYPLACE="_Misc" 					
MENUPATH=""					#this entry is calculated from ENTRYPLACE
SHORTENTRYPLACE=""				#this entry is calculated from ENTRYPLACE
ABOUTTEXT="All About my new Plugin - It's nice, isn't it?"
SMALLHELP="My new Plugin does useful things"
LONGHELP="This is a detailed help for my Plugin."
NUMBEROFSOURCES=2

echo 
echo
echo Welcome to the gxsm_plugin_helper
echo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
echo This is a small shell script for building a skeleton for
echo your next Gxsm-plugin.
echo It will ask several questions about your plugin and
echo generate the code for it as far as possible. After that you
echo will be directed to the directory where you will find your
echo new plugin for further development. 
echo
echo For many questions default values are given. If you made a mistake:
echo Dont worry. All entries you type are simple textvalues
echo in the created source-files. You can change them freely.
echo But now lets begin:

echo What is your name [$AUTHOR]:
DUMMY=""; read DUMMY
if test -n "$DUMMY" ; then 
  AUTHOR=$DUMMY 
fi

echo What is your Email address [$EMAIL]:
DUMMY=""; read DUMMY
if test -n "$DUMMY" ; then 
  EMAIL=$DUMMY 
fi

echo Give a unique, singleworded name, this will be your filename [$PLUGINNAME]:
DUMMY=""; read DUMMY
if test -n "$DUMMY"; then 
  PLUGINNAME=$DUMMY 
fi

echo Give a folder for your Plugin [$ENTRYPLACE]:
echo Possible Values: _Convert, _Arithmetic, _Background, 
echo                  Filter _1D, Filter _2D, _Statistics, _Transformations, _Misc
echo Attention: the leading underscore is important!
DUMMY=""; read DUMMY
if test -n "$DUMMY" ; then 
  ENTRYPLACE=$DUMMY 
fi

echo Give a Menuentry for your Plugin [$PLUGINMENUENTRY]:
DUMMY=""; read DUMMY
if test -n "$DUMMY" ; then 
  PLUGINMENUENTRY=$DUMMY 
fi

echo Give an About-Text [$ABOUTTEXT]:
DUMMY=""; read DUMMY
if test -n "$DUMMY" ; then 
  ABOUTTEXT=$DUMMY 
fi

echo Enter a small helptext for the statusline [$SMALLHELP]:
DUMMY=""; read DUMMY
if test -n "$DUMMY" ; then 
  SMALLHELP=$DUMMY 
fi


echo Enter a long helptext for the plugin-viewer [$LONGHELP]:
DUMMY=""; read DUMMY
if test -n "$DUMMY" ; then 
  LONGHELP=$DUMMY 
fi

echo Enter number of sources [$NUMBEROFSOURCES]
echo Possible values 1 or 2:
DUMMY=""; read DUMMY
if test -n "$DUMMY" ; then 
  NUMBEROFSOURCES=$DUMMY 
fi

#	calculate MENUPATH
MENUPATH="_Math/$ENTRYPLACE/"

#	calculate pluginname 
if [ "$ENTRYPLACE" = "_Background" ]; then
  SHORTENTRYPLACE="BG"
  TARGETDIRECTORY="math/background"  
elif [ "$ENTRYPLACE" = "_Convert" ]; then
  SHORTENTRYPLACE="CV"
  TARGETDIRECTORY="math/convert"  
elif [ "$ENTRYPLACE" = "_Arithmetic" ]; then
  SHORTENTRYPLACE="AR"
  TARGETDIRECTORY="math/arithmetic"  
elif [ "$ENTRYPLACE" = "Filter _1D" ]; then
  SHORTENTRYPLACE="F1D"
  TARGETDIRECTORY="math/filter1d"  
elif [ "$ENTRYPLACE" = "Filter _2D" ]; then
  SHORTENTRYPLACE="F2D"
  TARGETDIRECTORY="math/filter2d"  
elif [ "$ENTRYPLACE" = "_Statistics" ]; then
  SHORTENTRYPLACE="ST"
  TARGETDIRECTORY="math/statistik"  
elif [ "$ENTRYPLACE" = "_Transformations" ]; then
  SHORTENTRYPLACE="TR"
  TARGETDIRECTORY="math/transform"  
elif [ "$ENTRYPLACE" = "_Misc" ]; then
  SHORTENTRYPLACE="Misc"
  TARGETDIRECTORY="math/misc"  
fi

echo Our targetdirectory is $TARGETDIRECTORY

#
#	Adding plugin to appropriate Makefile.am
#

echo Now adding your plugin to $TARGETDIRECTORY/Makefile.am. Old Makefile.am is copied to oldMakefile.am
mv $TARGETDIRECTORY/Makefile.am $TARGETDIRECTORY/oldMakefile.am
sed "s/lib_LTLIBRARIES = /lib_LTLIBRARIES = lib$PLUGINNAME.la /" $TARGETDIRECTORY/oldMakefile.am > $TARGETDIRECTORY/Makefile.am

echo >> $TARGETDIRECTORY/Makefile.am
echo lib"$PLUGINNAME"_la_SOURCES = $PLUGINNAME.C >> $TARGETDIRECTORY/Makefile.am
echo lib"$PLUGINNAME"_la_LDFLAGS = -module -export-dynamic -avoid-version >> $TARGETDIRECTORY/Makefile.am

#
#	create source-file, based on extraction from this file
#

tail -n 344 ./generate_math_plugin.sh > $TARGETDIRECTORY/"$PLUGINNAME".TEMP1

#
# remove comments
#
sed "s/#%//g" $TARGETDIRECTORY/"$PLUGINNAME".TEMP1 > $TARGETDIRECTORY/"$PLUGINNAME".TEMP2

function swap {
  mv $TARGETDIRECTORY/"$PLUGINNAME".TEMP1 $TARGETDIRECTORY/"$PLUGINNAME".TEMP2
}

#
#	replace variables in source with correct values
#
sed "s/___AUTHOR/$AUTHOR/g" $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".TEMP1; swap;
sed "s/___EMAIL/$EMAIL/g" $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".TEMP1; swap;
sed "s/___PLUGINNAME/$PLUGINNAME/g" $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".TEMP1; swap;
sed "s/___PLUGINMENUENTRY/$PLUGINMENUENTRY/g" $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".TEMP1; swap;
sed "s+___MENUPATH+$MENUPATH+g" $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".TEMP1; swap;
sed "s/___ENTRYPLACE/$ENTRYPLACE/g" $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".TEMP1; swap;
sed "s/___SHORTENTRYPLACE/$SHORTENTRYPLACE/g" $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".TEMP1; swap;
sed "s/___ABOUTTEXT/$ABOUTTEXT/g" $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".TEMP1; swap;
sed "s/___SMALLHELP/$SMALLHELP/g" $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".TEMP1; swap;
sed "s/___LONGHELP/$LONGHELP/g" $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".TEMP1; swap;

if [ "$NUMBEROFSOURCES" = "1" ]; then
  sed "s+// #define GXSM_ONE_SRC_PLUGIN__DEF+#define GXSM_ONE_SRC_PLUGIN__DEF+" $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".C
else
  sed "s+// #define GXSM_TWO_SRC_PLUGIN__DEF+#define GXSM_TWO_SRC_PLUGIN__DEF+ " $TARGETDIRECTORY/"$PLUGINNAME".TEMP2 > $TARGETDIRECTORY/"$PLUGINNAME".C  
fi
rm $TARGETDIRECTORY/"$PLUGINNAME".TEMP2  

#
# do the compile stuff
#
echo The following files have been altered:
echo $TARGETDIRECTORY/Makefile.am and $TARGETDIRECTORY/"$PLUGINNAME".C
echo Now sleeping for 10 sec. Interrupt with control+C if you dont want to do make install
echo If you use stow, dont forget to run it.
sleep 10
make install
#
echo Finished. Now go to directory $TARGETDIRECTORY and have a look at "$PLUGINNAME".C
#


#%----------------------------------------------------------------------------
#%/* Gnome gxsm - Gnome X Scanning Microscopy
#% * universal STM/AFM/SARLS/SPALEED/... controlling and
#% * data analysis software
#% *
#% * plugin_helper reports your answers as
#%author		=___AUTHOR
#%email	        	=___EMAIL
#%pluginname		=___PLUGINNAME
#%pluginmenuentry 	=___PLUGINMENUENTRY
#%menupath		=___MENUPATH
#%entryplace		=___ENTRYPLACE
#%shortentryplace	=___SHORTENTRYPLACE
#%abouttext		=___ABOUTTEXT
#%smallhelp		=___SMALLHELP
#%longhelp		=___LONGHELP
#% * 
#% * Gxsm Plugin Name: ___PLUGINNAME.C
#% * ========================================
#% * 
#% * Copyright (C) 1999 The Free Software Foundation
#% *
#% * Authors: Percy Zahl <zahl@fkp.uni-hannover.de>
#% * additional features: Andreas Klust <klust@fkp.uni-hannover.de>
#% *
#% * This program is free software; you can redistribute it and/or modify
#% * it under the terms of the GNU General Public License as published by
#% * the Free Software Foundation; either version 2 of the License, or
#% * (at your option) any later version.
#% *
#% * This program is distributed in the hope that it will be useful,
#% * but WITHOUT ANY WARRANTY; without even the implied warranty of
#% * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#% * GNU General Public License for more details.
#% *
#% * You should have received a copy of the GNU General Public License
#% * along with this program; if not, write to the Free Software
#% * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
#% */
#%
#%
#%/* Please do not change the Begin/End lines of this comment section!
#% * this is a LaTeX style section used for auto generation of the PlugIn Manual 
#% * Chapter. Add a complete PlugIn documentation inbetween the Begin/End marks!
#% * All "% PlugInXXX" commentary tags are mandatory
#% * All "% OptPlugInXXX" tags are optional and can be removed or commented in
#% * --------------------------------------------------------------------------------
#%% BeginPlugInDocuSection
#%% PlugInDocuCaption: ___MENUPATH___PLUGINMENUENTRY replace this with a intuitive and short caption!
#%% PlugInName: ___PLUGINNAME
#%% PlugInAuthor: ___AUTHOR
#%% PlugInAuthorEmail: ___EMAIL
#%% PlugInMenuPath: ___MENUPATH___PLUGINMENUENTRY
#%
#%% PlugInDescription
#%A brief description goes here.
#%
#%% PlugInUsage
#%Write how to use it.
#%
#%%% OptPlugInSection: replace this by the section caption
#%%all following lines until next tag are going into this section
#%%...
#%
#%%% OptPlugInSubSection: replace this line by the subsection caption
#%%all following lines until next tag are going into this subsection
#%%...
#%
#%%% you can repeat OptPlugIn(Sub)Sections multiple times!
#%
#%%% OptPlugInSources
#%%The active channel is used as data source.
#%
#%%% OptPlugInObjects
#%%A optional rectangle is used for data extraction...
#%
#%%% OptPlugInDest
#%%The computation result is placed into an existing math channel, else into a new created math channel.
#%
#%%% OptPlugInConfig
#%%describe the configuration options of your plug in here!
#%
#%%% OptPlugInFiles
#%%Does it uses, needs, creates any files? Put info here!
#%
#%%% OptPlugInRefs
#%%Any references?
#%
#%%% OptPlugInKnownBugs
#%%Are there known bugs? List! How to work around if not fixed?
#%
#%%% OptPlugInNotes
#%%If you have any additional notes
#%
#%%% OptPlugInHints
#%%Any tips and tricks?
#%
#%% EndPlugInDocuSection
#% * -------------------------------------------------------------------------------- 
#% */
#%
#%#include <gtk/gtk.h>
#%#include "config.h"
#%#include "gxsm3/plugin.h"
#%
#%// Plugin Prototypes
#%static void ___PLUGINNAME_init( void );
#%static void ___PLUGINNAME_about( void );
#%static void ___PLUGINNAME_configure( void );
#%static void ___PLUGINNAME_cleanup( void );
#%
#%// Define Type of math plugin here, only one line should be commented in!!
#%// #define GXSM_ONE_SRC_PLUGIN__DEF
#%// #define GXSM_TWO_SRC_PLUGIN__DEF
#%
#%// Math-Run-Function, use only one of (automatically done :=)
#%#ifdef GXSM_ONE_SRC_PLUGIN__DEF
#%// "OneSrc" Prototype
#% static gboolean ___PLUGINNAME_run( Scan *Src, Scan *Dest );
#%#else
#%// "TwoSrc" Prototype
#% static gboolean ___PLUGINNAME_run( Scan *Src1, Scan *Src2, Scan *Dest );
#%#endif
#%
#%// Fill in the GxsmPlugin Description here
#%GxsmPlugin ___PLUGINNAME_pi = {
#%  NULL,                   // filled in and used by Gxsm, don't touch !
#%  NULL,                   // filled in and used by Gxsm, don't touch !
#%  0,                      // filled in and used by Gxsm, don't touch !
#%  NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
#%                          // filled in here by Gxsm on Plugin load, 
#%                          // just after init() is called !!!
#%  // ----------------------------------------------------------------------
#%  // Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
#%  "___PLUGINNAME-"
#%#ifdef GXSM_ONE_SRC_PLUGIN__DEF
#%  "M1S"
#%#else
#%  "M2S"
#%#endif
#%  "-___SHORTENTRYPLACE",
#%  // Plugin's Category - used to autodecide on Pluginloading or ignoring
#%  // NULL: load, else
#%  // example: "+noHARD +STM +AFM"
#%  // load only, if "+noHARD: no hardware" and Instrument is STM or AFM
#%  // +/-xxxHARD und (+/-INST or ...)
#%  NULL,
#%  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
#%  "___LONGHELP",                   
#%  // Author(s)
#%  "___AUTHOR",
#%  // Menupath to position where it is appendet to
#%  N_("___MENUPATH"),
#%  // Menuentry
#%  N_("___PLUGINMENUENTRY"),
#%  // help text shown on menu
#%  N_("___LONGHELP"),
#%  // more info...
#%  "___SMALLHELP",
#%  NULL,          // error msg, plugin may put error status msg here later
#%  NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
#%  // init-function pointer, can be "NULL", 
#%  // called if present at plugin load
#%  ___PLUGINNAME_init,  
#%  // query-function pointer, can be "NULL", 
#%  // called if present after plugin init to let plugin manage it install itself
#%  NULL, // query should be "NULL" for Gxsm-Math-Plugin !!!
#%  // about-function, can be "NULL"
#%  // can be called by "Plugin Details"
#%  ___PLUGINNAME_about,
#%  // configure-function, can be "NULL"
#%  // can be called by "Plugin Details"
#%  ___PLUGINNAME_configure,
#%  // run-function, can be "NULL", if non-Zero and no query defined, 
#%  // it is called on menupath->"plugin"
#%  NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
#%  // cleanup-function, can be "NULL"
#%  // called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

#%  ___PLUGINNAME_cleanup
#%};
#%
#%// special math Plugin-Strucure, use
#%// GxsmMathOneSrcPlugin ___PLUGINNAME_m1s_pi -> "OneSrcMath"
#%// GxsmMathTwoSrcPlugin ___PLUGINNAME_m2s_pi -> "TwoSrcMath"
#%#ifdef GXSM_ONE_SRC_PLUGIN__DEF
#% GxsmMathOneSrcPlugin ___PLUGINNAME_m1s_pi
#%#else
#% GxsmMathTwoSrcPlugin ___PLUGINNAME_m2s_pi
#%#endif
#% = {
#%   // math-function to run, see prototype(s) above!!
#%   ___PLUGINNAME_run
#% };
#%
#%// Text used in Aboutbox, please update!!
#%static const char *about_text = N_("Gxsm ___PLUGINNAME Plugin\n\n"
#%                                   "___ABOUTTEXT");
#%
#%// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
#%// Essential Plugin Function!!
#%GxsmPlugin *get_gxsm_plugin_info ( void ){ 
#%  ___PLUGINNAME_pi.description = g_strdup_printf(N_("Gxsm MathOneArg ___PLUGINNAME plugin %s"), VERSION);
#%  return &___PLUGINNAME_pi; 
#%}
#%
#%// Symbol "get_gxsm_math_one|two_src_plugin_info" is resolved by dlsym from Gxsm, 
#%// used to find out which Math Type the Plugin is!! 
#%// Essential Plugin Function!!
#%#ifdef GXSM_ONE_SRC_PLUGIN__DEF
#%GxsmMathOneSrcPlugin *get_gxsm_math_one_src_plugin_info( void ) {
#%  return &___PLUGINNAME_m1s_pi; 
#%}
#%#else
#%GxsmMathTwoSrcPlugin *get_gxsm_math_two_src_plugin_info( void ) { 
#%  return &___PLUGINNAME_m2s_pi; 
#%}
#%#endif
#%
#%/* Here we go... */
#%// init-Function
#%static void ___PLUGINNAME_init(void)
#%{
#%  PI_DEBUG (DBG_L2, "Plugin Init" );
#%}
#%
#%// about-Function
#%static void ___PLUGINNAME_about(void)
#%{
#%  const gchar *authors[] = { ___PLUGINNAME_pi.authors, NULL};
#%  gtk_show_about_dialog (NULL, "program-name",  ___PLUGINNAME_pi.name,
#%				  "version", VERSION,
#%				    "license", GTK_LICENSE_GPL_3_0,
#%				    "comments", about_text,
#%				    "authors", authors,
#%				    NULL
#%				    ));
#%}
#%
#%// configure-Function
#%static void ___PLUGINNAME_configure(void)
#%{
#%  if(___PLUGINNAME_pi.app)
#%    ___PLUGINNAME_pi.app->message("___PLUGINNAME Plugin Configuration");
#%}
#%
#%// cleanup-Function
#%static void ___PLUGINNAME_cleanup(void)
#%{
#%  PI_DEBUG (DBG_L2, "Plugin Cleanup");
#%}
#%
#%// run-Function
#%#ifdef GXSM_ONE_SRC_PLUGIN__DEF
#% static gboolean ___PLUGINNAME_run(Scan *Src, Scan *Dest)
#%#else
#% static gboolean ___PLUGINNAME_run(Scan *Src1, Scan *Src2, Scan *Dest)
#%#endif
#%{
#%  // put plugins math code here...
#%  // For more math-access methods have a look at xsmmath.C
#%#ifdef GXSM_ONE_SRC_PLUGIN__DEF
#%  // simple example for 1sourced Mathoperation: Source is taken and 100 added.
#%  for(int line=0; line < Dest->mem2d->GetNy (); line++)
#%    for(int col=0; col < Dest->mem2d->GetNx (); col++)
#%      Dest->mem2d->PutDataPkt (Src->mem2d->GetDataPkt (line, col) + 100, col, line);
#%#else
#%  // simple example for 2sourced Mathoperation: Source1 and Source2 are added.
#%  int line, col;
#%
#%  if(Src1->data.s.nx != Src2->data.s.nx || Src1->data.s.ny != Src2->data.s.ny)
#%    return MATH_SELECTIONERR;
#%
#%  for(line=0; line<Dest->mem2d->GetNy (); line++)
#%    for(col=0; col<Dest->mem2d->GetNx (); col++)
#%      Dest->mem2d->PutDataPkt(
#%                              Src1->mem2d->GetDataPkt (col, line)
#%                            + Src2->mem2d->GetDataPkt (col, line),
#%                              col, line);
#%#endif
#%
#%
#%#if 0
#%       // just more code for demonstration of how to acess layers and time domain -- a drift generator:
#%
#%	gapp->progress_info_new ("Auto aligning", 2); // setup an progress indicator
#%	gapp->progress_info_set_bar_fraction (0., 1);
#%	gapp->progress_info_set_bar_fraction (0., 2);
#%	gapp->progress_info_set_bar_text ("Time", 1);
#%	gapp->progress_info_set_bar_text ("Value", 2);
#%
#%	// number of time frames stored
#%	int n_times = Src->number_of_time_elements ();
#%
#%	// resize Dest to match Src
#%	Dest->mem2d->Resize (Src->mem2d->GetNx (), Src->mem2d->GetNy (), Src->mem2d->GetNv (), ZD_IDENT);
#%
#%	// artifical drift parameters
#%	double d_tau=18.;
#%	double tcx=0.2;
#%	double tcy=0.1;
#%	double drift_x, drift_y;
#%
#%	// start with zero offset
#%	drift_x = drift_y = 0.;
#%	
#%	// go for it....
#%	for(int time=0; time < n_times; ++time){ // time loop
#%		gapp->progress_info_set_bar_fraction ((gdouble)time/(gdouble)n_times, 1);
#%
#%		// get time frame to work on
#%		double real_time = Src->retrieve_time_element (time);
#%
#%		int n_values = Dest->mem2d->GetNv ();
#%		for(int value=0; value < n_values; ++value){ // value loop
#%			gapp->progress_info_set_bar_fraction ((gdouble)value/(gdouble)n_values, 2);
#%
#%			Src->mem2d->SetLayer(value);
#%			Dest->mem2d->SetLayer(value);
#%
#%			for(int line=0; line < Dest->mem2d->GetNy (); ++line) // lines (y)
#%				for(int col=0; col < Dest->mem2d->GetNx (); ++col){ // columns (x)
#%					double x = col - drift_x;
#%					double y = line - drift_y;
#%
#%					// Lookup pixel interpolated at x,y and put to col,line
#%					Dest->mem2d->PutDataPkt (Src->mem2d->GetDataPktInterpol (x, y), col, line);
#%				}
#%		}
#%
#%		// now append new time frame to Destination Scan
#%		Dest->append_current_to_time_elements (time, real_time);
#%
#%		// making up some drift
#%		drift_x += d_tau * exp(-(double)time * tcy);
#%		drift_y += d_tau * exp(-(double)time * tcy);
#%	}
#%
#%	gapp->progress_info_close (); // close progress indicator
#%#endif
#%
#%
#%  return MATH_OK;
#%}
#%
#%
