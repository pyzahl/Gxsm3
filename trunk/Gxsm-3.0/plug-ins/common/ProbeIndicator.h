/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Copyright (C) 2018 P.Zahl
 *
 * Authors: Percy Zahl
 *                                                                                
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
 *  * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include <math.h>
#include "cairo_item.h"




class hud_object {
public:
        hud_object(){
                // skeleton
                irad = 100.; orad = irad*1.2; delta = irad*0.05;
                rsz = irad/5.; asp=1.67; 
                ic = new cairo_item_circle (0.,0., irad);
                oc = new cairo_item_circle (0.,0., orad);

                // empty elements
                marks = NULL;
                indicators = NULL;
                tics = NULL;
                horizon = NULL;

        };
        ~hud_object(){
                delete ic;
                delete oc;
                g_slist_free_full (indicators, (GDestroyNotify)hud_object::remove_cairo_item);
                g_slist_free_full (marks, (GDestroyNotify)hud_object::remove_cairo_item);
        };
        static void remove_cairo_item(cairo_item *x) { delete x; };
        static void draw_cairo_item(cairo_item *x, cairo_t* cr) { x->draw (cr); };
        static void hide_cairo_item(cairo_item *x, void* data) { x->hide (); };
        static void show_cairo_item(cairo_item *x, void* data) { x->show (); };
        
        cairo_item_path_closed* add_mark (const gchar *id, double pos, double z){
                double y=irad*z;
                cairo_item_path_closed *m = new cairo_item_path_closed (5);
                m->set_id(id);
                m->set_angle(pos/400.*2.*M_PI);
                m->set_stroke_rgba (CAIRO_COLOR_MAGENTA);
                m->set_fill_rgba (1., 0., 0., 1.);
                m->set_position (0., 0.);
                m->set_xy (0, 0., y);
                m->set_xy (1, +rsz/asp, y+rsz);
                m->set_xy (2, +rsz/asp, irad);
                m->set_xy (3, -rsz/asp, irad);
                m->set_xy (4, -rsz/asp, y+rsz);
                m->set_stroke_rgba (CAIRO_COLOR_MAGENTA);
                m->set_fill_rgba (1., 0., 0., 1.);
                m->set_line_width (1.0);
                marks = g_slist_prepend (marks, m);
                return m;
        };
        void set_mark_len(cairo_item_path_closed* m, double z){
                double y=irad*z;
                m->set_xy (0, 0., y);
                m->set_xy (1, +rsz/asp, y+rsz);
                m->set_xy (4, -rsz/asp, y+rsz);
        };
        cairo_item_arc* add_indicator (const gchar *id, double pos, double val){
                cairo_item_arc *arc = new cairo_item_arc (0.,0., irad, orad-irad-delta, pos, val, 2.*M_PI/400.);
                arc->set_id(id);
                if (val < 0.) arc->set_stroke_rgba (CAIRO_COLOR_BLUE);
                else arc->set_stroke_rgba (CAIRO_COLOR_GREEN);
                indicators = g_slist_prepend (indicators, arc);
                return arc;
        };

        void set_indicator_val (cairo_item_arc *arc, double pos, double val){
                arc->set_arc (pos+(val>0.?1.:-1.), val);
        };
        
        cairo_item_arc* add_tics (const gchar *id, double pos, double val, int n, double dtic){
                cairo_item_arc *arc = new cairo_item_arc (0.,0., orad, (orad-irad)*0.3, pos+(dtic>0.?-0.5:0.5), val, 2.*M_PI/400., n, dtic);
                arc->set_id(id);
                arc->set_stroke_rgba (CAIRO_COLOR_BLACK);
                tics = g_slist_prepend (tics, arc);
                return arc;
        };

        cairo_item_path* add_horizon (const gchar *id, double pos, double z, gint n){
                double y=irad*z;
                cairo_item_path *h = new cairo_item_path (n);
                h->set_id(id);
                h->set_angle(pos/400.*2.*M_PI);
                h->set_stroke_rgba (CAIRO_COLOR_RED);
                h->set_position (0., y);
                h->set_line_width (1.0);
                horizon = g_slist_prepend (horizon, h);
                return h;
        };


        void queue_update (GtkWidget* imgarea) {
                ic->queue_update (imgarea);
                oc->queue_update (imgarea);
        };
        void hide (){
                ic->hide();
                oc->hide();
                g_slist_foreach (marks, (GFunc)hud_object::hide_cairo_item, this);
                g_slist_foreach (indicators, (GFunc)hud_object::hide_cairo_item, this);
                g_slist_foreach (tics, (GFunc)hud_object::hide_cairo_item, this);
                g_slist_foreach (horizon, (GFunc)hud_object::hide_cairo_item, this);
        };
        void show (){
                ic->show();
                oc->show();
                g_slist_foreach (marks, (GFunc)hud_object::show_cairo_item, this);
                g_slist_foreach (indicators, (GFunc)hud_object::show_cairo_item, this);
                g_slist_foreach (tics, (GFunc)hud_object::show_cairo_item, this);
                g_slist_foreach (horizon, (GFunc)hud_object::show_cairo_item, this);
        };
        virtual void draw (cairo_t* cr, double alpha=0.0, gboolean tr=true){
                ic->draw(cr);
                oc->draw(cr);
                g_slist_foreach (marks, (GFunc)hud_object::draw_cairo_item, cr);
                g_slist_foreach (indicators, (GFunc)hud_object::draw_cairo_item, cr);
                g_slist_foreach (tics, (GFunc)hud_object::draw_cairo_item, cr);
                g_slist_foreach (horizon, (GFunc)hud_object::draw_cairo_item, cr);
        }
private:
        double rsz, asp;
        double irad, orad, delta;
       	cairo_item *ic, *oc;
        GSList *indicators;
        GSList *marks;
        GSList *tics;
        GSList *horizon;
};


class ProbeIndicator : public AppBase{
public:
        ProbeIndicator ();
        virtual ~ProbeIndicator();

        void AppWindowInit(const gchar *title);

        static gboolean canvas_draw_callback (GtkWidget *widget, cairo_t *cr, ProbeIndicator *pv);
        static gint canvas_event_cb(GtkWidget *canvas, GdkEvent *event, ProbeIndicator *pv);
        
        void show() {
                // gtk_widget_show_all (window);
        };
        void run ();
        gint refresh (); // return TRUE if OK, FALSE if busy
        void start ();
        void stop ();

private:  
        gint       hud_size;

        guint      timer_id;

        GtkWidget  *canvas;

        // remplaced with: cairo_item_rectangle / text / path
        cairo_item_text  *info;
        hud_object *probe;
        cairo_item_arc *ipos, *ineg;
        cairo_item_path_closed *tip;
        cairo_item_path *horizon;
};
