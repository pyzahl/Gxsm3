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

#ifndef __SCAN_OBJECT_DATA_H
#define __SCAN_OBJECT_DATA_H

/**
 *  scan_object_ixy_func:
 *
 *  Coordinate access function type.
 */

typedef void (*scan_object_ixy_func)  (int, double&, double&);

/**
 *  scan_object_data:
 *  
 *  @_id: object id.
 *  @_name: object name.
 *  @_text: object text.
 *  @_np: number of points used by this object - can not be changed later.
 *  @f_ixy: Pointer to function #scan_object_ixy_func, which provides access 
 *  to the object coodrinates.
 *
 *  Constructor of class scan_object_data.
 */

class scan_object_data{
 public:
	scan_object_data(int _id,
			 const gchar *_name, 
			 const gchar *_text,
			 int _np, 
			 scan_object_ixy_func f_ixy){
		id = _id;
		np = _np;
		xy = new double[np*2];
		ixy = new double[np*2];
		for (int i=0; i<np; ++i){
			(*f_ixy)(i, xy[2*i], xy[2*i+1]);
			(*f_ixy)(-i-1, ixy[2*i], ixy[2*i+1]);
		}
		name = g_strdup (_name);
		text = g_strdup (_text);
	};
	~scan_object_data(){
		delete [] xy;
		delete [] ixy;
		g_free (name);
		g_free (text);

	};

/**
 *  dump:
 *
 *  Dumps object information to stdout.
 */
	void dump(){
		XSM_DEBUG (DBG_L2, 
			  "Objectid = " << id << std::endl
			  << "Name = " << name << std::endl
			  << "Text = " << text << std::endl
			  << "NumP = " << np );
		for (int i=0; i<np; ++i)
			XSM_DEBUG (DBG_L2, "P[" << i << "] = Ang:("
				  << xy[i*2] << ", " 
				  << xy[i*2+1] << "), Pix:(" 
				  << ixy[i*2] << ", " 
				  << ixy[i*2+1] << ")" 
				);
	};

/**
 *  update:
 *  @_name: new object name.
 *  @_text: new object text.
 *  @f_ixy: coordinate update function.
 *
 *  Updates the object data set.
 */
	void update (const gchar *_name, 
		     const gchar *_text,
		     scan_object_ixy_func f_ixy){
		for (int i=0; i<np; ++i){
			(*f_ixy)(i, xy[2*i], xy[2*i+1]);
			(*f_ixy)(-i-1, ixy[2*i], ixy[2*i+1]);
		}
		g_free (name);
		name = g_strdup (_name);
		g_free (text);
		text = g_strdup (_text); 
	};

/**
 *  get_xy:
 *  @i: index of coordinates to get (in Ang).
 *  @x: variable x to get.
 *  @y: variable y to get.
 *
 *  Gets the coordinates of the @i-th point.
 */

	double distance (scan_object_data *other, int i=0) {
		double dx = other->xy[2*i]  - xy[2*i];
		double dy = other->xy[2*i+1]- xy[2*i+1];
		return sqrt(dx*dx+dy*dy); 
	};

	void get_xy (int i, double &x, double &y) { 
		if (i<np) { x=xy[2*i]; y=xy[2*i+1]; } 
	};
 
	void get_xy_pixel (int i, double &x, double &y) { 
		if (i<np) { x=ixy[2*i]; y=ixy[2*i+1]; } 
	}; 

/**
 *  get_name:
 *
 *  Get the objects name.
 *
 *  Returns: pointer to name, do not modifiy!
 */
	gchar *get_name () { return name; };

/**
 *  get_text:
 *
 *  Get the objects text.
 *
 *  Returns: pointer to text, do not modifiy!
 */
	gchar *get_text () { return text; };

/**
 *  get_num_points:
 *
 *  Get the number of points, used by this object.
 *
 *  Returns: number of points.
 */
	int get_num_points () { return np; };

/**
 *  get_id:
 *
 *  Get the objects id.
 *
 *  Returns: object id.
 */
	int get_id () { return id; };

private:	
	int id;
	gchar *name;
	gchar *text;
	int np;
	double *xy;
	double *ixy;
};

#endif
