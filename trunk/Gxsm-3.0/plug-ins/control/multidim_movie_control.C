/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Gxsm Plugin Name: Multidim_Movie_Control.C
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
% PlugInModuleIgnore

% BeginPlugInDocuSection
% PlugInDocuCaption: Multi Dimensional Movie Control
% PlugInName: Multidim_Movie_Control
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: windows-sectionMovie Control

% PlugInDescription
Convenience panel for Multi Dimensional Movie Control and playback.

\GxsmScreenShot{Multidim_Movie_Control}{The CCD Control window.}

%% PlugInUsage
%Write how to use it.

%% OptPlugInConfig
%describe the configuration options of your plug in here!

%% OptPlugInRefs

%% OptPlugInKnownBugs

%% OptPlugInNotes
%If you have any additional notes

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */


#include <gtk/gtk.h>
#include "config.h"
#include "gxsm/plugin.h"

#include "gxsm/unit.h"
#include "gxsm/pcs.h"
#include "gxsm/glbvars.h"
#include "plug-ins/hard/modules/ccd.h"

#include "include/dsp-pci32/xsm/xsmcmd.h"



static void Multidim_Movie_Control_about( void );
static void Multidim_Movie_Control_query( void );
static void Multidim_Movie_Control_cleanup( void );

static void Multidim_Movie_Control_show_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data);
static void Multidim_Movie_Control_StartScan_callback( gpointer );

GxsmPlugin Multidim_Movie_Control_pi = {
	NULL,
	NULL,
	0,
	NULL,
	"Multidim_Movie_Control",
	NULL,
	NULL,
	"Percy Zahl",
	"windows-section",
	N_("Movie Control"),
	N_("open the multi dim movie control panel"),
	"Multi Dimensional Movie Control Panel",
	NULL,
	NULL,
	NULL,
	Multidim_Movie_Control_query,
	// about-function, can be "NULL"
	// can be called by "Plugin Details"
 	Multidim_Movie_Control_about,
	// configure-function, can be "NULL"
	// can be called by "Plugin Details"
	NULL,
	// run-function, can be "NULL", if non-Zero and no query defined, 
	// it is called on menupath->"plugin"
	NULL,
	Multidim_Movie_Control_show_callback, // direct menu entry callback1 or NULL
	NULL, // direct menu entry callback2 or NULL
	// cleanup-function, can be "NULL"
	// called if present at plugin removal
	Multidim_Movie_Control_cleanup
};

static const char *about_text = N_("Gxsm Multidim_Movie_Control Plugin:\n"
				   "This plugin opens a control panel for "
				   "multidimensional movie navigation and playback."
	);

GxsmPlugin *get_gxsm_plugin_info ( void ){ 
	Multidim_Movie_Control_pi.description = g_strdup_printf(N_("Gxsm Multidim_Movie_Control plugin %s"), VERSION);
	return &Multidim_Movie_Control_pi; 
}


class Multidim_Movie_Control : public AppBase{
public:
	Multidim_Movie_Control();
	virtual ~Multidim_Movie_Control();

	void update();
	static void l_play (GtkWidget *w, Multidim_Movie_Control *mmc);
	static void l_stop (GtkWidget *w, Multidim_Movie_Control *mmc);
	static void l_rewind (GtkWidget *w, Multidim_Movie_Control *mmc);
	static void t_play (GtkWidget *w, Multidim_Movie_Control *mmc);
	static void t_stop (GtkWidget *w, Multidim_Movie_Control *mmc);
	static void t_rewind (GtkWidget *w, Multidim_Movie_Control *mmc);
private:
	gboolean stop_play_layer;
	gboolean stop_play_time;
	double frame_delay;

	UnitObj *Unity, *Time;
	GtkWidget *contineous_autodisp;
public:
	gboolean t_play_flg;
	gboolean l_play_flg;
	int play_direction;
	GtkWidget *play_mode;
};


Multidim_Movie_Control *Multidim_Movie_ControlClass = NULL;
Multidim_Movie_Control *this_movie_control=NULL;

