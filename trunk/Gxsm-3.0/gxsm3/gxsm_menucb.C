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
#include <glib.h>
#include <glib/gi18n.h>

#include "gnome-res.h"
#include "gxsm_app.h"

#include "dataio.h"
#include "util.h"
#include "version.h"
#include "glbvars.h"

#include "gxsm_resoucetable.h"

#include "action_id.h"

//#include "tips_dialog.h"

/* File ================================================== */

void App::file_open_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
	gapp->xsm->load();
	return;
}

void App::file_open_in_new_window_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
	if(!gapp->xsm->ActivateFreeChannel())
		gapp->xsm->load();
	return;
}

void App::file_browse_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
	gapp->file_dialog (
		N_("DAT/NC/HDF file to load"), NULL, 
		"*.[nNdDsSh][cCaApPd]*", NULL, "browseload",
		(GappBrowseFunc) App::browse_callback, 
		(gpointer) gapp);
	return;
}

void App::browse_callback(gchar *selection, App* ap){ 
	if(!gapp) return;
	XSM_DEBUG(DBG_L2, "browsed:" << selection );
	if(selection)
		ap->xsm->load(selection);
}

void App::file_set_datapath_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
	gapp->xsm->save(CHANGE_PATH_ONLY);
	return;
}

void App::file_set_probepath_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	XsmRescourceManager xrm("FilingPathMemory");
	gchar *path = xrm.GetStr ("Probe_DataSavePath", xsmres.DataPath);
	gchar *newpath = gapp->file_dialog(N_("Select path for probe data files"), path, 
					   "", "", "Probe_DataPath");
	g_free (path);
	if (newpath){
		xrm.Put ("Probe_DataSavePath", newpath);
	}
	return;
}

void App::file_save_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
	gapp->xsm->save(AUTO_NAME_SAVE);
	return;
}

void App::file_save_as_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
	gapp->xsm->save(MANUAL_SAVE_AS);
	return;
}

void App::file_print_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
#if 0
	if (gapp->PluginCallPrinter)
		(*gapp->PluginCallPrinter) (widget, data);
	else
		gapp->message(N_("Sorry, no 'Printer' plugin loaded!"));
#endif
	return;
}

void App::file_close_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	//  gapp->xsm->??
	return;
}


void App::file_quit_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
	if(gapp->question_yes_no (Q_WANTQUIT) == 1){
                GApplication *application = (GApplication *) user_data;
                delete gapp;
                g_application_quit (application);
	}
}

/* Edit ================================================== */

void App::edit_crop_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
	gapp->xsm->MathOperation(CropScan);
	return;
}

void App::edit_copy_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
	gapp->xsm->MathOperation(CopyScan);
	return;
}

/* Action ================================================== */


void App::action_toolbar_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
        const gchar *action = (const gchar*) g_object_get_data (G_OBJECT (simple), "toolbar_action");
        if (!action){
		XSM_DEBUG(DBG_L2, "Toolbar Plugin \"toolbar_action\" string is not set. (NULL)" );
                return;
        }
        if (!gapp) return;

        gapp->signal_emit_toolbar_action (action, simple);
        return;
}

/* View ================================================== */

void App::view_autodisp_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
	gapp->xsm->AutoDisplay();
	return;
}

void App::view_autozoom_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
#if 0
        //GTK3QQQ
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))){
		SET_FLAG(gapp->xsm->ZoomFlg, VIEW_ZOOM);
	}else{
		CLR_FLAG(gapp->xsm->ZoomFlg, VIEW_ZOOM);
	}
#endif
	return;
}

void App::view_tolerant_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
#if 0
        //GTK3QQQ
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))){
		SET_FLAG(gapp->xsm->ZoomFlg, VIEW_TOLERANT);
	}else{
		CLR_FLAG(gapp->xsm->ZoomFlg, VIEW_TOLERANT);
	}
#endif
	return;
}

void App::view_palette_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(!gapp) return;
#if 0
        //GTK3QQQ
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))){
		SET_FLAG(gapp->xsm->ZoomFlg, VIEW_PALETTE);
		SET_FLAG(gapp->xsm->ZoomFlg, VIEW_COLOR);
	}else{
		CLR_FLAG(gapp->xsm->ZoomFlg, VIEW_PALETTE);
		CLR_FLAG(gapp->xsm->ZoomFlg, VIEW_COLOR);
	}
