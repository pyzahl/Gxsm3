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

#ifndef CAIRO_ITEM_H
#define CAIRO_ITEM_H

#include <math.h>
#include "gtk/gtk.h"

typedef struct { double x,y; } cairo_point;
#define BASIC_COLORS 7

extern float BasicColors[][4];
int cairo_basic_color_lookup (const gchar *color);

#define UNREF_DELETE_CAIRO_ITEM(ITEM, CANVAS) { if (ITEM){ ITEM->hide (); ITEM->queue_update (CANVAS); delete ITEM; ITEM=NULL; }}

#define CAIRO_BASIC_COLOR(I) BasicColors[(I)<0?0: (I)>9? 9 : I]

#define CAIRO_COLOR_RED     BasicColors[0]
#define CAIRO_COLOR_GREEN   BasicColors[1]
#define CAIRO_COLOR_CYAN    BasicColors[2]
#define CAIRO_COLOR_YELLOW  BasicColors[3]
#define CAIRO_COLOR_BLUE    BasicColors[4]
#define CAIRO_COLOR_MAGENTA BasicColors[5]
#define CAIRO_COLOR_GREY1   BasicColors[6]
#define CAIRO_COLOR_ORANGE  BasicColors[7]
#define CAIRO_COLOR_BLACK   BasicColors[8]
#define CAIRO_COLOR_WHITE   BasicColors[9]

typedef enum {
        CAIRO_COLOR_RED_ID,
        CAIRO_COLOR_GREEN_ID,
        CAIRO_COLOR_CYAN_ID,
        CAIRO_COLOR_YELLOW_ID,
        CAIRO_COLOR_BLUE_ID,
        CAIRO_COLOR_MAGENTA_ID,
        CAIRO_COLOR_GREY1_ID,
        CAIRO_COLOR_ORANGE_ID,
        CAIRO_COLOR_BLACK_ID,
        CAIRO_COLOR_WHITE_ID,
} CAIRO_BASIC_COLOR_IDS;

#define CAIRO_LINE_SOLID       0
#define CAIRO_LINE_DASHED      1
#define CAIRO_LINE_ON_OFF_DASH 2

#define CAIRO_ANCHOR_N      0
#define CAIRO_ANCHOR_NW     4
#define CAIRO_ANCHOR_W      8
#define CAIRO_ANCHOR_SW     12
#define CAIRO_ANCHOR_S      16
#define CAIRO_ANCHOR_SE     20
#define CAIRO_ANCHOR_E      24
#define CAIRO_ANCHOR_NE     28
#define CAIRO_ANCHOR_CENTER -1

#define CAIRO_JUSTIFY_CENTER  0
#define CAIRO_JUSTIFY_LEFT    1
#define CAIRO_JUSTIFY_RIGHT   2
#define CAIRO_JUSTIFY_TOP     3
#define CAIRO_JUSTIFY_BOTTOM  4


// #define __CIP_DEBUG

class cairo_item{
public:
        cairo_item () { 
                n=0; xy = NULL; 
                v0.x = v0.y = 0.;  
                set_line_width (1.0); set_stroke_rgba (0); set_fill_rgba (0); set_line_style (0);
                bbox[0]=bbox[1]=0.; bbox[2]=bbox[3]=0.; 
                grabbed = false;
                show();  
        };
	virtual ~cairo_item () { hide(); };
	virtual void draw (cairo_t* cr, double alpha=1.0, gboolean tr=true) {};
        virtual void set_xy_fast (int i, double x, double y) { xy[i].x=x, xy[i].y=y; };
        virtual void set_xy_test (int i, double x, double y) { 
                if (x == NAN || x == -NAN) x = 0.; // NAN catch
                if (y == NAN || y == -NAN) y = 0.; // NAN catch
                xy[i].x=x, xy[i].y=y; 
        };
        virtual void set_xy_hold_logmin (int i, double x, double y, double logmin) { 
                if (x == NAN || x == -NAN) x = 0.; // NAN catch
                if (y == NAN || y == -NAN) y = 0.; // NAN catch
                if (y < logmin) y = i > 1 ? xy[i-1].y : logmin;
                xy[i].x=x, xy[i].y=y; 
        };
        virtual void set_xy (int i, double x, double y) { if (i >= 0 && i < n) xy[i].x=x, xy[i].y=y; };
        virtual void set_xy (int i, int j) { if (i >= 0 && i < n && j >= 0 && j < n) xy[i].x= xy[j].x, xy[i].y= xy[j].y; };
        virtual void get_xy (int i, double &x, double &y) { if (i >= 0 && i < n) x=xy[i].x, y=xy[i].y; };
        void map_xy (void (*map_xy_func)(double &x, double &y)) {
                for (int i=0; i<n; ++i)
                        (*map_xy_func) (xy[i].x, xy[i].y);
        };

