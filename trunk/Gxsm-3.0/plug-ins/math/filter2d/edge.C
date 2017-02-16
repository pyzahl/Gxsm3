/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * plugin_helper reports your answers as
author		=Percy Zahl
email	        	=zahl@users.sf.net
pluginname		=edge
pluginmenuentry 	=Edge
menupath		=Math/Filter 2D/
entryplace		=Filter 2D
shortentryplace	=F2D
abouttext		=Laplace of Gaussian edge detect filter using 2D convolution.
smallhelp		=gaus edge
longhelp		=Gausian edge
 * 
 * Gxsm Plugin Name: edge.C
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
% PlugInDocuCaption: Edge
% PlugInName: edge
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: Math/Filter 2D/Edge

% PlugInDescription
A 2D Laplace of Gaussian (LoG) edge detect filter kernel is calculated and applied via convolution to teh source data set, feature size $\sigma$:

\[ K_{ij} = -\frac{1}{\pi\sigma^4} \left( 1 - \frac{i^2+j^2}{2\sigma^2}\right) e^{-\frac{i^2+j^2}{2\sigma^2}} \]

% PlugInUsage
Call \GxsmMenu{Math/Filter 2D/Edge}.

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
#include "gxsm/plugin.h"
#include "../../common/pyremote.h"

// Plugin Prototypes
static void edge_init( void );
static void edge_about( void );
static void edge_configure( void );
static void edge_cleanup( void );

static gboolean edge_run_radius(Scan *Src, Scan *Dest);
static void edge_non_interactive( GtkWidget *widget , gpointer pc );

// Define Type of math plugin here, only one line should be commented in!!
#define GXSM_ONE_SRC_PLUGIN__DEF

// Math-Run-Function, use only one of (automatically done :=)
 static gboolean edge_run( Scan *Src, Scan *Dest );

// Fill in the GxsmPlugin Description here
GxsmPlugin edge_pi = {
  NULL,                   // filled in and used by Gxsm, don't touch !
  NULL,                   // filled in and used by Gxsm, don't touch !
  0,                      // filled in and used by Gxsm, don't touch !
  NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
                          // filled in here by Gxsm on Plugin load, 
                          // just after init() is called !!!
  // ----------------------------------------------------------------------
  // Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
  "edge-"
#ifdef GXSM_ONE_SRC_PLUGIN__DEF
  "M1S"
#else
  "M2S"
#endif
  "-F2D",
  // Plugin's Category - used to autodecide on Pluginloading or ignoring
  // NULL: load, else
  // example: "+noHARD +STM +AFM"
  // load only, if "+noHARD: no hardware" and Instrument is STM or AFM
  // +/-xxxHARD und (+/-INST or ...)
  NULL,
  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
  g_strdup("Edge filter (LoG)"),                   
  // Author(s)
  "Percy Zahl",
  // Menupath to position where it is appendet to
  "math-filter2d-section",
  // Menuentry
  N_("Edge"),
  // help text shown on menu
  N_("Edge filter (LoG)"),
  // more info...
  "Edge filter (LoG)",
  NULL,          // error msg, plugin may put error status msg here later
  NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
  // init-function pointer, can be "NULL", 
  // called if present at plugin load
  edge_init,  
  // query-function pointer, can be "NULL", 
  // called if present after plugin init to let plugin manage it install itself
  NULL, // query should be "NULL" for Gxsm-Math-Plugin !!!
  // about-function, can be "NULL"
  // can be called by "Plugin Details"
  edge_about,
  // configure-function, can be "NULL"
  // can be called by "Plugin Details"
  edge_configure,
  // run-function, can be "NULL", if non-Zero and no query defined, 
  // it is called on menupath->"plugin"
  NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
  // cleanup-function, can be "NULL"
  // called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

  edge_cleanup
};

// special math Plugin-Strucure, use
// GxsmMathOneSrcPlugin edge_m1s_pi -> "OneSrcMath"

 GxsmMathOneSrcPlugin edge_m1s_pi
 = {
   // math-function to run, see prototype(s) above!!
   edge_run
 };

// Text used in Aboutbox, please update!!
static const char *about_text = N_("Gxsm edge Plugin\n\n"
                                   "Edge filter (LoG) using a 2D convolution.");

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
  edge_pi.description = g_strdup_printf(N_("Gxsm MathOneArg edge plugin %s"), VERSION);
  return &edge_pi; 
}

