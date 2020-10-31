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

#include "gxsm3/action_id.h"

#include "dsp-pci32/xsm/xsmcmd.h"

#include "sranger_hwi_control.h"
#include "sranger_hwi.h"
#include "../plug-ins/hard/modules/sranger_ioctl.h"

#define UTF8_DEGREE    "\302\260"
#define UTF8_MU        "\302\265"
#define UTF8_ANGSTROEM "\303\205"

extern GxsmPlugin sranger_hwi_pi;
extern sranger_hwi_dev *sranger_hwi_hardware;

#define X_SOURCE_MSK 0x10000000 // select for X-mapping
#define P_SOURCE_MSK 0x20000000 // select for plotting
#define A_SOURCE_MSK 0x40000000 // select for Avg plotting
#define S_SOURCE_MSK 0x80000000 // select for Sec plotting

// CoolRunner Counter w code -- calibration --
// -------------------------------------------
// for 1.000000MHz GateCount# read counts are
// --- #100   #10000           VHD gate count multiplier   GateTime range
// M3  9      899              x1                           0.111us .. 7.10ms 
// M5  44884  449874           x500                        55.5  us .. 3.55 s 
// M9  449889 44988874         x50000                       5.5  ms .. 355.2s

// ; 44988874/500000000*1e6*100
//         8997774.8
// ; 1/(44988874/500000000*1e6*100)
//         ~0.00000011113858950993
// ; 1/(44988874/500000000*1e6*100)*1e9
//         ~111.13858950993083312110       MHz GateRef <=> 89.9777ns

// approx 90ns je ref clock cycle (approx 111MHz)
#define CPLD_GATEREF 89.977748e-9



#define REMOTE_PREFIX "DSP_"
#define ADD_EC_WITH_SCALE(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST) \
        do{\
    	            GtkWidget *input = mygtk_create_spin_input(N_(LABEL), vbox_param, hbox_param);\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    SetupScale(ec->GetAdjustment(), hbox_param);\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
	}while(0)
#define ADD_EC_WITH_SCALE_XTERN(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST, EXTERN) \
        do{\
    	            GtkWidget *input = mygtk_create_spin_input(N_(LABEL), vbox_param, hbox_param);\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    SetupScale(ec->GetAdjustment(), hbox_param);\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    EXTERN; \
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
	}while(0)
#define ADD_EC_WITH_SCALE_FLIST(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST, FREEZE_LIST) \
        do{\
    	            GtkWidget *input = mygtk_create_spin_input(N_(LABEL), vbox_param, hbox_param);\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    SetupScale(ec->GetAdjustment(), hbox_param);\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
		    FREEZE_LIST = g_slist_prepend( FREEZE_LIST, ec);\
	}while(0)
#define ADD_EC_WITH_SCALE_FLIST_L(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST, FREEZE_LIST) \
        do{\
    	            GtkWidget *input = mygtk_create_spin_input(N_(LABEL), vbox_param, hbox_param);\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    SetupScale(ec->GetAdjustment(), hbox_param);\
		    ec->set_log (0);\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
		    FREEZE_LIST = g_slist_prepend( FREEZE_LIST, ec);\
	}while(0)
#define ADD_EC_SPIN(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST) \
        do{\
    	            GtkWidget *input = mygtk_create_spin_input(N_(LABEL), vbox_param, hbox_param);\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
	}while(0)
#define ADD_EC_SPIN_LIST(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST, LIST) \
        do{\
    	            GtkWidget *input = mygtk_create_spin_input(N_(LABEL), vbox_param, hbox_param);\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
		    LIST = g_slist_prepend (LIST, input);		\
		    LIST = g_slist_prepend (LIST, hbox_param);		\
	}while(0)
#define ADD_EC_SPIN_I(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST, INFO) \
        do{\
    	            GtkWidget *input = mygtk_create_spin_input(N_(LABEL), vbox_param, hbox_param);\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    ec->set_info (N_(INFO));					\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
	}while(0)

#define APP_EC_SPIN(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST) \
        do{\
    	            GtkWidget *input = mygtk_add_spin(hbox_param);\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
	}while(0)
#define APP_EC_SPIN_I(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST, INFO) \
        do{\
    	            GtkWidget *input = mygtk_add_spin(hbox_param);\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    ec->set_info (N_(INFO));					\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
	}while(0)
#define APP_EC_SPIN_LIST(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST, LIST) \
        do{\
    	            GtkWidget *input = mygtk_add_spin(hbox_param);\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
		    LIST = g_slist_prepend (LIST, input);		\
	}while(0)
#define APP_EC_SPIN_LIST_I(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST, LIST, INFO) \
        do{\
    	            GtkWidget *input = mygtk_add_spin(hbox_param);\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    ec->set_info (N_(INFO));					\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
		    LIST = g_slist_prepend (LIST, input);		\
	}while(0)
#define APP_EC_SPIN_SIZE(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST, W, XTERN) \
        do{\
		GtkWidget *input = mygtk_add_spin(hbox_param, -1, W);	\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
		    XTERN;\
	}while(0)
#define ADD_EC_SPIN_XTERN(LABEL, UNIT, ERROR_TEXT, VAR, LO, HI, FMT, STEP, PAGE, REMOTE_LIST, XTERN) \
        do{\
		GtkWidget *input = mygtk_create_spin_input(N_(LABEL), vbox_param, hbox_param);	\
                    g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(REMOTE_PREFIX LABEL));\
		    Gtk_EntryControl *ec = new Gtk_EntryControl (UNIT, N_(ERROR_TEXT), VAR, LO, HI, FMT, input, STEP, PAGE);\
		    ec->Set_ChangeNoticeFkt(DSPControl::ChangedNotify, this);\
		    EC_list = g_slist_prepend( EC_list, ec);\
		    REMOTE_LIST = ec->AddEntry2RemoteList(REMOTE_PREFIX LABEL, REMOTE_LIST);\
		    XTERN;\
	}while(0)

#define ADD_LABEL(LABEL)			\
	do{\
	label = gtk_label_new (N_(LABEL));\
	gtk_widget_set_size_request (label, 80, -1);\
	gtk_box_pack_start (GTK_BOX (hbox_param), label, FALSE, TRUE, 0);\
	}while(0)

#define ADD_LABEL_LIST(LABEL, LIST)			\
	do{\
	label = gtk_label_new (N_(LABEL));\
	gtk_widget_set_size_request (label, 80, -1);\
	gtk_box_pack_start (GTK_BOX (hbox_param), label, FALSE, TRUE, 0);\
	LIST = g_slist_prepend (LIST, label);				\
	}while(0)

#define ADD_TOGGLE(LABEL, CONTAINER, SOURCE, BIT_CODE, CB_FUNC)	\
        do {\
                   GtkWidget *checkbutton = gtk_check_button_new_with_label(N_(LABEL)); \
                   gtk_container_add(GTK_CONTAINER(CONTAINER), checkbutton);\
                   g_object_set_data(G_OBJECT(checkbutton), "Bit_Mask", GINT_TO_POINTER (BIT_CODE)); \
                   gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), (((SOURCE) & (BIT_CODE))?1:0)); \
                   g_signal_connect(checkbutton, "toggled",G_CALLBACK(CB_FUNC), this);\
        }while(0)

#define ADD_TOGGLE_LIST(LABEL, CONTAINER, SOURCE, BIT_CODE, CB_FUNC, LIST)	\
        do {\
                   GtkWidget *checkbutton = gtk_check_button_new_with_label(N_(LABEL)); \
                   gtk_container_add(GTK_CONTAINER(CONTAINER), checkbutton);\
                   g_object_set_data(G_OBJECT(checkbutton), "Bit_Mask", GINT_TO_POINTER (BIT_CODE)); \
                   gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), (((SOURCE) & (BIT_CODE))?1:0)); \
                   g_signal_connect(checkbutton, "toggled",G_CALLBACK(CB_FUNC), this);\
		   LIST = g_slist_prepend (LIST, checkbutton);		\
        }while(0)

#define ADD_TOGGLE_TO_TAB(LABEL, TAB, X, Y, SOURCE_CHANNEL_BIT_CODE, XTERN)	\
        do {\
                   GtkWidget *checkbutton = gtk_check_button_new_with_label(N_(LABEL)); \
                   gtk_table_attach (GTK_TABLE(TAB), checkbutton, X, (X)+1, Y, (Y)+1, GTK_FILL, GTK_FILL, 2,1); \
                   g_object_set_data (G_OBJECT(checkbutton), "Source_Channel", GINT_TO_POINTER (SOURCE_CHANNEL_BIT_CODE)); \
		   if (SOURCE_CHANNEL_BIT_CODE & X_SOURCE_MSK)\
		     gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), (((SOURCE_CHANNEL_BIT_CODE & 0xfffffff) & XSource)?1:0)); \
		   else if (SOURCE_CHANNEL_BIT_CODE & P_SOURCE_MSK)\
		     gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), (((SOURCE_CHANNEL_BIT_CODE & 0xfffffff) & PSource)?1:0)); \
		   else if (SOURCE_CHANNEL_BIT_CODE & A_SOURCE_MSK)\
		     gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), (((SOURCE_CHANNEL_BIT_CODE & 0xfffffff) & PlotAvg)?1:0)); \
		   else if (SOURCE_CHANNEL_BIT_CODE & S_SOURCE_MSK)\
		     gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), (((SOURCE_CHANNEL_BIT_CODE & 0xfffffff) & PlotSec)?1:0)); \
		   else\
		     gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), (((SOURCE_CHANNEL_BIT_CODE & 0xfffffff) & Source)?1:0)); \
                   g_signal_connect(checkbutton, "toggled",G_CALLBACK(change_source_callback), this);\
		   XTERN;\
        }while(0)


#define ADD_STATUS(LABEL, W, LIST)			\
	do {\
		hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);\
		gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param);\
		GtkWidget* label = gtk_label_new (N_(LABEL));\
		LIST = g_slist_prepend (LIST, label);\
		gtk_box_pack_start (GTK_BOX (hbox_param), label, FALSE, TRUE, 0);\
		gtk_widget_set_size_request (label, MYGTK_LSIZE, -1);\
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);\
		W = gtk_entry_new ();\
		gtk_entry_set_text (GTK_ENTRY(W), " --- ");\
		gtk_editable_set_editable (GTK_EDITABLE(W), FALSE);\
		LIST = g_slist_prepend (LIST, W);\
		gtk_container_add (GTK_CONTAINER (hbox_param), W);\
	}while(0)

#define BUILD_STANDART_PROBE_CONTROLS(HAVE_DUAL, OPTION_FLAGS, OPTION_CB, AUTO_FLAGS, AUTO_CB, EXEC_CB, WRITE_CB, GRAPH_CB, X_CB) \
	do {\
		hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);	\
		gtk_widget_show (hbox_param);				\
		gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param); \
		ADD_TOGGLE  ("Feedback On", hbox_param, OPTION_FLAGS, FLAG_FB_ON, OPTION_CB); \
		if(HAVE_DUAL) {						\
			ADD_TOGGLE  ("Dual Mode", hbox_param, OPTION_FLAGS, FLAG_DUAL, OPTION_CB); \
		}							\
		ADD_TOGGLE_LIST ("Oversampling", hbox_param, OPTION_FLAGS, FLAG_INTEGRATE, OPTION_CB, guru_list); \
		ADD_TOGGLE_LIST ("Full Ramp", hbox_param, OPTION_FLAGS, FLAG_SHOW_RAMP, OPTION_CB, guru_list); \
		ADD_TOGGLE  ("Auto Plot", hbox_param, AUTO_FLAGS, FLAG_AUTO_PLOT, AUTO_CB); \
		ADD_TOGGLE  ("Auto Save", hbox_param, AUTO_FLAGS, FLAG_AUTO_SAVE, AUTO_CB); \
									\
		frame_param = gtk_frame_new (N_("Controller"));		\
		gtk_widget_show(frame_param);				\
		gtk_container_add(GTK_CONTAINER (box), frame_param);	\
									\
		hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);			\
		gtk_container_add (GTK_CONTAINER (frame_param), hbox_param); \
									\
		button = gtk_button_new_with_label(N_("Execute"));	\
		gtk_widget_set_size_request (button, 100, -1);			\
		gtk_box_pack_start (GTK_BOX (hbox_param), button, TRUE, TRUE, 0); \
		gtk_widget_show (button);				\
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (EXEC_CB), this);	\
									\
		button = gtk_button_new_with_label(N_("Write Vectors")); \
		gtk_widget_set_size_request (button, 100, -1);			\
		gtk_box_pack_start (GTK_BOX (hbox_param), button, TRUE, TRUE, 0); \
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (WRITE_CB), this);	\
		guru_list = g_slist_prepend (guru_list, button);	\
									\
		button = gtk_button_new_with_label(N_("Plot"));		\
		gtk_widget_set_size_request (button, 100, -1);			\
		gtk_box_pack_start (GTK_BOX (hbox_param), button, TRUE, TRUE, 0); \
		gtk_widget_show (button);				\
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (GRAPH_CB), this); \
									\
		button = gtk_button_new_with_label(N_("Save")); \
		gtk_widget_set_size_request (button, 100, -1); \
		gtk_box_pack_start (GTK_BOX (hbox_param), button, TRUE, TRUE, 0); \
		gtk_widget_show (button); \
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (DSPControl::Probing_save_callback), this); \
									\
		if (0){ \
		button = gtk_button_new_with_label(N_("X")); \
		gtk_widget_set_size_request (button, 100, -1);			\
		gtk_box_pack_start (GTK_BOX (hbox_param), button, TRUE, TRUE, 0); \
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (X_CB), this);	\
		guru_list = g_slist_prepend (guru_list, button);	\
		} \
	}while(0)


#define OUT_OF_RANGE N_("Value out of range!")

