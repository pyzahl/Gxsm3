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

#include <config.h>

#include <time.h>
#include "surface.h"
#include "xsmmasks.h"
#include "glbvars.h"
#include "action_id.h"
#include "xsm.h"

#include "gxsm_monitor_vmemory_and_refcounts.h"

int scandatacount=0;

Scan::Scan(Scan *scanmaster){
        GXSM_LOG_DATAOBJ_ACTION (GXSM_GRC_SCANOBJ, "constructor");
        GXSM_REF_OBJECT (GXSM_GRC_SCANOBJ);
	TimeList = NULL;
	refcount=0;
	Running = 0;
	Pkt2d = NULL;
	numpkt2d = 0;
	objects_id = 0;
	objects_list = NULL;
	realloc_pkt2d(16);
	vdata = scanmaster->vdata;;
	mem2d = new Mem2d(1,1,scanmaster->mem2d->GetTyp()); // MemObj. anlegen
	if (vdata)
		data.CpUnits(*vdata);
	data.s.iyEnd=0;
	data.s.xdir=data.s.ydir=0;
	data.s.pixeltime=0.;
	data.s.tStart=data.s.tEnd=(time_t)0;
	data.s.ntimes=1;
	data.s.nvalues=1;
	view=NULL; // noch klein View
	VFlg  = scanmaster->VFlg;
	ChanNo= scanmaster->ChanNo;
	X_linearize=scanmaster->X_linearize;
	State = IS_FRESH;
}


Scan::Scan(int vtype, int vflg, int ChNo, SCAN_DATA *vd, ZD_TYPE mtyp){
        GXSM_LOG_DATAOBJ_ACTION (GXSM_GRC_SCANOBJ, "constructor");
        GXSM_REF_OBJECT (GXSM_GRC_SCANOBJ);
	TimeList = NULL;
	mem2d_refcount = 0;
	refcount = 0;
	Running  = 0;
	Pkt2d    = NULL;
	numpkt2d = 0;
	objects_id = 0;
	objects_list = NULL;
	realloc_pkt2d(16);
	vdata = vd;
	mem2d = new Mem2d(1,1,mtyp); // MemObj. anlegen
	if(vd)
		data.CpUnits(*vdata);
	data.s.iyEnd=0;
	data.s.xdir=data.s.ydir=0;
	data.s.pixeltime=0.;
	data.s.tStart=data.s.tEnd=(time_t)0;
	data.s.ntimes=1;
	data.s.nvalues=1;
	view=NULL; // noch klein View
	VFlg  = SCAN_V_QUICK;
	ChanNo= ChNo;
	State = IS_FRESH;
	if(vflg)
		VFlg = vflg;
	if(vtype)
		SetView(vtype);
	SetVM(vflg, vdata);
	X_linearize=FALSE;
}

Scan::~Scan(){
	if(State==NOT_SAVED)
		;
  	if(view) delete view; // ggf. alten View löschen
	view=NULL;

	free_time_elements ();

	delete mem2d; // MemObj. löschen

	delete[] Pkt2d; 
	destroy_all_objects ();
        GXSM_UNREF_OBJECT (GXSM_GRC_SCANOBJ);
        GXSM_LOG_DATAOBJ_ACTION (GXSM_GRC_SCANOBJ, "destructor");
}

int Scan::free_time_elements (){
	if (TimeList){
		g_list_foreach (TimeList, (GFunc) Scan::free_time_element, this);
		g_list_free (TimeList);
	}
	TimeList = NULL;
	return 0;
}