static void MovieControl_Freeze_callback (gpointer x){	 
         if (this_movie_control)	 
                 this_movie_control->freeze ();	 
}	 
static void MovieControl_Thaw_callback (gpointer x){	 
	if (this_movie_control)	 
		this_movie_control->thaw ();	 
}


static void Multidim_Movie_Control_query(void)
{
	// new ...
	Multidim_Movie_ControlClass = new Multidim_Movie_Control;

	Multidim_Movie_Control_pi.app->ConnectPluginToStartScanEvent
		( Multidim_Movie_Control_StartScan_callback );

	Multidim_Movie_Control_pi.status = g_strconcat(N_("Plugin query has attached "),
					   Multidim_Movie_Control_pi.name, 
					   N_(": Multidim_Movie_Control is created."),
					   NULL);
}

static void Multidim_Movie_Control_about(void)
{
	const gchar *authors[] = { "Percy Zahl", NULL};
	gtk_show_about_dialog (NULL,
			       "program-name",  Multidim_Movie_Control_pi.name,
			       "version", VERSION,
			       "license", GTK_LICENSE_GPL_3_0,
			       "comments", about_text,
			       "authors", authors,
			       NULL
			       );
}

static void Multidim_Movie_Control_cleanup( void ){
	PI_DEBUG (DBG_L2, "Multidim_Movie_Control Plugin Cleanup" );
	// delete ...
	if( Multidim_Movie_ControlClass )
		delete Multidim_Movie_ControlClass ;
}

static void Multidim_Movie_Control_show_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if( Multidim_Movie_ControlClass )
		Multidim_Movie_ControlClass->show();
}

static void Multidim_Movie_Control_StartScan_callback( gpointer ){
	Multidim_Movie_ControlClass->update();
}

#define XSM gapp->xsm

