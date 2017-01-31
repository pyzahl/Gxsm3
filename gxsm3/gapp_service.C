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


#include <config.h>

#include <gtk/gtk.h>

// Gxsm headers 
#include "gxsm_app.h"
#include "gxsm_window.h"

#include "gapp_service.h"
#include "pcs.h"
#include "glbvars.h"


// ============================================================
// MyGnomeTools
// ============================================================



GMenuModel *MyGnomeTools::find_extension_point_section (GMenuModel  *model,
                                                        const gchar *extension_point)
{
        const gint dbg=0;
        gint i, n_items;
        GMenuModel *section = NULL;

        if (model == NULL)
                return NULL;

        if (dbg > 0) g_print ("MyGnomeTools::find_extension_point_section '%s' in menu.\n", extension_point);

        n_items = g_menu_model_get_n_items (model);

        if (dbg > 0) g_print ("MyGnomeTools::find_extension_point_section n_items=%d\n", n_items);

        for (i = 0; i < n_items && !section; i++) {
                gchar *id = NULL;
                
                gboolean ret=g_menu_model_get_item_attribute (model, i, "id", "s", &id);
                if (dbg > 0) g_print ("MyGnomeTools::find_extension_point_section i=%d  id: %s %s\n", i, id, ret?"YES":"NO");
                if (id) { g_free (id); } id = NULL;
                gboolean retx1=g_menu_model_get_item_attribute (model, i, "name", "s", &id);
                if (dbg > 0) g_print ("MyGnomeTools::find_extension_point_section i=%d name: %s %s\n", i, id, retx1?"YES":"NO");
                if (id) { g_free (id); } id = NULL;
                gboolean retx2=g_menu_model_get_item_attribute (model, i, "label", "s", &id);
                if (dbg > 0) g_print ("MyGnomeTools::find_extension_point_section i=%d label: %s %s\n", i, id, retx2?"YES":"NO");
                if (id) { g_free (id); } id = NULL;


                if (ret){
                        if (g_menu_model_get_item_attribute (model, i, "id", "s", &id) &&
                            strcmp (id, extension_point) == 0) {
                                section = g_menu_model_get_item_link (model, i, G_MENU_LINK_SECTION); 
                                if (id) g_free (id);
                                return section;
                        }
                }
                
                GMenuModel *subsection;
                GMenuModel *submenu;
                gint j, j_items;
                
                if (dbg > 0) g_print ("MyGnomeTools::find_extension_point_section subsecton lookup\n");

                subsection = g_menu_model_get_item_link (model, i, G_MENU_LINK_SECTION);
                if (subsection == NULL){ // try here:
                        submenu = g_menu_model_get_item_link (model, i, G_MENU_LINK_SUBMENU);
                        if (submenu)
                                section = find_extension_point_section (submenu, extension_point);
                } else {
                        //                        if (subsection) {
                        j_items = g_menu_model_get_n_items (subsection);
                        if (dbg > 0) g_print ("MyGnomeTools::find_extension_point_section subsecton lookup -- j_items=%d\n", j_items);
                        
                        for (j = 0; j < j_items && !section; j++) {
                                if (dbg > 0) g_print ("MyGnomeTools::find_extension_point_section submenu lookup -- j_items=%d\n", j);
                                submenu = g_menu_model_get_item_link (subsection, j, G_MENU_LINK_SUBMENU);
                                if (submenu)
                                        section = find_extension_point_section (submenu, extension_point);
                        }
                }
        }

        return section;
}

GtkWidget* MyGnomeTools::mygtk_grid_add_input(const gchar* labeltxt, GtkWidget* grid, int  &x, int &y, int nx, GtkWidget *opt_label_widget, GSList **l){
        GtkWidget *label, *entry;

        if (labeltxt){
                XSM_DEBUG_GP (DBG_L11, "MyGnomeTools::mygtk_grid_add_input label='%s' %d %d\n", labeltxt, x, y);
                label = gtk_label_new (labeltxt);
                gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
                gtk_grid_attach (GTK_GRID (grid), label, x++, y, 1, 1);
                gtk_widget_show (label);
                if (l)
                        *l = g_slist_prepend (*l, label);
        }

        if (opt_label_widget){
                gtk_grid_attach (GTK_GRID (grid), opt_label_widget, x++, y, 1, 1);
                gtk_widget_show (opt_label_widget);
                if (l)
                        *l = g_slist_prepend (*l, opt_label_widget);
        }
        
	entry = gtk_entry_new ();
	gtk_widget_show (entry);
        gtk_grid_attach (GTK_GRID (grid), entry, x, y, nx, 1); x+=nx;
        if (l)
                *l = g_slist_prepend (*l, entry);
  
	return entry;
}

GtkWidget* MyGnomeTools::SetupScale(GtkAdjustment *adj, GtkWidget *grid, int &x, int &y, int nx){
	GtkWidget *skl = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL,  GTK_ADJUSTMENT(adj));
	gtk_scale_set_value_pos (GTK_SCALE (skl), GTK_POS_LEFT);
	gtk_scale_set_draw_value(GTK_SCALE(skl), FALSE);
        gtk_widget_set_hexpand (skl, TRUE);
	gtk_widget_show (skl); 
	gtk_grid_attach (GTK_GRID (grid), skl, x++, y, nx,1);
	return skl;
}

