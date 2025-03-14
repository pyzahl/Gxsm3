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

#ifndef GAPPSERVICE_H
#define GAPPSERVICE_H

#include <iostream>
#include <fstream>

#include <config.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>

#include "meldungen.h"
#include "unit.h"
#include "pcs.h"

typedef struct _Gxsm3appWindow         Gxsm3appWindow;
#define PYREMOTE_CHECK_HOOK_KEY_PREFIX "PyRemote:action([UN]CHECK-"
#define PYREMOTE_CHECK_HOOK_KEY(KEY)   PYREMOTE_CHECK_HOOK_KEY_PREFIX KEY")"

// Used Ressource Types
typedef enum
	{
		RES_BOOL, RES_STRING, RES_INT, RES_FLOAT, RES_IGNORE_INFO, RES_ENDE
	} RES_TYPE;


// first startup behaviour
typedef enum
        {
                RES_LEVEL_AUTO, RES_LEVEL_ASKFOR, RES_LEVEL_ASKFOR_DEPEND, RES_LEVEL_IGNORE, RES_LEVEL_INFOTEXT
        } RES_LEVEL;

// Resource entry descriptor
typedef struct
{
        RES_TYPE type;          /* RES_BOOL, ...                        */
        const gchar *path;      /* resource path/name                   */
        const gchar *vdefault;  /* resource default                     */
        void *var;              /* address for the variable             */
        const gchar **options;  /* NULL, or list of option strings      */
        const gchar *group;     /* Group number, eg. page no used by folder */
        const gchar *comment;   /* comment: explanation for user        */
        RES_LEVEL level;        /* how to handle by first startup       */
        GtkWidget *entry;       /* Edit Entry */
        gchar *tmp;             /* temporary variable storage as text */
        gint (*depend_func)();  /* to figure out dependencies for asking */
} RES_ENTRY;

typedef void (*GappBrowseFunc)(gchar *fname, gpointer user_data);

// for non bp compatibility/backporting
GtkWidget* check_button_remote_enabled (const gchar* labeltxt, const char *tooltip,
                                        GCallback cb, gpointer cb_data, guint64 source, guint64 mask, const gchar *control_id);


/* Example use:
	GtkDialogFlags flags =  (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);
	GtkWidget *dialog = gtk_dialog_new_with_buttons (N_("Mein Prima Dialog"),
							 GTK_WINDOW (gapp->get_app_window ()),
							 flags,
							 _("_OK"),
							 GTK_RESPONSE_ACCEPT,
							 _("_Cancel"),
							 GTK_RESPONSE_REJECT,
							 NULL);
	BuildParam bp;
	gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), bp.grid);
  
	frame_param = gtk_frame_new (N_("Settings for Move: PU X,Y;"));

	bp.grid_add_widget (frame_param);
	bp.new_line ();

	BuildParam bpf;

	gtk_container_add (GTK_CONTAINER (frame_param), bpf.grid);
	bpf.grid_add_ec ("U", Volt, &dsp_move.bias, -10., 10., "5g"); bpf.new_line ();
        // or just...
        dialog=...;
        BuildParam bp;
        gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), bp.grid);
        bp.grid_add_ec ("lv-inintial", Unity, &vi, 0, src->mem2d->GetNv (), ".0f"); bp.new_line ();
        bp.grid_add_label ("Source for layer:");
        bp.grid_add_widget (source1_combo);
        ...
	gtk_widget_show_all (dialog);
*/

class BuildParam{
 public:
        #define ECIMAX 32
        BuildParam (GtkWidget *build_grid=NULL, GSList *ec_list_start=NULL, GSList *ec_remote_list_start=NULL){
                x=y=1; wx=1; wy=1;
                ec=NULL; for (int i=0; i<ECIMAX; ++i) ec_iter[i]=NULL;
                input_nx=1;
                input_width_chars=-1;
                label_width_chars=-1;
                set_ec_label_xalign ();
                set_ec_label_margins ();
                no_spin = false;
                if (build_grid)
                        grid = build_grid;
                else
                        grid = gtk_grid_new ();
                
                ec = NULL;
                input = NULL;
                button = NULL;
                radiobutton = NULL;
                label = NULL;

                ec_list = ec_list_start;
                remote_list_ec = ec_remote_list_start;
                freeze_list_active = FALSE;
                freeze_list_ec = NULL;
                configure_list_active = FALSE;
                configure_list = NULL;
                configure_hide_list_active = FALSE;
                configure_hide_list = NULL;
                configure_hide_list_b_active = FALSE;
                configure_hide_list_b = NULL;
                remote_prefix = NULL;
                error_text = NULL;
                set_pcs_remote_prefix (""); // ""
                set_error_text ("Value out of range!"); // OUT_OF_RANGE;
                data = NULL;
        };
        virtual ~BuildParam(){ delete_all_ec (); };

        void new_grid (gint wwx=-1) {
                push_grid ();
                x=y=1;
                if (wwx > 0) wx=wwx;
                grid = gtk_grid_new ();
        };

        void grid_add_widget (GtkWidget *w, gint wwx=1, gint wwy=1){
                if (w){
                        any_widget = w;
                        gtk_grid_attach (GTK_GRID (grid), w, x, y, wwx, wwy);
                        if (configure_list_active)
                                configure_list = g_slist_prepend (configure_list, w);
                        if (configure_hide_list_active)
                                configure_hide_list = g_slist_prepend (configure_hide_list, w);
                        if (configure_hide_list_b_active)
                                configure_hide_list_b = g_slist_prepend (configure_hide_list_b, w);
                }
                x+=wwx;
      };

        void new_grid_with_frame (const gchar *frame_title, gint fwx=1, gint fwy=1) {
                frame = gtk_frame_new (N_(frame_title));   
                gtk_frame_set_label_align (GTK_FRAME (frame), 0.02, 0.5);
                grid_add_widget (frame, fwx, fwy);
                new_grid ();
                gtk_container_add (GTK_CONTAINER (frame), grid);
        };


