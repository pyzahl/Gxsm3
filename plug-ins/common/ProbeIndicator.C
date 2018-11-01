/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Gxsm Plugin Name: ProbeIndicator.C
 * ========================================
 * 
 * Copyright (C) 2018 The Free Software Foundation
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
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
 * --------------------------------------------------------------------------------
% BeginPlugInDocuSection
% PlugInDocuCaption: Probe Indicator

% PlugInName: ProbeIndicator
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: Tools/Probe Indicator
% PlugInDescription
 This is a handy tool which shows you, where your current scan is in the
 range of the maximum scan. Especially it gives you an error message if you
 leave the scan area. It also shows the position of rotated scans.

 In addition the current realtime XY position of the tip plus it
 indicates the Z-position of the tip is visualized by the marker on
 the right edge of the window.

\begin{figure}[hbt]
\center { \fighalf{ProbeIndicator}}
\caption{The ProbeIndicator window.}
\label{fig:ProbeIndicator}
\end{figure}


% PlugInUsage
 Although this is a plugin it is opened automatically upon startup of GXSM.
 There is not interaction with the user.

\begin{figure}[hbt]
\center { \fighalf{ProbeIndicator_indicators}}
\caption{The indicators of the ProbeIndicator window.}
\label{fig:ProbeIndicator}
\end{figure}

 1.) Indicator of the state machine on the DSP. In general the colors 
 indicate green=ON/in progress, red=OFF/inactive. From left to right
 the boxed indicate the status of the feedback, scan in progress, 
 vector proce in progress, mover in progress (coarse approach)

 2.) Indicators of the 8 GPIO channels. They can be read on/off 
 (red/black) or write on/off (green/white) giving you four possible 
 states.

 3.) Indicator of the Z position (Z-offset/z-scan)

% OptPlugInNotes
 The tip-position is close to realtime, but it is refreshed only
 several times per second and only if GXSM is idle. Thus the
 display/update may get stuck at times GXSM is very busy. Never the
 less the tip position is read back from the DSP and thus indicates
 the true position, regardless what in happening to GXSM of the DSP --
 so if anything goes wrong, you will see it here!

% OptPlugInHints
 For now the plug-in assumes a scan which is centered in the middle of the
 scan not in the middle of the topline (as default for the pci32).

% OptPlugInKnownBugs
 None

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */
#include "config.h"
#include "gxsm/plugin.h"
#include "glbvars.h"
#include "xsmtypes.h"

#include "gxsm_app.h"
#include "gxsm_window.h"

#include "ProbeIndicator.h"

#define UTF8_ANGSTROEM "\303\205"
#define UTF8_DEGREE "\302\260"

// Plugin Prototypes
static void ProbeIndicator_init (void);
static void ProbeIndicator_query (void);
static void ProbeIndicator_about (void);
static void ProbeIndicator_configure (void);
static void ProbeIndicator_cleanup (void);

ProbeIndicator *HUD_Window = NULL;
gboolean refresh_function(GtkWidget *w, GdkEvent *event, void *data);

gboolean ProbeIndicator_valid = FALSE;


static void ProbeIndicator_show_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data);

      // Fill in the GxsmPlugin Description here