GtkWidget* MyGnomeTools::mygtk_grid_add_spin_input (const gchar *labeltxt, GtkWidget *grid, int &x, int &y, int nx, GtkWidget* opt_label_widget, GSList **l){
	GtkAdjustment *adjust;
	GtkWidget *spin;
	GtkWidget *label;

        if (labeltxt){
                label = gtk_label_new (labeltxt);
                gtk_widget_show (label);
                gtk_grid_attach (GTK_GRID (grid), label, x++, y, 1,1);
                gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
                if (l)
                        *l = g_slist_prepend (*l, label);
        }
        if (opt_label_widget){
                gtk_grid_attach (GTK_GRID (grid), opt_label_widget, x++, y, 1, 1);
                gtk_widget_show (opt_label_widget);
                if (l)
                        *l = g_slist_prepend (*l, opt_label_widget);
        }
        
	adjust = gtk_adjustment_new (0., -10., 10., 1., 10., 10.);
	spin   = gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 1., 0);
	gtk_widget_show (spin);
        if (l)
                *l = g_slist_prepend (*l, spin);
        
        gtk_grid_attach (GTK_GRID (grid), spin, x, y, nx,1); x+=nx;
  
	g_object_set_data( G_OBJECT (spin), "Adjustment", adjust);
  
	return spin;
}



// >>>>>>>>>>>> ************* START ALL OBSOLETE BELOW !!!!!!!!!!!!!!!!!!!!!!
#if 0
GtkWidget* MyGnomeTools::mygtk_create_input(const gchar* labeltxt,
					    GtkWidget* vbox,
					    GtkWidget* &hbox, 
					    int lsize, 
					    int esize, 
					    GtkWidget* opt_label_widget){
        GtkWidget *label, *entry;
	label = opt_label_widget;

        XSM_DEBUG_GP (DBG_L8,"GXSM: mygtk_create_input: %s\n", labeltxt); // used to locate trouble
        std::cout << "OBSOLETE WARNING for " << labeltxt << ": MyGnomeTools::mygtk_create_input(.. vbox, hbox ..)\n"
                  << "Please use the new version: MyGnomeTools::mygtk_create_input(const gchar* labeltxt, GtkWidget* grid, int  &x, int &y...)."
                  << std::endl;
        
	if (vbox){
		hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_widget_show (hbox);
		//  gtk_container_add (GTK_CONTAINER (vbox), hbox);
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

		if (!label)
			label = gtk_label_new (labeltxt);

		gtk_widget_show (label);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, lsize ? TRUE:FALSE, lsize ? 0:GXSM_WIDGET_PAD);

		if (lsize)  
			gtk_widget_set_size_request (label, lsize, -1);
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

		if (esize == -1) 
			return NULL;
	}

	entry = gtk_entry_new ();
	if (esize){
		gtk_entry_set_width_chars (GTK_ENTRY (entry), (gint)(esize/10));
		gtk_entry_set_max_width_chars (GTK_ENTRY (entry), (gint)(esize/10));
                gtk_widget_set_size_request (entry, esize, -1);
        }
	gtk_widget_show (entry);
	gtk_container_add (GTK_CONTAINER (hbox), entry);
  
	return entry;
}


GtkWidget* MyGnomeTools::mygtk_add_input(GtkWidget *hbox, int esize){
	GtkWidget *entry;

        std::cout << "OBSOLETE WARNING: MyGnomeTools::mygtk_add_input(..  hbox ..)\n"
                  << "Please use the new version: MyGnomeTools::mygtk_add_input(GtkWidget *grid, int &x, int &y, int esize)."
                  << std::endl;

	entry = gtk_entry_new ();
	if (esize){
		gtk_entry_set_width_chars (GTK_ENTRY (entry), (gint)(esize/10));
		gtk_entry_set_max_width_chars (GTK_ENTRY (entry), (gint)(esize/10));
                gtk_widget_set_size_request (entry, esize, -1);
        }
	gtk_widget_show (entry);
	gtk_container_add (GTK_CONTAINER (hbox), entry);
  
	return entry;
}


GtkWidget* MyGnomeTools::mygtk_add_input(const gchar *labeltxt,
					 GtkWidget *hbox, 
					 int lsize, 
					 int esize){
	GtkWidget *label, *entry;

        std::cout << "OBSOLETE WARNING for " << labeltxt << ": MyGnomeTools::mygtk_add_input(..  hbox ..)\n"
                  << "Please use the new version: MyGnomeTools::mygtk_add_input(GtkWidget *grid, int &x, int &y, int esize)."
                  << std::endl;

	label = gtk_label_new (labeltxt);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, lsize ? TRUE:FALSE, lsize ? 0:GXSM_WIDGET_PAD);
	if (lsize)
		gtk_widget_set_size_request (label, lsize, -1);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

	entry = gtk_entry_new ();
	if (esize){
		gtk_entry_set_width_chars (GTK_ENTRY (entry), (gint)(esize/10));
		gtk_entry_set_max_width_chars (GTK_ENTRY (entry), (gint)(esize/10));
                gtk_widget_set_size_request (entry, esize, -1);
        }
	gtk_widget_show (entry);
	gtk_container_add (GTK_CONTAINER (hbox), entry);
  
	return entry;
}