int Scan::append_current_to_time_elements (int index, double t, Mem2d* other){
	TimeElementOfScan *tes = new TimeElementOfScan;
	if (!other)
		mem2d->set_frame_time (t);
	tes->index = 0;
	if (index >= 0)
		tes->index = index;
	else
		if (TimeList){
			TimeElementOfScan *tes_last = (TimeElementOfScan*) g_list_last (TimeList);
			index = tes->index = tes_last->index + 1;
		} else {
			index = 0;
			tes->index = 0;
		}

	if (other)
		tes->mem2d = new Mem2d (other, M2D_COPY);
	else
		tes->mem2d = new Mem2d (mem2d, M2D_COPY);
	tes->mem2d->set_frame_time (t);
	tes->mem2d->set_t_index(index); // only here and only!
	tes->refcount = 0;
	tes->sdata = new SCAN_DATA;
	tes->sdata->copy (data);

	TimeList = g_list_append (TimeList, tes);

	data.s.ntimes = g_list_length (TimeList);
	return data.s.ntimes;
}

int Scan::prepend_current_to_time_elements (int index, double t, Mem2d* other){
	TimeElementOfScan *tes = new TimeElementOfScan;
	if (!other)
		mem2d->set_frame_time (t);
	tes->index = 0;
	if (index >= 0)
		tes->index = index;
	else
		if (TimeList){
			TimeElementOfScan *tes_last = (TimeElementOfScan*) g_list_last (TimeList);
			index = tes->index = tes_last->index + 1;
		} else {
			index = 0;
			tes->index = 0;
		}

	if (other)
		tes->mem2d = new Mem2d (other, M2D_COPY);
	else
		tes->mem2d = new Mem2d (mem2d, M2D_COPY);
	tes->mem2d->set_frame_time (t);
	tes->mem2d->set_t_index(index); // set only here and only and by reindex!
	tes->refcount = 0;
	tes->sdata = new SCAN_DATA;
	tes->sdata->copy (data);

	TimeList = g_list_prepend (TimeList, tes);

	data.s.ntimes = g_list_length (TimeList);

	reindex_time_elements ();

	return data.s.ntimes;
}

void Scan::reindex_time_elements (){
	if (! TimeList) return;

	int nte = number_of_time_elements ();
	for (int i=0; i<nte; ++i){
		TimeElementOfScan *tes = (TimeElementOfScan*) g_list_nth_data (TimeList, i);
		tes->index = i;
		tes->mem2d->set_t_index(i); // only here and at app/prepend!
	}
}

// sorting tools
gint compare_time_list_elements_by_index (TimeElementOfScan *a, TimeElementOfScan *b) {
	if (a->mem2d->get_t_index () < b->mem2d->get_t_index ()) return -1;
	if (a->mem2d->get_t_index () > b->mem2d->get_t_index ()) return 1;
        return 0;
}
void Scan::sort_time_elements_by_index (){
 	if (TimeList)
                TimeList = g_list_sort (TimeList,  GCompareFunc (compare_time_list_elements_by_index)); 
}
                        
gint compare_time_list_elements_by_time (TimeElementOfScan *a, TimeElementOfScan *b) {
	if (a->mem2d->get_frame_time () < b->mem2d->get_frame_time ()) return -1;
	if (a->mem2d->get_frame_time () > b->mem2d->get_frame_time ()) return 1;
        return 0;
}
void Scan::sort_time_elements_by_time (){
 	if (TimeList)
                TimeList = g_list_sort (TimeList,  GCompareFunc (compare_time_list_elements_by_time)); 
}
                        
gint compare_time_list_elements_by_bias (TimeElementOfScan *a, TimeElementOfScan *b) {
        // g_message ("Sorting: %s %g <> %s %g", a->sdata->ui.name, a->sdata->s.Bias, b->sdata->ui.name, b->sdata->s.Bias);
	if (a->sdata->s.Bias < b->sdata->s.Bias) return -1;
	if (a->sdata->s.Bias > b->sdata->s.Bias) return 1;
        return 0;
}
void Scan::sort_time_elements_by_bias (){
 	if (TimeList)
                TimeList = g_list_sort (TimeList,  GCompareFunc (compare_time_list_elements_by_bias));
}

gint compare_time_list_elements_by_zsetpoint (TimeElementOfScan *a, TimeElementOfScan *b) {
	if (a->sdata->s.ZSetPoint < b->sdata->s.ZSetPoint) return -1;
	if (a->sdata->s.ZSetPoint > b->sdata->s.ZSetPoint) return 1;
        return 0;
}
void Scan::sort_time_elements_by_zsetpoint (){
 	if (TimeList)
                TimeList = g_list_sort (TimeList,  GCompareFunc (compare_time_list_elements_by_zsetpoint));
}

