/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Copyright (C) 1999,2000,2001,2002,2003 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * additional features: Andreas Klust <klust@users.sf.net>
 * WWW Home: http://gxsm.sf.net
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

/* ignore this module for docuscan
% PlugInModuleIgnore
*/



#include <locale.h>
#include <libintl.h>

#include <time.h>

#include "glbvars.h"
#include "plug-ins/hard/modules/dsp.h"
#include <fcntl.h>
#include <sys/ioctl.h>

#include "gxsm_app.h"
#include "gxsm_window.h"

#include "gxsm/action_id.h"

#include "dsp-pci32/xsm/xsmcmd.h"

#include "sranger_mk2_hwi_control.h"
#include "sranger_mk23common_hwi.h"
#include "../plug-ins/hard/modules/sranger_mk2_ioctl.h"
#include "plug-ins/common/pyremote.h"

#define UTF8_DEGREE    "\302\260"
#define UTF8_MU        "\302\265"
#define UTF8_ANGSTROEM "\303\205"

extern GxsmPlugin sranger_mk2_hwi_pi;
extern DSPControl *DSPControlClass;
extern sranger_common_hwi_dev *sranger_common_hwi; // instance of the HwI derived XSM_Hardware class
DSPMoverControl *this_mover_control=NULL;	 
extern DSPPACControl *DSPPACClass;


class MOV_GUI_Builder : public BuildParam{
public:
        MOV_GUI_Builder (GtkWidget *build_grid=NULL, GSList *ec_list_start=NULL, GSList *ec_remote_list=NULL) :
                BuildParam (build_grid, ec_list_start, ec_remote_list) {
                wid = NULL;
                config_checkbutton_list = NULL;
        };

        void start_notebook_tab (GtkWidget *notebook, const gchar *name, const gchar *settings_name,
                                 GSettings *settings) {
                new_grid (); tg=grid;
                grid_add_check_button("Configuration: Enable This");
                g_object_set_data (G_OBJECT (button), "TabGrid", grid);
                config_checkbutton_list = g_slist_append (config_checkbutton_list, button);
                configure_list = g_slist_prepend (configure_list, button);

                new_line ();
                
		MoverCtrl = gtk_label_new (name);
		gtk_widget_show (MoverCtrl);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid, MoverCtrl);

                g_settings_bind (settings, settings_name,
                                 G_OBJECT (button), "active",
                                 G_SETTINGS_BIND_DEFAULT);
        };

        void notebook_tab_show_all () {
                gtk_widget_show_all (tg);
        };

        GSList *get_config_checkbutton_list_head () { return config_checkbutton_list; };
        
        GtkWidget *wid;
        GtkWidget *tg;
        GtkWidget *MoverCtrl;
        GSList *config_checkbutton_list;
};




static void DSPMoverControl_Freeze_callback (gpointer x){	 
	if (this_mover_control)	 
		this_mover_control->freeze ();	 
}	 
static void DSPMoverControl_Thaw_callback (gpointer x){	 
	if (this_mover_control)	 
		this_mover_control->thaw ();	 
}

DSPMoverControl::DSPMoverControl ()
{
	XsmRescourceManager xrm("sranger_mk2_hwi_control");

        hwi_settings = g_settings_new (GXSM_RES_BASE_PATH_DOT".hwi.sranger-mk23-mover");
        mover_param.AFM_GPIO_setting = g_settings_get_int (hwi_settings, "mover-gpio-last");
       
	PI_DEBUG (DBG_L2, "DSPMoverControl::DSPMoverControl xrm read mk2 mover settings");

	xrm.Get("AFM_Amp", &mover_param.AFM_Amp, "1");
	xrm.Get("AFM_Speed", &mover_param.AFM_Speed, "3");
	xrm.Get("AFM_Steps", &mover_param.AFM_Steps, "10");
	// xrm.Get("AFM_GPIO_setting", &mover_param.AFM_GPIO_setting, "0");

  	// defaults presets storage for "Besocke" style coarse motions
	// [0..5] Besocke XY, Besocke Rotation, Besocke PSD, Besocke Lens, Auto, Stepper Motor

        for (int i=0; i< DSP_AFMMOV_MODES; ++i){
                gchar *id=g_strdup_printf ("AFM_usrAmp%d", i);
                xrm.Get(id, &mover_param.AFM_usrAmp[0], "1.0"); g_free (id);
                id=g_strdup_printf ("AFM_usrSpeed%d", i);
                xrm.Get(id, &mover_param.AFM_usrSpeed[0], "3"); g_free (id);
                id=g_strdup_printf ("AFM_usrSteps%d", i);
                xrm.Get(id, &mover_param.AFM_usrSteps[0], "10"); g_free (id);
                id=g_strdup_printf ("AFM_GPIO_setting%d", i);
                xrm.Get(id, &mover_param.AFM_GPIO_usr_setting[0], "0"); g_free (id);
        }
	xrm.Get("MOV_output", &mover_param.MOV_output, "0");
	xrm.Get("MOV_waveform_id", &mover_param.MOV_waveform_id, "0");
	xrm.Get("MOV_wave0_out_channel", &mover_param.wave_out_channel[0], "3");
	xrm.Get("MOV_wave1_out_channel", &mover_param.wave_out_channel[1], "4");
	xrm.Get("MOV_mode", &mover_param.MOV_mode, "0");
	xrm.Get("AUTO_final_delay", &mover_param.final_delay, "50");
	xrm.Get("AUTO_max_settling_time", &mover_param.max_settling_time, "1000");
	xrm.Get("InchWorm_phase", &mover_param.inch_worm_phase, "0");

	xrm.Get("Wave_space", &mover_param.Wave_space, "0.0");

	xrm.Get("GPIO_on", &mover_param.GPIO_on, "0");
	xrm.Get("GPIO_off", &mover_param.GPIO_off, "0");
	xrm.Get("GPIO_reset", &mover_param.GPIO_reset, "0");
	xrm.Get("GPIO_scan", &mover_param.GPIO_scan, "0");
	xrm.Get("GPIO_tmp1", &mover_param.GPIO_tmp1, "0");
	xrm.Get("GPIO_tmp2", &mover_param.GPIO_tmp2, "0");
	xrm.Get("GPIO_direction", &mover_param.GPIO_direction, "0");
	xrm.Get("GPIO_delay", &mover_param.GPIO_delay, "250.0");

	xrm.Get("Z0_speed", &Z0_speed, "500.");
	xrm.Get("Z0_adjust", &Z0_adjust, "500.");
	xrm.Get("Z0_goto", &Z0_goto, "0.");

	mover_param.MOV_wave_len=1024;
	for (int i=0; i < MOV_MAXWAVELEN; ++i)
		mover_param.MOV_waveform[i] = (short)0;

	Unity    = new UnitObj(" "," ");
	Phase    = new UnitObj(UTF8_DEGREE, "deg");
	Hex      = new UnitObj("h","h");
	Volt     = new UnitObj("V","V");
	Time     = new UnitObj("ms","ms");
	Length   = new UnitObj("nm","nm");
	Ang      = new UnitObj(UTF8_ANGSTROEM,"A");
	Speed    = new UnitObj(UTF8_ANGSTROEM"/s","A/s");

	PI_DEBUG (DBG_L2, "... OK");
	PI_DEBUG (DBG_L2, "DSPMoverControl::DSPMoverControl create folder");
	create_folder ();
	PI_DEBUG (DBG_L2, "... OK");
	this_mover_control=this;	 
	sranger_mk2_hwi_pi.app->ConnectPluginToStartScanEvent (DSPMoverControl_Freeze_callback);	 
	sranger_mk2_hwi_pi.app->ConnectPluginToStopScanEvent (DSPMoverControl_Thaw_callback);
}