GtkWidget* MyGnomeTools::SetupScale(GtkAdjustment *adj, 
				    GtkWidget *hbox, 
				    int sklsize){
        std::cout << "OBSOLETE WARNING: MyGnomeTools::SetupScale(.. hbox ..)\n"
                  << "Please use the new version: MyGnomeTools::SetupScale(.. grid ..)."
                  << std::endl;
	GtkWidget *skl = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL,  GTK_ADJUSTMENT(adj));
	if (sklsize)
		gtk_widget_set_size_request (skl, sklsize, -1);
        else
                gtk_widget_set_hexpand (skl, TRUE);

	gtk_scale_set_value_pos (GTK_SCALE (skl), GTK_POS_LEFT);
	gtk_scale_set_draw_value(GTK_SCALE(skl), FALSE);
	gtk_widget_show (skl); 
	gtk_container_add (GTK_CONTAINER (hbox), skl);
	return skl;
}


GtkWidget* MyGnomeTools::mygtk_create_spin_input(const gchar* labeltxt, GtkWidget* vbox,
						 GtkWidget* &hbox, int lsize, int esize, GtkWidget* opt_label_widget){
        std::cout << "OBSOLETE WARNING for " << labeltxt << ": MyGnomeTools::mygtk_create_spin_input(.. vbox, hbox ..)\n"
                  << "Please use the new version: MyGnomeTools::mygtk_add_spin_input(const gchar* labeltxt, GtkWidget* grid, int &x, int &y, ..)."
                  << std::endl;
	mygtk_create_input(labeltxt, vbox, hbox, lsize, -1, opt_label_widget);
	return mygtk_add_spin (hbox);
}


GtkWidget* MyGnomeTools::mygtk_add_spin (GtkWidget *hbox, 
					 int lsize,
					 int esize){
	GtkAdjustment *adjust;
        GtkWidget *spin;
        std::cout << "OBSOLETE WARNING: MyGnomeTools::mygtk_add_spin(.. hbox ..)\n"
                  << "Please use the new version: MyGnomeTools::mygtk_add_spin_g(GtkWidget* grid, int &x, int &y)."
                  << std::endl;
        
	adjust = gtk_adjustment_new (0., -10., 10., 1., 10., 10.);
#define USE_SPIN_BUTTON
#ifdef USE_SPIN_BUTTON
	spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 1., 0);
#else
        static const gchar *icons[] = {
                "audio-volume-muted-symbolic",
                "audio-volume-high-symbolic",
                "audio-volume-low-symbolic",
                "audio-volume-medium-symbolic"
        };
        spin = gtk_scale_button_new (GTK_ICON_SIZE_MENU, -10., 10., 0.1, icons);
        //        GTK_ICON_SIZE
        //        gdouble min,
        //        gdouble max,
        //        gdouble step,
        //        const gchar **icons);
        gtk_scale_button_set_adjustment ( GTK_SCALE_BUTTON (spin), adjust);
#endif
	gtk_widget_show (spin);
	gtk_container_add (GTK_CONTAINER (hbox), spin);
  
	g_object_set_data( G_OBJECT (spin), "Adjustment", adjust);
	if (esize)
		gtk_widget_set_size_request (spin, esize, -1);
  
	return spin;
}

GtkWidget* MyGnomeTools::mygtk_add_spin (const gchar *labeltxt, GtkWidget *hbox, 
					int lsize, int esize){
	GtkAdjustment *adjust;
	GtkWidget *spin;
	GtkWidget *label;
        std::cout << "OBSOLETE WARNING for " << labeltxt << ": MyGnomeTools::mygtk_add_spin(.. hbox ..)\n"
                  << "Please use the new version: MyGnomeTools::mygtk_add_spin_g(GtkWidget *grid, int &x, int &y)."
                  << std::endl;

	label = gtk_label_new (labeltxt);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, lsize ? TRUE:FALSE, lsize ? 0:GXSM_WIDGET_PAD);
	if (lsize)
		gtk_widget_set_size_request (label, lsize, -1);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

	adjust = gtk_adjustment_new (0., -10., 10., 1., 10., 10.);
	spin   = gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 1., 0);
	gtk_widget_show (spin);
	gtk_container_add (GTK_CONTAINER (hbox), spin);
  
	g_object_set_data( G_OBJECT (spin), "Adjustment", adjust);
  
	return spin;
}


GtkWidget* MyGnomeTools::mygtk_add_list(const gchar *labeltxt, 
					GtkWidget *hbox, 
					int lsize, 
					int esize){
        GtkWidget *label;
	GtkWidget *list;

	label = gtk_label_new (labeltxt);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, lsize ? TRUE:FALSE, lsize ? 0:GXSM_WIDGET_PAD);
	if (lsize)
		gtk_widget_set_size_request (label, lsize, -1);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);

	list  = gtk_tree_view_new ();
	gtk_widget_show (list);
	gtk_container_add (GTK_CONTAINER (hbox), list);
  
	return list;
}
#endif
// <<<<<<<<<<<<<< ********************* END OBSOLETE UTILS !!!!!!!!!!!!!!!!!!!!!!!!