// Achtung: Remote is not released :=(
// DSP-Param sollten lokal werden...
DSPControl::DSPControl ()
{
	int i,j;
	AmpIndex  AmpI;
	GSList *EC_list=NULL;

	RemoteEntryList = NULL;
	FreezeEntryList = NULL;

	GSList *expert_list=NULL;
	GSList *guru_list=NULL;

        GtkWidget *notebook;
        GtkWidget *scrolled_contents;

	GtkWidget *box;
	GtkWidget *frame_param, *vbox_param, *hbox_param;

	GtkWidget* wid, *label, *input;
	GtkWidget* menu;
	GtkWidget* menuitem;
	GtkWidget *checkbutton;
	GtkWidget *button;

	// init pc matrix to NULL
	for (i=0; i < 2*MAX_NUM_CHANNELS; ++i) 
		for (j=0; j < 2*MAX_NUM_CHANNELS; ++j) 
			probe_pc_matrix[i][j] = NULL;

	IV_status = NULL;
	LM_status = NULL;
	FZ_status = NULL;


	write_vector_mode=PV_MODE_NONE;

	probedata_list = NULL;
	num_probe_events = 0;
	last_probe_data_index = 0;
	nun_valid_data_sections = 0;

	probe_trigger_single_shot = 0;
	current_probe_data_index = 0;

	for (i=0; i<NUM_PROBEDATA_ARRAYS; ++i)
		garray_probedata [i] = NULL;

	read_dsp_scan_event_trigger ();

	XsmRescourceManager xrm("sranger_hwi_control");
        xrm.Get ("frq_ref", &frq_ref, "22100.0");
	xrm.Get ("bias", &bias, "0.0");
	xrm.Get ("current_set_point", &current_set_point, "0.1");
	xrm.Get ("voltage_set_point", &voltage_set_point, "0.1");
	xrm.Get ("move_speed_x", &move_speed_x, "500.0");
	xrm.Get ("scan_speed_x", &scan_speed_x_requested, "500.0");
	scan_speed_x = scan_speed_x_requested;
	xrm.Get ("gain_ratio", &gain_ratio, "1.0");
	xrm.Get ("usr_cp", &usr_cp, "0.0");
	xrm.Get ("usr_ci", &usr_ci, "0.1");
	xrm.Get ("dynamic_zoom", &dynamic_zoom, "1.0");
	xrm.Get ("pre_points", &pre_points, "0");
	xrm.Get ("expert_mode", &expert_mode, "0");
	xrm.Get ("guru_mode", &guru_mode, "0");
	xrm.Get ("center_return_flag", &center_return_flag, "1");
	xrm.Get ("area_slope_compensation_flag", &area_slope_compensation_flag, "0");
	xrm.Get ("area_slope_x", &area_slope_x, "0");
	xrm.Get ("area_slope_y", &area_slope_y, "0");

	ue_bias = bias;
	ue_scan_speed_x = 0.;
	ue_scan_speed_x_r = 0.;
	ue_usr_ci = 0.;
	ue_usr_cp = 0.;
	ue_current_set_point = current_set_point;
	ue_voltage_set_point = voltage_set_point;

	// LockIn and LockIn phase probe
	xrm.Get ("AC_amp", &AC_amp, "0.1");
	xrm.Get ("AC_frq", &AC_frq, "688");
	xrm.Get ("AC_phaseA", &AC_phaseA, "0");
	xrm.Get ("AC_phaseB", &AC_phaseB, "90");
	xrm.Get ("AC_lockin_avg_cycels", &AC_lockin_avg_cycels, "7");
	xrm.Get ("AC_phase_span", &AC_phase_span, "360");
	xrm.Get ("AC_points", &AC_points, "720");
	xrm.Get ("AC_repetitions", &AC_repetitions, "1");
	xrm.Get ("AC_phase_slope", &AC_phase_slope, "12");
	xrm.Get ("AC_final_delay", &AC_final_delay, "0.01");
	xrm.Get ("AC_option_flags", &AC_option_flags, "0");
	xrm.Get ("AC_auto_flags", &AC_auto_flags, "0");

	// Probing
	xrm.Get ("Probing_Sources", &Source, "0x3030");
	xrm.Get ("Probing_XSources", &XSource, "0x0");
	xrm.Get ("Probing_PSources", &PSource, "0x0");
	xrm.Get ("Probing_PlotAvg", &PlotAvg, "0x0");
	xrm.Get ("Probing_PlotSec", &PlotSec, "0x0");
	xrm.Get ("Probing_probe_trigger_raster_points", &probe_trigger_raster_points_user, "0");
	probe_trigger_raster_points = 0;

	// STS I-V
	xrm.Get ("Probing_IV_Ustart", &IV_start, "-1.0");
	xrm.Get ("Probing_IV_Uend", &IV_end, "1.0");
	xrm.Get ("Probing_IV_repetitions", &IV_repetitions, "1");
	xrm.Get ("Probing_IV_dz", &IV_dz, "0.0");
	xrm.Get ("Probing_IVdz_repetitions", &IVdz_repetitions, "0");
	xrm.Get ("Probing_IV_points", &IV_points, "100");
	xrm.Get ("Probing_IV_slope", &IV_slope, "10");
	xrm.Get ("Probing_IV_slope_ramp", &IV_slope_ramp, "50");
	xrm.Get ("Probing_IV_final_delay", &IV_final_delay, "0.01");
	xrm.Get ("Probing_IV_recover_delay", &IV_recover_delay, "0.3");
	xrm.Get ("Probing_IV_dx", &IV_dx, "50.0");
	xrm.Get ("Probing_IV_dy", &IV_dy, "0.0");
	xrm.Get ("Probing_IV_dxy_slope", &IV_dxy_slope, "100.0");
	xrm.Get ("Probing_IV_dxy_delay", &IV_dxy_delay, "1.0");
	xrm.Get ("Probing_IV_dxy_points", &IV_dxy_points, "1");
	xrm.Get ("Probing_IV_option_flags", &IV_option_flags, "0");
	xrm.Get ("Probing_IV_auto_flags", &IV_auto_flags, "0");
	// FZ
	xrm.Get ("Probing_FZ_Z_start", &FZ_start,"0.0");
	xrm.Get ("Probing_FZ_Z_end", &FZ_end,"100.0");
	xrm.Get ("Probing_FZ_points", &FZ_points,"100");
	xrm.Get ("Probing_FZ_slope", &FZ_slope,"100");
	xrm.Get ("Probing_FZ_slope_ramp", &FZ_slope_ramp, "100");
	xrm.Get ("Probing_FZ_final_delay", &FZ_final_delay,"0.01");
	xrm.Get ("Probing_FZ_option_flags", &FZ_option_flags, "0");
	xrm.Get ("Probing_FZ_auto_flags", &FZ_auto_flags, "0");
	// PL
	xrm.Get ("Probing_PL_duration", &PL_duration,"10");
	xrm.Get ("Probing_PL_volt", &PL_volt,"2");
	xrm.Get ("Probing_PL_slope", &PL_slope,"1e4");
	xrm.Get ("Probing_PL_repetitions", &PL_repetitions,"1");
	xrm.Get ("Probing_PL_final_delay", &PL_final_delay,"0.01");
	xrm.Get ("Probing_PL_option_flags", &PL_option_flags, "0");
	xrm.Get ("Probing_PL_auto_flags", &PL_auto_flags, "0");
	// LP
	xrm.Get ("Probing_LP_duration", &LP_duration,"10");
	xrm.Get ("Probing_LP_triggertime", &LP_triggertime,"10");
	xrm.Get ("Probing_LP_volt", &LP_volt,"2");
	xrm.Get ("Probing_LP_slope", &LP_slope,"1e4");
	xrm.Get ("Probing_LP_repetitions", &LP_repetitions,"1");
	xrm.Get ("Probing_LP_FZ_end", &LP_FZ_end,"0.0");
	xrm.Get ("Probing_LP_final_delay", &LP_final_delay,"10");
	xrm.Get ("Probing_LP_option_flags", &LP_option_flags, "0");
	xrm.Get ("Probing_LP_auto_flags", &LP_auto_flags, "0");
	// SP
	xrm.Get ("Probing_SP_duration", &SP_duration,"10");
	xrm.Get ("Probing_SP_volt", &SP_volt,"2");
	xrm.Get ("Probing_SP_ramptime", &SP_ramptime,"10");
	xrm.Get ("Probing_SP_flag_volt", &SP_flag_volt,"1");
	xrm.Get ("Probing_SP_repetitions", &SP_repetitions,"1");
	xrm.Get ("Probing_SP_final_delay", &SP_final_delay,"0.01");
	xrm.Get ("Probing_SP_option_flags", &SP_option_flags, "0");
	xrm.Get ("Probing_SP_auto_flags", &SP_auto_flags, "0");
	// TS
	xrm.Get ("Probing_TS_duration", &TS_duration,"1000");
	xrm.Get ("Probing_TS_points", &TS_points,"2048");
	xrm.Get ("Probing_TS_repetitions", &TS_repetitions,"1");
	xrm.Get ("Probing_TS_option_flags", &TS_option_flags, "0");
	xrm.Get ("Probing_TS_auto_flags", &TS_auto_flags, "0");
	// LM
	xrm.Get ("Probing_LM_dx", &LM_dx,"100.0");
	xrm.Get ("Probing_LM_dy", &LM_dy,"0.0");
	xrm.Get ("Probing_LM_dz", &LM_dz,"0.0");
	xrm.Get ("Probing_LM_points", &LM_points,"100");
	xrm.Get ("Probing_LM_slope", &LM_slope,"1000");
	xrm.Get ("Probing_LM_final_delay", &LM_final_delay,"0.01");
	xrm.Get ("Probing_LM_option_flags", &LM_option_flags, "0");
	xrm.Get ("Probing_LM_auto_flags", &LM_auto_flags, "0");
	// AX
	xrm.Get ("Probing_AX_start", &AX_start,"0.0");
	xrm.Get ("Probing_AX_end", &AX_end,"100.0");
	xrm.Get ("Probing_AX_points", &AX_points,"100");
	xrm.Get ("Probing_AX_repetitions", &AX_repetitions,"1");
	xrm.Get ("Probing_AX_slope", &AX_slope,"100");
	xrm.Get ("Probing_AX_slope_ramp", &AX_slope_ramp, "100");
	xrm.Get ("Probing_AX_final_delay", &AX_final_delay,"0.01");
	xrm.Get ("Probing_AX_gatetime", &AX_gatetime,"0.001");
	xrm.Get ("Probing_AX_gain", &AX_gain,"1");
	xrm.Get ("Probing_AX_resolution", &AX_resolution,"1");
	xrm.Get ("Probing_AX_option_flags", &AX_option_flags, "0");
	xrm.Get ("Probing_AX_auto_flags", &AX_auto_flags, "0");

	Unity    = new UnitObj(" "," ");
	Volt     = new UnitObj("V","V");
	Angstroem= new UnitObj(UTF8_ANGSTROEM,"A");
	Frq      = new UnitObj("Hz","Hz");
	Time     = new UnitObj("s","s");
	TimeUms  = new LinUnit("ms","ms",1e-3);
	msTime   = new UnitObj("ms","ms");
	minTime  = new UnitObj("min","min");
	Deg      = new UnitObj(UTF8_DEGREE,"Deg");
	Current  = new UnitObj("nA","nA");
	Speed    = new UnitObj(UTF8_ANGSTROEM"/s","A/s");
	PhiSpeed = new UnitObj(UTF8_DEGREE"/s","Deg/s");
	Vslope   = new UnitObj("V/s","V/s");
	SetPtUnit = sranger_hwi_pi.app->xsm->MakeUnit (xsmres.daqZunit[0], xsmres.daqZlabel[0]);

	probe_fname = g_strdup ("probe_test.data");
	probe_findex = 0;

	gchar *tmp = g_strdup_printf ("%s [%s]", N_("SR DSP Control"), xsmres.DSPDev);
	AppWindowInit(tmp);
	set_window_geometry ("dsp-control-0");


        notebook = gtk_notebook_new ();
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
	gtk_grid_attach (GTK_GRID (v_grid), notebook, 1,1, 1,1);
	gtk_widget_show (notebook);

        gtk_widget_set_size_request  (notebook, 550, 350);
        
// ==== Folder: Feedback & Scan ========================================
	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);
		
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
				  scrolled_contents,
				  gtk_label_new ("Feedback & Scan"));



	gtk_widget_show (box);
//	gtk_box_pack_start (GTK_BOX (vbox), box, TRUE, TRUE, 0);

	// ========================================
	frame_param = gtk_frame_new (N_("SR-FB Characteristics"));
	gtk_widget_show (frame_param);
	gtk_container_add (GTK_CONTAINER (box), frame_param);

	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	// ------- FB Characteristics

	ADD_EC_WITH_SCALE_FLIST_L ("Bias", Volt, OUT_OF_RANGE, &bias, -10., 10., "5g", 0.001, 0.01,  RemoteEntryList, FreezeEntryList);
	if (IS_AFM_CTRL) // AFM
		ADD_EC_WITH_SCALE_FLIST ("SetPoint", SetPtUnit, OUT_OF_RANGE,  &voltage_set_point, -500., 500., "5g", 0.001, 0.01,  RemoteEntryList, FreezeEntryList);
	else // STM
		ADD_EC_WITH_SCALE_FLIST_L ("Current", Current, OUT_OF_RANGE, &current_set_point, 0.0001, 50., "5g", 0.001, 0.01,  RemoteEntryList, FreezeEntryList);

	ADD_EC_WITH_SCALE ("CP", Unity, OUT_OF_RANGE, &usr_cp, -1., 1., "5g", 0.001, 0.01,  RemoteEntryList);
	ADD_EC_WITH_SCALE ("CI", Unity, OUT_OF_RANGE, &usr_ci, -1., 1., "5g", 0.001, 0.01,  RemoteEntryList);

	// ========================================
	frame_param = gtk_frame_new (N_("Scan Characteristics"));
	gtk_widget_show (frame_param);
	gtk_container_add (GTK_CONTAINER (box), frame_param);

	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

//	ADD_EC_WITH_SCALE ("DynZoom", Unity, OUT_OF_RANGE, &dynamic_zoom, 0., 5., "5g", 0.01, 0.1,  RemoteEntryList);
	ADD_EC_WITH_SCALE ("MoveSpd", Speed, OUT_OF_RANGE, &move_speed_x, 0.1, 2000., "5g", 1., 10.,  RemoteEntryList);
	ADD_EC_WITH_SCALE_XTERN ("ScanSpd", Speed, OUT_OF_RANGE, &scan_speed_x_requested, 0.1, 2000., "5g", 1., 10.,  RemoteEntryList, scan_speed_ec = ec);
//	ADD_EC_SPIN_XTERN ("ScanSpdReal", Speed, OUT_OF_RANGE, &scan_speed_x, 0., 1e9, "5g", 1., 10.,  RemoteEntryList, ec->Freeze ());

	// ======================================== Piezo Drive / Amplifier Settings
	frame_param = gtk_frame_new (N_("Piezo Drive Settings"));
	gtk_widget_show (frame_param);
	gtk_container_add (GTK_CONTAINER (box), frame_param);

	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	/* Amplifier Settings */
	hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show (hbox_param);
	gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param);

	for(j=0; j<6; j++){
		if (j == 3 && gapp->xsm->Inst->OffsetMode () == OFM_DSP_OFFSET_ADDING)
			break;
		if (j == 3){
			hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			gtk_widget_show (hbox_param);
			gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param);
		}

		switch(j){
		case 0: label = gtk_label_new ("VX"); break;
		case 1: label = gtk_label_new ("VY"); break;
		case 2: label = gtk_label_new ("VZ"); break;
		case 3: label = gtk_label_new ("VX0"); break;
		case 4: label = gtk_label_new ("VY0"); break;
		case 5: label = gtk_label_new ("VZ0"); break;
		}
		gtk_widget_show (label);
		gtk_container_add (GTK_CONTAINER (hbox_param), label);
		//    gtk_box_pack_start (GTK_BOX (hbox_param), label, FALSE, TRUE, 0);
		gtk_widget_set_size_request (label, 20, -1);

		wid = gtk_combo_box_text_new ();
		gtk_widget_set_size_request (wid, 50, -1);
		gtk_widget_show (wid);
		gtk_container_add (GTK_CONTAINER (hbox_param), wid);
		g_object_set_data  (G_OBJECT (wid), "chindex", GINT_TO_POINTER (j));

		// Init gain-choicelist
		for(i=0; i<GAIN_POSITIONS; i++){
			AmpI.l = 0L;
			AmpI.s.ch = j;
			AmpI.s.x  = i;
			gchar *Vxyz = g_strdup_printf("%g",xsmres.V[i]);
			gchar *id = g_strdup_printf ("%ld",AmpI.l);
			gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (wid), id, Vxyz);
			g_free (id);
			g_free (Vxyz);
		}
		switch(j){
		case 0: gtk_combo_box_set_active (GTK_COMBO_BOX (wid), xsmres.VXdefault); break;
		case 1: gtk_combo_box_set_active (GTK_COMBO_BOX (wid), xsmres.VYdefault); break;
		case 2: gtk_combo_box_set_active (GTK_COMBO_BOX (wid), xsmres.VZdefault); break;
		case 3: gtk_combo_box_set_active (GTK_COMBO_BOX (wid), xsmres.VX0default); break;
		case 4: gtk_combo_box_set_active (GTK_COMBO_BOX (wid), xsmres.VY0default); break;
		case 5: gtk_combo_box_set_active (GTK_COMBO_BOX (wid), xsmres.VZ0default); break;
		}
		g_signal_connect (G_OBJECT (wid), "changed",
				  G_CALLBACK (DSPControl::choice_Ampl_callback),
				  this);
	}

        gtk_widget_show_all (scrolled_contents);