// list management
int Scan::remove_time_element (int index){
	return 0; // to be completed.
}

int Scan::number_of_time_elements (){
	if (! TimeList)
		return 1; // we always have this !!

	return g_list_length (TimeList);
}

double Scan::retrieve_time_element (int index){
	if (! TimeList) 
		return 0.;

	TimeElementOfScan *tes = (TimeElementOfScan*) g_list_nth_data (TimeList, index);
	if (tes){
		int v=mem2d->data->GetLayer();
//		std::cout << "Scan::retrieve_time_element index=" << index << std::endl;
		mem2d->copy (tes->mem2d);
		mem2d->data->SetLayer(v);
		mem2d->set_frame_time (tes->mem2d->get_frame_time ());
		return tes->mem2d->get_frame_time ();
	}
	return 0.;
}

Mem2d* Scan::mem2d_time_element (int index){
	if (! TimeList) 
		return mem2d;

	TimeElementOfScan *tes = (TimeElementOfScan*) g_list_nth_data (TimeList, index);
	if (tes)
		return tes->mem2d;
	else
		return mem2d;
}


int Scan::get_current_time_element (){
//	g_list_find_custom (TimeList, ...);
//	tes->index = (TimeElementOfScan*)( g_list_last (TimeList)->data)->index + 1;
	return -1;
}


int Scan::add_object (const gchar *name, const gchar *text,
		     int np, void (*f_xyi)(int, double&, double&)){
	scan_object_data *sod = new scan_object_data (++objects_id, name, text, np, f_xyi); 
	objects_list = g_slist_prepend (objects_list, sod);
	return sod->get_id ();
}

void Scan::update_object (int id, 
			  const gchar *name, const gchar *text,
			  void (*f_xyi)(int, double&, double&)){
	((scan_object_data*) g_slist_nth_data(objects_list, find_object (id))) -> update (name, text, f_xyi); 
}

int Scan::find_object (int id){
	for(unsigned int i=0; i< g_slist_length(objects_list); i++)
		if (((scan_object_data*) g_slist_nth_data(objects_list, i)) -> get_id () == id)
			return (int)i;
	return -1;
}

int Scan::del_object (int id){
	for(unsigned int i=0; i< g_slist_length(objects_list); i++){
		scan_object_data* sod = (scan_object_data*) g_slist_nth_data(objects_list, i);
		if (sod->get_id() == id){
			objects_list = g_slist_remove((GSList*) objects_list, sod);
			delete sod;
			return 0;
		}
	}
	return -1;
}



void Scan::realloc_pkt2d(int n){
	if (numpkt2d < n){
		if (Pkt2d)
			delete[] Pkt2d;
		Pkt2d = new Point2D[numpkt2d = n];
		XSM_DEBUG(DBG_L2, "Scan::realloc_pkt2d: n=" << n );
		for (int i=0; i<n; ++i)
			Pkt2d[i].x = Pkt2d[i].y = 0;
		XSM_DEBUG(DBG_L2, "Scan::realloc_pkt2d done." );
	}
}

