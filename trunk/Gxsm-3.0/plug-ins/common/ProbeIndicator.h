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

# include <complex>
# include <fftw3.h>
#define c_re(c) ((c)[0])
#define c_im(c) ((c)[1])


#define SCOPE_ON  1
#define SCOPE_FFT 2
#define SCOPE_DBG 256

class hud_object {
public:
        hud_object(){
                // skeleton
                irad = 100.; orad = irad*1.2; delta = irad*0.05;
                rsz = irad/5.; asp=1.67; 

                // empty elements
                marks = NULL;
                indicators = NULL;
                tics = NULL;
                horizon = NULL;

                // default setup
                transparency = 0.5;
                set_color (cc_tics, CAIRO_COLOR_BLACK_ID, transparency);
                set_color (cc_gauge, CAIRO_COLOR_CYAN_ID, transparency);
                set_color (cc_marks, CAIRO_COLOR_CYAN_ID, transparency);
                set_color (cc_indpos, CAIRO_COLOR_RED_ID, transparency);
                set_color (cc_indneg, CAIRO_COLOR_BLUE_ID, transparency);
                set_color (cc_horizon, CAIRO_COLOR_GRAY5_ID, transparency);

                // gauge skeleton
                ic = new cairo_item_circle (0.,0., irad);
                ic->set_stroke_rgba (cc_gauge);
                ic->set_fill_rgba (0.,0.,0.,0.0);
                ic->set_line_width (2.0);
                oc = new cairo_item_circle (0.,0., orad);
                oc->set_stroke_rgba (cc_gauge);
                oc->set_fill_rgba (cc_gauge);
                oc->set_line_width (2.0);
                oc->set_fill_rgba (0.,0.,0.,0.05);


        };
        ~hud_object(){
                delete ic;
                delete oc;
                g_slist_free_full (indicators, (GDestroyNotify)hud_object::remove_cairo_item);
                g_slist_free_full (marks, (GDestroyNotify)hud_object::remove_cairo_item);
        };
        static void set_color(float c[4], int id, float alpha){
                c[0]=BasicColors[id][0];
                c[1]=BasicColors[id][1];
                c[2]=BasicColors[id][2];
                c[3]=alpha;
        };
        
        static void remove_cairo_item(cairo_item *x) { delete x; };
        static void draw_cairo_item(cairo_item *x, cairo_t* cr) { x->draw (cr); };
        static void hide_cairo_item(cairo_item *x, void* data) { x->hide (); };
        static void show_cairo_item(cairo_item *x, void* data) { x->show (); };
        static void q_update_cairo_item(cairo_item *x, GtkWidget* ima) { x->queue_update (ima); };

        // marker arrow
        cairo_item_path_closed* add_mark (const gchar *id, double pos, double z, int level=0, int pointing=1, double scale=1.){
                double ir=level == 0 ? irad : orad;
                double y=level == 0 ? ir*z*pointing : ir-rsz*scale*pointing;
                double xx = rsz*scale/asp;
                cairo_item_path_closed *m = new cairo_item_path_closed (5);
                m->set_id(id);
                m->set_angle(pos/400.*2.*M_PI);
                m->set_stroke_rgba (cc_marks);
                m->set_fill_rgba (cc_marks);
                m->set_position (0., 0.);
                m->set_xy (0, 0., y);
                m->set_xy (1, +xx, y+rsz*scale*pointing);
                m->set_xy (2, +xx, ir);
                m->set_xy (3, -xx, ir);
                m->set_xy (4, -xx, y+rsz*scale*pointing);
                m->set_stroke_rgba (cc_marks);
                m->set_fill_rgba (cc_marks);
                m->set_line_width (1.0);
                marks = g_slist_prepend (marks, m);
                return m;
        };
        void set_mark_color(cairo_item_path_closed* m, int id){
                float cc_tmp[4];
                if (id >=0)
                        set_color (cc_tmp, id, transparency);
                else
                        set_color (cc_tmp, CAIRO_COLOR_CYAN_ID, transparency);
                m->set_stroke_rgba (cc_tmp);
                m->set_fill_rgba (cc_tmp);
        };
        void set_mark_pos(cairo_item_path_closed* m, double pos){
                m->set_angle(pos/400.*2.*M_PI);
        };
        void set_mark_len(cairo_item_path_closed* m, double z){
                double y=irad*z;
                m->set_xy (0, 0., y);
                m->set_xy (1, +rsz/asp, y+rsz);
                m->set_xy (4, -rsz/asp, y+rsz);
        };

        // Indicator bar
        cairo_item_arc* add_indicator (const gchar *id, double pos, double val, int level=0, int levels=1){
                cairo_item_arc *arc = new cairo_item_arc (0.,0.,
                                                          irad+2+level*(orad-irad-delta)/levels,
                                                          (orad-irad-delta)/levels,
                                                          pos, val, 2.*M_PI/400.);
                arc->set_id(id);
                if (val < 0.) arc->set_stroke_rgba (cc_indneg);
                else arc->set_stroke_rgba (cc_indpos);
                indicators = g_slist_prepend (indicators, arc);
                return arc;
        };