// ==== Folder: Scan Events Trigger ========================================
	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);
		
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
				  scrolled_contents,
				  gtk_label_new ("Trigger"));
	gtk_widget_show (box);

	// ========================================
	frame_param = gtk_frame_new (N_("SR DSP Auto Scan Events Trigger"));
	gtk_widget_show (frame_param);
	gtk_container_add (GTK_CONTAINER (box), frame_param);

	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show (hbox_param);
	gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param);
	label = gtk_label_new (N_("Trigger"));
	gtk_widget_set_size_request (label, 100, -1);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox_param), label, FALSE, TRUE, 0);

	checkbutton = gtk_check_button_new_with_label(N_("Enable Auto Trigger"));
	gtk_widget_set_size_request (checkbutton, 100, -1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), dsp_scan_event_trigger.pflg ? 1:0);
	gtk_box_pack_start (GTK_BOX (hbox_param), checkbutton, TRUE, TRUE, 0);
	gtk_widget_show (checkbutton);
	g_signal_connect (G_OBJECT (checkbutton), "clicked",
			    G_CALLBACK (DSPControl::se_auto_trigger_callback), this);

	frame_param = gtk_frame_new (N_("Bias Trigger (Voltage Dependent Scan)"));
	gtk_widget_show (frame_param);
	gtk_container_add (GTK_CONTAINER (box), frame_param);

	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	i=0;
	ADD_EC_SPIN ("Trig-Xpm-A", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xp[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("Bias-Xp-A", Volt, OUT_OF_RANGE, &trigger_bias_setpoint_xp[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	APP_EC_SPIN ("Trig-Xm-A", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xm[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("Bias-Xm-A", Volt, OUT_OF_RANGE, &trigger_bias_setpoint_xm[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	++i;
	ADD_EC_SPIN ("Trig-Xpm-B", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xp[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("Bias-Xp-B", Volt, OUT_OF_RANGE, &trigger_bias_setpoint_xp[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	APP_EC_SPIN ("Trig-Xm-B", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xm[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("Bias-Xm-B", Volt, OUT_OF_RANGE, &trigger_bias_setpoint_xm[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	++i;
	ADD_EC_SPIN ("Trig-Xpm-C", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xp[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("Bias-Xp-C", Volt, OUT_OF_RANGE, &trigger_bias_setpoint_xp[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	APP_EC_SPIN ("Trig-Xm-C", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xm[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("Bias-Xm-C", Volt, OUT_OF_RANGE, &trigger_bias_setpoint_xm[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	++i;
	ADD_EC_SPIN ("Trig-Xpm-D", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xp[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("Bias-Xp-D", Volt, OUT_OF_RANGE, &trigger_bias_setpoint_xp[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	APP_EC_SPIN ("Trig-Xm-D", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xm[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("Bias-Xm-D", Volt, OUT_OF_RANGE, &trigger_bias_setpoint_xm[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	++i;

	frame_param = gtk_frame_new (N_("Setpoint/Current Trigger (Current Dependent Scan)"));
	gtk_widget_show (frame_param);
	gtk_container_add (GTK_CONTAINER (box), frame_param);

	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	ADD_EC_SPIN ("Trig-Xpm-E", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xp[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("SetPt-Xp-E", Current, OUT_OF_RANGE, &trigger_bias_setpoint_xp[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	APP_EC_SPIN ("Trig-Xm-E", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xm[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("SetPt-Xm-E", Current, OUT_OF_RANGE, &trigger_bias_setpoint_xm[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	++i;
	ADD_EC_SPIN ("Trig-Xpm-F", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xp[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("SetPt-Xp-F", Current, OUT_OF_RANGE, &trigger_bias_setpoint_xp[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	APP_EC_SPIN ("Trig-Xm-F", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xm[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("SetPt-Xm-F", Current, OUT_OF_RANGE, &trigger_bias_setpoint_xm[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	++i;
	ADD_EC_SPIN ("Trig-Xpm-G", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xp[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("SetPt-Xp-G", Current, OUT_OF_RANGE, &trigger_bias_setpoint_xp[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	APP_EC_SPIN ("Trig-Xm-G", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xm[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("SetPt-Xm-G", Current, OUT_OF_RANGE, &trigger_bias_setpoint_xm[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	++i;
	ADD_EC_SPIN ("Trig-Xpm-H", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xp[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("SetPt-Xp-H", Current, OUT_OF_RANGE, &trigger_bias_setpoint_xp[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);
	APP_EC_SPIN ("Trig-Xm-H", Unity, OUT_OF_RANGE, &dsp_scan_event_trigger.trig_i_xm[i], -1., 65534., ".0f", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("SetPt-Xm-H", Current, OUT_OF_RANGE, &trigger_bias_setpoint_xm[i], -10., 10., "5g", 0.01, 0.1,  RemoteEntryList);

        gtk_widget_show_all (scrolled_contents);


// ==== Folder: Advanced or DSP Expert (advanced Feedback & Scan settings) ========================================
	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);
		
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
				  scrolled_contents,
				  gtk_label_new ("Advanced"));
	gtk_widget_show (box);

	// ========================================
	frame_param = gtk_frame_new (N_("SR-FB Characteristics"));
	gtk_widget_show (frame_param);
	gtk_container_add (GTK_CONTAINER (box), frame_param);

	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	// ------- FB Characteristics

	hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show (hbox_param);
	gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param);
	label = gtk_label_new (N_("FB Switch"));
	gtk_widget_set_size_request (label, 100, -1);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox_param), label, FALSE, TRUE, 0);

	checkbutton = gtk_check_button_new_with_label(N_("Enable Feed Back Controller"));
	gtk_widget_set_size_request (checkbutton, 100, -1);
	gtk_box_pack_start (GTK_BOX (hbox_param), checkbutton, TRUE, TRUE, 0);
	// read currect FB (MD_PID) state
	read_dsp_state ();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), dsp_state.mode & MD_PID ? 1:0);
	gtk_widget_show (checkbutton);
	g_signal_connect (G_OBJECT (checkbutton), "clicked",
			    G_CALLBACK (DSPControl::feedback_callback), this);

	// -------- Automatic Raster Probe Mode Select
	frame_param = gtk_frame_new (N_("Automatic Raster Probe Trigger Control & Probe Details"));
	gtk_widget_show (frame_param);
	gtk_container_add (GTK_CONTAINER (box), frame_param);

	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);
	hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show (hbox_param);
	gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param);

	//yhyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy
	GtkWidget *auto_probe_menu = gtk_combo_box_text_new ();
	gtk_widget_set_size_request (auto_probe_menu, 60, -1);
	gtk_widget_show (auto_probe_menu);
	gtk_container_add (GTK_CONTAINER (hbox_param), auto_probe_menu);

	g_signal_connect (G_OBJECT (auto_probe_menu), "changed",
			  G_CALLBACK (DSPControl::auto_probe_callback),
			  this);

	gchar *id = g_strdup_printf ("%d", PV_MODE_NONE);
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (auto_probe_menu), id, N_("No Auto Probe"));
	g_free (id);

	Gtk_EntryControl* raster_ec;
	ADD_EC_SPIN_XTERN ("Raster", Unity, OUT_OF_RANGE, &probe_trigger_raster_points_user, 0, 200, "5g", 1., 10.,  RemoteEntryList, raster_ec=ec);
	raster_ec->Freeze ();
	g_object_set_data(G_OBJECT (auto_probe_menu), "raster_ec", (gpointer)raster_ec);
	//	g_object_set_data(G_OBJECT (menuitem), "auto_probe_mode", GINT_TO_POINTER (PV_MODE_NONE));

	hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show (hbox_param);
	gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param);
	GtkWidget *expert_checkbutton = gtk_check_button_new_with_label(N_("Show Expert controls"));
	gtk_container_add(GTK_CONTAINER(hbox_param), expert_checkbutton);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(expert_checkbutton), expert_mode);
	g_signal_connect(expert_checkbutton, "toggled",G_CALLBACK(DSPControl::DSP_expert_callback), this);

	GtkWidget *guru_checkbutton = gtk_check_button_new_with_label(N_("Guru mode"));
	gtk_container_add(GTK_CONTAINER(hbox_param), guru_checkbutton);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(guru_checkbutton), guru_mode);
	g_signal_connect(guru_checkbutton, "toggled", G_CALLBACK(DSPControl::DSP_guru_callback), this);

	// ========================================
	frame_param = gtk_frame_new (N_("Scan Characteristics - Expert"));
	gtk_widget_show (frame_param);
	gtk_container_add (GTK_CONTAINER (box), frame_param);

	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	ADD_EC_SPIN_LIST ("DynZoom", Unity, OUT_OF_RANGE, &dynamic_zoom, 0., 5., "5g", 0.01, 0.1,  RemoteEntryList, guru_list);
	ADD_EC_WITH_SCALE ("PrePts", Unity, OUT_OF_RANGE, &pre_points, 0., 100., "5g", 1., 10.,  RemoteEntryList);

	ADD_EC_WITH_SCALE ("SlopeX", Unity, OUT_OF_RANGE, &area_slope_x, -1., 1., "5g", 0.01, 0.1,  RemoteEntryList);
	ADD_EC_WITH_SCALE ("SlopeY", Unity, OUT_OF_RANGE, &area_slope_y, -1., 1., "5g", 0.01, 0.1,  RemoteEntryList);

	hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show (hbox_param);
	gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param);
	GtkWidget *slope_checkbutton = gtk_check_button_new_with_label(N_("Enable Slope Compensation"));
	gtk_container_add(GTK_CONTAINER(hbox_param), slope_checkbutton);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(slope_checkbutton), area_slope_compensation_flag);
	g_signal_connect(slope_checkbutton, "toggled",G_CALLBACK(DSPControl::DSP_slope_callback), this);

	hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show (hbox_param);
	gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param);
	GtkWidget *cret_checkbutton = gtk_check_button_new_with_label(N_("Enable automatic Tip return to center"));
	gtk_container_add(GTK_CONTAINER(hbox_param), cret_checkbutton);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(cret_checkbutton), center_return_flag);
	g_signal_connect(cret_checkbutton, "toggled",G_CALLBACK(DSPControl::DSP_cret_callback), this);

        gtk_widget_show_all (scrolled_contents);


// ==== Folder Set for Vector Probe ========================================
// ==== Folder: I-V STS setup ========================================
 	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
 	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
 					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);

 	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
 				  scrolled_contents,
 				  gtk_label_new ("STS"));
 	gtk_widget_show (box);

	// add item to auto-probe-menu
	id = g_strdup_printf ("%d", PV_MODE_IV);
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (auto_probe_menu), id, N_("STS-Raster"));
	g_free (id);

	//	g_object_set_data(G_OBJECT (menuitem), "auto_probe_mode", GINT_TO_POINTER (PV_MODE_IV));

	
	// ========================================
 	frame_param = gtk_frame_new (N_("I-V Type Spectroscopy"));

 	gtk_widget_show (frame_param);
 	gtk_container_add (GTK_CONTAINER (box), frame_param);
 	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_widget_show (vbox_param);
 	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	// stuff here ...
	ADD_EC_SPIN ("IV-Start-End", Volt, OUT_OF_RANGE, &IV_start, -10.0, 10., "5.3g", 0.1, 0.025,  RemoteEntryList);
	APP_EC_SPIN ("IV-End", Volt, OUT_OF_RANGE, &IV_end, -10.0, 10.0, "5.3g", 0.1, 0.025,  RemoteEntryList);
	ADD_LABEL_LIST ("#IV", expert_list);
	APP_EC_SPIN_LIST ("IV-rep", Unity, OUT_OF_RANGE, &IV_repetitions, 1., 100., "3g", 1., 5.,  RemoteEntryList, expert_list);
	ADD_EC_SPIN ("IV-dz", Angstroem, OUT_OF_RANGE, &IV_dz, -1000.0, 1000.0, "5.4g", 1., 2.,  RemoteEntryList);
	ADD_LABEL_LIST ("#DZ", expert_list);
	APP_EC_SPIN_LIST ("IV-dz-rep", Unity, OUT_OF_RANGE, &IVdz_repetitions, 0., 100., "3g", 1., 10.,  RemoteEntryList, expert_list);
	ADD_EC_SPIN ("Points", Unity, OUT_OF_RANGE, &IV_points, 1., 1000., "5g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("IV-Slope", Vslope, OUT_OF_RANGE, &IV_slope,0.1,1000.0, "5.3g", 1., 10.,  RemoteEntryList);
	ADD_LABEL ("Slope-Ramp");
	APP_EC_SPIN ("IV-Slope-Ramp", Vslope, OUT_OF_RANGE, &IV_slope_ramp,0.1,1000.0, "5.3g", 1., 10.,  RemoteEntryList);

	ADD_EC_SPIN_LIST ("Line-dXY", Angstroem, OUT_OF_RANGE, &IV_dx, -1e6, 1e6, "5.3g", 1., 10.,  RemoteEntryList, expert_list);
	APP_EC_SPIN_LIST ("Line-dY",  Angstroem, OUT_OF_RANGE, &IV_dy, -1e6, 1e6, "5.3g", 1., 10.,  RemoteEntryList, expert_list);
	ADD_LABEL_LIST ("#Pts", expert_list);
	APP_EC_SPIN_LIST ("IV-Line-Pts", Unity, OUT_OF_RANGE, &IV_dxy_points, 1., 1000., "3g", 1., 10.,  RemoteEntryList, expert_list);
	ADD_EC_SPIN_LIST ("IV-Line-slope", Speed, OUT_OF_RANGE, &IV_dxy_slope, -1e6, 1e6, "5.3g", 1., 10.,  RemoteEntryList, expert_list);
	ADD_LABEL_LIST ("Final-Delay", expert_list);
	APP_EC_SPIN_LIST ("IV-Line-Final-Delay", Time, OUT_OF_RANGE, &IV_dxy_delay, 0., 10., "3g", 0.01, 0.1,  RemoteEntryList, expert_list);

	ADD_EC_SPIN ("Final-Delay", Time, OUT_OF_RANGE, &IV_final_delay, 0., 1., "5.3g", 0.001, 0.01,  RemoteEntryList);
	ADD_LABEL_LIST ("Recover-Delay", expert_list);
	APP_EC_SPIN_LIST ("Recover-Delay", Time, OUT_OF_RANGE, &IV_recover_delay, 0., 1., "5.3g", 0.001, 0.01,  RemoteEntryList, expert_list);
	ADD_STATUS ("Status", IV_status, expert_list);

	BUILD_STANDART_PROBE_CONTROLS (TRUE, IV_option_flags, callback_change_IV_option_flags, IV_auto_flags,  callback_change_IV_auto_flags,
				       DSPControl::Probing_exec_IV_callback, DSPControl::Probing_write_IV_callback, DSPControl::Probing_graph_callback,
				       DSPControl::Probing_abort_callback);

	gtk_widget_show_all (scrolled_contents);

// ==== Folder: FZ (Force-Distance/AFM) setup ========================================
 	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
 	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
 					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);

 	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
 				  scrolled_contents,
 				  gtk_label_new ("Z"));
 	gtk_widget_show (box);
	
	// add item to auto-probe-menu
	id = g_strdup_printf ("%d", PV_MODE_FZ);
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (auto_probe_menu), id, N_("Z-Raster"));
	g_free (id);

	// ========================================
 	frame_param = gtk_frame_new (N_("Z Manipulation (F-Z, Tip tune, ...)"));
 	gtk_widget_show (frame_param);
 	gtk_container_add (GTK_CONTAINER (box), frame_param);
 	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_widget_show (vbox_param);
 	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	// stuff here ...
	ADD_EC_SPIN ("Z-Start-End", Angstroem, OUT_OF_RANGE, &FZ_start, -1000.0, 1000., "5.3g", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("Z-End", Angstroem, OUT_OF_RANGE, &FZ_end, -1000.0, 1000.0, "5.3g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Points", Unity, OUT_OF_RANGE, &FZ_points, 1, 4000, "5g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Z-Slope", Speed, OUT_OF_RANGE, &FZ_slope,0.1,10000.0, "5.3g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Slope-Ramp", Speed, OUT_OF_RANGE, &FZ_slope_ramp,0.1,10000.0, "5.3g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Final-Delay", Time, OUT_OF_RANGE, &FZ_final_delay, 0., 1., "5.3g", 0.001, 0.01,  RemoteEntryList);
	ADD_STATUS ("Status", FZ_status, expert_list);

	BUILD_STANDART_PROBE_CONTROLS (TRUE, FZ_option_flags, callback_change_FZ_option_flags, FZ_auto_flags, callback_change_FZ_auto_flags,
				       DSPControl::Probing_exec_FZ_callback, DSPControl::Probing_write_FZ_callback, DSPControl::Probing_graph_callback,
				       DSPControl::Probing_abort_callback);

	gtk_widget_show_all (scrolled_contents);

// ==== Folder: PL (Puls) setup ========================================
 	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
 	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
 					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);

 	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
 				  scrolled_contents,
 				  gtk_label_new ("PL"));
 	gtk_widget_show (box);
	
	// add item to auto-probe-menu
	id = g_strdup_printf ("%d", PV_MODE_PL);
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (auto_probe_menu), id, N_("PL-Raster"));
	g_free (id);

	// ========================================
 	frame_param = gtk_frame_new (N_("Bias Puls Control (Tip tune, ...)"));
 	gtk_widget_show (frame_param);
 	gtk_container_add (GTK_CONTAINER (box), frame_param);
 	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_widget_show (vbox_param);
 	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	// stuff here ...
	ADD_EC_SPIN ("Duration", msTime, OUT_OF_RANGE, &PL_duration, 0., 1000., ".2f", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Volts", Volt, OUT_OF_RANGE, &PL_volt, -10, 10, ".3f", .1, 1.,  RemoteEntryList);
	ADD_EC_SPIN ("Slope", Vslope, OUT_OF_RANGE, &PL_slope, 0.1,1e5, "5.3g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Final-Delay", Time, OUT_OF_RANGE, &PL_final_delay, 0., 1., "5.3g", 0.001, 0.01,  RemoteEntryList);
	ADD_EC_SPIN ("Repetitions", Unity, OUT_OF_RANGE, &PL_repetitions, 1., 100., ".0f", 1, 10,  RemoteEntryList);
	ADD_STATUS ("Status", PL_status, expert_list);

	BUILD_STANDART_PROBE_CONTROLS (FALSE, PL_option_flags, callback_change_PL_option_flags, PL_auto_flags,  callback_change_PL_auto_flags,
				       DSPControl::Probing_exec_PL_callback, DSPControl::Probing_write_PL_callback, DSPControl::Probing_graph_callback,
				       DSPControl::Probing_abort_callback);

	gtk_widget_show_all (scrolled_contents);

// ==== Folder: LP (Laserpuls) setup ========================================
	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);
 
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
				  scrolled_contents,
				  gtk_label_new ("LPC"));
	gtk_widget_show (box);
       
	// add item to auto-probe-menu
	id = g_strdup_printf ("%d", PV_MODE_LP);
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (auto_probe_menu), id, N_("LP-Raster"));
	g_free (id);
 
	// ========================================
 	frame_param = gtk_frame_new (N_("Laser Pulse Control (Trigger on X0, Offset adding: OFF)"));
	gtk_widget_show (frame_param);
	gtk_container_add (GTK_CONTAINER (box), frame_param);
	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);
 
	// stuff here ...
	ADD_EC_SPIN ("FB-Time", msTime, OUT_OF_RANGE, &LP_duration, 0., 1000., ".2f", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Trigger-Volts", Volt, OUT_OF_RANGE, &LP_volt, -10, 10, ".3f", .1, 1.,  RemoteEntryList);
	ADD_EC_SPIN ("Trigger-Time", msTime, OUT_OF_RANGE, &LP_triggertime, 0., 1000., ".2f", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Tip-Retract", Angstroem, OUT_OF_RANGE, &LP_FZ_end, -100.0, 100.0, "5.3g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Laser-Delay", msTime, OUT_OF_RANGE, &LP_final_delay, 0., 1000., ".2f", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Slope", Vslope, OUT_OF_RANGE, &LP_slope, 0.1,1e5, "5.3g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Repetitions", Unity, OUT_OF_RANGE, &LP_repetitions, 1., 100., ".0f", 1, 10,  RemoteEntryList);
	ADD_STATUS ("Status", LP_status, expert_list);
 
	BUILD_STANDART_PROBE_CONTROLS (FALSE, LP_option_flags, callback_change_LP_option_flags, LP_auto_flags,  callback_change_LP_auto_flags,
				       DSPControl::Probing_exec_LP_callback, DSPControl::Probing_write_LP_callback, DSPControl::Probing_graph_callback,
				       DSPControl::Probing_abort_callback);
 
	gtk_widget_show_all (scrolled_contents);

// ==== Folder: SP (Special/Slow Puls) setup ========================================
 	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
 	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
 					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);

 	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
 				  scrolled_contents,
 				  gtk_label_new ("SP"));
 	gtk_widget_show (box);
	
	// ========================================
 	frame_param = gtk_frame_new (N_("Slow Pulse Control (Sputter Anneal, ...)"));
 	gtk_widget_show (frame_param);
 	gtk_container_add (GTK_CONTAINER (box), frame_param);
 	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_widget_show (vbox_param);
 	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	// stuff here ...
	ADD_EC_SPIN ("Duration", minTime, OUT_OF_RANGE, &SP_duration, 0., 1000., ".2f", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Volts", Volt, OUT_OF_RANGE, &SP_volt, -10, 10, ".3f", .1, 1.,  RemoteEntryList);
	ADD_EC_SPIN ("Ramp-Time", minTime, OUT_OF_RANGE, &SP_ramptime, 0.1,1e5, ".2f", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Flag-VonX", Volt, OUT_OF_RANGE, &SP_flag_volt, -2.,2., ".3f", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Delay", minTime, OUT_OF_RANGE, &SP_final_delay, 0., 1., ".2f", 0.001, 0.01,  RemoteEntryList);
	ADD_EC_SPIN ("Repetitions", Unity, OUT_OF_RANGE, &SP_repetitions, 1., 100., ".0f", 1, 10,  RemoteEntryList);
	ADD_STATUS ("Status", SP_status, expert_list);


	BUILD_STANDART_PROBE_CONTROLS (FALSE, SP_option_flags, callback_change_SP_option_flags, SP_auto_flags,  callback_change_SP_auto_flags,
				       DSPControl::Probing_exec_SP_callback, DSPControl::Probing_write_SP_callback, DSPControl::Probing_graph_callback,
				       DSPControl::Probing_abort_callback);

	gtk_widget_show_all (scrolled_contents);


// ==== Folder: TS (Time Spectroscopy) setup ========================================
 	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
 	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
 					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);

 	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
 				  scrolled_contents,
 				  gtk_label_new ("TS"));
 	gtk_widget_show (box);
	
	// add item to auto-probe-menu
	id = g_strdup_printf ("%d", PV_MODE_TS);
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (auto_probe_menu), id, N_("TS-Raster"));
	g_free (id);

	// ========================================
 	frame_param = gtk_frame_new (N_("Time Spectroscopy (Signal/Noise Analysis, ...)"));
 	gtk_widget_show (frame_param);
 	gtk_container_add (GTK_CONTAINER (box), frame_param);
 	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_widget_show (vbox_param);
 	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	// stuff here ...
	ADD_EC_SPIN ("TS-Duration", msTime, OUT_OF_RANGE, &TS_duration, 0., 300000., ".2f", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("TS-Points", Unity, OUT_OF_RANGE, &TS_points, 16., 8192., ".0f", .1, 1.,  RemoteEntryList);
	ADD_EC_SPIN ("TS-Repetitions", Unity, OUT_OF_RANGE, &TS_repetitions, 1., 100., ".0f", 1, 10,  RemoteEntryList);
	ADD_STATUS ("Status", TS_status, expert_list);

	BUILD_STANDART_PROBE_CONTROLS (FALSE, TS_option_flags, callback_change_TS_option_flags, TS_auto_flags,  callback_change_TS_auto_flags,
				       DSPControl::Probing_exec_TS_callback, DSPControl::Probing_write_TS_callback, DSPControl::Probing_graph_callback,
				       DSPControl::Probing_abort_callback);

	gtk_widget_show_all (scrolled_contents);


// ==== Folder: LatMan (lateral manipulation) setup ========================================
 	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
 	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
 					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);

 	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
 				  scrolled_contents,
 				  gtk_label_new ("LM"));
 	gtk_widget_show (box);
	
	// ========================================
 	frame_param = gtk_frame_new (N_("Lateral (&Z) Manipulation at fixed Bias"));
 	gtk_widget_show (frame_param);
 	gtk_container_add (GTK_CONTAINER (box), frame_param);
 	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_widget_show (vbox_param);
 	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	// stuff here ...
	ADD_EC_SPIN ("dxyz", Angstroem, OUT_OF_RANGE, &LM_dx, -1000.0, 1000.0, "6.4g", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("dy", Angstroem, OUT_OF_RANGE, &LM_dy, -1000.0, 1000.0, "6.4g", 1., 10.,  RemoteEntryList);
	APP_EC_SPIN ("dz", Angstroem, OUT_OF_RANGE, &LM_dz, -1000.0, 1000.0, "6.4g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("LM-Points", Unity, OUT_OF_RANGE, &LM_points, 1, 4000, "5g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("LM-Slope", Speed, OUT_OF_RANGE, &LM_slope, 0.1,1000.0, "5.4g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Final-Delay", Time, OUT_OF_RANGE, &LM_final_delay, 0., 1., "5.3g", 0.001, 0.01,  RemoteEntryList);
	ADD_STATUS ("Status", LM_status, expert_list);

	BUILD_STANDART_PROBE_CONTROLS (TRUE, LM_option_flags, callback_change_LM_option_flags, LM_auto_flags, callback_change_LM_auto_flags,
				       DSPControl::Probing_exec_LM_callback, DSPControl::Probing_write_LM_callback, DSPControl::Probing_graph_callback,
				       DSPControl::Probing_abort_callback);

	gtk_widget_show_all (scrolled_contents);


// ==== Folder: LockIn  ========================================
	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);
		
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
				  scrolled_contents,
				  gtk_label_new ("LockIn"));
	gtk_widget_show (box);

	// ========================================
	frame_param = gtk_frame_new (N_("Digital Lock-In settings"));
	gtk_widget_show (frame_param);
	gtk_container_add (GTK_CONTAINER (box), frame_param);

	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	ADD_EC_SPIN ("AC-Amplitude", Volt, OUT_OF_RANGE, &AC_amp, 0., 1., "5g", 0.001, 0.01,  RemoteEntryList);
	ADD_EC_SPIN ("AC-Frequency", Frq, OUT_OF_RANGE, &AC_frq, 0., 2000., "5g", 10., 100.,  RemoteEntryList);
	ADD_EC_SPIN_I ("AC-Phase-A", Deg, OUT_OF_RANGE, &AC_phaseA, -360., 360., "5g", 1., 10.,  RemoteEntryList, " :A");
	APP_EC_SPIN_I ("AC-Phase-B", Deg, OUT_OF_RANGE, &AC_phaseB, -360., 360., "5g", 1., 10.,  RemoteEntryList, " :B");
	APP_EC_SPIN_LIST_I ("AC-Phase-Span", Deg, OUT_OF_RANGE, &AC_phase_span, -360., 360., "5g", 1., 10.,  RemoteEntryList, expert_list, " :span");
	ADD_EC_SPIN ("AC-avg-Cycles", Unity, OUT_OF_RANGE, &AC_lockin_avg_cycels, 1., 128., "5g", 1., 2.,  RemoteEntryList);
	APP_EC_SPIN_LIST_I ("AC-Points", Unity, OUT_OF_RANGE, &AC_points, 1, 1440, "5g", 1., 10.,  RemoteEntryList, expert_list, " #pts");
	ADD_EC_SPIN_LIST ("AC-Slope", PhiSpeed, OUT_OF_RANGE, &AC_phase_slope, 0.01,120.0, "5.4g", 1., 10.,  RemoteEntryList, expert_list);
	ADD_EC_SPIN_LIST ("AC-Final-Delay", Time, OUT_OF_RANGE, &AC_final_delay, 0., 1., "5.3g", 0.001, 0.01,  RemoteEntryList, expert_list);
	APP_EC_SPIN_LIST_I ("AC-Repetitions", Unity, OUT_OF_RANGE, &AC_repetitions, 1., 100., ".0f", 1, 10,  RemoteEntryList, expert_list, " #reps");
	ADD_STATUS ("Status", AC_status, expert_list);

	BUILD_STANDART_PROBE_CONTROLS (TRUE, AC_option_flags, callback_change_AC_option_flags, AC_auto_flags, callback_change_AC_auto_flags,
				       DSPControl::LockIn_exec_callback, DSPControl::LockIn_write_callback, DSPControl::Probing_graph_callback,
				       DSPControl::Probing_abort_callback);

// 	hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
// 	gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param);
// 	label = gtk_label_new (N_("Manual update:"));
// 	gtk_widget_set_size_request (label, 100, -1);
// 	gtk_box_pack_start (GTK_BOX (hbox_param), label, FALSE, TRUE, 0);
// 	expert_list = g_slist_prepend (expert_list, label);

// 	button = gtk_button_new_with_label(N_("Read"));
// 	gtk_widget_set_size_request (button, 100, -1);
// 	gtk_box_pack_start (GTK_BOX (hbox_param), button, TRUE, TRUE, 0);
// 	g_signal_connect (G_OBJECT (button), "clicked",
// 			    G_CALLBACK (DSPControl::LockIn_read_callback), this);
// 	expert_list = g_slist_prepend (expert_list, button);

// 	button = gtk_button_new_with_label(N_("Write"));
// 	gtk_widget_set_size_request (button, 100, -1);
// 	gtk_box_pack_start (GTK_BOX (hbox_param), button, TRUE, TRUE, 0);
// 	g_signal_connect (G_OBJECT (button), "clicked",
// 			    G_CALLBACK (DSPControl::LockIn_write_callback), this);
// 	expert_list = g_slist_prepend (expert_list, button);

        gtk_widget_show_all (scrolled_contents);


// ==== Folder: AX Auxillary Channel (QMA, CMA, ...)  ========================================
	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);
		
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
				  scrolled_contents,
				  gtk_label_new ("AX"));
	gtk_widget_show (box);

	// ========================================
 	frame_param = gtk_frame_new (N_("Auxillary Spectroscopy (Experimental)"));

 	gtk_widget_show (frame_param);
 	gtk_container_add (GTK_CONTAINER (box), frame_param);
 	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_widget_show (vbox_param);
 	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	// stuff here ...
	ADD_EC_SPIN ("V-Start-End", Volt, OUT_OF_RANGE, &AX_start, -10.0, 10., "6g", 0.1, 0.025,  RemoteEntryList);
	APP_EC_SPIN ("V-End", Volt, OUT_OF_RANGE, &AX_end, -10.0, 10.0, "6g", 0.1, 0.025,  RemoteEntryList);
	ADD_LABEL_LIST ("#", expert_list);
	APP_EC_SPIN_LIST ("AX-rep", Unity, OUT_OF_RANGE, &AX_repetitions, 1., 100., "5g", 1., 5.,  RemoteEntryList, expert_list);
	ADD_EC_SPIN ("Points", Unity, OUT_OF_RANGE, &AX_points, 1., 1000.0, "5g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("V-Slope", Vslope, OUT_OF_RANGE, &AX_slope,0.1,1000.0, "6g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Slope-Ramp", Vslope, OUT_OF_RANGE, &AX_slope_ramp,0.1,1000.0, "6g", 1., 10.,  RemoteEntryList);
	ADD_EC_SPIN ("Final-Delay", Time, OUT_OF_RANGE, &AX_final_delay, 0., 1., "6g", 0.001, 0.01,  RemoteEntryList);
	ADD_EC_SPIN ("GateTime", Time, OUT_OF_RANGE, &AX_gatetime, 0.1e-6, 355., "8g", 0.01, 0.1,  RemoteEntryList);
//	ADD_EC_SPIN ("Gain", Time, OUT_OF_RANGE, &AX_gain, 0., 10., "5.3g", 0.001, 0.01,  RemoteEntryList);
//	ADD_EC_SPIN ("Resolution", Time, OUT_OF_RANGE, &AX_resolution, 0., 10., "5.3g", 0.001, 0.01,  RemoteEntryList);
	ADD_STATUS ("Status", AX_status, expert_list);

	BUILD_STANDART_PROBE_CONTROLS (TRUE, AX_option_flags, callback_change_AX_option_flags, AX_auto_flags,  callback_change_AX_auto_flags,
				       DSPControl::Probing_exec_AX_callback, DSPControl::Probing_write_AX_callback, DSPControl::Probing_graph_callback,
				       DSPControl::Probing_abort_callback);

	gtk_widget_show_all (scrolled_contents);


// ==== Folder: Graphs -- Vector Probe Data Visualisation setup ========================================
 	scrolled_contents = gtk_scrolled_window_new (NULL, NULL);
 	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_contents), 
 					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_contents), box);

 	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
 				  scrolled_contents,
 				  gtk_label_new ("Graphs"));
 	gtk_widget_show (box);

        gtk_widget_show_all (scrolled_contents);

	frame_param = gtk_frame_new(N_("Probe Sources & Graph Setup"));
	gtk_widget_show(frame_param);
	gtk_container_add(GTK_CONTAINER (box), frame_param);
	// source channel setup
	GtkWidget *hbox_source;
        hbox_source = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add (GTK_CONTAINER (frame_param), hbox_source);
	
        GtkWidget *tab = gtk_table_new (12,8, FALSE);
	gtk_container_add (GTK_CONTAINER (hbox_source), tab);

	int   msklookup[] = { 0x000020, 0x000040, 0x000080, 0x000100, 0x000200, 0x000010, 0x000400, 0x000800,
			      0x000001, 0x000002, 
			      0x000008, 0x001000, 0x002000, 0x004000, 0x008000, 0x000004,
			      0x0100000, 0x0200000, 0x0400000, 0x0800000, 0x1000000, 0x2000000, 0x4000000,
			      -1 
	};
	const char* lablookup[] = { "ADC0", "ADC1", "ADC2", "ADC3", "ADC4", "ADC5-I","ADC6","ADC7",
				    "Zmon", "Umon",
				    "LockIn0", "LockIn1stA",  "LockIn1stB", "LockIn2ndA", "LockIn2ndB", "Counter",
				    "Time", "XS", "YS", "ZS", "U", "PHI", "SEC",
				    NULL
	};
	
	{
		int c=0;
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("Source"), c+0, c+1, 0, 1, GTK_FILL, GTK_FILL, 0,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("X"), c+1, c+2, 0, 1, GTK_FILL, GTK_FILL, 0,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("Y"), c+2, c+3, 0, 1, GTK_FILL, GTK_FILL, 6,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("A"), c+3, c+4, 0, 1, GTK_FILL, GTK_FILL, 6,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("S"), c+4, c+5, 0, 1, GTK_FILL, GTK_FILL, 6,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new (" "), c+5, c+6, 0, 1, GTK_FILL, GTK_FILL, 6,5);
		c+=7;
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("Source"), c+0, c+1, 0, 1, GTK_FILL, GTK_FILL, 0,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("X"), c+1, c+2, 0, 1, GTK_FILL, GTK_FILL, 0,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("Y"), c+2, c+3, 0, 1, GTK_FILL, GTK_FILL, 6,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("A"), c+3, c+4, 0, 1, GTK_FILL, GTK_FILL, 6,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("S"), c+4, c+5, 0, 1, GTK_FILL, GTK_FILL, 6,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new (" "), c+5, c+6, 0, 1, GTK_FILL, GTK_FILL, 6,5);
		c+=7;
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("Data"), c+0, c+1, 0, 1, GTK_FILL, GTK_FILL, 0,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("X"), c+1, c+2, 0, 1, GTK_FILL, GTK_FILL, 0,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("Y"), c+2, c+3, 0, 1, GTK_FILL, GTK_FILL, 6,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("A"), c+3, c+4, 0, 1, GTK_FILL, GTK_FILL, 6,5);
		gtk_table_attach (GTK_TABLE(tab), gtk_label_new ("S"), c+4, c+5, 0, 1, GTK_FILL, GTK_FILL, 6,5);
	}

	for (i=0; msklookup[i] >= 0 && lablookup[i]; ++i){
		if (!msklookup[i]) 
			continue;
		int c=i/8; 
		c*=7;
		ADD_TOGGLE_TO_TAB (lablookup[i], tab, c, i%8+1, (int) msklookup[i], expert_list);
		ADD_TOGGLE_TO_TAB (" ", tab, c+1, i%8+1, (int) (X_SOURCE_MSK | msklookup[i]), expert_list);
		ADD_TOGGLE_TO_TAB (" ", tab, c+2, i%8+1, (int) (P_SOURCE_MSK | msklookup[i]), expert_list);
		ADD_TOGGLE_TO_TAB (" ", tab, c+3, i%8+1, (int) (A_SOURCE_MSK | msklookup[i]), expert_list);
		ADD_TOGGLE_TO_TAB (" ", tab, c+4, i%8+1, (int) (S_SOURCE_MSK | msklookup[i]), expert_list);
	}


	frame_param = gtk_frame_new(N_("Plot / Save current data in buffer"));
	gtk_widget_show(frame_param);
	gtk_container_add(GTK_CONTAINER (box), frame_param);
	vbox_param = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show (vbox_param);
	gtk_container_add (GTK_CONTAINER (frame_param), vbox_param);

	hbox_param = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add (GTK_CONTAINER (vbox_param), hbox_param);

	button = gtk_button_new_with_label(N_("Plot"));
	gtk_widget_set_size_request (button, 100, -1);
	gtk_box_pack_start (GTK_BOX (hbox_param), button, TRUE, TRUE, 0);
	gtk_widget_show (button);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (DSPControl::Probing_graph_callback), this);
	
	button = gtk_button_new_with_label(N_("Save"));
	gtk_widget_set_size_request (button, 100, -1);
	gtk_box_pack_start (GTK_BOX (hbox_param), button, TRUE, TRUE, 0);
	gtk_widget_show (button);
	g_signal_connect (G_OBJECT (button), "clicked",
			    G_CALLBACK (DSPControl::Probing_save_callback), this);
	
	save_button = button;

	gtk_widget_show_all (scrolled_contents);

// ==== Folder List: Vector Probes ========================================
// Automatic frame generation


// ==== finish auto_probe_menu now
	gtk_combo_box_set_active (GTK_COMBO_BOX (auto_probe_menu), 0);

// ------------------------------------------------------------
	sranger_hwi_pi.app->RemoteEntryList = g_slist_concat (sranger_hwi_pi.app->RemoteEntryList, RemoteEntryList);

	// save List away...
	g_object_set_data( G_OBJECT (window), "DSP_EC_list", EC_list);
	g_object_set_data( G_OBJECT (expert_checkbutton), "DSP_expert_list", expert_list);
	g_object_set_data( G_OBJECT (guru_checkbutton), "DSP_guru_list", guru_list);

	DSP_expert_callback (expert_checkbutton, this);
	DSP_guru_callback (guru_checkbutton, this);
}



static void remove(gpointer entry, gpointer from){
	from = (gpointer) g_slist_remove ((GSList*)from, entry);
}

DSPControl::~DSPControl (){
	store_values ();

	// remove all left over pc's from pc matrix
	for (int i=0; i < 2*MAX_NUM_CHANNELS; ++i) 
		for (int j=0; j < 2*MAX_NUM_CHANNELS; ++j) 
			if (probe_pc_matrix[i][j]){
				delete probe_pc_matrix[i][j];
				probe_pc_matrix[i][j] = NULL; // just to be clean
			}

	g_slist_foreach(RemoteEntryList, (GFunc) remove, sranger_hwi_pi.app->RemoteEntryList);
	g_slist_free (RemoteEntryList);
	RemoteEntryList = NULL;

	free_probedata_arrays ();

	delete Unity;
	delete Volt;
	delete Angstroem;
	delete Frq;
	delete Deg;
	delete Current;
	delete Speed;
	delete PhiSpeed;
	delete Vslope;
	delete Time;
	delete TimeUms;
	delete msTime;
	delete minTime;
	delete SetPtUnit;
}

void DSPControl::store_values (){
	XsmRescourceManager xrm("sranger_hwi_control");
        xrm.Put ("frq_ref", frq_ref);
	xrm.Put ("bias", bias);
	xrm.Put ("current_set_point", current_set_point);
	xrm.Put ("voltage_set_point", voltage_set_point);
	xrm.Put ("move_speed_x", move_speed_x);
	xrm.Put ("scan_speed_x", scan_speed_x_requested);
	xrm.Put ("gain_ratio", gain_ratio);
	xrm.Put ("usr_cp", usr_cp);
	xrm.Put ("usr_ci", usr_ci);
	xrm.Put ("dynamic_zoom", dynamic_zoom);
	xrm.Put ("pre_points", pre_points);
	xrm.Put ("expert_mode", expert_mode);
	xrm.Put ("guru_mode", guru_mode);
	xrm.Put ("center_return_flag", center_return_flag);
	xrm.Put ("area_slope_compensation_flag", area_slope_compensation_flag);
	xrm.Put ("area_slope_x", area_slope_x);
	xrm.Put ("area_slope_y", area_slope_y);

	// LockIn and LockIn phase probe
	xrm.Put ("AC_amp", AC_amp);
	xrm.Put ("AC_frq", AC_frq);
	xrm.Put ("AC_phaseA", AC_phaseA);
	xrm.Put ("AC_phaseB", AC_phaseB);
	xrm.Put ("AC_lockin_avg_cycels", AC_lockin_avg_cycels);
	xrm.Put ("AC_phase_span", AC_phase_span);
	xrm.Put ("AC_points", AC_points);
	xrm.Put ("AC_repetitions", AC_repetitions);
	xrm.Put ("AC_phase_slope", AC_phase_slope);
	xrm.Put ("AC_final_delay", AC_final_delay);
	xrm.Put ("AC_option_flags", AC_option_flags);
	xrm.Put ("AC_auto_flags", AC_auto_flags);

	// Probing
	xrm.Put ("Probing_Sources", Source);
	xrm.Put ("Probing_XSources", XSource);
	xrm.Put ("Probing_PSources", PSource);
	xrm.Put ("Probing_PlotAvg", PlotAvg);
	xrm.Put ("Probing_PlotSec", PlotSec);
	xrm.Put ("Probing_probe_trigger_raster_points", probe_trigger_raster_points_user);
	// STS
	xrm.Put ("Probing_IV_Ustart", IV_start);
	xrm.Put ("Probing_IV_Uend", IV_end);
	xrm.Put ("Probing_IV_repetitions", IV_repetitions);
	xrm.Put ("Probing_IV_dz", IV_dz);
	xrm.Put ("Probing_IVdz_repetitions", IVdz_repetitions);
	xrm.Put ("Probing_IV_points", IV_points);
	xrm.Put ("Probing_IV_slope", IV_slope);
	xrm.Put ("Probing_IV_slope_ramp", IV_slope_ramp);
	xrm.Put ("Probing_IV_final_delay", IV_final_delay);
	xrm.Put ("Probing_IV_recover_delay", IV_recover_delay);
	xrm.Put ("Probing_IV_dx", IV_dx);
	xrm.Put ("Probing_IV_dy", IV_dy);
	xrm.Put ("Probing_IV_dxy_slope", IV_dxy_slope);
	xrm.Put ("Probing_IV_dxy_delay", IV_dxy_delay);
	xrm.Put ("Probing_IV_dxy_points", IV_dxy_points);
	xrm.Put ("Probing_IV_option_flags", IV_option_flags);
	xrm.Put ("Probing_IV_auto_flags", IV_auto_flags);
	// FZ
	xrm.Put ("Probing_FZ_Z_start", FZ_start);
	xrm.Put ("Probing_FZ_Z_end", FZ_end);
	xrm.Put ("Probing_FZ_points", FZ_points);
	xrm.Put ("Probing_FZ_slope", FZ_slope);
	xrm.Put ("Probing_FZ_slope_ramp", FZ_slope_ramp);
	xrm.Put ("Probing_FZ_final_delay", FZ_final_delay);
	xrm.Put ("Probing_FZ_option_flags", FZ_option_flags);
	xrm.Put ("Probing_FZ_auto_flags", FZ_auto_flags);
	// PL
	xrm.Put ("Probing_PL_duration", PL_duration);
	xrm.Put ("Probing_PL_volt", PL_volt);
	xrm.Put ("Probing_PL_slope", PL_slope);
	xrm.Put ("Probing_PL_final_delay", PL_final_delay);
	xrm.Put ("Probing_PL_repetitions", PL_repetitions);
	xrm.Put ("Probing_PL_option_flags", PL_option_flags);
	xrm.Put ("Probing_PL_auto_flags", PL_auto_flags);
	// LP
	xrm.Put ("Probing_LP_duration", LP_duration);
	xrm.Put ("Probing_LP_triggertime", LP_triggertime);
	xrm.Put ("Probing_LP_volt", LP_volt);
	xrm.Put ("Probing_LP_slope", LP_slope);
	xrm.Put ("Probing_LP_final_delay", LP_final_delay);
	xrm.Put ("Probing_LP_repetitions", LP_repetitions);
	xrm.Put ("Probing_LP_FZ_end", LP_FZ_end);
	xrm.Put ("Probing_LP_option_flags", LP_option_flags);
	xrm.Put ("Probing_LP_auto_flags", LP_auto_flags);
	// SP
	xrm.Put ("Probing_SP_duration", SP_duration);
	xrm.Put ("Probing_SP_volt", SP_volt);
	xrm.Put ("Probing_SP_ramptime", SP_ramptime);
	xrm.Put ("Probing_SP_flag_volt", SP_flag_volt);
	xrm.Put ("Probing_SP_final_delay", SP_final_delay);
	xrm.Put ("Probing_SP_repetitions", SP_repetitions);
	xrm.Put ("Probing_SP_option_flags", SP_option_flags);
	xrm.Put ("Probing_SP_auto_flags", SP_auto_flags);
	// TS
	xrm.Put ("Probing_TS_duration", TS_duration);
	xrm.Put ("Probing_TS_points", TS_points);
	xrm.Put ("Probing_TS_repetitions", TS_repetitions);
	xrm.Put ("Probing_TS_option_flags", TS_option_flags);
	xrm.Put ("Probing_TS_auto_flags", TS_auto_flags);
	// LM
	xrm.Put ("Probing_LM_dx", LM_dx);
	xrm.Put ("Probing_LM_dy", LM_dy);
	xrm.Put ("Probing_LM_dz", LM_dz);
	xrm.Put ("Probing_LM_points", LM_points);
	xrm.Put ("Probing_LM_slope", LM_slope);
	xrm.Put ("Probing_LM_final_delay", LM_final_delay);
	xrm.Put ("Probing_LM_option_flags", LM_option_flags);
	xrm.Put ("Probing_LM_auto_flags", LM_auto_flags);
	// AX
	xrm.Put ("Probing_AX_start", AX_start);
	xrm.Put ("Probing_AX_end", AX_end);
	xrm.Put ("Probing_AX_points", AX_points);
	xrm.Put ("Probing_AX_repetitions", AX_repetitions);
	xrm.Put ("Probing_AX_slope", AX_slope);
	xrm.Put ("Probing_AX_slope_ramp", AX_slope_ramp);
	xrm.Put ("Probing_AX_final_delay", AX_final_delay);
	xrm.Put ("Probing_AX_gatetime", AX_gatetime);
	xrm.Put ("Probing_AX_gain", AX_gain);
	xrm.Put ("Probing_AX_resolution", AX_resolution);
	xrm.Put ("Probing_AX_option_flags", AX_option_flags);
	xrm.Put ("Probing_AX_auto_flags", AX_auto_flags);
}

void DSPControl::save_values (NcFile *ncf){

	g_print ("DSPControl::save_values\n");
	gchar *i=NULL;
	gchar *hwii=NULL;
	if (sranger_hwi_hardware)
		hwii = sranger_hwi_hardware->get_info ();

	if (IS_AFM_CTRL) // AFM (linear)
		i = g_strconcat ("SRanger HwI interface: AFM mode selected.\nHardware-Info:\n", hwii, NULL);
	else
		i = g_strconcat ("SRanger HwI interface: STM mode selected.\nHardware-Info:\n", hwii, NULL);

	NcDim* infod  = ncf->add_dim("sranger_info_dim", strlen(i));
	NcVar* info   = ncf->add_var("sranger_info", ncChar, infod);
	info->add_att("long_name", "SRanger HwI plugin information");
	info->put(i, strlen(i));
	g_free (i);
	g_free (hwii);


// Basic Feedback/Scan Parameter ============================================================

	NcVar* ncv_bias = ncf->add_var ("sranger_hwi_bias", ncDouble);
	ncv_bias->add_att ("long_name", "SRanger: (Sample or Tip) Bias Voltage");
	ncv_bias->add_att ("short_name", "Bias");
	ncv_bias->add_att ("unit", "V");
	ncv_bias->add_att ("var_unit", "V");
	ncv_bias->add_att ("label", "Bias");
	ncv_bias->put (&bias);


	if (IS_AFM_CTRL){ // AFM (linear)
		NcVar* ncv_sp = ncf->add_var ("sranger_hwi_voltage_set_point", ncDouble);
		ncv_sp->add_att ("long_name", "SRanger: Feedback Set Point (AFM, linear FB mode)");
		ncv_sp->add_att ("short_name", "Set Point");
		ncv_sp->add_att ("unit", xsmres.daqZunit[0]);
		ncv_sp->add_att ("type", xsmres.daqZtype[0]);
		ncv_sp->add_att ("label", xsmres.daqZlabel[0]);
		ncv_sp->put (&voltage_set_point);
	} else {
		NcVar* ncv_sp = ncf->add_var ("sranger_hwi_current_set_point", ncDouble);
		ncv_sp->add_att ("long_name", "SRanger: Current set point (STM, log FB mode)");
		ncv_sp->add_att ("short_name", "Current");
		ncv_sp->add_att ("unit", "nA");
		ncv_sp->add_att ("label", "Current");
		ncv_sp->put (&current_set_point);
	}

	NcVar* ncv_move_speed = ncf->add_var ("sranger_hwi_move_speed_x", ncDouble);
	ncv_move_speed->add_att ("long_name", "SRanger: Move speed X");
	ncv_move_speed->add_att ("short_name", "Xm Velocity");
	ncv_move_speed->add_att ("unit", "A/s");
	ncv_move_speed->add_att ("label", "Velocity Xm");
	ncv_move_speed->put (&move_speed_x);

	NcVar* ncv_scan_speed = ncf->add_var ("sranger_hwi_scan_speed_x", ncDouble);
	ncv_scan_speed->add_att ("long_name", "SRanger: Scan speed X");
	ncv_scan_speed->add_att ("short_name", "Xs Velocity");
	ncv_scan_speed->add_att ("unit", "A/s");
	ncv_scan_speed->add_att ("label", "Velocity Xs");
	ncv_scan_speed->put (&scan_speed_x);

	NcVar* ncv_usr_cp = ncf->add_var ("sranger_hwi_usr_cp", ncDouble);
	ncv_usr_cp->add_att ("long_name", "SRanger: User CP");
	ncv_usr_cp->add_att ("short_name", "CP");
	ncv_usr_cp->add_att ("var_unit", "1");
	ncv_usr_cp->put (&usr_cp);

	NcVar* ncv_usr_ci = ncf->add_var ("sranger_hwi_usr_ci", ncDouble);
	ncv_usr_ci->add_att ("long_name", "SRanger: User CI");
	ncv_usr_ci->add_att ("short_name", "CI");
	ncv_usr_ci->add_att ("var_unit", "1");
	ncv_usr_ci->put (&usr_ci);

	NcVar* ncv_pre_points = ncf->add_var ("sranger_hwi_pre_points", ncDouble);
	ncv_pre_points->add_att ("long_name", "SRanger: Pre-Scanline points");
	ncv_pre_points->add_att ("short_name", "Pre-S points");
	ncv_pre_points->add_att ("var_unit", "1");
	ncv_pre_points->put (&pre_points);

	NcVar* ncv_Inst_VX = ncf->add_var ("sranger_hwi_XSM_Inst_VX", ncDouble);
	ncv_Inst_VX->add_att ("long_name", "FYI only::SRanger/XSM: Instrument VX (X-gain)");
	ncv_Inst_VX->add_att ("short_name", "VX");
	ncv_Inst_VX->add_att ("var_unit", "1");
	{ double tmp=sranger_hwi_pi.app->xsm->Inst->VX (); ncv_Inst_VX->put (&tmp); }

	NcVar* ncv_Inst_VY = ncf->add_var ("sranger_hwi_XSM_Inst_VY", ncDouble);
	ncv_Inst_VY->add_att ("long_name", "FYI only::SRanger/XSM: Instrument VY (Y-gain)");
	ncv_Inst_VY->add_att ("short_name", "VY");
	ncv_Inst_VY->add_att ("var_unit", "1");
	{ double tmp=sranger_hwi_pi.app->xsm->Inst->VY (); ncv_Inst_VY->put (&tmp); }

	NcVar* ncv_Inst_VZ = ncf->add_var ("sranger_hwi_XSM_Inst_VZ", ncDouble);
	ncv_Inst_VZ->add_att ("long_name", "FYI only::SRanger/XSM: Instrument VZ (Z-gain)");
	ncv_Inst_VZ->add_att ("short_name", "VZ");
	ncv_Inst_VZ->add_att ("var_unit", "1");
	{ double tmp=sranger_hwi_pi.app->xsm->Inst->VZ (); ncv_Inst_VZ->put (&tmp); }

	if (gapp->xsm->Inst->OffsetMode() == OFM_ANALOG_OFFSET_ADDING){
		NcVar* ncv_Inst_VX0 = ncf->add_var ("sranger_hwi_XSM_Inst_VX0", ncDouble);
		ncv_Inst_VX0->add_att ("long_name", "FYI only::SRanger/XSM: Instrument VX0 (X0-gain) used for analog offset adding");
		ncv_Inst_VX0->add_att ("short_name", "VX0");
		ncv_Inst_VX0->add_att ("var_unit", "1");
		{ double tmp=sranger_hwi_pi.app->xsm->Inst->VX0 (); ncv_Inst_VX0->put (&tmp); }
		
		NcVar* ncv_Inst_VY0 = ncf->add_var ("sranger_hwi_XSM_Inst_VY0", ncDouble);
		ncv_Inst_VY0->add_att ("long_name", "FYI only::SRanger/XSM: Instrument VY0 (Y0-gain) used for analog offset adding");
		ncv_Inst_VY0->add_att ("short_name", "VY0");
		ncv_Inst_VY0->add_att ("var_unit", "1");
		{ double tmp=sranger_hwi_pi.app->xsm->Inst->VY0 (); ncv_Inst_VY0->put (&tmp); }
		
		NcVar* ncv_Inst_VZ0 = ncf->add_var ("sranger_hwi_XSM_Inst_VZ0", ncDouble);
		ncv_Inst_VZ0->add_att ("long_name", "FYI only::SRanger/XSM: Instrument VZ0 (Z0-gain) used for analog offset adding");
		ncv_Inst_VZ0->add_att ("short_name", "VZ0");
		ncv_Inst_VZ0->add_att ("var_unit", "1");
		{ double tmp=sranger_hwi_pi.app->xsm->Inst->VZ0 (); ncv_Inst_VZ0->put (&tmp); }
	}

// LockIn ============================================================

	NcVar* ncv_tmp = ncf->add_var ("sranger_hwi_AC_amp", ncDouble);
	ncv_tmp->add_att ("long_name", "SRanger: (Sample) AC Amplitude (LockIn)");
	ncv_tmp->add_att ("var_unit", "Volt");
	ncv_tmp->put (&AC_amp);

	ncv_tmp = ncf->add_var ("sranger_hwi_AC_frq", ncDouble);
	ncv_tmp->add_att ("long_name", "SRanger: (Sample) AC Frequency (LockIn)");
	ncv_tmp->add_att ("var_unit", "Hertz");
	ncv_tmp->put (&AC_frq);

	ncv_tmp = ncf->add_var ("sranger_hwi_AC_phaseA", ncDouble);
	ncv_tmp->add_att ("long_name", "SRanger: (Sample) AC Phase A (LockIn)");
	ncv_tmp->add_att ("var_unit", "deg");
	ncv_tmp->put (&AC_phaseA);

	ncv_tmp = ncf->add_var ("sranger_hwi_AC_phaseB", ncDouble);
	ncv_tmp->add_att ("long_name", "SRanger: (Sample) AC Phase B (LockIn)");
	ncv_tmp->add_att ("var_unit", "deg");
	ncv_tmp->put (&AC_phaseB);

	ncv_tmp = ncf->add_var ("sranger_hwi_AC_lockin_avg_cycels", ncInt);
	ncv_tmp->add_att ("long_name", "SRanger: (Sample) AC LockIn average cycels (LockIn)");
	ncv_tmp->add_att ("var_unit", "#");
	ncv_tmp->put (&AC_lockin_avg_cycels);

// Vector Probe ============================================================
// to-do !!!???


// Scan Event Trigger ============================================================
	if (dsp_scan_event_trigger.pflg){
		for (int i=0; i<8; ++i){
			gchar* var = g_strdup_printf ("sranger_hwi_Trigger_Xp_at_%d", i);
			NcVar* ncv_trig = ncf->add_var (var, ncShort);
			g_free (var);
			ncv_trig->add_att ("long_name", "SRanger: Scan Event Xp Trigger at");
			ncv_trig->add_att ("var_unit", "RevIndex");
			ncv_trig->put (&dsp_scan_event_trigger.trig_i_xp[i]);

			if (i<4){
				var = g_strdup_printf ("sranger_hwi_Trigger_Xp_Bias_%d", i);
				ncv_trig = ncf->add_var (var, ncDouble);
				g_free (var);
				ncv_trig->add_att ("long_name", "SRanger: Scan Event Xp Bias");
				ncv_trig->add_att ("var_unit", "Volt");
			} else {
				var = g_strdup_printf ("sranger_hwi_Trigger_Xp_Current_%d", i);
				ncv_trig = ncf->add_var (var, ncDouble);
				g_free (var);
				ncv_trig->add_att ("long_name", "SRanger: Scan Event Xp Current");
				ncv_trig->add_att ("var_unit", "nA");
			}
			ncv_trig->put (&trigger_bias_setpoint_xp[i]);

			var = g_strdup_printf ("sranger_hwi_Trigger_Xm_at_%d", i);
			ncv_trig = ncf->add_var (var, ncShort);
			g_free (var);
			ncv_trig->add_att ("long_name", "SRanger: Scan Event Trigger Xm at");
			ncv_trig->add_att ("var_unit", "RevIndex");
			ncv_trig->put (&dsp_scan_event_trigger.trig_i_xm[i]);

			if (i<4){
				var = g_strdup_printf ("sranger_hwi_Trigger_Xm_Bias_%d", i);
				ncv_trig = ncf->add_var (var, ncDouble);
				g_free (var);
				ncv_trig->add_att ("long_name", "SRanger: Scan Event Xm Bias");
				ncv_trig->add_att ("var_unit", "Volt");
			} else {
				var = g_strdup_printf ("sranger_hwi_Trigger_Xm_Current_%d", i);
				ncv_trig = ncf->add_var (var, ncDouble);
				g_free (var);
				ncv_trig->add_att ("long_name", "SRanger: Scan Event Xm Current");
				ncv_trig->add_att ("var_unit", "nA");
			}
			ncv_trig->put (&trigger_bias_setpoint_xm[i]);
		}
	}
}


#define NC_GET_VARIABLE(VNAME, VAR) if(ncf->get_var(VNAME)) ncf->get_var(VNAME)->get(VAR)

void DSPControl::load_values (NcFile *ncf){
	g_print ("DSPControl::load_values\n");
	// Values will also be written in old style DSP Control window for the reason of backwards compatibility
	// OK -- but will be obsoleted and removed at any later point -- PZ
	NC_GET_VARIABLE ("sranger_hwi_bias", &bias);
	NC_GET_VARIABLE ("sranger_hwi_bias", &gapp->xsm->data.s.Bias);
        NC_GET_VARIABLE ("sranger_hwi_voltage_set_point", &voltage_set_point);
        NC_GET_VARIABLE ("sranger_hwi_current_set_point", &current_set_point);

	update ();
}


#define CONV_16_C(X) sranger_hwi_hardware->int_2_sranger_int (X)
#define CONV_32_C(X) sranger_hwi_hardware->long_2_sranger_long (X)

#define CONV_16(X) X = sranger_hwi_hardware->int_2_sranger_int (X)
#define CONV_32(X) X = sranger_hwi_hardware->long_2_sranger_long (X)

gint DSPControl::SetUserParam (gint n, gchar *id, double value){
	switch (n){
	case 6: // ENERGY in V --> AIC6 
		bias = value;
		read_dsp_analog ();
		dsp_analog.bias = (int)(gapp->xsm->Inst->VoltOut2Dig (bias));
		write_dsp_analog ();
		break;
	case 10: { // GATETIME in ms
		double gatetime = value*1e3; // value is in ms

		// setup counter/timer
		dsp_cr_generic_io.start = CONV_16_C (0);           /* Initiate =WO -- no action, only set mode and gatetime now! */
		dsp_cr_generic_io.stop  = CONV_16_C (0);           /* Cancel   =WO */
		dsp_cr_generic_io.port  = CONV_16_C (0);           /* --- init w zero --- */
		dsp_cr_generic_io.rw    = CONV_16_C (0);           /* --- init w zero --- */
		dsp_cr_generic_io.data_lo  = CONV_16_C (0);        /* --- init w zero --- */
		dsp_cr_generic_io.data_hi  = CONV_16_C (0);        /* --- init w zero --- */
		
		if (gatetime < 3e-3){
			dsp_cr_generic_io.mode     = CONV_16_C (3);    /* Gate Timer Mode with reset (FF=3 (100ns), Fast=5, Slow=9) */
			dsp_cr_generic_io.gatetime = CONV_16_C ((int)(round(gatetime/CPLD_GATEREF)));     /* Timer, downcounter for gatetime */
		} else if (gatetime < 1.5){
			dsp_cr_generic_io.mode     = CONV_16_C (5);    /* Gate Timer Mode with reset (FF=3 (100ns), Fast=5, Slow=9) */
			dsp_cr_generic_io.gatetime = CONV_16_C ((int)(round(gatetime/CPLD_GATEREF/500.)));     /* Timer, downcounter for gatetime */
		} else {
			dsp_cr_generic_io.mode     = CONV_16_C (9);    /* Gate Timer Mode with reset (FF=3 (100ns), Fast=5, Slow=9) */
			dsp_cr_generic_io.gatetime = CONV_16_C ((int)(round(gatetime/CPLD_GATEREF/50000.)));     /* Timer, downcounter for gatetime */
		}

		lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.CR_generic_io, SRANGER_SEEK_DATA_SPACE);
		sranger_hwi_hardware->sr_write (sranger_hwi_hardware->dsp, &dsp_cr_generic_io,  MAX_WRITE_CR_GENERIC_IO<<1); 
	        }
		break;
	default: break;
	}
}

double DSPControl::GetUserParam (gint n, gchar *id){
	return 0.;
}

void DSPControl::read_dsp_state (){
	if (!sranger_hwi_hardware) return; 

	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.statemachine, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_read (sranger_hwi_hardware->dsp, &dsp_state, sizeof(dsp_state)); 

	CONV_16 (dsp_state.mode);
	CONV_16 (dsp_state.DataProcessTime);
	CONV_16 (dsp_state.IdleTime);
}

void DSPControl::write_dsp_state (){
}
	
void DSPControl::conv_dsp_feedback (){
	CONV_16 (dsp_feedback.cp);
	CONV_16 (dsp_feedback.ci);
	CONV_16 (dsp_feedback.setpoint);
}

void DSPControl::read_dsp_feedback (){
	if (!sranger_hwi_hardware) return; 

	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.feedback, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_read (sranger_hwi_hardware->dsp, &dsp_feedback, MAX_WRITE_SPM_PI_FEEDBACK<<1); 

	// from DSP to PC -- only SetPoint!
	conv_dsp_feedback ();
}

void DSPControl::write_dsp_feedback (){
	if (!sranger_hwi_hardware) return; 

	// Scale Regler Consts. with 1/VZ and convert to SR-Q15
	dsp_feedback.cp = sranger_hwi_hardware->float_2_sranger_q15 (usr_cp / sranger_hwi_pi.app->xsm->Inst->VZ ());
	dsp_feedback.ci = sranger_hwi_hardware->float_2_sranger_q15 (usr_ci / sranger_hwi_pi.app->xsm->Inst->VZ ());

	// from PC to DSP
	conv_dsp_feedback ();

	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.feedback, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_write (sranger_hwi_hardware->dsp, &dsp_feedback, MAX_WRITE_SPM_PI_FEEDBACK<<1); 

	// from DSP to PC
	conv_dsp_feedback ();
}

void DSPControl::conv_dsp_analog (){
	CONV_16 (dsp_analog.bias);
}

void DSPControl::read_dsp_analog (){
	// only "bias" is touched here!
	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.analog, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_read  (sranger_hwi_hardware->dsp, &dsp_analog, sizeof (dsp_analog)); 

	// from DSP to PC
	conv_dsp_analog ();
}

void DSPControl::write_dsp_analog (){
	if (!sranger_hwi_hardware) return; 

	// only "bias" is touched here!

	// from PC to DSP
	conv_dsp_analog ();

	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.analog+START_WRITE_ANALOG, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_write (sranger_hwi_hardware->dsp, &dsp_analog.x_offset, MAX_WRITE_ANALOG<<1); 

	// from DSP to PC
	conv_dsp_analog ();
}

void DSPControl::conv_dsp_scan (){
//	CONV_16 (dsp_scan.rotxx);
//	CONV_16 (dsp_scan.rotxy);
//	CONV_16 (dsp_scan.rotyx);
//	CONV_16 (dsp_scan.rotyy);
	CONV_16 (dsp_scan.nx_pre);
	CONV_16 (dsp_scan.dnx_probe);
	CONV_16 (dsp_scan.raster_a);
	CONV_16 (dsp_scan.raster_b);
	CONV_32 (dsp_scan.srcs_xp);
	CONV_32 (dsp_scan.srcs_xm);
	CONV_32 (dsp_scan.srcs_2nd_xp);
	CONV_32 (dsp_scan.srcs_2nd_xm);
	CONV_16 (dsp_scan.nx);
	CONV_16 (dsp_scan.ny);
	CONV_32 (dsp_scan.fs_dx);
	CONV_32 (dsp_scan.fs_dy);
	CONV_32 (dsp_scan.num_steps_move_xy);
	CONV_32 (dsp_scan.fm_dx);
	CONV_32 (dsp_scan.fm_dy);
	CONV_32 (dsp_scan.fm_dzxy);
	CONV_32 (dsp_scan.fm_dzx);
	CONV_32 (dsp_scan.fm_dzy);
	CONV_16 (dsp_scan.dnx);
	CONV_16 (dsp_scan.dny);
	CONV_16 (dsp_scan.Zoff_2nd_xp);
	CONV_16 (dsp_scan.Zoff_2nd_xm);
	CONV_32 (dsp_scan.Xpos);
	CONV_32 (dsp_scan.Ypos);
	CONV_32 (dsp_scan.cfs_dx);
	CONV_32 (dsp_scan.cfs_dy);
// RO:
	CONV_16 (dsp_scan.sstate);
	CONV_16 (dsp_scan.pflg);
}

void DSPControl::read_dsp_scan (){
	if (!sranger_hwi_hardware) return; 

	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.scan, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_read  (sranger_hwi_hardware->dsp, &dsp_scan, sizeof (dsp_scan));

	// from DSP to PC
	conv_dsp_scan ();
}

void DSPControl::write_dsp_scan (){
	if (!sranger_hwi_hardware) return; 

	// from PC to DSP
	conv_dsp_scan ();

	// adjust ScanParams/Speed...
	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.scan, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_write  (sranger_hwi_hardware->dsp, &dsp_scan, MAX_WRITE_SCAN<<1);

	// from DSP to PC
	conv_dsp_scan ();
}

void DSPControl::conv_dsp_scan_event_trigger (){
	for (int i=0; i<8; ++i){
		CONV_16 (dsp_scan_event_trigger.trig_i_xp[i]);
		CONV_16 (dsp_scan_event_trigger.trig_i_xm[i]);
		CONV_16 (dsp_scan_event_trigger.xp_bias_setpoint[i]);
		CONV_16 (dsp_scan_event_trigger.xm_bias_setpoint[i]);
	}
	CONV_16 (dsp_scan_event_trigger.pflg);
}

void DSPControl::read_dsp_scan_event_trigger (){
	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.scan_event_trigger, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_read  (sranger_hwi_hardware->dsp, &dsp_scan_event_trigger, sizeof (dsp_scan_event_trigger)); 

	// from DSP to PC
	conv_dsp_scan_event_trigger ();

	for (int i=0; i<8; ++i){
		if (i<4){
			trigger_bias_setpoint_xp[i] = gapp->xsm->Inst->V2BiasV (gapp->xsm->Inst->Dig2VoltOut (dsp_scan_event_trigger.xp_bias_setpoint[i]));
			trigger_bias_setpoint_xm[i] = gapp->xsm->Inst->V2BiasV (gapp->xsm->Inst->Dig2VoltOut (dsp_scan_event_trigger.xm_bias_setpoint[i]));
		} else {
			if (IS_AFM_CTRL){ // AFM (linear)
				if (strncmp (xsmres.daqZunit[0], "nN", 2) == 0)
					;
				else if (strncmp (xsmres.daqZunit[0], "Hz", 2) == 0)
					;
				else
					;
			} else {// STM (log)
				double f = gapp->xsm->Inst->nAmpere2V (1.);
				trigger_bias_setpoint_xp[i] = gapp->xsm->Inst->Dig2VoltIn (dsp_scan_event_trigger.xp_bias_setpoint[i]) / f;
				trigger_bias_setpoint_xm[i] = gapp->xsm->Inst->Dig2VoltIn (dsp_scan_event_trigger.xm_bias_setpoint[i]) / f;
			}
		}
	}
}

void DSPControl::write_dsp_scan_event_trigger (){
	if (!sranger_hwi_hardware) return; 

	for (int i=0; i<8; ++i){
		if (i<4){
			dsp_scan_event_trigger.xp_bias_setpoint[i] = (DSP_INT) round (gapp->xsm->Inst->VoltOut2Dig (gapp->xsm->Inst->BiasV2Vabs (trigger_bias_setpoint_xp[i])));
			dsp_scan_event_trigger.xm_bias_setpoint[i] = (DSP_INT) round (gapp->xsm->Inst->VoltOut2Dig (gapp->xsm->Inst->BiasV2Vabs (trigger_bias_setpoint_xm[i])));
		} else {
			if (IS_AFM_CTRL){ // AFM (linear)
				if (strncmp (xsmres.daqZunit[0], "nN", 2) == 0)
					;
				else if (strncmp (xsmres.daqZunit[0], "Hz", 2) == 0)
					;
				else
					;
			} else {// STM (log)
				dsp_scan_event_trigger.xp_bias_setpoint[i] = (int) (gapp->xsm->Inst->VoltIn2Dig (gapp->xsm->Inst->nAmpere2V (trigger_bias_setpoint_xp[i])));
				dsp_scan_event_trigger.xm_bias_setpoint[i] = (int) (gapp->xsm->Inst->VoltIn2Dig (gapp->xsm->Inst->nAmpere2V (trigger_bias_setpoint_xm[i])));
			}
		}
	}

	// from PC to DSP
	conv_dsp_scan_event_trigger ();

	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.scan_event_trigger, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_write (sranger_hwi_hardware->dsp, &dsp_scan_event_trigger, MAX_WRITE_SCAN_EVENT_TRIGGER<<1); 

	// from DSP to PC
	conv_dsp_scan_event_trigger ();
}

void DSPControl::update(){
	g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (window), "DSP_EC_list"),
			(GFunc) App::update_ec, NULL);
}


double factor(double N, double delta){
	while (fmod (N, delta) != 0.) 
		delta -= 1.;
	return delta;
}

void DSPControl::recalculate_dsp_scan_speed_parameters (AREA_SCAN &dsp_scan_par){
	double frac  = (1<<16);
	double fs_dx = frac * sranger_hwi_pi.app->xsm->Inst->XA2Dig (scan_speed_x_requested) / frq_ref;
	double fs_dy = frac * sranger_hwi_pi.app->xsm->Inst->YA2Dig (scan_speed_x_requested) / frq_ref;
	
 	if ((frac * sranger_hwi_hardware->Dx / fs_dx) > (1<<15) || (frac * sranger_hwi_hardware->Dy / fs_dx) > (1<<15)){
 		std::cout << "Too slow, reaching 1<<15 steps inbetween! -- no change." << std::endl;
 		return;
 	}

	// fs_dx * N =!= frac*Dx  -> N = ceil [frac*Dx/fs_dx]  -> fs_dx' = frac*Dx/N
	
	// N: dnx
	dsp_scan_par.dnx = (DSP_INT) ceil (frac * sranger_hwi_hardware->Dx * dynamic_zoom / fs_dx);
	dsp_scan_par.dny = (DSP_INT) ceil (frac * sranger_hwi_hardware->Dy * dynamic_zoom / fs_dy);
	
	dsp_scan_par.fs_dx = (DSP_LONG) (frac*sranger_hwi_hardware->Dx / ceil (frac * sranger_hwi_hardware->Dx / fs_dx));
	dsp_scan_par.fs_dy = (DSP_LONG) (frac*sranger_hwi_hardware->Dy / ceil (frac * sranger_hwi_hardware->Dy / fs_dy));
	
	dsp_scan_par.nx_pre = dsp_scan_par.dnx * pre_points;
	
	dsp_scan_par.fs_dy *= sranger_hwi_hardware->scan_direction;


	// re-update to real speed
	scan_speed_x = sranger_hwi_pi.app->xsm->Inst->Dig2XA ((DSP_LONG)(dsp_scan_par.fs_dx * frq_ref / frac));

	gchar *info = g_strdup_printf (" (%g)", scan_speed_x);
	scan_speed_ec->set_info (info);
	g_free (info);

#if 1
	std::cout << "** Scan Speed and Step size ReComputation -*- best integer fit results **" << std::endl
		  << "Dx                 = " << sranger_hwi_hardware->Dx << std::endl
		  << "(double)fs_dx      = " << fs_dx << std::endl
		  << "dsp_scan.fs_dx     = " << dsp_scan_par.fs_dx << std::endl
		  << "dsp_scan.dnx       = " << dsp_scan_par.dnx << std::endl
		  << "dsp_scan.nx        = " << dsp_scan_par.nx << std::endl
		  << "dsp_scan.nx_pre    = " << dsp_scan_par.nx_pre << std::endl
		  << "scan_speed_x [A/s] = " << scan_speed_x << std::endl
		  << "real stepwidth [A] = " << sranger_hwi_pi.app->xsm->Inst->Dig2XA ((DSP_LONG)(dsp_scan_par.fs_dx/frac * dsp_scan_par.dnx)) << std::endl
		  << "real scanwidth [A] = " << sranger_hwi_pi.app->xsm->Inst->Dig2XA ((DSP_LONG)(dsp_scan_par.fs_dx/frac * dsp_scan_par.dnx * dsp_scan_par.nx)) << std::endl
		  << "dynamic-zoom fac   = " << dynamic_zoom << std::endl
		;
#endif
	
}

void DSPControl::recalculate_dsp_scan_slope_parameters (AREA_SCAN &dsp_scan_par){
	// setup slope compensation parameters 
	if (area_slope_compensation_flag){
		double zx_ratio = sranger_hwi_pi.app->xsm->Inst->Dig2XA (1) / sranger_hwi_pi.app->xsm->Inst->Dig2ZA (1);
		double zy_ratio = sranger_hwi_pi.app->xsm->Inst->Dig2YA (1) / sranger_hwi_pi.app->xsm->Inst->Dig2ZA (1);
		dsp_scan_par.fm_dzx = (DSP_LONG)round (zx_ratio * (double)dsp_scan_par.fs_dx * area_slope_x);
		dsp_scan_par.fm_dzy = (DSP_LONG)round (zy_ratio * (double)dsp_scan_par.fs_dy * area_slope_y);
		dsp_scan_par.fm_dzxy = (DSP_LONG)round (zx_ratio * (double)dsp_scan_par.fm_dx * area_slope_x
						+ zy_ratio * (double)dsp_scan_par.fm_dy * area_slope_y);
	}else {
		dsp_scan_par.fm_dzx = 0;
		dsp_scan_par.fm_dzy = 0;
		dsp_scan_par.fm_dzxy = 0;
	}
}

void DSPControl::updateDSP(int FbFlg){
	if (!sranger_hwi_hardware) return; 
	PI_DEBUG (DBG_L2, "Hallo SR DSP ! FB=" << FbFlg );

// mirror basic parameters to GXSM main
	gapp->xsm->data.s.Bias = bias;
	gapp->xsm->data.s.SetPoint = voltage_set_point;
	gapp->xsm->data.s.Current = current_set_point;

	switch(FbFlg){
	case DSP_FB_ON: sranger_hwi_hardware->ExecCmd(DSP_CMD_START); break;
	case DSP_FB_OFF: sranger_hwi_hardware->ExecCmd(DSP_CMD_HALT); break;
	}

	if (IS_AFM_CTRL){ // AFM (linear)
		if (strncmp (xsmres.daqZunit[0], "nN", 2) == 0)
			dsp_feedback.setpoint = (int)(gapp->xsm->Inst->VoltIn2Dig (gapp->xsm->Inst->nNewton2V (voltage_set_point)));
		else if (strncmp (xsmres.daqZunit[0], "Hz", 2) == 0)
			dsp_feedback.setpoint = (int)(gapp->xsm->Inst->VoltIn2Dig (gapp->xsm->Inst->dHertz2V (voltage_set_point)));
		else
			dsp_feedback.setpoint = (int)(gapp->xsm->Inst->VoltIn2Dig (voltage_set_point));
	} else {// STM (log)
		dsp_feedback.setpoint = (int)(gapp->xsm->Inst->VoltIn2Dig (gapp->xsm->Inst->nAmpere2V (current_set_point)));
		if (dsp_feedback.setpoint < 1)
			dsp_feedback.setpoint = 1;
	}
	write_dsp_feedback ();


	// do not touch if Auto Trigger is enabled and is Scanning
	if (!(dsp_scan_event_trigger.pflg && dsp_scan.pflg)){ 
		// Analog values (bias only is changed)
		read_dsp_analog ();
		dsp_analog.bias = (int)(gapp->xsm->Inst->VoltOut2Dig (gapp->xsm->Inst->BiasV2Vabs (bias)));
		write_dsp_analog ();
	}

	int addflag=FALSE;
	if (fabs (ue_bias - bias) > 1e-6){
		sranger_hwi_hardware->add_user_event_now ("BiasAdjust", "[V]", ue_bias, bias, addflag);
		ue_bias = bias;
		addflag=TRUE;
	}
	if (fabs (ue_current_set_point - current_set_point) > 1e-6){
		sranger_hwi_hardware->add_user_event_now ("CurrentSetPtAdjust", "[nA]", ue_current_set_point, current_set_point, addflag);
		ue_current_set_point = current_set_point;
		addflag=TRUE;
	}
	if (fabs (ue_voltage_set_point - voltage_set_point) > 1e-6){
		sranger_hwi_hardware->add_user_event_now ("VoltageSetPtAdjust", "[V]", ue_voltage_set_point, voltage_set_point, addflag);
		ue_voltage_set_point = voltage_set_point;
		addflag=TRUE;
	}
	if (fabs (ue_usr_cp - usr_cp) > 1e-6){
		sranger_hwi_hardware->add_user_event_now ("CP adjust", "[1]", ue_usr_cp, usr_cp, addflag);
		ue_usr_cp = usr_cp;
		addflag=TRUE;
	}
	if (fabs (ue_usr_ci - usr_ci) > 1e-6){
		sranger_hwi_hardware->add_user_event_now ("CI adjust", "[1]", ue_usr_ci, usr_ci, addflag);
		ue_usr_ci = usr_ci;
		addflag=TRUE;
	}


	// Scan characteristics update while scanning
	read_dsp_scan ();
	if (dsp_scan.pflg && 
	    (fabs (ue_scan_speed_x_r - scan_speed_x_requested) 
	     + fabs(ue_slope_x-area_slope_x) 
	     + fabs(ue_slope_y-area_slope_y) 
	     + fabs((double)(ue_slope_flg-area_slope_compensation_flag))) > 1e-6){
		// compute 32bit stepsize and num steps inbetween from scan_speed_x and Dx, Dy
		// sranger_hwi_pi.app->xsm->Inst->XA2Dig, Dig2XA

		if (fabs (ue_scan_speed_x_r - scan_speed_x_requested) > 1.){
			recalculate_dsp_scan_speed_parameters (dsp_scan);

			sranger_hwi_hardware->add_user_event_now ("SpeedAdjust", " [A/s]", ue_scan_speed_x, scan_speed_x, addflag); 
//			sranger_hwi_hardware->add_user_event_now ("SpeedAdjust from %g A/s to %g A/s, real step: %g A, real X-width: %g A [fs_dx:%d, dnx:%d]"), ue_scan_speed_x, scan_speed_x, 
//							     sranger_hwi_pi.app->xsm->Inst->Dig2XA ((DSP_LONG)(dsp_scan.fs_dx/(double)(1<<16) * dsp_scan.dnx)),
//							     sranger_hwi_pi.app->xsm->Inst->Dig2XA ((DSP_LONG)(dsp_scan.fs_dx/(double)(1<<16) * dsp_scan.dnx * dsp_scan.nx)),
//							     dsp_scan.fs_dx, dsp_scan.dnx
//						    ), addflag);
			ue_scan_speed_x_r = scan_speed_x_requested;
			ue_scan_speed_x   = scan_speed_x;
			addflag=TRUE;
		}

		if ((fabs(ue_slope_x-area_slope_x) + fabs(ue_slope_y-area_slope_y) + fabs((double)(ue_slope_flg-area_slope_compensation_flag))) > 1e-6)
			recalculate_dsp_scan_slope_parameters (dsp_scan);

		write_dsp_scan ();

		if (area_slope_compensation_flag && fabs(ue_slope_x-area_slope_x) > 1e-6){
			sranger_hwi_hardware->add_user_event_now ("SlopeX_Adj", "[1]", ue_slope_x, area_slope_x, addflag);
			addflag=TRUE;
		}

		if (area_slope_compensation_flag && fabs(ue_slope_y-area_slope_y) > 1e-6){
			sranger_hwi_hardware->add_user_event_now ("SlopeY_Adj", ue_slope_y, area_slope_y, addflag);
			addflag=TRUE;
		}
		
		if (area_slope_compensation_flag && !ue_slope_flg){
			sranger_hwi_hardware->add_user_event_now ("SlopeCompensation_Adj", "[bool]",  0.,1., addflag);
			addflag=TRUE;
		}
		
		if (!area_slope_compensation_flag && ue_slope_flg){
			sranger_hwi_hardware->add_user_event_now ("SlopeCompensation_Adj", "[bool]", 1.,0., addflag);
			addflag=TRUE;
		}

		ue_slope_x = area_slope_x;
		ue_slope_y = area_slope_y;
		ue_slope_flg = area_slope_compensation_flag;
	}
}

void DSPControl::update_trigger(gboolean flg){
	if (flg){
		dsp_scan_event_trigger.pflg=1;
		g_slist_foreach (FreezeEntryList, (GFunc) App::freeze_ec, NULL);
	} else {
		dsp_scan_event_trigger.pflg=0;
		g_slist_foreach (FreezeEntryList, (GFunc) App::thaw_ec, NULL);
	}	
	write_dsp_scan_event_trigger ();
}


int DSPControl::ChangedAction(GtkWidget *widget, DSPControl *dspc){
	dspc->updateDSP();
	return 0;
}

void DSPControl::ChangedNotify(Param_Control* pcs, gpointer dspc){
	//  gchar *us=pcs->Get_UsrString();
	//  PI_DEBUG (DBG_L2, "DSPC: " << us );
	//  g_free(us);
	((DSPControl*)dspc)->updateDSP();
}

int DSPControl::feedback_callback( GtkWidget *widget, DSPControl *dspc){
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->updateDSP(DSP_FB_ON);
	else
		dspc->updateDSP(DSP_FB_OFF);
	return 0;
}

int DSPControl::se_auto_trigger_callback(GtkWidget *widget, DSPControl *dspc){
	dspc->update_trigger (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
	return 0;
}


// LockIn write (optional)

int DSPControl::LockIn_read_callback( GtkWidget *widget, DSPControl *dspc){
	dspc->read_dsp_probe ();
}

int DSPControl::LockIn_write_callback( GtkWidget *widget, DSPControl *dspc){
	dspc->write_dsp_probe ();
}

// PV write
// Note: Exec and/or Write always includes LockIn and PV data write to DSP!

int DSPControl::LockIn_exec_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_NONE);

	dspc->current_auto_flags = dspc->AC_auto_flags;
	dspc->probe_trigger_single_shot = 1;
	dspc->write_dsp_probe (1, PV_MODE_AC); // Exec AC-phase probing here
	sranger_hwi_hardware->start_fifo_read (0, 0,0,0,0, NULL,NULL,NULL,NULL);
}

int DSPControl::Probing_exec_IV_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_NONE);

	dspc->current_auto_flags = dspc->IV_auto_flags;
	dspc->probe_trigger_single_shot = 1;
	dspc->write_dsp_probe (1, PV_MODE_IV); // Exec STS probing here
	sranger_hwi_hardware->start_fifo_read (0, 0,0,0,0, NULL,NULL,NULL,NULL);
}

int DSPControl::Probing_write_IV_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_IV);
}

int DSPControl::Probing_exec_FZ_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_NONE);

	dspc->current_auto_flags = dspc->FZ_auto_flags;
	dspc->probe_trigger_single_shot = 1;
	dspc->write_dsp_probe (1, PV_MODE_FZ); // Exec FZ probing here
	sranger_hwi_hardware->start_fifo_read (0, 0,0,0,0, NULL,NULL,NULL,NULL);
}

int DSPControl::Probing_write_FZ_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_FZ);
}

// PL

int DSPControl::Probing_exec_PL_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_NONE);

	dspc->current_auto_flags = dspc->PL_auto_flags;
	dspc->probe_trigger_single_shot = 1;
	dspc->write_dsp_probe (1, PV_MODE_PL); // Exec FZ probing here
	sranger_hwi_hardware->start_fifo_read (0, 0,0,0,0, NULL,NULL,NULL,NULL);
}

int DSPControl::Probing_write_PL_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_PL);
}

// LP

int DSPControl::Probing_exec_LP_callback( GtkWidget *widget, DSPControl *dspc){
	dspc->write_dsp_probe (0, PV_MODE_NONE);
	
	dspc->current_auto_flags = dspc->LP_auto_flags;
	dspc->probe_trigger_single_shot = 1;
	dspc->write_dsp_probe (1, PV_MODE_LP); // Exec FZ probing here
	sranger_hwi_hardware->start_fifo_read (0, 0,0,0,0, NULL,NULL,NULL,NULL);
}

int DSPControl::Probing_write_LP_callback( GtkWidget *widget, DSPControl *dspc){
	dspc->write_dsp_probe (0, PV_MODE_LP);
}

// SP

int DSPControl::Probing_exec_SP_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_NONE);

	dspc->current_auto_flags = dspc->SP_auto_flags;
	dspc->probe_trigger_single_shot = 1;
	dspc->write_dsp_probe (1, PV_MODE_SP); // Exec FZ probing here
	sranger_hwi_hardware->start_fifo_read (0, 0,0,0,0, NULL,NULL,NULL,NULL);
}

int DSPControl::Probing_write_SP_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_SP);
}

