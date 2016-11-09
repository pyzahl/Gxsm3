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

/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

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

#include "gxsm/action_id.h"

#include "dsp-pci32/xsm/xsmcmd.h"

#include "sranger_hwi_control.h"
#include "sranger_hwi.h"
#include "../plug-ins/hard/modules/sranger_ioctl.h"

#define UTF8_DEGREE    "\302\260"
#define UTF8_MU        "\302\265"
#define UTF8_ANGSTROEM "\303\205"


// override, always MoverCtrl.
#ifdef IS_MOVER_CTRL
# undef IS_MOVER_CTRL
# define IS_MOVER_CTRL TRUE
#else
# define IS_MOVER_CTRL TRUE
#endif


extern GxsmPlugin sranger_hwi_pi;
extern DSPControl *DSPControlClass;
extern sranger_hwi_dev *sranger_hwi_hardware;

DSPMoverControl::DSPMoverControl ()
{
	XsmRescourceManager xrm("sranger_hwi_control");

	xrm.Get("MOV_Ampl", &mover_param.MOV_Ampl, "1.0");
	xrm.Get("MOV_WavePeriod", &mover_param.MOV_WavePeriod, "4.0");
	xrm.Get("MOV_Steps", &mover_param.MOV_Steps, "5.0");
	xrm.Get("AFM_Amp", &mover_param.AFM_Amp);
	xrm.Get("AFM_WavePeriod", &mover_param.AFM_WavePeriod);
	xrm.Get("AFM_Steps", &mover_param.AFM_Steps);

  	// defaults for AFM Mover (hardware SICAF)
	// [0] Besocke XY
	// [1] Besocke rotation
	// [2] Besocke PSD
	// [3] Besocke lens
  
	xrm.Get("AFM_usrAmp0", &mover_param.AFM_usrAmp[0], "1.0");
	xrm.Get("AFM_usrAmp1", &mover_param.AFM_usrAmp[1], "1.0");
	xrm.Get("AFM_usrAmp2", &mover_param.AFM_usrAmp[2], "1.0");
	xrm.Get("AFM_usrAmp3", &mover_param.AFM_usrAmp[3], "1.0");
	xrm.Get("AFM_usrWavePeriod0", &mover_param.AFM_usrWavePeriod[0], "3");
	xrm.Get("AFM_usrWavePeriod1", &mover_param.AFM_usrWavePeriod[1], "3");
	xrm.Get("AFM_usrWavePeriod2", &mover_param.AFM_usrWavePeriod[2], "5");
	xrm.Get("AFM_usrWavePeriod3", &mover_param.AFM_usrWavePeriod[3], "5");
	xrm.Get("AFM_usrSteps0", &mover_param.AFM_usrSteps[0], "10");
	xrm.Get("AFM_usrSteps1", &mover_param.AFM_usrSteps[1], "2");
	xrm.Get("AFM_usrSteps2", &mover_param.AFM_usrSteps[2], "100");
	xrm.Get("AFM_usrSteps3", &mover_param.AFM_usrSteps[3], "100");

	xrm.Get("MOV_output", &mover_param.MOV_output, "0");
	xrm.Get("MOV_waveform_id", &mover_param.MOV_waveform_id, "0");
	xrm.Get("MOV_mode", &mover_param.MOV_mode, "0");
	xrm.Get("AUTO_final_delay", &mover_param.final_delay, "50");

	mover_param.MOV_wave_len=1024;
	for (int i=0; i < MOV_MAXWAVELEN; ++i)
		mover_param.MOV_waveform[i] = (short)0;


	Unity    = new UnitObj(" "," ");
	Volt     = new UnitObj("V","V");
	Time     = new UnitObj("ms","ms");
	Length   = new UnitObj("nm","nm");

	create_folder ();
}

