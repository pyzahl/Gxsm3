/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Plugin Name: quicktime_im_export.C
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
 * --------------------------------------------------------------------------------
% BeginPlugInDocuSection
% PlugInDocuCaption: Quicktime
% PlugInName: quicktime_im_Export
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: File/Import/Quicktime

% PlugInDescription
 The \GxsmEmph{quicktime\_im\_export} plug-in allows exporting of single
 and multidimensional image data sets as Quicktime Movie.

% PlugInUsage
 The plug-in is called by \GxsmMenu{File/Import/Quicktime}.

\GxsmNote{Recommended Format: MJPEG-A, works fine with Power-Point and most Movie Players.}

\GxsmScreenShot{quicktime_export_file}{QT Export File Dialog}
\GxsmScreenShotDual{quicktime_export_setup1}{QT Export Setup step one.}{quicktime_export_setup2}{QT Export Setup step two.}

%% OptPlugInKnownBugs
There seam to be much more formats available from the lib-quicktime, 
but for an unknown reason some are just not working or crashing the program if used.
This seams to depend on the system and libquicktime version used, so please try it out.

%% OptPlugInRefs


% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */

#include <gtk/gtk.h>
#include "config.h"
#include "gxsm/plugin.h"
#include "gxsm/dataio.h"
#include "gxsm/action_id.h"
#include "gxsm/util.h"
#include "gxsm/xsmtypes.h"
#include "gxsm/glbvars.h"
#include "gxsm/gapp_service.h"

// custom includes go here
#include "lqt/quicktime.h"
#include "lqt/colormodels.h"
#include "lqt/lqt.h"
#include "lqt/lqt_codecinfo.h"

// enable std namespace
using namespace std;

// Plugin Prototypes
static void quicktime_im_export_init (void);
static void quicktime_im_export_query (void);
static void quicktime_im_export_about (void);
static void quicktime_im_export_configure (void);
static void quicktime_im_export_cleanup (void);

static void quicktime_im_export_filecheck_load_callback (gpointer data );
static void quicktime_im_export_filecheck_save_callback (gpointer data );

static void quicktime_im_export_import_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data);
static void quicktime_im_export_export_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data);

// Fill in the GxsmPlugin Description here
GxsmPlugin quicktime_im_export_pi = {
	NULL,                   // filled in and used by Gxsm, don't touch !
	NULL,                   // filled in and used by Gxsm, don't touch !
	0,                      // filled in and used by Gxsm, don't touch !
	NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
	// filled in here by Gxsm on Plugin load, 
	// just after init() is called !!!
// -- START EDIT --
	"QUICKTIME-ImExport",            // PlugIn name
	NULL,                   // PlugIn's Categorie, set to NULL for all, I just don't want this always to be loaded!
	// Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
	NULL,
	"Percy Zahl",
	"file-import-section,file-export-section", // sep. im/export menuentry path by comma!
	N_("Quicktime,Quicktime"), // menu entry (same for both)
	N_("Quicktime import,Quicktime export"), // short help for menu entry
	N_("Quicktime im/export filter."), // info
// -- END EDIT --
	NULL,          // error msg, plugin may put error status msg here later
	NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
	quicktime_im_export_init,
	quicktime_im_export_query,
	// about-function, can be "NULL"
	// can be called by "Plugin Details"
	quicktime_im_export_about,
	// configure-function, can be "NULL"
	// can be called by "Plugin Details"
	quicktime_im_export_configure,
	// run-function, can be "NULL", if non-Zero and no query defined, 
	// it is called on menupath->"plugin"
	NULL,
	// cleanup-function, can be "NULL"
	// called if present at plugin removal
        quicktime_im_export_import_callback, // direct menu entry callback1 or NULL
        quicktime_im_export_export_callback, // direct menu entry callback2 or NULL

	quicktime_im_export_cleanup
};

// Text used in Aboutbox, please update!!
static const char *about_text = N_("This GXSM plugin im-/exports from/to QUICKTIME");

static const char *file_mask = "*.mov";

int video_dimension=0;

int offset_index_value=0;
int step_index_value=1;
int max_index_value=1;
double start_value=0.;
double step_value=1.;

int offset_index_time=0;
int step_index_time=1;
int max_index_time=1;
double start_time=0.;
double step_time=1.;

double frame_rate=15.;

int qt_quality = 85;
int qt_codec = 0;
gchar *qt_video_codec = NULL;

gboolean OSD_grab_mode=FALSE;
gboolean conti_autodisp_mode=FALSE;