        static void frame_lock_callback (GtkWidget *button, GtkWidget *data) {
                GtkWidget *w = data;
                gtk_button_set_image
                        (GTK_BUTTON (button),
                         gtk_image_new_from_icon_name
                         (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))
                          ? "changes-prevent-symbolic" : "changes-allow-symbolic",
                          GTK_ICON_SIZE_BUTTON));

                if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))){
                        gtk_widget_hide (w);
                        gtk_widget_hide ((GtkWidget*)g_object_get_data( G_OBJECT (w), "label"));
                } else {
                        gtk_widget_show (w);
                        gtk_widget_show ((GtkWidget*)g_object_get_data( G_OBJECT (w), "label"));
                }
        };

        void new_grid_with_frame_lock (const gchar *frame_title, gint fwx=1, gint fwy=1, gboolean state=true) {
                frame = gtk_frame_new (N_(frame_title));   
                gtk_frame_set_label_align (GTK_FRAME (frame), 0.02, 0.5);
                grid_add_widget (frame, fwx, fwy);
                new_grid ();
                gtk_container_add (GTK_CONTAINER (frame), grid);

                GtkWidget *flbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
                GtkWidget *fll = gtk_label_new (frame_title);
                GtkWidget *flb = gtk_toggle_button_new ();
                gtk_container_add (GTK_CONTAINER(flbox), flb);
                gtk_container_add (GTK_CONTAINER(flbox), fll);
                gtk_button_set_image (GTK_BUTTON (flb),
                                      gtk_image_new_from_icon_name (state ? "changes-prevent-symbolic" : "changes-allow-symbolic", GTK_ICON_SIZE_BUTTON));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), state);

                g_object_set_data (G_OBJECT (grid), "frame", frame);
                g_object_set_data (G_OBJECT (grid), "label", fll);

                g_signal_connect(G_OBJECT (flb), "toggled", G_CALLBACK(frame_lock_callback), grid);

                gtk_frame_set_label_widget (GTK_FRAME (frame), flbox);
        };

        
        GtkWidget* grid_add_label (const gchar* labeltxt, const char *tooltip=NULL, gint lwx=1, gfloat xalign=0.5){
                label = gtk_label_new (N_(labeltxt));
                if (fabs (xalign-0.5) > 0.01)
                        gtk_label_set_xalign (GTK_LABEL (label), xalign);
                if (tooltip)
                        gtk_widget_set_tooltip_text(label, tooltip);
                if (label_width_chars > 0)
                        gtk_label_set_width_chars (GTK_LABEL (label), label_width_chars);
		grid_add_widget (label, lwx);
                return label;
        };

        GtkWidget* grid_add_input(const gchar* labeltxt=NULL, gint nx=1, GtkWidget *opt_label_widget=NULL, gint lwx=1){
                GtkWidget *label;

                if (labeltxt){
                        label = gtk_label_new (labeltxt);
                        gtk_label_set_xalign (GTK_LABEL (label), ec_label_xalign);
                        gtk_widget_set_margin_start (GTK_WIDGET (label), margin_start);
                        gtk_widget_set_margin_end (GTK_WIDGET (label), margin_end);
                        gtk_widget_set_halign (GTK_WIDGET (label), GTK_ALIGN_END);
                        grid_add_widget (label, lwx);
                        if (label_width_chars > 0)
                                gtk_label_set_width_chars (GTK_LABEL (label), label_width_chars);
                }
                
                if (opt_label_widget){
                        grid_add_widget (opt_label_widget, lwx);
                }
                
                input = gtk_entry_new ();
                if (input_width_chars > 0)
                        gtk_entry_set_width_chars (GTK_ENTRY (input), input_width_chars);
                
                grid_add_widget (input, nx);
                
                return input;
        };
        
        GtkWidget* setup_scale(GtkAdjustment *adj, int nx=1){
                GtkWidget *skl = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL,  GTK_ADJUSTMENT(adj));
                gtk_scale_set_value_pos (GTK_SCALE (skl), GTK_POS_LEFT);
                gtk_scale_set_draw_value(GTK_SCALE(skl), FALSE);
                gtk_widget_set_hexpand (skl, TRUE);
                grid_add_widget (skl, nx);
                return skl;
        };

        GtkWidget* grid_add_spin_input (const gchar *labeltxt, int nx=1, GtkWidget* opt_label_widget=NULL, gint lwx=1){
                GtkAdjustment *adjust;
                
                if (labeltxt){
                        label = gtk_label_new (labeltxt);
                        grid_add_widget (label, lwx);
                        gtk_label_set_xalign (GTK_LABEL (label), ec_label_xalign);
                        gtk_widget_set_margin_start (GTK_WIDGET (label), margin_start);
                        gtk_widget_set_margin_end (GTK_WIDGET (label), margin_end);
                        if (label_width_chars > 0)
                                gtk_label_set_width_chars (GTK_LABEL (label), label_width_chars);
                }
                if (opt_label_widget){
                        grid_add_widget (opt_label_widget, lwx);
                }
                
                adjust = gtk_adjustment_new (0., -10., 10., 1., 10., 10.);
                spin   = gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 1., 0);
                grid_add_widget (spin, nx);
                
                g_object_set_data( G_OBJECT (spin), "Adjustment", adjust);
                
                return spin;
        };


        GtkWidget* grid_add_ec (const gchar* label, UnitObj *unit, double *var, double lo, double hi, const gchar *fmt, double step=0., double page=0., const gchar *remote_id=NULL, gint ec_array_index=-1){
                gchar *rid = NULL;
                gchar *arr = NULL;

                // step and page must be set to zero for NO ADJUSTMENT USE !!
                
                if (ec_array_index >= 0)
                        arr = g_strdup_printf ("%02d", ec_array_index);
                        
                if (remote_id)
                        rid = g_strconcat (remote_prefix, remote_id, arr, NULL); // do not free, keep
                if (arr)
                        g_free (arr);
                
                if (no_spin)
                        input = grid_add_input (N_(label), input_nx);
                else
                        input = grid_add_spin_input (N_(label), input_nx);

                if (input_width_chars > 0)
                        gtk_entry_set_width_chars (GTK_ENTRY (input), input_width_chars);
                
                if (rid)
                        g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)rid);
                else
                        g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", NULL);
                
                ec = new Gtk_EntryControl (unit, N_(error_text), var, lo, hi, fmt, input, step, page, 0., ec_array_index >= 0 ? ec_array_index+1 : 0);

                ec_list = g_slist_prepend (ec_list, ec);
                
                if (rid)
                        remote_list_ec = ec->AddEntry2RemoteList(rid, remote_list_ec);

                if (freeze_list_active)
                        freeze_list_ec = g_slist_prepend (freeze_list_ec, ec);

                if (data)
                        ec->Set_ChangeNoticeFkt (ChangeNoticeFkt, data);

                // link ec array list
                if (ec_array_index >= 0){
                        if (x < ECIMAX){
                                if (ec_array_index == 0){
                                        ec->set_head_iter (ec); ec->set_count (1); 
                                } else {
                                        ec->set_head_iter (ec_iter[x]->get_iter_head ());
                                        ec_iter[x]->set_next_iter (ec);   
                                }
                                ec_iter[x] = ec;
                                // ec->init_pcs_array (); // must run after last ec element created to initialize
                                // run --> init_ec_array ();
                        } else {
                                g_print ("BUILD-ERROR for ARRAY EC. Col Pos Mem x >= 32 [%d]\n", x);
                        }
                }

                
                scale = NULL;
                return input;
        };

        GtkWidget *grid_add_ec (const gchar* label, UnitObj *unit, int *var, int lo, int hi, const gchar *fmt, const gchar *remote_id=NULL, gint ec_array_index=-1){
                gchar *rid = NULL;
                gchar *arr = NULL;
                
                if (ec_array_index >= 0)
                        arr = g_strdup_printf ("%02d", ec_array_index);
                        
                if (remote_id)
                        rid = g_strconcat (remote_prefix, remote_id, arr, NULL); // do not free, keep
                if (arr)
                        g_free (arr);
                
                if (no_spin)
                        input = grid_add_input (N_(label), input_nx);
                else
                        input = grid_add_spin_input (N_(label), input_nx);
                
                if (input_width_chars > 0)
                        gtk_entry_set_width_chars (GTK_ENTRY (input), input_width_chars);
                
                // if (rid == NULL)
                //        rid = g_strconcat (remote_prefix, label, NULL); // do not free, keep

                if (rid)
                        g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)rid);
                else
                        g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", NULL);
                
                ec = new Gtk_EntryControl (unit, N_(error_text), var, lo, hi, fmt, input, 1., 10., 0., ec_array_index >= 0 ? ec_array_index+1 : 0);

                ec_list = g_slist_prepend (ec_list, ec);
                
                if (rid)
                        remote_list_ec = ec->AddEntry2RemoteList(rid, remote_list_ec);

                if (freeze_list_active)
                        freeze_list_ec = g_slist_prepend (freeze_list_ec, ec);

                if (data)
                        ec->Set_ChangeNoticeFkt (ChangeNoticeFkt, data);

                if (ec_array_index >= 0){
                        if (x < ECIMAX){
                                if (ec_array_index == 0){
                                        ec->set_head_iter (ec); ec->set_count (1); 
                                } else {
                                        ec->set_head_iter (ec_iter[x]->get_iter_head ());
                                        ec_iter[x]->set_next_iter (ec);   
                                }
                                ec_iter[x] = ec;
                                // ec->init_pcs_array (); // must run after last ec element created to initialize
                                // run --> init_ec_array ();
                        } else {
                                g_print ("BUILD-ERROR for ARRAY EC. Col Pos Mem x >= 32 [%d]\n", x);
                        }
                }

                scale = NULL;
                return input;
        };

        GtkWidget *grid_add_ec (const gchar* label, UnitObj *unit, const gchar *var, const gchar *remote_id=NULL, gint ec_array_index=-1){
                gchar *rid = NULL;
                gchar *arr = NULL;
                
                if (remote_id)
                        rid = g_strconcat (remote_prefix, remote_id, arr, NULL); // do not free, keep
                if (arr)
                        g_free (arr);
                
                input = grid_add_input (N_(label), input_nx);
                
                if (input_width_chars > 0)
                        gtk_entry_set_width_chars (GTK_ENTRY (input), input_width_chars);
                
                ec = new Gtk_EntryControl (unit, N_(error_text), var, input, ec_array_index >= 0 ? ec_array_index+1 : 0);

                if (rid)
                        g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", (void*)rid);
                else
                        g_object_set_data (G_OBJECT (input), "Adjustment_PCS_Name", NULL);
                
                ec_list = g_slist_prepend (ec_list, ec);
                
                if (rid)
                        remote_list_ec = ec->AddEntry2RemoteList(rid, remote_list_ec);

                if (freeze_list_active)
                        freeze_list_ec = g_slist_prepend (freeze_list_ec, ec);

                if (data)
                        ec->Set_ChangeNoticeFkt (ChangeNoticeFkt, data);

                if (ec_array_index >= 0){
                        if (x < ECIMAX){
                                if (ec_array_index == 0){
                                        ec->set_head_iter (ec); ec->set_count (1); 
                                } else {
                                        ec->set_head_iter (ec_iter[x]->get_iter_head ());
                                        ec_iter[x]->set_next_iter (ec);   
                                }
                                ec_iter[x] = ec;
                                // ec->init_pcs_array (); // must run after last ec element created to initialize
                                // run --> init_ec_array ();
                        } else {
                                g_print ("BUILD-ERROR for ARRAY EC. Col Pos Mem x >= 32 [%d]\n", x);
                        }
                }

                scale = NULL;
                return input;
        };

        
        GtkWidget* grid_add_ec_with_scale (const gchar* labeltxt, UnitObj *unit, double *var, double lo, double hi, const gchar *fmt, double step=1., double page=1., const gchar *remote_id=NULL){
                grid_add_ec (labeltxt, unit, var, lo, hi, fmt, step, page, remote_id);
                scale = setup_scale(ec->GetAdjustment(), wx);
                return input;
        };

        GtkWidget* grid_add_ec_with_scale (const gchar* labeltxt, UnitObj *unit, int *var, int lo, int hi, const gchar *fmt, const gchar *remote_id=NULL){
                grid_add_ec (labeltxt, unit, var, lo, hi, fmt, remote_id);
                scale = setup_scale(ec->GetAdjustment(), wx);
                return input;
        };

        void grid_add_frame_with_grid (const gchar* frametitle, GtkWidget *f_grid, int lwx=1){
                frame = gtk_frame_new (N_(frametitle));
		grid_add_widget (frame, lwx);
                gtk_container_add (GTK_CONTAINER (frame), f_grid);
        };
        
        GtkWidget* grid_add_button (const gchar* labeltxt, const char *tooltip=NULL, int bwx=1,
                                    GCallback cb=NULL, gpointer data=NULL,
                                    const gchar *data_key=NULL, gpointer key_data=NULL){
                button = gtk_button_new_with_label (N_(labeltxt));

                if (data_key)
                        g_object_set_data (G_OBJECT (button), data_key, key_data);
                if (tooltip)
                        gtk_widget_set_tooltip_text (button, tooltip);
                if (cb){
                        g_signal_connect(G_OBJECT (button), "clicked", G_CALLBACK(cb), data);
                }
		grid_add_widget (button, bwx);
                return button;
        };
        GtkWidget* grid_add_button_icon_name (const gchar* iconname, const char *tooltip=NULL, int bwx=1,
                                              GCallback cb=NULL, gpointer data=NULL,
                                              const gchar *data_key=NULL, gpointer key_data=NULL){
                button = gtk_button_new_from_icon_name (iconname, GTK_ICON_SIZE_BUTTON);

                if (data_key)
                        g_object_set_data (G_OBJECT (button), data_key, key_data);
                if (tooltip)
                        gtk_widget_set_tooltip_text (button, tooltip);
                if (cb){
                        g_signal_connect(G_OBJECT (button), "clicked", G_CALLBACK(cb), data);
                }
		grid_add_widget (button, bwx);
                return button;
        };

        // generate ID-KEY for tooltip remote hook
        const gchar* PYREMOTE_CHECK_HOOK_KEY_FUNC(const gchar* tt, const gchar *idkey, int k=-1){
                g_message ("PYREMOTE_CHECK_HOOK_KEY_FUNC: %s. %s%s%02d)",tt,PYREMOTE_CHECK_HOOK_KEY_PREFIX,idkey,k);
                if (k >= 0)
                        return g_strdup_printf ("%s. %s%s%02d)",tt,PYREMOTE_CHECK_HOOK_KEY_PREFIX,idkey,k);
                else
                        return g_strdup_printf ("%s. %s%s)",tt,PYREMOTE_CHECK_HOOK_KEY_PREFIX,idkey);
        };

        // add this (example) is tooltip string for remote enabling:  PYREMOTE_CHECK_HOOK_KEY("SCAN-REPEAT")
        GtkWidget* grid_add_check_button_remote_enabled (const gchar* labeltxt, const char *tooltip=NULL, int bwx=1,
                                                         GCallback cb=NULL, gpointer data=NULL, guint64 source=0, guint64 mask=0, const gchar *control_id=NULL);
        
        GtkWidget* grid_add_check_button (const gchar* labeltxt, const char *tooltip=NULL, int bwx=1, GCallback cb=NULL, gpointer data=NULL, int source=0, int mask=0);

        //GtkWidget* grid_add_check_button_guint64(const gchar* labeltxt, const char *tooltip=NULL, int bwx=1, GCallback cb=NULL, gpointer data=NULL, guint64 source=0, guint64 mask=0, const gchar *control_id=NULL);

        
        GtkWidget* grid_add_exec_button (const gchar* labeltxt,
                                         GCallback exec_cb, gpointer cb_data, const gchar *control_id,
                                         int bwx=1,
                                         const gchar *data_key=NULL, gpointer key_data=NULL);
        /*
        GtkWidget* grid_add_check_button (const gchar* labeltxt, const char *tooltip=NULL, int bwx=1,
                                          GCallback cb=NULL, gpointer data=NULL, gint source=0, gint mask=-1){
                button = gtk_check_button_new_with_label (N_(labeltxt));
                if (tooltip)
                        gtk_widget_set_tooltip_text (button, tooltip);
                if (mask > 0){
                        g_object_set_data(G_OBJECT(button), "Bit_Mask", GINT_TO_POINTER (mask));
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), (((source) & (mask)) ? 1:0));
                }
                if (mask == 0){
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), (source)? 1:0);
                }
                if (cb){
                        g_signal_connect(G_OBJECT (button), "toggled", G_CALLBACK(cb), data);
                }
                grid_add_widget (button, bwx);
                return button;
        };
        */
        static void lock_callback (GtkWidget *button, void *data) {
                gtk_button_set_image
                        (GTK_BUTTON (button),
                         gtk_image_new_from_icon_name
                         (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))
                          ? "changes-prevent-symbolic" : "changes-allow-symbolic",
                          GTK_ICON_SIZE_BUTTON));
        };
        GtkWidget* grid_add_lock_button (const char *tooltip=NULL, gboolean state=false, int bwx=1,
                                         GCallback cb=NULL, gpointer data=NULL){
                button = gtk_toggle_button_new ();
                gtk_button_set_image (GTK_BUTTON (button),
                                      gtk_image_new_from_icon_name (state ? "changes-prevent-symbolic" : "changes-allow-symbolic", GTK_ICON_SIZE_BUTTON));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), state);
                if (tooltip)
                        gtk_widget_set_tooltip_text (button, tooltip);

                if (cb){
                        g_signal_connect(G_OBJECT (button), "toggled", G_CALLBACK(cb), data);
                } else {
                        g_signal_connect(G_OBJECT (button), "toggled", G_CALLBACK(lock_callback), this);
                }
                grid_add_widget (button, bwx);
                return button;
        };
        GtkWidget* grid_add_permission_button (const char *tooltip=NULL, gboolean perm=true, int bwx=1){
                GPermission* p = g_simple_permission_new (perm);
                button = gtk_lock_button_new (p);
                // bool = g_permission_get_allowed (gtk_lock_button_get_permission (GTK_LOCK_BUTTON (button)));

                if (tooltip)
                        gtk_widget_set_tooltip_text (button, tooltip);

                grid_add_widget (button, bwx);
                return button;
        };

        GtkWidget* grid_add_check_button_gint32 (const gchar* labeltxt, const char *tooltip=NULL, int bwx=1,
                                                 GCallback cb=NULL, gpointer data=NULL, gint32 source=0, gint32 mask=-1, const gchar *control_id=NULL){
                button = gtk_check_button_new_with_label (N_(labeltxt));
                if (tooltip)
                        gtk_widget_set_tooltip_text (button, tooltip);
                if (mask > 0){
                        g_object_set_data(G_OBJECT(button), "Bit_Mask", GINT_TO_POINTER (mask));
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), (((source) & (mask)) ? 1:0));
                }
                if (mask == 0){
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), (source)? 1:0);
                }
                if (cb){
                        g_signal_connect(G_OBJECT (button), "toggled", G_CALLBACK(cb), data);
                }
                grid_add_widget (button, bwx);
                return button;
        };

        GtkWidget* grid_add_check_button_guint64(const gchar* labeltxt, const char *tooltip=NULL, int bwx=1, GCallback cb=NULL, gpointer data=NULL, guint64 source=0, guint64 mask=0, const gchar *control_id=NULL);

        //GtkWidget* grid_add_check_button_guint64 (const gchar* labeltxt, const char *tooltip, int bwx,  GCallback cb, gpointer data, guint64 source, guint64 mask, const gchar *control_id=NULL);

        GtkWidget* grid_add_radio_button (const gchar* labeltxt, const char *tooltip=NULL,
                                          GCallback cb=NULL, gpointer data=NULL, gboolean start=false){
                if (start || !radiobutton)
                        radiobutton = gtk_radio_button_new_with_label (NULL, N_(labeltxt));
                else
                        radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton), N_(labeltxt));
                if (tooltip)
                        gtk_widget_set_tooltip_text (radiobutton, tooltip);
                if (cb){
                        g_signal_connect(G_OBJECT (radiobutton), "clicked", G_CALLBACK(cb), data);
                }
                grid_add_widget (radiobutton);
                return radiobutton;
        };

        void grid_add_widget_with_label (const gchar* labeltxt, GtkWidget *w, int lwx=1, int wwx=1){
                label = gtk_label_new (N_(labeltxt));
                if (label_width_chars > 0)
                        gtk_label_set_width_chars (GTK_LABEL (label), label_width_chars);
		grid_add_widget (label, lwx);
		grid_add_widget (w, wwx);
        };
        

        void set_ec_change_notice_fkt (void (*newChangeNoticeFkt)(Param_Control* pcs, gpointer data), gpointer new_data) {
                ec->Set_ChangeNoticeFkt (newChangeNoticeFkt, new_data);  
        };
        void set_default_ec_change_notice_fkt (void (*newChangeNoticeFkt)(Param_Control* pcs, gpointer data), gpointer new_data) {
                ChangeNoticeFkt = newChangeNoticeFkt;
                data = new_data;
        };
        
        // .. more special .. via class derive ..
        
        void init_ec_array () { ec->init_pcs_array (); }; // must run after last ec element created to initialize
        void set_no_spin (gboolean ns=true) { no_spin = ns; };
        void start (int wxs=0) { x=1; y=1; if (wxs>0) wx = wxs; }; // start at 1,1 with scale width wxs if > 0
        void new_line (int wxs=0, gint indent=1) { x=indent; ++y; if (wxs>0) wx = wxs; }; // start new line with scale width wxs if > 0
        void skip_row (int wxs=1) { x += wxs; }; // skip row(s)
        void show_all () { gtk_widget_show_all (grid); };

        // set default entry/inpout width chars to be configured, -1: leve unset/default
        void set_input_width_chars (gint nwc=-1){
                input_width_chars = nwc;
        };
        void set_input_nx (gint inx=1){
                input_nx = inx;
        };
        void set_scale_nx (gint snx=1){
                wx=snx;
        };

        void set_label_width_chars (gint nwc=-1){
                label_width_chars = nwc;
        }
        
        static void delete_ec (gpointer data){
                Gtk_EntryControl *ec_tmp = (Gtk_EntryControl*)data;
                delete ec_tmp;
        };
        static void update_ec(Gtk_EntryControl* ec, gpointer data){ ec->Put_Value(); };
        void update_all_ec () { g_slist_foreach (ec_list, (GFunc) BuildParam::update_ec, NULL); };
        void delete_all_ec () {
               g_slist_free_full (ec_list, BuildParam::delete_ec);
               ec_list = NULL;
        };

        GSList *get_ec_list_head () { return ec_list; };
        void add_to_ec_list (Gtk_EntryControl *ecx ) { ec_list = g_slist_prepend (ec_list, ecx); };

        GSList *get_remote_list_head () { return remote_list_ec; };
        
        void set_freeze_list_head (GSList *fl=NULL) { freeze_list_ec = fl; };
        void set_freeze_list_mode_off () { freeze_list_active = FALSE; };
        void set_freeze_list_mode_on  () { freeze_list_active = TRUE; };
        GSList *get_freeze_list_head  () { return freeze_list_ec; };

        void set_configure_list_head (GSList *cl=NULL) { configure_list = cl; };
        void set_configure_list_mode_off () { configure_list_active = FALSE; };
        void set_configure_list_mode_on () { configure_list_active = TRUE; };
        GSList *get_configure_list_head () { return configure_list; };
        void add_to_configure_list (GtkWidget *w){
                if (configure_list_active)
                        configure_list = g_slist_prepend (configure_list, w);
        };

        void set_configure_hide_list_head (GSList *cl=NULL) { configure_hide_list = cl; };
        void set_configure_hide_list_mode_off () { configure_hide_list_active = FALSE; };
        void set_configure_hide_list_mode_on () { configure_hide_list_active = TRUE; };
        GSList *get_configure_hide_list_head () { return configure_hide_list; };
        void add_to_configure_hide_list (GtkWidget *w){
                configure_hide_list = g_slist_prepend (configure_hide_list, w);
        };

        void set_configure_hide_list_b_head (GSList *cl=NULL) { configure_hide_list_b = cl; };
        void set_configure_hide_list_b_mode_off () { configure_hide_list_b_active = FALSE; };
        void set_configure_hide_list_b_mode_on () { configure_hide_list_b_active = TRUE; };
        GSList *get_configure_hide_list_b_head () { return configure_hide_list_b; };
        void add_to_configure_hide_list_b (GtkWidget *w){
                configure_hide_list_b = g_slist_prepend (configure_hide_list_b, w);
        };

        // only one level so far! May use GSLists if needed later...
        void push_grid() {
                grid_push=grid; xy_push[0]=x; xy_push[1]=y; xy_push[2]=wx; xy_push[3]=wy;
        };
        
        void pop_grid() {
                grid = grid_push; x=xy_push[0]; y=xy_push[1]; wx=xy_push[2]; wy=xy_push[3];
        };

        void set_xy (gint sx, gint sy) { x=sx; y=sy; };

        void set_error_text (const gchar* new_error_text = NULL){
                if (error_text)
                        g_free (error_text);
                error_text = NULL;
                if (new_error_text)
                        error_text = g_strdup (new_error_text);
        };
        
        void set_pcs_remote_prefix (const gchar* new_remote_prefix = NULL){
                if (remote_prefix)
                        g_free (remote_prefix);
                remote_prefix = NULL;
                if (new_remote_prefix)
                        remote_prefix = g_strdup (new_remote_prefix);
        };
        
        void set_ec_label_xalign (gfloat xalign=1.0) { ec_label_xalign = xalign; };
        void set_ec_label_margins (gint mstart=2, gint mend=5) { margin_start=mstart; margin_end=mend; };

        GtkWidget *grid;
        int x,y, wx, wy;
        int input_nx;
        int input_width_chars;
        int label_width_chars;

        gboolean no_spin;
        
        GSList *ec_list;
        GSList *remote_list_ec;

        gboolean freeze_list_active;
        GSList *freeze_list_ec;

        gboolean configure_list_active;
        GSList *configure_list;
        gboolean configure_hide_list_active;
        GSList *configure_hide_list;
        gboolean configure_hide_list_b_active;
        GSList *configure_hide_list_b;

        gchar *remote_prefix;
        gchar *error_text;

        void (*ChangeNoticeFkt)(Param_Control* pcs, gpointer data);
        gpointer data;

        // output
        GtkWidget *frame;
        GtkWidget *label;
        GtkWidget *button;
        GtkWidget *radiobutton;
        GtkWidget *input;
        GtkWidget *spin;
        GtkWidget *scale;
        GtkWidget *any_widget;
        Gtk_EntryControl *ec;
        Gtk_EntryControl *ec_iter[ECIMAX];