void Scan::determine_display (int Delta, double sm_eps){
        double hi,lo;
        int success = FALSE;
        int n_obj = number_of_object ();
        Point2D p[2];
        hi=lo=0.;

	if (view)
		view->setup_data_transformation();

        while (n_obj--){
                scan_object_data *obj_data = get_object_data (n_obj);
		
                if (strncmp (obj_data->get_name (), "Rectangle", 9) )
                        continue; // only points are used!

                if (obj_data->get_num_points () != 2) 
                        continue; // sth. is weired!

                double x,y; x=y=0.;
                obj_data->get_xy_pixel (0, x, y);
                p[0].x = (int)x; p[0].y = (int)y;
                obj_data->get_xy_pixel (1, x, y);
                p[1].x = (int)x; p[1].y = (int)y;

                success = TRUE;
                break;
        }

        if (success){
                if(data.display.ViewFlg & SCAN_V_SCALE_SMART)
                        mem2d->AutoHistogrammEvalMode (&Pkt2d[0], &Pkt2d[1], Delta, sm_eps);
                else{
                        if (data.display.ViewFlg & SCAN_V_LOG){
                                mem2d->HiLo (&hi, &lo, FALSE, &Pkt2d[0], &Pkt2d[1], Delta);
                                mem2d->SetHiLo (hi, lo);
                        } else
                                mem2d->HiLoMod (&p[0], &p[1], Delta);
                }
        } else {
                if(data.display.ViewFlg & SCAN_V_SCALE_SMART)
                        mem2d->AutoHistogrammEvalMode (NULL, NULL, Delta, sm_eps);
                else{
                        if (data.display.ViewFlg & SCAN_V_LOG){
                                mem2d->HiLo (&hi, &lo, FALSE, NULL, NULL, Delta);
                                mem2d->SetHiLo (hi, lo);
                        } else
                                mem2d->HiLoMod (NULL, NULL, Delta);
                }
        }

}

void Scan::auto_display (){
        double hi,lo;
        // determine ranges on selection(s)
        determine_display (xsmres.HiLoDelta, (double)xsmres.SmartHistEpsilon);

        // update view parameters automatically from selected range(s)
        mem2d->GetZHiLo (&hi, &lo);
        data.display.vrange_z = (hi-lo)*data.s.dz;
        data.display.voffset_z = 0.;

        // calculate contrast and bright
        mem2d->AutoDataSkl (&data.display.contrast, &data.display.bright);

        draw ();
}

void Scan::set_display (){
        // determine ranges on selection(s)
        determine_display (xsmres.HiLoDelta, (double)xsmres.SmartHistEpsilon);

        // recompute from vrange/offset
        mem2d->SetDataVRangeZ (data.display.vrange_z, 
                               data.display.voffset_z,
                               data.s.dz);

        // calculate contrast and bright
        mem2d->AutoDataSkl (&data.display.contrast, &data.display.bright);

        draw ();
}

#if 0
void Scan::AutoDisplay (doule hi, double lo, int Delta, double sm_eps){


        GXSM_LOG_ANY_ACTION ("AutoDisp_callback", "in");

	if (hi == 0. && lo == 0.){
		SetVM (-2, NULL, Delta, sm_eps);

		// step 2: calculate contrast and bright from Zmin, Zrange
		mem2d->AutoDataSkl (&data.display.contrast, &data.display.bright);
		// store high and low in data
		mem2d->GetZHiLo (&data.display.cpshigh, &data.display.cpslow);
		// calculate Vrange in Units
		double signum = data.display.vrange_z > 0. ? 1.:-1.;
		data.display.vrange_z = signum * (1e-100+fabs (data.s.dz * mem2d->GetZRange ()));
		data.display.voffset_z = 0.;
		// only neede by SPALEED
		data.display.cpshigh /= data.display.cnttime; // correct to CPS now!
		data.display.cpslow  /= data.display.cnttime;

	} else {

		mem2d->SetHiLo (hi, lo);
		mem2d->AutoDataSkl (&data.display.contrast, &data.display.bright);
		mem2d->GetZHiLo (&data.display.cpshigh, &data.display.cpslow);
		data.display.vrange_z = data.s.dz * fabs (mem2d->GetZRange ())
			* (data.display.vrange_z > 0. ? 1.:-1.);
		data.display.voffset_z = 0.;
		data.display.cpshigh /= data.display.cnttime; // correct to CPS now!
		data.display.cpslow  /= data.display.cnttime;
	}

        if (vdata)
                vdata->GetDisplay_Param (data);

        GXSM_LOG_ANY_ACTION ("AutoDisp_callback", "out");
}
#endif