// duration in ms
// amp in V SR out (+/-2.05V max)
void DSPMoverControl::create_waveform (double amp, double duration){
#define SR_VFAC    (32767./2.05) // SRanger max Volt out is 2.05V

	guint64 wave_length = (guint64)round (DSPControlClass->frq_ref*duration*1e-3);

	mover_param.MOV_wave_speed = 1;
	while (wave_length > MOV_MAXWAVELEN){
		wave_length /= 2;
		mover_param.MOV_wave_speed *= 2;
	}

	mover_param.MOV_wave_len = (int)wave_length;

	if (mover_param.MOV_wave_len < 2)
		mover_param.MOV_wave_len = 2;


	std::cout << "DSPMoverControl::create_waveform: " << mover_param.MOV_wave_speed << " * " << mover_param.MOV_wave_len
		  << " MOV_output: " << mover_param.MOV_output << " MOV_waveform_id: " << mover_param.MOV_waveform_id
		  << std::endl;

	double n = (double)mover_param.MOV_wave_len;
	double n2 = n/2.;
	double t=0.;
	
	switch (mover_param.MOV_waveform_id){
	case MOV_WAVE_SAWTOOTH:
		for (int i=0; i < mover_param.MOV_wave_len; ++i)
			mover_param.MOV_waveform[i] = sranger_hwi_hardware->int_2_sranger_int ((short)round (SR_VFAC*amp*((double)i-n2)/n2));
		break;
	case MOV_WAVE_SINE:
		for (int i=0; i < mover_param.MOV_wave_len; ++i)
			mover_param.MOV_waveform[i] = sranger_hwi_hardware->int_2_sranger_int((short)round (SR_VFAC*amp*sin ((double)i*2.*M_PI/n)));
		break;
	case MOV_WAVE_USER:
	{
		int i;
		int i_on = mover_param.MOV_wave_len * 150 / (150+300);
		for (i=0; i < i_on; ++i)
			mover_param.MOV_waveform[i] = sranger_hwi_hardware->int_2_sranger_int((short)round (SR_VFAC*amp*(1.)));
		for (; i < mover_param.MOV_wave_len; ++i)
			mover_param.MOV_waveform[i] = sranger_hwi_hardware->int_2_sranger_int((short)round (SR_VFAC*amp*(0.)));
		break;
	}
	case MOV_WAVE_USER_TTL:
	{
		int i;
		// generate any 8bit pattern for CR Port 4 (is masked by 0xff)
		int i_on = mover_param.MOV_wave_len * 150 / (150+300);
		for (i=0; i < i_on; ++i)
			mover_param.MOV_waveform[i] = sranger_hwi_hardware->int_2_sranger_int(0xff);
		for (; i < mover_param.MOV_wave_len; ++i)
			mover_param.MOV_waveform[i] = sranger_hwi_hardware->int_2_sranger_int(0x00);
		break;
	}
	case MOV_WAVE_CYCLO:
	case MOV_WAVE_CYCLO_PL:
	case MOV_WAVE_CYCLO_MI:
		t=0.;
		for (int i=0; i < (mover_param.MOV_wave_len - 1) ; ++i){
			double dt = 1./(mover_param.MOV_wave_len - 1);
			double a = 1.;
			switch (mover_param.MOV_waveform_id){
			case MOV_WAVE_CYCLO_PL:
				mover_param.MOV_waveform[i] = sranger_hwi_hardware->int_2_sranger_int((short)round(SR_VFAC*amp*a*( (t*t*t*t)) * (-1) ));
				break;	
			case MOV_WAVE_CYCLO_MI:
				mover_param.MOV_waveform[i] = sranger_hwi_hardware->int_2_sranger_int((short)round(SR_VFAC*amp*a*( (t*t*t*t) - 1 ) * (-1) ));
				break;
			default:
				mover_param.MOV_waveform[i] = sranger_hwi_hardware->int_2_sranger_int((short)round(SR_VFAC*amp*(a * cos (t))));
				break;
			}
			t += dt;
		}
		break;
	case MOV_WAVE_CYCLO_IPL:
	case MOV_WAVE_CYCLO_IMI:
		t=0.;
		for (int i=0; i < mover_param.MOV_wave_len; ++i){
			double dt = 1./mover_param.MOV_wave_len;
			double a = 1.;
			switch (mover_param.MOV_waveform_id){
			case MOV_WAVE_CYCLO_IPL:
				mover_param.MOV_waveform[i] = sranger_hwi_hardware->int_2_sranger_int((short)round(SR_VFAC*amp*a*( (t*t*t*t) - 1 ) * (1) ));
				break;	
			case MOV_WAVE_CYCLO_IMI:
				mover_param.MOV_waveform[i] = sranger_hwi_hardware->int_2_sranger_int((short)round(SR_VFAC*amp*a*( (t*t*t*t)) * (1) ));
				break;
			}
				t += dt;
		}
		break;
	}
}