private:
        GtkWidget *grid_push;
        gint xy_push[4];
        gfloat ec_label_xalign;
        gint   margin_start, margin_end;
};

#define MYGTK_LSIZE 100
#define MYGTK_ESIZE 100

#define MAX_PROGRESS_LEVELS 13

class MyGnomeTools{
public:
	MyGnomeTools(){ };
	virtual ~MyGnomeTools(){ };

//#if 0
	/* My helper functions used for App Creation Stuff */
	// setup a nice input field

	// new gtk3.x grid based input
	static GtkWidget* mygtk_grid_add_input(const char* labeltxt, GtkWidget* grid, int &x, int &y, int nx=1, GtkWidget* opt_label_widget=NULL, GSList **l=NULL);

	// add a scale widget for quick mouse adjustment an wheel usage
	static GtkWidget* SetupScale(GtkAdjustment *adj, GtkWidget *grid, int &x, int &y, int nx=1);

        // input with spinbuttons
	static GtkWidget* mygtk_grid_add_spin_input(const gchar *labeltxt, GtkWidget *grid, int &x, int &y, int nx=1, GtkWidget *opt_label_widget=NULL, GSList **l=NULL);
#if 0

	//  old and depricated, use "mygtk_add_input grid version"
	static GtkWidget* mygtk_create_input(const char* labeltxt, GtkWidget* vbox, 
					     GtkWidget* &hbox, 
					     int lsize=MYGTK_LSIZE, int esize=MYGTK_ESIZE, GtkWidget* opt_label_widget=NULL);
	static GtkWidget* mygtk_add_input(GtkWidget *hbox, int esize=MYGTK_ESIZE);
	static GtkWidget* mygtk_add_input(const gchar *labeltxt, GtkWidget *hbox, int lsize=MYGTK_LSIZE, int esize=MYGTK_ESIZE);

