/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Gxsm Plugin Name: nndistribution.C
 * ========================================
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

/* Please do not change the Begin/End lines of this comment section!
 * this is a LaTeX style section used for auto generation of the PlugIn Manual 
 * Chapter. Add a complete PlugIn documentation inbetween the Begin/End marks!
 * All "% PlugInXXX" commentary tags are mandatory
 * All "% OptPlugInXXX" tags are optional
 * --------------------------------------------------------------------------------
% BeginPlugInDocuSection
% PlugInDocuCaption: OpenCV Re-Center Feature

% PlugInName: opencvrecenter_stack

% PlugInAuthor: Percy Zahl

% PlugInAuthorEmail: zahl@users.sf.net

% PlugInMenuPath: Math/Statistics/Opencvrecenter_stack

% PlugInDescription
The OpenCV Recenter Stack Feature identifies the most likely position of a
given template feature (hold in a Channel set to Mode-X) in the active
channel and sets the Scan-Offset to the resulting position.
It goes over all time elements and sets the shift xy display setting in manual mode.

% PlugInUsage
Call it from Gxsm Math/Statistics menu.

% OptPlugInSources
The active channel is used as data source. Channel set to X-Mode is used as template.

% OptPlugInObjects

% OptPlugInDest
The computation result of matching threasholds is placed into an new math channel for reference.

%% OptPlugInConfig

%% OptPlugInFiles

%% OptPlugInRefs

%% OptPlugInKnownBugs

%% OptPlugInNotes

% OptPlugInHints

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


#include <math.h>
#include <gtk/gtk.h>
#include "config.h"
#include "gxsm3/plugin.h"
#include "gxsm3/action_id.h"
#include "gxsm3/app_profile.h"
#include "gxsm3/view.h"
#include "gxsm3/app_view.h"

using namespace cv;



static void opencvrecenter_stack_init( void );
static void opencvrecenter_stack_about( void );
static void opencvrecenter_stack_configure( void );
static void opencvrecenter_stack_cleanup( void );
static gboolean opencvrecenter_stack_run( Scan *Src, Scan *ScanRef, Scan *Dest);

GxsmPlugin opencvrecenter_stack_pi = {
  NULL,
  NULL,
  0,
  NULL,
  "Opencvrecenter_stack-M2S-Stat",
  NULL,
  NULL,
  "Percy Zahl",
  "math-statistics-section",
  N_("feature recenter stack"),
  N_("find and recenter feature in image stack (time)"),
  "no more info",
  NULL,
  NULL,
  opencvrecenter_stack_init,
  NULL,
  opencvrecenter_stack_about,
  opencvrecenter_stack_configure,
  NULL,
  NULL,
  NULL,
  opencvrecenter_stack_cleanup
};

GxsmMathTwoSrcPlugin opencvrecenter_stack_m2s_pi = {
  opencvrecenter_stack_run
};

static const char *about_text = N_("Gxsm OpenCV Recenter_Stack Plugin\n\n"
                                   "recenter_stack and mark");
static UnitObj *Events = NULL;

GxsmPlugin *get_gxsm_plugin_info ( void ){ 
  opencvrecenter_stack_pi.description = g_strdup_printf(N_("Gxsm MathTwoArg opencvrecenter_stack plugin %s"), VERSION);
  return &opencvrecenter_stack_pi; 
}

GxsmMathTwoSrcPlugin *get_gxsm_math_two_src_no_same_size_check_plugin_info( void ) { 
  return &opencvrecenter_stack_m2s_pi; 
}

static void opencvrecenter_stack_init(void)
{
  PI_DEBUG (DBG_L2, "Opencvrecenter_stack Plugin Init");
}

static void opencvrecenter_stack_about(void)
{
  const gchar *authors[] = { opencvrecenter_stack_pi.authors, NULL};
  gtk_show_about_dialog (NULL,
			 "program-name",  opencvrecenter_stack_pi.name,
			 "version", VERSION,
			 "license", GTK_LICENSE_GPL_3_0,
			 "comments", about_text,
			 "authors", authors,
			 NULL
			 );
}

static void opencvrecenter_stack_configure(void)
{
	if(opencvrecenter_stack_pi.app)
		opencvrecenter_stack_pi.app->message("Opencvrecenter_stack Plugin Configuration");
}

static void opencvrecenter_stack_cleanup(void)
{
	PI_DEBUG (DBG_L2, "Opencvrecenter_stack Plugin Cleanup");
	if (Events){
		delete Events;
		Events = NULL;
	}
}


void setup_opencv_recenter_stack (const gchar *title, Scan *src, double &threshold, int &object_radius, int &method, int &max_markers, int &i_mg){
	UnitObj *Pixel = new UnitObj("Pix","Pix");
	UnitObj *Unity = new UnitObj(" "," ");

	GtkWidget *dialog = gtk_dialog_new_with_buttons (N_(title),
							 NULL, 
							 (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                                         _("_OK"), GTK_RESPONSE_ACCEPT,
							 NULL); 
	BuildParam bp;
        gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), bp.grid);
        bp.grid_add_label ("Recenter_Stacking Method id's, see for details\n"
			   "http://docs.opencv.org/modules/imgproc/doc/object_detection.html\n"
			   "CV_TM_SQDIFF        =0,\n"
			   "CV_TM_SQDIFF_NORMED =1,\n"
			   "CV_TM_CCORR         =2,\n"
			   "CV_TM_CCORR_NORMED  =3,\n"
			   "CV_TM_CCOEFF        =4,\n"
			   "CV_TM_CCOEFF_NORMED =5 ");
	bp.new_line ();

	bp.grid_add_ec ("Recenter_Stack Threshold (0..1)", Unity, &threshold, 0., 1., ".3f"); bp.new_line ();
	bp.grid_add_ec ("Individual Object Radius", Pixel, &object_radius, 1., src->mem2d->GetNx ()/10., ".0f"); bp.new_line ();
	bp.grid_add_ec ("Method", Unity, &method, 0, 5, ".0f"); bp.new_line ();
	bp.grid_add_ec ("limit for max # markers", Unity, &max_markers, 0, 50000, ".0f"); bp.new_line ();
	bp.grid_add_ec ("marker group", Unity, &i_mg, 0, 5, ".0f"); bp.new_line ();

	gtk_widget_show_all (dialog);
	gtk_dialog_run (GTK_DIALOG(dialog));

	gtk_widget_destroy (dialog);

	delete Pixel;
	delete Unity;	
}


void recenter_stack_mem2d (Mem2d *ref, Mem2d *src,
                     double recenter_stack_threshold,
                     int method,
                     int &jp, int &ip
                     ){
	double high, low;

        // convert Scan->SrcRef data into OpenCV Mat => img2
	ref->HiLo(&high, &low, FALSE, NULL, NULL, 1);

	Mat img2 = Mat (ref->GetNy (), ref->GetNx (), CV_32F);
	for (int i=0; i < ref->GetNy (); ++i){
		for (int j=0; j < ref->GetNx (); ++j){
			double pv = ref->data->Z(j,i);
			img2.at<float>(i,j) = (float)((pv-low)/(high-low));
		}
	}

	// convert Scan->Src data into OpenCV Mat => img1
	src->HiLo(&high, &low, FALSE, NULL, NULL, 1);

	Mat img1 = Mat (src->GetNy (), src->GetNx (), CV_32F);
	for (int i=0; i<src->GetNy (); ++i){
		for (int j=0; j<src->GetNx (); ++j){
			double pv = src->data->Z(j,i);
			img1.at<float>(i,j) = (float)((pv-low)/(high-low));
		}
	}

        // sanity check
	if(img1.empty() || img2.empty()){
		return ;
	}

	// do recenter_stacking via OpenCV library and some data post processing
	Mat img_result, img_recenter_stack, img_tmp, img_t8, img_r8;
	matchTemplate (img1, img2, img_recenter_stack, method);
	abs (img_recenter_stack);
	pow (img_recenter_stack, 3., img_result);
	threshold(img_result, img_tmp, recenter_stack_threshold, 0., THRESH_TOZERO);

	img_tmp.convertTo(img_result, CV_32F, 1., 0.);

#if 0
        // store matching threaholds to result channel as reference
	Dest->mem2d->Resize(img_result.cols, img_result.rows, ZD_FLOAT);
	for (int i=0; i<Dest->mem2d->GetNy () && i < img_result.rows; ++i){
		for (int j=0; j<Dest->mem2d->GetNx () && j < img_result.cols; ++j){
			double pv = img_result.at<float>(i,j);
			Dest->mem2d->PutDataPkt(pv, j,i);
		}
	}
#endif
        
	// now mark features in Src Scan using Marker Objects, avoid duplicated in a radius "r=4 pix (object_radius)"
        // and set scan offset to best match location (center of ref scan)
	gchar *tmp;
	int count=0;
	double peak=0.;
	ip=jp=0;
	for (int i=0; i<img_result.rows; ++i){
		for (int j=0; j<img_result.cols; ++j){
			if (img_result.at<float>(i,j) < recenter_stack_threshold)
				continue;
			if (img_result.at<float>(i,j) > peak){
			        peak = img_result.at<float>(i,j);
				ip=i, jp=j;
			}
			
		}
	}

        g_message("Peak=%g @(%d, %d)", peak, jp, ip);
      
}


static gboolean opencvrecenter_stack_run(Scan *Src, Scan *SrcRef, Scan *Dest)
{
	double recenter_stack_threshold = 0.9; // suggestion recenter_stack quality
	int object_radius = 4; // no more than one mark withing this radius
	int max_markers = 5000; // limit to this number -- in case things go weird
	int method = 5; //CV_TM_CCOEFF_NORMED; // recenter_stacking algorithm, see http://docs.opencv.org/modules/imgproc/doc/object_detection.html
        // ????  /usr/include/opencv4/opencv2/imgproc/types_c.h:    CV_TM_CCOEFF_NORMED =5

	int i_marker_group = 0;
	PI_DEBUG (DBG_L2, "OpenCV recenter_stack");

	const gchar *marker_group[] = { 
		"*Marker:red", "*Marker:green", "*Marker:blue", "*Marker:yellow", "*Marker:cyan", "*Marker:magenta",  
		NULL };

	double x1=-1,y1,x2,y2;
	int n_obj = SrcRef->number_of_object ();

        int interactive = 1;
        if (interactive)
                setup_opencv_recenter_stack ("Setup Feature Recenter_Stacking", Src, recenter_stack_threshold, object_radius, method, max_markers, i_marker_group);


        int tf = Src->number_of_time_elements ();

        for (int time_index=0; time_index <= tf; ++time_index){
                g_message("Processing time element #%d", time_index);
                Src->retrieve_time_element (time_index);

#if 0
                Dest->data.copy (Src->data);
                // make sure nx,ny match to memory object
                Dest->data.s.nx = Src->mem2d->GetNx();
                Dest->data.s.ny = Src->mem2d->GetNy();
#endif

                int jp=0;
                int ip=0;
                recenter_stack_mem2d (SrcRef->mem2d, Src->mem2d_time_element (time_index), recenter_stack_threshold, method, jp, ip);
                
                if (Src->view){
                        if (Src->view->Get_ViewControl ()){
                                ViewControl *vc = Src->view->Get_ViewControl ();
                                Src->data.display.px_shift_xy[0] = jp - SrcRef->mem2d->GetNx ()/2; // - Src->mem2d->GetNx()/2;
                                Src->data.display.px_shift_xy[1] = ip - SrcRef->mem2d->GetNy ()/2; // - Src->mem2d->GetNy()/2;
                                Src->data.display.px_shift_xy[2] = -1; // Manual
                                vc->update_view_panel ();
                                Src->set_display_shift ();
                        }
                }
#if 1
                // center to ref box (not origin)
                jp += SrcRef->mem2d->GetNx ()/2;
                ip += SrcRef->mem2d->GetNy ()/2;
                if (Src->view){
                        if (Src->view->Get_ViewControl ()){
                                VObject *vo;
                                double xy[2];
                                gfloat c[4] = { 1.,0.,0.,1.};
                                int spc[2][2] = {{0,0},{0,0}};
                                int sp00[2] = {1,1};
                                int s = 0;
                                Src->Pixel2World (jp, ip, xy[0], xy[1]);
                                gchar *lab = g_strdup_printf ("Center");
                                (Src->view->Get_ViewControl ())->AddObject (vo = new VObPoint ((Src->view->Get_ViewControl ())->canvas, xy, FALSE, VOBJ_COORD_ABSOLUT, lab, 1.));
                                vo->set_obj_name (marker_group[i_marker_group]);
                                vo->set_custom_label_font ("Sans Bold 12");
                                vo->set_custom_label_color (c);
                                vo->set_on_spacetime  (sp00[0] ? FALSE:TRUE, spc[0]);
                                vo->set_off_spacetime (sp00[1] ? FALSE:TRUE, spc[1]);
                                vo->show_label (s);
                                vo->remake_node_markers ();

                                gapp->xsm->data.s.x0 = xy[0];
                                gapp->xsm->data.s.y0 = xy[1];
                                gapp->spm_offset_check (NULL, gapp);
                        }
                }
#endif
        }

	return MATH_OK;
}