DSPMoverControl::~DSPMoverControl (){
	XsmRescourceManager xrm("sranger_hwi_control");

	xrm.Put("MOV_Ampl", mover_param.MOV_Ampl);
	xrm.Put("MOV_WavePeriod", mover_param.MOV_WavePeriod);
	xrm.Put("MOV_Steps", mover_param.MOV_Steps);

	xrm.Put("AFM_Amp", mover_param.AFM_Amp);
	xrm.Put("AFM_WavePeriod", mover_param.AFM_WavePeriod);
	xrm.Put("AFM_Steps", mover_param.AFM_Steps);
	xrm.Put("AFM_usrAmp0", mover_param.AFM_usrAmp[0]);
	xrm.Put("AFM_usrAmp1", mover_param.AFM_usrAmp[1]);
	xrm.Put("AFM_usrAmp2", mover_param.AFM_usrAmp[2]);
	xrm.Put("AFM_usrAmp3", mover_param.AFM_usrAmp[3]);
	xrm.Put("AFM_usrWavePeriod0", mover_param.AFM_usrWavePeriod[0]);
	xrm.Put("AFM_usrWavePeriod1", mover_param.AFM_usrWavePeriod[1]);
	xrm.Put("AFM_usrWavePeriod2", mover_param.AFM_usrWavePeriod[2]);
	xrm.Put("AFM_usrWavePeriod3", mover_param.AFM_usrWavePeriod[3]);
	xrm.Put("AFM_usrSteps0", mover_param.AFM_usrSteps[0]);
	xrm.Put("AFM_usrSteps1", mover_param.AFM_usrSteps[1]);
	xrm.Put("AFM_usrSteps2", mover_param.AFM_usrSteps[2]);
	xrm.Put("AFM_usrSteps3", mover_param.AFM_usrSteps[3]);

	xrm.Put("MOV_output", mover_param.MOV_output);
	xrm.Put("MOV_waveform_id", mover_param.MOV_waveform_id);
	xrm.Put("MOV_mode", mover_param.MOV_mode);
	xrm.Put("AUTO_final_delay", mover_param.final_delay);

	delete Length;
	delete Time;
	delete Volt;
	delete Unity;
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
			

#define KEY_LAB(L,A,B,C,D) \
			lab = gtk_label_new (L);\
			gtk_widget_show (lab);\
			gtk_table_attach (GTK_TABLE (tab), lab, A, B, C, D,\
					  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),\
					  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),\
					  FALSE, FALSE)