#define FILE_DIM_V   0x01
#define FILE_DIM_T   0x02
#define FILE_ASKDIM  0x04
#define FILE_CODEC   0x08
#define FILE_DECODE  0x10
#define FILE_IMPORT  0x20
int file_dim=0;

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
	quicktime_im_export_pi.description = g_strdup_printf(N_("Gxsm im_export plugin %s"), VERSION);
	return &quicktime_im_export_pi; 
}

// Query Function, installs Plugin's in File/Import and Export Menupaths!
// ----------------------------------------------------------------------
// Import Menupath is "File/Import/PNG"
// Export Menupath is "File/Export/PNGt"
// ----------------------------------------------------------------------

static void quicktime_im_export_query(void)
{

	if(quicktime_im_export_pi.status) g_free(quicktime_im_export_pi.status); 
	quicktime_im_export_pi.status = g_strconcat (
		N_("Plugin query has attached "),
		quicktime_im_export_pi.name, 
		N_(": File IO Filters are ready to use."),
		NULL);
	

	// register this plugins filecheck functions with Gxsm now!
	// This allows Gxsm to check files from DnD, open, 
	// and cmdline sources against all known formats automatically - no explicit im/export is necessary.
	quicktime_im_export_pi.app->ConnectPluginToLoadFileEvent (quicktime_im_export_filecheck_load_callback);
	quicktime_im_export_pi.app->ConnectPluginToSaveFileEvent (quicktime_im_export_filecheck_save_callback);
}


// 5.) Start here with the plugins code, vars def., etc.... here.
// ----------------------------------------------------------------------
//


// init-Function
static void quicktime_im_export_init(void)
{
	PI_DEBUG (DBG_L2, quicktime_im_export_pi.name << " Plugin Init");
}

// about-Function
static void quicktime_im_export_about(void)
{
	const gchar *authors[] = { quicktime_im_export_pi.authors, NULL};
	gtk_show_about_dialog (NULL, 
			       "program-name",  quicktime_im_export_pi.name,
			       "version", VERSION,
			       "license", GTK_LICENSE_GPL_3_0,
			       "comments", about_text,
			       "authors", authors,
			       NULL
			       );
}

