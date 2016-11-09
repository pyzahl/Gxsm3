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

/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

#ifndef APP_VOBJ_H
#define APP_VOBJ_H

#include <config.h>

#include <gtk/gtk.h>

#include "xsmtypes.h"
#include "gapp_service.h"
#include "scan.h"

#include "gxsm_app.h"
#include "app_vinfo.h"

#include "cairo_item.h"

// this is the type of the current active object...
// we need to get rid of this...
typedef enum { 
        O_NONE,
        O_POINT,
        O_LINE,
        O_RECTANGLE,
        O_POLYLINE,
        O_PARABEL,
        O_CIRCLE,
        O_TRACE,
        O_EVENT,
        O_KSYS
} V_OBJECT_TYPE;

class ProfileControl;

typedef enum { VOBJ_COORD_FROM_MOUSE, VOBJ_COORD_ABSOLUT, VOBJ_COORD_RELATIV } VOBJ_COORD_MODE;

class VObject{
 public:
	VObject (GtkWidget *canvas, double *xy0, int npkt, Point2D *P2d, int pflg=FALSE, VOBJ_COORD_MODE cmode=VOBJ_COORD_FROM_MOUSE, const gchar *lab=NULL, double Marker_scale=1.);
	virtual ~VObject ();

	virtual gboolean check_event(GdkEvent *event, double xy[2]);

	virtual void Update()=0;
	virtual void draw_extra(cairo_t *cr) {};
	virtual void SetUpPos(VOBJ_COORD_MODE cmode=VOBJ_COORD_FROM_MOUSE, int node=-1);
	virtual void SetUpScan();
	virtual void set_offset();
	virtual void set_global_ref();
	virtual void properties();
	virtual void GoLocMax(int r=10);
	virtual void follow_on(){};
	virtual void follow_off(){};
	virtual int follow(){ return FALSE; };
	virtual void AddNode(){};
	virtual void DelNode(){};
	virtual void show_label(int flg = -1);

	void show_profile (gboolean pflg=TRUE);
	void set_xy_node(double *xy_node, VOBJ_COORD_MODE cmode, int node=0);
	void insert_node(double *xy_node=NULL);

	cairo_item* node_marker (cairo_item* item, double *xy, int i);

	void remake_node_markers ();

	void draw (cairo_t *cr);
        void show () { show_flag = TRUE; };
        void hide () { show_flag = FALSE; };

	void Activate ();
	void lock_object (int l) { lock = l? 1:0; };

	void set_color_to_active (); // "blue"
	void set_color_to_inactive (); // "red"
	void set_color_to_hilit ();
	void set_color_to_custom (gfloat fillcolor[4], gfloat outlinecolor[4]);

	void set_marker_scale (double ms) { marker_scale=ms; };
	double get_marker_scale () { return marker_scale/vinfo->GetZfac(); };
	void set_label_offset (double *xy) { label_offset_xy[0]=xy[0]; label_offset_xy[1]=xy[1]; };
	void set_object_label(const gchar *lab){ if (text) g_free (text); text = g_strdup (lab); }

	void set_custom_label_font (const gchar *f) { if (custom_label_font) g_free (custom_label_font); custom_label_font = g_strdup (f); };
	void set_custom_label_color (gfloat c[4]) { custom_label_color [0] = c[0]; custom_label_color [1] = c[1]; custom_label_color [2] = c[2]; custom_label_color [3] = c[3]; };
	void set_custom_element_color (gfloat c[4]) { custom_element_color [0] = c[0]; custom_element_color [1] = c[1]; custom_element_color [2] = c[2]; custom_element_color [3] = c[3]; };
	void set_custom_element_b_color (gfloat c[4]) { custom_element_b_color [0] = c[0]; custom_element_b_color [1] = c[1]; custom_element_b_color [2] = c[2]; custom_element_b_color [3] = c[3]; };

	// Show/Hide controls for spacetime (Layer, Time)
	void set_on_spacetime (gboolean flag, int spacetime[2], int id=0);
	void set_off_spacetime (gboolean flag, int spacetime[2], int id=0);
	void set_spacetime (int spacetime[2]);
	void get_spacetime (int spacetime[2]);
	gboolean is_spacetime (); // is showtime?
	