// TS

int DSPControl::Probing_exec_TS_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_NONE);

	dspc->current_auto_flags = dspc->TS_auto_flags;
	dspc->probe_trigger_single_shot = 1;
	dspc->write_dsp_probe (1, PV_MODE_TS); // Exec FZ probing here
	sranger_hwi_hardware->start_fifo_read (0, 0,0,0,0, NULL,NULL,NULL,NULL);
}

int DSPControl::Probing_write_TS_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_TS);
}

// LM

int DSPControl::Probing_exec_LM_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_NONE);

	dspc->current_auto_flags = dspc->LM_auto_flags;

	dspc->probe_trigger_single_shot = 1;
	dspc->write_dsp_probe (1, PV_MODE_LM); // Exec FZ probing here
	sranger_hwi_hardware->start_fifo_read (0, 0,0,0,0, NULL,NULL,NULL,NULL);
}

int DSPControl::Probing_write_LM_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_LM);
}


// AX

int DSPControl::Probing_exec_AX_callback( GtkWidget *widget, DSPControl *dspc){
        dspc->write_dsp_probe (0, PV_MODE_NONE);

	dspc->current_auto_flags = dspc->AX_auto_flags;

	dspc->probe_trigger_single_shot = 1;
	dspc->write_dsp_probe (1, PV_MODE_AX); // Exec AX probing here
	sranger_hwi_hardware->start_fifo_read (0, 0,0,0,0, NULL,NULL,NULL,NULL);
}