	// depricated
	static GtkWidget* SetupScale(GtkAdjustment *adj, GtkWidget *hbox, int sklsize=150);


	// old and depricated, use "mygtk_add_spin_input grid version"
	static GtkWidget* mygtk_create_spin_input(const gchar* labeltxt, GtkWidget* vbox, GtkWidget* &hbox, int lsize=MYGTK_LSIZE, int esize=-1, GtkWidget *opt_label_widget=NULL);
	static GtkWidget* mygtk_add_spin(GtkWidget *hbox, int lsize=MYGTK_LSIZE, int esize=MYGTK_ESIZE);
	static GtkWidget* mygtk_add_spin(const gchar *labeltxt, GtkWidget *hbox, int lsize=MYGTK_LSIZE, int esize=MYGTK_ESIZE);

	// add a selectionlist
	static GtkWidget* mygtk_add_list(GtkWidget *hbox, int lsize=MYGTK_LSIZE, int esize=MYGTK_ESIZE);
	static GtkWidget* mygtk_add_list(const gchar *labeltxt, GtkWidget *hbox, int lsize=MYGTK_LSIZE, int esize=MYGTK_ESIZE);
#endif
        
	static void show_info_callback (GtkWidget *widget, const char *info){
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (widget)),
							    GTK_DIALOG_DESTROY_WITH_PARENT,
							    GTK_MESSAGE_INFO,
							    GTK_BUTTONS_CLOSE,
							    "%s", info);
		g_signal_connect_swapped (G_OBJECT (dialog), "response",
					  G_CALLBACK (gtk_widget_destroy),
					  G_OBJECT (dialog));
		gtk_widget_show (dialog);
	};
	
	static GMenuModel *find_extension_point_section (GMenuModel  *model, const gchar *extension_point);
};