// duration in ms
// amp in V SR out (+/-2.05V max)
void DSPMoverControl::create_waveform (double amp, double duration){
#define SR_VFAC    (32767./10.00) // A810 max Volt out is 10V

	gint space_len = (int)round ( DSPControlClass->frq_ref*2.* mover_param.Wave_space*1e-3); 

	mover_param.MOV_wave_len = (int)round ( DSPControlClass->frq_ref*2.*duration*1e-3);
	mover_param.MOV_speed_fac = 1;

	while (mover_param.MOV_wave_len/mover_param.MOV_speed_fac >= MOV_MAXWAVELEN)
		++mover_param.MOV_speed_fac;

	mover_param.MOV_wave_len /= mover_param.MOV_speed_fac;
	space_len /= mover_param.MOV_speed_fac;

	if (mover_param.MOV_wave_len < 2)
		mover_param.MOV_wave_len = 2;


	PI_DEBUG (DBG_L2, "DSPMoverControl::create_waveform:" << mover_param.MOV_wave_len);


	double n = (double)mover_param.MOV_wave_len;
	double n2 = n/2.;
	double t=0.;
	
	switch (mover_param.MOV_waveform_id){
	case MOV_WAVE_SAWTOOTH:
		for (int i=0; i < mover_param.MOV_wave_len; ++i)
			mover_param.MOV_waveform[i] = ((short)round (SR_VFAC*amp*((double)i-n2)/n2));
		break;
	case MOV_WAVE_SINE:
		for (int i=0; i < mover_param.MOV_wave_len; ++i)
			mover_param.MOV_waveform[i] = ((short)round (SR_VFAC*amp*sin ((double)i*2.*M_PI/n)));
		break;
	case MOV_WAVE_CYCLO:
	case MOV_WAVE_CYCLO_PL:
	case MOV_WAVE_CYCLO_MI:
		t=0.;
		for (int i=0; i < mover_param.MOV_wave_len; ++i){
			double dt = 1./(mover_param.MOV_wave_len - 1);
			double a = 1.;
			switch (mover_param.MOV_waveform_id){
			case MOV_WAVE_CYCLO_PL:
				mover_param.MOV_waveform[i] = ((short)round(SR_VFAC*amp*a*( (t*t*t*t)) * (-1) ));
				t += dt;
				break;	
			case MOV_WAVE_CYCLO_MI:
				mover_param.MOV_waveform[i] = ((short)round(SR_VFAC*amp*a*( (t*t*t*t) - 1 ) * (-1) ));
				t += dt;
				break;
			default:
				dt = M_PI/2./(mover_param.MOV_wave_len - 1.)*2;
				mover_param.MOV_waveform[i] = ((short)round(SR_VFAC*amp*(a * (1.-cos (t)))));
				if (i < (mover_param.MOV_wave_len/2))
					t += dt;
				else
					t -= dt;
				break;
			}
		}
		break;
	case MOV_WAVE_CYCLO_IPL:
	case MOV_WAVE_CYCLO_IMI:
		t=0.;
		for (int i=0; i < mover_param.MOV_wave_len; ++i){
			double dt = 1./(mover_param.MOV_wave_len - 1.);
			double a = 1.;
			switch (mover_param.MOV_waveform_id){
			case MOV_WAVE_CYCLO_IPL:
				mover_param.MOV_waveform[i] = ((short)round(SR_VFAC*amp*a*( (t*t*t*t) - 1 ) * (1) ));
				break;	
			case MOV_WAVE_CYCLO_IMI:
				mover_param.MOV_waveform[i] = ((short)round(SR_VFAC*amp*a*( (t*t*t*t)) * (1) ));
				break;
			}
			t += dt;
		}
		break;
	case MOV_WAVE_PULSE_P:
		for (int i=0; i < mover_param.MOV_wave_len; ++i){
			mover_param.MOV_waveform[i] = ((short)round(SR_VFAC*amp*(round((double)i/mover_param.MOV_wave_len)))); 
		}
		break;
/* ------------------------------------------------------------------------------------------------------------------------------------------------
// Code for negative pulse, but not usefull yet as output voltage is set to zero after pulse sequence. (stm, 08.10.2010)
//-------------------------------------------------------------------------------------------------------------------------------------------------
//	case MOV_WAVE_PULSE_M:
//		for (int i=0; i < mover_param.MOV_wave_len; ++i){
//			mover_param.MOV_waveform[i] = sranger_common_hwi->int_2_sranger_int((short)round(SR_VFAC*amp*(1-round((double)i/mover_param.MOV_wave_len)))); 
//		}
//		mover_param.MOV_waveform[mover_param.MOV_wave_len-1] = sranger_common_hwi->int_2_sranger_int((short)round(SR_VFAC*amp*(1)));
//		break;
--------------------------------------------------------------------------------------------------------------------------------------------------*/
	break;
	
	}

	
	for (int i = mover_param.MOV_wave_len; i < mover_param.MOV_wave_len+space_len; ++i)
		mover_param.MOV_waveform[i] = mover_param.MOV_waveform[0];

	mover_param.MOV_wave_len += space_len;
	

}


DSPMoverControl::~DSPMoverControl (){
	this_mover_control=NULL;

	delete Length;
	delete Phase;
	delete Time;
	delete Volt;
	delete Unity;
	delete Hex;
	delete Ang;
	delete Speed;
}



#define ARROW_SIZE 40

static gboolean create_window_key_press_event_lcb(GtkWidget *widget, GdkEventKey *event,GtkWidget *win)
{
	if (event->keyval == GDK_KEY_Escape) {
		PI_DEBUG (DBG_L2, "Got escape\n" );
		return TRUE;
	}
	return FALSE;
}

// ============================================================
// Popup Menu and Object Action Map
// ============================================================

static GActionEntry win_DSPMover_popup_entries[] = {
        { "dsp-mover-configure", DSPMoverControl::configure_callback, NULL, "false", NULL },
};

void DSPMoverControl::configure_callback (GSimpleAction *action, GVariant *parameter, 
                                          gpointer user_data){
        DSPMoverControl *mc = (DSPMoverControl *) user_data;
        GVariant *old_state, *new_state;

        if (action){
                old_state = g_action_get_state (G_ACTION (action));
                new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));
                
                g_simple_action_set_state (action, new_state);
                g_variant_unref (old_state);

                PI_DEBUG_GP (DBG_L4, "Toggle action %s activated, state changes from %d to %d\n",
                             g_action_get_name (G_ACTION (action)),
                             g_variant_get_boolean (old_state),
                             g_variant_get_boolean (new_state));

                g_simple_action_set_state (action, new_state);
                g_variant_unref (old_state);
        } else {
                new_state = g_variant_new_boolean (false);
        }

	if (g_variant_get_boolean (new_state)){
                g_slist_foreach
                        ( mc->mov_bp->get_configure_list_head (),
                          (GFunc) App::show_w, NULL
                          );
                g_slist_foreach
                        ( mc->mov_bp->get_config_checkbutton_list_head (),
                          (GFunc) DSPMoverControl::show_tab_to_configure, NULL
                          );
        } else  {
                g_slist_foreach
                        ( mc->mov_bp->get_configure_list_head (),
                          (GFunc) App::hide_w, NULL
                          );
                g_slist_foreach
                        ( mc->mov_bp->get_config_checkbutton_list_head (),
                          (GFunc) DSPMoverControl::show_tab_as_configured, NULL
                          );
        }
        if (!action){
                g_variant_unref (new_state);
        }
}