	virtual void set_text (const gchar *text) {};
	virtual void set_text (double x, double y, const gchar *text) {};
        virtual void update_bbox (gboolean add_lw=true) {
                if (n){
                        bbox[2]=bbox[0]=xy[0].x; 
                        bbox[3]=bbox[1]=xy[0].y;
                        for (int i=1; i<n; ++i){
                                if (bbox[0] > xy[i].x) bbox[0] = xy[i].x; 
                                if (bbox[2] < xy[i].x) bbox[2] = xy[i].x; 
                                if (bbox[1] > xy[i].y) bbox[1] = xy[i].y; 
                                if (bbox[3] < xy[i].y) bbox[3] = xy[i].y; 
                        }
                        if (add_lw){
                                bbox[0] -= lw;
                                bbox[2] += lw;
                                bbox[1] -= lw;
                                bbox[3] += lw;
                        }
                        bbox[0] += v0.x;
                        bbox[1] += v0.y;
                        bbox[2] += v0.x;
                        bbox[3] += v0.y;
                }
        };
        int get_n_nodes() { return n; };
        void get_bb_min (double &x, double &y) { x=bbox[0]; y=bbox[1]; };
        void get_bb_max (double &x, double &y) { x=bbox[2]; y=bbox[3]; };
        virtual gboolean check_grab_bbox (double x, double y) {
                // g_message ("check_grab_bbox x=%g %g %g, y=%g %g %g", x, bbox[0], bbox[2], y, bbox[1], bbox[3] );
                return (x >= bbox[0] && x <= bbox[2] && y >= bbox[1] && y <= bbox[3]) ? true : false;
        }
        virtual gboolean check_grab_bbox_dxy (double x, double y, double dx, double dy) {
                return (x >= bbox[0]-dx && x <= bbox[2]+dx && y >= bbox[1]-dy && y <= bbox[3]+dy) ? true : false;
        }
	virtual double distance (double x, double y) { 
                double dx = x-v0.x;
                double dy = y-v0.y;
                return sqrt (dx*dx+dy*dy); 
        };

        void show () { show_flag = true; };
        void hide () { show_flag = false; };
        void grab () { grabbed = true; };
        void ungrab () { grabbed = false; };
        gboolean is_grabbed () { return grabbed; };

        void set_position (double x, double y) { v0.x=x; v0.y=y; };
        void set_line_width (double line_width) { lw = line_width; };
        void set_line_style (int line_style) { ls = line_style; };
        void set_stroke_rgba (int basic_color_index) {
                for (int i=0; i<4; ++i)
                        stroke_rgba[i]=BasicColors[basic_color_index % BASIC_COLORS][i];
        };
        void set_stroke_rgba (float c[4]) {
                for (int i=0; i<4; ++i)
                        stroke_rgba[i]=c[i];
        };
        void set_stroke_rgba (GdkRGBA *rgba) {
                stroke_rgba[0]=rgba->red; stroke_rgba[1]=rgba->green; stroke_rgba[2]=rgba->blue; stroke_rgba[3]=rgba->alpha;
        };
        void set_stroke_rgba (const gchar *color) {
                GdkRGBA rgba;
                if (gdk_rgba_parse (&rgba, color))
                        set_stroke_rgba (&rgba); 
        };
        void set_stroke_rgba (double r, double g, double b, double a=0.) {
                stroke_rgba[0]=r; stroke_rgba[1]=g; stroke_rgba[2]=b; stroke_rgba[3]=a;
        };
        void set_fill_rgba (double r, double g, double b, double a=0.) {
                fill_rgba[0]=r; fill_rgba[1]=g; fill_rgba[2]=b; fill_rgba[3]=a;
        };
        void set_fill_rgba (float c[4]) {
                for (int i=0; i<4; ++i)
                        fill_rgba[i]=c[i];
        };
        void set_fill_rgba (GdkRGBA *rgba) {
                fill_rgba[0]=rgba->red; fill_rgba[1]=rgba->green; fill_rgba[2]=rgba->blue; fill_rgba[3]=rgba->alpha;
        };
        void set_fill_rgba (const gchar *color) {
                GdkRGBA rgba;
                if (gdk_rgba_parse (&rgba, color))
                        set_fill_rgba (&rgba); 
        };
        void set_fill_rgba (int basic_color_index) {
                for (int i=0; i<4; ++i)
                        fill_rgba[i]=BasicColors[basic_color_index % BASIC_COLORS][i];
        };
        virtual void queue_update_bbox (GtkWidget* imgarea) {
                gtk_widget_queue_draw_area (imgarea, bbox[0], bbox[1], bbox[2], bbox[3]);
                update_bbox ();
                gtk_widget_queue_draw_area (imgarea, bbox[0], bbox[1], bbox[2], bbox[3]);
        }
        virtual void queue_update (GtkWidget* imgarea) {
                update_bbox ();
                gtk_widget_queue_draw (imgarea);
        };
        
protected:
        int n;
        cairo_point v0;
        cairo_point *xy;
        double lw;
        int ls;
        double stroke_rgba[4];
        double fill_rgba[4];
        gboolean show_flag;
        double bbox[4];
        gboolean grabbed;
};