void DSPMoverControl::create_folder (){
	GSList *EC_list=NULL;

	Gtk_EntryControl *ec;

	GtkWidget *vbox_param, *hbox_param, *hbox, *vbox_param2, *frame_param, *frame_param2;
	GtkWidget *input;
	GtkWidget *notebook;
	GtkWidget *MoverCrtl;
	GtkWidget *tab, *button, *img, *lab;
	GtkAccelGroup *accel_group=NULL;

	if( IS_MOVER_CTRL ){
		accel_group = gtk_accel_group_new ();
		AppWindowInit(MOV_MOVER_TITLE);
	}
	else
		AppWindowInit(MOV_SLIDER_TITLE);

	set_window_geometry ("dsp-mover-control");
	
	notebook = gtk_notebook_new ();
	gtk_widget_show (notebook);
	gtk_grid_attach (GTK_GRID (v_grid), notebook, 1,1, 1,1);

	const char *MoverNames[] = { "Besocke X&Y", "Rotation", "PSD", "Lens", "Auto", "Config", NULL};

	Gtk_EntryControl *Ampl, *Spd, *Stp;
	int i,itab;
	for(itab=i=0; MoverNames[i]; ++i){
		if( IS_SLIDER_CTRL && i < 4 ) continue;
		PI_DEBUG (DBG_L2, "DSPMoverControl::SiCaf - Mover:" << MoverNames[i]);

		vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_widget_show (vbox_param);
		MoverCrtl = gtk_label_new (MoverNames[i]);
		gtk_widget_show (MoverCrtl);
		itab++;
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox_param, MoverCrtl);

		if( i==5 ){
			GtkWidget *radiobutton;
			GSList    *radiogroup;

			frame_param = gtk_frame_new ("Output Configuration");
			gtk_widget_show (frame_param);
			gtk_container_add (GTK_CONTAINER (vbox_param), frame_param);
			
			lab = gtk_label_new ("*) Parabolic: DSP gen. wave, duration means speed here");
			gtk_box_pack_start (GTK_BOX (vbox_param), lab, FALSE, FALSE, 0);
			gtk_widget_show (lab);


			input = mygtk_create_input("Auto App. Delay", vbox_param, hbox_param);
			ec = new Gtk_EntryControl (Time, MLD_WERT_NICHT_OK, &mover_param.final_delay, 0., 10000., "4.1f", input);
			ec->Set_ChangeNoticeFkt(DSPMoverControl::ChangedNotify, this);
			EC_list = g_slist_prepend( EC_list, ec);

			hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			gtk_widget_show (hbox);
			gtk_container_add (GTK_CONTAINER (frame_param), hbox);

			frame_param2 = gtk_frame_new ("Curve Mode");
			gtk_widget_show (frame_param2);
			gtk_container_add (GTK_CONTAINER (hbox), frame_param2);

			vbox_param2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
			gtk_widget_show (vbox_param2);
			gtk_container_add (GTK_CONTAINER (frame_param2), vbox_param2);

			radiobutton = gtk_radio_button_new_with_label( NULL, "Parabolic *)"); // DSP build in mode
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", (gpointer)0); // 0 is default AAP_MOVER_
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
					  G_CALLBACK (DSPMoverControl::config_mode), this);

			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), mover_param.MOV_mode == 0 ? 1:0);
			
			radiobutton =  gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Sawtooth"); // arbitrary waveform
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", (gpointer)AAP_MOVER_WAVE);
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", (gpointer)MOV_WAVE_SAWTOOTH);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
			
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_mode == AAP_MOVER_WAVE && mover_param.MOV_waveform_id == MOV_WAVE_SAWTOOTH) ? 1:0);

			radiobutton =  gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Sine"); // arbitrary waveform
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", (gpointer)AAP_MOVER_WAVE);
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", (gpointer)MOV_WAVE_SINE);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
			
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_mode == AAP_MOVER_WAVE && mover_param.MOV_waveform_id == MOV_WAVE_SINE) ? 1:0);

			radiobutton =  gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Cyclo"); // arbitrary waveform
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", (gpointer)AAP_MOVER_WAVE);
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", (gpointer)MOV_WAVE_CYCLO);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
			
// ==
			radiobutton =  gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Cyclo+"); // arbitrary waveform
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", (gpointer)AAP_MOVER_WAVE);
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", (gpointer)MOV_WAVE_CYCLO_PL);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
			
			radiobutton =  gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Cyclo-"); // arbitrary waveform
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", (gpointer)AAP_MOVER_WAVE);
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", (gpointer)MOV_WAVE_CYCLO_MI);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
			
			radiobutton =  gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Inv Cyclo+"); // arbitrary waveform
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", (gpointer)AAP_MOVER_WAVE);
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", (gpointer)MOV_WAVE_CYCLO_IPL);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
			
			radiobutton =  gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: Inv Cyclo-"); // arbitrary waveform
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", (gpointer)AAP_MOVER_WAVE);
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", (gpointer)MOV_WAVE_CYCLO_IMI);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
			
			radiobutton =  gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave: User"); // arbitrary waveform
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", (gpointer)AAP_MOVER_WAVE);
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", (gpointer)MOV_WAVE_USER);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_waveform), this);
			
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_mode == AAP_MOVER_WAVE && mover_param.MOV_waveform_id == MOV_WAVE_SINE) ? 1:0);