class AppBase;

#ifndef GXSM_WIDGET_PAD
#define GXSM_WIDGET_PAD          8
#endif

#define WGEO_FLAG 0
#define WGEO_SHOW 1
#define WGEO_XPOS 2
#define WGEO_YPOS 3
#define WGEO_WIDTH  4
#define WGEO_HEIGHT 5
#define WGEO_SIZE 6

class AppBase : public MyGnomeTools{
public:
	AppBase();
	virtual ~AppBase();
  
	// Basic Setup Window/Widget
	virtual void AppWindowInit(const gchar *title);
        virtual void add_window_to_window_menu(const gchar *menu_item_label, const gchar* key);

	int set_window_geometry (const gchar *key, gint index=-1, gboolean add_to_menu=true);

	static void SaveGeometryCallback(AppBase *apb);

	int SaveGeometry(int savealways=TRUE);
	int LoadGeometry();

	/* action callbacks */
        static gboolean window_state_watch_callback (GtkWidget *widget,
                                                     GdkEvent  *event,
                                                     gpointer   user_data);

        static gboolean window_close_callback (GtkWidget *widget,
                                               GdkEvent  *event,
                                               gpointer   user_data);
  
        static void window_action_callback (GSimpleAction *simple,
                                            GVariant *parameter,
                                            gpointer user_data);
        