        void set_indicator_color(cairo_item_arc* a, int id, int sig){
                float cc_tmp[4];
                if (id >=0)
                        set_color (cc_tmp, id, transparency);
                else
                        set_color (cc_tmp, sig>0? CAIRO_COLOR_RED_ID:CAIRO_COLOR_BLUE_ID, transparency);
                a->set_stroke_rgba (cc_tmp);
                a->set_fill_rgba (cc_tmp);
        };
        void set_indicator_val (cairo_item_arc *arc, double pos, double val){
                arc->set_arc (pos+(val>0.?1.:-1.), val);
        };
        
        cairo_item_arc* add_tics (const gchar *id, double pos, double val, int n, double dtic){
                cairo_item_arc *arc = new cairo_item_arc (0.,0., orad, (orad-irad)*0.3, pos+(dtic>0.?-0.5:0.5), val, 2.*M_PI/400., n, dtic);
                arc->set_id(id);
                arc->set_stroke_rgba (cc_tics);
                tics = g_slist_prepend (tics, arc);
                return arc;
        };

        cairo_item_path* add_horizon (const gchar *id, double pos, double z, gint n){
                double y=irad*z;
                cairo_item_path *h = new cairo_item_path (n);
                h->set_id(id);
                h->set_angle(pos/400.*2.*M_PI);
                h->set_stroke_rgba (cc_horizon);
                h->set_position (0., y);
                h->set_line_width (2.0);
                horizon = g_slist_prepend (horizon, h);
                return h;
        };
        void set_horizon_color(cairo_item_path* p, int id){
                float cc_tmp[4];
                set_color (cc_tmp, id, transparency);
                p->set_stroke_rgba (cc_tmp);
                p->set_fill_rgba (cc_tmp);
        };


        void queue_update (GtkWidget* imgarea) {
                ic->queue_update (imgarea);
                oc->queue_update (imgarea);
                g_slist_foreach (marks, (GFunc)hud_object::q_update_cairo_item, imgarea);
                g_slist_foreach (indicators, (GFunc)hud_object::q_update_cairo_item, imgarea);
                g_slist_foreach (tics, (GFunc)hud_object::q_update_cairo_item, imgarea);
                g_slist_foreach (horizon, (GFunc)hud_object::q_update_cairo_item, imgarea);
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

        float transparency;
        float cc_tics[4];
        float cc_gauge[4];
        float cc_indpos[4];
        float cc_indneg[4];
        float cc_horizon[4];
        float cc_marks[4];
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

        gint run_fft (gint len, gfloat *data, gfloat *psd_db, double min, double max, double mu=1.){
                static gint n=0;
                static double *in=NULL;
                //static double *out=NULL;
                static fftw_complex *out=NULL;
                static fftw_plan plan=NULL;

                if (n != len || !in || !out || !plan){
                        if (plan){
                                fftw_destroy_plan (plan); plan=NULL;
                        }
                        // free temp data memory
                        if (in) delete[] in; in=NULL;
                        if (out) delete[] out; out=NULL;
                        n=0;
                        if (len < 2) // clenaup only, exit
                                return 0;

                        n = len;
                        // get memory for complex data
                        in  = new double [n];
                        out = new fftw_complex [n];
                        //out = new double [n];

                        // create plan for fft
                        plan = fftw_plan_dft_r2c_1d (n, in, out, FFTW_ESTIMATE);
                        //plan = fftw_plan_r2r_1d (n, in, out, FFTW_REDFT00, FFTW_ESTIMATE);
                        if (plan == NULL)
                                return -1;
                }

                
                // prepare data for fftw
                double x=0.;
                for (int i = 0; i < n; ++i)
                        x+=data[i];
                x/=n;
                // prepare data for fftw
                double s=2.*M_PI/(n-1);
                double a0=0.54;
                double a1=1.-a0;
                int n2=n/2;
                for (int i = 0; i < n; ++i){
                        double w=a0+a1*cos((i-n2)*s);
                        in[i] = (data[i]-x)*w;
                        //in[i] = min*sin(100.*i*M_PI/n)*w;
                        //in[i] += max*sin(300.*i*M_PI/n)*w;
                }
                //g_print("FFTin %g",in[0]);
                
                // compute transform
                fftw_execute (plan);

                //double N=2*(n-1);
                //double scale = 1./(max*2*(n-1));
                double scale = 5./(max*(n-1));
                double db=0.;
                for (int i=0; i<n; ++i){
                        db = scale * sqrt(c_re(out[i])*c_re(out[i]) + c_im(out[i])*c_im(out[i]));
                        //db = scale * out[i];
                        //psd_db[i] = db;
                        if (db > min)
                                psd_db[i] = (1.-mu)*psd_db[i] + mu*20.*log(db);
                        else
                                psd_db[i] = (1.-mu)*psd_db[i] + mu*20.*log(min);
                        //g_print("%d %g %g %g %g\n",i,data[i],in[i], db, psd_db[i]);

                }
                return 0;
        };
        
private:  
        gint       modes;
        
        gint       hud_size;

        guint      timer_id;

        GtkWidget  *canvas;

        // remplaced with: cairo_item_rectangle / text / path
        cairo_item_text  *info;
        hud_object *probe;
        cairo_item_arc *ipos, *ineg, *ipos2, *ineg2;
        cairo_item_arc *fpos, *fneg, *fpos2, *fneg2;
        cairo_item_path_closed *tip;
        cairo_item_path_closed *m1, *m2;
        cairo_item_path *horizon[3];
};