// == user wave &0xff mapped to CR Port 4 Out
			radiobutton =  gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "Wave@CoolRunner Port4");
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
// 			gtk_widget_set_sensitive (radiobutton, FALSE);
			g_object_set_data (G_OBJECT (radiobutton), "CurveMask", (gpointer)(AAP_MOVER_PULSE | AAP_MOVER_WAVE));
			g_object_set_data (G_OBJECT (radiobutton), "CurveId", (gpointer)MOV_WAVE_USER_TTL);
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_mode), this);
			
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_mode == AAP_MOVER_PULSE) ? 1:0);

// ==================================================

			frame_param2 = gtk_frame_new ("Output on");
			gtk_widget_show (frame_param2);
			gtk_container_add (GTK_CONTAINER (hbox), frame_param2);

			vbox_param2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
			gtk_widget_show (vbox_param2);
			gtk_container_add (GTK_CONTAINER (frame_param2), vbox_param2);

			radiobutton = gtk_radio_button_new_with_label( NULL, "XY-XY-Offset");
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_XYOFFSET));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_output), this);
			
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_output == AAP_MOVER_XYOFFSET) ? 1:0);
			
			radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "XX-XY-Offset");
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_XXOFFSET));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_output), this);
			
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_output == AAP_MOVER_XXOFFSET) ? 1:0);
			
			radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "XY-XY-Scan");
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_XYSCAN));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_output), this);
			
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_output == AAP_MOVER_XYSCAN) ? 1:0);

			radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "X-Motor");
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_XYMOTOR));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_output), this);
			
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_output == AAP_MOVER_XYMOTOR) ? 1:0);

			radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "X-Z-Scan Add");
			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
			gtk_widget_show (radiobutton);
			g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_ZSCANADD));
 			g_signal_connect (G_OBJECT (radiobutton), "clicked",
 					    G_CALLBACK (DSPMoverControl::config_output), this);
			
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), (mover_param.MOV_output == AAP_MOVER_ZSCANADD) ? 1:0);

 			radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), "CoolRunner IOxx");
 			gtk_box_pack_start (GTK_BOX (vbox_param2), radiobutton, FALSE, FALSE, 0);
 			gtk_widget_show (radiobutton);
 			gtk_widget_set_sensitive (radiobutton, FALSE);