void DSPMoverControl::AppWindowInit(const gchar *title){
        if (title) { // stage 1
                PI_DEBUG (DBG_L2, "DSPMoverControl::AppWindowInit -- header bar");

                app_window = gxsm3_app_window_new (GXSM3_APP (gapp->get_application ()));
                window = GTK_WINDOW (app_window);

                header_bar = gtk_header_bar_new ();
                gtk_widget_show (header_bar);
                // hide close, min, max window decorations
                gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), false);

                XSM_DEBUG (DBG_L2,  "VC::VC setup titlbar" );

                gtk_window_set_title (GTK_WINDOW (window), title);
                gtk_header_bar_set_title ( GTK_HEADER_BAR (header_bar), title);
                // gtk_header_bar_set_subtitle (GTK_HEADER_BAR  (header_bar), title);
                gtk_window_set_titlebar (GTK_WINDOW (window), header_bar);

                g_signal_connect (G_OBJECT(window),
                                  "delete_event",
                                  G_CALLBACK(App::close_scan_event_cb),
                                  this);
        
                v_grid = gtk_grid_new ();
                gtk_container_add (GTK_CONTAINER (window), v_grid);
                g_object_set_data (G_OBJECT (window), "v_grid", v_grid); // was "vbox"

                gtk_widget_show_all (GTK_WIDGET (window));

                //        g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
                g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (AppBase::window_close_callback), this);
                
        } else {
                PI_DEBUG (DBG_L2, "DSPMoverControl::AppWindowInit -- header bar -- stage two, hook configure menu");

                g_action_map_add_action_entries (G_ACTION_MAP (app_window),
                                                 win_DSPMover_popup_entries, G_N_ELEMENTS (win_DSPMover_popup_entries),
                                                 this);

                // create window PopUp menu  ---------------------------------------------------------------------
                mc_popup_menu = gtk_menu_new_from_model (G_MENU_MODEL (gapp->get_hwi_mover_popup_menu ()));
                g_assert (GTK_IS_MENU (mc_popup_menu));

                GtkIconSize tmp_toolbar_icon_size = GTK_ICON_SIZE_LARGE_TOOLBAR;

                // attach popup menu configuration to tool button --------------------------------
                GtkWidget *header_menu_button = gtk_menu_button_new ();
                gtk_button_set_image (GTK_BUTTON (header_menu_button), gtk_image_new_from_icon_name ("applications-utilities-symbolic", tmp_toolbar_icon_size));
                gtk_menu_button_set_popup (GTK_MENU_BUTTON (header_menu_button), mc_popup_menu);
                gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_menu_button);
                gtk_widget_show (header_menu_button);

                g_settings_bind (hwi_settings, "configure-mode",
                                 G_OBJECT (GTK_BUTTON (header_menu_button)), "active",
                                 G_SETTINGS_BIND_DEFAULT);  
        }
}

void DSPMoverControl::create_folder (){
	GSList *EC_list=NULL;

	Gtk_EntryControl *ec;
	GtkWidget *tg, *fg, *fg2, *frame_param, *frame_param2;
	GtkWidget *input;
	GtkWidget *notebook;
	GtkWidget *MoverCrtl;
	GtkWidget *button, *img, *lab;
	GtkAccelGroup *accel_group=NULL;

        PI_DEBUG (DBG_L2, "DSPMoverControl::create_folder");

	if( IS_MOVER_CTRL ){
                //		accel_group = gtk_accel_group_new ();
		AppWindowInit (MOV_MOVER_TITLE); // stage one
	}
	else {
		AppWindowInit (MOV_SLIDER_TITLE); // stage one
        }
        
	// ========================================
	notebook = gtk_notebook_new ();
        // gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);

	gtk_widget_show (notebook);
	gtk_grid_attach (GTK_GRID (v_grid), notebook, 1,1, 1,1);

	const char *MoverNames[] = { "X&Y", "Rot", "PSD", "Lens", "Auto", "SM", "Z0", "Config", NULL};
	const char *mover_tab_key[] = { "mover-tab-xy", "mover-tab-rot", "mover-tab-psd", "mover-tab-lens", "mover-tab-auto", "mover-tab-sm", "mover-tab-z0", "mover-tab-config", NULL};
	const char *pcs_tab_remote_key_prefix[] = { "dspmover-xy-", "dspmover-rot-", "dspmover-psd-", "dspmover-lens-", "dspmover-auto-", "dspmover-sm-", "dspmover-z0-", "dspmover-config-", NULL};

	Gtk_EntryControl *Ampl, *Spd, *Stp, *GPIO_on, *GPIO_off, *GPIO_reset, *GPIO_scan, *GPIO_dir, *Wave_out0, *Wave_out1;
	int i,itab;

        mov_bp = new MOV_GUI_Builder (v_grid);
        mov_bp->set_error_text ("Invalid Value.");
        mov_bp->set_input_width_chars (10);
        mov_bp->set_no_spin ();
        
	for(itab=i=0; MoverNames[i]; ++i){
		if (gapp->xsm->Inst->OffsetMode() != OFM_ANALOG_OFFSET_ADDING && i == 5) continue;
		if (IS_SLIDER_CTRL && i < 4 ) continue;
		PI_DEBUG (DBG_L2, "DSPMoverControl::Mover:" << MoverNames[i]);

                mov_bp->start_notebook_tab (notebook, MoverNames[i], mover_tab_key[i], hwi_settings);
                itab++;
                        
                mov_bp->set_pcs_remote_prefix (pcs_tab_remote_key_prefix[i]);

                if (i==6){ // Z0 Tab
                        mov_bp->new_grid_with_frame ("Z-Offset Control");
			mov_bp->set_default_ec_change_notice_fkt (DSPMoverControl::ChangedNotify, this);

                        mov_bp->set_configure_list_mode_on ();
  			mov_bp->grid_add_ec ("Adjust Speed", Speed, &Z0_speed, 0.1, 5000., "4.1f", 10., 100., "speed");
                        mov_bp->new_line ();
  			mov_bp->grid_add_ec ("Adjust Range", Ang, &Z0_adjust, 1., 50000., "4.0f", 10., 100., "range");
                        mov_bp->new_line ();
  			mov_bp->grid_add_ec ("Adjust Goto", Ang, &Z0_goto, -1e6, 1e6, "8.1f", 10., 100., "goto");
                        mov_bp->set_configure_list_mode_off ();
                        mov_bp->new_line ();

                        mov_bp->new_grid_with_frame ("Direction & Action Control");
			// ========================================
			// Direction Buttons
			mov_bp->set_xy (1,11); mov_bp->grid_add_label ("Z+");
			mov_bp->set_xy (1,13); mov_bp->grid_add_label ("Z-");
			mov_bp->set_xy (5,11); mov_bp->grid_add_label ("Auto Adj.");
			mov_bp->set_xy (5,12); mov_bp->grid_add_label ("Center");
			mov_bp->set_xy (5,13); mov_bp->grid_add_label ("Goto");

			// STOP
                        mov_bp->set_xy (3,12);  mov_bp->grid_add_widget (button=gtk_button_new_from_icon_name ("process-stopall-symbolic", GTK_ICON_SIZE_BUTTON));
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_Z0_STOP));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (99));
			g_signal_connect ( G_OBJECT (button), "pressed",
                                           G_CALLBACK (DSPMoverControl::CmdAction),
                                           this);
			// UP arrow (back)
			mov_bp->set_xy (3,11);  mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("seek-backward-symbolic", GTK_ICON_SIZE_BUTTON));
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_Z0_P));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (99));
			g_signal_connect ( G_OBJECT (button), "pressed",
                                           G_CALLBACK (DSPMoverControl::CmdAction),
                                           this);
			g_signal_connect ( G_OBJECT (button), "released",
                                           G_CALLBACK (DSPMoverControl::StopAction),
                                           this);
			g_signal_connect( G_OBJECT(v_grid), "key_press_event", 
                                          G_CALLBACK(create_window_key_press_event_lcb), this);

			// DOWN arrow (forward)
			mov_bp->set_xy (3,13);  mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("seek-forward-symbolic", GTK_ICON_SIZE_BUTTON));
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_Z0_M));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (99));
			g_signal_connect ( G_OBJECT (button), "pressed",
                                           G_CALLBACK (DSPMoverControl::CmdAction),
                                           this);
			g_signal_connect ( G_OBJECT (button), "released",
                                           G_CALLBACK (DSPMoverControl::StopAction),
                                           this);

                        // approach (connect, disconnect)
			mov_bp->set_xy (6,11);  mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("goto-center-symbolic", GTK_ICON_SIZE_BUTTON));
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_Z0_AUTO));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (99));
			g_signal_connect ( G_OBJECT (button), "pressed",
                                           G_CALLBACK (DSPMoverControl::CmdAction),
                                           this);

			mov_bp->set_xy (6,12);  mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("goto-home-symbolic", GTK_ICON_SIZE_BUTTON));
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_Z0_CENTER));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (99));
			g_signal_connect ( G_OBJECT (button), "pressed",
                                           G_CALLBACK (DSPMoverControl::CmdAction),
                                           this);

                        mov_bp->set_xy (6,13);  mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("goto-position-symbolic", GTK_ICON_SIZE_BUTTON));
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_Z0_GOTO));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (99));
			g_signal_connect ( G_OBJECT (button), "pressed",
                                           G_CALLBACK (DSPMoverControl::CmdAction),
                                           this);

			mov_bp->notebook_tab_show_all ();
			continue;
		}

		if (i==7){ // configure tab
			GtkWidget *radiobutton;
			
                        mov_bp->new_grid_with_frame ("Output Configuration");
			mov_bp->set_default_ec_change_notice_fkt (DSPMoverControl::ChangedNotify, this);

                        mov_bp->new_grid_with_frame ("Curve Mode");

			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label (NULL, "Wave: Sawtooth"), 2); // arbitrary waveform
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", GINT_TO_POINTER (AAP_MOVER_WAVE));
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", GINT_TO_POINTER (MOV_WAVE_SAWTOOTH));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
			
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_mode == AAP_MOVER_WAVE && mover_param.MOV_waveform_id == MOV_WAVE_SAWTOOTH) ? 1:0);
                        mov_bp->new_line ();

			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Sine"), 2); // arbitrary waveform
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", GINT_TO_POINTER (AAP_MOVER_WAVE));
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", GINT_TO_POINTER (MOV_WAVE_SINE));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
			
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_mode == AAP_MOVER_WAVE && mover_param.MOV_waveform_id == MOV_WAVE_SINE) ? 1:0);
                        mov_bp->new_line ();

			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Cyclo"), 2); // arbitrary waveform
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", GINT_TO_POINTER (AAP_MOVER_WAVE));
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", GINT_TO_POINTER (MOV_WAVE_CYCLO));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
                        mov_bp->new_line ();
			