class cairo_item_path : public cairo_item{
public:
	cairo_item_path (int nodes) { xy = g_new (cairo_point, nodes); n=nodes; impulse_floor=0.; mark_radius=1.; linemode=0; };
	cairo_item_path (cairo_item_path *cip) { xy = g_new (cairo_point, n=cip->get_n_nodes());  impulse_floor=0.; mark_radius=1.; linemode=0; };
	virtual ~cairo_item_path () { g_free (xy); };
	
	void set_linemode (gint m) { linemode=m; };
	void set_impulse_floor (double y) { impulse_floor=y; };
	void set_mark_radius (double r) { mark_radius=r; };
        
        virtual void draw (cairo_t* cr, double alpha=1.0, gboolean tr=true) { // add qf???
                if (show_flag && n > 1){
                        cairo_save (cr);
                        cairo_translate (cr, v0.x, v0.y);
                        cairo_set_source_rgba (cr, stroke_rgba[0], stroke_rgba[1], stroke_rgba[2], stroke_rgba[3]);
                        cairo_set_line_width (cr, lw); 
#ifdef __CIP_DEBUG
                        g_print ("cip::draw  M %+8.3f,%+8.3fg\n", xy[0].x, xy[0].y);
#endif
                        switch (linemode){
                        case 0: // connect solid
                                cairo_move_to (cr, xy[0].x, xy[0].y);
                                for (int i=1; i<n; ++i){
                                        cairo_line_to (cr, xy[i].x, xy[i].y);
#ifdef __CIP_DEBUG
                                        g_print ("cip::draw  L %+8.3f,%+8.3fg\n", xy[i].x, xy[i].y);
#endif
                                }
                                break;
                        case 1: // dots (h-bars, lw) 
                                for (int i=0; i<n; ++i){
                                        cairo_move_to (cr, xy[i].x, xy[i].y);
                                        cairo_line_to (cr, xy[i].x+lw, xy[i].y);
                                }
                                break;
                        case 2: // impulse 
                                for (int i=0; i<n; ++i){
                                        cairo_move_to (cr, xy[i].x, impulse_floor);
                                        cairo_line_to (cr, xy[i].x, xy[i].y);
                                }
                                break;
                        case 3: // X marks
                                for (int i=0; i<n; ++i){
                                        cairo_move_to (cr, xy[i].x-mark_radius, xy[i].y-mark_radius);
                                        cairo_line_to (cr, xy[i].x+mark_radius, xy[i].y+mark_radius);
                                        cairo_move_to (cr, xy[i].x+mark_radius, xy[i].y-mark_radius);
                                        cairo_line_to (cr, xy[i].x-mark_radius, xy[i].y+mark_radius);
                                }
                                break;
                        default:
                                g_warning ("cairo_item_path::draw called with unhandled linemode.");
                                break;
                        }
                        cairo_stroke (cr);
                        cairo_restore (cr);
                } else {
                        g_warning ("cairo_item_path::draw called with insufficent node number.");
                }
        };

private:
        gint linemode;
        double impulse_floor;
        double mark_radius;
};

