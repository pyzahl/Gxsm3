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


#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include "gxsm_app.h"
#include "gxsm_window.h"

#include "unit.h"
#include "pcs.h"
#include "xsmtypes.h"
#include "action_id.h"
#include "glbvars.h"

#include "gtk/gtk.h"
#include <cairo.h>

#include "app_profile.h"
#include "app_vobj.h"
#include "app_vinfo.h"
#include "app_view.h"

#include <sstream>
using namespace std;

#include <netcdf.hh>
//#include <netcdf>
//using namespace netCDF;


#define UTF8_ANGSTROEM "\303\205"
#define OUT_OF_RANGE N_("Value out of range!")

#define VIEW_PREFIX "AppView_"

// #define	XSM_DEBUG(X,D) std::cout << D << std::endl;


#define APP_SELECTOR(CB, LABEL)                                         \
        do{                                                             \
                if (strlen (LABEL)){                                    \
                        label = gtk_label_new (N_(LABEL));              \
                        gtk_grid_attach (GTK_GRID (grid), label, x++, y, 1,1); \
                }                                                       \
		CB = gtk_combo_box_text_new ();                         \
                gtk_grid_attach (GTK_GRID (grid), CB, x++, y, 1,1);     \
	}while(0)

#define ADD_TOGGLE(TOG, LABEL, DEFAULT)			\
        do {\
                   TOG = gtk_check_button_new_with_label(N_(LABEL)); \
		   gtk_widget_show (TOG);				\
                   gtk_grid_attach (GTK_GRID (grid), TOG, x++, y, 1,1); \
                   gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (TOG), (DEFAULT)?1:0); \
        }while(0)

//                   g_signal_connect (TOG, "toggled",G_CALLBACK(CB_FUNC), this);
// obsolete:
//                gtk_style_context_get_background_color (gtk_widget_get_style_context (GTK_WIDGET (VAR)), GTK_STATE_FLAG_INSENSITIVE, &color); 
//                gtk_render_background (gtk_widget_get_style_context (GTK_WIDGET (VAR)), GTK_STATE_FLAG_INSENSITIVE, &color); 

/* in SETUP_ENTRY
                GdkRGBA color; 
                gtk_style_context_get_background_color (gtk_widget_get_style_context (GTK_WIDGET (VAR)), GTK_STATE_FLAG_INSENSITIVE, &color); 
		gtk_widget_override_background_color (GTK_WIDGET (VAR), GTK_STATE_FLAG_NORMAL, &color); 
*/
#define SETUP_ENTRY(VAR, TXT)			\
	do {\
		gtk_entry_set_text (GTK_ENTRY (VAR), TXT); \
		gtk_editable_set_editable (GTK_EDITABLE (VAR), FALSE); \
		gtk_widget_set_sensitive (VAR, TRUE); \
		gtk_widget_show (VAR); \
	}while(0)


#define SETUP_LABEL(LAB) \
	do {\
		gtk_widget_set_size_request (LAB, 150, -1); \
		gtk_widget_show (LAB); \
	}while(0)


// ============================================================
// Popup Menu and Object Action Map
// ============================================================

static GActionEntry win_view_popup_entries[] = {
        // { "preferences", preferences_activated, NULL, NULL, NULL },
        // { "quit", quit_activated, NULL, NULL, NULL },
        { "view-activate", ViewControl::Activate_callback, NULL, NULL, NULL },
        { "view-autodisp", ViewControl::AutoDisp_callback, NULL, NULL, NULL },
        { "view-set-off", ViewControl::SetOff_callback, NULL, NULL, NULL },
        { "view-set-active", ViewControl::Activate_callback, NULL, NULL, NULL },
        { "view-set-on", ViewControl::SetOn_callback, NULL, NULL, NULL },
        { "view-set-math", ViewControl::SetMath_callback, NULL, NULL, NULL },
        { "view-set-x", ViewControl::SetX_callback, NULL, NULL, NULL },
        { "view-open", ViewControl::view_file_openhere_callback, NULL, NULL, NULL },
        { "view-save-auto", ViewControl::view_file_save_callback, NULL, NULL, NULL },
        { "view-save-as", ViewControl::view_file_save_as_callback, NULL, NULL, NULL },
        { "view-save-as-image", ViewControl::view_file_saveimage_callback, NULL, NULL, NULL },
        { "view-save-objects", ViewControl::view_file_saveobjects_callback, NULL, NULL, NULL },
        { "view-load-objecs", ViewControl::view_file_loadobjects_callback, NULL, NULL, NULL },
        { "view-get-info", ViewControl::view_file_getinfo_callback, NULL, NULL, NULL },
        { "view-print", ViewControl::view_file_print_callback, NULL, NULL, NULL },
        { "view-close", ViewControl::view_file_kill_callback, NULL, NULL, NULL },
        { "copy-scan-to-new-channel", ViewControl::view_edit_copy_callback, NULL, NULL, NULL },
        { "crop-rect-to-new-channel", ViewControl::view_edit_crop_callback, NULL, NULL, NULL },
        { "zoom-into-rect", ViewControl::view_edit_zoomin_callback, NULL, NULL, NULL },
        { "zoom-out-of-rect", ViewControl::view_edit_zoomout_callback, NULL, NULL, NULL },
        { "side-pane", ViewControl::side_pane_action_callback, NULL, "true", NULL },
        { "view-mode", ViewControl::view_view_set_view_mode_radio_callback, "s", "'quick'", NULL },
        { "x-linearize", ViewControl::view_view_x_linearize_callback, NULL, "false", NULL },
        { "show-redline", ViewControl::view_view_redline_callback, NULL, "false", NULL },
        { "show-blueline", ViewControl::view_view_blueline_callback, NULL, "false", NULL },
        { "autozoom-mode", ViewControl::view_view_autozoom_callback, NULL, "false", NULL },
        { "palette-mode", ViewControl::view_view_color_callback, NULL, "false", NULL },
        { "rgb-mode", ViewControl::view_view_color_rgb_callback, NULL, "false", NULL },
        { "coordinate-mode", ViewControl::view_view_coordinate_mode_radio_callback, "s", "'absolute'", NULL },
        { "zoom-in", ViewControl::view_view_zoom_out_callback, NULL, NULL, NULL },
        { "zoom-out", ViewControl::view_view_zoom_in_callback, NULL, NULL, NULL },
        { "fix-zoom", ViewControl::view_view_zoom_fix_radio_callback, "s", "'zoomfactor-auto'", NULL },
        { "object-mode", ViewControl::view_object_mode_radio_callback, "s", "'rectangle'", NULL },
        { "show-object-lables", ViewControl::view_tool_labels_callback, NULL, "false", NULL },
        { "reset-object-counter", ViewControl::obj_reset_counter_callback, NULL, NULL, NULL },
        { "show-object-counter", ViewControl::obj_show_counter_callback, NULL, NULL, NULL },
        { "set-marker-group", ViewControl::view_tool_marker_group_radio_callback, "s", "'red'", NULL },
        { "set-local-radius", ViewControl::view_tool_mvprop_radius_radio_callback, "s", "'10'", NULL },
        { "move-all-objects-loc-max", ViewControl::view_tool_all2locmax_callback, NULL, NULL, NULL },
        { "remove-all-objects", ViewControl::view_tool_removeall_callback, NULL, NULL, NULL },
        { "show-probe-events", ViewControl::events_probe_callback, NULL, NULL, NULL },
        { "show-user-events", ViewControl::events_user_callback, NULL, NULL, NULL },
        { "show-event-lables", ViewControl::events_labels_callback, NULL, NULL, NULL },
        { "events-verbose", ViewControl::events_verbose_callback, NULL, NULL, NULL },
        { "remove-all-events", ViewControl::events_remove_callback, NULL, NULL, NULL },
        { "remove-all-trails", ViewControl::indicators_remove_callback, NULL, NULL, NULL }
};

static GActionEntry win_object_popup_entries[] = {
        { "remove-point-object", ViewControl::obj_remove_callback, NULL, NULL, NULL },
        { "open-object-properties-dialog", ViewControl::obj_properties_callback, NULL, NULL, NULL },
        { "get-coordinates-offset", ViewControl::obj_setoffset_callback, NULL, NULL, NULL },
        { "get-coordinates-as-global-reference", ViewControl::obj_global_ref_point_callback, NULL, NULL, NULL },
        { "follow", ViewControl::obj_follow_callback, NULL, "false", NULL },
        { "goto-local-max", ViewControl::obj_go_locmax_callback, NULL, NULL, NULL },
        { "dump-object-properties", ViewControl::obj_dump_callback, NULL, NULL, NULL },
        { "remove-line-object", ViewControl::obj_remove_callback, NULL, NULL, NULL },
        { "object-properties", ViewControl::obj_properties_callback, NULL, NULL, NULL },
        { "get-coordinates", ViewControl::obj_getcoords_callback, NULL, NULL, NULL },
        { "look-for-maximum-goto", ViewControl::obj_go_locmax_callback, NULL, NULL, NULL },
        { "objmode-polyline-node", ViewControl::obj_addnode_callback, NULL, NULL, NULL },
        { "dump-object-properties", ViewControl::obj_dump_callback, NULL, NULL, NULL },
        { "remove-line-object", ViewControl::obj_remove_callback, NULL, NULL, NULL },
        { "object-properties", ViewControl::obj_properties_callback, NULL, NULL, NULL },
        { "add-polyline-node", ViewControl::obj_addnode_callback, NULL, NULL, NULL },
        { "delete-polyline-node", ViewControl::obj_delnode_callback, NULL, NULL, NULL },
        { "dump-object-properties", ViewControl::obj_dump_callback, NULL, NULL, NULL },
        { "use-event-data", ViewControl::obj_event_use_callback, NULL, NULL, NULL },
        { "open-event", ViewControl::obj_event_open_callback, NULL, NULL, NULL },
        { "save-event", ViewControl::obj_event_save_callback, NULL, NULL, NULL },
        { "object-properties", ViewControl::obj_properties_callback, NULL, NULL, NULL },
        { "get-coordinates-offset", ViewControl::obj_setoffset_callback, NULL, NULL, NULL }
};


VObject *current_vobject = NULL;

class NcDumpToWidget : public NcFile{
public:
	NcDumpToWidget (const char* path, NcFile::FileMode mode = ReadOnly) 
		: NcFile(path, mode) { 
		maxvals = 10; 
	} ;
	~NcDumpToWidget (){ };
	static void show_info_callback (GtkWidget *widget, gchar *message){
                GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
                GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (widget)), // parent window
                                                            flags,
                                                            GTK_MESSAGE_INFO,
                                                            GTK_BUTTONS_CLOSE,
                                                            "%s",
                                                            message
                                                            );
                gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy (dialog);

                // GTK3QQQ OK?
                // gtk_widget_show (gnome_message_box_new (message, GNOME_MESSAGE_BOX_INFO, GNOME_STOCK_BUTTON_OK, NULL));
	};
	void cleanup (GtkWidget *box){
		g_slist_foreach (
                                 (GSList*) g_object_get_data (
                                                              G_OBJECT (box), "info_list" ), 
                                 (GFunc) free_info_elem, 
                                 NULL
                                 );
		g_list_foreach (
                                gtk_container_get_children ( GTK_CONTAINER (box)),
                                (GFunc) gtk_widget_destroy, 
                                NULL
			);
	};
	static void varshow_toggel_cb (GtkWidget *widget, gchar *varname){
		XsmRescourceManager xrm("App_View_NCraw");
                // gsettings!!!
                // ==> <key name="view-nc-raw" type="b">
		// xrm.PutBool (varname, gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (widget)));
	};
	void setup_toggle (GtkWidget *tog, gchar *varname){
                // gsettings!!!
                // ==> <key name="view-nc-raw" type="b">
                // GTK3QQQ GSettings!!!!!!!! Tuple List ???
                // XsmRescourceManager xrm("App_View_NCraw");
                // gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tog), xrm.GetBool (varname, FALSE) ? 1:0);
		g_signal_connect (tog, "toggled", G_CALLBACK(NcDumpToWidget::varshow_toggel_cb), g_strdup(varname));
		gtk_widget_set_size_request (tog, 150, -1);
		gtk_widget_show (tog);
	};

	static void free_info_elem(gpointer txt, gpointer data){ g_free((gchar*) txt); };

	void dump ( GtkWidget *box, GtkWidget *box_selected );

	int maxvals;
};


// General NC Formatting Dumpingutil, Output into GTK-Window
void NcDumpToWidget::dump ( GtkWidget *box, GtkWidget *box_selected ){
	GtkWidget *sep;
	GtkWidget *lab;
	GtkWidget *grid, *grid_selected;
	GtkWidget *VarName, *VarName_i;
	GtkWidget *variable, *variable_i;
	GtkWidget *info;
        int grid_row=0;
        int grid_row_s=0;
        
	// cleanup old contents if exists
        cleanup (box_selected);
        cleanup (box);

	grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (box), grid);

	grid_selected = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (box_selected), grid_selected);

	gtk_grid_attach (GTK_GRID (grid), lab=gtk_label_new("Selected NetCDF values"), 0, grid_row++, 10, 1);
	gtk_grid_attach (GTK_GRID (grid), sep=gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, grid_row++, 10, 1);
    
	// ===============================================================================
	// DUMP:  global attributes
	// ===============================================================================
	gtk_grid_attach (GTK_GRID (grid), lab=gtk_label_new("Global Attributes"), 0, grid_row++, 10, 1);
	gtk_grid_attach (GTK_GRID (grid), sep=gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, grid_row++, 10, 1);

	NcAtt *ap;
	for(int n = 0; (ap = get_att(n)); n++) {

		VarName = gtk_label_new (ap->name());
		SETUP_LABEL (VarName);
                gtk_grid_attach (GTK_GRID (grid), VarName, 0, grid_row, 2, 1);
      
		variable = gtk_entry_new ();
		NcValues* vals;
		SETUP_ENTRY(variable, (vals = ap->values())->as_string(0));
                gtk_grid_attach (GTK_GRID (grid), variable, 2, grid_row++, 1, 1);

		delete vals;
		delete ap;
	}

	GSList *infolist=NULL;