// ============================================================
// GnomeAppService
// ============================================================


gint GnomeAppService::setup_multidimensional_data_copy (const gchar *title, Scan *src, int &ti, int &tf, int &vi, int &vf,
                                                        int *tnadd, int *vnadd, int *crop_window_xy, gboolean crop){
	UnitObj *Pixel = new UnitObj("Pix","Pix");
	UnitObj *Unity = new UnitObj(" "," ");
	GtkWidget *dialog = gtk_dialog_new_with_buttons (N_(title),
							 window, 
							 (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
							 _("_OK"), GTK_RESPONSE_ACCEPT,
							 _("_CANCEL"), GTK_RESPONSE_CANCEL,
							 NULL); 
        
	BuildParam bp;
        gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), bp.grid);

        bp.grid_add_label ("New Time and Layer bounds:"); bp.new_line ();

	gint t_max = src->number_of_time_elements ()-1;
	if (t_max < 0) t_max=0;

        bp.grid_add_ec ("t-inintial",  Unity, &ti, 0, t_max, ".0f"); bp.new_line ();
        bp.grid_add_ec ("t-final",     Unity, &tf, 0, t_max, ".0f"); bp.new_line ();
	if (tnadd){
		bp.grid_add_ec ("t-#add",  Unity, tnadd, 1, t_max, ".0f"); bp.new_line ();
	}
        bp.grid_add_ec ("lv-inintial", Unity, &vi, 0, src->mem2d->GetNv (), ".0f"); bp.new_line ();
        bp.grid_add_ec ("lv-final",    Unity, &vf, 0, src->mem2d->GetNv (), ".0f"); bp.new_line ();

	if (vnadd){
		bp.grid_add_ec ("lv-#add",  Unity, vnadd, 1, src->mem2d->GetNv (), ".0f"); bp.new_line ();
	}

	if (crop && crop_window_xy){
                bp.grid_add_label ("Crop window bounds:"); bp.new_line ();
                bp.grid_add_ec ("X-left",  Pixel, &crop_window_xy[0], 0, src->mem2d->GetNx ()-1, ".0f"); bp.new_line ();
                bp.grid_add_ec ("Y-top",   Pixel, &crop_window_xy[1], 0, src->mem2d->GetNx ()-1, ".0f"); bp.new_line ();
                bp.grid_add_ec ("X-right", Pixel, &crop_window_xy[2], 0, src->mem2d->GetNx ()-1, ".0f"); bp.new_line ();
                bp.grid_add_ec ("Y-bottom",Pixel, &crop_window_xy[3], 0, src->mem2d->GetNx ()-1, ".0f"); bp.new_line ();
	}

        // bp.show_all ();
	gtk_widget_show_all (dialog);
	gint ret = gtk_dialog_run (GTK_DIALOG(dialog));

	gtk_widget_destroy (dialog);

	delete Pixel;
	delete Unity;

        return ret;
}


int GnomeAppService::choice(const char *s1, const char *s2, const char *s3, int numb, const char *b1, const char *b2, const char *b3, int def){
	static gchar *s = NULL;
	if(s){ g_free(s); s=NULL; }
	if(!s1 && !s2) return 0;
	s = g_strconcat( s2, "\n", s3, NULL);
	return dialog(s1, s, b1, b2, b3, TRUE);
}

int GnomeAppService::dialog(const char *title, const char *content, 
			    const char *b1, const char *b2, const char *b3, 
			    int wait){
	GtkWidget *label = gtk_label_new (N_(content));
	gtk_widget_show (label);
	
	GtkWidget *dialog = gtk_dialog_new_with_buttons (N_(title),
							 window,
							 (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
							 N_(b1), 1,
							 N_(b2), 2,
							 N_(b3), 3,
							 NULL);

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                            label, TRUE, TRUE, GXSM_WIDGET_PAD);

	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
  	gtk_widget_destroy (dialog);
	return result;
}

GtkWidget* GnomeAppService::progress_info_new (const gchar *title, gint levels, GCallback cancel_cb, gpointer data, gboolean modal){
	static gint last_levels=0;
	static GCallback last_cancel_cb=NULL;
	static gpointer last_data=NULL;

	progress_dialog_schedule_close = 0;

	if (progress_dialog && (last_levels != levels || last_cancel_cb != cancel_cb || last_data != data))
		progress_info_destroy_now();

	last_levels = levels;
	last_cancel_cb = cancel_cb;
	last_data = data;

	if (!progress_dialog){
                progress_dialog = gtk_dialog_new_with_buttons (N_(title?title:"Progress"),
                                                               gapp->get_window (), 
                                                               (GtkDialogFlags)((modal ? GTK_DIALOG_MODAL:0) | GTK_DIALOG_DESTROY_WITH_PARENT),
                                                               NULL, NULL, NULL);
                //_("_Cancel"),
                // GTK_RESPONSE_REJECT,
                // NULL);

                //progress_dialog = gtk_dialog_new ();

		for (int i=0; i<MAX_PROGRESS_LEVELS; ++i)
			progress_bar[i]	= NULL;
	} else {
                gtk_window_set_title (GTK_WINDOW (progress_dialog), N_(title?title:"Progress"));
        }

	//	Add GtkProgressBar

	if (levels>0)
		for (int i=0; i<levels && i<MAX_PROGRESS_LEVELS; ++i){

			if (!progress_bar[i]){
				progress_bar[i]	= gtk_progress_bar_new ();
				gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (progress_dialog))), progress_bar[i], TRUE, TRUE, GXSM_WIDGET_PAD);
                                gtk_progress_bar_set_ellipsize (GTK_PROGRESS_BAR (progress_bar[i]), PANGO_ELLIPSIZE_START);
			}
		}

	if (cancel_cb){
                GtkWidget* button = gtk_dialog_add_button (GTK_DIALOG (progress_dialog), N_("Cancel"), 100);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (cancel_cb), data);
        }

	gtk_widget_show_all (progress_dialog);
	check_events();
	return progress_dialog;
}

