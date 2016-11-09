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

#include <locale.h>
#include <libintl.h>
#include <stdlib.h>
#include <ctype.h>

#include "gxsm_app.h"

#include "glbvars.h"

#include "surface.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include <gtk/gtk.h>

enum {
        TARGET_URI_LIST,
        TARGET_URL
};

/* The following functions are a quick hack to get URL drops
 * from Netscape working. I make no pretense of correctness
 * or great security. Plus it leaves files lying in temp.
 * but it's fun.
 */

typedef struct {
        gchar *name;
        gint pid;
} ChildData;

static char *download_dir = NULL;

/* The following functions are a quick hack to get URL drops
 * from Netscape working. I make no pretense of correctness
 * or great security. Plus it leaves files lying in temp.
 * but it's fun.
 */

/* Called when the child quits */
gboolean App::input_func(GIOChannel *source, GIOCondition condition, gpointer data)
{
        char buf[2];
        ChildData *child_data = (ChildData*)data;
        int status;

        if (read (g_io_channel_unix_get_fd(source), buf, 2) != 2 ||
            (buf[0] != 'O') || (buf[1] != 'K')) /* failure */
		{
                        g_warning ("Download failed!\n");
		}
        else
		{
                        gapp->xsm->load (child_data->name);
		}

        waitpid (child_data->pid, &status, 0);

        g_free (child_data->name);
        g_free (child_data);
  
        close(g_io_channel_unix_get_fd(source));

        return FALSE;
}

void App::grab_url(const gchar *name)
{
        int pid;
        int fds[2];

        if (!download_dir)
		{
                        int fd;
#define GXSMTMPPREFIX "/tmp/GxsmTmp"
                        char *buffer = g_strdup(GXSMTMPPREFIX"_XXXXXX");
  
                        if (!(fd=mkstemp(buffer)) )
				{
                                        g_warning ("Could not generate temporary directory: %s\n",
                                                   g_strerror(errno));
                                        return;
				}
                        close(fd);
                        FILE *cmd = popen("rm -rf " GXSMTMPPREFIX "*", "w");
                        if(cmd)
                                pclose(cmd);

                        if ((mkdir(buffer, 0755) != 0))
				{
                                        g_warning ("Could not generate temporary directory: %s\n",
                                                   g_strerror(errno));
                                        return;
				}
      
                        download_dir = buffer;
		}

        if (pipe(fds))
		{
                        g_warning ("Could not create pipe: %s\n", g_strerror(errno));
                        return;
		}
  
        if (!(pid = fork()))
		{
                        /* Child */

                        close(fds[0]);
      
                        /* Fork off a wget */

                        if (!(pid = fork()))
				{
                                        execlp("wget", "wget", "-q", "-P", download_dir, name, NULL);
                                        g_warning("Could not run wget: %s\n", g_strerror(errno));
                                        _exit(0);
				}
                        else if (pid > 0)
				{
                                        int status;
                                        waitpid (pid, &status, 0);

                                        if (!status)
                                                if (write (fds[1], "OK", 2)<0)
                                                        ;

                                        _exit(0);
				}
                        else
				{
                                        g_warning ("Could not fork!\n");
                                        _exit(0);
				}
		}
        else if (pid > 0)
		{
                        /* Parent */

                        char const *tail;
                        ChildData *child_data;
                        GIOChannel *ioc;

                        close(fds[1]);

                        tail = strrchr(name, '/');
                        child_data = g_new0 (ChildData, 1);

                        if (tail)
                                child_data->name = g_strconcat (download_dir, "/", ++tail, NULL);
                        else
                                child_data->name = g_strconcat (download_dir, "/", name, NULL);
      
                        child_data->pid = pid;

                        ioc = g_io_channel_unix_new (fds[0]);
                        g_io_add_watch(ioc, (GIOCondition)(G_IO_IN|G_IO_HUP|G_IO_NVAL), input_func, child_data);
                        g_io_channel_unref(ioc);
		}
        else
                g_warning ("Could not fork\n");
}