//	static gchar *types[] = {"","byte","char","short","long","float","double"};
	NcVar *vp;

	// ===============================================================================

	gtk_grid_attach (GTK_GRID (grid), sep=gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, grid_row++, 10, 1);
	gtk_grid_attach (GTK_GRID (grid), sep=gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, grid_row++, 10, 1);

	// ===============================================================================
	// DUMP:  dimension value
	// ===============================================================================
  
	gtk_grid_attach (GTK_GRID (grid), lab=gtk_label_new("NC Dimensions"), 0, grid_row++, 10, 1);
	gtk_grid_attach (GTK_GRID (grid), sep=gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, grid_row++, 10, 1);

	for (int n=0; n < num_dims(); n++) {

		NcDim* dim = get_dim(n);
		gchar *dimname = g_strconcat("Dim_",(gchar*)dim->name(), NULL);

		VarName = gtk_check_button_new_with_label (dimname);
		setup_toggle (VarName, dimname);
                gtk_grid_attach (GTK_GRID (grid), VarName, 0, grid_row, 2, 1);
      
		variable = gtk_entry_new ();
		gchar *dimval;
		if (dim->is_unlimited())
			dimval = g_strdup_printf("UNLIMITED, %d currently", (int)dim->size());
		else
			dimval = g_strdup_printf("%d", (int)dim->size());
    
		SETUP_ENTRY(variable, dimval);
                gtk_grid_attach (GTK_GRID (grid), variable, 2, grid_row++, 1, 1);

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (VarName))){
			VarName_i = gtk_label_new (dimname);
			SETUP_LABEL(VarName_i);
			variable_i = gtk_entry_new ();
			SETUP_ENTRY(variable_i, dimval);

                        gtk_grid_attach (GTK_GRID (grid_selected), VarName_i, 0, grid_row_s, 2, 1);
                        gtk_grid_attach (GTK_GRID (grid_selected), variable_i, 2, grid_row_s++, 1, 1);
		}
		g_free(dimname);
		g_free(dimval);

	}

	// ===============================================================================

	gtk_grid_attach (GTK_GRID (grid), sep=gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, grid_row++, 10, 1);
	gtk_grid_attach (GTK_GRID (grid), sep=gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, grid_row++, 10, 1);

	// ===============================================================================
	// DUMP:  vartype varname(dims)   data
	// ===============================================================================

	gtk_grid_attach (GTK_GRID (grid), lab=gtk_label_new("NC Data"), 0, grid_row++, 10, 1);
	gtk_grid_attach (GTK_GRID (grid), sep=gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, grid_row++, 10, 1);

	for (int n = 0; (vp = get_var(n)); n++) {
		int unlimited_flag = FALSE;
		gchar *vdims = g_strdup(" ");
		if (vp->num_dims() > 0) {
			g_free(vdims);
			vdims = g_strdup("(");
			for (int d = 0; d < vp->num_dims(); d++) {
				gchar *tmp = g_strconcat(vdims, (gchar*)vp->get_dim(d)->name(), 
							 ((d<vp->num_dims()-1)?", ":")"), NULL);

				if (vp->get_dim(d)->is_unlimited())
					unlimited_flag = TRUE;

				g_free(vdims);
				vdims = g_strdup(tmp);
				g_free(tmp);
			}
		}
//		gchar *vardef = g_strconcat(types[vp->type()], " ", (gchar*)vp->name(), vdims, NULL);
		gchar *vardef = g_strconcat((gchar*)vp->name(), vdims, NULL);
		g_free(vdims);
		VarName = gtk_check_button_new_with_label (vardef);
		setup_toggle (VarName, (gchar*)vp->name());
//		std::cout << vardef << std::endl;
		g_free(vardef);

                gtk_grid_attach (GTK_GRID (grid), VarName, 0, grid_row, 2, 1);

		variable = gtk_entry_new ();
		ostringstream ostr_val;

		if (unlimited_flag){
			ostr_val << "** Unlimited Data Set, data suppressed **";
		} else {
			NcValues *v = vp->values();
			if(vp->type() == ncChar){
				ostr_val << v->as_string(0);
			}else{
				if(v){
					if(v->num() > 1){
						if(v->num() > maxvals)
							ostr_val << "[#="<< vp->num_vals() << "] " << " ... (too many, suppressed) ";
						else
							ostr_val << "[#="<< vp->num_vals() << "] " << *v;
					}else
						ostr_val << *v;
				} else
					ostr_val << "** Empty **";
			}
			delete v;
		}
		SETUP_ENTRY(variable, (const gchar*)ostr_val.str().c_str());

                gtk_grid_attach (GTK_GRID (grid), variable, 2, grid_row, 1, 1);

		NcToken vname = vp->name();
		NcAtt *ap;
      
		ostringstream  ostr_att;
		if((ap=vp->get_att(0))){
			delete ap;
			ostr_att << "Details and NetCDF Varibale Attributes:" << endl;

			for(int n = 0; (ap = vp->get_att(n)); n++) {
				NcValues *v = ap->values();
				ostr_att << vname << ":" 
					 << ap->name() << " = "
					 << *v << endl;
				delete ap;
				delete v;
			}
		}
		else
			ostr_att << "Sorry, no info available for \"" << vname << "\" !";

		ostr_att << "\nValue(s):\n" << ostr_val.str().c_str(); 
		ostr_att << ends;
		gchar *infotxt = g_strdup( (const gchar*)ostr_att.str().c_str() );
		infolist = g_slist_prepend( infolist, infotxt);
		info = gtk_button_new_with_label (" Details ");

                gtk_grid_attach (GTK_GRID (grid), info, 3, grid_row++, 1, 1);

		g_signal_connect (G_OBJECT (info), "clicked",
				    G_CALLBACK (show_info_callback),
				    infotxt);

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (VarName))){
			NcAtt *short_name = NULL;
			if ((short_name = vp->get_att("short_name"))){
				NcValues *tmp = short_name->values();
				VarName_i = gtk_label_new (tmp->as_string(0));
				delete tmp;
			} else
				VarName_i = gtk_label_new (vp->name());
			SETUP_LABEL(VarName_i);

			NcAtt *unit_att = NULL;
			NcAtt *label_att = NULL;
			gchar *value_str = NULL;
			if ((unit_att = vp->get_att("unit"))){
				if ((label_att = vp->get_att("label"))){
					NcValues *unit  = unit_att->values();
					NcValues *label = label_att->values();
					UnitObj *u = gapp->xsm->MakeUnit (unit->as_string(0), label->as_string(0));
					double tmp;
					vp->get (&tmp);
					value_str = u->UsrString (tmp);
					delete unit;
					delete label;
					delete u;
					delete label_att;
					label_att = NULL;
				}
				delete unit_att;
				unit_att = NULL;
			}
			variable_i = gtk_entry_new ();
			if (value_str){
				SETUP_ENTRY(variable_i, value_str);
				g_free (value_str);
			}else{
				if ((unit_att = vp->get_att("var_unit"))){
					NcValues *u = unit_att->values();
					ostr_val << " [" << u->as_string(0) << "]"; // << " [vu]";
				} else if ((unit_att = vp->get_att("unit"))){
					NcValues *u = unit_att->values();
					ostr_val << " [" << u->as_string(0) << "]"; // << " [u]";
				} // else  ostr_val << " [??]";

				SETUP_ENTRY(variable_i, (const gchar*)ostr_val.str().c_str());
			}
                        gtk_grid_attach (GTK_GRID (grid_selected), VarName_i, 0, grid_row_s, 2, 1);
                        gtk_grid_attach (GTK_GRID (grid_selected), variable_i, 2, grid_row_s++, 1, 1);
		}
	}

	g_object_set_data (G_OBJECT (box), "info_list", infolist);

        gtk_widget_show_all (grid);
        gtk_widget_show_all (grid_selected);
        
}

void ViewControl::tip_follow_control (gboolean follow){
        tip_follow_flag = follow;
}

void ViewControl::setup_side_pane (gboolean show){
        //show_side_pane = show;

        // GTK3QQQ -- testing only
        //g_settings_set_boolean (view_settings, "sidepane-show", show);

	if (show){
		gtk_widget_show (sidepane);
		side_panel_width = 300;
                gtk_widget_set_size_request (hpaned, usx + side_panel_width, usy);
                gtk_paned_set_position (GTK_PANED (hpaned), usx);
		NcDumpToWidget ncdump (scan->data.ui.name);
		ncdump.dump (tab_ncraw, tab_info);
	}
	else{
		gtk_widget_hide (sidepane);
		side_panel_width = 0;
                gtk_widget_set_size_request (hpaned, usx + side_panel_width, usy);
	}
}

ViewControl::ViewControl (char *title, int nx, int ny, 
			  int ChNo, Scan *sc, 
			  int ZoomFac, int QuenchFac){
	GtkWidget *statusbar;
	GtkWidget *scrollarea;
	GtkWidget *base_grid, *grid, *frame_param;
	GtkWidget *label;
        int x,y,ii;
        
	GtkWidget *notebook1;
	GtkWidget *scrolledwindow1;
	GtkWidget *label1;
	GtkWidget *scrolledwindow2;
	GtkWidget *label2;
	GtkWidget *label3;
	GtkWidget *label4;
	GtkWidget *label5;
	GtkWidget *label6;

        destruction_in_progress = false;
        tip_follow_flag = false;
        view_settings = g_settings_new (GXSM_RES_BASE_PATH_DOT ".gui.view");
 
	XSM_DEBUG (DBG_L2, "ViewControl::ViewControl");

        tmp_object_op = NULL;

	border = 16;
	rulewidth = 0;
        ActiveFrameWidth = 1.;

	active_event = NULL; 
	gobjlist = NULL;
	geventlist = NULL;
	event_filter = NULL;
	gindicatorlist = NULL;

	guchar *array;
        gsize n_stores=0;

        // <key name="osd-enable" type="ab">
        GVariant *storage = g_settings_get_value (view_settings, "osd-enable");
        array = (guchar*) g_variant_get_fixed_array (storage, &n_stores, sizeof (guchar));

        XSM_DEBUG_GP (DBG_L4, "************** OSD n_stores=%d: [ %d, %d, %d..]\n", (int)n_stores, array[0]?0:1, array[1]?0:1, array[2]?0:1);
        
	for (gsize i=0; i<OSD_MAX; ++i){
		osd_entry[i] = NULL;
		osd_item[i] = NULL;
                osd_item_enable[i] = 0;
                if (i < n_stores)
                        osd_item_enable[i] = array[i] ? 1:0;
	}

	RedLine  = NULL;
	EventPlot = NULL;
	v_trace  = NULL;
	AddObjFkt = ViewControl::view_tool_addrectangle;
	ZoomQFkt = NULL;
	scan = sc;
	scan->RedLineActive = FALSE;
	scan->BlueLineActive = FALSE;
	chno=ChNo;
	local_radius = 10;
	npx = nx;
	npy = ny;
	vinfo = new ViewInfo(scan, QuenchFac, ZoomFac);

	if (ChNo < 0)
		vinfo->SetCoordMode (SCAN_COORD_RELATIVE);
        // ??? fix negativ ???
        
	CursorXYVt[0]=0.;
	CursorXYVt[1]=0.;
	CursorXYVt[2]=0.;
	CursorXYVt[3]=0.;

	SetMarkerGroup ();

	AppWindowInit (title);
        set_window_geometry ("view-scan2d", ChNo+1);
        
	g_object_set_data  (G_OBJECT (window), "Ch", GINT_TO_POINTER (ChNo));
	g_object_set_data  (G_OBJECT (window), "ChNo", GINT_TO_POINTER (ChNo+1));
	g_object_set_data  (G_OBJECT (window), "ViewControl", this);

	gapp->configure_drop_on_widget (GTK_WIDGET (window));

        // ==================================================
        // Setup 2D Grey Scan View with Sidepane on/off
        // ==================================================
        
	// hpanned box
	XSM_DEBUG (DBG_L2,  "VC::VC hpaned" );
	hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_grid_attach (GTK_GRID (v_grid), hpaned, 1,1, 3,3);

	// New Statusbar 2nd and last element, bottom
	XSM_DEBUG (DBG_L2,  "VC::VC statusbar" );
	statusbar = gtk_statusbar_new ();
	g_object_set_data (G_OBJECT (window), "statusbar", statusbar);
	gtk_grid_attach (GTK_GRID (v_grid), statusbar, 1,10, 3,1);

        // Setup Scrollarea for 2D SCAN CANVAS
	XSM_DEBUG (DBG_L2,  "VC::VC scrollarea" );
	scrollarea = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_set_hexpand (scrollarea, TRUE);
        gtk_widget_set_vexpand (scrollarea, TRUE);
        gtk_widget_show (scrollarea);
	gtk_container_set_border_width (GTK_CONTAINER (scrollarea), 2);
	
	/* the policy is one of GTK_POLICY AUTOMATIC, or GTK_POLICY_ALWAYS.
	 * GTK_POLICY_AUTOMATIC will automatically decide whether you need
	 * scrollbars, whereas GTK_POLICY_ALWAYS will always leave the scrollbars
	 * there.  The first one is the horizontal scrollbar, the second, 
	 * the vertical. */
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollarea),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	
        // pack1: scrollarea for CANVAS on left
        gtk_paned_pack1 (GTK_PANED (hpaned), scrollarea, TRUE, FALSE);   // place scrollarea for Scan Image Gnome-Canvas

        // ==================================================
        // 2D CANVAS with ShmImage2D
        // ==================================================
	XSM_DEBUG (DBG_L2,  "VC::VC canvas setup" );

        canvas = gtk_drawing_area_new(); // gtk3 cairo drawing-area -> "canvas"

	XSM_DEBUG (DBG_L2,  "VC::VC data attach" );
	g_object_set_data (G_OBJECT (canvas), "statusbar", statusbar);
	g_object_set_data (G_OBJECT (canvas), "ViewInfo", vinfo);
	g_object_set_data (G_OBJECT (canvas), "Ch", GINT_TO_POINTER (ChNo));
	g_object_set_data (G_OBJECT (canvas), "ViewControl", this);
	g_object_set_data (G_OBJECT (canvas), "Scan", scan);

	XSM_DEBUG (DBG_L2,  "VC::VC signal connect" );

        gtk_widget_add_events (canvas,
                               GDK_BUTTON_PRESS_MASK
                               | GDK_BUTTON_RELEASE_MASK 
                               | GDK_BUTTON_MOTION_MASK
			       | GDK_POINTER_MOTION_HINT_MASK
                               );

        /* Event signals */
        g_signal_connect (G_OBJECT (canvas), "event",
                          G_CALLBACK (ViewControl::canvas_event_cb), this);

        /* Hook Ximg class up to canvas */
	XSM_DEBUG (DBG_L2,  "VC::VC ximg" );
	// setup image object into canvas
	ximg = new ShmImage2D (canvas, nx/ZoomFac, ny/ZoomFac,0,0);

        // configure canvas draw callback 
        g_signal_connect (G_OBJECT (canvas), "draw",
                          G_CALLBACK (ViewControl::canvas_draw_callback), this);

        // place canvas into scrollarea
	XSM_DEBUG (DBG_L2,  "VC::VC container_add canvas to scrollarea" );
        gtk_container_add (GTK_CONTAINER (scrollarea), canvas);

        
	// ---------------------- Setup Information Sidepane ----------------------------
	XSM_DEBUG (DBG_L2,  "VC::VC Side Pane Setup" );

	// -- Side-Info-Pane Notebook --
        // ==================================================
	notebook1 = gtk_notebook_new ();
	gtk_widget_show (notebook1);
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook1), GTK_POS_RIGHT);
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook1), TRUE);
	gtk_notebook_popup_enable (GTK_NOTEBOOK (notebook1));

	gtk_paned_pack2 (GTK_PANED (hpaned), notebook1, TRUE, FALSE);   // place scrollarea for Scan Image Gnome-Canvas

	sidepane = notebook1;

	//if (show_side_pane)
        //  gtk_widget_show (sidepane);


	// -- Info Tab
        // ==================================================
	XSM_DEBUG (DBG_L2,  "VC::VC Info-Tab" );

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
        //	gtk_widget_show (scrolledwindow1);
	gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow1);
	gtk_widget_set_size_request (scrolledwindow1, 250, -1);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	tab_info = scrolledwindow1;

	label1 = gtk_label_new (N_("Info"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label1);
	gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_CENTER);
	gtk_label_set_angle (GTK_LABEL (label1), 90);

	// -- NC raw Tab
        // ==================================================
	XSM_DEBUG (DBG_L2,  "VC::VC NetCDF full view-Tab" );

	scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
        //	gtk_widget_show (scrolledwindow2);
	gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow2);
	gtk_widget_set_size_request (scrolledwindow2, 250, -1);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	tab_ncraw = scrolledwindow2;

        //	if (show_side_pane){
        NcDumpToWidget ncdump (scan->data.ui.name);
        ncdump.dump (tab_ncraw, tab_info);
        //}

        // ==================================================
	label2 = gtk_label_new (N_("NetCDF"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label2);
	gtk_label_set_use_markup (GTK_LABEL (label2), TRUE);
	gtk_label_set_angle (GTK_LABEL (label2), 90);

	// -- Probe Events Tab
        // ==================================================
	base_grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (notebook1), base_grid);

	label3 = gtk_label_new (N_("Probe Events"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label3);
	gtk_label_set_angle (GTK_LABEL (label3), 90);

	//     Events Control ----------------------------------------
	XSM_DEBUG (DBG_L2,  "VC::VC EVCtrl-Tab" );

        // gsettings!!!
        //  <key name="view-cursor-radius" type="d">
        //  <key name="view-max-number-events" type="d"> 
        //  <key name="view-arrow-size" type="d">
	XsmRescourceManager xrm("App_View");
        xrm.Get ("CursorRadius", &CursorRadius, "100.");
        xrm.Get ("MaxNumberEventsCursorRadius", &MaxNumberEvents, "30");
        xrm.Get ("ArrowSize", &ArrowSize, "25.");

	frame_param = gtk_frame_new (N_("Probe Events"));
	gtk_grid_attach (GTK_GRID (base_grid), frame_param, 1,1, 1,1);
	grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (frame_param), grid);
        x=y=1;

	APP_SELECTOR (active_event_list, "Type");
 	ADD_TOGGLE (tog_probe_events, "Show", 0);
 	g_signal_connect (tog_probe_events, "toggled", G_CALLBACK (ViewControl:: events_probe_callback), this);

        x=1, ++y;

	pe_info[ii=0] = gtk_entry_new ();
	SETUP_ENTRY(pe_info[ii], "---");
	gtk_grid_attach (GTK_GRID (grid), gtk_label_new ("X:"), x++, y, 1,1); 
	gtk_grid_attach (GTK_GRID (grid), pe_info[ii], x++, y, 1,1); 
        x=1, ++y; 

	pe_info[++ii] = gtk_entry_new ();
	SETUP_ENTRY(pe_info[ii], "---");
	gtk_grid_attach (GTK_GRID (grid), gtk_label_new ("Y:"), x++, y, 1,1); 
	gtk_grid_attach (GTK_GRID (grid), pe_info[ii], x++, y, 1,1); 
        x=1, ++y;

	pe_info[++ii] = gtk_entry_new ();
	SETUP_ENTRY(pe_info[ii], "---");
	gtk_grid_attach (GTK_GRID (grid), gtk_label_new ("Z:"), x++, y, 1,1); 
	gtk_grid_attach (GTK_GRID (grid), pe_info[ii], x++, y, 1,1); 
        x=1, ++y;

	pe_info[++ii] = gtk_entry_new ();
	SETUP_ENTRY(pe_info[ii], "---");
	gtk_grid_attach (GTK_GRID (grid), gtk_label_new ("t:"), x++, y, 1,1); 
	gtk_grid_attach (GTK_GRID (grid), pe_info[ii], x++, y, 1,1); 
        x=1, ++y;

        // ==================================================
	frame_param = gtk_frame_new (N_("Plot & Selection Control"));
	gtk_grid_attach (GTK_GRID (base_grid), frame_param, 1,2, 1,1);
	grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (frame_param), grid);
        x=y=1;

	APP_SELECTOR (active_event_xchan, "X");
        x=1, ++y;
	APP_SELECTOR (active_event_ychan, "Y");
        x=1, ++y;