	/* window actions */
	void hide ();
	void show ();
	void show_auto ();
	void position_auto ();
	void resize_auto ();
	void freeze(){  gtk_window_set_accept_focus (window, FALSE); /* gtk_widget_set_sensitive (window, FALSE); */ };
	void thaw(){  gtk_window_set_accept_focus (window, TRUE); /* gtk_widget_set_sensitive (window, TRUE); */ };

	/* sorry, but I need this for external access form main only... */  
	GtkWindow* get_window() { return window; };
	Gxsm3appWindow* get_app_window() { return app_window; };

protected:
	void destroy(){ if (window) { gtk_widget_destroy (GTK_WIDGET (window)); window=NULL; } nodestroy=TRUE; };
	int nodestroy;

        Gxsm3appWindow *app_window;
	GtkWindow* window;     // main window for this object
	GtkWidget* header_bar;
	GtkWidget* v_grid; // 1st level grid in window

private:

        GSettings *geometry_settings;
        gchar *window_key;

        gint32 *window_geometry;
        gint32 showstate;
};

class DlgBase : public MyGnomeTools{
public:
	DlgBase(){ runflg=FALSE; };
	virtual ~DlgBase(){};

	virtual void run()=0;
	int running(){ return runflg; };

protected:
	void DlgStart(){ runflg=TRUE; };
	void DlgDone(){ runflg=FALSE; };

private:
	int runflg;
};