// configure-Function
static void quicktime_im_export_configure(void)
{
	if(quicktime_im_export_pi.app){
		XsmRescourceManager xrm("QUICKTIME_IM_EXPORT");
		GtkWidget *dim_sel = NULL;
		GtkWidget *codec_sel = NULL;
		GtkWidget *OSD_grab_flg = NULL;
		GtkWidget *cont_autodisp_flg = NULL;
		lqt_codec_info_t **ci_list = NULL;

//		xrm.Get ("file_offset_index_value", &offset_index_value, "0");
		xrm.Get ("file_step_index_value", &step_index_value, "1");
		xrm.Get ("file_start_value", &start_value, "0");
		xrm.Get ("file_step_value", &step_value, "1");

//		xrm.Get ("file_offset_index_time", &offset_index_time, "0");
		xrm.Get ("file_step_index_time", &step_index_time, "1");
		xrm.Get ("file_start_time", &start_time, "0");
		xrm.Get ("file_step_time", &step_time, "1");

		xrm.Get ("file_frame_rate", &frame_rate, "15.");
		xrm.Get ("file_qt_codec", &qt_codec, "0");
		xrm.Get ("file_qt_quality", &qt_quality, "85");

		xrm.Get ("file_video_dimension", &video_dimension, "0");


		GtkDialogFlags flags =  (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);
		GtkWidget *dialog = gtk_dialog_new_with_buttons (N_("Quicktime Video Export"),
								 GTK_WINDOW (gapp->get_app_window ()),
								 flags,
								 _("_OK"),
								 GTK_RESPONSE_ACCEPT,
								 _("_Cancel"),
								 GTK_RESPONSE_REJECT,
								 NULL);
		BuildParam bp;
		gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), bp.grid);


                ////////
                
		if (file_dim & FILE_ASKDIM){
			dim_sel = gtk_combo_box_text_new ();
			gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (dim_sel),"video-time", "Video Time Dimension: Time");
			gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (dim_sel),"video-layer", "Video Time Dimension: Layer (Value)");
			gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (dim_sel),"video-layer-time", "Video Time Dimension: Layer and Time");
			gtk_combo_box_set_active_id (GTK_COMBO_BOX (dim_sel), "video-time");
                        bp.grid_add_widget (dim_sel); bp.new_line ();
		}

		if (file_dim & FILE_CODEC){

			codec_sel = gtk_combo_box_text_new ();

			// query list of codecs for AUDIO=0, VIDEO=1, ENCODE, DECODE
			ci_list = lqt_query_registry(0, 1, (file_dim & FILE_DECODE) ? 0:1, (file_dim & FILE_DECODE) ? 1:0);
			for (lqt_codec_info_t **ci=ci_list; *ci; ++ci){
				gchar *info=NULL;
				cout << "LQT_VIDEO_CODEC [" << (*ci)->name << "]: " << (*ci)->long_name << endl;
				cout << "Description: " << (*ci)->description << endl;
				info = g_strdup_printf ("%s [%s]", (*ci)->long_name, (*ci)->name);
				gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (codec_sel), info, info);
				g_free (info);
			}
			gtk_combo_box_set_active (GTK_COMBO_BOX (codec_sel), qt_codec);
                        bp.grid_add_widget (codec_sel); bp.new_line ();

			if (file_dim & FILE_DECODE){
				bp.grid_add_ec ("Quality [%]", quicktime_im_export_pi.app->xsm->Unity, &qt_quality, 0., 100., ".0f"); bp.new_line ();
				bp.grid_add_ec ("Frame Rate [1/s]", quicktime_im_export_pi.app->xsm->Unity, &frame_rate, 1e-6, 100., ".3f"); bp.new_line ();
			}
		}
		if (file_dim & FILE_DIM_V){
                        bp.grid_add_ec ("Max Index Values", quicktime_im_export_pi.app->xsm->Unity, &max_index_value, 1., 1e6, ".0f"); bp.new_line ();
			bp.grid_add_ec ("Index Offset", quicktime_im_export_pi.app->xsm->Unity, &offset_index_value, -1e6, 1e6, ".0f"); bp.new_line ();
			bp.grid_add_ec ("Index Step", quicktime_im_export_pi.app->xsm->Unity, &step_index_value, -1000, 1000, ".0f"); bp.new_line ();
			if (file_dim & FILE_IMPORT){
                                bp.grid_add_ec ("Start Value", quicktime_im_export_pi.app->xsm->Unity, &start_value, -1e6, 1e6, ".3f"); bp.new_line ();
                                bp.grid_add_ec ("Step Value", quicktime_im_export_pi.app->xsm->Unity, &step_value, -1e6, 1e6, ".3f"); bp.new_line ();
			}
		}		

		if (file_dim & FILE_DIM_T){
			bp.grid_add_ec ("Max Index Times", quicktime_im_export_pi.app->xsm->Unity, &max_index_time, 1., 1e6, ".0f"); bp.new_line ();
			bp.grid_add_ec ("Index Offset", quicktime_im_export_pi.app->xsm->Unity, &offset_index_time, -1e6, 1e6, ".0f"); bp.new_line ();
			bp.grid_add_ec ("Index Step", quicktime_im_export_pi.app->xsm->Unity, &step_index_time, -1000, 1000, ".0f"); bp.new_line ();
			if (file_dim & FILE_IMPORT){
                                bp.grid_add_ec ("Start Time", quicktime_im_export_pi.app->xsm->Unity, &start_time, -1e6, 1e6, ".3f"); bp.new_line ();
                                bp.grid_add_ec ("Step Time", quicktime_im_export_pi.app->xsm->Unity, &step_time, -1e6, 1e6, ".3f"); bp.new_line ();
                        }
		}		
                //	bp.grid_add_ec ("Time Origin", quicktime_im_export_pi.app->xsm->TimeUnit, &realtime0_user, 0., 1e20, ".1f");

	        cont_autodisp_flg = gtk_check_button_new_with_label(N_("Contineous AutoDisp"));
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (cont_autodisp_flg), conti_autodisp_mode);
		bp.grid_add_widget (cont_autodisp_flg); bp.new_line ();

	        OSD_grab_flg = gtk_check_button_new_with_label(N_("Scan/OSD life grab mode"));
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON (OSD_grab_flg), OSD_grab_mode);
		bp.grid_add_widget (OSD_grab_flg); bp.new_line ();

		bp.grid_add_label ("Note: Keep the Scan/OSD window on top!");

		gtk_widget_show_all (dialog);
		gtk_dialog_run (GTK_DIALOG(dialog));

		if (OSD_grab_flg)
		        OSD_grab_mode = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (OSD_grab_flg));
		if (cont_autodisp_flg)
		  conti_autodisp_mode = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cont_autodisp_flg));

		if (dim_sel)
			video_dimension = gtk_combo_box_get_active (GTK_COMBO_BOX (dim_sel));

		if (codec_sel){
			qt_codec = gtk_combo_box_get_active (GTK_COMBO_BOX (codec_sel));
			if (ci_list){
				if (qt_video_codec)
					g_free (qt_video_codec);
				qt_video_codec = g_strdup ((ci_list[qt_codec])->name);
				lqt_destroy_codec_info (ci_list);
				cout << "Codec Selected: [" << qt_video_codec << "]" << endl;
			}
		}

		gtk_widget_destroy (dialog);


		xrm.Put ("file_max_index_value", max_index_value);
		xrm.Put ("file_offset_index_value", offset_index_value);
		xrm.Put ("file_step_index_value", step_index_value);
		xrm.Put ("file_start_value", start_value);
		xrm.Put ("file_step_value", step_value);

		xrm.Put ("file_max_index_time", max_index_time);
		xrm.Put ("file_offset_index_time", offset_index_time);
		xrm.Put ("file_step_index_time", step_index_time);
		xrm.Put ("file_start_time", start_time);
		xrm.Put ("file_step_time", step_time);

		xrm.Put ("file_frame_rate", frame_rate);

		xrm.Put ("file_video_dimension", video_dimension);
		xrm.Put ("file_qt_codec", qt_codec);
		xrm.Put ("file_qt_quality", qt_quality);

	}
}