//  			g_object_set_data (G_OBJECT (radiobutton), "OutputMask", GINT_TO_POINTER (AAP_MOVER_CRBIT_XX));
//   			g_signal_connect (G_OBJECT (radiobutton), "clicked",
//   					    G_CALLBACK (DSPMoverControl::config_output), this);
		

			continue;
		}

		if( i<4 ){
			frame_param = gtk_frame_new ("Mover Timing");
			gtk_widget_show (frame_param);
			gtk_container_add (GTK_CONTAINER (vbox_param), frame_param);
			
			vbox_param2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
			gtk_widget_show (vbox_param2);
			gtk_container_add (GTK_CONTAINER (frame_param), vbox_param2);

  			input = mygtk_create_input  ("Amplitude", vbox_param2, hbox_param);
			g_object_set_data( G_OBJECT (input), "MoverNo", GINT_TO_POINTER (i));
			Ampl = new Gtk_EntryControl (Volt, MLD_WERT_NICHT_OK, &mover_param.AFM_usrAmp[i], -20., 20., "5.2f", input);
			Ampl->Set_ChangeNoticeFkt   (DSPMoverControl::ChangedNotify, this);
			EC_list = g_slist_prepend   ( EC_list, Ampl);
      
			input = mygtk_create_input  ("Wave Period", vbox_param2, hbox_param);
			g_object_set_data( G_OBJECT (input), "MoverNo", GINT_TO_POINTER (i));
			Spd = new Gtk_EntryControl  (Time, MLD_WERT_NICHT_OK, &mover_param.AFM_usrWavePeriod[i], 0.5, 1000., "5.1f", input);
			Spd->Set_ChangeNoticeFkt    (DSPMoverControl::ChangedNotify, this);
			EC_list = g_slist_prepend   ( EC_list, Spd);
      
			input = mygtk_create_input  ("Max. Steps", vbox_param2, hbox_param);
			g_object_set_data( G_OBJECT (input), "MoverNo", GINT_TO_POINTER (i));
			Stp = new Gtk_EntryControl  (Unity, MLD_WERT_NICHT_OK, &mover_param.AFM_usrSteps[i], 1., 5000., "4.0f", input);
			Stp->Set_ChangeNoticeFkt    (DSPMoverControl::ChangedNotify, this);
			EC_list = g_slist_prepend   ( EC_list, Stp);
		}
		if(i==4){
			if( IS_MOVER_CTRL ){
				frame_param = gtk_frame_new ("Mover Timing");
				gtk_widget_show (frame_param);
				gtk_container_add (GTK_CONTAINER (vbox_param), frame_param);
        
				vbox_param2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
				gtk_widget_show (vbox_param2);
				gtk_container_add (GTK_CONTAINER (frame_param), vbox_param2);
      
				input = mygtk_create_input("Amplitude", vbox_param2, hbox_param);
				ec = new Gtk_EntryControl (Volt, MLD_WERT_NICHT_OK, &mover_param.MOV_Ampl, -20., 20., "5.2f", input);
				ec->Set_ChangeNoticeFkt(DSPMoverControl::ChangedNotify, this);
				EC_list = g_slist_prepend( EC_list, ec);
        
				input = mygtk_create_input("Wave Period", vbox_param2, hbox_param);
				ec = new Gtk_EntryControl (Time, MLD_WERT_NICHT_OK, &mover_param.MOV_WavePeriod, 0.5, 1000., "5.1f", input);
				ec->Set_ChangeNoticeFkt(DSPMoverControl::ChangedNotify, this);
				EC_list = g_slist_prepend( EC_list, ec);
        
				input = mygtk_create_input("Max. Steps", vbox_param2, hbox_param);
				ec = new Gtk_EntryControl (Unity, MLD_WERT_NICHT_OK, &mover_param.MOV_Steps, 1., 5000., "4.0f", input);
				ec->Set_ChangeNoticeFkt(DSPMoverControl::ChangedNotify, this);
				EC_list = g_slist_prepend( EC_list, ec);
			}
		}
		// ========================================
    
		frame_param = gtk_frame_new ("Direction & Action Control");
		gtk_widget_show (frame_param);
		gtk_container_add (GTK_CONTAINER (vbox_param), frame_param);

		tab = gtk_table_new (6, 3, FALSE);
		gtk_widget_show (tab);
		gtk_container_add (GTK_CONTAINER (frame_param), tab);
    
		// ========================================
		// Direction Buttons

		if( IS_MOVER_CTRL ){
			// STOP
			button = gtk_button_new_from_icon_name ("process-stopall-symbolic", GTK_ICON_SIZE_BUTTON);
			gtk_container_add (GTK_CONTAINER (button), img);

			gtk_table_attach (GTK_TABLE (tab), button, 2, 3, 1, 2,
					  (GtkAttachOptions)(GTK_FILL),
					  (GtkAttachOptions)(GTK_FILL),
					  FALSE, FALSE);
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::StopAction),
					    this);
			// UP
			button = gtk_button_new_from_icon_name ("seek-backwards-symbolic", GTK_ICON_SIZE_BUTTON);
			gtk_widget_show (button);
			gtk_table_attach (GTK_TABLE (tab), button, 2, 3, 0, 1,
					  (GtkAttachOptions)(GTK_FILL),
					  (GtkAttachOptions)(GTK_FILL),
					  FALSE, FALSE);
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
//      gtk_widget_add_accelerator (button, "pressed", accel_group,
//                                  GDK_F3+4*i, (GdkModifierType)0,
//                                  GTK_ACCEL_VISIBLE);
      
			KEY_LAB (i==4?"Z-":"X-",0,1,1,2);
			KEY_LAB (i==4?"Z+":"X+",4,5,1,2);
			KEY_LAB ("Y+",0,1,0,1);
			KEY_LAB ("Y-",0,1,2,3);
			// LEFT
			button = gtk_button_new_from_icon_name ("seek-left-symbolic", GTK_ICON_SIZE_BUTTON);
			gtk_widget_show (button);
			gtk_table_attach (GTK_TABLE (tab), button, 1, 2, 1, 2,
					  (GtkAttachOptions)(GTK_FILL),
					  (GtkAttachOptions)(GTK_FILL),
					  FALSE, FALSE);
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_AFM_MOV_XM));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::CmdAction),
					    this);
			g_signal_connect (G_OBJECT (button), "released",
					    G_CALLBACK (DSPMoverControl::StopAction),
					    this);