int Scan::SetVM(int vflg, SCAN_DATA *src, int Delta, double sm_eps){
	if (vflg > 0)
		data.display.ViewFlg=vflg;

	if (view)
		view->setup_data_transformation();

	if (src)
		data.GetDisplay_Param (*src);

	if (IS_SPALEED_CTRL || (!strncasecmp(xsmres.InstrumentType, "CCD",3))){
		if (vflg == -2){
			int success = FALSE;
			int n_obj = number_of_object ();
			Point2D p[2];
			while (n_obj--){
				scan_object_data *obj_data = get_object_data (n_obj);
		
				if (strncmp (obj_data->get_name (), "Rectangle", 9) )
					continue; // only points are used!
				
				if (obj_data->get_num_points () != 2) 
					continue; // sth. is weired!

				double x,y; x=y=0.;
				obj_data->get_xy_pixel (0, x, y);
				p[0].x = (int)x; p[0].y = (int)y;
				obj_data->get_xy_pixel (1, x, y);
				p[1].x = (int)x; p[1].y = (int)y;

				success = TRUE;
				break;
			}
			if (success){
				if(data.display.ViewFlg & SCAN_V_SCALE_SMART)
					mem2d->AutoHistogrammEvalMode (&Pkt2d[0], &Pkt2d[1], Delta, sm_eps);
				else{
					if (data.display.ViewFlg & SCAN_V_LOG){
						double hi,lo;
						mem2d->HiLo (&hi, &lo, FALSE, &Pkt2d[0], &Pkt2d[1], Delta);
						mem2d->SetHiLo (hi, lo);
					} else
						mem2d->HiLoMod (&p[0], &p[1], Delta);
				}
			} else {
				if(data.display.ViewFlg & SCAN_V_SCALE_SMART)
					mem2d->AutoHistogrammEvalMode (NULL, NULL, Delta, sm_eps);
				else{
					if (data.display.ViewFlg & SCAN_V_LOG){
						double hi,lo;
						mem2d->HiLo (&hi, &lo, FALSE, NULL, NULL, Delta);
						mem2d->SetHiLo (hi, lo);
					} else
						mem2d->HiLoMod (NULL, NULL, Delta);
				}
			}
			mem2d->AutoDataSkl (&vdata->display.contrast, &vdata->display.bright);
			// store high and low in vdata
			mem2d->GetZHiLo (&vdata->display.cpshigh, &vdata->display.cpslow);
			
			// calculate Vrange in Units
			vdata->display.vrange_z = fabs (vdata->s.dz * mem2d->GetZRange ())
				* (vdata->display.vrange_z > 0. ? 1.:-1.);
			vdata->display.voffset_z = 0.;
			// only neede by SPALEED
			vdata->display.cpshigh /= vdata->display.cnttime; // correct to CPS now!
			vdata->display.cpslow  /= vdata->display.cnttime;
			data.GetDisplay_Param(*vdata);
		}
		mem2d->SetHiLo(data.display.GetCntHigh(), data.display.GetCntLow());
		mem2d->AutoDataSkl(&data.display.contrast, &data.display.bright);
	}else{
		int success = FALSE;
		int n_obj = number_of_object ();
		Point2D p[2];
		while (n_obj--){
			scan_object_data *obj_data = get_object_data (n_obj);
		
			if (strncmp (obj_data->get_name (), "Rectangle", 9) )
				continue; // only points are used!

			if (obj_data->get_num_points () != 2) 
				continue; // sth. is weired!

			double x,y; x=y=0.;
			obj_data->get_xy_pixel (0, x, y);
			p[0].x = (int)x; p[0].y = (int)y;
			obj_data->get_xy_pixel (1, x, y);
			p[1].x = (int)x; p[1].y = (int)y;

			success = TRUE;
			break;
		}
		if (success){
                        if(data.display.ViewFlg & SCAN_V_SCALE_SMART)
				mem2d->AutoHistogrammEvalMode (&Pkt2d[0], &Pkt2d[1], Delta, sm_eps);
			else{
				if (data.display.ViewFlg & SCAN_V_LOG){
					double hi,lo;
					mem2d->HiLo (&hi, &lo, FALSE, &Pkt2d[0], &Pkt2d[1], Delta);
					mem2d->SetHiLo (hi, lo);
				} else
					mem2d->HiLoMod (&p[0], &p[1], Delta);
			}
		} else {
                        if(data.display.ViewFlg & SCAN_V_SCALE_SMART)
				mem2d->AutoHistogrammEvalMode (NULL, NULL, Delta, sm_eps);
			else{
				if (data.display.ViewFlg & SCAN_V_LOG){
					double hi,lo;
					mem2d->HiLo (&hi, &lo, FALSE, NULL, NULL, Delta);
					mem2d->SetHiLo (hi, lo);
				} else
					mem2d->HiLoMod (NULL, NULL, Delta);
			}
		}
		
		if (vflg >= 0) {
			// step 2: adjust to user settings, 
			//         offset is relative to data average
			mem2d->SetDataVRangeZ (data.display.vrange_z, 
					       data.display.voffset_z,
					       data.s.dz);
			// calculate contrast and bright
			mem2d->AutoDataSkl(&data.display.contrast, &data.display.bright);
		}
	}

	if (vflg >= 0)
		draw ();

	return 0;
}