#endif
	return;
}

void App::view_zoom_in_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gapp->xsm->MathOperation(ZoomInScan);
	return;
}

void App::view_zoom_out_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gapp->xsm->MathOperation(ZoomOutScan);
	return;
}

/* Math, Filter, ... ================================================== */

typedef gboolean  (*MOpND)(MATHOPPARAMSNODEST);
#define M_OPND(f)                    ((MOpND) (f))

typedef gboolean  (*MOp)(MATHOPPARAMS);
#define M_OP(f)                    ((MOp) (f))

typedef gboolean  (*MOpDO)(MATHOPPARAMDONLY);
#define M_OPDO(f)                    ((MOpDO) (f))

typedef gboolean  (*M2Op)(MATH2OPPARAMS);
#define M_2OP(f)                   ((M2Op) (f))

void App::math_onearg_nodest_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){ // gboolean (*MOp)(MATHOPPARAMSNODEST)
        gapp->xsm->MathOperationNoDest (M_OPND (g_object_get_data (G_OBJECT (simple), "plugin_MOp")));
	return;
}

void App::math_onearg_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){ // gboolean (*MOp)(MATHOPPARAMS)){
        gapp->xsm->MathOperation (M_OP (g_object_get_data (G_OBJECT (simple), "plugin_MOp")));
	return;
}

void App::math_onearg_dest_only_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){ // gboolean (*MOpDO)(MATHOPPARAMDONLY)){
        gapp->xsm->MathOperationS (M_OPDO (g_object_get_data (G_OBJECT (simple), "plugin_MOp")));
	return;
}

void App::math_onearg_for_all_vt_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){ // gboolean (*MOp)(MATHOPPARAMS)){
	gapp->xsm->MathOperation_for_all_vt (M_OP (g_object_get_data (G_OBJECT (simple), "plugin_MOp")));
        return;
}

void App::math_twoarg_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){ //  gboolean (*MOp)(MATH2OPPARAMS)){
	gapp->xsm->MathOperationX (M_2OP (g_object_get_data (G_OBJECT (simple), "plugin_MOp")), ID_CH_M_X, TRUE);
	return;
}

void App::math_twoarg_no_same_size_check_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){ // gboolean (*MOp)(MATH2OPPARAMS)){
	gapp->xsm->MathOperationX (M_2OP (g_object_get_data (G_OBJECT (simple), "plugin_MOp")), ID_CH_M_X, FALSE);
	return;
}

/* Tools ================================================== */

void App::tools_monitor_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gapp->monitorcontrol->show();
	return;
}

// void App::tools_remote_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
// 	if (gapp->PluginCallRemote)
// 		(*gapp->PluginCallRemote) (widget, data);
// 	else
// 		gapp->message(N_("Sorry, no 'pyremote' plugin loaded!"));
// 	return;
// }

void App::tools_chanselwin_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gapp->channelselector->show();
	return;
}

void App::tools_mkicons_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
#if 0
	if (gapp->PluginCallMkicons)
		(*gapp->PluginCallMkicons) (widget, user_data);
	else
		gapp->message(N_("Sorry, no 'Mkicons' plugin loaded!"));
	return;
#endif
}

void App::tools_plugin_reload_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gapp->reload_gxsm_plugins();
	return;
}

void App::tools_plugin_info_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if(gapp){
		if(gapp->GxsmPlugins)
			gapp->GxsmPlugins->view_pi_info();
		else
			gapp->message(N_("No Plugins loaded!"));
	}
	return;
}

/* Options ================================================== */

void App::options_preferences_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gxsm_search_for_palette();
	gxsm_search_for_HwI();
	GnomeResPreferences *pref = gnome_res_preferences_new (xsm_res_def, GXSM_RES_PREFERENCES_PATH);
	gnome_res_read_user_config (pref);
	gnome_res_run_change_user_config (pref, N_("Gxsm Preferences")); 
// on Dlg close pref is destroyed!
	return;
}