class cairo_item_segments : public cairo_item{
public:
	cairo_item_segments (int nodes) { xy = g_new (cairo_point, nodes); n=nodes; };
	cairo_item_segments (cairo_item_path *cip) { xy = g_new (cairo_point, n=cip->get_n_nodes()); };
	virtual ~cairo_item_segments () { g_free (xy); };
	
        virtual void draw (cairo_t* cr, double alpha=1.0, gboolean tr=true) { // add qf???
                if (show_flag){
                        cairo_save (cr);
                        cairo_translate (cr, v0.x, v0.y);
                        cairo_set_source_rgba (cr, stroke_rgba[0], stroke_rgba[1], stroke_rgba[2], stroke_rgba[3]);
                        cairo_set_line_width (cr, lw); 
                        for (int i=0; i<n; ){
                                cairo_move_to (cr, xy[i].x, xy[i].y); ++i;
                                cairo_line_to (cr, xy[i].x, xy[i].y); ++i;
                                cairo_stroke (cr);
                        }
                        cairo_restore (cr);
                }
        };

private:
};

class cairo_item_path_closed : public cairo_item{
public:
	cairo_item_path_closed (int nodes) { xy = g_new (cairo_point, nodes); n=nodes; };
	virtual ~cairo_item_path_closed () { g_free (xy); };
	
        virtual void draw (cairo_t* cr, double alpha=1.0, gboolean tr=true) { // add qf???
                if (show_flag && n>1){
                        cairo_save (cr);
                        if (tr)
                                cairo_translate (cr, v0.x, v0.y);
                        cairo_set_source_rgba (cr, stroke_rgba[0], stroke_rgba[1], stroke_rgba[2], alpha*stroke_rgba[3]);
                        cairo_set_line_width (cr, lw); 
                        cairo_move_to (cr, xy[0].x, xy[0].y);
                        for (int i=1; i<n; ++i){
                                cairo_line_to (cr, xy[i].x, xy[i].y);
                        }
                        cairo_close_path (cr);
                        // cairo_set_source_rgba (cr, fill_rgba[0], fill_rgba[1], fill_rgba[2], alpha*fill_rgba[3]);
                        cairo_fill (cr);
                        cairo_restore (cr);
                } else {
                        g_warning ("cairo_item_path_closed::draw called with insufficent node number.");
                }
        };

private:
};

class cairo_item_rectangle : public cairo_item{
public:
	cairo_item_rectangle () { xy = g_new (cairo_point, 2); n=2; };
	cairo_item_rectangle (double x1, double y1, double x2, double y2) { xy = g_new (cairo_point, 2); n=2; xy[0].x=x1, xy[0].y=y1; xy[1].x=x2, xy[1].y=y2; };
	virtual ~cairo_item_rectangle () { g_free (xy); };
	
        virtual void draw (cairo_t* cr, double alpha=1.0, gboolean tr=true) { // add qf???
                if (show_flag){
#ifdef __CIP_DEBUG
                        g_print ("cip::draw  R %+8.3f,%+8.3fg : ", xy[0].x, xy[0].y);
                        g_print (" %+8.3f,%+8.3fg\n", xy[1].x, xy[1].y);
#endif
                        cairo_save (cr);
                        if (tr)
                                cairo_translate (cr, v0.x, v0.y);
                        cairo_set_line_width (cr, lw); 
                        cairo_set_source_rgba (cr, fill_rgba[0], fill_rgba[1], fill_rgba[2], alpha*fill_rgba[3]);
                        cairo_rectangle (cr,  xy[0].x, xy[0].y,   xy[1].x-xy[0].x, xy[1].y-xy[0].y);
                        cairo_fill (cr);
                        cairo_set_source_rgba (cr, stroke_rgba[0], stroke_rgba[1], stroke_rgba[2], alpha*stroke_rgba[3]);
                        cairo_rectangle (cr,  xy[0].x, xy[0].y,   xy[1].x-xy[0].x, xy[1].y-xy[0].y);
                        cairo_stroke (cr);
                        cairo_restore (cr);
                }
        };

private:
};