Multidim_Movie_Control::Multidim_Movie_Control ()
{
	GSList *EC_list=NULL;
	Gtk_EntryControl *ec;

	GtkWidget *grid;
	GtkWidget *frame;
	GtkWidget *input;
	GtkWidget *label;
	GtkWidget *button;
	int x,y;
	x=y=1;
	
	stop_play_layer = TRUE;
	stop_play_time  = TRUE;
	frame_delay = 0.;
	t_play_flg = false;
	l_play_flg = false;
	play_direction = 1;

	Unity    = new UnitObj(" "," ");
	Time     = new UnitObj("ms","ms");

	AppWindowInit(N_("Multi Dimensional Movie Control"));

	grid = gtk_grid_new ();

	// ========================================
	frame = gtk_frame_new (N_("Multi Dimensional Movie Control"));
	gtk_grid_attach (GTK_GRID (v_grid), frame, 1,1, 1,1);
	gtk_container_add (GTK_CONTAINER (frame), grid);


	// Layer Dimension Control
	input = mygtk_grid_add_spin_input (N_("Layer"), grid, x, y, 2);
	g_object_set_data( G_OBJECT (v_grid), "LayerSelectSpin", input);
	ec = new Gtk_EntryControl (XSM->Unity, N_("Layer out of range"), &XSM->data.display.vlayer, 0., 10., ".0f", input, 1., 10.);
	ec->Set_ChangeNoticeFkt(App::spm_select_layer, gapp);
	SetupScale (ec->GetAdjustment(), grid, x,y);
	EC_list = g_slist_prepend( EC_list, ec);

	button = gtk_button_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_BUTTON);
	gtk_grid_attach (GTK_GRID (grid), button, x++, y, 1, 1);
	g_signal_connect ( G_OBJECT (button), "pressed",
			     G_CALLBACK (Multidim_Movie_Control::l_play),
			     this);

	button = gtk_button_new_from_icon_name("media-playback-stop", GTK_ICON_SIZE_BUTTON);	
	gtk_grid_attach (GTK_GRID (grid), button, x++, y, 1, 1);
	g_signal_connect ( G_OBJECT (button), "pressed",
			     G_CALLBACK (Multidim_Movie_Control::l_stop),
			     this);

	button = gtk_button_new_from_icon_name("media-seek-backward", GTK_ICON_SIZE_BUTTON);	
	gtk_grid_attach (GTK_GRID (grid), button, x++, y, 1, 1);
	g_signal_connect ( G_OBJECT (button), "pressed",
			     G_CALLBACK (Multidim_Movie_Control::l_rewind),
			     this);

	x=1,++y;
	// Time Dimension Control...
	input = mygtk_grid_add_spin_input (N_("Time"), grid, x, y, 2);
	g_object_set_data( G_OBJECT (v_grid), "TimeSelectSpin", input);
	ec = new Gtk_EntryControl (XSM->Unity, N_("Time out of range"), &XSM->data.display.vframe, 0., 10., ".0f", input, 1., 10.);
	ec->Set_ChangeNoticeFkt(App::spm_select_time, gapp);
	SetupScale (ec->GetAdjustment(), grid, x, y);
	EC_list = g_slist_prepend( EC_list, ec);


	button = gtk_button_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_BUTTON);	
	gtk_grid_attach (GTK_GRID (grid), button, x++, y, 1, 1);
	g_signal_connect ( G_OBJECT (button), "pressed",
			     G_CALLBACK (Multidim_Movie_Control::t_play),
			     this);

	button = gtk_button_new_from_icon_name("media-playback-stop", GTK_ICON_SIZE_BUTTON);
	gtk_grid_attach (GTK_GRID (grid), button, x++, y, 1, 1);
	g_signal_connect ( G_OBJECT (button), "pressed",
			     G_CALLBACK (Multidim_Movie_Control::t_stop),
			     this);

	button = gtk_button_new_from_icon_name("media-seek-backward", GTK_ICON_SIZE_BUTTON);
	gtk_grid_attach (GTK_GRID (grid), button, x++, y, 1, 1);
	g_signal_connect ( G_OBJECT (button), "pressed",
			     G_CALLBACK (Multidim_Movie_Control::t_rewind),
			     this);

	x=1,++y;
        input = mygtk_grid_add_input("Delay", grid, x, y, 1);
        ec = new Gtk_EntryControl (Time, MLD_WERT_NICHT_OK, &frame_delay, 0., 10000., ".0f", input);

        contineous_autodisp = gtk_check_button_new_with_label(N_("Contineous AutoDisp"));
	gtk_grid_attach (GTK_GRID (grid), contineous_autodisp, x++, y, 1, 1);

	play_mode = gtk_combo_box_text_new ();
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (play_mode),"once", "Once");
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (play_mode),"loop", "Loop");
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (play_mode),"rock", "Rock");
	gtk_combo_box_set_active (GTK_COMBO_BOX (play_mode), 0);
	gtk_grid_attach (GTK_GRID (grid), play_mode, x++, y, 1, 1);

	gtk_widget_show_all (frame);
	
	g_object_set_data( G_OBJECT (window), "MMC_EC_list", EC_list);

	this_movie_control=this;
	Multidim_Movie_Control_pi.app->ConnectPluginToStartScanEvent (MovieControl_Freeze_callback);	 
	Multidim_Movie_Control_pi.app->ConnectPluginToStopScanEvent (MovieControl_Thaw_callback);

	set_window_geometry ("multi-dim-movie-control");
}

Multidim_Movie_Control::~Multidim_Movie_Control (){
	this_movie_control=NULL;
	delete Unity;
	delete Time;
}

void Multidim_Movie_Control::update(){
	g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (window), "MMC_EC_list"),
			(GFunc) App::update_ec, NULL);
}