//	gtk_combo_box_text_append (GTK_COMBO_BOX (active_event_list), "---");
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (active_event_xchan), "XCH-index", "Index");
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (active_event_ychan), "YCH-index", "Index");
	active_event_num_channels = 0;
	active_event_num_events = 0;
 	gtk_combo_box_set_active (GTK_COMBO_BOX (active_event_list), 0);
 	gtk_combo_box_set_active (GTK_COMBO_BOX (active_event_xchan), 0);
 	gtk_combo_box_set_active (GTK_COMBO_BOX (active_event_ychan), 0);

	APP_SELECTOR (select_events_by, "Plot");
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (select_events_by), "active", "Active");
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (select_events_by), "visible", "Visible");
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (select_events_by), "all", "All");
 	gtk_combo_box_set_active (GTK_COMBO_BOX (select_events_by), 0);

        x=1, ++y;
	GtkWidget* button = gtk_button_new_with_label(N_("Go!"));
	gtk_grid_attach (GTK_GRID (grid), button, x++, y, 1,1);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (ViewControl:: obj_event_plot_callback), this);

	XSM_DEBUG (DBG_L2,  "VC::VC EVCtrl-Tab c" );

	ADD_TOGGLE (tog_plot, "On Click", 0);
//	ADD_TOGGLE (tog_average, "All Avg.", 0);

	XSM_DEBUG (DBG_L2,  "VC::VC EVCtrl-Tab cc: LU" << gapp->xsm->LenUnit );

	XSM_DEBUG (DBG_L2,  "VC::VC EVCtrl-Tab cc: LU" << gapp->xsm->LenUnit );

        // !!!!!!!!!!!!! PORT TO BUILD_PARAM !!!!!!!!!!!!!
        x=1, ++y;
        {
                GtkWidget *input = mygtk_grid_add_spin_input (N_("Radius"), grid, x, y);
                // g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)(VIEW_PREFIX LABEL));
                ec_radius = new Gtk_EntryControl (gapp->xsm->LenUnit ? gapp->xsm->LenUnit : gapp->xsm->X_Unit,
                                                  OUT_OF_RANGE,
                                                  &CursorRadius, 0., 100000., ".1f", input, 10., 100.);
	}

        x=1, ++y;
        {
                GtkWidget *input = mygtk_grid_add_spin_input (N_("Number"), grid, x, y);
                ec_number = new Gtk_EntryControl (gapp->xsm->Unity,
                                                  OUT_OF_RANGE,
                                                  &MaxNumberEvents, 0., 500., ".0f", input, 1., 10.);
	}
        x=1, ++y;
        {
                GtkWidget *input = mygtk_grid_add_spin_input (N_("Arrow-Size"), grid, x, y);
                ec_arrowsize = new Gtk_EntryControl (gapp->xsm->Unity,
                                                     OUT_OF_RANGE,
                                                     &ArrowSize, 0., 200., ".1f", input, 1., 10.);
	}
        x=1, ++y;
	gtk_grid_attach (GTK_GRID (grid), gtk_label_new (N_("* click middle button to choose area of interest")), x++, y, 4,1); 

        // ==================================================
        frame_param = gtk_frame_new (N_("Export Data"));
	gtk_grid_attach (GTK_GRID (base_grid), frame_param, 1,3, 1,1);
	grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (frame_param), grid);
        x=y=1;

	button = gtk_button_new_with_label(N_("Dump to stdout"));
	gtk_grid_attach (GTK_GRID (grid), button, x++,y, 1,1);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (ViewControl:: obj_event_dump_callback), this);

        x=1, ++y;
	gtk_grid_attach (GTK_GRID (grid), gtk_label_new(N_("Note: To export or save data shown, please use the\nplot window, popup menu file or print to xmgrace.")), x++, y, 2,1);

        
	// -- User Events Tab
        // ==================================================
	base_grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (notebook1), base_grid);

	label4 = gtk_label_new (N_("User Events"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), label4);
	gtk_label_set_angle (GTK_LABEL (label4), 90);

	frame_param = gtk_frame_new (N_("User Events"));
	grid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (base_grid), frame_param, 1,1, 1,1);
	gtk_container_add (GTK_CONTAINER (frame_param), grid);
        x=y=1;

 	ADD_TOGGLE (tog_user_events, "Show", 0);
 	g_signal_connect (tog_user_events, "toggled", G_CALLBACK (ViewControl:: events_user_callback), this);
        x=1, ++y;

        const gchar *ue_what[] = { "Type", "X", "Y", "Z", "time", "What", "...", "...", "...", "...", NULL };
        
	for (int i=0; ue_what[i] && i<10; ++i){
                ue_info[i] = gtk_entry_new ();
                SETUP_ENTRY(ue_info[i], "---");
                gtk_grid_attach (GTK_GRID (grid), gtk_label_new (ue_what[i]), x++, y, 1,1); 
                gtk_grid_attach (GTK_GRID (grid), ue_info[i], x++, y, 1,1); 
                x=1, ++y;
        }

	// -- Objects Tab
        // ==================================================
	base_grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (notebook1), base_grid);

	label5 = gtk_label_new (N_("Objects"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 4), label5);
	gtk_label_set_angle (GTK_LABEL (label5), 90);

        grid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (base_grid), grid, 1,1, 1,1);
        x=y=1;

	GtkListStore *v_objects_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	GtkTreeIter iter;
	gtk_list_store_append (v_objects_store, &iter);
	gtk_list_store_set (v_objects_store, &iter,
		       0, "Obj[1]",
		       1, "0",
		       2, "00",
		       -1);
	gtk_list_store_append (v_objects_store, &iter);
	gtk_list_store_set (v_objects_store, &iter,
		       0, "Obj[2]",
		       1, "0",
		       2, "00",
		       -1);
	GtkCellRenderer     *renderer;
	GtkTreeModel        *model = GTK_TREE_MODEL (v_objects_store);
	GtkWidget           *view;

	view = gtk_tree_view_new ();

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
						     -1,      
						     "Label",  
						     renderer,
						     "text", 0,
						     NULL);

	gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);
	gtk_grid_attach (GTK_GRID (grid), view, 1,1, 1,1);


	// -- On Scan Display Setup (OSD) -- Tab
	XSM_DEBUG (DBG_L2,  "VC::VC OSD-Tab" );

        // ==================================================
	GtkWidget *scrollarea_sp = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (scrollarea_sp), 0);
	
	/* the policy is one of GTK_POLICY AUTOMATIC, or GTK_POLICY_ALWAYS.
	 * GTK_POLICY_AUTOMATIC will automatically decide whether you need
	 * scrollbars, whereas GTK_POLICY_ALWAYS will always leave the scrollbars
	 * there.  The first one is the horizontal scrollbar, the second, 
	 * the vertical. */
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollarea_sp),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (notebook1), scrollarea_sp);
	base_grid = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (scrollarea_sp), base_grid);

	label6 = gtk_label_new (N_("OSD"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 5), label6);
	gtk_label_set_angle (GTK_LABEL (label6), 90);
        
        grid = gtk_grid_new ();
	gtk_grid_attach (GTK_GRID (base_grid), grid, 1,1, 1,1);
        x=y=1;

	GtkWidget *osd_show, *osd_on, *osd_off;
	for (int row=0; row<OSD_MAX; ++row){
                x=1;
		osd_on = gtk_toggle_button_new_with_label ("0");
		g_object_set_data (G_OBJECT (osd_on), "OSD_POS", GINT_TO_POINTER (row));
		g_signal_connect (G_OBJECT (osd_on), "toggled",
				    G_CALLBACK (osd_on_toggle_callback),
				    this);
		osd_off = gtk_toggle_button_new_with_label ("00");
		g_object_set_data (G_OBJECT (osd_off), "OSD_POS", GINT_TO_POINTER (row));
		g_signal_connect (G_OBJECT (osd_off), "toggled",
				    G_CALLBACK (osd_off_toggle_callback),
				    this);

		osd_show = gtk_check_button_new ();
		g_object_set_data (G_OBJECT (osd_show), "OSD_POS", GINT_TO_POINTER (row));
		g_signal_connect (G_OBJECT (osd_show), "toggled",
				    G_CALLBACK (osd_toggle_callback),
				    this);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (osd_show), osd_item_enable[row]);

		osd_entry[row] = gtk_entry_new ();
		g_object_set_data (G_OBJECT (osd_entry[row]), "OSD_SELECTOR", GINT_TO_POINTER (osd_show));
		SETUP_ENTRY(osd_entry[row], "---");
		gtk_grid_attach (GTK_GRID (grid), osd_entry[row], x++, y, 1,1);
		gtk_grid_attach (GTK_GRID (grid), osd_on,         x++, y, 1,1);
		gtk_grid_attach (GTK_GRID (grid), osd_off,        x++, y, 1,1);
		gtk_grid_attach (GTK_GRID (grid), osd_show,       x++, y, 1,1);
                ++y;
	}

	// ------------------- Ende Side Pane Setup ---------------------------


        Resize (NULL, -1, -1, 0, NULL, ZoomFac, QuenchFac);

	XSM_DEBUG (DBG_L2,  "VC::VC show" );
	gtk_widget_show_all ( v_grid );
}

ViewControl::~ViewControl (){
	XSM_DEBUG (DBG_L2,  "~ViewControl" );

        destruction_in_progress = true;

        g_settings_set_double (view_settings, "view-cursor-radius", CursorRadius);
        g_settings_set_double (view_settings, "view-max-number-events", MaxNumberEvents);
        g_settings_set_double (view_settings, "view-cursor-radius", ArrowSize);
 
	RemoveObjects();

	if(RedLine)
		delete RedLine;

	if (EventPlot)
		delete EventPlot;

	
	remove_trace ();

	RemoveEventObjects ();

	RemoveIndicators();

	delete ximg;
	delete vinfo;

        delete ec_radius;
        delete ec_number;
        delete ec_arrowsize;
        
        g_clear_object (&view_settings);
}

void ViewControl::AppWindowInit(const gchar *title){
	XSM_DEBUG (DBG_L2,  "ViewControl::AppWindowInit -- header bar,..." );

        app_window = gxsm3_app_window_new (GXSM3_APP (gapp->get_application ()));
        window = GTK_WINDOW (app_window);

        header_bar = gtk_header_bar_new ();
        gtk_widget_show (header_bar);
        gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), true);

        // link view popup actions
        g_action_map_add_action_entries (G_ACTION_MAP (app_window),
                                         win_view_popup_entries, G_N_ELEMENTS (win_view_popup_entries),
                                         this);

        // link popup actions with window
        g_action_map_add_action_entries (G_ACTION_MAP (app_window),
                                         win_object_popup_entries, G_N_ELEMENTS (win_object_popup_entries),
                                         this);
        
        // create window PopUp menu  ---------------------------------------------------------------------
        XSM_DEBUG (DBG_L2,  "VC::VC popup" );
        v_popup_menu = gtk_menu_new_from_model (G_MENU_MODEL (gapp->get_view2d_menu ()));
        XSM_DEBUG (DBG_L2,  "VC::VC popup a" );
        g_assert (GTK_IS_MENU (v_popup_menu));
        XSM_DEBUG (DBG_L2,  "VC::VC popup b" );


        XSM_DEBUG (DBG_L2,  "VC::VC popup Header Buttons setup. " );
	GtkIconSize tmp_toolbar_icon_size = GTK_ICON_SIZE_LARGE_TOOLBAR;

        // attach full view popup menu to tool button ----------------------------------------------------
        GtkWidget *header_menu_button = gtk_menu_button_new ();
        //        gtk_button_set_image (GTK_BUTTON (header_menu_button), gtk_image_new_from_icon_name ("emblem-system-symbolic", tmp_toolbar_icon_size));
        gtk_menu_button_set_popup (GTK_MENU_BUTTON (header_menu_button), v_popup_menu);
        gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_menu_button);
        gtk_widget_show (header_menu_button);

        // attach display mode section from popup menu to tool button --------------------------------
        header_menu_button = gtk_menu_button_new ();
        gtk_button_set_image (GTK_BUTTON (header_menu_button), gtk_image_new_from_icon_name ("emblem-system-symbolic", tmp_toolbar_icon_size));
        GMenuModel *section = find_extension_point_section (G_MENU_MODEL (gapp->get_view2d_menu ()), "view-display-mode-section");
        if (section) {
                gtk_menu_button_set_popup (GTK_MENU_BUTTON (header_menu_button), gtk_menu_new_from_model (G_MENU_MODEL (section)));
                gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_menu_button);
                gtk_widget_show (header_menu_button);
        }

        // attach object section from popup menu to tool button --------------------------------
        header_menu_button = gtk_menu_button_new ();
        section = find_extension_point_section (G_MENU_MODEL (gapp->get_view2d_menu ()), "view-objects-section");
        gtk_button_set_image (GTK_BUTTON (header_menu_button), gtk_image_new_from_icon_name ("applications-utilities-symbolic", tmp_toolbar_icon_size));
        if (section) {
                gtk_menu_button_set_popup (GTK_MENU_BUTTON (header_menu_button), gtk_menu_new_from_model (G_MENU_MODEL (section)));
                gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_menu_button);
                gtk_widget_show (header_menu_button);
        }

        // attach channel mode section from popup menu to tool button --------------------------------
        header_menu_button = gtk_menu_button_new ();
        section = find_extension_point_section (G_MENU_MODEL (gapp->get_view2d_menu ()), "view-channel-mode-section");
        gtk_button_set_image (GTK_BUTTON (header_menu_button), gtk_image_new_from_icon_name ("emblem-photos-symbolic", tmp_toolbar_icon_size));
        if (section) {
                gtk_menu_button_set_popup (GTK_MENU_BUTTON (header_menu_button), gtk_menu_new_from_model (G_MENU_MODEL (section)));
                gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_menu_button);
                gtk_widget_show (header_menu_button);
        }

        // attach file section from popup menu to tool button --------------------------------
        header_menu_button = gtk_menu_button_new ();
        gtk_button_set_image (GTK_BUTTON (header_menu_button), gtk_image_new_from_icon_name ("document-open-symbolic", tmp_toolbar_icon_size));
        section = find_extension_point_section (G_MENU_MODEL (gapp->get_view2d_menu ()), "view-file-section");
        if (section) {
                gtk_menu_button_set_popup (GTK_MENU_BUTTON (header_menu_button), gtk_menu_new_from_model (G_MENU_MODEL (section)));
                gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_menu_button);
                gtk_widget_show (header_menu_button);
        }

        header_menu_button = side_pane_control = gtk_toggle_button_new ();
        gtk_button_set_image (GTK_BUTTON (header_menu_button), gtk_image_new_from_icon_name ("view-dual-symbolic", tmp_toolbar_icon_size));
        gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_menu_button);
        gtk_widget_show (header_menu_button);
	gtk_widget_set_tooltip_text (header_menu_button, N_("Show Side Info/Control Pane"));
        g_settings_bind (view_settings, "sidepane-show",
                         G_OBJECT (side_pane_control), "active",
                         G_SETTINGS_BIND_DEFAULT);
        g_signal_connect (G_OBJECT (side_pane_control), "toggled",
                          G_CALLBACK (ViewControl::side_pane_callback), this);
        
        header_menu_button = side_pane_control = gtk_toggle_button_new ();
        gtk_button_set_image (GTK_BUTTON (header_menu_button), gtk_image_new_from_icon_name ("mark-location-symbolic", tmp_toolbar_icon_size));
        gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_menu_button);
        gtk_widget_show (header_menu_button);
        gtk_widget_set_name (header_menu_button, "view-headerbar-tip-follow"); // name used by CSS to apply custom color scheme
	gtk_widget_set_tooltip_text (header_menu_button, N_("Enable Tip (Scan Position) Follow Point Objects"));
        g_signal_connect (G_OBJECT (side_pane_control), "toggled",
                          G_CALLBACK (ViewControl::tip_follow_callback), this);
        //        GtkStyleContext *context = gtk_widget_get_style_context (header_menu_button);
        //        gtk_style_context_add_class(context, ".view-headerbar-tip-follow");
        
        XSM_DEBUG (DBG_L2,  "VC::VC setup titlbar" );

        gtk_window_set_title (GTK_WINDOW (window), title);
        gtk_header_bar_set_title ( GTK_HEADER_BAR (header_bar), title);
        gtk_header_bar_set_subtitle (GTK_HEADER_BAR  (header_bar), title);
        gtk_window_set_titlebar (GTK_WINDOW (window), header_bar);

        //        The “activate-default” signal -- dose NOT work
        g_signal_connect (G_OBJECT(window),
                          "activate-default",
                          G_CALLBACK (ViewControl::Activate_window_callback),
                          this);
                
        g_signal_connect (G_OBJECT(window),
                          "delete_event",
                          G_CALLBACK(App::close_scan_event_cb),
                          this);
        
	v_grid = gtk_grid_new ();
        gtk_container_add (GTK_CONTAINER (window), v_grid);
	g_object_set_data (G_OBJECT (window), "v_grid", v_grid); // was "vbox"

        //        g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
        g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (AppBase::window_close_callback), this);

	gtk_widget_show_all (GTK_WIDGET (window));
}