void App::process_one_filename (GtkWidget * widget, const gchar *filename)
{
	int ch = (int) GPOINTER_TO_INT (g_object_get_data  (G_OBJECT (widget), "ChNo"));
	if(ch){
		if(ch>0 && ch <= MAX_CHANNELS)
			gapp->xsm->ActivateChannel( ch-1 );
	}else
		if(gapp->xsm->ActivateFreeChannel())
			return;
	
	if (strncmp (filename, "http://", 7) == 0)
		grab_url (filename);
	else if (strncmp (filename, "file://", 7) == 0)
		gapp->xsm->load (&filename[7]);
        else gapp->xsm->load (filename); // try...
}

void urldecode2(gchar *dst, const gchar *src)
{
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}

void App::filenames_dropped(GtkWidget * widget,
                            GdkDragContext   *context,
                            gint              x,
                            gint              y,
                            GtkSelectionData *selection_data,
                            guint             info,
                            guint             time)
{
        if (!gtk_selection_data_get_data (selection_data))
                return;
	
        switch (info){
        case TARGET_URI_LIST:
		{
                        char **names = gtk_selection_data_get_uris (selection_data);
                        if (!names)
                                return;
                        std::cout << "DND-DATA=[" << (char *)gtk_selection_data_get_data(selection_data) << "]" << std::endl;

                        for (gchar **name = names; *name; name++){
                                gchar *filename = g_strdup(*name);
                                urldecode2 (filename, *name); // do it myself.
                                std::cout << "DND-FILE=[" << *name << "]" << std::endl;
                                // gchar *filename = g_uri_unescape_string (filename, NULL); // ??? 
                                // gchar *filename = g_filename_from_uri (filename, NULL, NULL); // ??????
                                if (filename)
                                        process_one_filename (widget, filename);
                                g_free (filename);
                        }
                        g_strfreev (names);

                        break;
		}
        case TARGET_URL:
                gchar *url = g_strdup ((gchar *)gtk_selection_data_get_data (selection_data));
                gchar *p;
                if ((p=strchr (url, '\n')) != NULL) // cut "href info" off
                        *p = 0;
                process_one_filename (widget, url);
                g_free (url);
                break;
        }
}

void
App::configure_drop_on_widget(GtkWidget * widget)
{
        static GtkTargetEntry drag_types[] =
                {
                        { g_strdup("text/uri-list"), 0, TARGET_URI_LIST },
                        { g_strdup("_NETSCAPE_URL"), 0, TARGET_URL }
                };
        static gint n_drag_types = sizeof(drag_types)/sizeof(drag_types[0]);

        gtk_drag_dest_set 
                ( widget,
                  (GtkDestDefaults)(
                                    GTK_DEST_DEFAULT_MOTION | 
                                    GTK_DEST_DEFAULT_HIGHLIGHT | 
                                    GTK_DEST_DEFAULT_DROP
                                    ),
                  drag_types, 
                  n_drag_types,
                  GDK_ACTION_COPY
                  );

        g_signal_connect(G_OBJECT(widget), "drag_data_received",
                         G_CALLBACK(filenames_dropped), NULL);
}

void App::drag_data_get (GtkWidget        *widget,
                         GdkDragContext   *context,
                         GtkSelectionData *selection_data,
                         guint             info,
                         guint             time)
{
        gchar *file=NULL;
        gchar *uri_list;

        //  file = ee_image_get_filename(image_display);

        if (file) /* ignore non-file-images for now */
		{
                        uri_list = g_strconcat ("file:", file, NULL);
                        gtk_selection_data_set (selection_data,
                                                gtk_selection_data_get_target (selection_data), 8,
                                                (const guchar*)uri_list, 
                                                (int)strlen(uri_list));
                        g_free (uri_list);
		}
        else
		{
                        gtk_selection_data_set (selection_data,
                                                gtk_selection_data_get_target (selection_data), 8,
                                                NULL, 0);
		}
}

void
App::configure_drag_on_widget(GtkWidget * widget)
{
        static GtkTargetEntry drag_types[] =
                {
                        { g_strdup("text/uri-list"), 0, TARGET_URI_LIST }
                };
        static gint n_drag_types = sizeof(drag_types)/sizeof(drag_types[0]);

        gtk_drag_source_set (widget, 
                             GDK_BUTTON1_MASK,
                             drag_types, n_drag_types,
                             GDK_ACTION_COPY);

        g_signal_connect (G_OBJECT(widget), "drag_data_get",
                          G_CALLBACK(drag_data_get), NULL);
}