// cleanup-Function, remove all "custom" menu entrys here!
static void quicktime_im_export_cleanup(void)
{
#if 0
	gchar **path  = g_strsplit (quicktime_im_export_pi.menupath, ",", 2);
	gchar **entry = g_strsplit (quicktime_im_export_pi.menuentry, ",", 2);

	gchar *tmp = g_strconcat (path[0], entry[0], NULL);
	gnome_app_remove_menus (GNOME_APP (quicktime_im_export_pi.app->getApp()), tmp, 1);
	g_free (tmp);

	tmp = g_strconcat (path[1], entry[1], NULL);
	gnome_app_remove_menus (GNOME_APP (quicktime_im_export_pi.app->getApp()), tmp, 1);
	g_free (tmp);

	g_strfreev (path);
	g_strfreev (entry);

#endif
	PI_DEBUG (DBG_L2, "Plugin Cleanup done.");
}


// make a new derivate of the base class "Dataio"
class Quicktime_ImExportFile : public Dataio{
public:
	Quicktime_ImExportFile(Scan *s, const char *n); 
	virtual ~Quicktime_ImExportFile();
	virtual FIO_STATUS Read(gboolean append_in_time=FALSE);
	virtual FIO_STATUS Write();
private:
};

Quicktime_ImExportFile::Quicktime_ImExportFile(Scan *s, const char *n) : Dataio(s,n){
}

Quicktime_ImExportFile::~Quicktime_ImExportFile(){
}

FIO_STATUS Quicktime_ImExportFile::Read(gboolean append_in_time){
	FIO_STATUS ret;
	gchar *fname=NULL;

	PI_DEBUG (DBG_L2, "reading");

	fname = (gchar*)name;

	// name should have at least 4 chars: ".ext"
	if (fname == NULL || strlen(fname) < 4)
		return status=FIO_NOT_RESPONSIBLE_FOR_THAT_FILE;

	if (strncasecmp (fname+strlen(fname)-4,".mov", 4))
		return status=FIO_NOT_RESPONSIBLE_FOR_THAT_FILE;

	file_dim=0;
//	file_dim |= FILE_DIM_V;
	file_dim |= FILE_DIM_T;

	int index_time=0;
	ret=FIO_OK;
	
	quicktime_im_export_configure ();

	gapp->progress_info_new ("QT Import", 2);
	gapp->progress_info_set_bar_fraction (0., 1);
	gapp->progress_info_set_bar_fraction (0., 2);
	gapp->progress_info_set_bar_text (fname, 1);
	do {
		int index_value=0;
		do {
			gapp->progress_info_set_bar_fraction ((gdouble)index_time/(gdouble)max_index_time, 1);
			gapp->progress_info_set_bar_fraction ((gdouble)index_value/(gdouble)max_index_value, 2);
//			gapp->progress_info_set_bar_text (fname_expand, 2);
			++index_value;
			
		} while (ret == FIO_OK && index_value < max_index_value);
		
		scan->append_current_to_time_elements (index_time, start_time + step_time*(index_time));
		++index_time;
		
	} while (ret == FIO_OK && index_time < max_index_time);
	gapp->progress_info_close ();
	scan->retrieve_time_element (0);

	scan->SetVM(SCAN_V_DIRECT);
	return ret;
//	return status=FIO_NOT_RESPONSIBLE_FOR_THAT_FILE;
}