int GnomeAppService::progress_info_set_bar_fraction (gdouble fraction, gint level){
	if (!progress_dialog) return -1;
	if (level<1 || level > MAX_PROGRESS_LEVELS) return -1;

	if (progress_bar[level-1] && fraction >= 0.)
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar[level-1]), fraction);

	check_events();
	return 0;
}

int GnomeAppService::progress_info_set_bar_pulse (gint level, gdouble fraction){
	if (!progress_dialog) return -1;
	if (level<1 || level > MAX_PROGRESS_LEVELS) return -1;

	if (progress_bar[level-1]){
		if (fraction >= 0.)
			gtk_progress_bar_set_pulse_step (GTK_PROGRESS_BAR (progress_bar[level-1]), fraction);
		else
			gtk_progress_bar_pulse (GTK_PROGRESS_BAR (progress_bar[level-1]));
		check_events();
	}
	return 0;
}

int GnomeAppService::progress_info_set_bar_text (const gchar* text, gint level){
	if (!progress_dialog) return -1;
	if (level<1 || level > MAX_PROGRESS_LEVELS) return -1;

	if (progress_bar[level-1]){
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progress_bar[level-1]), text);
                gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR (progress_bar[level-1]), TRUE);
        }

	check_events();
	return 0;
}

int GnomeAppService::progress_info_add_info (const gchar* info){
	if (!progress_dialog)
		return -1;

	GtkWidget *label = gtk_label_new (N_(info));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (progress_dialog))), label, 
                            TRUE, TRUE, GXSM_WIDGET_PAD);

	check_events ();
	return 0;
}

static guint gas_close_progress (GnomeAppService *gas){
	if (gas->progress_info_close_scheduled () > 1){
		gas->progress_info_close_schedule_dec ();
		return TRUE;
	}
	if (gas->progress_info_close_scheduled () == 1)
		gas->progress_info_destroy_now();
	return FALSE; // terminate timeout call
}

void GnomeAppService::progress_info_destroy_now(){
	if (progress_dialog){
		progress_dialog_schedule_close = 0;
		gtk_widget_destroy (progress_dialog);
		progress_dialog=NULL;
	}
}

// schedules dealyed progress infor window destruction, it may be reused automatically if more similar progress requests
void GnomeAppService::progress_info_close (){
	if (progress_dialog){
		progress_dialog_schedule_close = 2;
		g_timeout_add (1000, (GSourceFunc)gas_close_progress, this);
	}
}

/*
  must g_free returned file name!
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "all");
  gtk_file_filter_add_pattern (filter, "*");
*/
gchar *GnomeAppService::file_dialog_save (const gchar *title, 
                                          const gchar *path, 
                                          const gchar *name,
                                          GtkFileFilter **filter
                                          ){
        gchar *filename = NULL;
        GtkWidget *dialog = gtk_file_chooser_dialog_new (title,
                                                         window, // parent_window,
                                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                                         N_("_Cancel"), GTK_RESPONSE_CANCEL,
                                                         N_("_Save"), GTK_RESPONSE_ACCEPT,
                                                         NULL);

        gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

        if (path){
                gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), path);
        }
        
        if (name){
                gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), name);
                gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), name);
        }

        if (filter)
                while (*filter)
                        gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), *filter++);

        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT){
                filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        }

        gtk_widget_destroy (dialog);
        return filename; // must g_free is non NULL
}

gchar *GnomeAppService::file_dialog_load (const gchar *title, 
                                          const gchar *path, 
                                          const gchar *name,
                                          GtkFileFilter **filter
                                          ){
        gchar *filename = NULL;
        GtkWidget *dialog = gtk_file_chooser_dialog_new (title,
                                                         window, // parent_window,
                                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                                         N_("_Cancel"), GTK_RESPONSE_CANCEL,
                                                         N_("_Load"), GTK_RESPONSE_ACCEPT,
                                                         NULL);
        if (path){
                gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), path);
        }

        if (name){
                gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), name);
                gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), name);
        }

        if (filter)
                while (*filter)
                        gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), *filter++);
        
        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
                filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        
        gtk_widget_destroy (dialog);
        return filename;
}

void  GnomeAppService::string_cb (gchar *string, gpointer data){
}