gboolean ViewControl::canvas_draw_callback (GtkWidget *widget, cairo_t *cr, ViewControl *vc){
        if (vc->destruction_in_progress)
                return false;

        XSM_DEBUG (DBG_L2,  "ViewControl:::canvas_draw_callback ********************** SCAN DRAW *********************" );

        double zf = vc->vinfo->GetZfac();
        vc->ximg->set_translate_offset (vc->rulewidth+vc->border/zf, vc->rulewidth+vc->border/zf);
        cairo_translate (cr, (double)(vc->rulewidth+vc->border/zf), (double)(vc->rulewidth+vc->border/zf));
        cairo_scale (cr, zf, zf);

        // 1) draw ActiveFrame (or not)
        if (vc->ActiveFrameWidth > 0.){
        //	XSM_DEBUG (DBG_L2,  "ViewControl:::canvas_draw_callback ********************** SCAN PAINT FRAME *********************" );
                cairo_set_source_rgb (cr, 1.0, 1.0, 0.0); // yellow
                cairo_set_line_width (cr, 4.*vc->ActiveFrameWidth);
                cairo_rectangle (cr, 
                                 -vc->ActiveFrameWidth/2./zf,  -vc->ActiveFrameWidth/2./zf,
                                 (vc->ActiveFrameWidth+vc->npx)/zf, (vc->ActiveFrameWidth+vc->npy)/zf);
                cairo_stroke(cr);
        }

        // 2) draw image and red line via ShmImage2D
	vc->ximg->draw_callback (cr);

        // 3) Draw Objects and Events
        //	XSM_DEBUG (DBG_L2,  "ViewControl:::canvas_draw_callback ********************** SCAN PAINT OBJ *********************" );
        vc->DrawObjects (cr);

        if(vc->RedLine){
                cairo_translate (cr, 0, vc->npy/zf);
                cairo_scale (cr, vc->npx/zf/vc->RedLine->get_drawing_width()*1.3, vc->npy/zf/vc->RedLine->get_drawing_width()*0.2);
                 
                vc->RedLine->cairo_draw_profile_only_callback (cr, vc->RedLine);
        }
        
        //	XSM_DEBUG (DBG_L2,  "ViewControl:::canvas_draw_callback ********************** SCAN PAINT * DONE. *********************" );

        return TRUE;
}

void ViewControl::Resize (char *title, int nx, int ny, 
			  int ChNo, Scan *sc, 
			  int ZoomFac, int QuenchFac){
        if (nx > 0){
                npx=nx; npy=ny;
                vinfo->SetQfZf(QuenchFac, ZoomFac);

                XSM_DEBUG (DBG_L2,  "VC::RESIZE ximg->Resize" );
                ximg->Resize (nx/ZoomFac,ny/ZoomFac);
	}

        //	usx = MIN((nx+rulewidth+2*border), (2*gdk_screen_width()/3));
        //        usy = MIN((int)(ny+rulewidth+2*border), (2*gdk_screen_height()/3));
	usx = MIN((nx+rulewidth+2*border), 550);
        usy = MIN((int)(ny+rulewidth+2*border), 550);
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (side_pane_control))){
                NcDumpToWidget ncdump (scan->data.ui.name);
		ncdump.dump (tab_ncraw, tab_info);
                side_panel_width = 400;
                setup_side_pane (true);
        } else {
                side_panel_width = 0;
                setup_side_pane (false);
        }
        
	// refit image object into canvas
	XSM_DEBUG (DBG_L2,  "VC::RESIZE setting window default size: " << usx << ", "<< usy );
        gtk_widget_set_size_request (canvas, rulewidth+(nx+2*border), rulewidth+(ny+2*border));
        gtk_widget_set_size_request (hpaned, usx+2*border+2*rulewidth+side_panel_width, usy);
	gtk_paned_set_position (GTK_PANED (hpaned), usx+2*border+2*rulewidth);

        gtk_window_resize (GTK_WINDOW (window), usx+2*border+2*rulewidth+side_panel_width, usy);

	XSM_DEBUG (DBG_L2, "VC::RESIZE done" );
}

void ViewControl::SetTitle(const gchar *title, const gchar *subtitle){
	gtk_window_set_title (GTK_WINDOW (window), title);

        gtk_header_bar_set_title ( GTK_HEADER_BAR (header_bar), title);
        if (subtitle)
                gtk_header_bar_set_subtitle (GTK_HEADER_BAR  (header_bar), subtitle);

        CheckOptions();
}

void get_obj_coords_wrapper(int i, double &x, double &y){
	if (current_vobject)
		current_vobject->obj_get_xy_i (i,x,y);
}

void ViewControl::AddObject(VObject *vo){
	gobjlist = g_slist_prepend (gobjlist, vo);
	vo->Update ();
	if (current_vobject){
		XSM_DEBUG(DBG_L2, "ViewControl::AddObject  ERROR, recursive call occured!!" );
		return;
	}
	current_vobject = vo;
	vo->obj_id (scan->add_object (vo->obj_name (), vo->obj_text (),
				      vo->obj_num_points (),
				      get_obj_coords_wrapper));

	vo->set_marker_scale (xsmres.HandleSize/100.);
	current_vobject = NULL;
}

// -- Manage Indicator Objects
void ViewControl::AddIndicator(VObject *vo){
	gindicatorlist = g_slist_prepend (gindicatorlist, vo);
	vo->Update ();
	vo->obj_id (scan->add_object (vo->obj_name (), vo->obj_text (),
				      vo->obj_num_points (),
				      get_obj_coords_wrapper));

	vo->set_marker_scale (xsmres.HandleSize/100.);
}

void ViewControl::RemoveIndicators(){
	g_slist_foreach((GSList*) gindicatorlist, (GFunc) ViewControl::remove_obj, this);
	g_slist_free(gindicatorlist);
	gindicatorlist = NULL;
}

// filter: 'P' for Probe Event, 'U' for User Event
// pos: 0,1  [0]<-P,-   1<-U,-
void ViewControl::SetEventFilter(const gchar *filter, gint pos){
	if (!event_filter)
		event_filter = g_strdup ("--");
	switch (pos){
	case 0: event_filter[0] = *filter; break;
	case 1: event_filter[1] = *filter; break;
	}
}

void ViewControl::RescanEventObjects(){
	if (event_filter)
		scan->mem2d->ReportScanEvents ((GFunc) ViewControl::add_event_obj, this, CursorXYVt, CursorRadius, MaxNumberEvents);
}


gint check_func (gpointer *vo, gpointer *se){
	return ((VObEvent*) vo)->get_scan_event() == (ScanEvent*)se ? 0:-1;
}

void ViewControl::add_event_obj(ScanEvent *se, ViewControl *vc)
{
	double xy[2];

	if (se->flag) return;
	if (! ((EventEntry*)(se->event_list->data))->description_id_match (vc->event_filter)) return;

// nice check, but gets horribly inefficient and slow for a very long list... using a flag now
// 	if (vc->geventlist) // check if existing
// 		if (g_slist_find_custom (vc->geventlist, se, (GCompareFunc)check_func))
// 			return;

	se->get_position (xy[0], xy[1]);
	VObEvent *voe = new VObEvent (vc->canvas, xy, vc->scan->Pkt2d, FALSE, VOBJ_COORD_ABSOLUT, NULL, vc->ArrowSize/100.);
	voe->set_scan_event (se);
	gchar *obn = g_strconcat ("Ev-", ((EventEntry*)(se->event_list->data))->description(), NULL);
	voe->set_obj_name (obn);
	g_free (obn);
	vc->geventlist = g_slist_prepend (vc->geventlist, voe);
	se->flag = TRUE; // set object flag

//      set object label to info
	if (((EventEntry*)(se->event_list->data))->description_id () == 'U'){
		UserEntry* ue = (UserEntry*) (se->event_list->data);
		voe->obj_text (ue->get (0));
	}
}

void ViewControl::RemoveEventObjects(){
// unsets object flag and delete obj!
	g_slist_foreach((GSList*) geventlist, (GFunc) ViewControl::unflag_scan_event_and_remove_obj, this); 
	g_slist_free(geventlist);
	geventlist = NULL;

	active_event = NULL; 
}

void ViewControl::set_event_label(ScanEvent *se, ViewControl *vc){
}

void ViewControl::SetEventLabels(int mode){
	if (mode)
		g_slist_foreach((GSList*) geventlist, (GFunc) ViewControl::obj_label_on, this);
	else
		g_slist_foreach((GSList*) geventlist, (GFunc) ViewControl::obj_label_off, this);

}

void ViewControl::update_event_panel (ScanEvent *se){
	gint xi,yi;

	if (!se){
		active_event = NULL; 
		return; 
	}

	active_event = se;
//	se->print ();

	xi = gtk_combo_box_get_active (GTK_COMBO_BOX (active_event_xchan));
	yi = gtk_combo_box_get_active (GTK_COMBO_BOX (active_event_ychan));

	while (active_event_num_events)
		gtk_combo_box_text_remove (GTK_COMBO_BOX_TEXT (active_event_list), active_event_num_events--);

	while (active_event_num_channels){
		gtk_combo_box_text_remove (GTK_COMBO_BOX_TEXT (active_event_xchan), active_event_num_channels);
		gtk_combo_box_text_remove (GTK_COMBO_BOX_TEXT (active_event_ychan), active_event_num_channels--);
	}
	active_event_num_channels = 0;
	active_event_num_events = active_event->get_event_count();

	GSList *ev=active_event->event_list;
	for (guint i=0; i<active_event_num_events; ++i){
		EventEntry *ee = (EventEntry*) ev->data;
//		gchar *txt = g_strdup_printf ("%s @ %u", ee->description(), (guint)ee->get_time());
		gchar *txt = g_strdup_printf ("%s", ee->description());
		gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (active_event_list), txt, txt);
		g_free (txt);
		txt = g_strdup_printf ("%u sec since 00:00:00 UTC, January 1, 1970", (guint)ee->get_time());
		if (ee->description_id () == 'P')
			gtk_entry_set_text (GTK_ENTRY (pe_info[3]), txt);
		else if (ee->description_id () == 'U')
			gtk_entry_set_text (GTK_ENTRY (ue_info[4]), txt);
		g_free (txt);
		ev=ev->next;
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX (active_event_list), 0);

	EventEntry *ee = (EventEntry*) active_event->event_list->data;
	if (ee->description_id () == 'P'){
		ProbeEntry* pe = (ProbeEntry*) ee;
		active_event_num_channels = pe->get_chunk_size ();
		double x,y;
		gchar *txt = g_strdup_printf ("%.2f", se->get_position(x,y));
		gtk_entry_set_text (GTK_ENTRY (pe_info[2]), txt);
		g_free (txt);
		double ix,iy;
		scan->World2Pixel (x,y, ix, iy);
		txt = g_strdup_printf ("%.2f " UTF8_ANGSTROEM " [%.1f px]", x, ix);
		gtk_entry_set_text (GTK_ENTRY (pe_info[0]), txt);
		g_free (txt);
		txt = g_strdup_printf ("%.2f " UTF8_ANGSTROEM " [%.1f px]", y, iy);
		gtk_entry_set_text (GTK_ENTRY (pe_info[1]), txt);
		g_free (txt);
		for (guint i=0; i<active_event_num_channels; ++i){
			gchar *txt = g_strdup_printf ("%s [%s]", pe->get_label (i), pe->get_unit_symbol (i)); 
			gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (active_event_xchan), "xchan", txt);	
			gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (active_event_ychan), "ychan", txt);	
			g_free (txt);
		}
	}

	if (ee->description_id () == 'U'){
		UserEntry* ue = (UserEntry*) ee;
		double x,y;
		gchar *txt = g_strdup_printf ("%.2f", se->get_position(x,y));
		gtk_entry_set_text (GTK_ENTRY (ue_info[3]), txt);
		g_free (txt);
		double ix,iy;
		scan->World2Pixel (x,y, ix, iy);
		txt = g_strdup_printf ("%.2f " UTF8_ANGSTROEM " [%.1f px]", x, ix);
		gtk_entry_set_text (GTK_ENTRY (ue_info[1]), txt);
		g_free (txt);
		txt = g_strdup_printf ("%.2f " UTF8_ANGSTROEM " [%.1f px]", y, iy);
		gtk_entry_set_text (GTK_ENTRY (ue_info[2]), txt);
		g_free (txt);
		gtk_entry_set_text (GTK_ENTRY (ue_info[0]), ee->description()); 
		for (int i=0; i<ue->get_num_sets (); ++i)
			if (i<4)
				gtk_entry_set_text (GTK_ENTRY (ue_info[5+i]), ue->get(i));
		
	}

	gtk_combo_box_set_active (GTK_COMBO_BOX (active_event_xchan), xi);
	gtk_combo_box_set_active (GTK_COMBO_BOX (active_event_ychan), yi);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (tog_plot)))
		obj_event_plot_callback (NULL, NULL, this);

// 	i = gtk_combo_box_get_active (GTK_COMBO_BOX (active_event_xchan));
}

