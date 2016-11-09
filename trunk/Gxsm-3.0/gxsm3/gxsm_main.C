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
// grep -rl gtk_object_ . | xargs sed -i s/gtk_object_/g_object_/g

#include <new>
#include <cstring>

#include <locale.h>
#include <libintl.h>

#include <config.h>

#include <gtk/gtk.h>

#include "gxsm_app.h"
#include "gxsm_window.h"
#include "gxsm_resoucetable.h"
#include "action_id.h"


XSMRESOURCES xsmres;

App *gapp = NULL;
int restarted = 0;
int debug_level = 0;
int pi_debug_level = 0;
int force_gconf_defaults = 0;
int no_glx_init = 0;
int generate_preferences_gschema = 0;
int generate_gl_preferences_gschema = 0;

int generate_pcs_gschema = 0;
int generate_pcs_adj_gschema = 0;
gchar *current_pcs_gschema_path_group = NULL;

/* True if parsing determined that all the work is already done.  */
int just_exit = 0;

static const GOptionEntry gxsm_options[] =
{
	/* Version */
	{ "version", 'V', 0, G_OPTION_ARG_NONE, NULL, N_("Show the application's version"), NULL },
        
	{ "hardware-card", 'h', 0, G_OPTION_ARG_STRING, &xsmres.HardwareTypeCmd,
          N_("Hardware Card: no | ... (depends on available HwI plugins)"), NULL
        },

	{ "Hardware-DSPDev", 'd', 0, G_OPTION_ARG_STRING, &xsmres.DSPDevCmd,
	  N_("Hardware DSP Device Path: /dev/sranger0 | ... (depends on module type and index if multiple DSPs)"), NULL
        },

	{ "User-Unit", 'u', 0, G_OPTION_ARG_STRING, &xsmres.UnitCmd,
	  N_("XYZ Unit: AA | nm | um | mm | BZ | sec | V | 1 "), NULL
        },


	{ "disable-plugins", 'd', 0, G_OPTION_ARG_NONE, &xsmres.disableplugins,
          N_("Disable default plugin loading on startup"), NULL
        },
        
	{ "force-configure", 0, 0, G_OPTION_ARG_NONE, &xsmres.force_config,
          N_("Force to reconfigure Gxsm on startup"), NULL
        },

	{ "force-rebuild-configuration-defaults", 0, 0, G_OPTION_ARG_NONE, &force_gconf_defaults,
          N_("Forces to restore all GXSM gconf defaults on startup"), NULL
        },

	{ "write-gxsm-preferences-gschema", 0, 0, G_OPTION_ARG_NONE, &generate_preferences_gschema,
          N_("Generate Gxsm preferences gschema file on startup with build in defaults and exit"), NULL
        },

	{ "write-gxsm-gl-preferences-gschema", 0, 0, G_OPTION_ARG_NONE, &generate_gl_preferences_gschema,
          N_("Generate Gxsm GL preferences gschema file on startup with build in defaults and exit"), NULL
        },

	{ "write-gxsm-pcs-gschema", 0, 0, G_OPTION_ARG_NONE, &generate_pcs_gschema,
          N_("Generate Gxsm pcs gschema file on startup with build in defaults while execution"), NULL
        },

	{ "write-gxsm-pcs-adj-gschema", 0, 0, G_OPTION_ARG_NONE, &generate_pcs_adj_gschema,
          N_("Generate Gxsm pcs adjustements gschema file on startup with build in defaults while execution"), NULL
        },

	{ "debug-level", 'D', 0, G_OPTION_ARG_INT, &debug_level,
          N_("Set Gxsm debug level. 0: no debug output on console, 1: normal, 2: more verbose, ...5 increased verbosity"), "DN"
        },

	{ "pi-debug-level", 'P', 0, G_OPTION_ARG_INT, &pi_debug_level,
          N_("Set Gxsm Plug-In debug level. 0: no debug output on console, 1: normal, 2: more verbose, ...5 increased verbosity"), "PDN"
        },

	/* New instance */
	{
		"standalone", 's', 0, G_OPTION_ARG_NONE, NULL,
		N_("Run gxsm3 in standalone mode"),
		NULL
	},

	/* collects file arguments */
	{
		G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, NULL, NULL,
		N_("[FILE...] [+LINE[:COLUMN]]")
	},

	{NULL}
};