GxsmPlugin ProbeIndicator_pi = {
	  NULL,                   // filled in and used by Gxsm, don't touch !
	  NULL,                   // filled in and used by Gxsm, don't touch !
	  0,                      // filled in and used by Gxsm, don't touch !
	  NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is
	  // filled in here by Gxsm on Plugin load,
	  // just after init() is called !!!
	  // ----------------------------------------------------------------------
	  // Plugins Name, CodeStly is like: Name-M1S[ND]|M2S-BG|F1D|F2D|ST|TR|Misc
	  (char *)"Probe Indicator",
	  NULL,
	  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
	  (char *)"Scan Area visualization and life SPM parameter readings",
	  // Author(s)
	  (char *) "Percy Zahl",
	  // Menupath to position where it is appended to
	  (char *)"windows-section",
	  // Menuentry
	  N_("Probe Indicator"),
	  // help text shown on menu
	  N_("Probe Status visualization."),
	  // more info...
	  (char *)"See Manual.",
	  NULL,          // error msg, plugin may put error status msg here later
	  NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
	  // init-function pointer, can be "NULL",
	  // called if present at plugin load
	  ProbeIndicator_init,
	  // query-function pointer, can be "NULL",
	  // called if present after plugin init to let plugin manage it install itself
	  ProbeIndicator_query, // query should be "NULL" for Gxsm-Math-Plugin !!!
	  // about-function, can be "NULL"
	  // can be called by "Plugin Details"
	  ProbeIndicator_about,
	  // configure-function, can be "NULL"
	  // can be called by "Plugin Details"
	  ProbeIndicator_configure,
	  // run-function, can be "NULL", if non-Zero and no query defined,
	  // it is called on menupath->"plugin"
	  NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
	  // cleanup-function, can be "NULL"
	  // called if present at plugin removal
          ProbeIndicator_show_callback, // direct menu entry callback1 or NULL
          NULL, // direct menu entry callback2 or NULL

	  ProbeIndicator_cleanup
};

//
//Text used in the About Box
//
static const char *about_text = N_("HUD style probe status indicator");
                                   
	
//
// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
//
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
	ProbeIndicator_pi.description = g_strdup_printf(N_("Gxsm HUD probe indicator window %s"), VERSION);
	return &ProbeIndicator_pi; 
}

//
// init-Function
//

static void ProbeIndicator_show_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
        PI_DEBUG (DBG_L2, "ProbeIndicator_show_callback" );
	if( HUD_Window ){
                HUD_Window->show();
		HUD_Window->start();
	}
}

static void ProbeIndicator_refresh_callback (gpointer data){
	if (HUD_Window)
		HUD_Window->refresh ();
}

static void ProbeIndicator_init(void)
{
	PI_DEBUG(DBG_L2, "ProbeIndicator Plugin Init" );
}

// Query Function, installs Plugin's in File/Import and Export Menupaths!
// ----------------------------------------------------------------------
// Import Menupath is "File/Import/abc import"
// Export Menupath is "File/Export/abc import"
// ----------------------------------------------------------------------
// !!!! make sure the "spm_scancontrol_cleanup()" function (see below) !!!!
// !!!! removes the correct menuentries !!!!

// Add Menu Entries:
// windows-sectionSPM Scan Control
// Action/SPM Scan Start/Pause/Stop + Toolbar

static void ProbeIndicator_query(void)
{
        PI_DEBUG (DBG_L2, "ProbeIndicator_query" );

	if(ProbeIndicator_pi.status) g_free(ProbeIndicator_pi.status); 
	ProbeIndicator_pi.status = g_strconcat (
		N_("Plugin query has attached, status:"),
		(xsmres.HardwareType[0] == 'n' && xsmres.HardwareType[1] == 'o')?"Offline (no hardware)":"Online",
		ProbeIndicator_pi.name, 
		NULL);
	
	HUD_Window = new ProbeIndicator(); // ProbeIndicator(ProbeIndicator_pi.app->getApp());
	ProbeIndicator_pi.app->ConnectPluginToSPMRangeEvent (ProbeIndicator_refresh_callback);
	HUD_Window->start ();

// not yet needed
//	ProbeIndicator_pi.app->ConnectPluginToCDFSaveEvent (ProbeIndicator_SaveValues_callback);

	PI_DEBUG (DBG_L2, "ProbeIndicator_query:done" );
}

//
// about-Function
//
static void ProbeIndicator_about(void)
{
	const gchar *authors[] = { ProbeIndicator_pi.authors, NULL};
	gtk_show_about_dialog (NULL, 
			       "program-name",  ProbeIndicator_pi.name,
			       "version", VERSION,
			       "license", GTK_LICENSE_GPL_3_0,
			       "comments", about_text,
			       "authors", authors,
			       NULL
			       );
}
//
// configure-Function
//
static void ProbeIndicator_configure(void)
{
        PI_DEBUG (DBG_L2, "ProbeIndicator_configure" );
	if(ProbeIndicator_pi.app)
		ProbeIndicator_pi.app->message("ProbeIndicator Plugin Configuration");

	if (HUD_Window){
                HUD_Window->start ();
	}
}