void  ViewControl::obj_event_plot_callback (GSimpleAction *simple, GVariant *parameter, 
                                            gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (!vc->active_event) return;

	EventEntry *ee = (EventEntry*) vc->active_event->event_list->data;

	if (ee->description_id () == 'P'){
		ProbeEntry* pe = (ProbeEntry*) ee;
		gint xi,yi,nn;
		double xmin, xmax, x;

		nn = pe->get_num_sets ();
		xi = gtk_combo_box_get_active (GTK_COMBO_BOX (vc->active_event_xchan)) - 1;
		yi = gtk_combo_box_get_active (GTK_COMBO_BOX (vc->active_event_ychan)) - 1;

		UnitObj *UXaxis = new UnitObj(pe->get_unit_symbol (xi), " ", "g", pe->get_label (xi));
		UnitObj *UYaxis = new UnitObj(pe->get_unit_symbol (yi), " ", "g", pe->get_label (yi));
		
		// find min and max X limit
		xmin = xmax = pe->get (0, xi);
		for(int i = 1; i < nn; i++){
			x =  pe->get (i, xi);
			if (x > xmax) xmax = x;
			if (x < xmin) xmin = x;
		}

		if (!vc->EventPlot){
			gchar   *title  = g_strdup_printf ("Probe Event");
			vc->EventPlot = new ProfileControl (title, nn, UXaxis, UYaxis, xmin, xmax, "EventPlot");
			g_free (title);
		} else {
			vc->EventPlot->SetXrange (xmin, xmax);
			vc->EventPlot->SetXlabel (pe->get_label (xi));
			vc->EventPlot->SetYlabel (pe->get_label (yi));
//			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (vc->tog_average))){
			if ( gtk_combo_box_get_active (GTK_COMBO_BOX (vc->select_events_by)) == 1){
				gchar* txt = g_strdup_printf ("Average of all probe events shown: %s", pe->get_label (yi));
				vc->EventPlot->SetTitle (txt);
				g_free (txt);
			}else if ( gtk_combo_box_get_active (GTK_COMBO_BOX (vc->select_events_by)) == 2){
				gchar* txt = g_strdup_printf ("All probe events shown: %s", pe->get_label (yi));
				vc->EventPlot->SetTitle (txt);
				g_free (txt);
			}else
				vc->EventPlot->SetTitle (pe->get_label (yi));
		}
		
//		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (vc->tog_average))){

		if ( gtk_combo_box_get_active (GTK_COMBO_BOX (vc->select_events_by)) == 0){
			vc->EventPlot->RemoveScans ();
			vc->EventPlot->scan1d->mem2d->Resize (nn, 1);
			vc->EventPlot->AddScan (vc->EventPlot->scan1d, 0);
		        for(int i = 0; i < nn; i++)
			        vc->EventPlot->SetPoint (i, pe->get (i, xi), pe->get (i, yi));
		} else if ( gtk_combo_box_get_active (GTK_COMBO_BOX (vc->select_events_by)) == 1){
			vc->EventPlot->RemoveScans ();
			vc->EventPlot->scan1d->mem2d->Resize (nn, 1);
			vc->EventPlot->AddScan (vc->EventPlot->scan1d, 0);
			GSList* all = vc->scan->mem2d->ReportScanEvents (NULL, NULL, vc->CursorXYVt, 0., 0);
			GSList* ev = all;
			int i=0;
			int count=0;
			while (ev){
				ScanEvent *sen = (ScanEvent*) ev->data;
				if (sen != vc->active_event){
					EventEntry *een = (EventEntry*) sen->event_list->data;
					if (een->description_id () == 'P'){
						ProbeEntry* pen = (ProbeEntry*) een;
						if (++i > vc->MaxNumberEvents || ((ScanEvent*)(ev->data))->distance (vc->CursorXYVt) > vc->CursorRadius)
							break;
						
						
						for(int i = 0; i < nn; i++)
							vc->EventPlot->AddPoint (i, pen->get (i, yi));
						
						++count;
					}
				}
				ev = g_slist_next (ev);
			}
			g_slist_free (all);
			if (count > 0)
			        for(int i = 0; i < nn; i++)
				        vc->EventPlot->MulPoint (i, 1./((double)count));

		} else if ( gtk_combo_box_get_active (GTK_COMBO_BOX (vc->select_events_by)) == 2){
			GSList* all = vc->scan->mem2d->ReportScanEvents (NULL, NULL, vc->CursorXYVt, 0., 0);
			GSList* ev = all;
			int i=0;
			int count=0;
			while (ev){
				ScanEvent *sen = (ScanEvent*) ev->data;
				if (sen != vc->active_event){
					EventEntry *een = (EventEntry*) sen->event_list->data;
					if (een->description_id () == 'P'){
						if (++i > vc->MaxNumberEvents || ((ScanEvent*)(ev->data))->distance (vc->CursorXYVt) > vc->CursorRadius)
							break;
						count++;
					}
				}
				ev = g_slist_next (ev);
			}
			vc->EventPlot->RemoveScans ();
			vc->EventPlot->scan1d->mem2d->Resize (nn, count);
			vc->EventPlot->scan1d->mem2d->data->MkYLookup (1, count);
			count=0;
			i=0;
			ev = all;
			while (ev){
				ScanEvent *sen = (ScanEvent*) ev->data;
				if (sen != vc->active_event){
					EventEntry *een = (EventEntry*) sen->event_list->data;
					if (een->description_id () == 'P'){
						ProbeEntry* pen = (ProbeEntry*) een;
						if (++i > vc->MaxNumberEvents || ((ScanEvent*)(ev->data))->distance (vc->CursorXYVt) > vc->CursorRadius)
							break;
						
						vc->EventPlot->AddScan (vc->EventPlot->scan1d, count);
						for(int i = 0; i < nn; i++)
						        vc->EventPlot->SetPoint (i, pen->get (i, yi), count);
						
						++count;
					}
				}
				ev = g_slist_next (ev);
			}
			g_slist_free (all);
		}
		else{
			for(int i = 0; i < nn; i++)
				vc->EventPlot->SetPoint (i, pe->get (i, xi), pe->get (i, yi));
		}

                // CHECK --- OBSOLETE ????
                //		vc->EventPlot->drawScans ();
                vc->EventPlot->UpdateArea ();
                //		vc->EventPlot->show ();
	}
}

void ViewControl::DrawObjects(cairo_t *cr){
	g_slist_foreach((GSList*) gobjlist, (GFunc) ViewControl::draw_obj, cr);
	g_slist_foreach((GSList*) geventlist, (GFunc) ViewControl::draw_obj, cr);
}


void ViewControl::PaintAllRegionsInactive(){
	g_slist_foreach((GSList*) gobjlist, (GFunc) ViewControl::deactivate_obj, this); 
	g_slist_foreach((GSList*) geventlist, (GFunc) ViewControl::deactivate_obj, this); 
}

void ViewControl::PaintAllRegionsActive(){
	g_slist_foreach((GSList*) gobjlist, (GFunc) ViewControl::activate_obj, this); 
	g_slist_foreach((GSList*) geventlist, (GFunc) ViewControl::activate_obj, this); 
}

void ViewControl::MoveAllObjects2LocMax(){
	g_slist_foreach((GSList*) gobjlist, (GFunc) ViewControl::move2locmax_obj, this);
}

void ViewControl::CheckAllObjectsLabels(gboolean flg){
	if (flg)
		g_slist_foreach((GSList*) gobjlist, (GFunc) ViewControl::obj_label_on, this);
	else
		g_slist_foreach((GSList*) gobjlist, (GFunc) ViewControl::obj_label_off, this);
}

void ViewControl::RemoveObjects(){
	g_slist_foreach((GSList*) gobjlist, (GFunc) ViewControl::remove_obj, this);
	g_slist_free(gobjlist);
	gobjlist = NULL;
	scan->PktVal=0;
}

void ViewControl::UpdateObjects(){
	g_slist_foreach((GSList*) gobjlist, (GFunc) ViewControl::obj_update, this);
}

void ViewControl::SaveObjects(){
	g_slist_foreach((GSList*) gobjlist, (GFunc) ViewControl::save_obj, this);
}

void ViewControl::SaveObjectsHPGL(){
	g_slist_foreach((GSList*) gobjlist, (GFunc) ViewControl::save_obj_HPGL, this);
}

void ViewControl::SetActive(int flg){
	GtkWidget *statusbar = (GtkWidget*)g_object_get_data (G_OBJECT (canvas), "statusbar");
	gint statusid  = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "drag");
	
	if(flg){
                ActiveFrameWidth=vinfo->GetZfac ();
                //		gnome_canvas_item_show(ActiveFrame);
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), statusid, "channel is active now");
	}else{
                ActiveFrameWidth=0.;
                //		gnome_canvas_item_hide(ActiveFrame);
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), statusid, "inactive");
	}
}

gint ViewControl::canvas_event_cb(GtkWidget *canvas, GdkEvent *event, ViewControl *vc){
	static int dragging=FALSE;
	static GtkWidget *coordpopup=NULL;
	static GtkWidget *coordlab=NULL;
        double mouse_pix_xy[2];
	GtkWidget *frame;
	gchar *mld;
        //	GdkCursor *fleur;

        // undo cairo image translation/scale:
        double zf = vc->vinfo->GetZfac();
        mouse_pix_xy[0] = (event->button.x - (double)(vc->rulewidth+vc->border/zf))/zf;
        mouse_pix_xy[1] = (event->button.y - (double)(vc->rulewidth+vc->border/zf))/zf;
             
        // 1st check if mouse on editable object
        vc->tmp_event = event;     // data for foreach
        vc->tmp_xy = mouse_pix_xy; // data for foreach
        vc->tmp_effected = 0;

        if (!dragging){
                if (vc->tmp_object_op){
                        // g_print ("CANVAS EVENT: grab mode\n");
                        if (!vc->tmp_object_op->check_event (vc->tmp_event, vc->tmp_xy))
                                vc->tmp_object_op = NULL;
                        
                        return FALSE;
                }
                
                g_slist_foreach((GSList*) vc->gobjlist, (GFunc) ViewControl::check_obj_event, vc);

                if (vc->tmp_effected > 0) // handled by object, done. no more action here!
                        return FALSE;
        }
           
	switch (event->type) {
	case GDK_BUTTON_PRESS:
		switch(event->button.button) {
		case 1: 
                        
			if(vc->AddObjFkt){
                                g_object_set_data (G_OBJECT (canvas), "mouse_pix_xy", (gpointer) mouse_pix_xy);
				(*vc->AddObjFkt)(NULL, vc); return TRUE; // Add Obj
                        }
			break;
		case 2: // Show XYZ display
                        // g_print ("BUTTON_PRESS image-pixel XY: %g, %g\n", mouse_pix_xy[0], mouse_pix_xy[1]);
			mld = vc->vinfo->makeXYZinfo (mouse_pix_xy[0], mouse_pix_xy[1], NULL);
			coordpopup = gtk_window_new (GTK_WINDOW_POPUP);
			gtk_window_set_position (GTK_WINDOW (coordpopup), GTK_WIN_POS_MOUSE);
			gtk_container_add (GTK_CONTAINER (coordpopup), frame = gtk_frame_new (NULL));
			gtk_container_add (GTK_CONTAINER (frame),   coordlab = gtk_label_new (mld));
			gtk_widget_show_all (coordpopup);
			g_free(mld);
                        // fleur = gdk_cursor_new(GDK_CROSSHAIR);
                        //  g_object_unref (fleur);
			dragging=TRUE;

			if (vc->event_filter){
				int ix = (int)round (mouse_pix_xy[0]*vc->vinfo->GetQfac());
				int iy = (int)round (mouse_pix_xy[1]*vc->vinfo->GetQfac());
				vc->scan->Pixel2World (ix,iy, vc->CursorXYVt[0], vc->CursorXYVt[1]);
				if (vc->event_filter[0] == 'P'){
					vc->RemoveEventObjects ();
					vc->RescanEventObjects ();
				}
			}
                        break;
                case 3: // do popup
                        gtk_menu_popup_at_pointer (GTK_MENU (vc->v_popup_menu), event);
                        break;
		case 4: if(vc->ZoomQFkt) (*vc->ZoomQFkt)(0,1,vc->ZQFktData); break; // Zoom Out
		case 5: if(vc->ZoomQFkt) (*vc->ZoomQFkt)(1,0,vc->ZQFktData); break; // Zoom In
		}
		break;
		
	case GDK_MOTION_NOTIFY:
                // g_print ("MOTION XY: %g, %g\n", event->button.x, event->button.y);
		if (dragging && (event->motion.state & GDK_BUTTON2_MASK)){
			mld = vc->vinfo->makeXYZinfo (mouse_pix_xy[0], mouse_pix_xy[1], NULL);
			gtk_label_set_text (GTK_LABEL (coordlab), mld);
			g_free(mld);
		}
		break;
		
	case GDK_BUTTON_RELEASE:
		switch(event->button.button){
		case 2:  // remove XYZ display
			gtk_widget_destroy (coordpopup);
			coordpopup=NULL;
			dragging=FALSE;
			break;
		}
		break;
	default: break;
	}
	return FALSE;
}

void ViewControl::CheckRedLine(){
	if(RedLine){
		if (vinfo->sc->RedLineActive)
			RedLine->NewData_redprofile (vinfo->sc, 'r');

		if (vinfo->sc->BlueLineActive && RedLine){
			int ch_next = vinfo->sc->get_channel_id ()+1;
			if (ch_next >= 0 && ch_next < MAX_CHANNELS){
				if (gapp->xsm->scan [ch_next])
					RedLine->NewData_redprofile (gapp->xsm->scan [ch_next], 'b');
			}
		}
		RedLine->UpdateArea();
	}
}


void ViewControl::CheckOptions(){
//	GtkWidget *menushell;
//	GtkWidget *menuitem;
//	gint pos;
	gchar *vmode=NULL;
	
	switch(scan->GetVM()){
	case SCAN_V_DIRECT: vmode=g_strconcat(N_("View"),"/",N_("Direct"), NULL); break;
	case SCAN_V_QUICK:  vmode=g_strconcat(N_("View"),"/",N_("Quick"), NULL); break;
	case SCAN_V_LOG:    vmode=g_strconcat(N_("View"),"/",N_("Logarithmic"), NULL); break;
	case SCAN_V_DIFFERENTIAL:    vmode=g_strconcat(N_("View"),"/",N_("Differential"), NULL); break;
	case SCAN_V_PERIODIC:    vmode=g_strconcat(N_("View"),"/",N_("Periodic"), NULL); break;
	}
	if(vmode){
/*
		menushell = (GtkWidget*)gnome_app_find_menu_pos 
			(
				(GtkWidget*)g_object_get_data (G_OBJECT (canvas), "viewpopup"), 
				vmode, 
				&pos
				);
		if(menushell){
			menuitem = (GtkWidget*)g_list_nth_data(GTK_MENU_SHELL (menushell) -> children, --pos);
			if(menuitem){
				if (!gtk_check_menu_item_get_active  (GTK_CHECK_MENU_ITEM(menuitem)))
					gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), TRUE);
			}
		}
*/
		g_free(vmode);
	}
}

void ViewControl::Activate_window_callback (GtkWindow *window, gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;
	gapp->xsm->ActivateChannel (vc->chno);
}

void ViewControl::Activate_callback (GSimpleAction *simple, GVariant *parameter,
                                     gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;
	gapp->xsm->ActivateChannel(vc->chno);
}

void ViewControl::AutoDisp_callback (GSimpleAction *simple, GVariant *parameter, 
                                     gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;
	gapp->xsm->ActivateChannel(vc->chno);
	gapp->xsm->AutoDisplay();
}

void ViewControl::SetOff_callback (GSimpleAction *simple, GVariant *parameter, 
                                   gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;
        gapp->xsm->SetMode(vc->chno, ID_CH_M_OFF);
}
void ViewControl::SetOn_callback (GSimpleAction *simple, GVariant *parameter, 
                                  gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;
        gapp->xsm->SetMode(vc->chno, ID_CH_M_ON);
}
void ViewControl::SetMath_callback (GSimpleAction *simple, GVariant *parameter, 
                                    gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;
        gapp->xsm->SetMode(vc->chno, ID_CH_M_MATH);
}
void ViewControl::SetX_callback (GSimpleAction *simple, GVariant *parameter, 
                                 gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;
        gapp->xsm->SetMode(vc->chno, ID_CH_M_X);
}

void ViewControl::view_file_openhere_callback (GSimpleAction *simple, GVariant *parameter, 
                                               gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;
	gapp->xsm->ActivateChannel(vc->chno);
	gapp->xsm->load();
}

void ViewControl::view_file_save_callback (GSimpleAction *simple, GVariant *parameter, 
                                           gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;
	gapp->xsm->save(AUTO_NAME_SAVE, NULL, vc->chno);
}

void ViewControl::view_file_saveobjects_callback (GSimpleAction *simple, GVariant *parameter, 
                                                  gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	gchar *oname = g_strconcat(vc->scan->data.ui.basename,"-objects",NULL);
	gchar *fname = gapp->file_dialog("Save Objects, list or .plt (HPGL)", NULL, NULL, oname, "objectsave");
	g_free(oname);
	if (!fname) return;
	vc->objsavestream.open(fname, std::ios::out);
	if (strncmp(fname+strlen(fname)-4,".plt",4))
		vc->SaveObjects();
	else
		vc->SaveObjectsHPGL();
	vc->objsavestream.close();
}