void Scan::hide(){;}

void Scan::Activate(){
	XSM_DEBUG (DBG_L2, "Scan::Activate VFlg=" << VFlg);
	
	GetDataSet(*vdata);
	VFlg = data.display.ViewFlg;
	gapp->spm_update_all(VFlg);
}

int Scan::SetView(int vtype){ 
	if(view) delete view; // ggf. alten View löschen
	view = NULL;
	switch(vtype){
	case ID_CH_V_NO:
		return ID_CH_V_NO;
		break;
	case ID_CH_V_GREY: 
		view = new Grey2D(this, ChanNo);
		SetVM();
		draw();
		return ID_CH_V_GREY;
		break;
	case ID_CH_V_PROFILE:
		view = new Profiles(this, ChanNo);
		draw();
		return ID_CH_V_PROFILE;
		break;
	case ID_CH_V_SURFACE:
		//    return ID_CH_V_NO; // just disable
		view = new Surf3d(this, ChanNo);
		SetVM();
		draw();
		return ID_CH_V_SURFACE;
		break;
	default:    
		return ID_CH_V_NO;
	}

}

int Scan::draw(int y1, int y2){
	if (view){
		mem2d->SetDataSkl (data.display.contrast, data.display.bright);
		//    XSMDEBUGLVL(8,"Scan::draw");
		if(y1>=0) // was >0
			view->update(y1,y2);
		else
			view->draw();
	}
	//  else XSM_DEBUG (DBG_L2, "no view !");
	return 0;
}

int Scan::create(gboolean RoundFlg, gboolean subgrid, gdouble direction, gint fast_scan){
	if(vdata){
		Display_Param disp_tmp;
		// jetzt alle Werte übernehmen
		data.GetScan_Param(*vdata);
		data.GetUser_Info(*vdata);
// **		data.GetDSP_Param(*vdata);
		data.UpdateUnits();
//    XSM_DEBUG (DBG_L2, "Scan:Create, viewdata:" << " c:" << data.display.contrast << " vdata:" << vdata->s.nx << "x" << vdata->s.ny);
		
		if(State == IS_FRESH){ // nur falls neu, sonst beibehalten
			data.GetDisplay_Param(*vdata);
// **			data.GetLayer_Param(*vdata);
		}
	}
	
	data.ui.SetName("noname");
	
	mem2d->Resize(data.s.nx, data.s.ny, data.s.nvalues);

//--OffRotFix--
	// check if non linear sine X scale (fast scan set) 
	if (fast_scan){
		double rx2 = data.s.rx/2.;
		for (int i=0; i<data.s.nx; ++i)
			mem2d->data->SetXLookup(i, -rx2 * direction * cos (M_PI*(double)i / (double)(data.s.nx)) );
	} 
	else
		mem2d->data->MkXLookup (-direction*data.s.rx/2, direction*data.s.rx/2);

	switch(data.orgmode){
	case SCAN_ORG_MIDDLETOP:
		mem2d->data->MkYLookup (0., -data.s.ry); break;
	case SCAN_ORG_CENTER:
		mem2d->data->MkYLookup (data.s.ry/2, -data.s.ry/2); break;
	}
	
	// "Anzeige-Daten" aktualisieren
	if (vdata){
		vdata->GetScan_Param(data);
		vdata->GetUser_Info(data);
	} else {
		data.s.nvalues = 1;
		data.s.ntimes = 1;
	}

	XSM_DEBUG (DBG_L2, "Scan::create done");
	State= IS_NEW;
	Running = 0;
	
	return 0;
}