//
// cleanup-Function, make sure the Menustrings are matching those above!!!
//
static void ProbeIndicator_cleanup(void)
{
	ProbeIndicator_valid = FALSE;

	PI_DEBUG(DBG_L2, "ProbeIndicator_cleanup(void). Entering.\n");
	if (HUD_Window) {
		delete HUD_Window;
	}
	PI_DEBUG(DBG_L2, "ProbeIndicator_cleanup(void). Exiting.\n");
}

static gint ProbeIndicator_tip_refresh_callback (ProbeIndicator *pv){
	if (!gapp->xsm->hardware)
		return TRUE;

	if (gapp->xsm->hardware->IsSuspendWatches ())
		return TRUE;

	if (pv){
                return pv->refresh ();
	}
	return FALSE;
}


ProbeIndicator::ProbeIndicator (){ 
        hud_size = 150;
	timer_id = 0;
	probe = NULL;
        modes = SCOPE_ON;
        
	AppWindowInit (N_("HUD Probe Indicator"));

	canvas = gtk_drawing_area_new(); // make a drawing area

        gtk_widget_add_events (canvas,
                               GDK_BUTTON_PRESS_MASK
                               | GDK_ENTER_NOTIFY_MASK      
                               | GDK_LEAVE_NOTIFY_MASK  
                               | GDK_POINTER_MOTION_MASK
                               );

        /* Event signals */
        g_signal_connect (G_OBJECT (canvas), "event",
                          G_CALLBACK (ProbeIndicator::canvas_event_cb), this);


        
        g_signal_connect (G_OBJECT (canvas), "draw",
			  G_CALLBACK (ProbeIndicator::canvas_draw_callback), this);
		
	gtk_widget_set_size_request (canvas, 2*hud_size, 2*hud_size);

	gtk_widget_show (canvas);
	gtk_grid_attach (GTK_GRID (v_grid), canvas, 1,1, 10,10);

        probe = new hud_object();
        probe->add_tics ("T1", 0, 1., 9, 25.);
        probe->add_tics ("T1", 240, 1., 13, 10.);
        
        tip=probe->add_mark ("MT", 200, 0.);
        probe->add_mark ("MB", 0, 0.7);

        m1=probe->add_mark ("M1", 0, 0., 1, -1, 0.75);
        m2=probe->add_mark ("M2", 0, 0., 1, -1, 0.75);
        probe->set_mark_color (m1, CAIRO_COLOR_RED_ID);
        probe->set_mark_color (m2, CAIRO_COLOR_RED_ID);
        
        ipos=probe->add_indicator ("IPos", 100.0, 50., 0, 2);
        ipos2=probe->add_indicator ("IPos", 100.0, 50., 1, 2);
        ineg=probe->add_indicator ("INeg", 100.0, -50., 0, 2);
        //ineg2=probe->add_indicator ("INeg", 100.0, -50., 1, 2);

        fpos=probe->add_indicator ("IPos", 300.0, 25., 0, 2);
        fpos2=probe->add_indicator ("IPos", 300.0, 20., 1, 2);
        fneg=probe->add_indicator ("INeg", 300.0, -12., 0, 2);
        fneg2=probe->add_indicator ("INeg", 300.0, -5., 1, 2);


        horizon[0]=probe->add_horizon ("H0", 0.0, 0.0, 128);
        probe->set_horizon_color(horizon[0], CAIRO_COLOR_RED_ID);

        horizon[1]=probe->add_horizon ("H1", 0.0, 0.0, 128);

        horizon[2]=probe->add_horizon ("H2", 0.0, 0.0, 128);
        probe->set_horizon_color(horizon[2], CAIRO_COLOR_RED_ID);

        
	//probe->queue_update (canvas);
        
	info = new cairo_item_text (60.0, 35.0, "Probe HUD");
        //info->set_text ("Probe HUD")
	info->set_stroke_rgba (CAIRO_COLOR_BLUE);
	info->set_font_face_size ("Ununtu", 12.);
	info->set_spacing (-.1);
	info->set_anchor (CAIRO_ANCHOR_E);
	//info->queue_update (canvas);

	refresh ();
}