/* Help ================================================== */
void App::help_license_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gapp->message(N_(
			 "GXSM - Gnome X Scanning Microscopy\n"
			 "universal STM/AFM/SARLS/SPALEED/... controlling and\n"
			 "data analysis software\n"
			 "\n"
			 "Copyright (C) 1999-2010 Percy Zahl\n"
			 "\n"
			 "Authors: Percy Zahl <zahl@users.sf.net>\n"
			 "additional features: Andreas Klust <klust@users.sf.net>\n"
			 "WWW Home: http://gxsm.sourceforge.net\n"
			 "\n"
			 "This program is free software; you can redistribute it and/or modify\n"
			 "it under the terms of the GNU General Public License as published by\n"
			 "the Free Software Foundation; either version 2 of the License, or\n"
			 "(at your option) any later version.\n"
			 "\n"
			 "This program is distributed in the hope that it will be useful,\n"
			 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
			 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
			 "GNU General Public License for more details.\n"
			 "\n"
			 "You should have received a copy of the GNU General Public License\n"
			 "along with this program; if not, write to the Free Software\n"
			 "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.\n"
			 "\n"
			 "Online at http://www.gnu.org/copyleft/gpl.html\n"
			 ));
}

/* Help ================================================== */
void App::help_about_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gchar *message;
	const gchar *authors[] = {
		/* Here should be your names */
		"Percy Zahl  (zahl@users.sourceforge.net)",
		"Andreas Klust  (klust@users.sourceforge.net)",
		"Thorsten Wagener (stm@users.sourceforge.net)",
		"Stefan Schroeder  (stefan_fkp@users.sourceforge.net)",
		"Juan de la Figuera  (johnnybegood@users.sourceforge.net)",
		"and others http://gxsm.sourceforge.net",
		NULL
	};
	
	const gchar *documenters[] = {
		/* Here should be the documenters names */
		"Percy Zahl",
		"Andreas Klust",
		"Stefan Schroeder",
		NULL
	};
	
#ifdef XSM_DEBUG_OPTION
	gchar *dbg_lvl_tmp = g_strdup_printf ("\n\nGXSM / PlugIn debug level is: %d/%d", debug_level, pi_debug_level);
#else
	gchar *dbg_lvl_tmp = g_strdup_printf ("\n\nGXSM debug support is disabled.");
#endif

	message = g_strconcat
		(N_("GXSM is a Universal Scanning Probe Micoscopy Data Aquisitation and Visulaization System."
		   "\nApplications: STM, AFM, SARLS+NM, SPA-LEED."
		   "\nThis is the all new Gtk-3.0 Gxsm-3 revision."
		   "\n\nHardware: "),
		 gapp->xsm->hardware->Info(0),
		 dbg_lvl_tmp,
		 N_("\n\ncompiled by "),
		 COMPILEDBYNAME,
		 N_("\n\nGXSM-3 evolved from Gxsm, Xxsm, pmstm."),
		 NULL);

	g_free (dbg_lvl_tmp);

        // GdkPixbuf *logo = gdk_pixbuf_new_from_file ("../pixmaps/GxsmBWlogoTransparentColor.png", NULL);
	// GdkPixbuf *logo = gdk_pixbuf_new_from_file ("/usr/local/share/gxsm3/icons/gxsm3-icon.svg", NULL);
        GdkPixbuf *logo = gdk_pixbuf_new_from_file (PACKAGE_ICON_DIR "/gxsm3-icon.svg", NULL);
	gtk_show_about_dialog (NULL,
			       "program-name", "GXSM 3",
			       "version", VERSION" \""GXSM_VERSION_NAME"\"",
                               "logo", logo,
			       "title", N_("About GXSM"),
			       "authors", authors,
			       "documenters", documenters,
			       "translator-credits", "Juan de la Figuera",
			       "copyright", "(C) PyZahl et al 2000-2016",
			       "website", "http://gxsm.sf.net",
                               "comments", message,
                               "license-type", GTK_LICENSE_GPL_3_0,
			       NULL, NULL);
	g_free(message);
	return;
}

/*
void App::help_tip_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	tips_dialog_create ();
}

void App::help_home_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
        //GTK3QQQ	gnome_url_show ("http://gxsm.sourceforge.net", NULL);
}

void App::help_manual_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
        //GTK3QQQ	gnome_url_show ("http://gxsm.sourceforge.net/Gxsm2-main.pdf", NULL);
}
*/

/* Diverse CallBack Handler */

gint App::close_scan_event_cb(GtkWidget *window, GdkEventAny* e, gpointer data){
	if(gapp->xsm->SetMode ((long) GPOINTER_TO_INT (g_object_get_data (G_OBJECT (window), "Ch")), ID_CH_M_OFF))
		return FALSE;
	else
		return TRUE;
}