void Multidim_Movie_Control::l_play (GtkWidget *w, Multidim_Movie_Control *mmc){
	if (mmc->t_play_flg) return;
	if (XSM->data.s.nvalues <= 1) return;
	int l = XSM->data.display.vlayer;
	if (w)
		switch (gtk_combo_box_get_active (GTK_COMBO_BOX (mmc->play_mode))){
		case 0: mmc->play_direction = 1; break;
		case 1: mmc->play_direction = 1; break;
		case 2: mmc->play_direction = mmc->play_direction>0 ? -1:1; break;
		default: break;
		}
	l += mmc->play_direction;
	if (l < XSM->data.s.nvalues && l >= 0){
		if (w)
			mmc->stop_play_layer = FALSE;

		XSM->data.display.vlayer = l;
		App::spm_select_layer (NULL, gapp);
		if (mmc->frame_delay > 0.){
			if ( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (mmc->contineous_autodisp)))
				gapp->xsm->ActiveScan->auto_display ();
			else
				gapp->spm_update_all ();
			mmc->update ();
			usleep ((unsigned long)(mmc->frame_delay * 1e3));
		}
		gapp->check_events ();

		if (!mmc->stop_play_layer)
			l_play (NULL, mmc);
		return;
	}
	switch (gtk_combo_box_get_active (GTK_COMBO_BOX (mmc->play_mode))){
	case 0: mmc->play_direction = 1; break;
	case 1: mmc->play_direction = 1; 
		XSM->data.display.vlayer = 0;
		App::spm_select_layer (NULL, gapp);
		l_play (NULL, mmc);
		break;
	case 2: mmc->play_direction = mmc->play_direction>0 ? -1:1; l_play (NULL, mmc); break;
	default: break;
	}

	mmc->update ();
	gapp->spm_update_all ();
}
void Multidim_Movie_Control::l_stop (GtkWidget *w, Multidim_Movie_Control *mmc){
	mmc->stop_play_layer = TRUE;
	mmc->update ();
	gapp->spm_update_all ();
	mmc->l_play_flg = false;
}
void Multidim_Movie_Control::l_rewind (GtkWidget *w, Multidim_Movie_Control *mmc){
	XSM->data.display.vlayer=0;
	App::spm_select_layer (NULL, gapp);
	mmc->update ();
	gapp->spm_update_all ();
}

void Multidim_Movie_Control::t_play (GtkWidget *w, Multidim_Movie_Control *mmc){
	if (mmc->l_play_flg) return;
	if (XSM->data.s.ntimes <= 1) return;
	int l = XSM->data.display.vframe;
	if (w)
		switch (gtk_combo_box_get_active (GTK_COMBO_BOX (mmc->play_mode))){
		case 0: mmc->play_direction = 1; break;
		case 1: mmc->play_direction = 1; break;
		case 2: mmc->play_direction = mmc->play_direction>0 ? -1:1; break;
		default: break;
		}
	l += mmc->play_direction;
	if (l < XSM->data.s.ntimes && l >= 0){
		if (w)
			mmc->stop_play_time = FALSE;

		XSM->data.display.vframe = l;
		App::spm_select_time (NULL, gapp);
		if (mmc->frame_delay > 0.){
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (mmc->contineous_autodisp)))
			        gapp->xsm->ActiveScan->auto_display ();
			else
				gapp->spm_update_all ();
			mmc->update ();
			usleep ((unsigned long)(mmc->frame_delay * 1e3));
		}
		gapp->check_events ();

		if (!mmc->stop_play_time)
			t_play (NULL, mmc);
		return;
	}
	switch (gtk_combo_box_get_active (GTK_COMBO_BOX (mmc->play_mode))){
	case 0: mmc->play_direction = 1; break;
	case 1: mmc->play_direction = 1;
		XSM->data.display.vframe = 0;
		App::spm_select_time (NULL, gapp);
		t_play (NULL, mmc); 
		break;
	case 2: mmc->play_direction = mmc->play_direction>0 ? -1:1; t_play (NULL, mmc); break;
	default: break;
	}
	mmc->update ();
	gapp->spm_update_all ();
}
void Multidim_Movie_Control::t_stop (GtkWidget *w, Multidim_Movie_Control *mmc){
	mmc->stop_play_time = TRUE;
	mmc->update ();
	gapp->spm_update_all ();
	mmc->t_play_flg = false;
}
void Multidim_Movie_Control::t_rewind (GtkWidget *w, Multidim_Movie_Control *mmc){
	XSM->data.display.vframe=0;
	App::spm_select_time (NULL, gapp);
	mmc->update ();
	gapp->spm_update_all ();
}