// ==
			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Cyclo+"), 2); // arbitrary waveform
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", GINT_TO_POINTER (AAP_MOVER_WAVE));
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", GINT_TO_POINTER (MOV_WAVE_CYCLO_PL));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
                        mov_bp->new_line ();
			
			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Cyclo-"), 2); // arbitrary waveform
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", GINT_TO_POINTER (AAP_MOVER_WAVE));
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", GINT_TO_POINTER (MOV_WAVE_CYCLO_MI));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
                        mov_bp->new_line ();
			
			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Inv Cyclo+"), 2); // arbitrary waveform
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", GINT_TO_POINTER (AAP_MOVER_WAVE));
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", GINT_TO_POINTER (MOV_WAVE_CYCLO_IPL));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
                        mov_bp->new_line ();
			
			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Inv Cyclo-"), 2); // arbitrary waveform
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", GINT_TO_POINTER (AAP_MOVER_WAVE));
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", GINT_TO_POINTER (MOV_WAVE_CYCLO_IMI));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
                        mov_bp->new_line ();
			
			// Pulses are generated as arbitrary wave forms (stm, 08.10.2010)
			// positive pluse: base line is zero, pulse as height given by amplitude
			// ratio between on/off: 1:1
			// total time = ton + toff
			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Pulse: positive"), 2); // arbitrary waveform
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", GINT_TO_POINTER (AAP_MOVER_WAVE));
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", GINT_TO_POINTER (MOV_WAVE_PULSE_P));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_mode == AAP_MOVER_WAVE && mover_param.MOV_waveform_id == MOV_WAVE_PULSE_P) ? 1:0);
                        mov_bp->new_line ();

/* ------------------------------------------------------------------------------------------------------------------------------------------------
// Code for negative pulse, but not usefull yet as output voltage is set to zero after pulse sequence. (stm, 08.10.2010)
//-------------------------------------------------------------------------------------------------------------------------------------------------
			// negative pluse: base line is equal to amplitude, pulse are given by zero amplitude
			// ratio between on/off: 1:1
			// total time = ton + toff
			// radiobutton = gtk_radio_button_new_with_label (radiogroup, "Pulse: negative"); // arbitrary waveform
			// gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			// gtk_widget_show (radiobutton);
			// g_object_set_data (G_OBJECT (radiobutton), "CurveMask", GINT_TO_POINTER (AAP_MOVER_WAVE));
			// g_object_set_data (G_OBJECT (radiobutton), "CurveId", GINT_TO_POINTER (MOV_WAVE_PULSE_M));
 			// g_signal_connect (G_OBJECT (radiobutton), "clicked",
 			//		    G_CALLBACK (DSPMoverControl::config_mode), this);
 			// g_signal_connect (G_OBJECT (radiobutton), "clicked",
 			//		    G_CALLBACK (DSPMoverControl::config_waveform), this);
			// radiogroup = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobutton));
			// gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_mode == AAP_MOVER_WAVE && mover_param.MOV_waveform_id == MOV_WAVE_PULSE_M) ? 1:0);
-----------------------------------------------------------------------------------------------------------------------------------------------*/
// ==
			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "(none) GPIO Pulse"), 2);
// 			gtk_widget_set_sensitive (radiobutton, FALSE);
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", GINT_TO_POINTER (AAP_MOVER_PULSE));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_mode == AAP_MOVER_PULSE) ? 1:0);
                        mov_bp->new_line ();

                        mov_bp->grid_add_ec ("Space", Time, &mover_param.Wave_space, 0., 1000., ".3f", 1., 10., "Wave-Space");
			g_object_set_data( G_OBJECT (mov_bp->input), "Wavespace ", GINT_TO_POINTER (i));
			gtk_widget_set_tooltip_text (mov_bp->input, "Wave form spacing ");
                        mov_bp->new_line ();

			mov_bp->grid_add_ec ("IW Phase", Phase, &mover_param.inch_worm_phase, 0., 360., "3.0f", 1., 60., "IW-Phase");
			gtk_widget_set_tooltip_text (mov_bp->input,
                                                     "For Inch Worm mode set > 0 to specify phase.\n"
                                                     "Set this to Zero (0) for regular output separated by X|Y modes.\n"
                                                     "Else wave form is output on [X,Y] or [X0,Y0] with phase.");