void Scan::start(int l, double lv){	
	time_t t; // Frame Startzeit
	time(&t);

	inc_refcount ();
	if (l==0){
		data.UpdateUnits ();
		data.s.tStart = time (0);
		data.s.ntimes = 1;
		data.ui.SetOriginalName ("unknown (not saved)");

		if (vdata){
			vdata->ui.SetOriginalName ("unknown (not saved)");
			gapp->ui_update();
		}

		// clean up LayerInformation and auto add basic set
		mem2d->remove_layer_information ();
	}

	mem2d->SetLayer (l);
	mem2d->SetLayerDataPut (l);
	mem2d->data->SetVLookup (l, lv);
        // mem2d->data->ResetLineInfo ();
	mem2d->add_layer_information (new LayerInformation ("Bias", gapp->xsm->data.s.Bias, "%5.3f V"));
	mem2d->add_layer_information (new LayerInformation ("Layer", l, "%03.0f"));
	mem2d->add_layer_information (new LayerInformation ("Name", data.ui.name));
	mem2d->add_layer_information (new LayerInformation (data.ui.dateofscan)); // Date of scan (series) start
	mem2d->add_layer_information (new LayerInformation ("Current", gapp->xsm->data.s.Current, "%5.2f nA"));
	mem2d->add_layer_information (new LayerInformation ("Current", gapp->xsm->data.s.Current*1000, "%5.1f pA"));
	mem2d->add_layer_information (new LayerInformation ("Layer-Param", lv, "%5.3f [V]"));
	mem2d->add_layer_information (new LayerInformation ("Frame-Start",ctime(&t))); // Date for frame start (now)
	mem2d->add_layer_information (new LayerInformation ("X-size original", data.s.rx, "Rx: %5.1f \303\205"));
	mem2d->add_layer_information (new LayerInformation ("Y-size original", data.s.ry, "Ry: %5.1f \303\205"));
	mem2d->add_layer_information (new LayerInformation ("SetPoint", data.s.SetPoint, "%5.2f V"));
	mem2d->add_layer_information (new LayerInformation ("ZSetPoint XXX", data.s.ZSetPoint, "%5.2f  \303\205"));

	Running = 1;
}

void Scan::stop(int StopFlg, int line){
	data.s.tEnd = time (0);
#if 0 // this is saving memory, but causes a lot of trouble elsewhere...
	if (StopFlg && gapp->xsm->hardware->FreeOldData ()){
		mem2d->Resize (data.s.nx, data.s.ny=line);
		data.s.ry = data.s.ny*data.s.dy;
		draw ();
	}
#endif
	data.s.iyEnd=line;
	mem2d->SetLayer (0);
	Running = 0;
	dec_refcount ();
	if (vdata){
		vdata->ui.SetOriginalName ("unknown (not saved)");
		gapp->ui_update(); // hack as long signal "changed" did not work
		data.ui.SetComment (vdata->ui.comment);
	}
}

// get updated copy of user fields comment, ...
void Scan::CpyUserEntries(SCAN_DATA &src){
	data.GetUser_Info (src);
}

// get complete data set
void Scan::CpyDataSet(SCAN_DATA &src){
	data.GetScan_Param (src);
// **	data.GetLayer_Param (src);
	data.GetUser_Info (src);
// **	data.GetDSP_Param (src);
	data.GetDisplay_Param (src);
	data.UpdateUnits ();
}