// Symbol "get_gxsm_math_one|two_src_plugin_info" is resolved by dlsym from Gxsm, 
// if "get_gxsm_math_one_for_all_vt_plugin_info" is resolved by dlsym from Gxsm for operations on all layers (values) and times, run may get called multiple times!
// used to find out which Math Type the Plugin is!! 

// Essential Math Plugin Function!!

GxsmMathOneSrcPlugin *get_gxsm_math_one_src_for_all_vt_plugin_info( void ) {
  return &edge_m1s_pi; 
}


/* Here we go... */

double       edge_radius = 5.;
class MemEdgeKrn *edge_kernel=NULL;


// init-Function
static void edge_init(void)
{
  PI_DEBUG (DBG_L2, "edge Plugin Init");

// This is action remote stuff, stolen from the peak finder PI.
  remote_action_cb *ra;
  GtkWidget *dummywidget = gtk_menu_item_new();

  ra = g_new( remote_action_cb, 1);
  ra -> cmd = g_strdup_printf("edge_PI");
  ra -> RemoteCb = &edge_non_interactive;
  ra -> widget = dummywidget;
  ra -> data = NULL;
  gapp->RemoteActionList = g_slist_prepend ( gapp->RemoteActionList, ra );
  PI_DEBUG (DBG_L2, "edge-plugin: Adding new Remote Cmd: edge_PI");
// remote action stuff
}

static void edge_non_interactive( GtkWidget *widget , gpointer pc )
{
  PI_DEBUG (DBG_L2, "edge-plugin: edge is called from script.");

//  cout << "pc: " << ((gchar**)pc)[1] << endl;
//  cout << "pc: " << ((gchar**)pc)[2] << endl;
//  cout << "pc: " << atof(((gchar**)pc)[2]) << endl;

  gapp->xsm->MathOperation(&edge_run_radius);
  return;

}

// about-Function
static void edge_about(void)
{
  const gchar *authors[] = { edge_pi.authors, NULL};
  gtk_show_about_dialog (NULL,
			 "program-name",  edge_pi.name,
			 "version", VERSION,
			 "license", GTK_LICENSE_GPL_3_0,
			 "comments", about_text,
			 "authors", authors,
			 NULL
			 );
}

// configure-Function
static void edge_configure(void)
{
  if(edge_pi.app)
    edge_pi.app->message("edge Plugin Configuration");
}

// cleanup-Function
static void edge_cleanup(void)
{
	PI_DEBUG (DBG_L2, "edge Plugin Cleanup");
}

/*
 * special kernels for MemDigiFilter
 * Gausian Edge Kernel
 */

class MemEdgeKrn : public MemDigiFilter{
public:
	MemEdgeKrn (double xms, double xns, int m, int n):MemDigiFilter (xms, xns, m, n){};
	virtual gboolean CalcKernel (){
		int i,j;
		double sig2=xms*xns/4.; // set sigmal to "r/2"
		for (i= -m; i<=m; i++)
                        for (j = -n; j<=n; j++){
                                double r2 = j*j+i*i;
                                data->Z ((1.-r2/(2.*sig2))/(M_PI*sig2*sig2) * exp (-r2/(2*sig2)), j+n, i+m);
                        }
		data->norm ();
		return 0;
	};
};

void setup_multidimensional_data_copy (const gchar *title, Scan *src, int &ti, int &tf, int &vi, int &vf, int *crop_window_xy=NULL, gboolean crop=FALSE){
	UnitObj *Pixel = new UnitObj("Pix","Pix");
	UnitObj *Unity = new UnitObj(" "," ");
	GtkWidget *dialog = gtk_dialog_new_with_buttons (N_(title),
							 NULL, 
							 (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
							 _("_OK"), GTK_RESPONSE_ACCEPT,
							 NULL); 
	BuildParam bp;
        gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), bp.grid);

        bp.grid_add_label ("New Time and Layer bounds:"); bp.new_line ();

	gint t_max = src->number_of_time_elements ()-1;
	if (t_max < 0) t_max=0;

        bp.grid_add_ec ("t-inintial",  Unity, &ti, 0, t_max, ".0f"); bp.new_line ();
        bp.grid_add_ec ("t-final",     Unity, &tf, 0, t_max, ".0f"); bp.new_line ();