void GnomeAppService::destroy (GtkWidget *widget, GnomeAppService *p){
	if(p->fdlg == -1)
		p->fname=NULL;
	p->fdlg=0;
}

int GnomeAppService::check_file(gchar *fn){
	int r = 3;
	while (fn && r==3){
		std::ifstream f;
		f.open(fn, std::ios::in);
		if(f.good()){
			f.close();
			if((r=choice(WRN_WARNING, fn, WRN_FILEEXISTS, 2, L_CANCEL, L_OVERWRITE, L_RETRY, 1)) == 1)
				return FALSE;
			else if(r == 2)
				return TRUE;
		}
		else 
			return TRUE;
	}
	return FALSE;
}

void GnomeAppService::ValueRequest(const gchar *title, const gchar *label, const gchar *infotxt, 
				   UnitObj *uobj, double minv, double maxv, const gchar *vfmt,
				   double *value){
	GtkWidget *dialog = gtk_dialog_new_with_buttons (N_(title),
							 window,
							 (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
							 _("_OK"), GTK_RESPONSE_ACCEPT,
							 NULL);
       
        BuildParam bp;

	gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), bp.grid);

        bp.grid_add_ec (infotxt, uobj, value, minv, maxv, vfmt, 0.1, 1.0);
	bp.show_all ();

	gtk_widget_show(dialog);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);
}


gint GnomeAppService::terminate_timeout_func (gpointer data){
        gchar *m  = (gchar*)g_object_get_data (G_OBJECT (data), "SM");
        gint c = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (data), "CM"));
        --c; // c=0 if dialog closed, will terminate rigth away.
        if (c > 0){
                gchar *message = g_strdup_printf ("%s\nTerminating in %d s.", m, c);
                // g_message (message);
                g_object_set_data (G_OBJECT (data), "CM", GINT_TO_POINTER (c));
                gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (data), message);
                gtk_window_present (GTK_WINDOW (data));
                g_free (message);
                return true;
        }
        if (m) g_free (m);
        g_critical ("GXSM is terminating now due to DSP software version mismatch. Please update DSP.");
        exit (-1);
        return false;
}

void GnomeAppService::alert(const gchar *s1, const gchar *s2, const gchar *s3, int c){
        if(window){
                GtkWidget *dialog = gtk_message_dialog_new_with_markup (window,
                                                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                                                        GTK_MESSAGE_WARNING,
                                                                        GTK_BUTTONS_CLOSE,
                                                                        "<span foreground='red' size='large' weight='bold'>%s</span>\n%s\n%s", s1, s2, s3);
                g_signal_connect_swapped (G_OBJECT (dialog), "response",
                                          G_CALLBACK (gtk_widget_destroy),
                                          G_OBJECT (dialog));

                gtk_widget_show (dialog);

                if (c > 5){
                        g_message ("adding timeout for forced exit");
                        g_object_set_data (G_OBJECT (dialog), "SM", (gpointer)g_strdup_printf ("<span foreground='red' size='large' weight='bold'>%s</span>\n%s\n%s", s1, s2, s3));
                        g_object_set_data (G_OBJECT (dialog), "CM", GINT_TO_POINTER (c));
                        g_timeout_add ((guint)1000, 
                                       (GSourceFunc) GnomeAppService::terminate_timeout_func, 
                                       dialog
                                       );
                }
        }
}


// ============================================================
// AppBase
// ============================================================

AppBase::AppBase(){ 
	XSM_DEBUG(DBG_L2, "AppBase" ); 
        app_window = NULL;
        window = NULL;
	window_key=NULL;
        window_geometry=NULL;
	showstate=FALSE; 
	nodestroy=FALSE;
        geometry_settings=g_settings_new (GXSM_RES_BASE_PATH_DOT".window-geometry");
}

AppBase::~AppBase(){ 
	XSM_DEBUG (DBG_L2, "AppBase::~AppBase destructor for window '" << (window_key?window_key:"--") << "'."); 

	if(window_key){ // autosave show/hide state
		SaveGeometry ();
                // remove from menu!
	}
        
	if(!nodestroy){
		XSM_DEBUG_GP (DBG_L2, "~AppBase -- calling widget destroy for window '%s'.",  (window_key?window_key:"--")); 
		destroy();
	}
        
        if (window_key)
                g_free(window_key);

        if (window_geometry)
                g_free (window_geometry);

	XSM_DEBUG (DBG_L2, "AppBase::~AppBase done." );
}

void AppBase::AppWindowInit(const gchar *title){
	XSM_DEBUG(DBG_L2, "AppBase::WidgetInit: " << title );

        app_window =  gxsm3_app_window_new (GXSM3_APP (gapp->get_application ()));    // gtk_application_window_new (GTK_APPLICATION (gapp->get_application ()));
        window = GTK_WINDOW (app_window);

        //window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
        header_bar = gtk_header_bar_new ();
        gtk_widget_show (header_bar);

        // gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), true);
        // gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), GtkWidget *child);

        gtk_window_set_title (GTK_WINDOW (window), title);
        gtk_header_bar_set_title ( GTK_HEADER_BAR (header_bar), title);
        // gtk_header_bar_set_subtitle (GTK_HEADER_BAR  (header_bar), title);

        gtk_window_set_titlebar (GTK_WINDOW (window), header_bar);

	v_grid = gtk_grid_new ();
	g_object_set_data (G_OBJECT (window), "v_grid", v_grid);
        gtk_container_add (GTK_CONTAINER (window), v_grid);

	gtk_widget_show_all (GTK_WIDGET (window));

        g_signal_connect (G_OBJECT (window), "window-state-event", G_CALLBACK (AppBase::window_state_watch_callback), this);
        g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (AppBase::window_close_callback), this);
        //        g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
}