// ==================================================
                        mov_bp->pop_grid ();
			mov_bp->new_grid_with_frame ("Output on");

			if (DSPPACClass){
			        mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label (NULL, "Wave[0,1] out on:"), 2);
				g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_XXOFFSET));
				g_signal_connect (G_OBJECT (radiobutton), "clicked",
						    G_CALLBACK (DSPMoverControl::config_output), this);
				
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_output == AAP_MOVER_XXOFFSET) ? 1:0);
                                mov_bp->new_line ();

                                mov_bp->set_default_ec_change_notice_fkt (DSPMoverControl::ChangedWaveOut, this);
				mov_bp->grid_add_ec ("OUTMIX8 to Ch", Unity, &mover_param.wave_out_channel[0], 0, 7, ".0f", "wave-out8-ch");
				g_object_set_data( G_OBJECT (mov_bp->input), "WAVE_OUT_CH", GINT_TO_POINTER (0));
				gtk_widget_set_tooltip_text (mov_bp->input, "map Wave [0,1] on OUTMIX_CH8 to Channel 0-7");
                                mov_bp->new_line ();
				
				mov_bp->grid_add_ec ("OUTMIX9 to Ch", Unity, &mover_param.wave_out_channel[1], 0.,7., ".0f","wave-out9-ch");
				g_object_set_data( G_OBJECT (mov_bp->input), "WAVE_OUT_CH", GINT_TO_POINTER (1));
				gtk_widget_set_tooltip_text (mov_bp->input, "map Wave [0,1] on OUTMIX_CH9 to Channel 0-7");
                                mov_bp->new_line ();

                                mov_bp->set_default_ec_change_notice_fkt (DSPMoverControl::ChangedNotify, this);
			}

                        // get default => radio/check link
			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "XY-Scan CH[3,4]=Wave[0,1]"), 2);
			g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_XYSCAN));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
                                          G_CALLBACK (DSPMoverControl::config_output), this);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_output == AAP_MOVER_XYSCAN) ? 1:0);
                        mov_bp->new_line ();

			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "XY-Offset CH[0,1]=Wave[0,1]"), 2);
			g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_XYOFFSET));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
                                          G_CALLBACK (DSPMoverControl::config_output), this);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_output == AAP_MOVER_XYOFFSET) ? 1:0);
                        mov_bp->new_line ();
			
			if (!DSPPACClass){
				mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "XY-Offset* CH[0,1]=Wave[0,0]"), 2);
				g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_XXOFFSET));
				g_signal_connect (G_OBJECT (radiobutton), "clicked",
						    G_CALLBACK (DSPMoverControl::config_output), this);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_output == AAP_MOVER_XXOFFSET) ? 1:0);
                                mov_bp->new_line ();

				mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "X-Motor CH[7]=Wave[0]"), 2);
				g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_XYMOTOR));
				g_signal_connect (G_OBJECT (radiobutton), "clicked",
						    G_CALLBACK (DSPMoverControl::config_output), this);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_output == AAP_MOVER_XYMOTOR) ? 1:0);
                                mov_bp->new_line ();
				
				mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "X-Z-Scan Add CH[5]=Wave[0]+Z Servo"), 2);
				g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_ZSCANADD));
				g_signal_connect (G_OBJECT (radiobutton), "clicked",
                                                  G_CALLBACK (DSPMoverControl::config_output), this);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_output == AAP_MOVER_ZSCANADD) ? 1:0);
                                mov_bp->new_line ();
			}
 			mov_bp->grid_add_widget (radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "GPIO Pulse"), 2);
// 			gtk_widget_set_sensitive (radiobutton, FALSE);
  			g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_PULSE));
   			g_signal_connect (G_OBJECT (radiobutton), "clicked",
                                          G_CALLBACK (DSPMoverControl::config_output), this);
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton),  (mover_param.MOV_output == AAP_MOVER_PULSE) ? 1:0);
                        mov_bp->new_line ();

			mov_bp->grid_add_ec ("GPIO Pon", Hex, &mover_param.GPIO_on, 0x0000, 0xffff, "04X", "GPIO-on");
			g_object_set_data( G_OBJECT (mov_bp->input), "GPIO_on", GINT_TO_POINTER (i));
			gtk_widget_set_tooltip_text (mov_bp->input, "GPIO setting for Pulses >High<.\nNote: Added bits to GPIO setting for each tab.");
                        mov_bp->new_line ();
                        
			mov_bp->grid_add_ec ("GPIO Poff", Hex, &mover_param.GPIO_off, 0x0000, 0xffff, "04X", "GPIO-off"); 
			g_object_set_data( G_OBJECT (mov_bp->input), "GPIO_off", GINT_TO_POINTER (i));
			gtk_widget_set_tooltip_text (mov_bp->input, "GPIO setting for Pulses >Low<.\nNote: Added bits to GPIO setting for each tab.");
                        mov_bp->new_line ();
                        
			mov_bp->grid_add_ec ("GPIO Preset", Hex, &mover_param.GPIO_reset, 0x0000, 0xffff, "04X", "GPIO-reset");
			g_object_set_data( G_OBJECT (mov_bp->input), "GPIO_reset", GINT_TO_POINTER (i));
			gtk_widget_set_tooltip_text (mov_bp->input, "GPIO setting after Pulses are done.\nNote: Added bits to GPIO setting for each tab.");
                        mov_bp->new_line ();

			mov_bp->grid_add_ec ("GPIO XY-scan", Hex, &mover_param.GPIO_scan, 0x0000, 0xffff, "04X", "GPIO-scan"); 
                        g_object_set_data( G_OBJECT (mov_bp->input), "GPIO_scan", GINT_TO_POINTER (i));
			gtk_widget_set_tooltip_text (mov_bp->input, "GPIO setting for regular XY-scan mode [XY, Main].");
                        mov_bp->new_line ();

			mov_bp->grid_add_ec ("GPIO tmp1", Hex, &mover_param.GPIO_tmp1, 0x0000, 0xffff, "04X", "GPIO-tmp1");
			g_object_set_data( G_OBJECT (mov_bp->input), "GPIO_tmp1", GINT_TO_POINTER (i));
			gtk_widget_set_tooltip_text (mov_bp->input, "GPIO intermediate setting 1 for switching to XY-scan mode [XY, Main].");
                        mov_bp->new_line ();

			mov_bp->grid_add_ec ("GPIO tmp2", Hex, &mover_param.GPIO_tmp2, 0x0000, 0xffff, "04X", "GPIO-tmp2");
			g_object_set_data( G_OBJECT (mov_bp->input), "GPIO_tmp2", GINT_TO_POINTER (i));
			gtk_widget_set_tooltip_text (mov_bp->input, "GPIO intermediate setting 2 for switching to XY-scan mode [XY, Main].");
                        mov_bp->new_line ();

			mov_bp->grid_add_ec ("GPIO direction", Hex, &mover_param.GPIO_direction, 0x0000, 0xffff, "04X", "GPIO-direction");
			g_object_set_data( G_OBJECT (mov_bp->input), "GPIO_direction", GINT_TO_POINTER (i));
			gtk_widget_set_tooltip_text (mov_bp->input, "GPIO direction (in/out) configuration\nWARNING: DO NOT TOUCH IF UNSURE.\nPut 0 for all inputs (safe), but no action (will also disable use of GPIO).");
                        mov_bp->new_line ();

			mov_bp->grid_add_ec ("GPIO delay", Time, &mover_param.GPIO_delay, 0.,1000., ".1f", 1., 10., "GPIO-delay"); 
			g_object_set_data( G_OBJECT (mov_bp->input), "GPIO_delay", GINT_TO_POINTER (i));
			gtk_widget_set_tooltip_text (mov_bp->input, "GPIO delay: delay in milliseconds after switching bits.");

                        mov_bp->pop_grid ();
                        mov_bp->new_line ();
                        
			mov_bp->new_grid_with_frame ("Auto Approach Timings and IW Phase");

			mov_bp->grid_add_ec("Auto App. Delay", Time, &mover_param.final_delay, 0., 430., "4.1f", 1., 10., "Auto-App-Delay");
			gtk_widget_set_tooltip_text (mov_bp->input, "IVC recovery delay after step series\nfired for auto approach.");
                        mov_bp->new_line ();

			mov_bp->grid_add_ec("Auto App. max settling time ", Time, &mover_param.max_settling_time, 0., 10000., "6.1f", 1., 10., "Auto-App-Max-Setting-Time");
			gtk_widget_set_tooltip_text (mov_bp->input, "max allowed time to settle for feedback\nbefore declaring it to be finished.\n(max allowed time/cycle for FB-delta>0)");

			mov_bp->notebook_tab_show_all ();
			continue;
		}

		if (i<DSP_AFMMOV_MODES){ // modes stores 0..5
                        mov_bp->new_grid_with_frame ("Mover Timing");
                        mov_bp->set_default_ec_change_notice_fkt (DSPMoverControl::ChangedNotify, this);

                        mov_bp->grid_add_ec ("Max. Steps", Unity, &mover_param.AFM_usrSteps[i], 1., 5000., "4.0f", 1., 10., "max-steps"); mov_bp->new_line ();
                        g_object_set_data( G_OBJECT (mov_bp->input), "MoverNo", GINT_TO_POINTER (i));
                        
                        mov_bp->set_configure_list_mode_on ();
                        mov_bp->grid_add_ec ("Amplitude", Volt, &mover_param.AFM_usrAmp[i], -20., 20., "5.2f", 0.1, 1., "amplitude"); mov_bp->new_line ();
                        g_object_set_data( G_OBJECT (mov_bp->input), "MoverNo", GINT_TO_POINTER (i));

                        mov_bp->grid_add_ec ("Duration", Time, &mover_param.AFM_usrSpeed[i], 0.1, 110., "4.3f", 0.1, 1., "duration"); mov_bp->new_line ();
                        g_object_set_data( G_OBJECT (mov_bp->input), "MoverNo", GINT_TO_POINTER (i));

                        mov_bp->grid_add_ec ("GPIO", Hex, &mover_param.AFM_GPIO_usr_setting[i], 0, 0xffff, "04X", "gpio"); mov_bp->new_line ();
                        g_object_set_data( G_OBJECT (mov_bp->input), "MoverNo", GINT_TO_POINTER (i));
                        gtk_widget_set_tooltip_text (mov_bp->input, "GPIO setting used to engage this tab mode.");

                        mov_bp->set_configure_list_mode_off ();
                }

                mov_bp->pop_grid ();
                mov_bp->new_line ();
                mov_bp->new_grid_with_frame ("Direction & Action Control");

		// ========================================
		// Direction Buttons

		if( IS_MOVER_CTRL ){
			// STOP
			mov_bp->set_xy (3,2); mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("process-stopall-symbolic", GTK_ICON_SIZE_BUTTON));
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::StopAction),
					    this);
                        // UP
			mov_bp->set_xy (3,1); mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("seek-backward-symbolic", GTK_ICON_SIZE_BUTTON));
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_AFM_MOV_YP));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::CmdAction),
					    this);
			g_signal_connect (G_OBJECT (button), "released",
					    G_CALLBACK (DSPMoverControl::StopAction),
					    this);
			/*
			g_signal_connect(G_OBJECT(v_grid *** box), "key_press_event", 
					   G_CALLBACK(create_window_key_press_event_lcb), this);
			*/
			{ // pyremote hook
				remote_action_cb *ra = g_new( remote_action_cb, 1);
				ra -> cmd = g_strdup_printf("DSP_CMD_MOV-YP_%d",i);
				ra -> RemoteCb = (void (*)(GtkWidget*, void*))DSPMoverControl::CmdAction;
				ra -> widget = button;
				ra -> data = this;
				gapp->RemoteActionList = g_slist_prepend ( gapp->RemoteActionList, ra );
				gchar *help = g_strconcat ("Remote example: action (\"", ra->cmd, "\"", NULL);
				gtk_widget_set_tooltip_text (button, help);
			}

