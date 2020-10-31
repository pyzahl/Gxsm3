/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Plugin Name: despike2d.C
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
% PlugInDocuCaption: Despike 2d
% PlugInName: despike2d
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: Math/Background/Despike2d

% PlugInDescription
Despike 2d filter.

% PlugInUsage
Call \GxsmMenu{Math/Filter 2D/Despike}.

% OptPlugInSources
The active channel is used as data source.

% OptPlugInDest
The computation result is placed into an existing math channel, else
into a new created math channel.

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */


#include <gtk/gtk.h>
#include "config.h"
#include "gxsm3/plugin.h"

// Plugin Prototypes
static void despike2d_init( void );
static void despike2d_about( void );
static void despike2d_configure( void );
static void despike2d_cleanup( void );

// Define Type of math plugin here, only one line should be commented in!!
#define GXSM_ONE_SRC_PLUGIN__DEF
// #define GXSM_TWO_SRC_PLUGIN__DEF

// Math-Run-Function, use only one of (automatically done :=)
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
// "OneSrc" Prototype
 static gboolean despike2d_run( Scan *Src, Scan *Dest );
#else
// "TwoSrc" Prototype
 static gboolean despike2d_run( Scan *Src1, Scan *Src2, Scan *Dest );
#endif

// Fill in the GxsmPlugin Description here
GxsmPlugin despike2d_pi = {
  NULL,                   // filled in and used by Gxsm, don't touch !
  NULL,                   // filled in and used by Gxsm, don't touch !
  0,                      // filled in and used by Gxsm, don't touch !
  NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
                          // filled in here by Gxsm on Plugin load, 
                          // just after init() is called !!!
  // ----------------------------------------------------------------------
  // Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
  "despike2d-"
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
  "M1S"
#else
  "M2S"
#endif
  "-BG",
  // Plugin's Category - used to autodecide on Pluginloading or ignoring
  // NULL: load, else
  // example: "+noHARD +STM +AFM"
  // load only, if "+noHARD: no hardware" and Instrument is STM or AFM
  // +/-xxxHARD und (+/-INST or ...)
  NULL,
  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
  g_strdup("Despike2d math filter."),                   
  // Author(s)
  "Percy Zahl",
  // Menupath to position where it is appendet to
  "math-filter2d-section",
  // Menuentry
  N_("Despike"),
  // help text shown on menu
  N_("Despike 2d filter for data."),
  // more info...
  "Despike 2d removed spikes from data.",
  NULL,          // error msg, plugin may put error status msg here later
  NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
  // init-function pointer, can be "NULL", 
  // called if present at plugin load
  despike2d_init,  
  // query-function pointer, can be "NULL", 
  // called if present after plugin init to let plugin manage it install itself
  NULL, // query should be "NULL" for Gxsm-Math-Plugin !!!
  // about-function, can be "NULL"
  // can be called by "Plugin Details"
  despike2d_about,
  // configure-function, can be "NULL"
  // can be called by "Plugin Details"
  despike2d_configure,
  // run-function, can be "NULL", if non-Zero and no query defined, 
  // it is called on menupath->"plugin"
  NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
  // cleanup-function, can be "NULL"
  // called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

  despike2d_cleanup
};

// special math Plugin-Strucure, use
// GxsmMathOneSrcPlugin despike2d_m1s_pi -> "OneSrcMath"
// GxsmMathTwoSrcPlugin despike2d_m2s_pi -> "TwoSrcMath"
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
 GxsmMathOneSrcPlugin despike2d_m1s_pi
#else
 GxsmMathTwoSrcPlugin despike2d_m2s_pi
#endif
 = {
   // math-function to run, see prototype(s) above!!
   despike2d_run
 };

// Text used in Aboutbox, please update!!
static const char *about_text = N_("Gxsm Despike2d Plugin\n\n"
                                   "This Plugin does a despike2d action with data:\n"
				   "data = data - spike-detect;"
	);

double ConstLast = 0.;
double constval = 0.;

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
  despike2d_pi.description = g_strdup_printf(N_("Gxsm MathOneArg despike2d plugin %s"), VERSION);
  return &despike2d_pi; 
}

// Symbol "get_gxsm_math_one|two_src_plugin_info" is resolved by dlsym from Gxsm, 
// used to find out which Math Type the Plugin is!! 
// Essential Plugin Function!!
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
GxsmMathOneSrcPlugin *get_gxsm_math_one_src_plugin_info( void ) {
  return &despike2d_m1s_pi; 
}
#else
GxsmMathTwoSrcPlugin *get_gxsm_math_two_src_plugin_info( void ) { 
  return &despike2d_m2s_pi; 
}
#endif

/* Here we go... */
// init-Function
static void despike2d_init(void)
{
  PI_DEBUG (DBG_L2, "despike2d Plugin Init");
}