class Scan;

class GnomeAppService : public AppBase{
private:
	GtkWidget *app;
        gboolean  thread_safe_no_gui_mode;
public:
	GnomeAppService(GtkWidget *App=NULL){ app=App; fname = NULL; progress_dialog=NULL; thread_safe_no_gui_mode=false; };
	virtual ~GnomeAppService(){ 
		if(fname){ g_free(fname); fname=NULL; }
		choice(NULL, NULL, NULL, 0, NULL, NULL, NULL, 0);
		progress_info_close ();	
	};

        inline void enter_thread_safe_no_gui_mode() { thread_safe_no_gui_mode=true; };
        inline void exit_thread_safe_no_gui_mode() { thread_safe_no_gui_mode=false; };
        inline gboolean is_thread_safe_no_gui_mode() { return thread_safe_no_gui_mode; };
        
	void GnomeAppServiceSetApp(GtkWidget *App){ app=App; };

	GtkWidget *getApp(){ return app; };


        gint setup_multidimensional_data_copy (const gchar *title, Scan *src,
                                               int &ti, int &tf, int &vi, int &vf,
                                               int *tnadd=NULL, int *vnadd=NULL,
                                               int *crop_window_xy=NULL, gboolean crop=FALSE);
        
	inline void check_events(const gchar *info=NULL){
                if (is_thread_safe_no_gui_mode()) return;
                progress_info_new (info?info:"Instrument Busy",1,NULL, NULL, TRUE);  while(gtk_events_pending()) gtk_main_iteration(); progress_info_close ();
        };
	inline void check_events_self(){
                if (is_thread_safe_no_gui_mode()) return;
                while(gtk_events_pending()) gtk_main_iteration();
        };

/* FixMe !!!! */
	void flash(const char *mld){
		std::cout << "************" << mld << "************" << std::endl;
	};
	void warning(const char *mld){
                if (is_thread_safe_no_gui_mode()) return;
		if(window){
			GtkWidget *dialog = gtk_message_dialog_new_with_markup
                                (window,
                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                 GTK_MESSAGE_WARNING,
                                 GTK_BUTTONS_CLOSE,
                                 "<span foreground='red' size='large' weight='bold'>%s</span>\n%s", N_("Warning!"),mld);
			g_signal_connect_swapped
                                (G_OBJECT (dialog), "response",
                                 G_CALLBACK (gtk_widget_destroy),
                                 G_OBJECT (dialog));
			gtk_widget_show (dialog);
                        //gapp->monitorcontrol->LogEvent ("Warning", mld);
		}
	};
        static gint terminate_timeout_func (gpointer data);
	void alert(const gchar *s1, const gchar *s2, const gchar *s3, int c);
	void errormsg(const char *mld){
                if (is_thread_safe_no_gui_mode()) return;
		if(window){
			GtkWidget *dialog = gtk_message_dialog_new_with_markup
                                (window,
                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                 GTK_MESSAGE_ERROR,
                                 GTK_BUTTONS_CLOSE,
                                 "<span foreground='red' size='large' weight='bold'>%s</span>\n%s", N_("Error!"), mld);
			g_signal_connect_swapped
                                (G_OBJECT (dialog), "response",
                                 G_CALLBACK (gtk_widget_destroy),
                                 G_OBJECT (dialog));
			gtk_widget_show (dialog);
		}
	};
	void message(const char *mld){
                if (is_thread_safe_no_gui_mode()) return;
		if(window){
			GtkWidget *dialog = gtk_message_dialog_new_with_markup
                                (window,
                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                 GTK_MESSAGE_INFO,
                                 GTK_BUTTONS_CLOSE,
                                 "<span foreground='blue' size='large' weight='bold'>%s</span>\n%s", N_("Info"), mld);
			g_signal_connect_swapped
                                (G_OBJECT (dialog), "response",
                                 G_CALLBACK (gtk_widget_destroy),
                                 G_OBJECT (dialog));
			gtk_widget_show (dialog);
                        //gapp->monitorcontrol->LogEvent ("Message", mld);
		}
	};
	gboolean question_yes_no (const gchar *question, GtkWindow *pw=NULL, const gchar *format=NULL){ // Yes / No ?
                if (is_thread_safe_no_gui_mode()) return TRUE;
		GtkWindow *w = pw;
                if (w == NULL)
			w = window;
		if (w){
			GtkDialogFlags flags = GTK_DIALOG_USE_HEADER_BAR; //  GTK_DIALOG_DESTROY_WITH_PARENT;
			GtkWidget *dialog = gtk_message_dialog_new_with_markup
                                (w,
                                 flags,
                                 GTK_MESSAGE_QUESTION,
                                 GTK_BUTTONS_YES_NO,
                                 format ? format : "<span foreground='blue' size='large' weight='bold'>%s</span>", question);
                        
			gint result = gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
                        //gapp->monitorcontrol->LogEvent (question, result == GTK_RESPONSE_YES ? "User Response: YES" : "User Response: NO");
			return result == GTK_RESPONSE_YES;
		}
		return TRUE;
	};