// global group control atg build time

const gchar* pcs_get_current_gschema_group (){
        return current_pcs_gschema_path_group ?
                strlen (current_pcs_gschema_path_group) > 0 ?
                current_pcs_gschema_path_group : "core-x" : "core-y";
}

void pcs_set_current_gschema_group (const gchar *group){
        if (current_pcs_gschema_path_group)
                g_free (current_pcs_gschema_path_group);
        current_pcs_gschema_path_group=g_strdup (group);
}


static void
add_accelerator (GApplication    *app,
                 const gchar *action_name_and_target,
                 const gchar *accel)
{
	const gchar *vaccels[] = {
		accel,
		NULL
	};
        //        PI_DEBUG_GP (DBG_L1, "add_accel for %s = %s\n", action_name_and_target, accel );
	gtk_application_set_accels_for_action (GTK_APPLICATION (app), action_name_and_target, vaccels);
}


/*
 * G_APPLICATION CODE FOR GXSM3 GOBJECT
 */

struct _Gxsm3app
{
  GtkApplication parent;
};

struct _Gxsm3appClass
{
  GtkApplicationClass parent_class;
};

G_DEFINE_TYPE(Gxsm3app, gxsm3_app, GTK_TYPE_APPLICATION);

static void
gxsm3_app_init (Gxsm3app *app)
{
        // ................**********************************************************************
        XSM_DEBUG(DBG_L2, "gxsm3_app_init =======================================================" );
}