	// holds ScanEvent, non if NULL
	void set_scan_event (ScanEvent *se) { scan_event = se; };
	ScanEvent *get_scan_event () { return scan_event; };

	int GetRn(double *xyi, int n){ 
		if(n<np && n>=0){
			xyi[0] = xy[n<<1]; xyi[1] = xy[(n<<1)+1]; 
			return FALSE;
		}
		else return TRUE;
	};

	double Dist(int i=0, int j=1){
		double dx=xy[2*j  ]-xy[2*i  ];
		double dy=xy[2*j+1]-xy[2*i+1];
		return sqrt(dx*dx+dy*dy);
	}

	double Area(int i=0, int j=1){
		double dx=xy[2*j  ]-xy[2*i  ];
		double dy=xy[2*j+1]-xy[2*i+1];
		return fabs(dx*dy);
	}

	double Phi(double dx, double dy){
		double q23=0.;
		if(dx<0.)
			q23=180.;
		if(fabs(dx)>1e-5)
			return q23+180.*atan(dy/dx)/M_PI;
		else return dy>0.?90.:-90.;
	}

	double Phi(int i=0, int j=1){
		double dx=xy[2*j  ]-xy[2*i  ];
		double dy=xy[2*j+1]-xy[2*i+1];
		return Phi(dx, dy);
	}

	void save(std::ofstream &o){
		o << "(VObject \"" << name << "\" CustomColor (" << custom_element_color[0] << " " << custom_element_color[1] << " " << custom_element_color[2] << " " << custom_element_color[3] << ")"
		  << std::endl
		  << "   (Label \"" << text << "\" "
		  << "Font \"" << custom_label_font << "\" "
		  << "MarkerSkl (" << marker_scale << ") "
		  << "Color (" << custom_label_color[0] << " " << custom_label_color[1] << " " << custom_label_color[2] << " " << custom_label_color[3] << ") "
		  << "SpaceTimeOnOff ((" << space_time_on[0] << " " << space_time_on[1] << ") (" << space_time_off[0] << " " << space_time_off[1] << ")) "
		  << "SpaceTimeFromUntil (" << (space_time_from_0 ? 1:0) << " " << (space_time_until_00 ? 1:0) << ") "
		  << "Show (" << (label? 1:0) << ") "
		  << ")" << std::endl
		  << "   (NPkte " << np << ")" << std::endl
		  << "   (Coords i X Y (XAng YAng)" << std::endl;
		for(int i=0; i<np; ++i) {
			double x,y;
			x=xy[2*i]; y=xy[2*i+1];
			o << "     (" << i << " " << x << " " << y;
			vinfo->W2Angstroem (x,y);
			o << " (" << x << " " << y << "))" << std::endl;
		}
		if (profile){
			o << "   )" << std::endl 
			  << "   (ProfileActive PathWidthStep (" << path_width << " " << path_step << ") PathSerDimAllG2d (" << path_dimension << " " << series_dimension << " " << series_all << " " << plot_g2d << "))"
			  << ")" << std::endl 
			  << std::endl;
		} else
			o << "))" << std::endl 
			  << std::endl;
	};