// put back dataset without dsp-params
void Scan::GetDataSet(SCAN_DATA &dst){
	double x0,y0;
	x0 = dst.s.x0; // keep for restore if running scan
	y0 = dst.s.y0;

	dst.GetScan_Param (data);

	if (Running){ // keep, i.e. restore User-Scan-Offset if scan is running -- NEW 20060915-PZ-fix !!
		dst.s.x0 = x0;
		dst.s.y0 = y0;
	}

	if (!Running) // do not put back if scan is running!!
		dst.GetUser_Info (data);

	dst.GetDisplay_Param (data);
// **	if (IS_SPALEED_CTRL) // need to get Energy and gatettime !
// **		dst.GetDSP_Param (data);
	dst.UpdateUnits ();
// **	dst.GetLayer_Param (data);
	dst.SetZUnit (data.Zunit);

//	std::cout << "Scan::GetDataSet  s.ntimes=" << dst.s.ntimes << std::endl;
//	std::cout << "...               s.values=" << dst.s.nvalues << std::endl;

}

int Scan::Pixel2World (int ix, int iy, double &wx, double &wy, SCAN_COORD_MODE scm){
	switch (scm){
	case SCAN_COORD_ABSOLUTE:
	{
		double s = sin (data.s.alpha*M_PI/180.);
		double c = cos (data.s.alpha*M_PI/180.);
		double rx = mem2d->data->GetXLookup (ix);
		double ry = mem2d->data->GetYLookup (iy);
		// offset + rotation
		wx = data.s.x0 + c*rx + s*ry;
		wy = data.s.y0 - s*rx + c*ry;
	}
	break;
	case SCAN_COORD_RELATIVE:
		wx = mem2d->data->GetXLookup (ix);
		wy = mem2d->data->GetYLookup (iy);
		break;
	}
	return 0;
}

int Scan::Pixel2World (double ix, double iy, double &wx, double &wy, SCAN_COORD_MODE scm){
	double rx  = mem2d->data->GetXLookup ((int)round(ix));
	double rx2 = mem2d->data->GetXLookup ((int)round(ix)+1);
	rx += (rx2-rx)*(ix-round(ix));
	double ry  = mem2d->data->GetYLookup ((int)round(iy));
	double ry2 = mem2d->data->GetYLookup ((int)round(iy)+1);
	ry += (ry2-ry)*(iy-round(iy));
	switch (scm){
	case SCAN_COORD_ABSOLUTE:
	{
		double s = sin (data.s.alpha*M_PI/180.);
		double c = cos (data.s.alpha*M_PI/180.);
		// offset + rotation
		wx = data.s.x0 + c*rx + s*ry;
		wy = data.s.y0 - s*rx + c*ry;
	}
	break;
	case SCAN_COORD_RELATIVE:
		wx = rx;
		wy = ry;
		break;
	}
	return 0;
}

int Scan::World2Pixel (double wx, double wy, int &ix, int &iy, SCAN_COORD_MODE scm){
	double rx=0.,ry=0.;

	World2Pixel (wx, wy, rx, ry, scm);

	ix = (int)round(rx);
	iy = (int)round(ry);

	return 0;
}

int Scan::World2Pixel (double wx, double wy, double &ix, double &iy, SCAN_COORD_MODE scm){
	double rx=0.,ry=0.;

	switch (scm){
	case SCAN_COORD_ABSOLUTE:
	{
		double s = sin (data.s.alpha*M_PI/180.);
		double c = cos (data.s.alpha*M_PI/180.);
		wx -= data.s.x0; // remove offset
		wy -= data.s.y0;
		rx = c*wx - s*wy; // inverse rotation
		ry = s*wx + c*wy;
	}
	break;
	case SCAN_COORD_RELATIVE:
		rx = wx;
		ry = wy;
		break;
	}

	ix = rx / data.s.dx + (double)data.s.nx/2.;
	iy = -ry / data.s.dy + ((data.orgmode == SCAN_ORG_CENTER)? (double)data.s.ny/2. : 0.); // was - (minus) testing if + work now...

	return 0;
}