//      gtk_widget_add_accelerator (button, "pressed", accel_group,
//                                  GDK_F1+4*i, (GdkModifierType)0,
//                                  GTK_ACCEL_VISIBLE);

			// RIGHT
			button = gtk_button_new_from_icon_name ("seek-right-symbolic", GTK_ICON_SIZE_BUTTON);
			gtk_widget_show (button);
			gtk_table_attach (GTK_TABLE (tab), button, 3, 4, 1, 2,
					  (GtkAttachOptions)(GTK_FILL),
					  (GtkAttachOptions)(GTK_FILL),
					  FALSE, FALSE);
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_AFM_MOV_XP));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::CmdAction),
					    this);
			g_signal_connect (G_OBJECT (button), "released",
					    G_CALLBACK (DSPMoverControl::StopAction),
					    this);

// gtk_widget_get_toplevel()
//      gtk_widget_add_accelerator (button, "pressed", accel_group,
//                                  GDK_F1+4*i, (GdkModifierType)0,
//                                  GTK_ACCEL_VISIBLE);

			// DOWN
			button = gtk_button_new_from_icon_name ("seek-forward-symbolic", GTK_ICON_SIZE_BUTTON);
			gtk_widget_show (button);
			gtk_table_attach (GTK_TABLE (tab), button, 2, 3, 2, 3,
					  (GtkAttachOptions)(GTK_FILL),
					  (GtkAttachOptions)(GTK_FILL),
					  FALSE, FALSE);
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_AFM_MOV_YM));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::CmdAction),
					    this);
			g_signal_connect (G_OBJECT (button), "released",
					    G_CALLBACK (DSPMoverControl::StopAction),
					    this);