void GetColor (gchar *line, const gchar *tag, gfloat *c){
	gchar *p = strstr (line, tag);
	if (p){
		p += strlen(tag);
		c[0] = atof ( strtok( p, " ,;()"));
		c[1] = atof ( strtok( NULL, " ,;()"));
		c[2] = atof ( strtok( NULL, " ,;()"));
		c[3] = atof ( strtok( NULL, " ,;()"));
	}
}

void GetTuple (gchar *line, const gchar *tag, int c[2]){
	gchar *p = strstr (line, tag);
	if (p){
		p += strlen(tag);
		c[0] = atoi ( strtok( p, " ,;()"));
		c[1] = atoi ( strtok( NULL, " ,;()"));
	}
}

void GetDTuple (gchar *line, const gchar *tag, int c[2][2]){
	gchar *p = strstr (line, tag);
	if (p){
		p += strlen(tag);
		c[0][0] = atoi ( strtok( p, " ,;()"));
		c[0][1] = atoi ( strtok( NULL, " ,;()"));
		c[1][0] = atoi ( strtok( NULL, " ,;()"));
		c[1][1] = atoi ( strtok( NULL, " ,;()"));
	}
}

gfloat GetNumber (gchar *line, const gchar *tag){
	gchar *p = strstr (line, tag);
	if (p){
		p += strlen(tag);
		return atof ( strtok( p, " ,;()"));
	}
	return 0.;
}

gchar *GetString (gchar *line, const gchar *tag){
	gchar *p = strstr (line, tag);
	if (p){
		p += strlen(tag);
		p = strchr (p, '\"') + 1;
		*(strchr (p, '\"')) = 0;
		return g_strdup (p);
	} else return NULL;
}

gchar *GetLabelInfo (std::ifstream &is, gchar **fnt, gfloat *mas, gfloat col[4], int spc[2][2], int sp00[2], int *show){
	gchar line[512];
	gchar *lab = NULL;
	is.getline (line, 512); // label
	gchar *tmp = g_strdup(line);
	lab = g_strdup (GetString (tmp, "Label"));
	g_free (tmp);
	tmp = g_strdup(line);
	*fnt = g_strdup (GetString (tmp, "Font"));
	g_free (tmp);
	tmp = g_strdup(line);
	*mas = GetNumber (tmp, "MarkerSkl");
	g_free (tmp);
	tmp = g_strdup(line);
	GetColor (tmp, "Color",col);
	g_free (tmp);
	tmp = g_strdup(line);
	GetDTuple (tmp, "SpaceTimeOnOff",spc);
	g_free (tmp);
	tmp = g_strdup(line);
	GetTuple (tmp, "SpaceTimeFromUntil",sp00);
	g_free (tmp);
	tmp = g_strdup(line);
	*show = (GetNumber (tmp, "Show") > 0.) ? 1:0;
	g_free (tmp);
	
	return lab;
}

int GetXAngYAng (std::ifstream &is, double *xy, int initial){
	gchar line[64];
	int n;

	n=0;
	// skip NPkte, Info, ... until Coords tag
	while (initial && is.good ()){
		is.getline (line, 64); // read and check
		if (strstr (line, "(NPkte")){
			n = atoi (strtok(line, " ,;(NPkte)"));
			continue;
		}
		if (strstr (line, "(Coords i X Y"))
			break; // found, proceed below
	}
	is.getline (line, 64); // get XY data
        // int i = atoi (strtok(line, " ,;()"));
	strtok(NULL, " ,;()"); // ignore X in pix
	strtok(NULL, " ,;()"); // ignore Y in pix
	xy[0] = atof (strtok(NULL, " ,;()"));
	xy[1] = atof (strtok(NULL, " ,;()"));
	return n;
}

int GetProfileConfig (std::ifstream &is, int *params_pws, int *params_dims){
//  (ProfileActive PathWidthStep (3 1) PathSerDimAllG2d (0 3 1 1)))
	int i=2;
	gchar line[128];
	while (is.good () && i--){
		is.getline (line, 128); // read and check
		if (strstr (line, "))"))
			return 0;
		if (strstr (line, "(ProfileActive PathWidthStep")){
			params_pws[0] = atoi (strtok(line, " ,;(ProfileActive PathWidthStep)"));
			params_pws[1] = atoi (strtok(NULL, " ,;(ProfileActive PathWidthStep)"));
			params_dims[0] = atoi (strtok(NULL, " ,;(PathSerDimAllG2d)"));
			params_dims[1] = atoi (strtok(NULL, " ,;(PathSerDimAllG2d)"));
			params_dims[2] = atoi (strtok(NULL, " ,;(PathSerDimAllG2d)"));
			params_dims[3] = atoi (strtok(NULL, " ,;(PathSerDimAllG2d)"));
			return 1;
		}
	}
	return 0;
}

void ViewControl::view_file_loadobjects_callback (GSimpleAction *simple, GVariant *parameter, 
                                                  gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	gchar *oname = g_strconcat (vc->scan->data.ui.basename,"-objects",NULL);
	gchar *fname = gapp->file_dialog_load ("Load Objects list (not .plt)", NULL, NULL, oname);
	g_free (oname);
	vc->objloadstream.open (fname, std::ios::in);

// very primitive parser of object data
//  I've no idea how to choose from "Object-Name" automatically the right object class 
//  without using a explicit if ( strcpm (..)) chooser list!
// loads only Point,Line,Rect,Circle
	while (vc->objloadstream.good ()){
		gchar line[512];
		gchar *lab = NULL;
		vc->objloadstream.getline (line, 512);
		if (!strncmp (line, "#C Vector Probe Header List --", 30)){

// #C Vector Probe Header List -----------------
// #C # ####	 time[ms]  	 dt[ms]    	 X[Ang]   	 Y[Ang]   	 Z[Ang]    	 Sec
// #       0	          0	          0	          0	 -0.0519132	     328.58	  0	 
// #       1	    92.3733	    92.3733	   -2.64757	   -1.50548	    328.262	 25	 
// ...
	
		       vc->objloadstream.getline (line, 512); // skip header line
		       double arr[7];
		       double xy[2] = {0., 0.};
		       gfloat c[4]  = {0., 0., 1., 1.};
		       gfloat mas   = 0.2;
		       VObject *vo;
		       int spc[2][2] = {{0,0},{0,0}};
		       int sp00[2] = {1,1};
		       std::cout << "Reading Trail to Objetcs..." << std::endl << line << std::endl;
		       while (vc->objloadstream.good ()){
			   vc->objloadstream.getline (line, 512); // skip line end to next
			   std::cout << line << std::endl;
			   if (!strncmp (line, "#C END", 6))
				   break;
			   
			   arr[0] = atof (strtok(line, " ,;()#"));
			   for (int i=1; i<7; ++i)
				   arr[i] = atof (strtok(NULL, " ,;()#"));
//			   std::cout << arr[0] << " t" << arr[1]  << " x" << arr[3]  << " y" << arr[4] << " z" << arr[5]  << " s" << arr[6];
			   xy[0] = arr[3];
			   xy[1] = arr[4];
			   lab = g_strdup_printf ("T%05.0f:%.3fms:Z=%gA", arr[0], arr[1], arr[5]);
//			   lab = g_strdup_printf ("T%05.0f", arr[0]);
			   std::cout << " => " << lab << std::endl;
			   vc->AddObject (vo = new VObPoint (vc->canvas, xy, vc->scan->Pkt2d, FALSE, VOBJ_COORD_ABSOLUT, lab, mas/vc->vinfo->GetZfac ()));
			   g_free (lab); lab=NULL;
//			   vo->set_obj_name ("Trl");
			   vo->set_obj_name ("*Marker:yellow");
			   vo->set_custom_label_font ("Sans 6");
			   vo->set_custom_label_color (c);
			   vo->set_on_spacetime  (sp00[0] ? FALSE:TRUE, spc[0]);
			   vo->set_off_spacetime (sp00[1] ? FALSE:TRUE, spc[1]);
			   vo->show_label (0);
			   vo->remake_node_markers ();
		       }
		} else if (!strncmp (line, "(VObject \"Point\"", 16)
		    ||
		    !strncmp (line, "(VObject \"*Marker:", 18)
			){
			double xy[2];
			gfloat c[4], mas;
			gchar *f, *nm;
			int spc[2][2], sp00[2], s;
			VObject *vo;
			nm = GetString (line, "VObject");
			lab = GetLabelInfo (vc->objloadstream, &f, &mas, c, spc, sp00, &s);
			GetXAngYAng (vc->objloadstream, xy, TRUE);
			XSM_DEBUG(DBG_L2, "Adding Point@xy:" << xy[0] << ", " << xy[1] );
			vc->AddObject (vo = new VObPoint (vc->canvas, xy, vc->scan->Pkt2d, FALSE, VOBJ_COORD_ABSOLUT, lab, mas/vc->vinfo->GetZfac ()));
			vo->set_obj_name (nm);
			vo->set_custom_label_font (f);
			vo->set_custom_label_color (c);
			vo->set_on_spacetime  (sp00[0] ? FALSE:TRUE, spc[0]);
			vo->set_off_spacetime (sp00[1] ? FALSE:TRUE, spc[1]);
			vo->show_label (s);
			vo->remake_node_markers ();
		} else if (!strncmp (line, "(VObject \"Line\"", 15)){
			double xy[2*7];
			gfloat ec[4], c[4], mas;
			gchar *f;
			int n,i;
			int spc[2][2], sp00[2], s;
			int pws[2], pdd[4];
			VObject *vo;
			GetColor (line, "CustomColor", ec);
			lab = GetLabelInfo (vc->objloadstream, &f, &mas, c, spc, sp00, &s);
			n=GetXAngYAng (vc->objloadstream, xy, TRUE);
			for (i=1; i<n && i < 7; ++i)
				GetXAngYAng (vc->objloadstream, xy+2*i, FALSE);
			
			XSM_DEBUG(DBG_L2, "Adding Line@xy:" << xy[0] << ", " << xy[1]
			     << " : " << xy[2] << ", " << xy[3] );
			vc->AddObject (vo = new VObLine (vc->canvas, xy, vc->scan->Pkt2d, FALSE, VOBJ_COORD_ABSOLUT, lab, mas/vc->vinfo->GetZfac ()));
			if (n>2){
				for (i=2; i<n && i < 7; ++i)
					vo->insert_node (&xy[2*i]);
			}
			if (GetProfileConfig (vc->objloadstream, pws, pdd)) {
				    vo->set_profile_path_width (pws[0]); 
				    vo->set_profile_path_step (pws[1]); 
				    vo->set_profile_path_dimension ((MEM2D_DIM) pdd[0]);
				    vo->set_profile_series_dimension ((MEM2D_DIM) pdd[1]);
				    vo->set_profile_series_all (pdd[2]);
				    vo->set_profile_series_pg2d (pdd[3]);
				    vo->show_profile (TRUE);
			}
			
			vo->set_custom_label_font (f);
			vo->set_custom_label_color (c);
			vo->set_custom_element_color (ec);
			vo->set_on_spacetime  (sp00[0] ? FALSE:TRUE, spc[0]);
			vo->set_off_spacetime (sp00[1] ? FALSE:TRUE, spc[1]);
			vo->show_label (s);
			vo->remake_node_markers ();
		} else if (!strncmp (line, "(VObject \"Rectangle\"", 20)){
			double xy[4];
			gfloat ec[4], c[4], mas;
			gchar *f;
			int spc[2][2], sp00[2], s;
			VObject *vo;			
			GetColor (line, "CustomColor", ec);
			lab = GetLabelInfo (vc->objloadstream, &f, &mas, c, spc, sp00, &s);
			GetXAngYAng (vc->objloadstream, xy, TRUE);
			GetXAngYAng (vc->objloadstream, xy+2, FALSE);
			XSM_DEBUG(DBG_L2, "Adding Rectangle@xy:" << xy[0] << ", " << xy[1]
			     << " : " << xy[2] << ", " << xy[3] );
			vc->AddObject (vo = new VObRectangle (vc->canvas, xy, vc->scan->Pkt2d, FALSE, VOBJ_COORD_ABSOLUT, lab, mas/vc->vinfo->GetZfac ()));
			vo->set_custom_label_font (f);
			vo->set_custom_label_color (c);
			vo->set_custom_element_color (ec);
			vo->set_on_spacetime  (sp00[0] ? FALSE:TRUE, spc[0]);
			vo->set_off_spacetime (sp00[1] ? FALSE:TRUE, spc[1]);
			vo->show_label (s);
			vo->remake_node_markers ();
		} else if (!strncmp (line, "(VObject \"Circle\"", 17)){
			double xy[4];
			gfloat ec[4], c[4], mas;
			gchar *f;
			int spc[2][2], sp00[2], s;
			VObject *vo;		
			lab = GetLabelInfo (vc->objloadstream, &f, &mas, c, spc, sp00, &s);
			GetXAngYAng (vc->objloadstream, xy, TRUE);
			GetColor (line, "CustomColor", ec);
			GetXAngYAng (vc->objloadstream, xy+2, FALSE);
			XSM_DEBUG(DBG_L2, "Adding Circle@xy:" << xy[0] << ", " << xy[1]
			     << " : " << xy[2] << ", " << xy[3] );
			vc->AddObject (vo = new VObCircle (vc->canvas, xy, vc->scan->Pkt2d, FALSE, VOBJ_COORD_ABSOLUT, lab, mas/vc->vinfo->GetZfac ()));
			vo->set_custom_label_font (f);
			vo->set_custom_label_color (c);
			vo->set_custom_element_color (ec);
			vo->set_on_spacetime  (sp00[0] ? FALSE:TRUE, spc[0]);
			vo->set_off_spacetime (sp00[1] ? FALSE:TRUE, spc[1]);
			vo->show_label (s);
			vo->remake_node_markers ();
		} else if (!strncmp (line, "(VObject \"Ksys\"", 15)){
			double xy[6];
			gfloat ec[4], c[4], mas;
			gchar *f;
			int spc[2][2], sp00[2], s;
			VObject *vo;			
			GetColor (line, "CustomColor", ec);
			lab = GetLabelInfo (vc->objloadstream, &f, &mas, c, spc, sp00, &s);
			GetXAngYAng (vc->objloadstream, xy, TRUE);
			GetXAngYAng (vc->objloadstream, xy+2, FALSE);
			GetXAngYAng (vc->objloadstream, xy+4, FALSE);
			XSM_DEBUG(DBG_L2, "Adding Ksys@xy:" << xy[0] << ", " << xy[1]
			     << " : " << xy[2] << ", " << xy[3] );
			vc->AddObject (vo = new VObKsys (vc->canvas, xy, vc->scan->Pkt2d, FALSE, VOBJ_COORD_ABSOLUT, lab, mas/vc->vinfo->GetZfac ()));
			vo->set_custom_label_font (f);
			vo->set_custom_label_color (c);
			vo->set_custom_element_color (ec);
			vo->set_on_spacetime  (sp00[0] ? FALSE:TRUE, spc[0]);
			vo->set_off_spacetime (sp00[1] ? FALSE:TRUE, spc[1]);
			vo->show_label (s);
			vo->remake_node_markers ();
		}
		if (lab) g_free(lab);
		lab = NULL;
	}
	vc->objloadstream.close ();
}

void ViewControl::view_file_save_as_callback (GSimpleAction *simple, GVariant *parameter, 
                                              gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;
	gapp->xsm->save(MANUAL_SAVE_AS, NULL, vc->chno);
}