//      gtk_widget_add_accelerator (button, "pressed", accel_group,
//                                  GDK_F3+4*i, (GdkModifierType)0,
//                                  GTK_ACCEL_VISIBLE);
      
			if (i == 6){
                                mov_bp->set_xy (1,1); mov_bp->grid_add_label ("Z+");
                                mov_bp->set_xy (1,3); mov_bp->grid_add_label ("Z-");
			} else {
                                mov_bp->set_xy (1,2); mov_bp->grid_add_label (i==4 ? "Z-":"X-");
                                mov_bp->set_xy (5,2); mov_bp->grid_add_label (i==4 ? "Z+":"X+");
                                mov_bp->set_xy (1,1); mov_bp->grid_add_label ("Y+");
                                mov_bp->set_xy (1,3); mov_bp->grid_add_label ("Y-");
			}

			if (i!=6) {
				// LEFT
                                mov_bp->set_xy (2,2); mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("seek-left-symbolic", GTK_ICON_SIZE_BUTTON));
				g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_AFM_MOV_XM));
				g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
				g_signal_connect (G_OBJECT (button), "pressed",
						    G_CALLBACK (DSPMoverControl::CmdAction),
						    this);
				g_signal_connect (G_OBJECT (button), "released",
						    G_CALLBACK (DSPMoverControl::StopAction),
						    this);
				{ // pyremote hook
					remote_action_cb *ra = g_new( remote_action_cb, 1);
					ra -> cmd = g_strdup_printf("DSP_CMD_MOV-XM_%d",i);
					ra -> RemoteCb = (void (*)(GtkWidget*, void*))DSPMoverControl::CmdAction;
					ra -> widget = button;
					ra -> data = this;
					gapp->RemoteActionList = g_slist_prepend ( gapp->RemoteActionList, ra );
					gchar *help = g_strconcat ("Remote example: action (\"", ra->cmd, "\"", NULL);
					gtk_widget_set_tooltip_text (button, help);
				}
	//      gtk_widget_add_accelerator (button, "pressed", accel_group,
	//                                  GDK_F1+4*i, (GdkModifierType)0,
	//                                  GTK_ACCEL_VISIBLE);

				// RIGHT
                                //                                button = gtk_button_new_from_icon_name ("seek-right-symbolic", GTK_ICON_SIZE_BUTTON);
                                mov_bp->set_xy (4,2); mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("media-seek-forward-symbolic", GTK_ICON_SIZE_BUTTON));
				g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_AFM_MOV_XP));
				g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
				g_signal_connect (G_OBJECT (button), "pressed",
						    G_CALLBACK (DSPMoverControl::CmdAction),
						    this);
				g_signal_connect (G_OBJECT (button), "released",
						    G_CALLBACK (DSPMoverControl::StopAction),
						    this);

				{ // pyremote hook
					remote_action_cb *ra = g_new( remote_action_cb, 1);
					ra -> cmd = g_strdup_printf("DSP_CMD_MOV-XP_%d",i);
					ra -> RemoteCb = (void (*)(GtkWidget*, void*))DSPMoverControl::CmdAction;
					ra -> widget = button;
					ra -> data = this;
					gapp->RemoteActionList = g_slist_prepend ( gapp->RemoteActionList, ra );
					gchar *help = g_strconcat ("Remote example: action (\"", ra->cmd, "\"", NULL);
					gtk_widget_set_tooltip_text (button, help);
				}
			}
// gtk_widget_get_toplevel()
//      gtk_widget_add_accelerator (button, "pressed", accel_group,
//                                  GDK_F1+4*i, (GdkModifierType)0,
//                                  GTK_ACCEL_VISIBLE);

			// DOWN
                        mov_bp->set_xy (3,3); mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("seek-forward-symbolic", GTK_ICON_SIZE_BUTTON));
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_AFM_MOV_YM));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::CmdAction),
					    this);
			g_signal_connect (G_OBJECT (button), "released",
					    G_CALLBACK (DSPMoverControl::StopAction),
					    this);
			{ // pyremote hook
				remote_action_cb *ra = g_new( remote_action_cb, 1);
				ra -> cmd = g_strdup_printf("DSP_CMD_MOV-YM_%d",i);
				ra -> RemoteCb = (void (*)(GtkWidget*, void*))DSPMoverControl::CmdAction;
				ra -> widget = button;
				ra -> data = this;
				gapp->RemoteActionList = g_slist_prepend ( gapp->RemoteActionList, ra );
				gchar *help = g_strconcat ("Remote example: action (\"", ra->cmd, "\"", NULL);
				gtk_widget_set_tooltip_text (button, help);
			}