class cairo_item_circle : public cairo_item{
public:
	cairo_item_circle (int n_circels=1) { xy = g_new (cairo_point, n_circels); n=n_circels; radius=1.; };
	cairo_item_circle (double x, double y, double r) { xy = g_new (cairo_point, 1); n=1; xy[0].x=x, xy[0].y=y; radius=r; };
	virtual ~cairo_item_circle () { g_free (xy); };
        void set_radius (double r) { radius = r; };
        virtual void update_bbox (gboolean add_lw=true) {
                bbox[0] = xy[0].x-radius-lw;
                bbox[1] = xy[0].y-radius-lw;
                bbox[2] = xy[0].x+radius+lw;
                bbox[3] = xy[0].y+radius+lw;
        };

        virtual void draw (cairo_t* cr, double alpha=1.0, gboolean tr=true) { // add qf???
                if (show_flag){
                        cairo_save (cr);
                        cairo_translate (cr, v0.x, v0.y);
                        cairo_set_line_width (cr, lw);
                        for (int i=0; i<n; ++i){
                                cairo_arc (cr,  xy[i].x, xy[i].y,  radius, 0., 2.*M_PI);
                                cairo_set_source_rgba (cr, fill_rgba[0], fill_rgba[1], fill_rgba[2], fill_rgba[3]);
                                cairo_fill (cr);
                                cairo_set_source_rgba (cr, stroke_rgba[0], stroke_rgba[1], stroke_rgba[2], stroke_rgba[3]);
                                cairo_arc (cr,  xy[i].x, xy[i].y,  radius, 0., 2.*M_PI);
                                cairo_stroke (cr);
                        }
                        cairo_restore (cr);
                }
        };

private:
        double radius;
};


// font_faces: "Georgia" "Ununtu"
class cairo_item_text : public cairo_item{
public:
	cairo_item_text () { \
                xy = g_new (cairo_point, 1); n=1; t=NULL; 
                xy[0].x=0., xy[0].y=0.; 
                pango_font = NULL;
                font_face = g_strdup ("Ununtu"); font_size = 16.; t_anchor = 0; t_justify = 0; spacing = 1.1; 
        };
	cairo_item_text (double x, double y, const gchar *text) { 
                xy = g_new (cairo_point, 1); n=1; 
                xy[0].x=0., xy[0].y=0.; 
                v0.x=x, v0.y=y; 
                pango_font = NULL;
                t=g_strdup (text);
                font_face = NULL;
                t_anchor = 0; t_justify = 0; spacing = 1.1;
                set_font_face_size ("Ununtu", 16.);
        };
	virtual ~cairo_item_text () { g_free (xy); if (t) g_free (t); if (font_face) g_free (font_face); if (pango_font) pango_font_description_free (pango_font); };
	virtual void set_text (const gchar *text) { if (t) g_free (t); t=g_strdup (text); };
	virtual void set_text (double x, double y, const gchar *text) {  
                v0.x=x, v0.y=y;
                if (t)
                        g_free (t);
                t=g_strdup (text); 
        };
	virtual void set_font_face_size (const gchar *face, double size, cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL, cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL) { 
                if (font_face)
                        g_free (font_face);
                font_face = g_strdup (face); 
                font_size = size; font_slant = slant; font_weight = weight;
        };
        virtual void set_pango_font (const gchar *pango_font_name) { 
                if (pango_font) pango_font_description_free (pango_font);
                pango_font = pango_font_description_from_string (pango_font_name); 
        };
	virtual void set_anchor (int anchor) { t_anchor = anchor; }; 
	virtual void set_justify (int justify) { t_justify = justify; }; 
	virtual void set_spacing (double line_spacing) { spacing = line_spacing > 0. ? 1.+line_spacing : -1.-line_spacing; }; 
        virtual void update_bbox (gboolean add_lw=true) {
                // needs pango, cr, etc, updated on draw
        };
        virtual void queue_update (GtkWidget* imgarea) {
                        gtk_widget_queue_draw (imgarea);
        };
        virtual void queue_update_bbox (GtkWidget* imgarea) {
                if (bbox[0] == 0 && bbox[2] == 0.)
                        gtk_widget_queue_draw (imgarea);
                else
                        gtk_widget_queue_draw_area (imgarea, bbox[0], bbox[1], bbox[2], bbox[3]);
        };

        virtual void draw (cairo_t* cr, double alpha=0.0, gboolean tr=true);

private:
        gchar *t;
        gchar *font_face;
        double font_size;
        cairo_font_slant_t font_slant;
        cairo_font_weight_t font_weight;
        PangoFontDescription *pango_font;
        double spacing;
        gint    t_anchor;
        gint    t_justify;
};

#endif