#if 0
	if (tnadd){
		bp.grid_add_ec ("t-#add",  Unity, tnadd, 1, t_max, ".0f"); bp.new_line ();
	}
#endif
        bp.grid_add_ec ("lv-inintial", Unity, &vi, 0, src->mem2d->GetNv (), ".0f"); bp.new_line ();
        bp.grid_add_ec ("lv-final",    Unity, &vf, 0, src->mem2d->GetNv (), ".0f"); bp.new_line ();

#if 0
	if (vnadd){
		bp.grid_add_ec ("lv-#add",  Unity, vnadd, 1, src->mem2d->GetNv (), ".0f"); bp.new_line ();
	}
#endif

	if (crop && crop_window_xy){
                bp.grid_add_label ("Crop window bounds:"); bp.new_line ();
                bp.grid_add_ec ("X-left",  Pixel, &crop_window_xy[0], 0, src->mem2d->GetNx ()-1, ".0f"); bp.new_line ();
                bp.grid_add_ec ("Y-top",   Pixel, &crop_window_xy[1], 0, src->mem2d->GetNx ()-1, ".0f"); bp.new_line ();
                bp.grid_add_ec ("X-right", Pixel, &crop_window_xy[2], 0, src->mem2d->GetNx ()-1, ".0f"); bp.new_line ();
                bp.grid_add_ec ("Y-bottom",Pixel, &crop_window_xy[3], 0, src->mem2d->GetNx ()-1, ".0f"); bp.new_line ();
	}

	gtk_widget_show_all (dialog);
	gtk_dialog_run (GTK_DIALOG(dialog));

	gtk_widget_destroy (dialog);

	delete Pixel;
	delete Unity;	
}


// run-Function
static gboolean edge_run___for_all_vt(Scan *Src, Scan *Dest)
{
	double r = 5.;    // Get Radius
	gapp->ValueRequest("2D Convol. Filter Size", "Radius", "Edge kernel size: s = 1+radius  LoG detect sigma=r/2",
			   gapp->xsm->Unity, 0., Src->mem2d->GetNx()/10., ".0f", &r);

	int    s = 1+(int)(r + .9); // calc. approx Matrix Radius
	MemEdgeKrn krn(r,r, s,s); // Setup Kernelobject



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
			setup_multidimensional_data_copy ("Multidimensional Edge", Src, ti, tf, vi, vf);
		} while (ti > tf || vi > vf);

		gapp->progress_info_new ("Multidimenssional Edge", 2);
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

			krn.Convolve(m, Dest->mem2d); // Do calculation of Kernel and then do convolution

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




// run-Function -- may gets called for all layers and time if multi dim scan data is present!
static gboolean edge_run(Scan *Src, Scan *Dest)
{

// check for multi dim calls, make sure not to ask user for paramters for every layer or time step!
	if (((Src ? Src->mem2d->get_t_index ():0) == 0 && (Src ? Src->mem2d->GetLayer ():0) == 0) || !edge_kernel) {
		double r = edge_radius;    // Get Radius
		gapp->ValueRequest("2D Convol. Filter Size", "Radius", "Edge kernel size: s = 1+radius  LoG detect sigma=r/2",
				   gapp->xsm->Unity, 0., Src->mem2d->GetNx()/10., ".0f", &r);
		edge_radius = r;
		if (edge_kernel)
			free (edge_kernel);

		int    s = 1+(int)(r + .9); // calc. approx Matrix Radius
		edge_kernel = new MemEdgeKrn (r,r, s,s); // calc. convol kernel

		if (!Src || !Dest)
			return 0;
	}	

	edge_kernel->Convolve (Src->mem2d, Dest->mem2d); // do convolution

	return MATH_OK;
}



static gboolean edge_run_radius(Scan *Src, Scan *Dest)
{
	// equals edge-run except ValueRequest
	double r = 5.0;    // Get Radius
	gapp->ValueRequest("2D Convol. Filter Size", "Radius", "Edge kernel size: s = 1+radius  LoG detect sigma=r/2",
			   gapp->xsm->Unity, 0., Src->mem2d->GetNx()/10., ".0f", &r);

	int    s = 1+(int)(r + .9); // calc. approx Matrix Radius
	MemEdgeKrn krn(r,r, s,s); // Setup Kernelobject

	krn.Convolve(Src->mem2d, Dest->mem2d); // Do calculation of Kernel and then do convolution

	return MATH_OK;
}

