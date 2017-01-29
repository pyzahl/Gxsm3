/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Percy Zahl <zahl@fkp.uni-hannover.de>
 * additional features: Andreas Klust <klust@fkp.uni-hannover.de>
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

/*
 * Project: Gxsm
 */

#include "gxsm_app.h"

#include "unit.h"
#include "pcs.h"
#include "xsmtypes.h"

typedef struct{
  const gchar *label;
  gint  width;
  gchar *data;
} MONITORLOGENTRY;

#define MAXCOLUMNS 10
MONITORLOGENTRY tabledef[] = {
  { "time stamp", 150, NULL },
  { "action",  100, NULL },
  { "comment", 200, NULL },
  { "value1", 100, NULL },
  { "value2", 100, NULL },
  { "value3", 100, NULL },
  { NULL, 0, NULL }
};

MonitorControl::MonitorControl (gint loglevel, gint maxlines):Monitor(loglevel)
{
        set_max_lines (maxlines);

        AppWindowInit(N_("GXSM Activity Monitor and Logbook"));

        log_view = gtk_text_view_new ();
        gtk_text_view_set_editable (GTK_TEXT_VIEW (log_view), FALSE);
        log_buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (log_view));

        GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                             GTK_SHADOW_IN);
        gtk_widget_set_hexpand (scrolled_window, TRUE);
        gtk_widget_set_vexpand (scrolled_window, TRUE);
        gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (log_view)) ;
        gtk_grid_attach (GTK_GRID (v_grid), scrolled_window, 1,1, 10,1);
        gtk_widget_show_all (v_grid);

        set_window_geometry ("monitor");
}

MonitorControl::~MonitorControl (){
        LogEvent ("MonitorControl", "End of Session Log. Exit OK.");
}

void MonitorControl::LogEvent (const gchar *Action, const gchar *Entry, gint level){
        if (logging_level >= level){
                // add to log file
                PutLogEvent (Action, Entry);

                // no logging into text buffer if max_lines < 0
                if (max_lines < 0)
                        return;
                
                GtkTextIter start_iter, end_trim_iter, end_iter;
                GtkTextMark *end_mark;
                gint lines = gtk_text_buffer_get_line_count (log_buf);
         
#if 0
                gtk_text_buffer_get_bounds (log_buf, &start_iter, &end_iter);
                // limit buffer max_lines lines unless max_lines == 0
                if (max_lines > 1)
                        if (lines > max_lines)
                                gtk_text_buffer_get_iter_at_line_index (log_buf, &start_iter, lines-max_lines, 0);

                GString *output = g_string_new (gtk_text_buffer_get_text (log_buf,
                                                                          &start_iter, &end_iter,
                                                                          FALSE));
                // append to log
                GTimeVal gt;
                g_get_current_time (&gt);
                gchar *tmp = g_time_val_to_iso8601 (&gt);
                output = g_string_append(output, tmp);
                g_free (tmp);
                output = g_string_append(output, ": \t");
                output = g_string_append(output, Action);
                output = g_string_append(output, ": \t");
                output = g_string_append(output, Entry);
                output = g_string_append(output, "\n");

                gtk_text_buffer_set_text (log_buf, output->str, -1);
                g_string_free(output, TRUE);

                // scroll to end
                gtk_text_buffer_get_end_iter (log_buf, &end_iter);
                end_mark = gtk_text_buffer_create_mark (log_buf, "cursor", &end_iter,
                                                        FALSE);
                g_object_ref (end_mark);
                gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (log_view),
                                              end_mark, 0.0, FALSE, 0.0, 0.0);
                g_object_unref (end_mark);

#else
                // append to log buffer view
                GTimeVal gt;
                g_get_current_time (&gt);
                gchar *tmp = g_time_val_to_iso8601 (&gt);
                GString *output = g_string_new (tmp);
                g_free (tmp);
                output = g_string_append(output, ": \t");
                output = g_string_append(output, Action);
                output = g_string_append(output, ": \t");
                output = g_string_append(output, Entry);
                output = g_string_append(output, "\n");

                gtk_text_buffer_get_bounds (log_buf, &start_iter, &end_iter);

                // limit buffer max_lines lines unless max_lines == 0
                if (max_lines > 1 && lines > max_lines){
                        gtk_text_buffer_get_iter_at_line_index (log_buf, &end_trim_iter, lines-max_lines, 0);
                        gtk_text_buffer_delete (log_buf,  &start_iter,  &end_trim_iter);
                }

                end_mark = gtk_text_buffer_create_mark (log_buf, "cursor", &end_iter, false);
                g_object_ref (end_mark);

                gtk_text_buffer_move_mark (log_buf, end_mark, &end_iter );
                gtk_text_buffer_insert_at_cursor(log_buf, output->str, -1 );

                g_string_free(output, TRUE);

                gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (log_view), end_mark, 0.0, true, 0.5, 1);
                g_object_unref (end_mark);
#endif
        }
}