int DSPControl::Probing_write_AX_callback( GtkWidget *widget, DSPControl *dspc){
	// setup counter
	dspc->dsp_cr_generic_io.start = CONV_16_C (0);           /* Initiate =WO -- no action, only set mode and gatetime now! */
	dspc->dsp_cr_generic_io.stop  = CONV_16_C (0);           /* Cancel   =WO */
	dspc->dsp_cr_generic_io.port  = CONV_16_C (0);           /* --- init w zero --- */
	dspc->dsp_cr_generic_io.rw    = CONV_16_C (0);           /* --- init w zero --- */
	dspc->dsp_cr_generic_io.data_lo  = CONV_16_C (0);        /* --- init w zero --- */
	dspc->dsp_cr_generic_io.data_hi  = CONV_16_C (0);        /* --- init w zero --- */


	if (dspc->AX_gatetime < 3e-3){
		dspc->dsp_cr_generic_io.mode     = CONV_16_C (3);    /* Gate Timer Mode with reset (FF=3 (100ns), Fast=5, Slow=9) */
		dspc->dsp_cr_generic_io.gatetime = CONV_16_C ((int)(round(dspc->AX_gatetime/CPLD_GATEREF)));     /* Timer, downcounter for gatetime */
	} else if (dspc->AX_gatetime < 1.5){
		dspc->dsp_cr_generic_io.mode     = CONV_16_C (5);    /* Gate Timer Mode with reset (FF=3 (100ns), Fast=5, Slow=9) */
		dspc->dsp_cr_generic_io.gatetime = CONV_16_C ((int)(round(dspc->AX_gatetime/CPLD_GATEREF/500.)));     /* Timer, downcounter for gatetime */
	} else {
		dspc->dsp_cr_generic_io.mode     = CONV_16_C (9);    /* Gate Timer Mode with reset (FF=3 (100ns), Fast=5, Slow=9) */
		dspc->dsp_cr_generic_io.gatetime = CONV_16_C ((int)(round(dspc->AX_gatetime/CPLD_GATEREF/50000.)));     /* Timer, downcounter for gatetime */
	}

// TESTing version
//	dspc->dsp_cr_generic_io.mode     = CONV_16_C ((int)dspc->AX_resolution);   /* Gate Timer Mode with reset (FF=3 (100ns), Fast=5, Slow=9) */
//	dspc->dsp_cr_generic_io.gatetime = CONV_16_C ((int)dspc->AX_gatetime);     /* Timer, downcounter for gatetime */

	lseek (sranger_hwi_hardware->dsp, sranger_hwi_hardware->magic_data.CR_generic_io, SRANGER_SEEK_DATA_SPACE);
	sranger_hwi_hardware->sr_write (sranger_hwi_hardware->dsp, &dspc->dsp_cr_generic_io,  MAX_WRITE_CR_GENERIC_IO<<1); 

	// setup vector table
        dspc->write_dsp_probe (0, PV_MODE_AX);
}


