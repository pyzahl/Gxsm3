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

#include <gtk/gtk.h>
#include <iostream>
#include <fstream>

#include "gxsm_app.h"
#include "gxsm_window.h"

struct _Gxsm3appWindow
{
        GtkApplicationWindow parent;
};

struct _Gxsm3appWindowClass
{
        GtkApplicationWindowClass parent_class;
};

typedef struct _Gxsm3appWindowPrivate Gxsm3appWindowPrivate;

struct _Gxsm3appWindowPrivate
{
        GSettings *settings;
        GtkWidget *stack;
};

G_DEFINE_TYPE_WITH_PRIVATE(Gxsm3appWindow, gxsm3_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void
gxsm3_app_window_init (Gxsm3appWindow *win)
{
        Gxsm3appWindowPrivate *priv;

        // ................**********************************************************************
        XSM_DEBUG(DBG_L2, "gxsm3_app_window_init ================================================" );

        priv = (Gxsm3appWindowPrivate *) gxsm3_app_window_get_instance_private (win);
        // no template in use currently
        // gtk_widget_init_template (GTK_WIDGET (win));

        if (gapp->gxsm_app_window_present ()){
                // VIEW WINDOW
                XSM_DEBUG(DBG_L2, "gxsm3_app_window_init ** GENERIC VIEW WINDOW =================================" );
        } else {
                // THIS IS FOR THE GXSM MAIN CONTROL WINDOW
                XSM_DEBUG(DBG_L2, "gxsm3_app_window_init ** MAIN WINDOW =================================" );
                
                XSM_DEBUG(DBG_L1, "START ** GXSM GUI building");
                gapp->build_gxsm (win);
                XSM_DEBUG(DBG_L1, "DONE ** GXSM GUI building");
        }
}

static void
gxsm3_app_window_dispose (GObject *object)
{
        Gxsm3appWindow *win;
        Gxsm3appWindowPrivate *priv;

        win = GXSM3_APP_WINDOW (object);
        priv = (Gxsm3appWindowPrivate *) gxsm3_app_window_get_instance_private (win);

        // g_clear_object (&priv->settings);

        G_OBJECT_CLASS (gxsm3_app_window_parent_class)->dispose (object);
}

static void
gxsm3_app_window_class_init (Gxsm3appWindowClass *klass)
{
        G_OBJECT_CLASS (klass)->dispose = gxsm3_app_window_dispose;

        //gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
        //                                             "/"GXSM_RES_BASE_PATH"/window.ui");

        //  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), Gxsm3appWindow, gears);

}

Gxsm3appWindow *
gxsm3_app_window_new (Gxsm3app *app)
{
        return (Gxsm3appWindow *) g_object_new (GXSM3_APP_WINDOW_TYPE, "application", app, NULL);
}

void
gxsm3_app_window_open (Gxsm3appWindow *win,
		       GFile            *file)
{
        Gxsm3appWindowPrivate *priv;
        gchar *basename;
        gboolean re_use = false;

        priv = (Gxsm3appWindowPrivate *) gxsm3_app_window_get_instance_private (win);
        basename = g_file_get_basename (file);

        std::ifstream test;
        test.open (basename, std::ios::in);

        if (test.good ()) {

                test.close ();
                XSM_DEBUG(DBG_L2, "Attempt to load/import <" << basename << ">");

                if (re_use)
                        re_use = gapp->xsm->load (basename);
                else
                        if(!gapp->xsm->ActivateFreeChannel())
                                // if no success, reuse this active scan next!
                                re_use = gapp->xsm->load (basename);
        }

        g_free (basename);
}