gboolean AppBase::window_close_callback (GtkWidget *widget,
                                         GdkEvent  *event,
                                         gpointer   user_data){
        AppBase *app_w = (AppBase *)user_data;
        // dop NOT close/destrry, just minimize/hide!
        app_w->hide ();

        return true; // no further actions!!
}

gboolean AppBase::window_state_watch_callback (GtkWidget *widget,
                                               GdkEvent  *event,
                                               gpointer   user_data){
        AppBase *app_w = (AppBase *)user_data;

        GdkEventWindowState *ws = (GdkEventWindowState *)event;

	XSM_DEBUG (DBG_L2, "AppBase::window_state_watch_callback: window_new_state=" << ws->new_window_state);

        switch (ws->new_window_state){
        case GDK_WINDOW_STATE_ICONIFIED: app_w->showstate=FALSE; break;
        case GDK_WINDOW_STATE_MAXIMIZED: break;
        case GDK_WINDOW_STATE_STICKY: break;
        case GDK_WINDOW_STATE_FULLSCREEN: break;
        case GDK_WINDOW_STATE_ABOVE: break;
        case GDK_WINDOW_STATE_BELOW: break;
        case GDK_WINDOW_STATE_FOCUSED: break;
        case GDK_WINDOW_STATE_TILED: break;
        case GDK_WINDOW_STATE_WITHDRAWN: break;
        }
        
        return false;
}
        
void AppBase::window_action_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
        AppBase *app_w = (AppBase *)user_data;
        app_w->show ();
}

void AppBase::add_window_to_window_menu(const gchar *menu_item_label, const gchar* key){
        const gchar *menusection = "windows-section";
        
        if (strlen(menu_item_label) < 1)
                return;

        // generate valid action string from menu path
        gchar *tmp = g_strconcat (menusection, "-show-", key, NULL);
        g_strdelimit (tmp, " /:", '-');
        gchar *tmpaction = g_strconcat ( tmp, NULL);
        GSimpleAction *ti_action;
        
        XSM_DEBUG_GP (DBG_L2, "AppBase::add_window_to_window_menu <%s>  MenuItem= %s ==> label, generated action: [%s] <%s>\n", menusection, menu_item_label, tmp, tmpaction );

        if (!strcmp (menusection, "windows-section-xx")) { // add toggle -- testing, not yet working -- disabled via -xx
                ti_action = g_simple_action_new_stateful (tmpaction,
                                                          G_VARIANT_TYPE_BOOLEAN,
                                                          g_variant_new_boolean (true));
                g_signal_connect (ti_action, "toggled", G_CALLBACK (AppBase::window_action_callback), this); // GTK_APPLICATION ( gapp->get_application ()));
        } else {
                ti_action = g_simple_action_new (tmpaction, NULL);
                g_signal_connect (ti_action, "activate", G_CALLBACK (AppBase::window_action_callback), this);
        }
        
        g_object_set_data (G_OBJECT (ti_action), "AppBase", this);

        g_action_map_add_action (G_ACTION_MAP ( gapp->get_application ()), G_ACTION (ti_action));

        gchar *app_tmpaction = g_strconcat ( "app.", tmpaction, NULL);

#if 1 // pretty rewrite name
        gchar *label = g_strdelimit (g_strdup (menu_item_label), "-:._/", ' ');
        gchar *p = label;
        *p = g_ascii_toupper (*p); ++p;
        for (; *p; ++p)
                if (*p == ' ' && *(p+1))
                        *(p+1) = g_ascii_toupper (*(p+1));
#else
        gchar *label = g_strdup (menu_item_label);
#endif
        gapp->gxsm_app_extend_menu (menusection, label, app_tmpaction);

        g_free (label);
        g_free (app_tmpaction);
        g_free (tmpaction);
        g_free (tmp);
}

int AppBase::set_window_geometry (const gchar *key, gint index){
        XSM_DEBUG_GP (DBG_L4, "AppBase::set_window_geometry and append '%s' to Windows Menu.\n", key);

        if (window_key){
                XSM_DEBUG_GP (DBG_L2, "AppBase::set_window_geometry geometry already setup. DUPLICATE WARNING append '%s' to Windows Menu already done.\n", key);
                // remove from menu!
                g_free (window_key);
        }
        
        XSM_DEBUG_GP (DBG_L9, "AppBase::set_window_geometry and append '%s' to Windows Menu\n", key);
        if (index >= 0 && index <= 4) // limit for now
                window_key = g_strdup_printf ("%s-%d", key, index);
        else if (index > 4)
                return -1; // do not handle at this time.
        else
                window_key = g_strdup (key);

        XSM_DEBUG_GP (DBG_L9, "AppBase::set_window_geometry and append '%s' to Windows Menu -- cpy2.\n", window_key);

	LoadGeometry ();

        XSM_DEBUG_GP (DBG_L9, "AppBase::set_window_geometry and append '%s' to Windows Menu -- add to menu.\n", window_key);

        add_window_to_window_menu (window_key, window_key);

        XSM_DEBUG_GP (DBG_L9, "AppBase::set_window_geometry and append '%s' to Windows Menu -- done.\n", window_key);
	return 0;
}