	void saveHPGL(std::ofstream &o){
		double x,y;
		int i;
		for(i=0; i<np; ++i){
			x=xy[2*i]; y=xy[2*i+1];
			vinfo->W2Angstroem (x,y);
			if(i == 0) 
				o << "PU " << x << "," << y << ";" << std::endl;
			else
				o << "PD " << x << "," << y << ";" << std::endl;
		}
		if(i == 1) 
			o << "PD " << x << "," << y << ";" << std::endl;
	};
	int obj_id (int newid=0) { if (newid) id = newid; return id; };
	V_OBJECT_TYPE obj_type_id (V_OBJECT_TYPE newtid=O_NONE) { if (newtid!=O_NONE) type_id = newtid; return type_id; };
	gchar *obj_name () { return name; };
	void set_obj_name (const gchar *newname) { if(name) g_free(name); name = g_strdup(newname); };
	gchar *obj_text ( const gchar *t=NULL ) { 
		if (t) { 
			if (text) g_free (text);
			text = g_strdup (t);
			return NULL; 
		} else return text; 
	};
	int obj_num_points () { return np; };
	void obj_get_xy_i (int i, double &x, double &y) { 
		if (i>=0) { // default (i=0...np) is Angstroems -- conveniance hack
			if (i<np){ 
				x=xy[2*i]; y=xy[2*i+1]; 
				vinfo->W2Angstroem (x,y);
			}
		} else { // but -1, ... -np-1 is in pixels!
			i = -i-1;
			if (i<np){ 
				x=xy[2*i] * vinfo->GetQfac (); 
				y=xy[2*i+1] * vinfo->GetQfac (); 
			}
		} 
	};

	// profile
	void   set_profile_path_width (double w=1.) { path_width = w; };
	double get_profile_path_width () { return path_width; };
	void   set_profile_path_step (double s=1.) { path_step = s; };
	double get_profile_path_step () { return path_step; };
	void   set_profile_series_limit (int i, int s=0) { if (i>=0 && i<2) series_limits[i] = s; };
	int    get_profile_series_limit (int i) { if (i>=0 && i<2) return (int)series_limits[i]; else return 0; };
	void   set_profile_path_dimension  (MEM2D_DIM d=MEM2D_DIM_XY) { path_dimension = d; };
	MEM2D_DIM get_profile_path_dimension  () { return path_dimension; };
	void   set_profile_series_dimension  (MEM2D_DIM d=MEM2D_DIM_LAYER) { series_dimension = d; };
	MEM2D_DIM get_profile_series_dimension  () { return series_dimension; };
	void   set_profile_series_all (gboolean all=FALSE) { series_all = all; };
	gboolean get_profile_series_all () { return series_all; };
	void   set_profile_series_pg2d (gboolean pg2d=FALSE) { plot_g2d = pg2d; };
	gboolean get_profile_series_pg2d () { return plot_g2d; };

	void set_osd_style (gboolean flg);

	GtkWidget *canvas;
	GSimpleActionGroup *gs_action_group;

 protected:
	ProfileControl *profile;
	double path_width; // path width or area radius
	double path_step; // path step (average length in path direction)
	double series_limits[2];
	MEM2D_DIM path_dimension;
	MEM2D_DIM series_dimension;
	gboolean series_all; // FALSE: current only, TRUE: plot all series
	gboolean plot_g2d; // FALSE: no 2d grey 2d plot, TRUE: plot also in as 2d grey/false color image

	gint grid_mode; // GRID/CIRC
	double grid_aspect;
	double grid_base;
	gint grid_multiples;
	gint grid_size;

	gchar *name;
	gchar *text;

	int np;
	double marker_scale;
	double label_offset_xy[2];
	double *xy;
	Point2D  *p2d;
	cairo_item **abl;
	cairo_item_text *label;
	cairo_item *arrow_head[6];
	cairo_item *cursors[2];
	cairo_item *avg_area_marks[2*6];
	cairo_item *avg_circ_marks[2*6];
	cairo_item *selected_bbox;
	GtkWidget *popup;
	GtkWidget *statusbar;
	gint statusid;
	ViewInfo *vinfo;

	cairo_item *touched_item;
	double touched_xy[2];

	ScanEvent *scan_event;
	gboolean label_osd_stye;

	float custom_element_color[4];
	float custom_element_b_color[4];
 private:
	int id;
	V_OBJECT_TYPE type_id;
	int lock;
        gboolean show_flag;

	int space_time_now[2], space_time_on[2], space_time_off[2];
	gboolean space_time_from_0, space_time_until_00;

	gchar *custom_label_font;
	float custom_label_color[4];

	GtkWidget *obj_popup_menu;
};


class VObPoint : public VObject{
 public:
	VObPoint(GtkWidget *canvas, double *xy0, Point2D *P2d, int pflg=FALSE, VOBJ_COORD_MODE cmode=VOBJ_COORD_FROM_MOUSE, const gchar *lab=NULL, double Marker_scale=1.);
	virtual ~VObPoint();
	