ProbeIndicator::~ProbeIndicator (){
	ProbeIndicator_valid = FALSE;

        PI_DEBUG (DBG_L4, "ProbeIndicator::~ProbeIndicator -- stop_tip_monitor RTQuery");

	stop ();
        run_fft (0, NULL, NULL, 0.,0.);

        UNREF_DELETE_CAIRO_ITEM (probe, canvas);
        UNREF_DELETE_CAIRO_ITEM (info, canvas);

	PI_DEBUG (DBG_L4, "ProbeIndicator::~ProbeIndicator () -- done.");
}

void ProbeIndicator::AppWindowInit(const gchar *title){
        PI_DEBUG (DBG_L2, "DSPMoverControl::AppWindowInit -- header bar");

        app_window = gxsm3_app_window_new (GXSM3_APP (gapp->get_application ()));
        window = GTK_WINDOW (app_window);

	gtk_window_set_default_size (GTK_WINDOW(window), 2*hud_size, 2*hud_size);
	gtk_window_set_title (GTK_WINDOW(window), title);
	gtk_widget_set_opacity (GTK_WIDGET(window), 1.0);
	gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	gtk_window_set_decorated (GTK_WINDOW(window), FALSE);
	gtk_window_set_keep_above (GTK_WINDOW(window), TRUE);
	//gtk_window_stick (GTK_WINDOW(window));
        
	v_grid = gtk_grid_new ();
        gtk_container_add (GTK_CONTAINER (window), v_grid);
	g_object_set_data (G_OBJECT (window), "v_grid", v_grid);
	gtk_widget_show_all (GTK_WIDGET (window));

	set_window_geometry ("probe-hud");
}

gint ProbeIndicator::canvas_event_cb(GtkWidget *canvas, GdkEvent *event, ProbeIndicator *pv){
	static int dragging=FALSE;
	static GtkWidget *coordpopup=NULL;
	static GtkWidget *coordlab=NULL;
        static int pi=-1;
        static int pj=-1;
        double mouse_pix_xy[2];
        static double preset[2];
        
        //---------------------------------------------------------
	// cairo_translate (cr, 12.+pv->WXS/2., 12.+pv->WYS/2.);
        // scale to volate range
	// cairo_scale (cr, 0.5*pv->WXS/pv->max_x, -0.5*pv->WYS/pv->max_y);

        // undo cairo image translation/scale:
        mouse_pix_xy[0] = event->button.x-pv->hud_size; //(event->button.x - (double)(12+pv->WXS/2.))/( 0.5*pv->WXS/pv->max_x);
        mouse_pix_xy[1] = event->button.y-pv->hud_size; //(event->button.y - (double)(12+pv->WYS/2.))/( 0.5*pv->WYS/pv->max_y);


        switch (event->type) {
	case GDK_BUTTON_PRESS:
		switch(event->button.button) {
		case 1:
                        //g_object_set_data (G_OBJECT(canvas), "preset_xy", preset);
                        //gapp->offset_to_preset_callback (canvas, gapp);
                        g_message ("ProbeIndicator Button1 Pressed at XY=%g, %g",  mouse_pix_xy[0]);
                        if (mouse_pix_xy[0] > 0)
                                pv->modes = (pv->modes & ~SCOPE_ON) | SCOPE_ON;
                        else
                                pv->modes &= ~SCOPE_ON;

                        if (mouse_pix_xy[1] < 0)
                                pv->modes = (pv->modes & ~SCOPE_DBG) | SCOPE_DBG;
                        else
                                pv->modes &= ~SCOPE_DBG;
			break;
                }
                break;
		
	case GDK_MOTION_NOTIFY:
		break;
		
	case GDK_ENTER_NOTIFY:
                //pv->show_preset_grid = true;
		break;
	case GDK_LEAVE_NOTIFY:
                //pv->show_preset_grid = false;
                break;
		
	default: break;
	}
        
	return FALSE;
}