void ViewControl::view_file_saveimage_callback (GSimpleAction *simple, GVariant *parameter, 
                                                gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;

	gchar *imgname;
	gchar *suggest = g_strdup_printf ("%s-snap.png", vc->scan->data.ui.name);

        cairo_surface_t *surface;
        cairo_t *cr;
        cairo_status_t status;

	imgname = gapp->file_dialog("Save Canvas as png or svg file", NULL, "*.png", suggest);
	g_free (suggest);

	if (imgname == NULL || strlen(imgname) < 5) 
		return;

        int png=0;

	if (strncasecmp (imgname+strlen(imgname)-3,".svg", 3)==0){
#if 0
#ifdef CAIRO_HAS_SVG_SURFACE
                surface = cairo_svg_surface_create (imgname, (double)vc->npx, (double)vc->npy);
                cairo_svg_surface_restrict_to_version (surface, CAIRO_SVG_VERSION_1_2);
#else
                g_print ("Sorry -- CAIRO_HAS_SVG_SURFACE not defined/not available.\n");
                return;
#endif
        } else if (strncasecmp (imgname+strlen(imgname)-3,".pdf", 3)==0){
#ifdef CAIRO_HAS_PDF_SURFACE
                surface = cairo_pdf_surface_create (imgname, (double)vc->npx, (double)vc->npy);
#else
                g_print ("Sorry -- CAIRO_HAS_PDF_SURFACE not defined/not available.\n");
                return;
#endif
#else
                g_print ("Sorry -- CAIRO SVG/PDF_SURFACE is not available.\n");
                return;
#endif
        } else {
                surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, vc->npx, vc->npy);
                png=1;
        }

        cr = cairo_create (surface);
        //        cairo_scale (cr, IMAGE_DPI/72.0, IMAGE_DPI/72.0);
        
        double zf = vc->vinfo->GetZfac();
        cairo_scale (cr, zf, zf);
        cairo_save (cr);

        // 1) draw Frame -- not here
        // 2) draw Image and red line via ShmImage2D
	vc->ximg->draw_callback (cr);

        // 3) draw Objects and Events
        vc->DrawObjects (cr);

        cairo_restore (cr);

        status = cairo_status(cr);
        if (status)
                printf("%s\n", cairo_status_to_string (status));
        
        cairo_destroy (cr);

	if (png){
                g_print ("Cairo save scan view to png: '%s'\n", imgname);
                status = cairo_surface_write_to_png (surface, imgname);
                if (status)
                        printf("%s\n", cairo_status_to_string (status));
        } else {
                cairo_surface_flush (surface);
                cairo_surface_finish (surface);
                g_print ("Cairo save scan view to sng: '%s'\n", imgname);
        }
        
        cairo_surface_destroy (surface);
}

void ViewControl::view_file_getinfo_callback (GSimpleAction *simple, GVariant *parameter, 
                                              gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;

	gapp->xsm->ActivateChannel(vc->chno);
	XSM_DEBUG(DBG_L2, gapp->xsm->ActiveScan->data.ui.name );
	gapp->CallGetNCInfoPlugin (gapp->xsm->ActiveScan->data.ui.name);
}

void ViewControl::view_file_print_callback (GSimpleAction *simple, GVariant *parameter, 
                                            gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;

	gapp->xsm->ActivateChannel(vc->chno);
        //	gapp->file_print_callback(widget, NULL);
        XSM_DEBUG(DBG_L2, "VIEWCONTROL FILE PRINT CALLBACK!\n" );
}

void ViewControl::view_file_kill_callback (GSimpleAction *simple, GVariant *parameter, 
                                           gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;

	gapp->xsm->SetMode(vc->chno, ID_CH_M_OFF);
}

void ViewControl::view_edit_copy_callback (GSimpleAction *simple, GVariant *parameter, 
                                           gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0){
		gapp->xsm->ActivateFreeChannel ();
		gapp->xsm->GetFromMem2d (vc->scan->mem2d);
		return;
	}

	gapp->xsm->ActivateChannel(vc->chno);
	gapp->xsm->MathOperation(CopyScan);
}

void ViewControl::view_edit_crop_callback (GSimpleAction *simple, GVariant *parameter, 
                                           gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;

	gapp->xsm->ActivateChannel(vc->chno);
	gapp->xsm->MathOperation(CropScan);
}

void ViewControl::view_edit_zoomin_callback (GSimpleAction *simple, GVariant *parameter, 
                                             gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;

	gapp->xsm->ActivateChannel(vc->chno);
	gapp->xsm->MathOperation(ZoomInScan);
}

void ViewControl::view_edit_zoomout_callback (GSimpleAction *simple, GVariant *parameter, 
                                              gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	if (vc->chno < 0) return;

	gapp->xsm->ActivateChannel(vc->chno);
	gapp->xsm->MathOperation(ZoomOutScan);
}

void ViewControl::view_tool_all2locmax_callback (GSimpleAction *simple, GVariant *parameter, 
                                                 gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	vc->MoveAllObjects2LocMax();
}

void ViewControl::view_tool_removeall_callback (GSimpleAction *simple, GVariant *parameter, 
                                                gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	vc->RemoveObjects();
	vc->scan->PktVal=0;
}

void ViewControl::view_tool_labels_callback (GSimpleAction *action, GVariant *parameter, 
                                             gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));
                
        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);

        vc->CheckAllObjectsLabels (g_variant_get_boolean (new_state));
}

void ViewControl::view_tool_marker_group_radio_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data) { 
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_string (g_variant_get_string (parameter, NULL));
                
        g_print ("TOOL-MARKER Radio action %s activated, state changes from %s to %s\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_string (old_state, NULL),
                 g_variant_get_string (new_state, NULL));

        vc->SetMarkerGroup (g_variant_get_string (new_state, NULL));
        
        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);
}

void ViewControl::SetMarkerGroup (const gchar *mgcolor){
	if (mgcolor){
		switch (*mgcolor){
			case 'r': marker_group=0; break;
			case 'g': marker_group=1; break;
			case 'b': marker_group=2; break;
			case 'y': marker_group=3; break;
			case 'm': marker_group=4; break;
			case 'c': marker_group=5; break;
			case '0': marker_counter [marker_group]=0; break;
			case '?': 
				std::cout << "Marker Counts:" << std::endl
					  << "#0# Red......: " << marker_counter[0] << std::endl
					  << "#1# Green....: " << marker_counter[1] << std::endl
					  << "#2# Blue.....: " << marker_counter[2] << std::endl
					  << "#3# Yellow...: " << marker_counter[3] << std::endl
					  << "#4# Magenta..: " << marker_counter[4] << std::endl
					  << "#5# Cyan.....: " << marker_counter[5] << std::endl
					  << "===========================" << std::endl
					  << "### Total....: " << (marker_counter[5]+marker_counter[4]+marker_counter[3]+marker_counter[2]+marker_counter[1]+marker_counter[0]) << std::endl
					  << "Current Group: " << marker_group
					  << std::endl << std::endl;
				break;
		}
	} else {
		marker_group=0;
		marker_counter[0]=0;
		marker_counter[1]=0;
		marker_counter[2]=0;
		marker_counter[3]=0;
		marker_counter[4]=0;
		marker_counter[5]=0;
	}
}

// ---------------

#define MAKE_VOB_DEFAULTS(TYPE)  new TYPE(vc->canvas, xy, vc->scan->Pkt2d, FALSE, VOBJ_COORD_FROM_MOUSE, NULL, OB_MARKER_SCALE)
#define MAKE_VOB_DEFAULTS_WMS(TYPE, MS)  new TYPE(vc->canvas, xy, vc->scan->Pkt2d, FALSE, VOBJ_COORD_FROM_MOUSE, NULL, MS)
#define MAKE_VOB_DEFAULTS_SHOW(TYPE)  new TYPE(vc->canvas, xy, vc->scan->Pkt2d, TRUE, VOBJ_COORD_FROM_MOUSE, NULL, OB_MARKER_SCALE)

void ViewControl::view_tool_addpoint (GtkWidget *widget, ViewControl *vc){
	double xy[2] = {0.,0.};
	vc->AddObject (MAKE_VOB_DEFAULTS (VObPoint));
	vc->scan->PktVal=1;
}
void ViewControl::view_tool_addmarker (GtkWidget *widget, ViewControl *vc){
	double xy[2] = {0.,0.};
	gchar *mgn = vc->MakeMarkerLabel ();
	VObPoint *vp;
	vc->AddObject (vp = new VObPoint (vc->canvas, xy, vc->scan->Pkt2d, FALSE, VOBJ_COORD_FROM_MOUSE, mgn, 0.5));
	g_free (mgn);
	switch (vc->marker_group){
		case 0: vp->set_obj_name ("*Marker:red"); break;
		case 1: vp->set_obj_name ("*Marker:green"); break;
		case 2: vp->set_obj_name ("*Marker:blue"); break;
		case 3: vp->set_obj_name ("*Marker:yellow"); break;
		case 4: vp->set_obj_name ("*Marker:magenta"); break;
		case 5: vp->set_obj_name ("*Marker:cyan"); break;
	}
	vc->scan->PktVal=1;
}
void ViewControl::view_tool_addshowpoint (GtkWidget *widget, ViewControl *vc){
	double xy[2] = {0.,0.};
	vc->AddObject (MAKE_VOB_DEFAULTS_SHOW (VObPoint));
	vc->scan->PktVal=1;
}
void ViewControl::view_tool_addline (GtkWidget *widget, ViewControl *vc){
	double xy[4] = {0.,0.,15.,15.};
	vc->AddObject (MAKE_VOB_DEFAULTS (VObLine));
	vc->scan->PktVal=2;
}
void ViewControl::view_tool_addpolyline (GtkWidget *widget, ViewControl *vc){
	double xy[13] = {6., 0.,0., 10.,10., 20.,20., 30.,30., 40.,40., 50.,50. };
	vc->scan->realloc_pkt2d (6);
	vc->AddObject (MAKE_VOB_DEFAULTS (VObPolyLine));
	vc->scan->PktVal=6;
}
void ViewControl::view_tool_addksys (GtkWidget *widget, ViewControl *vc){
	double xy[6] = {60.,0.,0.,0.,0.,60.};
	vc->AddObject (MAKE_VOB_DEFAULTS_WMS (VObKsys, 4.));
	vc->scan->PktVal=3;
}
void ViewControl::view_tool_addparabel (GtkWidget *widget, ViewControl *vc){
	double xy[6] = {0.,0.,25.,30.,50.,50.};
	vc->AddObject(new VObParabel(vc->canvas, xy, vc->scan->Pkt2d));
	vc->scan->PktVal=3;
}
void ViewControl::view_tool_addshowline (GtkWidget *widget, ViewControl *vc){
	double xy[4] = {0.,0.,15.,15.};
	vc->AddObject (MAKE_VOB_DEFAULTS_SHOW (VObLine));
	vc->scan->PktVal=2;
}
void ViewControl::view_tool_addrectangle (GtkWidget *widget, ViewControl *vc){
	double xy[4] = {0.,0.,15.,15.};
	vc->AddObject (MAKE_VOB_DEFAULTS (VObRectangle));
	vc->scan->PktVal=2;
}
void ViewControl::view_tool_addcircle (GtkWidget *widget, ViewControl *vc){
	double xy[4] = {0.,0.,15.,15.};
	vc->AddObject (MAKE_VOB_DEFAULTS (VObCircle));
	vc->scan->PktVal=2;
}
void ViewControl::view_tool_addshowcircle (GtkWidget *widget, ViewControl *vc){
	double xy[4] = {0.,0.,15.,15.};
	vc->AddObject (MAKE_VOB_DEFAULTS_SHOW (VObCircle));
	vc->scan->PktVal=2;
}

void ViewControl::view_object_mode_radio_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data) { 
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_string (g_variant_get_string (parameter, NULL));
                
        g_print ("OBJECT-MODE Radio action %s activated, state changes from %s to %s\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_string (old_state, NULL),
                 g_variant_get_string (new_state, NULL));

        if (!strcmp (g_variant_get_string (new_state, NULL), "rectangle")){
                vc->AddObjFkt = ViewControl::view_tool_addrectangle;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "point")){
                vc->AddObjFkt = ViewControl::view_tool_addpoint;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "marker")){
                vc->AddObjFkt = ViewControl::view_tool_addmarker;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "line")){
                vc->AddObjFkt = ViewControl::view_tool_addline;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "circle")){
                vc->AddObjFkt = ViewControl::view_tool_addcircle;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "polyline")){
                vc->AddObjFkt = ViewControl::view_tool_addpolyline;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "ksys")){
                vc->AddObjFkt = ViewControl::view_tool_addksys;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "parabel")){
                vc->AddObjFkt = ViewControl::view_tool_addparabel;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "point-show")){
                vc->AddObjFkt = ViewControl::view_tool_addshowpoint;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "line-show")){
                vc->AddObjFkt = ViewControl::view_tool_addshowline;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "circle-show")){
                vc->AddObjFkt = ViewControl::view_tool_addshowcircle;
        } else {
                vc->AddObjFkt = NULL; // "disable"
        }
        
        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);
}

void ViewControl::view_tool_mvprop_radius_radio_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data) { 
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_string (g_variant_get_string (parameter, NULL));
                
        g_print ("MVPROP-MODE RADIUS Radio action %s activated, state changes from %s to %s\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_string (old_state, NULL),
                 g_variant_get_string (new_state, NULL));

        vc->local_radius = atoi (g_variant_get_string (new_state, NULL));
        g_print ("r=%d\n",  vc->local_radius);
        
        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);
}

// ---------------

void ViewControl::view_view_set_view_mode_radio_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data) { 
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        gint mode=SCAN_V_QUICK; // fall back
        
        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_string (g_variant_get_string (parameter, NULL));
                
        g_print ("VIEW-MODE Radio action %s activated, state changes from %s to %s\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_string (old_state, NULL),
                 g_variant_get_string (new_state, NULL));


        if (!strcmp (g_variant_get_string (new_state, NULL), "quick")){
                mode = SCAN_V_QUICK;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "direct")){
                mode = SCAN_V_DIRECT;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "hilit")){
                mode = SCAN_V_HILITDIRECT;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "plane")){
                mode = SCAN_V_PLANESUB;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "horizontal")){
                mode = SCAN_V_HORIZONTAL;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "periodic")){
                mode = SCAN_V_PERIODIC;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "log")){
                mode = SCAN_V_LOG;
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "diff")){
                mode = SCAN_V_DIFFERENTIAL;
        }

   	if (vc->chno < 0){
                vc->scan->SetVM(mode);
        } else {
                gapp->xsm->ActivateChannel(vc->chno);
		gapp->xsm->SetVM(mode);
        }
      
        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);
}


void ViewControl::view_view_x_linearize_callback (GSimpleAction *action, GVariant *parameter, 
                                                  gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));
                
        g_print ("Toggle action %s activated, state changes from %d to %d\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_boolean (old_state),
                 g_variant_get_boolean (new_state));

        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);


	if (g_variant_get_boolean (new_state)){
		vc->scan->x_linearize (TRUE);
	} else {
		vc->scan->x_linearize (FALSE);
	}
	if (vc->chno < 0) return;
	gapp->xsm->ActivateChannel(vc->chno);
	gapp->xsm->AutoDisplay();
}

void ViewControl::view_view_redline_callback (GSimpleAction *action, GVariant *parameter, 
                                              gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));
                
        g_print ("Toggle action %s activated, state changes from %d to %d\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_boolean (old_state),
                 g_variant_get_boolean (new_state));

        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);


	if (g_variant_get_boolean (new_state)){
		vc->scan->RedLineActive=TRUE;
		if(!vc->RedLine){
			gchar *tmp = g_strdup_printf ("Red Line Ch%d" ,vc->scan->get_channel_id ()+1);
			vc->RedLine = new ProfileControl(tmp);
			g_free (tmp);
		}
	}else{
		vc->scan->RedLineActive=FALSE;
		if(vc->RedLine && !vc->scan->BlueLineActive){
			delete vc->RedLine;
			vc->RedLine=NULL;
		}
	}
}

void ViewControl::view_view_blueline_callback (GSimpleAction *action, GVariant *parameter, 
                                               gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));
                
        g_print ("Toggle action %s activated, state changes from %d to %d\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_boolean (old_state),
                 g_variant_get_boolean (new_state));

        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);


	if (g_variant_get_boolean (new_state))
		vc->scan->BlueLineActive=TRUE;
	else
		vc->scan->BlueLineActive=FALSE;
}

void ViewControl::view_view_autozoom_callback (GSimpleAction *action, GVariant *parameter, 
                                               gpointer user_data){
        //ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));
                
        g_print ("Toggle action %s activated, state changes from %d to %d\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_boolean (old_state),
                 g_variant_get_boolean (new_state));

        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);


	if (g_variant_get_boolean (new_state))
		gapp->xsm->ZoomFlg = (gapp->xsm->ZoomFlg & ~VIEW_ZOOM) | VIEW_ZOOM;
	else
		gapp->xsm->ZoomFlg &= ~VIEW_ZOOM;
}

