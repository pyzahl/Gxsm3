
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

#ifndef __SCAN_H
#define __SCAN_H

#ifndef __MELDUNGEN_H
#include "meldungen.h"
#endif

#ifndef __XSMTYPES_H
#include "xsmtypes.h"
#endif

#ifndef __XSMHARD_H
#include "xsmhard.h"
#endif

#ifndef __VIEW_H
#include "view.h"
#endif

#ifndef __MEM2D_H
#include "mem2d.h"
#endif

#ifndef __SCAN_OBJECT_DATA_H
#include "scan_object_data.h"
#endif

/*
 * XSM Surface Scan Basis-Objekt:
 * vereint alle Scan-Methoden
 * ============================================================
 */

#define NOT_SAVED 0
#define IS_SAVED  1
#define IS_NEW    2
#define IS_FRESH  3

// this is the type of the current active object...
// we need to get rid of this...
typedef enum { 
        MNONE,
        MPOINT,
        MLINE,
        MRECTANGLE,
        MPOLYLINE,
        MPARABEL,
        MCIRCLE,
        MTRACE,
        MEVENT,
        MKSYS
} OBJECT_TYPE;

// Scan Coordinate System Modes
typedef enum {
	SCAN_COORD_ABSOLUTE, // absolute world coordinates, including offset and rotation
	SCAN_COORD_RELATIVE  // relative world coordinates, no offset, no rotation (in local scan coords)
} SCAN_COORD_MODE;
 
typedef struct {
	int       index;
	double    time;
	int       refcount;
	Mem2d     *mem2d;
	SCAN_DATA *sdata;
} TimeElementOfScan;

class Scan{
public:
	Scan(Scan *scanmaster);
	Scan(int vtype=0, int vflg=0, int ChNo=-1, SCAN_DATA *vd=NULL, ZD_TYPE mtyp=ZD_SHORT);
	virtual ~Scan();
	
	virtual void hide();
	virtual int draw(int y1=-1, int y2=-2);
	virtual int create(gboolean RoundFlg=FALSE, gboolean subgrid=FALSE, gdouble direction=1., gint fast_scan=0);
	void Saved(){ State = IS_SAVED; };
	
	virtual void start(int l=0, double lv=0.);
	virtual void stop(int StopFlg=FALSE, int line=0);
	
	void inc_refcount() { ++refcount; };
	void dec_refcount() { --refcount; };
	int get_refcount() { return refcount; };
	
	void CpyUserEntries(SCAN_DATA &src);
	void CpyDataSet(SCAN_DATA &src);
	void GetDataSet(SCAN_DATA &dst);
	
	int SetView(int vtype=0);
	void AutoDisplay(double hi=0., double lo=0., int Delta=4);
	int SetVM(int vflg=0, SCAN_DATA *src=NULL, int Delta=4);
	int GetVM(){ return VFlg; };
	void Activate();

	void realloc_pkt2d(int n);
	
	/* Data */
	GList *TimeList; /* List multiple of Scan-Data elements (always a copy) */
	Mem2d *mem2d;    /* 2d Daten */
	int   mem2d_refcount;
	SCAN_DATA data;  /* Daten des letzten Scans - Scanbezogen*/

	SCAN_DATA *vdata; /* ever valid Pointer to XSM-(User)-Data (may be manipulated) */

	View  *view;     /* View Objekt */
	
	Point2D Pkt2dScanLine[2];
	int     RedLineActive;
	int     BlueLineActive;
	
	/* Time dimension handling */
	int free_time_elements (); /* free all time elements -- keeps the current, destroys all others! */
	int append_current_to_time_elements (int index=-1, double t=0., Mem2d* other=NULL); /* append current (or other if given) dataset to time elemets */
	int prepend_current_to_time_elements (int index=-1, double t=0., Mem2d* other=NULL); /* prepend current (or other if given) dataset to time elemets */
	int remove_time_element (int index); /* time elemet */
	int number_of_time_elements (); /* find and return number of time elements in list */
	void reindex_time_elements (); /* reindex time elements in list */
	double retrieve_time_element (int index); /* retrieve time element "index" and revert to it, returns time */
	Mem2d* mem2d_time_element (int index); /* retrieve mem2d ptr from time element "index" */
	int get_current_time_element (); /* find index of current time element, -1 if not in list */
	static void free_time_element (TimeElementOfScan *tes, Scan *s){ delete tes->mem2d; delete tes; };

	/* Scan Coodinate System Handling */
	int Pixel2World (int ix, int iy, double &wx, double &wy, SCAN_COORD_MODE scm = SCAN_COORD_ABSOLUTE);
	int Pixel2World (double ix, double iy, double &wx, double &wy, SCAN_COORD_MODE scm = SCAN_COORD_ABSOLUTE);
	int World2Pixel (double wx, double wy, int &ix, int &iy, SCAN_COORD_MODE scm = SCAN_COORD_ABSOLUTE);
	int World2Pixel (double wx, double wy, double &ix, double &iy, SCAN_COORD_MODE scm = SCAN_COORD_ABSOLUTE);
	/* Conveniance wrappers */
	double GetWorldX (int ix){
		double x,y;
		Pixel2World (ix,0, x, y, SCAN_COORD_RELATIVE);
		return x;
	};
	double GetWorldY (int ix){
		double x,y;
		Pixel2World (ix,0, x, y, SCAN_COORD_RELATIVE);
		return y;
	};

	Point2D *Pkt2d;
	int     PktVal;
	
	int add_object (const gchar *name, const gchar *text,
		       int np, void (*f_ixy)(int, double &, double &));
	void update_object (int id, const gchar *name, const gchar *text,
			    void (*f_ixy)(int, double &, double &));
	int del_object (int id);
	int find_object (int id);
	void destroy_all_objects () { for (int i=0; i<=objects_id; ++i) del_object (i); };

	unsigned int number_of_object () { return g_slist_length(objects_list); };
	scan_object_data* get_object_data (int i) { return (scan_object_data*) g_slist_nth_data(objects_list, i); };
	// do not modify data!!!!!!
	void dump_object_data (int i) { ((scan_object_data*) g_slist_nth_data(objects_list, i)) -> dump (); };
	int get_channel_id () { return ChanNo; };

	void x_linearize (int f) { X_linearize=f; };
	int x_linearize () { return X_linearize; };

 private: 
	int VFlg;
	int ChanNo;
	int State;
	int Running;
	int numpkt2d;
	int refcount;
	int X_linearize;

	int     objects_id;
	GSList  *objects_list;
};

/*
 * TopoGraphic Ableitung
 * ============================================================
 */

class TopoGraphicScan : public Scan{
public:
	TopoGraphicScan(int vtype=0, int vflg=0, int ChNo=-1, SCAN_DATA *vd=0);
	virtual ~TopoGraphicScan();
  
private:  

};


/*
 * SpaScan Ableitung
 * ============================================================
 */

class SpaScan : public Scan{
public:
	SpaScan(int vtype=0, int vflg=0, int ChNo=-1, SCAN_DATA *vd=0);
	virtual ~SpaScan();
  
private:  

};


#endif