// for TESTING and DEBUGGIG only
int DSPControl::Probing_exec_RF_callback( GtkWidget *widget, DSPControl *dspc){
	dspc->probe_trigger_single_shot = 1;
	sranger_hwi_hardware->start_fifo_read (0, 0,0,0,0, NULL,NULL,NULL,NULL);
}


void DSPControl::StartScanPreCheck (){
	dynamic_zoom = 1.;
	update ();

	if (write_vector_mode == PV_MODE_NONE)
		probe_trigger_raster_points = 0;
	else{
		probe_trigger_raster_points = probe_trigger_raster_points_user;
		raster_auto_flags = current_auto_flags;
		write_dsp_probe (0, write_vector_mode);
	}
}

int DSPControl::auto_probe_callback(GtkWidget *widget, DSPControl *dspc){
	dspc->write_vector_mode = (pv_mode) atoi (gtk_combo_box_get_active_id (GTK_COMBO_BOX (widget)));
	Gtk_EntryControl* ri = (Gtk_EntryControl*) g_object_get_data( G_OBJECT (widget), "raster_ec");
	if (!ri) return 0;

	if (dspc->write_vector_mode == PV_MODE_NONE)
		ri->Freeze ();
	else
		ri->Thaw ();

	return 0;	      
}


int DSPControl::choice_Ampl_callback (GtkWidget *widget, DSPControl *dspc){
	gint i = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
	gint j = GPOINTER_TO_INT (g_object_get_data( G_OBJECT (widget), "chindex"));
	switch(j){
	case 0: sranger_hwi_pi.app->xsm->Inst->VX(i);
		if (gapp->xsm->Inst->OffsetMode() == OFM_DSP_OFFSET_ADDING)
			sranger_hwi_pi.app->xsm->Inst->VX0(i);
		break;
	case 1: sranger_hwi_pi.app->xsm->Inst->VY(i);
		if (gapp->xsm->Inst->OffsetMode() == OFM_DSP_OFFSET_ADDING)
			sranger_hwi_pi.app->xsm->Inst->VY0(i);
		break;
	case 2: sranger_hwi_pi.app->xsm->Inst->VZ(i); 
		if (gapp->xsm->Inst->OffsetMode() == OFM_DSP_OFFSET_ADDING)
			sranger_hwi_pi.app->xsm->Inst->VZ0(i); 
		break;
	case 3:
		if (gapp->xsm->Inst->OffsetMode() == OFM_ANALOG_OFFSET_ADDING)
			sranger_hwi_pi.app->xsm->Inst->VX0(i);
		break;
	case 4:
		if (gapp->xsm->Inst->OffsetMode() == OFM_ANALOG_OFFSET_ADDING)
			sranger_hwi_pi.app->xsm->Inst->VY0(i);
		break;
	case 5:
		if (gapp->xsm->Inst->OffsetMode() == OFM_ANALOG_OFFSET_ADDING)
			sranger_hwi_pi.app->xsm->Inst->VZ0(i);
		break;
	}
	PI_DEBUG (DBG_L2, "Ampl: CH=" << j << " i=" << i );
	sranger_hwi_pi.app->spm_range_check(NULL, sranger_hwi_pi.app);
	dspc->updateDSP();
	return 0;
}