static void
gxsm3_app_startup (GApplication *app)
{
        GtkBuilder *builder;
        // ................**********************************************************************
        XSM_DEBUG(DBG_L2, "gxsm3_app_startup ====================================================" );
        

        G_APPLICATION_CLASS (gxsm3_app_parent_class)->startup (app);

        XSM_DEBUG(DBG_L2, "gxsm3_app ** adding css styles" );

        /* add additional stylings */
        GtkCssProvider* provider = gtk_css_provider_new();
        GdkDisplay* display = gdk_display_get_default();
        GdkScreen* screen = gdk_display_get_default_screen(display);
        
        gtk_style_context_add_provider_for_screen(screen,
                                                  GTK_STYLE_PROVIDER(provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_USER);
        gtk_css_provider_load_from_resource (GTK_CSS_PROVIDER(provider), "/" GXSM_RES_BASE_PATH "/gxsm3-styles.css");

        // https://developer.gnome.org/gtk3/stable/chap-css-overview.html
        /*
        gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider),
                                        "#view-headerbar-tip-follow:checked{\n"
                                        "    color: #dd0000;\n"
                                        "}\n"
                                        "\n",
                                        -1, NULL);
        */
        g_object_unref(provider);
        // GXSM core application -- NOT the G APPLCIATION/WINDOW MANAGEMENT
        XSM_DEBUG(DBG_L2, "gxsm3 core startup" );
       
        XSM_DEBUG(DBG_L1, "START ** Setup dynamic preferences");
        gxsm_init_dynamic_res ();
        XSM_DEBUG(DBG_L1, "DONE ** Setup dynamic preferences");

        XSM_DEBUG(DBG_L1, "START ** GXSM run druids check");
        // Check for new user, present druids
        gxsm_check_new_user_config ();
        XSM_DEBUG(DBG_L1, "DONE ** GXSM run druids check");

        XSM_DEBUG(DBG_L1, "GXSM / PlugIn debug level is: " << debug_level << " / " << pi_debug_level);



        XSM_DEBUG(DBG_L2, "gapplication startup -- app menu installations" );
  
        //#define USE_COMPILED_IN_RESOURCES
#define USE_COMPILED_IN_RESOURCES
#ifdef USE_COMPILED_IN_RESOURCES
        PI_DEBUG_GP (DBG_L1, "USING COMPILED IN RESCOURCES -- gxsm3-menu.ui.\n");
        builder = gtk_builder_new_from_resource ("/" GXSM_RES_BASE_PATH "/gxsm3-menu.ui");
#else  // testing/development
        GError *error=NULL;
        PI_DEBUG_GP (DBG_L1, "USING BUILDER RESCOURCES from file -- gxsm3-menu.ui.\n");
        builder = gtk_builder_new ();
        gtk_builder_add_from_file (builder, "gxsm3-menu.ui", &error);
        if (error != NULL){
                // Report error to user, and free error
                PI_DEBUG_GP (DBG_L1, "gxsm3-menu.ui file not found: (%s) fallback to build in resource.\n", error->message);
                g_error_free (error);
                g_object_unref (builder);
                builder = gtk_builder_new_from_resource ("/" GXSM_RES_BASE_PATH "/gxsm3-menu.ui");
        }
#endif
        GObject *app_menu = gtk_builder_get_object (builder, "appmenu");
        if (!app_menu)
                PI_DEBUG_GP (DBG_L1, "ERROR: id appmenu can not be found in resource.\n");
        else
                gtk_application_set_app_menu (GTK_APPLICATION (app), G_MENU_MODEL (app_menu));

        GObject *gxsm_menubar = gtk_builder_get_object (builder, "gxsm-menubar");
        if (!gxsm_menubar)
                PI_DEBUG_GP (DBG_L1, "ERROR: id gxsm_menubar can not be found in resource.\n");

#ifdef ENABLE_GXSM_MENUBAR

        else
                gtk_application_set_menubar (GTK_APPLICATION (app), G_MENU_MODEL (gxsm_menubar));
#endif
        
        // g_object_unref (builder); // tryed to add a  g_object_ref (...menu) -- dose not help, keep.


        
     	//      add_accelerator (app, "app.new-window", "<Primary>N");
        //	add_accelerator (app, "app.quit", "<Primary>Q");
	add_accelerator (app, "win.view-activate", "F1");
	add_accelerator (app, "win.view-autodisp", "F2");
	add_accelerator (app, "win.object-mode::rectangle", "F5");
	add_accelerator (app, "win.object-mode::point", "F6");
	add_accelerator (app, "win.object-mode::line-show", "F7");
	add_accelerator (app, "win.remove-all-objects", "F12");

	add_accelerator (app, "win.side-pane", "F9");

	add_accelerator (app, "win.fix-zoom::zoomfactor-1x", "Q");
	add_accelerator (app, "win.fix-zoom::zoomfactor-2x", "<Shift>W");
	add_accelerator (app, "win.fix-zoom::zoomfactor-1by2", "W");

	add_accelerator (app, "win.gear-menu", "F10");

 
        XSM_DEBUG(DBG_L1, "GXSM: create core application ============ gapp = new App ()");

  	// Now create GxsmApplication (gapp):
	// this starts at App::App() and fires up the application's initialisiation

        gapp = new App (app);

        gapp->set_gxsm_main_menu (gxsm_menubar);
        gapp->set_gxsm_app_menu (app_menu);

        // create all later used POPUP menus and keep GObjects for later activations

        XSM_DEBUG(DBG_L1, "************ GXSM: loading menu rescourses ***************");
        
#define USE_COMPILED_IN_RESOURCES
#ifdef USE_COMPILED_IN_RESOURCES
        builder = gtk_builder_new_from_resource ("/" GXSM_RES_BASE_PATH "/gxsm3-popupmenus.ui");
#else  // testing/development
        error=NULL;
        builder = gtk_builder_new ();
        gtk_builder_add_from_file (builder, "gxsm3_popupmenus.ui", &error);
        if (error != NULL){
                // Report error to user, and free error
                PI_DEBUG_GP (DBG_L1, "gxsm3_appmenu.ui file not found: (%s) fallback to build in resource.\n", error->message);
                g_error_free (error);
                g_object_unref (builder);
                builder = gtk_builder_new_from_resource ("/" GXSM_RES_BASE_PATH "/gxsm3_popupmenus.ui");
        }
#endif

        XSM_DEBUG(DBG_L1, "************ GXSM: loading popup menus from rescources ***************");
   
        GObject *view2d_menu = gtk_builder_get_object (builder, "view2d-menu");
        if (!view2d_menu)
                PI_DEBUG_GP (DBG_L1, "ERROR: id view2d-menu not found in resource.\n");
        else
                gapp->set_view2d_menu (view2d_menu);

        GObject *vobj_ctx_menu_1p = gtk_builder_get_object (builder, "vobj-ctx-menu-1p");
        if (!vobj_ctx_menu_1p)
                PI_DEBUG_GP (DBG_L1, "ERROR: id vobj-ctx-menu-1p  not found in resource.\n");
        else
                gapp->set_vobj_ctx_menu_1p (vobj_ctx_menu_1p);

        GObject *vobj_ctx_menu_2p = gtk_builder_get_object (builder, "vobj-ctx-menu-2p");
        if (!vobj_ctx_menu_2p)
                PI_DEBUG_GP (DBG_L1, "ERROR: id vobj-ctx-menu-2p not found in resource.\n");
        else 
                gapp->set_vobj_ctx_menu_2p (vobj_ctx_menu_2p);

        GObject *vobj_ctx_menu_np = gtk_builder_get_object (builder, "vobj-ctx-menu-np");
        if (!vobj_ctx_menu_np)
                PI_DEBUG_GP (DBG_L1, "ERROR: id vobj-ctx-menu-np not found in resource.\n");
        else 
                gapp->set_vobj_ctx_menu_np (vobj_ctx_menu_np);

        GObject *vobj_ctx_menu_event = gtk_builder_get_object (builder, "vobj-ctx-menu-event");
        if (!vobj_ctx_menu_event)
                PI_DEBUG_GP (DBG_L1, "ERROR: id vobj-ctx-menu-event not found in resource.\n");
        else
                gapp->set_vobj_ctx_menu_event (vobj_ctx_menu_event);

        GObject *view3d_menu = gtk_builder_get_object (builder, "view3d-menu");
        if (!view3d_menu)
                PI_DEBUG_GP (DBG_L1, "ERROR: id view3d-menu not found in resource.\n");
        else
                gapp->set_view3d_menu (view3d_menu);

        GObject *profile_popup_menu = gtk_builder_get_object (builder, "profile-popup-menu");
        if (!profile_popup_menu)
                PI_DEBUG_GP (DBG_L1, "ERROR: id profile-popup-menu not found in resource.\n");
        else
                gapp->set_profile_popup_menu (profile_popup_menu);

        XSM_DEBUG(DBG_L1, "************ GXSM: loading popup menus for hwi from rescources ***************");

        builder = gtk_builder_new_from_resource ("/" GXSM_RES_BASE_PATH "/gxsm3-hwi-menus.ui");

        GObject *hwi_mover_popup_menu = gtk_builder_get_object (builder, "hwi-mover-popup-menu");
        if (!hwi_mover_popup_menu)
                PI_DEBUG_GP (DBG_L1, "ERROR: id hwi-mover-popup-menu not found in resource.\n");
        else
                gapp->set_hwi_mover_popup_menu (hwi_mover_popup_menu);

        GObject *hwi_control_popup_menu = gtk_builder_get_object (builder, "hwi-control-popup-menu");
        if (!hwi_control_popup_menu)
                PI_DEBUG_GP (DBG_L1, "ERROR: id hwi-control-popup_menu not found in resource.\n");
        else
                gapp->set_hwi_control_popup_menu (hwi_control_popup_menu);

        XSM_DEBUG(DBG_L1, "************ GXSM: loading popup menus for plugins from rescources ***************");

        builder = gtk_builder_new_from_resource ("/" GXSM_RES_BASE_PATH "/gxsm3-plugin-pyremote-menu.ui");

        GObject *plugin_pyremote_file_menu = gtk_builder_get_object (builder, "plugin-pyremote-file-menu");
        if (!plugin_pyremote_file_menu)
                PI_DEBUG_GP (DBG_L1, "ERROR: id plugin-pyremote-file-menu not found in resource.\n");
        else
                gapp->set_plugin_pyremote_file_menu (plugin_pyremote_file_menu);

        
        // g_object_unref (builder); // tryed to add a  g_object_ref (...menu) -- dose not help, keep.
}

static void
gxsm3_app_activate (GApplication *app)
{
        Gxsm3appWindow *win;
        // ................**********************************************************************
        XSM_DEBUG(DBG_L2, "gxsm3_app_activate ==================================================" );

        if (gapp){
                if (gapp->gxsm_app_window_present ()){
                        XSM_DEBUG(DBG_L2, "gxsm3_app_activate -- presenting existing Gxsm Main Window ===================" );
                        win = gapp->get_app_window ();
                } else
                        win = gxsm3_app_window_new (GXSM3_APP (app));
        } else {
                win = gxsm3_app_window_new (GXSM3_APP (app));
        }
        gtk_window_present (GTK_WINDOW (win));
}

static void
gxsm3_app_open (GApplication  *app,
                  GFile        **files,
                  gint           n_files,
                  const gchar   *hint)
{
  GList *windows;
  Gxsm3appWindow *win;
  int i;

        // ................**********************************************************************
        XSM_DEBUG(DBG_L2, "gxsm3_app_open =======================================================" );

        windows = gtk_application_get_windows (GTK_APPLICATION (app));
        if (windows)
                win = GXSM3_APP_WINDOW (windows->data);
        else
                win = gxsm3_app_window_new (GXSM3_APP (app));

        for (i = 0; i < n_files; i++)
                gxsm3_app_window_open (win, files[i]);

        gtk_window_present (GTK_WINDOW (win));
}

static void
gxsm3_app_class_init (Gxsm3appClass *klass)
{
        // ................**********************************************************************
        XSM_DEBUG(DBG_L2, "gxsm3_app_class_init =================================================" );

        G_APPLICATION_CLASS (klass)->startup = gxsm3_app_startup;
        G_APPLICATION_CLASS (klass)->activate = gxsm3_app_activate;
        G_APPLICATION_CLASS (klass)->open = gxsm3_app_open;
}

Gxsm3app *
gxsm3_app_new (void)
{
        // ................**********************************************************************
        XSM_DEBUG(DBG_L2, "gxsm3_app_new ========================================================" );

        return (Gxsm3app*) g_object_new (GXSM3_APP_TYPE,
                                         "application-id", GXSM_RES_BASE_PATH_DOT,
                                         "flags", G_APPLICATION_HANDLES_OPEN,
                                         NULL);
}



static void gxsm3_app_shutdown (GApplication *app, gpointer user_data)
{
        // ................**********************************************************************
        XSM_DEBUG(DBG_L2, "gxsm3_app_shutdown ===================================================" );

        XSM_DEBUG(DBG_L1, "GXSM::SHUTDOWN");
	delete gapp;
	gapp = NULL;
}

#if 0

static gint
gxsm_app_command_line (GApplication            *application,
                       GApplicationCommandLine *cl)
{
	GVariantDict *options;
	const gchar **remaining_args;

        // ................**********************************************************************
        XSM_DEBUG(DBG_L2, "gxsm3_capp_comand_line ===============================================" );

	options = g_application_command_line_get_options_dict (cl);

        //	g_variant_dict_lookup (options, "new-window", "b", &priv->new_window);
        //	g_variant_dict_lookup (options, "new-document", "b", &priv->new_document);
        //	g_variant_dict_lookup (options, "geometry", "s", &priv->geometry);

	if (g_variant_dict_contains (options, "wait"))
                {
                        //		priv->command_line = cl;
                }


	/* Parse filenames */
	if (g_variant_dict_lookup (options, G_OPTION_REMAINING, "^a&ay", &remaining_args))
                {
                        gint i;

                        for (i = 0; remaining_args[i]; i++)
                                {
                                        GFile *file;

                                        file = g_application_command_line_create_file_for_arg (cl, remaining_args[i]);
                        //                        priv->file_list = g_slist_prepend (priv->file_list, file);
		}

                //		priv->file_list = g_slist_reverse (priv->file_list);
		g_free (remaining_args);
	}

	g_application_activate (application);

	return 0;
}

static gint
gxsm_app_handle_local_options (GApplication *application,
                               GVariantDict *options)
{
	if (g_variant_dict_contains (options, "version"))
	{
		PI_DEBUG_GP (DBG_L1, "%s - Version %s\n", g_get_application_name (), VERSION);
		return 0;
	}
#if 0
	if (g_variant_dict_contains (options, "standalone"))
	{
		GApplicationFlags old_flags;

		old_flags = g_application_get_flags (application);
		g_application_set_flags (application, old_flags | G_APPLICATION_NON_UNIQUE);
	}

	if (g_variant_dict_contains (options, "wait"))
	{
		GApplicationFlags old_flags;

		old_flags = g_application_get_flags (application);
		g_application_set_flags (application, old_flags | G_APPLICATION_IS_LAUNCHER);
	}
#endif
	return -1;
}

/* Note: when launched from command line we do not reach this method
 * since we manually handle the command line parameters in order to
 * parse +LINE:COL, stdin, etc.
 * However this method is called when open() is called via dbus, for
 * instance when double clicking on a file in nautilus
 */
static void
gxsm_app_open (GApplication  *application,
                GFile        **files,
                gint           n_files,
                const gchar   *hint)
{
	gint i;
        //	GSList *file_list = NULL;
        int re_use=0;

	for (i = 0; i < n_files; i++){
                //		file_list = g_slist_prepend (file_list, files[i]);
                gchar *fpath = g_file_get_path (files[i]);
                std::ifstream test;
                test.open (fpath, std::ios::in);
                if (test.good ()) {
                        test.close ();
                        XSM_DEBUG(DBG_L2, "Attempt to load/import <" << fpath << ">");
                        if (re_use)
                                re_use = gapp->xsm->load (fpath);
                        else
                                if(!gapp->xsm->ActivateFreeChannel())
                                        // if no success, reuse this active scan next!
                                        re_use = gapp->xsm->load (fpath);
                }
                g_free (fpath);
        }
        if (re_use)
                gapp->xsm->SetMode(-1, ID_CH_M_OFF, TRUE); // kill unused scan

        //	file_list = g_slist_reverse (file_list);
        //	g_slist_free (file_list);
}

#endif

int main (int argc, char **argv)
{
        GError *error = NULL;
        GOptionContext *context;

        // init GL engine ????
        //        if (gtk_gl_init_check (&argc, argv)
        //                glutInit (&argc, argv);

        pcs_set_current_gschema_group ("core-init");

        // uninstalled for testing only! Schema in workign dir
        // g_setenv ("GSETTINGS_SCHEMA_DIR", ".", FALSE);
        // --------------------------------------------------

        context = g_option_context_new ("List of loadable file(s) .nc, ...");
        g_option_context_add_main_entries (context, gxsm_options, GETTEXT_PACKAGE);
        g_option_context_add_group (context, gtk_get_option_group (TRUE));

        if (!g_option_context_parse (context, &argc, &argv, &error))
                {
                        PI_DEBUG_GP (DBG_EVER, "option parsing failed: %s\n", error->message);
                        exit (1);
                }
        else
                {
                        PI_DEBUG_GP (DBG_L1, "GXSM3 comandline option parsing results:\n");
                        PI_DEBUG_GP (DBG_L1, "=> xsmres.HardwareTypeCmd = %s\n", xsmres.HardwareTypeCmd);
                        PI_DEBUG_GP (DBG_L1, "=> xsmres.DSPDevCmd  .... = %s\n", xsmres.DSPDevCmd);
                        PI_DEBUG_GP (DBG_L1, "=> xsmres.UnitCmd ....... = %s\n", xsmres.UnitCmd);
                        PI_DEBUG_GP (DBG_L1, "=> xsmres.force_config .. = %d\n", xsmres.force_config);
                        PI_DEBUG_GP (DBG_L1, "=> force_gconf_defaults . = %d   (GSettings now)\n", force_gconf_defaults);
                        PI_DEBUG_GP (DBG_L1, "=> debug_level .......... = %d\n", debug_level);
                        PI_DEBUG_GP (DBG_L1, "=> pi_debug_level ....... = %d\n", pi_debug_level);
                }

        // ................**********************************************************************
        XSM_DEBUG(DBG_L2, "gxsm3_main g_application_run =========================================" );

        return g_application_run (G_APPLICATION (gxsm3_app_new ()), argc, argv);
}