//      gtk_widget_add_accelerator (button, "pressed", accel_group,
//                                  GDK_F2+4*i, (GdkModifierType)0,
//                                  GTK_ACCEL_VISIBLE);

    
		}
    
		if(i==4){
			// ========================================
			// Auto App. Control Buttons  GTK_STOCK_GOTO_TOP  GTK_STOCK_GOTO_BOTTOM GTK_STOCK_MEDIA_STOP
			mov_bp->set_xy (6,1); mov_bp->grid_add_label ("Auto Control");

                        mov_bp->set_xy (6,2); mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("approach-symbolic", GTK_ICON_SIZE_BUTTON));
			gtk_widget_set_tooltip_text (button, "Start Auto Approaching. GPIO [Rot, Coarse Mode]");
			if(IS_MOVER_CTRL)
				g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_APPROCH_MOV_XP));
			else
				g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_APPROCH));

			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::CmdAction),
					    this);
      
			{ // remote hook
				remote_action_cb *ra = g_new( remote_action_cb, 1);
				ra -> cmd = g_strdup_printf("DSP_CMD_AUTOAPP");
				ra -> RemoteCb = (void (*)(GtkWidget*, void*))DSPMoverControl::CmdAction;
				ra -> widget = button;
				ra -> data = this;
				gapp->RemoteActionList = g_slist_prepend ( gapp->RemoteActionList, ra );
				gchar *help = g_strconcat ("Remote example: action (\"", ra->cmd, "\"", NULL);
				gtk_widget_set_tooltip_text (button, help);
			}

                        mov_bp->set_xy (6,3); mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("system-off-symbolic", GTK_ICON_SIZE_BUTTON));
			gtk_widget_set_tooltip_text (button, "Cancel Auto Approach.");

			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_CLR_PA));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::CmdAction),
					    this);
      
			{ // remote hook
				remote_action_cb *ra = g_new( remote_action_cb, 1);
				ra -> cmd = g_strdup_printf("DSP_CMD_STOPALL");
				ra -> RemoteCb = (void (*)(GtkWidget*, void*))DSPMoverControl::CmdAction;
				ra -> widget = button;
				ra -> data = this;
				gapp->RemoteActionList = g_slist_prepend ( gapp->RemoteActionList, ra );
				gchar *help = g_strconcat ("Remote example: action (\"", ra->cmd, "\"", NULL);
				gtk_widget_set_tooltip_text (button, help);
			}

                        mov_bp->set_xy (5,3); mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("xymode-symbolic", GTK_ICON_SIZE_BUTTON));
			gtk_widget_set_tooltip_text (button, "Switch GPIO to XY Scan (Main) Mode.");

			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_CLR_PA));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (100));
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::CmdAction),
					    this);
		}
		
		// STEPPER MOTOR controls (29/1/2013 dalibor.sulc@gmail.com) 		
		if(i==6){
			// ========================================
                        mov_bp->set_xy (6,1); mov_bp->grid_add_label ("Stepper Motor");
                        mov_bp->set_xy (6,2); mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("approach-symbolic", GTK_ICON_SIZE_BUTTON));
			gtk_widget_set_tooltip_text (button, "Start Auto Approaching. GPIO [Rot, Coarse Mode]");
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
			g_signal_connect ( G_OBJECT (button), "pressed",
                                           G_CALLBACK (DSPMoverControl::CmdAction),
                                           this);

			if(IS_MOVER_CTRL)
				g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_APPROCH_MOV_XP));
			else
				g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_APPROCH));

      
			{ // remote hook
				remote_action_cb *ra = g_new( remote_action_cb, 1);
				ra -> cmd = g_strdup_printf("DSP_CMD_AUTOAPP");
				ra -> RemoteCb = (void (*)(GtkWidget*, void*))DSPMoverControl::CmdAction;
				ra -> widget = button;
				ra -> data = this;
				gapp->RemoteActionList = g_slist_prepend ( gapp->RemoteActionList, ra );
				gchar *help = g_strconcat ("Remote example: action (\"", ra->cmd, "\"", NULL);
				gtk_widget_set_tooltip_text (button, help);
			}

			mov_bp->set_xy (6,3); mov_bp->grid_add_widget (button = gtk_button_new_from_icon_name ("system-off-symbolic", GTK_ICON_SIZE_BUTTON));
			gtk_widget_set_tooltip_text (button, "Stop Auto Approaching.");
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_CLR_PA));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
			g_signal_connect ( G_OBJECT (button), "pressed",
                                           G_CALLBACK (DSPMoverControl::CmdAction),
                                           this);
      
			{ // remote hook
				remote_action_cb *ra = g_new( remote_action_cb, 1);
				ra -> cmd = g_strdup_printf("DSP_CMD_STOPALL");
				ra -> RemoteCb = (void (*)(GtkWidget*, void*))DSPMoverControl::CmdAction;
				ra -> widget = button;
				ra -> data = this;
				gapp->RemoteActionList = g_slist_prepend ( gapp->RemoteActionList, ra );
				gchar *help = g_strconcat ("Remote example: action (\"", ra->cmd, "\"", NULL);
				gtk_widget_set_tooltip_text (button, help);
			}
		}

                mov_bp->notebook_tab_show_all ();
	}
  
	// ============================================================
	// save List away...
	g_object_set_data( G_OBJECT (window), "MOVER_EC_list", mov_bp->get_ec_list_head ());
	sranger_mk2_hwi_pi.app->RemoteEntryList = g_slist_concat (sranger_mk2_hwi_pi.app->RemoteEntryList, mov_bp->get_remote_list_head ());

        AppWindowInit (NULL); // stage two
        configure_callback (NULL, NULL, this); // configure "false"
        
        set_window_geometry ("dsp-mover-control");
}

void DSPMoverControl::update(){
	g_slist_foreach
		( (GSList*) g_object_get_data( G_OBJECT (window), "MOVER_EC_list"),
		  (GFunc) App::update_ec, NULL
			);
}