// about-Function
static void despike2d_about(void)
{
  const gchar *authors[] = { despike2d_pi.authors, NULL};
  gtk_show_about_dialog (NULL, 
			 "program-name",  despike2d_pi.name,
			 "version", VERSION,
			 "license", GTK_LICENSE_GPL_3_0,
			 "comments", about_text,
			 "authors", authors,
			 NULL
			 );
}

// configure-Function
static void despike2d_configure(void)
{
  if(despike2d_pi.app)
    despike2d_pi.app->message("Despike2d Plugin Configuration");
}

// cleanup-Function
static void despike2d_cleanup(void)
{
  PI_DEBUG (DBG_L2, "Despike2d Plugin Cleanup");
}


void despike_32650max (Mem2d *in, Mem2d *out){
	int i,j,k,l,za=0,nx,ny;
	int anz=1;
	int num=1;
	double reihe1[20],reihe2[20],mi;

        nx=out->GetNx();
        ny=out->GetNy();

        for (j=anz; j<ny-anz; ++j){
                for (i=anz; i<nx-anz; ++i) {
                        for (k=j-anz, l=0; k<=j+anz; k++,l++)
                                reihe1[l] = in->data->Z(i,k);
                        for (k=0; k<2*anz+1;k++)  {
                                mi = 32650;
                                for (l=0; l<2*anz+1;l++) {
                                        if (reihe1[l]<mi) {
                                                mi=reihe1[l];
                                                reihe2[k]=mi;
                                                za=l;
                                        }
                                }
                                reihe1[za]=32650;
                        }
                        out->data->Z(reihe2[num], i,j);
                }  /*i*/
        } /*j*/
}

void despike_d (Mem2d *in, Mem2d *out){
	int i,j,k,l,za=0,nx,ny;
	int anz=1;
	int num=1;
	double reihe1[20],reihe2[20],mi;

        nx=out->GetNx();
        ny=out->GetNy();

        double hi, lo;
        in->HiLo (&hi, &lo);
        
        for (j=anz; j<ny-anz; ++j){
                for (i=anz; i<nx-anz; ++i) {
                        for (k=j-anz, l=0; k<=j+anz; k++,l++)
                                reihe1[l] = in->data->Z(i,k);
                        for (k=0; k<2*anz+1;k++)  {
                                mi = hi;
                                for (l=0; l<2*anz+1;l++) {
                                        if (reihe1[l]<mi && reihe1[l] != 0.0) {
                                                mi=reihe1[l];
                                                reihe2[k]=mi;
                                                za=l;
                                        }
                                }
                                reihe1[za]=hi;
                        }
                        out->data->Z(reihe2[num], i,j);
                }  /*i*/
        } /*j*/
}


// run-Function
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
 static gboolean despike2d_run(Scan *Src, Scan *Dest)
#else
 static gboolean despike2d_run(Scan *Src1, Scan *Src2, Scan *Dest)
#endif
{
	int ti=0; 
	int tf=0;
	int vi=0;
	int vf=0;
	gboolean multidim = FALSE;
	
	if (Src->data.s.ntimes != 1 || Src->mem2d->GetNv () != 1){
		multidim = TRUE;
		do {
			ti=vi=0;
			tf=Src->number_of_time_elements ()-1;
			if (tf < 0) tf = 0;
			vf=Src->mem2d->GetNv ()-1;
			gapp->setup_multidimensional_data_copy ("Multidimensional Despike2d", Src, ti, tf, vi, vf);
		} while (ti > tf || vi > vf);

		gapp->progress_info_new ("Multidimenssional Despike2d", 2);
		gapp->progress_info_set_bar_fraction (0., 1);
		gapp->progress_info_set_bar_fraction (0., 2);
		gapp->progress_info_set_bar_text ("Time", 1);
		gapp->progress_info_set_bar_text ("Value", 2);
	}

	int ntimes_tmp = tf-ti+1;
	for (int time_index=ti; time_index <= tf; ++time_index){
		Mem2d *m = Src->mem2d_time_element (time_index);
		if (multidim)
			gapp->progress_info_set_bar_fraction ((gdouble)(time_index-ti)/(gdouble)ntimes_tmp, 1);

		Dest->mem2d->Resize (m->GetNx (), m->GetNy (), vf-vi+1, m->GetTyp());
		for (int v_index = vi; v_index <= vf; ++v_index){
			m->SetLayer (v_index);
			Dest->mem2d->SetLayer (v_index-vi);	

			Dest->mem2d->CopyFrom(m, 0,0, 0,0, 
					      Dest->mem2d->GetNx(), Dest->mem2d->GetNy());
                        //despike_32650max (m, Dest->mem2d);
                        despike_d (m, Dest->mem2d);

		}
		Dest->append_current_to_time_elements (time_index-ti, m->get_frame_time ());
	}

	Dest->data.s.ntimes = ntimes_tmp;
	Dest->data.s.nvalues=Dest->mem2d->GetNv ();

	if (multidim){
		gapp->progress_info_close ();
		Dest->retrieve_time_element (0);
		Dest->mem2d->SetLayer(0);
	}
	
	return MATH_OK;
}