FIO_STATUS Quicktime_ImExportFile::Write(){
	quicktime_t *qtfile;
	gint nx,ny;
	GtkWidget *wid_canvas = NULL;
	GdkPixbuf* OSD_pixbuf = NULL;
	gint w,h;
	unsigned char palette_rgb[1024][3];
	int maxcol=256;


	if (name == NULL) return FIO_NO_NAME;

	nx = scan->mem2d->GetNx ();
	ny = scan->mem2d->GetNy ();

	file_dim=0;
	file_dim |= FILE_ASKDIM;

	{	
		XsmRescourceManager xrm("QUICKTIME_IM_EXPORT");
		xrm.Get ("file_video_dimension", &video_dimension, "0");
	}
	quicktime_im_export_configure ();

	offset_index_time  = scan->mem2d->get_t_index ();
	offset_index_value = scan->mem2d->GetLayer ();

	file_dim=0;

	switch (video_dimension){
	case 0: // time
		file_dim |= FILE_DIM_T;
		max_index_time  = scan->data.s.ntimes-1;
		max_index_value = offset_index_value;
		break;
	case 1: // layer
		file_dim |= FILE_DIM_V;
		max_index_time  = offset_index_time;
		max_index_value = scan->mem2d->GetNv ()-1;
		break;
	default: // both
		file_dim |= FILE_DIM_V;
		file_dim |= FILE_DIM_T;
		offset_index_time  = 0;
		offset_index_value = 0;
		max_index_time  = scan->data.s.ntimes-1;
		max_index_value = scan->mem2d->GetNv ()-1;
		break;
	}

	file_dim |= FILE_CODEC;
	file_dim |= FILE_DECODE;
	quicktime_im_export_configure ();

// check if scan has an valid view we can grab from, else fallback to plain export

	if (OSD_grab_mode && (!scan->view->grab_OSD_canvas()))
		OSD_grab_mode = FALSE; 
	else
		wid_canvas = (GtkWidget*)(scan->view->grab_OSD_canvas());

	if (wid_canvas && OSD_grab_mode){
                w = gdk_window_get_width (GDK_WINDOW (wid_canvas));
                h = gdk_window_get_height (GDK_WINDOW (wid_canvas));
		cout << "OSD canvas widget ready to grab from OK. Size: " << w <<", "<< h << endl;
		nx = w; ny = h; // now size as big as the canvas widget is
	} else	if (OSD_grab_mode){
		OSD_grab_mode = FALSE; 
		cout << "OSD canvas widget not accessible (NULL) -- using fall back mode to non OSD" << endl;
	}

// and open the qtfile in write mode:

	if ((qtfile = quicktime_open(name, 0, 1)) == 0)
		return FIO_NO_NAME;

// Immediately after opening the file, set up some tracks to write with these commands:

	cout << "quicktime_set_video [" << qt_video_codec << "]" << endl;
	quicktime_set_video(qtfile, 1, nx, ny, frame_rate, qt_video_codec);

// To set a jpeg compression quality of 80, for example, do the following:

	cout << "quicktime_set_parameter quality[" << qt_quality << "]" << endl;
	gchar *jq = g_strdup ("jpeg_quality");
	quicktime_set_parameter(qtfile, jq, &qt_quality);
	g_free (jq);

// setup and check Encoding video is supported

	cout << "check for video support... " << endl;
	if ( quicktime_supported_video(qtfile, 0) == 0 ){
		quicktime_close (qtfile);
		cout << "Codec not supported" << endl;
		return  FIO_NO_NAME; // Codec not supported
	}
	cout << "OK." << endl;

	cout << "check for color model RGB888 support... " << endl;
	if ( quicktime_writes_cmodel(qtfile, BC_RGB888, 0) == 0){
		quicktime_close (qtfile);
		cout << "Color Model RGB888 not supported" << endl;
		return  FIO_NO_NAME; // Color model not supported
	}
	cout << "OK." << endl;

	cout << "Setting color model." << endl;
	quicktime_set_cmodel(qtfile, BC_RGB888);


	if (!OSD_grab_mode){
		int cval;
		ifstream cpal;
		char pline[256];
		int r,g,b, pnx, pny;
		double val;
		int ival;
	  
		gapp->progress_info_new ("QT Export", 2);
		gapp->progress_info_set_bar_text ("Time", 1);
		gapp->progress_info_set_bar_text ("Layer", 2);

	  
		cout << "Setting up custom palette." << endl;

		// make default 256 grey level palette
		for (cval=0; cval<maxcol; ++cval)
			palette_rgb[cval][2] = palette_rgb[cval][1] = palette_rgb[cval][0] = cval * 255 / maxcol; 
	  
		// if view palette active, check and replace by it
		if (gapp->xsm->ZoomFlg & VIEW_PALETTE){
			cpal.open(xsmres.Palette, ios::in);
			if(cpal.good()){
				cpal.getline(pline, 255);
				cpal.getline(pline, 255);
				cpal >> pnx >> pny;
				cpal.getline(pline, 255);
				cpal.getline(pline, 255);
				
				for(maxcol=min(pnx, 1024), cval=0; cval<maxcol; ++cval){
					cpal >> r >> g >> b ;
					palette_rgb[cval][0] = r;
					palette_rgb[cval][1] = g;
					palette_rgb[cval][2] = b;
				}
				cpal.close();
			}
		}
	}

	cout << "Setting up pixbuf" << endl;
	OSD_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, nx,ny);
	if (!OSD_pixbuf){
		cout << "Error allocating pixbuf - exiting" << endl;
		quicktime_close (qtfile);
		return  FIO_NO_NAME; // Error Pixbuf
	}

	guchar **row_pointers = new guchar* [ny];
	gint rowstride = gdk_pixbuf_get_rowstride (OSD_pixbuf);
	guchar *pixels = gdk_pixbuf_get_pixels (OSD_pixbuf);
	for (int i=0; i<ny; ++i) row_pointers[i] = pixels + i*rowstride;

	cout << "Composing Video..." << endl;
	int frame = 0;
	for (int time_index=offset_index_time; time_index<=max_index_time; ++time_index){
		for (int value_index=offset_index_value; value_index<=max_index_value; ++value_index){

			if (OSD_grab_mode){
				gapp->xsm->data.display.vlayer = value_index;
				gapp->xsm->data.display.vframe = time_index;
				App::spm_select_layer (NULL, gapp);
				App::spm_select_time (NULL, gapp);
				if (conti_autodisp_mode)
					gapp->xsm->AutoDisplay ();
				gapp->check_events ();
// GTK3QQQ
//				gdk_pixbuf_get_from_drawable (OSD_pixbuf, GDK_DRAWABLE (wid_canvas), NULL, 0,0,0,0, nx,ny);
			} else {

				gapp->progress_info_set_bar_fraction ((gdouble)time_index/(gdouble)max_index_time, 1);
				gapp->progress_info_set_bar_fraction ((gdouble)value_index/(gdouble)max_index_value, 2);

				// Set View-Mode Data Range and auto adapt Vcontrast/Bright
				scan->mem2d_time_element (time_index)->SetDataRange(0, maxcol);
//				scan->mem2d_time_element (time_index)->AutoDataSkl(NULL, NULL);
				scan->mem2d_time_element (time_index)->SetLayer (value_index);
				
				// use GetDataVMode to use the current set View-Mode (Direct, Quick, ...)

				int k,j;
				for (int i=0; i<ny; ++i)
					for (k=j=0; j<nx; ++j){
						double val = scan->mem2d_time_element (time_index)->GetDataVMode (j,i);
						int ival = (int)((val >= maxcol ? maxcol-1 : val < 0 ? 0 : val) + .5);
						
						*(row_pointers[i] + k++) = (unsigned char)palette_rgb[ival][0];
						*(row_pointers[i] + k++) = (unsigned char)palette_rgb[ival][1];
						*(row_pointers[i] + k++) = (unsigned char)palette_rgb[ival][2];
					}
			}

			quicktime_insert_keyframe(qtfile, frame++, 0);
			if ( quicktime_encode_video(qtfile, row_pointers, 0) != 0){
				quicktime_close (qtfile);
				cout << "Encode of Video Frame failed" << endl;
				return  FIO_NO_NAME; // Encode failed
			}

		}
	}

	cout << "Video job done. Cleaning up." << endl;
	delete[] row_pointers;

	g_object_unref (OSD_pixbuf);
	  
	if (!OSD_grab_mode)
		gapp->progress_info_close ();

	quicktime_close (qtfile);

	cout << "Finished." << endl;
	return status=FIO_OK; 
}