void AppBase::hide (){
        gtk_window_iconify (window);
        showstate=FALSE;
        SaveGeometry ();
}

void AppBase::show (){
        gtk_window_deiconify (window);
        gtk_window_present (window);
        showstate=TRUE;
        position_auto ();
        resize_auto ();
        SaveGeometry ();
}

void AppBase::show_auto (){
        if (window_geometry){
                if (window_geometry[WGEO_FLAG] && window_geometry[WGEO_SHOW]){
                        show ();
                } else
                        hide ();
        } else
                hide ();
}

void AppBase::position_auto (){
        if (window_geometry){
                if (window_geometry[WGEO_FLAG]){
                        gtk_window_move (GTK_WINDOW (window), window_geometry[WGEO_XPOS], window_geometry[WGEO_YPOS]);
#if 0
                        GdkDisplay *display = gdk_display_get_default();
                        GdkScreen *screen = gdk_display_get_default_screen(display);

                        // ==> consider gdk_monitor_get_workarea (), ...
                        gint screenWidth = gdk_screen_get_width(screen);
                        gint screenHeight = gdk_screen_get_height(screen);
                        
                        /* reposition the window but make sure we're not putting it off the
                         * screen. If so, reset to default values. */
                        
                        if(window_geometry[WGEO_XPOS] >= 0 && window_geometry[WGEO_YPOS] >= 0){
                                gtk_window_move (GTK_WINDOW (window), window_geometry[WGEO_XPOS], window_geometry[WGEO_YPOS]);
                        }
#endif
                }
        }
}

void AppBase::resize_auto (){
        if (window_geometry){
                if (window_geometry[WGEO_FLAG]){
                        gtk_window_resize (GTK_WINDOW (window), window_geometry[WGEO_WIDTH], window_geometry[WGEO_HEIGHT]);
#if 0
                        GdkDisplay *display = gdk_display_get_default();
                        GdkScreen *screen = gdk_display_get_default_screen(display);
                        gint screenWidth = gdk_screen_get_width(screen);
                        gint screenHeight = gdk_screen_get_height(screen);
                        if (window_geometry[WGEO_WIDTH] > 0 && window_geometry[WGEO_HEIGHT] > 0
                            && window_geometry[WGEO_WIDTH] < screenWidth && window_geometry[WGEO_HEIGHT] < screenHeight)
                                gtk_window_resize (GTK_WINDOW (window), window_geometry[WGEO_WIDTH], window_geometry[WGEO_HEIGHT]);
#endif
                }
        }
}

void AppBase::SaveGeometryCallback(AppBase *apb){
	apb->SaveGeometry();
}

int AppBase::SaveGeometry(int savealways){
        if (! window_key){
                XSM_DEBUG_ERROR (DBG_L1, "ERROR ** AppBase::SaveGeometry called with no window_key.");
                return -1;
        }
	XSM_DEBUG (DBG_L2, "** AppBase::SaveGeometry of " << window_key);

        // just in case it was not loaded right
        if (!window_geometry)
                window_geometry = g_new (gint32, WGEO_SIZE);
        
        window_geometry[WGEO_FLAG] = 1;
        window_geometry[WGEO_SHOW] = showstate;
        gtk_window_get_position (GTK_WINDOW (window), &window_geometry[WGEO_XPOS], &window_geometry[WGEO_YPOS]); 
        gtk_window_get_size (GTK_WINDOW (window), &window_geometry[WGEO_WIDTH], &window_geometry[WGEO_HEIGHT]);

        GVariant *storage = g_variant_new_fixed_array (g_variant_type_new ("i"), window_geometry, WGEO_SIZE, sizeof (gint32));
        g_settings_set_value (geometry_settings, window_key, storage);
  
        //g_free (storage); // ??
      
	return 0;
}

int AppBase::LoadGeometry(){
        if (!window_key){
                XSM_DEBUG_ERROR (DBG_L1, "AppBase::LoadGeometry -- error, no window_key set.");
                return -1;
        }
	XSM_DEBUG (DBG_L2, "AppBase::LoadGeometry -- Load Geometry for window " << window_key );

        gsize n_stores;

        GVariant *storage = g_settings_get_value (geometry_settings, window_key);
        gint32 *tmp = (gint32*) g_variant_get_fixed_array (storage, &n_stores, sizeof (gint32));

        if (!window_geometry)
                window_geometry = g_new (gint32, WGEO_SIZE);
        memcpy (window_geometry, tmp, WGEO_SIZE*sizeof (gint32));

        // g_free (storage); // ??

        g_assert_cmpint (n_stores, ==, WGEO_SIZE);

        position_auto ();
#if 0
        resize_auto ();
#endif
        show_auto ();

	return 0;
}