gboolean  ProbeIndicator::canvas_draw_callback (GtkWidget *widget, cairo_t *cr, ProbeIndicator *pv){
        // translate origin to window center
	cairo_translate (cr, pv->hud_size, pv->hud_size);
        cairo_save (cr);

        // scale to volate range
	//cairo_scale (cr, 0.5*pv->WXS/pv->max_x, -0.5*pv->WYS/pv->max_y);

	pv->probe->draw (cr);         // pan area
	pv->info->draw (cr);

        cairo_restore (cr);
        return TRUE;
}
 

void ProbeIndicator::start (){
	if (!timer_id){
		PI_DEBUG (DBG_L1, "ProbeIndicator::start_tip_monitor \n");
		timer_id = g_timeout_add (150, (GSourceFunc) ProbeIndicator_tip_refresh_callback, this);
	}
}

void ProbeIndicator::stop (){
	if (timer_id){
		PI_DEBUG (DBG_L1, "ProbeIndicator::stop_tip_monitor \n");
		timer_id = 0;
	}
	PI_DEBUG (DBG_L1, "ProbeIndicator::stop_tip_monitor OK.\n");
}


gint ProbeIndicator::refresh(){
        #define SCOPE_N 4096
        static gfloat scope[4][SCOPE_N+1];
        static gfloat scope_min[4];
        static gfloat scope_max[4];
        static gint busy=FALSE;
        static double tics=0.;
        
        if (busy) return FALSE;

	double x,y,z,q,Ilg, Ilgmp, Ilgmi;
        double max_z = xsmres.AnalogVMaxOut*gapp->xsm->Inst->VZ();

        busy = TRUE;
	x=y=z=q=0.;
        
        if (gapp->xsm->hardware)
                gapp->xsm->hardware->RTQuery ("zxy", z, x, y); // get tip position in volts
        else {
                busy = FALSE;
                return FALSE;
        }
        
        probe->set_mark_len (tip, z/max_z);
        if (fabs(z/max_z) < 0.8)
                probe->set_mark_color (tip, -1);
        else
                probe->set_mark_color (tip, CAIRO_COLOR_RED_ID);

	if (gapp->xsm->hardware){
		double x0,y0,z0;
#if 0
		if (gapp->xsm->hardware->RTQuery ("O", z0, x0, y0)){ // get HR Offset
			gchar *tmp = NULL;
			tmp = g_strdup_printf ("Offset Z0: %7.3f " UTF8_ANGSTROEM
                                               "\nXY0: %7.3f " UTF8_ANGSTROEM
                                               ", %7.3f " UTF8_ANGSTROEM
					       "\nXYs: %7.3f " UTF8_ANGSTROEM
                                               ", %7.3f " UTF8_ANGSTROEM,
					       gapp->xsm->Inst->V2ZAng(z0),
					       gapp->xsm->Inst->V2XAng(x0),
					       gapp->xsm->Inst->V2YAng(y0),
					       gapp->xsm->Inst->V2XAng(x),
					       gapp->xsm->Inst->V2YAng(y));
                        infoXY0->set_text (tmp);
                        //infoXY0->queue_update (canvas);
			g_free (tmp);

			// Z0 position marker
			rsz = WXS/2./10.;
                        if (tip_marker_z0 == NULL){
                                tip_marker_z0 = new cairo_item_path (2);
                                tip_marker_z0->set_xy (0, +rsz, 0.);
                                tip_marker_z0->set_xy (1, -rsz, 0.);
                                tip_marker_z0->set_fill_rgba (CAIRO_COLOR_BLUE_ID, 0.8);
                                tip_marker_z0->set_stroke_rgba (CAIRO_COLOR_BLUE_ID, 0.8);
                                tip_marker_z0->set_line_width (get_lw (3.0));
                        }
                        // XY Scan&Offset position marker
                        tip_marker_z0->set_position (WXS/2., WYS/2.*z0/z0r);
                        tip_marker_z0->set_line_width ( WYS/2.*0.03);
                        if (fabs(z/z0r) < 0.75)
                                tip_marker_z0->set_stroke_rgba (CAIRO_COLOR_BLUE_ID, 0.8);
                        else
                                tip_marker_z0->set_stroke_rgba (CAIRO_COLOR_RED);
                        //tip_marker_z0->queue_update (canvas); // schedule update
		}
#endif

                // Life Paramater Info
		gapp->xsm->hardware->RTQuery ("f0I", x, y, q); // get f0, I -- val1,2,3=[fo,Iav,Irms]

                // Freq
                if (fabs(x) < 200.){
                        probe->set_indicator_val (fpos2, 300.0, x > 0. ? x:0.);
                        probe->set_indicator_val (fneg2, 300.0, x < 0. ? x:0.);
                } else {
                        probe->set_indicator_val (fpos2, 300.0, 0.);
                        probe->set_indicator_val (fneg2, 300.0, 0.);
                }
                if (fabs(x) > 10.)
                        x *= 0.1;
                if (fabs(x) > 10.)
                        x = 10.*x/fabs(x);
                probe->set_indicator_val (fpos,  300.0, x > 0.? 10.*x : 0.);
                probe->set_indicator_val (fneg,  300.0, x < 0.? 10.*x : 0.);
              
                // Current
                Ilg = log10 (fabs(1000.*y) + 1.0);
                probe->set_indicator_val (ipos,  100.0, y > 0.? 25.*Ilg : 0.);
                probe->set_indicator_val (ineg,  100.0, y < 0.? -25.*Ilg : 0.);

                if (modes & SCOPE_ON){
                        gapp->xsm->hardware->RTQuery ("S1", SCOPE_N, &scope[0][0]); // RT Query S1
                        gapp->xsm->hardware->RTQuery ("S2", SCOPE_N, &scope[1][0]); // RT Query S2
                        gapp->xsm->hardware->RTQuery ("T",  SCOPE_N, NULL); // RT Query, Trigger next
                }
                gfloat xmax, xmin, xrms;
                gint dec=SCOPE_N/128;
                xmax=xmin=scope[0][0]*dec;
                xrms=0.;
                int k=0;
                gfloat xr,xc;
                xc = 0.5*(scope_max[0]+scope_min[0]);
                xr = scope_max[0]-scope_min[0];
                
                for(int i=0; i<128; ++i, tics+=1./128.){
                        gfloat x=0.;
                        for (int j=0; j<dec; ++j, ++k){
                                x += scope[0][k];
                                xrms += scope[0][k]*scope[0][k];
                        }
                        // x /= dec; // no need as auto scaled regardless
                        if (x>xmax)
                                xmax = x;
                        if (x<xmin)
                                xmin = x;
                      
                        horizon[0]->set_xy (i, i-64., -32.*(x-xc)/xr);
                }
                xrms /= SCOPE_N;
                xrms = sqrt(xrms);
                scope_max[0] = 0.9*scope_max[0] + 0.1*xmax;
                scope_min[0] = 0.9*scope_min[0] + 0.1*xmin;
               
                Ilgmp = log10 (fabs(1000.*scope_max[0]/dec / gapp->xsm->Inst->nAmpere2V(1.)) + 1.0);
                Ilgmi = log10 (fabs(1000.*scope_min[0]/dec / gapp->xsm->Inst->nAmpere2V(1.)) + 1.0);
                double upper=25.*(scope_max[0] > 0.? Ilgmp : -Ilgmp);
                double lower=25.*(scope_min[0] > 0.? Ilgmi : -Ilgmi);
                probe->set_indicator_val (ipos2, 100.+upper, lower-upper);
                probe->set_mark_pos (m1,  upper);
                probe->set_mark_pos (m2,  lower);

                k=0;
                //run_fft (SCOPE_N+1, &scope[0][0], &scope[2][0], 1e-1810./32768, 10., 1.0);
                run_fft (SCOPE_N+1, &scope[0][0], &scope[2][0], 1e-18, 10., 1.0);
                for(int i=0; i<128; ++i, tics+=1./128.){
                        gfloat xr,xc;
                        gfloat x=0.;
                        for (int j=0; j<dec/2; ++j, ++k){
                                if (x > scope[2][k])
                                        x = scope[2][k];
                                //x += scope[2][k];
                        }
                        //x /= dec;
                        //g_print ("%g ", x);
                        horizon[2]->set_xy (i, i-64., -32.*x/1000.); // x: 0..-70db
                }

                
                xmax=xmin=scope[1][0]*dec;
                k=0;
                xc = 0.5*(scope_max[1]+scope_min[1]);
                xr = scope_max[1]-scope_min[1];
                for(int i=0; i<128; ++i, tics+=1./128.){
                        gfloat xr,xc;
                        gfloat x=0.;
                        for (int j=0; j<dec; ++j, ++k)
                                x += scope[1][k];

                        x /= dec;
                        x = gapp->xsm->Inst->V2ZAng(x);
                        
                        if (x>xmax)
                                xmax = x;
                        if (x<xmin)
                                xmin = x;

                        horizon[1]->set_xy (i, i-64., 32.*(x-xc)/20.); // xr is too jumpy ... fixed 20A
                }
                scope_max[1] = 0.9*scope_max[1] + 0.1*xmax;
                scope_min[1] = 0.9*scope_min[1] + 0.1*xmin;


                gchar *tmp = NULL;
                if (modes & SCOPE_ON)
                        y = xrms/gapp->xsm->Inst->nAmpere2V(1.);
                if (modes & SCOPE_DBG){
                        if (fabs(y) < 0.25)
                                tmp = g_strdup_printf ("I: %8.1f pA\ndF: %8.1f Hz\nZ: %8.4f" UTF8_ANGSTROEM "\n%g : %g\n%g : %g",
                                                       y*1000., x,
                                                       scope_min[0]/dec, scope_max[0]/dec,
                                                       scope_min[1]/dec, scope_max[1]/dec
                                                       );
                        else
                                tmp = g_strdup_printf ("I: %8.1f nA\ndF: %8.1f Hz\nZ: %8.4f" UTF8_ANGSTROEM "\n%g : %g\n%g : %g",
                                                       y, x,
                                                       scope_min[0]/dec, scope_max[0]/dec,
                                                       scope_min[1]/dec, scope_max[1]/dec
                                                       );
                        //tmp = g_strdup_printf ("I: %8.4f nA\ndF: %8.1f Hz\nZ: %8.4f" UTF8_ANGSTROEM,
                        //                               y, x); //  "\n%g:%g", gapp->xsm->Inst->V2ZAng(z), scope_min[0]/dec,scope_max[0]/dec);
                } else {
                        if (fabs(y) < 0.25)
                                tmp = g_strdup_printf ("I: %8.1f pA\ndF: %8.1f Hz\nZ: %8.4f" UTF8_ANGSTROEM, y*1000., x);
                        else
                                tmp = g_strdup_printf ("I: %8.4f nA\ndF: %8.1f Hz\nZ: %8.4f" UTF8_ANGSTROEM, y, x);
                }
                info->set_text (tmp);
                g_free (tmp);
                info->queue_update (canvas);
	}

        probe->queue_update (canvas);
        
        busy = FALSE;
	return TRUE;
}

////////////////////////////////////////
// ENDE PROBE INDCATOR   ///////////////
////////////////////////////////////////