// Plugin's Notify Cb's, registered to be called on file load/save to check file
// return via filepointer, it is set to Zero or passed as Zero if file has been processed!
// That's all fine, you should just change the Text Stings below...


static void quicktime_im_export_filecheck_load_callback (gpointer data ){
	gchar **fn = (gchar**)data;
	if (*fn){
		PI_DEBUG (DBG_L2, "checking >" << *fn << "<" );

		Scan *dst = gapp->xsm->GetActiveScan();
		if(!dst){ 
			gapp->xsm->ActivateFreeChannel();
			dst = gapp->xsm->GetActiveScan();
		}
		Quicktime_ImExportFile fileobj (dst, *fn);

		FIO_STATUS ret = fileobj.Read(); 
		if (ret != FIO_OK){ 
			// I'am responsible! (But failed)
			if (ret != FIO_NOT_RESPONSIBLE_FOR_THAT_FILE)
				*fn=NULL;
			// no more data: remove allocated and unused scan now, force!
//			gapp->xsm->SetMode(-1, ID_CH_M_OFF, TRUE); 
			PI_DEBUG (DBG_L2, "Read Error " << ((int)ret) );
		}else{
			// got it!
			*fn=NULL;

			// Now update gxsm main window data fields
			gapp->xsm->ActiveScan->GetDataSet(gapp->xsm->data);
			gapp->spm_update_all();
			dst->draw();
		}
	}else{
		PI_DEBUG (DBG_L2, "Skipping" << *fn << "<" );
	}
}

