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

#include "gtk/gtk.h"
#include <pango/pangocairo.h>
#include "cairo_item.h"

#include "vectorutil.h"

float BasicColors[][4] = { // "red","green","cyan","yellow","blue","magenta","grey"
        { 1.0, 0.0, 0.0, 1.0 }, // RGBA red
        { 0.0, 1.0, 0.0, 1.0 }, // green
        { 0.0, 1.0, 1.0, 1.0 }, // cyan
        { 1.0, 1.0, 0.0, 1.0 }, // yellow
        { 0.0, 0.0, 1.0, 1.0 }, // blue
        { 1.0, 0.0, 1.0, 1.0 }, // magenta
        { 0.95, 0.85, 0.85, 0.6 }, // grey1
        { 1.0, 1.0, 0.5, 1.0 }, // orange
        { 0.0, 0.0, 0.0, 1.0 }, // black
        { 1.0, 1.0, 1.0, 1.0 } // black
};

int cairo_basic_color_lookup (const gchar *color){
        switch (color[0]){
        case 'r': return 0;
        case 'g': return 1;
        case 'c': return 2;
        case 'y': return 3;
        case 'b': return 4;
        case 'm': return 5;
        default: return 0;
        }
}

#if 0
static void
draw_text (cairo_t *cr)
{
#define RADIUS 150
#define N_WORDS 10
#define FONT "Sans Bold 27"

  PangoLayout *layout;
  PangoFontDescription *desc;
  int i;

  /* Center coordinates on the middle of the region we are drawing
   */
  cairo_translate (cr, RADIUS, RADIUS);

  /* Create a PangoLayout, set the font and text */
  layout = pango_cairo_create_layout (cr);

  pango_layout_set_text (layout, "Text", -1);
  desc = pango_font_description_from_string (FONT);
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free (desc);

  /* Draw the layout N_WORDS times in a circle */
  for (i = 0; i < N_WORDS; i++)
    {
      int width, height;
      double angle = (360. * i) / N_WORDS;
      double red;

      cairo_save (cr);

      /* Gradient from red at angle == 60 to blue at angle == 240 */
      red   = (1 + cos ((angle - 60) * G_PI / 180.)) / 2;
      cairo_set_source_rgb (cr, red, 0, 1.0 - red);

      cairo_rotate (cr, angle * G_PI / 180.);

      /* Inform Pango to re-layout the text with the new transformation */
      pango_cairo_update_layout (cr, layout);

      pango_layout_get_size (layout, &width, &height);
      cairo_move_to (cr, - ((double)width / PANGO_SCALE) / 2, - RADIUS);
      pango_cairo_show_layout (cr, layout);

      cairo_restore (cr);
    }
            width, height = 0,0
              angle = (360. * i) / N_WORDS;
              
              cr.save ()

              red   = (1 + math.cos ((angle - 60) * math.pi / 180.)) / 2
              cr.set_source_rgb ( red, 0, 1.0 - red)
              cr.rotate ( angle * math.pi / 180.)
              #/* Inform Pango to re-layout the text with the new transformation */
              PangoCairo.update_layout (cr, layout)
              width, height = layout.get_size()
              cr.move_to ( - (float(width) / 1024.) / 2, - RADIUS)
              PangoCairo.show_layout (cr, layout)
              cr.restore()
              
  /* free the layout object */
  g_object_unref (layout);
}
#endif


void cairo_item_text::draw (cairo_t* cr, double angle, gboolean tr) {
        if (show_flag){
                double y = 0;
                gchar **lines = g_strsplit (t,"\n",0);
                cairo_text_extents_t te;
                PangoLayout *layout = NULL;

                cairo_save (cr);
                if (pango_font){
                        layout = pango_cairo_create_layout (cr);  /* Create a PangoLayout, set the font and text */
                } else {
                        cairo_select_font_face (cr, font_face, font_slant, font_weight);
                        cairo_set_font_size (cr, font_size);
                }
                cairo_set_source_rgba (cr, stroke_rgba[0], stroke_rgba[1], stroke_rgba[2], stroke_rgba[3]);
                cairo_set_line_width (cr, lw); 
                        
                //double xy0[3] = {v0.x, v0.y, 0. };
                cairo_translate (cr, v0.x, v0.y);
                cairo_translate (cr, xy[0].x, xy[0].y);
                if (fabs (angle) > 0.){
                        //      double M[3][3];
                        //double r[3] = {v0.x, v0.y, 0. };
                        //double zero[3] = {0.,0.,0.};

                        cairo_rotate (cr, angle * M_PI / 180.);

                        //g_print_vec ("XY0i", xy0);
                        //make_mat_rot_xy_tr (M, -angle, zero);
                        //mul_mat_vec (xy0, M, r);
                        //g_print_vec ("XY0r", xy0);
                }

                
                for (gchar **l=lines; *l; ++l){
                        double x0,y0;
                        double xj = 0.;
                        double yj = 0.;

                        if (pango_font){
                                int w,h;
                                pango_layout_set_text (layout, *l, -1);
                                pango_layout_set_font_description (layout, pango_font);
                                pango_cairo_update_layout (cr, layout);
                                pango_layout_get_size (layout, &w, &h);
                                // g_print ("PFONT WH=%g, %g\n", (double)w / PANGO_SCALE, (double)h / PANGO_SCALE);
                                te.width = w / PANGO_SCALE; te.height = h / PANGO_SCALE;
                        } else
                                cairo_text_extents (cr, *l, &te);

                        xj = te.width / 2.;
                        yj = te.height / 2;
                        switch (t_anchor){
                        case CAIRO_ANCHOR_W:  xj = 0.; break;
                        case CAIRO_ANCHOR_E:  xj = te.width; break;
                        case CAIRO_ANCHOR_S:  yj = 0.; break;
                        case CAIRO_ANCHOR_N:  yj = te.height; break;
                        default: break; // this is CAIRO_ANCHOR_CENTER
                        }
                        
                        cairo_move_to (cr, x0 = - te.x_bearing - xj, y0 = y - te.y_bearing - yj);

                        if (pango_font)
                                pango_cairo_show_layout (cr, layout);
                        else
                                cairo_show_text (cr, *l);


                        if (y == xy[0].y){
                                bbox[0]=x0;
                                bbox[1]=y0;
                                bbox[2]=x0+te.width;
                                bbox[3]=y0+te.height;
                        } else {
                                if (bbox[0]>x0) bbox[0]=x0;
                                if (bbox[1]>y0) bbox[1]=y0;
                                if (bbox[2]<x0+te.width) bbox[2]=x0+te.width;
                                if (bbox[3]<y0+te.height) bbox[3]=y0+te.height;
                        }
                        y -= font_size * spacing;
                }

                bbox[0] += v0.x;
                bbox[1] += v0.y;
                bbox[2] += v0.x;
                bbox[3] += v0.y;

                cairo_restore (cr);

                g_strfreev (lines);

                if (layout)
                        g_object_unref (layout);
        }
}