	virtual void Update();
	virtual void follow_on();
	virtual void follow_off();
	virtual int follow_me(){ return follow; };

 private:
	void update_offset();
	void update_scanposition();
	int follow;
};


class VObLine : public VObject{
 public:
	VObLine(GtkWidget *canvas, double *xy0, Point2D *P2d, int pflg=FALSE, VOBJ_COORD_MODE cmode=VOBJ_COORD_FROM_MOUSE, const gchar *lab=NULL, double Marker_scale=1.);
	virtual ~VObLine();
	virtual void AddNode();

	virtual void Update();

 private:
	double posA, posB;
};

class VObPolyLine : public VObject{
 public:
	VObPolyLine(GtkWidget *canvas, double *xy0, Point2D *P2d, int pflg=FALSE, VOBJ_COORD_MODE cmode=VOBJ_COORD_FROM_MOUSE, const gchar *lab=NULL, double Marker_scale=1.);
	virtual ~VObPolyLine();
	virtual void AddNode();
	virtual void DelNode();

	virtual void Update();
};

class VObTrace : public VObject{
 public:
	VObTrace(GtkWidget *canvas, double *xy0, Point2D *P2d, int pflg=FALSE, VOBJ_COORD_MODE cmode=VOBJ_COORD_FROM_MOUSE, const gchar *lab=NULL, double Marker_scale=1.);
	virtual ~VObTrace();
	virtual void Change(double *xy0);

	virtual void Update();
	private:
	int trlen;
	cairo_item *trail;
};

class VObKsys : public VObject{
 public:
	VObKsys(GtkWidget *canvas, double *xy0, Point2D *P2d, int pflg=FALSE, VOBJ_COORD_MODE cmode=VOBJ_COORD_FROM_MOUSE, const gchar *lab=NULL, double Marker_scale=1.);
	virtual ~VObKsys();

	virtual void Update();
	virtual void draw_extra(cairo_t *cr) {
	  if (atoms) atoms->draw (cr);
	  if (lines) lines->draw (cr);
	  if (bounds) bounds->draw (cr);
	};

 private:
	void calc_grid();
	void destroy_atoms();
	void update_grid();
	cairo_item_segments *lines;
	cairo_item_circle *atoms;
	cairo_item_segments *bounds;
	gint n_atoms;
	gint n_lines;
	gint n_bounds;
};

class VObParabel : public VObject{
 public:
	VObParabel(GtkWidget *canvas, double *xy0, Point2D *P2d, int pflg=FALSE, VOBJ_COORD_MODE cmode=VOBJ_COORD_FROM_MOUSE, const gchar *lab=NULL, double Marker_scale=1.);
	virtual ~VObParabel();

	virtual void Update();

 private:
};

class VObRectangle : public VObject{
 public:
	VObRectangle(GtkWidget *canvas, double *xy0, Point2D *P2d, int pflg=FALSE, VOBJ_COORD_MODE cmode=VOBJ_COORD_FROM_MOUSE, const gchar *lab=NULL, double Marker_scale=1.);
	virtual void SetUpScan();
	virtual ~VObRectangle();
	virtual void Update();

};

class VObCircle : public VObject{
 public:
	VObCircle(GtkWidget *canvas, double *xy0, Point2D *P2d, int pflg=FALSE, VOBJ_COORD_MODE cmode=VOBJ_COORD_FROM_MOUSE, const gchar *lab=NULL, double Marker_scale=1.);
	virtual ~VObCircle();

	double Radius(){
		return Dist();
	}

	virtual void Update();
};

class VObEvent : public VObject{
 public:
	VObEvent(GtkWidget *canvas, double *xy0, Point2D *P2d, int pflg=FALSE, VOBJ_COORD_MODE cmode=VOBJ_COORD_FROM_MOUSE, const gchar *lab=NULL, double marker_scale=0.4);
	virtual ~VObEvent();
	
	virtual void Update();

 private:
};

#endif