static void quicktime_im_export_filecheck_save_callback (gpointer data ){
	gchar **fn = (gchar**)data;
	if (*fn){
		Scan *src;
		PI_DEBUG (DBG_L2, "Saving/(checking) >" << *fn << "<" );

		Quicktime_ImExportFile fileobj (src = gapp->xsm->GetActiveScan(), *fn);

		FIO_STATUS ret;
		ret = fileobj.Write(); 

		if(ret != FIO_OK){
			// I'am responsible! (But failed)
			if (ret != FIO_NOT_RESPONSIBLE_FOR_THAT_FILE)
				*fn=NULL;
			PI_DEBUG (DBG_L2, "Write Error " << ((int)ret) );
		}else{
			// write done!
			*fn=NULL;
		}
	}else{
		PI_DEBUG (DBG_L2, "Skipping >" << *fn << "<" );
	}
}

// Menu callback functions -- usually no need to edit

static void quicktime_im_export_import_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gchar **help = g_strsplit (quicktime_im_export_pi.help, ",", 2);
	gchar *dlgid = g_strconcat (quicktime_im_export_pi.name, "-import", NULL);
	gchar *fn = gapp->file_dialog_load (help[0], NULL, file_mask, NULL);
	g_strfreev (help); 
	g_free (dlgid);
	if (fn){
                quicktime_im_export_filecheck_load_callback (&fn );
                g_free (fn);
	}
}

static void quicktime_im_export_export_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gchar **help = g_strsplit (quicktime_im_export_pi.help, ",", 2);
	gchar *dlgid = g_strconcat (quicktime_im_export_pi.name, "-export", NULL);
	gchar *fn = gapp->file_dialog_save (help[1], NULL, file_mask, NULL);
	g_strfreev (help); 
	g_free (dlgid);
	if (fn){
                quicktime_im_export_filecheck_save_callback (&fn );
                g_free (fn);
	}
}








/***************** QuickTime Write HowTo *******************/

#if 0

// Step 1: Open the file
// *********************
// The first step in any Quicktime operation is to open the file. Include the Quicktime header:


// create a quicktime pointer:

quicktime_t *qtfile;

// and open the file in read or write mode:

if ((qtfile = quicktime_open(name, 0, 1)) == 0)
	return FIO_NO_NAME;

// Argument 1 is the path to a file. Argument 2 is a flag for read access. Argument 3 is a flag for write access. You can specify read or write access by setting these flags. Never specify read and write.

// quicktime_open returns a NULL if the file couldn't be opened or the format couldn't be recognized. Now you can do all sorts of operations on the file.

// When you're done using the file, call
// quicktime_close(quicktime_t *file);

//***************
  // WRITING VIDEO
  //***************

    // Immediately after opening the file, set up some tracks to write with these commands:

    // (do not call for no audio!) quicktime_set_audio(quicktime_t *file, int channels, long sample_rate, int bits, char *compressor);

  cout << "quicktime_set_video [" << qt_video_codec << "]" << endl;
quicktime_set_video(qtfile, 1, nx, ny, frame_rate, qt_video_codec);

// --> make a codec list <-- http://libquicktime.sourceforge.net/doc/apiref/group__video__codecs.html#ga5

// If you intend to use the library's built-in compression routines specify a compressor #define from quicktime.h as the compressor argument. If you want to write your own compression routine, specify any 4 byte identifier you want but don't expect the library to handle compression. The compressor applies to all tracks of the same media type, for sanity reasons.

// Once these routines are called you can optionally call

//*** void quicktime_set_parameter(quicktime_t *file, char *key, void *value);

// to set compression parameters for the codecs. Each parameter for a codec consists of a unique string and a pointer to a value. The string is unique to the codec and the parameter. The value is in a specific data type recognized by the parameter.

// To set a jpeg compression quality of 80, for example, do the following:

//	int quality = 80;
cout << "quicktime_set_parameter quality[" << qt_quality << "]" << endl;
quicktime_set_parameter(qtfile, "jpeg_quality", &qt_quality);

// The data type of the value depends on the parameter. Currently the best way to determine what parameters and value data types a particular codec supports is to look at the codec's source code. A better way may become available in the future.

// If you don't call quicktime_set_parameter the codecs will use default parameters.


// Encoding video
//------------------
//The library generates compressed video frames from a frame buffer of any colormodel in colormodels.h. First use