//      gtk_widget_add_accelerator (button, "pressed", accel_group,
//                                  GDK_F2+4*i, (GdkModifierType)0,
//                                  GTK_ACCEL_VISIBLE);

    
		}
		// ========================================
		// used to auto-center ...
		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_widget_show (hbox);
		gtk_table_attach (GTK_TABLE (tab), hbox, 0, 1, 1, 2,
				  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				  (GtkAttachOptions)(GTK_FILL),
				  FALSE, FALSE);
    
		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_widget_show (hbox);
		gtk_table_attach (GTK_TABLE (tab), hbox, 4, 5, 1, 2,
				  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				  (GtkAttachOptions)(GTK_FILL),
				  FALSE, FALSE);
    
		if(i==4){
			// ========================================
			// Auto App. Control Buttons  GTK_STOCK_GOTO_TOP  GTK_STOCK_GOTO_BOTTOM GTK_STOCK_MEDIA_STOP
			lab = gtk_label_new ("Auto Control");
			gtk_widget_show (lab);
			gtk_table_attach (GTK_TABLE (tab), lab, 5, 6, 0, 1,
					  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					  FALSE, FALSE);

			button = gtk_button_new ();
			gtk_widget_set_size_request (button, ARROW_SIZE, ARROW_SIZE);
			gtk_widget_show (button);

			img = gtk_image_new_from_stock ("gtk-connect", GTK_ICON_SIZE_BUTTON);
			gtk_widget_show (img);
			gtk_container_add (GTK_CONTAINER (button), img);

			gtk_table_attach (GTK_TABLE (tab), button, 5, 6, 1, 2,
					  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					  FALSE, FALSE);
			if(IS_MOVER_CTRL)
				g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_APPROCH_MOV_XP));
			else
				g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_APPROCH));

			g_object_set_data( G_OBJECT (button), "MoverNo", (gpointer)i);
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::CmdAction),
					    this);
      
			button = gtk_button_new ();
			gtk_widget_set_size_request (button, ARROW_SIZE, ARROW_SIZE);
			gtk_widget_show (button);

			img = gtk_image_new_from_stock ("gtk-stop", GTK_ICON_SIZE_BUTTON);
			gtk_widget_show (img);
			gtk_container_add (GTK_CONTAINER (button), img);

			gtk_table_attach (GTK_TABLE (tab), button, 5, 6, 2, 3,
					  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
					  FALSE, FALSE);
			g_object_set_data( G_OBJECT (button), "DSP_cmd", GINT_TO_POINTER (DSP_CMD_CLR_PA));
			g_object_set_data( G_OBJECT (button), "MoverNo", GINT_TO_POINTER (i));
			g_signal_connect (G_OBJECT (button), "pressed",
					    G_CALLBACK (DSPMoverControl::CmdAction),
					    this);
		}
    
	}
  
	// ============================================================
	// save List away...
	g_object_set_data( G_OBJECT (window), "MOVER_EC_list", EC_list);


}

void DSPMoverControl::update(){
	g_slist_foreach
		( (GSList*) g_object_get_data( G_OBJECT (window), "MOVER_EC_list"),
		  (GFunc) App::update_ec, NULL
			);
}

void DSPMoverControl::updateDSP(int sliderno){
	PI_DEBUG (DBG_L2, "Hallo DSP ! Mover No:" << sliderno );
	if(sliderno >= 0 && sliderno < 4){
		mover_param.AFM_Amp   = mover_param.AFM_usrAmp  [sliderno];
		mover_param.AFM_WavePeriod = mover_param.AFM_usrWavePeriod[sliderno];
		mover_param.AFM_Steps = mover_param.AFM_usrSteps[sliderno];
	}else{
		mover_param.AFM_Amp   = mover_param.MOV_Ampl;
		mover_param.AFM_WavePeriod = mover_param.MOV_WavePeriod;
		mover_param.AFM_Steps = mover_param.MOV_Steps;
	}
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
}

int DSPMoverControl::CmdAction(GtkWidget *widget, DSPMoverControl *dspc){
	int idx=-1;
	int cmd;
	PI_DEBUG (DBG_L2, "MoverCrtl::CmdAction " );
	if(IS_MOVER_CTRL)
		idx = GPOINTER_TO_INT(g_object_get_data( G_OBJECT (widget), "MoverNo"));
	dspc->updateDSP(idx);
	cmd = GPOINTER_TO_INT(g_object_get_data( G_OBJECT (widget), "DSP_cmd"));
	if(cmd>0)
		dspc->ExecCmd(cmd);
	PI_DEBUG (DBG_L2, "cmd=" << cmd << " Mover=" << idx );
	return 0;
}

int DSPMoverControl::StopAction(GtkWidget *widget, DSPMoverControl *dspc){
	PI_DEBUG (DBG_L2, "DSPMoverControl::StopAction" );
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

void DSPMoverControl::ExecCmd(int cmd){
	sranger_hwi_hardware->ExecCmd(cmd);
}