void ViewControl::view_view_color_callback (GSimpleAction *action, GVariant *parameter, 
                                            gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;

        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));
                
        g_print ("Toggle action %s activated, state changes from %d to %d\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_boolean (old_state),
                 g_variant_get_boolean (new_state));

        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);


	if (g_variant_get_boolean (new_state)){
		vc->scan->view->color_mode  (USER_FALSE_COLOR);
//		SET_FLAG(gapp->xsm->ZoomFlg, VIEW_PALETTE);
//		SET_FLAG(gapp->xsm->ZoomFlg, VIEW_COLOR);
	}else{
		vc->scan->view->color_mode (DEFAULT_GREY);
//		CLR_FLAG(gapp->xsm->ZoomFlg, VIEW_PALETTE);
//		CLR_FLAG(gapp->xsm->ZoomFlg, VIEW_COLOR);
	}
	if (vc->chno < 0) return;
	gapp->xsm->ActivateChannel(vc->chno);
	gapp->xsm->AutoDisplay();
	gapp->xsm->AutoDisplay();
}

void ViewControl::view_view_color_rgb_callback (GSimpleAction *action, GVariant *parameter, 
                                                gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));
                
        g_print ("Toggle action %s activated, state changes from %d to %d\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_boolean (old_state),
                 g_variant_get_boolean (new_state));

        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);


	if (g_variant_get_boolean (new_state))
		vc->scan->view->color_mode (NATIVE_4L_RGBA);
	else
		vc->scan->view->color_mode (DEFAULT_GREY);
}

void ViewControl::view_view_coordinate_mode_radio_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_string (g_variant_get_string (parameter, NULL));
                
        g_print ("ZOOM-FIX Radio action %s activated, state changes from %s to %s\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_string (old_state, NULL),
                 g_variant_get_string (new_state, NULL));

        if (!strcmp (g_variant_get_string (new_state, NULL), "absolute")){
		vc->vinfo->SetCoordMode (SCAN_COORD_ABSOLUTE);
		vc->vinfo->SetPixelUnit (FALSE);
		vc->vinfo->ChangeXYUnit (NULL);
		vc->vinfo->ChangeZUnit (NULL);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "relative")){
		vc->vinfo->SetCoordMode (SCAN_COORD_RELATIVE);
		vc->vinfo->SetPixelUnit (FALSE);
		vc->vinfo->ChangeXYUnit (NULL);
		vc->vinfo->ChangeZUnit (NULL);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "pixels")){
		vc->vinfo->SetCoordMode (SCAN_COORD_RELATIVE);
		vc->vinfo->SetPixelUnit (TRUE);
		vc->vinfo->ChangeXYUnit (gapp->xsm->Unity);
		vc->vinfo->ChangeZUnit (gapp->xsm->Unity);
        }

        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);
}

void ViewControl::view_view_coord_time_callback (GSimpleAction *action, GVariant *parameter, 
                                                 gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));
                
        g_print ("Toggle action %s activated, state changes from %d to %d\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_boolean (old_state),
                 g_variant_get_boolean (new_state));

        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);


	if (g_variant_get_boolean (new_state))
		vc->vinfo->EnableTimeDisplay ();
	else
		vc->vinfo->EnableTimeDisplay (FALSE);
}


void ViewControl::view_view_zoom_fix_radio_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_string (g_variant_get_string (parameter, NULL));
                
        g_print ("ZOOM-FIX Radio action %s activated, state changes from %s to %s\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_string (old_state, NULL),
                 g_variant_get_string (new_state, NULL));

        if (!strcmp (g_variant_get_string (new_state, NULL), "zoomfactor-10x")){
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(10,1,vc->ZQFktData);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "zoomfactor-5x")){
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(5,1,vc->ZQFktData);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "zoomfactor-4x")){
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(4,1,vc->ZQFktData);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "zoomfactor-3x")){
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(3,1,vc->ZQFktData);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "zoomfactor-2x")){
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(2,1,vc->ZQFktData);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "zoomfactor-1x")){
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(1,1,vc->ZQFktData);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "zoomfactor-1by2")){
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(1,2,vc->ZQFktData);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "zoomfactor-1by3")){
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(1,3,vc->ZQFktData);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "zoomfactor-1by4")){
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(1,4,vc->ZQFktData);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "zoomfactor-1by5")){
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(1,5,vc->ZQFktData);
        } else if (!strcmp (g_variant_get_string (new_state, NULL), "zoomfactor-1by10")){
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(1,10,vc->ZQFktData);
        } else {
                if(vc->ZoomQFkt) (*vc->ZoomQFkt)(1,1,vc->ZQFktData); // shall be auto -- TDB!!!!
        }

        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);
}

// Events control
void ViewControl::events_probe_callback (GSimpleAction *simple, GVariant *parameter, 
                                         gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	vc->RemoveEventObjects ();

	int state;
        state = g_variant_get_int32 (parameter);

	if (state)
		vc->SetEventFilter ("P",0);
	else
		vc->SetEventFilter ("-",0);

	vc->RescanEventObjects ();
}
void ViewControl::events_user_callback (GSimpleAction *simple, GVariant *parameter, 
                                        gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	vc->RemoveEventObjects ();

	int state;
        state = g_variant_get_int32 (parameter);

	if (state)
		vc->SetEventFilter ("U",1);
	else
		vc->SetEventFilter ("-",1);

	vc->RescanEventObjects ();
}

void ViewControl::events_labels_callback (GSimpleAction *simple, GVariant *parameter, 
                                          gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	vc->SetEventLabels (g_variant_get_int32 (parameter) ? TRUE:FALSE);
}

void ViewControl::events_verbose_callback (GSimpleAction *simple, GVariant *parameter, 
                                           gpointer user_data){
        // ViewControl *vc = (ViewControl *) user_data;
}

void ViewControl::events_remove_callback (GSimpleAction *simple, GVariant *parameter, 
                                          gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	vc->RemoveEventObjects ();
	vc->scan->mem2d->RemoveScanEvents ();
}

void ViewControl::events_update (){
	if (event_filter)
		RescanEventObjects ();
}

void ViewControl::indicators_remove_callback (GSimpleAction *simple, GVariant *parameter, 
                                              gpointer user_data){
        ViewControl *vc = (ViewControl *) user_data;
	vc->RemoveIndicators ();
}



// ======================================== Object Cbs

void ViewControl::obj_remove_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	vc->scan->PktVal=0;
	vc->remove_obj(vo, vc);
	vc->gobjlist = g_slist_remove((GSList*) vc->gobjlist, vo);
}

void ViewControl::obj_properties_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	vo->properties ();
}

void ViewControl::obj_dump_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	vc->scan->dump_object_data (vc->scan->find_object (vo->obj_id ()));
}

void ViewControl::obj_reset_counter_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	// VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	vc->SetMarkerGroup ("0");
}

void ViewControl::obj_show_counter_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	// VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	vc->SetMarkerGroup ("?");
}

void ViewControl::side_pane_callback (GtkWidget *widget, gpointer user_data) {
        ViewControl *vc = (ViewControl *) user_data; 
        vc->setup_side_pane (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
}

void ViewControl::tip_follow_callback (GtkWidget *widget, gpointer user_data) {
        ViewControl *vc = (ViewControl *) user_data; 
        vc->tip_follow_control (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
}

void ViewControl::side_pane_action_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data) {
        ViewControl *vc = (ViewControl *) user_data; 
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (simple));
        new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));
                
        vc->setup_side_pane (g_variant_get_boolean (new_state)); 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (vc->side_pane_control), g_variant_get_boolean (new_state));

        g_simple_action_set_state (simple, new_state);
        g_variant_unref (old_state);
}
 
void ViewControl::close_side_pane_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data) { 
        //        ViewControl *vc = (ViewControl *) user_data; vc->setup_side_pane (FALSE); 
}


void ViewControl::obj_setoffset_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	vo->set_offset ();
}

void ViewControl::obj_global_ref_point_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	vo->set_global_ref ();
}

void ViewControl::obj_getcoords_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	vo->SetUpScan ();
}

void ViewControl::obj_go_locmax_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){  
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	//  vo->GoLocMax(((ViewControl*)g_object_get_data (G_OBJECT (widget), "ViewControl"))->local_radius);
	vo->GoLocMax ();
}

void ViewControl::obj_follow_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data){  
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));
                
        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);

        if (g_variant_get_boolean (new_state))
		vo->follow_on ();
	else
		vo->follow_off ();
}

void ViewControl::obj_addnode_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	vo->AddNode ();
}

void ViewControl::obj_delnode_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	vo->DelNode ();
}

void ViewControl::obj_event_dump_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	// VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	if(vc->GetActiveScanEvent ())
		(vc->GetActiveScanEvent ())->print ();
}

// handle Event Objects
void ViewControl::obj_event_use_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
	if (vo)
		if (vo->get_scan_event())
			(vo->get_scan_event())->print ();
}

void ViewControl::obj_event_open_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	// ViewControl *vc = (ViewControl *) user_data;
	// VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
}




void ViewControl::obj_event_save_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	ViewControl *vc = (ViewControl *) user_data;
	VObject *vo = (VObject*)g_object_get_data (G_OBJECT (vc->canvas), "VObject");
        GtkWidget *filew;

        filew = gtk_file_chooser_dialog_new ("Save File",
                                             GTK_WINDOW (gtk_widget_get_toplevel (vc->canvas)), // GTK_WINDOW(widget),
                                             GTK_FILE_CHOOSER_ACTION_SAVE,
                                             _("_Cancel"), GTK_RESPONSE_CANCEL,
                                             _("_Save"), GTK_RESPONSE_ACCEPT,
                                             NULL);

        gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (filew), TRUE);

        if (gtk_dialog_run (GTK_DIALOG(filew)) == GTK_RESPONSE_ACCEPT)
                if (vo)
                        if (vo->get_scan_event()){
                                vo->get_scan_event()->saveto=(gchar*)gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filew));
                                (vo->get_scan_event())->save ();
                        }

        gtk_widget_destroy (filew);
}



void ViewControl::update_trace (double *xy, int len){
	double *nxy = new double[1+2*len];
	nxy[0] = len;
	memcpy (&nxy[1], xy, 2*len*sizeof(double));
	if (v_trace){
		v_trace->Change (nxy);
	}else{
		v_trace = new VObTrace(canvas, nxy, scan->Pkt2d, FALSE, VOBJ_COORD_ABSOLUT, "Tip Trace");
	}
	delete[] nxy;
	v_trace->Update ();
}

void ViewControl::remove_trace (){
	if (v_trace){
		delete v_trace;
		v_trace = NULL;
	}
}

void ViewControl::osd_on_toggle_callback (GtkWidget *widget, ViewControl *vc){
	gint pos = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "OSD_POS"));
	if (vc->osd_item[pos]){
		int spt[2];
		spt[0] = 0;
		spt[1] = 0;
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))){
			spt[0] = vc->scan->data.display.vlayer;
			spt[1] = vc->scan->data.display.vframe;
			gchar *spct = g_strdup_printf ("%d:%d", spt[1]+1, spt[0]+1);
			vc->osd_item[pos]->set_on_spacetime (TRUE, spt);
			gtk_button_set_label (GTK_BUTTON (widget), spct);
			g_free (spct);
		} else {
			vc->osd_item[pos]->set_on_spacetime (FALSE, spt);
			gtk_button_set_label (GTK_BUTTON (widget), "0");
		}
	}
}
void ViewControl::osd_off_toggle_callback (GtkWidget *widget, ViewControl *vc){
	gint pos = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "OSD_POS"));
	if (vc->osd_item[pos]){
		int spt[2];
		spt[0] = 0;
		spt[1] = 0;
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))){
			spt[0] = vc->scan->data.display.vlayer;
			spt[1] = vc->scan->data.display.vframe;
			gchar *spct = g_strdup_printf ("%d:%d", spt[1]+1, spt[0]+1);
			vc->osd_item[pos]->set_off_spacetime (TRUE, spt);
			gtk_button_set_label (GTK_BUTTON (widget), spct);
			g_free (spct);
		} else {
			vc->osd_item[pos]->set_off_spacetime (FALSE, spt);
			gtk_button_set_label (GTK_BUTTON (widget), "00");
		}
	}
}

void ViewControl::osd_toggle_callback (GtkWidget *widget, ViewControl *vc){
        guchar *array;
        gsize n_stores = OSD_MAX;
        array = g_new (guchar, n_stores);

        gint pos = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "OSD_POS"));
	if (pos >= OSD_MAX) return;
	vc->osd_item_enable[pos] = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	if (vc->osd_item_enable[pos]){
		gchar *ly_info = vc->scan->mem2d->get_layer_information (pos);
		if (ly_info){
			double x,y;
			gchar *varx = g_strdup_printf ("osd_x%02d", pos);
			gchar *vary = g_strdup_printf ("osd_y%02d", pos);
			vc->set_osd (ly_info, pos);
			vc->osd_item[pos] -> show_label (1);
			g_free (ly_info);
			vc->osd_item[pos] -> obj_get_xy_i (0, x, y);
			x-=vc->scan->data.s.x0; x /= vc->scan->data.s.rx/2; // make relative
			y-=vc->scan->data.s.y0; y /= vc->scan->data.s.ry/2;
                        // gsettings!!!
                        // ==> <key name="osd-position" type="a(iii)">
                        //			xrm.Put (varx, x);
                        //			xrm.Put (vary, -y);
                        //			xrm.Put (flag, vc->osd_item_enable[pos]);
			g_free (varx);
			g_free (vary);
		}
	} else {
		if (vc->osd_item[pos]){
			vc->osd_item[pos] -> show_label (0);
                        //			xrm.Put (flag, vc->osd_item_enable[pos]);
		}
	}

        for (gsize i=0; i<n_stores; ++i)
                array[i] = vc->osd_item_enable[i] ? true : false;
        
        GVariant *storage = g_variant_new_fixed_array (g_variant_type_new ("b"), array, n_stores, sizeof (guchar));
        g_settings_set_value (vc->view_settings, "osd-enable", storage);

        // g_free array, storgae ????
}

void ViewControl::set_osd (gchar *osd_text, int pos){
	gchar *ot = g_strdup (osd_text);

	if (pos < OSD_MAX){
		if (osd_entry[pos])
			gtk_entry_set_text (GTK_ENTRY (osd_entry[pos]), ot);
		g_free (ot);
		ot = scan->mem2d->get_layer_information_osd (pos);
		if (osd_item_enable[pos])
			if (osd_item[pos]){
				int spt[2];
				spt[0] = scan->data.display.vlayer;
				spt[1] = scan->data.display.vframe;
				osd_item[pos] -> obj_text (ot);
				osd_item[pos] -> set_spacetime (spt);
				osd_item[pos] -> show_label ();
				osd_item[pos] -> Update ();
			}else{
                                //	XsmRescourceManager xrm("App_View_OSD");
				double xy[2];
				gchar *varx = g_strdup_printf ("osd_x%02d", pos);
				gchar *vary = g_strdup_printf ("osd_y%02d", pos);
                                // gsettings!!!
                                // ==> <key name="osd-position" type="a(iii)">
                                //				xrm.Get (varx, &xy[0], "10."); // relative to size now: +/-1 for left/right
                                //				xrm.Get (vary, &xy[1], "10.");
				g_free (varx);
				g_free (vary);
				if (fabs (xy[0]) > 1. || fabs (xy[1]) > 1.){
					xy[0] =  2.*((pos%2)-0.5) * 0.7 + 0.25;
					xy[1] = -2.*(((pos/2)%2)-0.5) * (0.9 - (0.2*((pos/4)%4)));
				} 
				xy[0] *= scan->data.s.rx/2; xy[0]+=scan->data.s.x0;
				xy[1] *= scan->data.s.ry/2; xy[1]+=scan->data.s.y0;
				osd_item[pos] = new VObPoint (canvas, xy, scan->Pkt2d, FALSE, VOBJ_COORD_ABSOLUT, ot, 0.);
				xy[0]=xy[1]=0.;
				osd_item[pos] -> set_osd_style (TRUE);
				osd_item[pos] -> set_label_offset (xy);
				osd_item[pos] -> show_label (1);
			}
		else{
			if (osd_item[pos])
				osd_item[pos] -> show_label (0);
		}
	}
	g_free (ot);
}