cout << "check for video support... " << endl;
if ( quicktime_supported_video(qtfile, 0) == 0 ){
	quicktime_close (qtfile);
	cout << "Codec not supported" << endl;
	return  FIO_NO_NAME; // Codec not supported
}
cout << "OK." << endl;

// to find out if the codec for the track is in the library. This returns 1 if it is and 0 if it isn't supported. Then use

cout << "check for colro model RGB888 support... " << endl;
if ( quicktime_writes_cmodel(qtfile, BC_RGB888, 0) == 0){
	quicktime_close (qtfile);
	cout << "Color Model RGB888 not supported" << endl;
	return  FIO_NO_NAME; // Color model not supported
}
cout << "OK." << endl;

// To query the library for a colormodel which doesn't require downsampling to drive the codec. colormodels.h contains a set of colormodel #defines which supply the colormodel argument. The function returns True or False depending on whether the colormodel argument is optimum. When a colormodel doesn't require downsampling it returns 1. Then call

cout << "Setting color model." << endl;
quicktime_set_cmodel(qtfile, BC_RGB888);

// to set the colormodel your frame buffer is in. Finally call


cout << "Allocating scratch memory." << endl;
// 	  unsigned char **row_pointers = lqt_rows_alloc ( nx, ny, BC_RGB888, 0, 0);
unsigned char **row_pointers = new unsigned char* [ny];
for (int i=0; i<ny; row_pointers[i++] = new unsigned char[3*nx]);


GtkWidget *wid_canvas = NULL;
GdkPixbuf* OSD_pixbuf = NULL;
gint w,h;
if (!scan->view->grab_OSD_canvas()) 
	OSD_grab_mode = FALSE; 
else
	wid_canvas = (GtkWidget*)(scan->view->grab_OSD_canvas());

if (wid_canvas){
	gdk_window_get_size (wid_canvas->window, &w, &h);
	OSD_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 24, w,h);
}
	  
cout << "Composing Video..." << endl;
int frame = 0;
for (int time_index=offset_index_time; time_index<=max_index_time; ++time_index){
	for (int value_index=offset_index_value; value_index<=max_index_value; ++value_index){

		if (OSD_grab_mode){
			gapp->xsm->data.display.vlayer = value_index;
			gapp->xsm->data.display.vframe = time_index;
			App::spm_select_layer (NULL, gapp);
			App::spm_select_time (NULL, gapp);
			if (conti_autodisp_mode)
				gapp->xsm->AutoDisplay ();
			gapp->check_events ();

			gdk_pixbuf_get_from_drawable (OSD_pixbuf, wid_canvas->window, NULL, 0,0,0,0, w,h);
		}

		gapp->progress_info_set_bar_fraction ((gdouble)time_index/(gdouble)max_index_time, 1);
		gapp->progress_info_set_bar_fraction ((gdouble)value_index/(gdouble)max_index_value, 2);

		// Set View-Mode Data Range and auto adapt Vcontrast/Bright
		scan->mem2d_time_element (time_index)->SetDataRange(0, maxcol);
//			scan->mem2d_time_element (time_index)->AutoDataSkl(NULL, NULL);
		scan->mem2d_time_element (time_index)->SetLayer (value_index);

		// use GetDataVMode to use the current set View-Mode (Direct, Quick, ...)

		int k,j;
		for (int i=0; i<ny; ++i)
			for (k=j=0; j<nx; ++j){
				double val = scan->mem2d_time_element (time_index)->GetDataVMode (j,i);
				int ival = (int)((val >= maxcol ? maxcol-1 : val < 0 ? 0 : val) + .5);
					
				*(row_pointers[i] + k++) = (unsigned char)palette_rgb[ival][0];
				*(row_pointers[i] + k++) = (unsigned char)palette_rgb[ival][1];
				*(row_pointers[i] + k++) = (unsigned char)palette_rgb[ival][2];
			}

		quicktime_insert_keyframe(qtfile, frame++, 0);
		if ( quicktime_encode_video(qtfile, row_pointers, 0) != 0){
			quicktime_close (qtfile);
			cout << "Encode of Video Frame failed" << endl;
			return  FIO_NO_NAME; // Encode failed
		}

	}
}

cout << "Video job done. Cleaning up." << endl;
//	  lqt_rows_free (row_pointers);
for (int i=0; i<ny; delete[] row_pointers[i++]);
delete[] row_pointers;
	  
gapp->progress_info_close ();

// to compress the frame pointed to by **row_pointers, write it at the current position of the track and advance the current position. The return value is always 1 for failure and 0 for success. The row pointers must point to rows stored in the colormodel. Planar colormodels use only the first 3 row pointers, each pointing to one of the planes.

quicktime_close (qtfile);


#endif