	gint question_yes_no_with_action (const gchar *question, const gchar *action_label, gint &var,
                                          GtkWidget *action=NULL, GtkWindow *pw=NULL){ // Yes / No ?
                if (is_thread_safe_no_gui_mode()) return TRUE;
		GtkWindow *w = pw;
                if (w == NULL)
			w = window;
		if (w){
			GtkDialogFlags flags = GTK_DIALOG_USE_HEADER_BAR; //  GTK_DIALOG_DESTROY_WITH_PARENT;
			GtkWidget *dialog = gtk_message_dialog_new_with_markup
                                (w,
                                 flags,
                                 GTK_MESSAGE_QUESTION,
                                 GTK_BUTTONS_YES_NO,
                                 "<span foreground='blue' size='large' weight='bold'>%s</span>\n", question);
                        if (!action)
                                action = gtk_check_button_new_with_label( N_(action_label));
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (action), var? true:false);
                        gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), action);
                        gtk_widget_show_all (dialog);
			gint result = gtk_dialog_run (GTK_DIALOG (dialog));
                        var = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (action));
			gtk_widget_destroy (dialog);
			return result;
		}
		return TRUE;
	};

	void messages(const char *s1, const char *s2, const char *s3){
		gchar *mld = g_strconcat( s1, "\n", s2, "\n", s3, NULL);
		message((const char*) mld);
		g_free(mld);
	};

	int dialog(const char *title, const char *content, 
		   const char *b1, const char *b2, const char *b3, 
		   int wait=FALSE);

	int choice(const char *s1, const char *s2, const char *s3, int numb, const char *b1, const char *b2, const char *b3, int def);

	const char *input(const char *lab, const char *defs){
		gchar *string=NULL;
		return string;
	}
  
	// returns TRUE if file does not exists or user decided to overwrite file
	// else FALSE
	int check_file(gchar *fn);

	//	static void file_ok_sel (GtkFileSelection *gw, GnomeAppService *p);
	static void string_cb (gchar *string, gpointer data);

	static void destroy (GtkWidget *widget, GnomeAppService *p);

	// File Dialog
	gchar *file_dialog_load (const gchar *title, 
				 const gchar *path, 
				 const gchar *name,
                                 GtkFileFilter **filter=NULL);
	gchar *file_dialog_save (const gchar *title, 
				 const gchar *path, 
				 const gchar *name,
                                 GtkFileFilter **filter=NULL);

        // compat for PLugIns -- obsoleted
        gchar *file_dialog(const gchar *title, 
                           const gchar *path, 
                           const gchar *mask,
                           const gchar *name,
                           const gchar *id){
                g_warning ("gapp_service function File_dialog is obsoleted, use file_dialog_load/save.");
                if (mask){
                        GtkFileFilter *f1 = gtk_file_filter_new ();
                        gtk_file_filter_set_name (f1, mask);
                        gtk_file_filter_add_pattern (f1, mask);
                        GtkFileFilter *f0 = gtk_file_filter_new ();
                        gtk_file_filter_set_name (f0, "All");
                        gtk_file_filter_add_pattern (f0, "*");
                        GtkFileFilter *filter[] = { f1, f0, NULL };
                        if (name) // guessing
                                return file_dialog_save (title, path, name, filter);
                        else
                                return file_dialog_load (title, path, name, filter);
                }

                if (name) // guessing
                        return file_dialog_save (title, path, name);
                else
                        return file_dialog_load (title, path, name);
        };
        
	// Value Request
	void ValueRequest(const gchar *title, const gchar *label, const gchar *infotxt, 
			  UnitObj *u, double minv, double maxv, const gchar *vfmt,
			  double *value);

        void ValueRequestList (const gchar *title,
                          const gchar *label[], const gchar *infotxt[], 
                          UnitObj *uobj[], double minv[], double maxv[], const gchar *vfmt[],
                          double *value[]);
  
	// Progress Dialog
	GtkWidget* progress_info_new (const gchar *title=NULL, gint levels=1, GCallback cancel_cb=NULL, gpointer data=NULL, gboolean modal=false);
	int progress_info_set_bar_fraction (gdouble fraction=0., gint level=1);
	int progress_info_set_bar_pulse (gint level=1, gdouble fraction=-1.);
	int progress_info_set_bar_text (const gchar* text=NULL, gint level=1);
	int progress_info_add_info (const gchar* info);
	void progress_info_close ();
	void progress_info_destroy_now ();
	gint progress_info_close_scheduled () { return progress_dialog_schedule_close; };
	gint progress_info_close_schedule_dec () { return --progress_dialog_schedule_close; };

	GtkWidget* progress_dialog;
	GtkWidget* progress_bar[MAX_PROGRESS_LEVELS];
	gint progress_dialog_schedule_close;

	GtkWidget *filew;
	int  fdlg;
	gchar *fname;
	int  qreply;
	int  dlgreply, lastbutton;
};

#endif