void DSPMoverControl::updateDSP(int sliderno){
	PI_DEBUG (DBG_L2, "Hallo DSP ! Mover No:" << sliderno );

	if (DSPPACClass){
		if (sliderno == 200){
			switch (mover_param.MOV_output){
			case AAP_MOVER_XXOFFSET:
				mover_param.wave_out_channel_dsp[0]=mover_param.wave_out_channel[0];
				mover_param.wave_out_channel_dsp[1]=mover_param.wave_out_channel[1];
				break;
			case AAP_MOVER_XYSCAN: // use pre defined basic settings
				mover_param.wave_out_channel_dsp[0]=3;
				mover_param.wave_out_channel_dsp[1]=4;
				break;
			case AAP_MOVER_XYOFFSET:
				mover_param.wave_out_channel_dsp[0]=0;
				mover_param.wave_out_channel_dsp[1]=1;
				break;
			}
			return;
		}
	}
	if (sliderno == 100){
	        // auto offset ZERO
		sranger_common_hwi->SetOffset (0,0); // set
		sranger_common_hwi->SetOffset (0,0); // wait for finish
		if (mover_param.AFM_GPIO_setting != mover_param.GPIO_scan){
			if ( mover_param.GPIO_tmp1 ){ // unmask special bit from value
				mover_param.AFM_GPIO_setting = mover_param.GPIO_scan | mover_param.GPIO_tmp1;
				ExecCmd(DSP_CMD_GPIO_SETUP);
				mover_param.AFM_GPIO_setting = mover_param.GPIO_scan;
				ExecCmd(DSP_CMD_GPIO_SETUP);
			} else if ( mover_param.GPIO_tmp2 ){ // invert action unmask special bit from value
			        mover_param.AFM_GPIO_setting = mover_param.GPIO_scan;
				ExecCmd(DSP_CMD_GPIO_SETUP);
				mover_param.AFM_GPIO_setting = mover_param.GPIO_scan | mover_param.GPIO_tmp2;
				ExecCmd(DSP_CMD_GPIO_SETUP);
			} else {
 			        mover_param.AFM_GPIO_setting = mover_param.GPIO_scan;
			}
		}
                g_settings_set_int (hwi_settings, "mover-gpio-last", mover_param.AFM_GPIO_setting);
		return;
	}

	if (mover_param.AFM_GPIO_setting == mover_param.GPIO_scan 
	    && ((sliderno >= 0 && sliderno < DSP_AFMMOV_MODES)? mover_param.AFM_GPIO_usr_setting[sliderno] : mover_param.MOV_GPIO_setting) != mover_param.GPIO_scan){
	        // auto offset ZERO
		sranger_common_hwi->MovetoXY (0,0); // set
		sranger_common_hwi->SetOffset (0,0); // set
		sranger_common_hwi->SetOffset (0,0); // wait for finish
		
		if ( mover_param.GPIO_tmp1 ){ // unmask special bit from value
		        mover_param.AFM_GPIO_setting = mover_param.GPIO_scan | mover_param.GPIO_tmp1;
			ExecCmd(DSP_CMD_GPIO_SETUP);
			mover_param.AFM_GPIO_setting = mover_param.GPIO_scan;
			ExecCmd(DSP_CMD_GPIO_SETUP);
		} else if ( mover_param.GPIO_tmp2 ){ // invert action unmask special bit from value
		        mover_param.AFM_GPIO_setting = mover_param.GPIO_scan;
			ExecCmd(DSP_CMD_GPIO_SETUP);
			mover_param.AFM_GPIO_setting = mover_param.GPIO_scan | mover_param.GPIO_tmp2;
			ExecCmd(DSP_CMD_GPIO_SETUP);
		} else {
		        mover_param.AFM_GPIO_setting = mover_param.GPIO_scan;
		}
	}
	if (sliderno >= 0 && sliderno < DSP_AFMMOV_MODES){
	        // auto scan XY and offset to ZERO
                sranger_common_hwi->MovetoXY (0,0);
		sranger_common_hwi->SetOffset (0,0); // set
		sranger_common_hwi->SetOffset (0,0); // wait for finish
		mover_param.AFM_Amp   = mover_param.AFM_usrAmp  [sliderno];
		mover_param.AFM_Speed = mover_param.AFM_usrSpeed[sliderno];
		mover_param.AFM_Steps = mover_param.AFM_usrSteps[sliderno];

		if ( mover_param.GPIO_tmp1 ){ // unmask special bit from value
		        if ( mover_param.AFM_GPIO_setting != mover_param.AFM_GPIO_usr_setting[sliderno] ){
			        mover_param.AFM_GPIO_setting = mover_param.AFM_GPIO_usr_setting[sliderno] | mover_param.GPIO_tmp1;
				ExecCmd(DSP_CMD_GPIO_SETUP);
				mover_param.AFM_GPIO_setting = mover_param.AFM_GPIO_usr_setting[sliderno];
			        ExecCmd(DSP_CMD_GPIO_SETUP);
			}
		} else if ( mover_param.GPIO_tmp2 ){ // invert action unmask special bit from value
		        if ( mover_param.AFM_GPIO_setting != mover_param.AFM_GPIO_usr_setting[sliderno] | mover_param.GPIO_tmp2 ){
			        mover_param.AFM_GPIO_setting = mover_param.AFM_GPIO_usr_setting[sliderno];
				ExecCmd(DSP_CMD_GPIO_SETUP);
				mover_param.AFM_GPIO_setting = mover_param.AFM_GPIO_usr_setting[sliderno] | mover_param.GPIO_tmp2;
				ExecCmd(DSP_CMD_GPIO_SETUP);
			}
		} else {
		        mover_param.AFM_GPIO_setting = mover_param.AFM_GPIO_usr_setting[sliderno];
		}
	}
        g_settings_set_int (hwi_settings, "mover-gpio-last", mover_param.AFM_GPIO_setting);
}

int DSPMoverControl::config_mode(GtkWidget *widget, DSPMoverControl *dspc){
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
		dspc->mover_param.MOV_mode = GPOINTER_TO_INT(g_object_get_data( G_OBJECT (widget), "CurveMask")); 
}

int DSPMoverControl::config_waveform(GtkWidget *widget, DSPMoverControl *dspc){
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->mover_param.MOV_waveform_id = GPOINTER_TO_INT(g_object_get_data( G_OBJECT (widget), "CurveId"));
}

int DSPMoverControl::config_output(GtkWidget *widget, DSPMoverControl *dspc){
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
		dspc->mover_param.MOV_output = GPOINTER_TO_INT(g_object_get_data( G_OBJECT (widget), "OutputMask")); 

	((DSPMoverControl*)dspc)->updateDSP(200);
}

int DSPMoverControl::CmdAction(GtkWidget *widget, DSPMoverControl *dspc){
	int idx=-1;
	int cmd;
	PI_DEBUG (DBG_L2, "MoverCrtl::CmdAction " );

	// make sure to update wave output configuration settings
	dspc->updateDSP(200);

	if(IS_MOVER_CTRL)
	{
		idx = GPOINTER_TO_INT(g_object_get_data( G_OBJECT (widget), "MoverNo"));
	}

	if (idx < 10 || idx == 100 || idx == 200)
	{
		dspc->updateDSP(idx);
	}

	cmd = GPOINTER_TO_INT(g_object_get_data( G_OBJECT (widget), "DSP_cmd"));
	if(cmd>0){
		dspc->ExecCmd(DSP_CMD_GPIO_SETUP);
		dspc->ExecCmd(cmd);
	}
	PI_DEBUG (DBG_L2, "cmd=" << cmd << " Mover=" << idx );
	return 0;
}

int DSPMoverControl::StopAction(GtkWidget *widget, DSPMoverControl *dspc){
	PI_DEBUG (DBG_L2, "DSPMoverControl::StopAction" );

	int idx = GPOINTER_TO_INT(g_object_get_data( G_OBJECT (widget), "MoverNo"));
	if (idx == 99)
		dspc->ExecCmd(DSP_CMD_Z0_STOP);
	else
		dspc->ExecCmd(DSP_CMD_CLR_PA);

	return 0;
}

void DSPMoverControl::ChangedNotify(Param_Control* pcs, gpointer dspc){
	int idx=-1;
	gchar *us=pcs->Get_UsrString();
	PI_DEBUG (DBG_L2, "MoverCrtl:: Param Changed: " << us );
	g_free(us);

	if(IS_MOVER_CTRL)
		idx = GPOINTER_TO_INT(pcs->GetEntryData("MoverNo"));

        ((DSPMoverControl*)dspc)->updateDSP(idx);
}

void DSPMoverControl::ChangedWaveOut(Param_Control* pcs, gpointer dspc){
	int idx = GPOINTER_TO_INT(pcs->GetEntryData("WAVE_OUT_CH"));

	PI_DEBUG (DBG_L2, "DSPMoverControl::ChangedWaveOut WAVE[" << idx << "] = CH "
                  << ((DSPMoverControl*)dspc)->mover_param.wave_out_channel[idx]
                  );

	((DSPMoverControl*)dspc)->updateDSP(200);
}

void DSPMoverControl::ExecCmd(int cmd){
	PI_DEBUG (DBG_L2, "DSPMoverControl::ExecCmd ==> >" << cmd);

        //<< " Amp=" << sranger_common_hwi->mover_param.AFM_Amp
        //<< " Speed=" << sranger_common_hwi->mover_param.AFM_Speed
        //<< " Steps=" << sranger_common_hwi->mover_param.AFM_Steps
        //<< " GPIO=0x" << std::hex << sranger_common_hwi->mover_param.AFM_GPIO_setting << std::dec
                
        sranger_common_hwi->ExecCmd (cmd);
}