int DSPControl::callback_change_AC_option_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = (DSP_LONG) GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->AC_option_flags = (dspc->AC_option_flags & (~msk)) | msk;
	else
		dspc->AC_option_flags &= ~msk;

}
int DSPControl::callback_change_AC_auto_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = (DSP_LONG) GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->AC_auto_flags = (dspc->AC_auto_flags & (~msk)) | msk;
	else
		dspc->AC_auto_flags &= ~msk;

	if (dspc->write_vector_mode == PV_MODE_AC)
		dspc->raster_auto_flags = dspc->AC_auto_flags;

}

int DSPControl::callback_change_IV_option_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = (DSP_LONG) GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->IV_option_flags = (dspc->IV_option_flags & (~msk)) | msk;
	else
		dspc->IV_option_flags &= ~msk;

}
int DSPControl::callback_change_IV_auto_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = (DSP_LONG) GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->IV_auto_flags = (dspc->IV_auto_flags & (~msk)) | msk;
	else
		dspc->IV_auto_flags &= ~msk;

	if (dspc->write_vector_mode == PV_MODE_IV)
		dspc->raster_auto_flags = dspc->IV_auto_flags;

}

int DSPControl::callback_change_FZ_option_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = (DSP_LONG) GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->FZ_option_flags = (dspc->FZ_option_flags & (~msk)) | msk;
	else
		dspc->FZ_option_flags &= ~msk;

}
int DSPControl::callback_change_FZ_auto_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = (DSP_LONG) GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->FZ_auto_flags = (dspc->FZ_auto_flags & (~msk)) | msk;
	else
		dspc->FZ_auto_flags &= ~msk;

	if (dspc->write_vector_mode == PV_MODE_FZ)
		dspc->raster_auto_flags = dspc->FZ_auto_flags;
}

int DSPControl::callback_change_PL_option_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = (DSP_LONG) GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->PL_option_flags = (dspc->PL_option_flags & (~msk)) | msk;
	else
		dspc->PL_option_flags &= ~msk;

	if (dspc->write_vector_mode == PV_MODE_PL)
		dspc->raster_auto_flags = dspc->PL_auto_flags;
}

int DSPControl::callback_change_PL_auto_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->PL_auto_flags = (dspc->PL_auto_flags & (~msk)) | msk;
	else
		dspc->PL_auto_flags &= ~msk;

}

int DSPControl::callback_change_LP_option_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = (DSP_LONG) GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->LP_option_flags = (dspc->LP_option_flags & (~msk)) | msk;
	else
		dspc->LP_option_flags &= ~msk;
 
	if (dspc->write_vector_mode == PV_MODE_LP)
		dspc->raster_auto_flags = dspc->LP_auto_flags;
}
 
int DSPControl::callback_change_LP_auto_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->LP_auto_flags = (dspc->LP_auto_flags & (~msk)) | msk;
	else
		dspc->LP_auto_flags &= ~msk;
 
}

int DSPControl::callback_change_SP_option_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = (DSP_LONG) GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->SP_option_flags = (dspc->SP_option_flags & (~msk)) | msk;
	else
		dspc->SP_option_flags &= ~msk;

	if (dspc->write_vector_mode == PV_MODE_SP)
		dspc->raster_auto_flags = dspc->SP_auto_flags;
}

int DSPControl::callback_change_SP_auto_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->SP_auto_flags = (dspc->SP_auto_flags & (~msk)) | msk;
	else
		dspc->SP_auto_flags &= ~msk;

}

int DSPControl::callback_change_TS_option_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = (DSP_LONG) GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->TS_option_flags = (dspc->TS_option_flags & (~msk)) | msk;
	else
		dspc->TS_option_flags &= ~msk;

	if (dspc->write_vector_mode == PV_MODE_TS)
		dspc->raster_auto_flags = dspc->TS_auto_flags;
}

int DSPControl::callback_change_TS_auto_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->TS_auto_flags = (dspc->TS_auto_flags & (~msk)) | msk;
	else
		dspc->TS_auto_flags &= ~msk;

}

int DSPControl::callback_change_LM_option_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->LM_option_flags = (dspc->LM_option_flags & (~msk)) | msk;
	else
		dspc->LM_option_flags &= ~msk;

	if (dspc->write_vector_mode == PV_MODE_LM)
		dspc->raster_auto_flags = dspc->LM_auto_flags;
}

int DSPControl::callback_change_LM_auto_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->LM_auto_flags = (dspc->LM_auto_flags & (~msk)) | msk;
	else
		dspc->LM_auto_flags &= ~msk;

}

int DSPControl::callback_change_AX_option_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->AX_option_flags = (dspc->LM_option_flags & (~msk)) | msk;
	else
		dspc->AX_option_flags &= ~msk;

	if (dspc->write_vector_mode == PV_MODE_AX)
		dspc->raster_auto_flags = dspc->AX_auto_flags;
}

int DSPControl::callback_change_AX_auto_flags (GtkWidget *widget, DSPControl *dspc){
	long msk = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Bit_Mask"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		dspc->AX_auto_flags = (dspc->AX_auto_flags & (~msk)) | msk;
	else
		dspc->AX_auto_flags &= ~msk;

}

int DSPControl::DSP_expert_callback (GtkWidget *widget, DSPControl *dspc){
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (widget), "DSP_expert_list"),
				(GFunc) gtk_widget_show, NULL);
	else
		g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (widget), "DSP_expert_list"),
				(GFunc) gtk_widget_hide, NULL);

	dspc->expert_mode = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

int DSPControl::DSP_guru_callback (GtkWidget *widget, DSPControl *dspc){
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (widget), "DSP_guru_list"),
				(GFunc) gtk_widget_show, NULL);
	else
		g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (widget), "DSP_guru_list"),
				(GFunc) gtk_widget_hide, NULL);

	dspc->guru_mode = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

int DSPControl::DSP_slope_callback (GtkWidget *widget, DSPControl *dspc){
	dspc->area_slope_compensation_flag = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));	
	dspc->updateDSP();
}

int DSPControl::DSP_cret_callback (GtkWidget *widget, DSPControl *dspc){
	dspc->center_return_flag = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));	
}

int DSPControl::change_source_callback (GtkWidget *widget, DSPControl *dspc){
	long channel;
	channel = (long) GPOINTER_TO_INT (g_object_get_data(G_OBJECT(widget), "Source_Channel"));
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
		if (channel & X_SOURCE_MSK)
			dspc->XSource |= channel;
		else if (channel & P_SOURCE_MSK)
			dspc->PSource |= channel;
		else if (channel & A_SOURCE_MSK)
			dspc->PlotAvg |= channel;
		else if (channel & S_SOURCE_MSK)
			dspc->PlotSec |= channel;
		else
			dspc->Source |= channel;
		
	}
	else {
		if (channel & X_SOURCE_MSK)
			dspc->XSource &= ~channel;
		else if (channel & P_SOURCE_MSK)
			dspc->PSource &= ~channel;
		else if (channel & A_SOURCE_MSK)
			dspc->PlotAvg &= ~channel;
		else if (channel & S_SOURCE_MSK)
			dspc->PlotSec &= ~channel;
		else
			dspc->Source &= ~channel;
	}
	return 0;
}

