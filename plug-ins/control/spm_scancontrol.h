/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Plugin Name: spm_scancontrol.h
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

#ifndef __SPM_SCANCONTROL_H
#define __SPM_SCANCONTROL_H

#include <config.h>

typedef enum { SCAN_DIR_TOPDOWN, SCAN_DIR_TOPDOWN_BOTUP, SCAN_DIR_BOTUP } SCAN_DIR;
typedef enum { SCAN_FLAG_READY, SCAN_FLAG_STOP,  SCAN_FLAG_PAUSE,  SCAN_FLAG_RUN } SCAN_FLAG;
typedef enum { SCAN_LINESCAN, SCAN_FRAMECAPTURE } SCAN_DT_TYPE;

class MultiVoltEntry : public AppBase{
public:
	MultiVoltEntry (GtkWidget *grid, UnitObj *Volt, int i, double v) { 
		int x,y;
		x=1, y=i;
		value = v; 
		index = i;
		label = g_strdup_printf ("V%03d", index);
		input = mygtk_grid_add_input (N_(label), grid, x,y);
		ec = new Gtk_EntryControl (Volt, N_("Out of Range"), &value, -1e6, 1e6, "6.3f", input, 0.1, 0.5);

	};
	MultiVoltEntry (GtkWidget *grid, UnitObj *Volt, int i) { 
		XsmRescourceManager xrm("SPMScanControl");
		int x,y;
		x=1, y=i;
		index = i;
		label = g_strdup_printf ("V%03d", index);
		xrm.Get (label, &value, "0.0");
		input = mygtk_grid_add_input (N_(label), grid, x,y);
		ec = new Gtk_EntryControl (Volt, N_("Out of Range"), &value, -1e6, 1e6, "6.3f", input, 0.1, 0.5);

	};
	~MultiVoltEntry (){
                PI_DEBUG (DBG_L2, "~MultiVolt: " << label);
		// XsmRescourceManager xrm("SPMScanControl");
		// xrm.Put (label, value);
		//delete ec;
		g_free (label);
	};

	double volt (double v = -99999999.) { 
		if (fabs (v)  <  89999999.) { 
			XsmRescourceManager xrm("SPMScanControl");
			value=v; ec->Set_Parameter (value, TRUE, FALSE); 
			ec->Thaw (); 
			xrm.Put (label, value);
		} 
		return value; 
	};
	void   set_inactive () { ec->Freeze (); };
	int    position () { return index; };

private:
	gchar *label;
	GtkWidget *input;
	Gtk_EntryControl *ec;
	double value;
	int index;
};

// Scan Control Class based on AppBase
// -> AppBase provides a GtkWindow and some window handling basics used by Gxsm
class SPM_ScanControl : public AppBase{
public:
	SPM_ScanControl(); // create window and setup it contents, connect buttons, register cb's...
	virtual ~SPM_ScanControl(); // unregister cb's
	
	void update(); // window update (inputs, etc. -- here currently not really necessary)

	int free_scan_lists (); // clean up all old/previous scan lists
        
	int initialize_scan_lists (); /* this scans the current channel list (Channelselector)
					 and sets up a list of all scans used for data storage.
				      */
	int initialize_default_pid_src (); // not jet exsistent (plans to split off initialize_scan_lists).
	int initialize_pid_src (); // not jet exsistent.
	int initialize_daq_srcs (); // not jet exsistent.

	int prepare_to_start_scan (SCAN_DT_TYPE st=SCAN_LINESCAN); /* prepare to start a scan:
								      set common scan parameters,
								      signal "scan start event" to hardware,
								      setup basic scan parameters and put to hardware,
								      initialize scan lists,
								      check for invalid parameters -- cancel in case of bad settings (return -1 if fails, 0 if OK)
								   */
								      
	int setup_scan (int ch, const gchar *titleprefix, 
			const gchar *name, 
			const gchar *unit, const gchar *type, const gchar *label,
			double d2u=0.);
	/* configure a scan -- gets settings from channelselector/configuration */
	void do_scanline (int init=FALSE); // execute a single scan line -- or if "line == -1" a HS 2D Area Capture/Scan
	void run_probe (int ipx, int ipy); // run a local probe
	int do_scan (int l=0); // do a full scan, line by line
	int do_hscapture (); // do a full frame capute
	void set_subscan (int xs=-1, int xn=0, int ys=0, int yn=0); /* setup for partial/sub scan, 
								       current line, start at x = ix0, num points = num
								    */
	void set_sls_mode (gboolean m) { sls_mode=m; }

	void stop_scan () { 
		gapp->xsm->hardware->EndScan2D ();
		if (scan_flag == SCAN_FLAG_RUN || scan_flag == SCAN_FLAG_PAUSE) 
			scan_flag = SCAN_FLAG_STOP; 
	}; // interrupt/cancel a scan in progress
	int resume_scan () {
		if (scan_flag == SCAN_FLAG_PAUSE){
			gapp->xsm->hardware->ResumeScan2D ();
			scan_flag = SCAN_FLAG_RUN; 
		}
		return scan_flag;
	};
	int pause_scan () { 
		if (scan_flag == SCAN_FLAG_RUN){
			gapp->xsm->hardware->PauseScan2D ();
			scan_flag = SCAN_FLAG_PAUSE; 
		} else
			resume_scan ();

		return scan_flag == SCAN_FLAG_PAUSE;
	}; // pause a scan in progress
	int scan_in_progress() { 
		return scan_flag == SCAN_FLAG_RUN || scan_flag == SCAN_FLAG_PAUSE 
			? TRUE : FALSE; 
	}; // check if a scan is in progress
	int finish_scan (); /* finish the scan:
			       return to origin (center of first line (SPM)),
			       add some log info,
			       free scan lists
			     */

	double update_status_info (int reset=FALSE); // compute and show some scan status info
	void autosave_check (double sec, int initvalue=0); // check of autosave if requested

	int set_x_lookup_value (int i, double lv); // not jet used (future plans for remote...)
	int set_y_lookup_value (int i, double lv); // not jet used
	int set_l_lookup_value (int i, double lv); // not jet used

	// some helpers
	static void call_scan_start (Scan* sc, gpointer data){ 
		if (data)
			sc->start (((MultiVoltEntry*)data)->position (), ((MultiVoltEntry*)data)->volt ());
		else
			sc->start (0, gapp->xsm->data.s.Bias);
	};
	static void call_scan_draw_line (Scan* sc, gpointer data){
		gint y_realtime = gapp->xsm->hardware->RTQuery ();
		gint y_update = ((SPM_ScanControl*)data)->line2update;
//		std::cout << __func__ << " y_realtime=" << y_realtime << " y_update=" << y_update << std::endl;
		if (y_realtime >= 0 && fabs ((double)(y_realtime-y_update)) < 2)
			sc->draw ( y_update, y_update+1); // force line only refresh ### y,y+1
		else
			if (y_realtime >= 0 && fabs ((double)(y_realtime-y_update)) < 3){
				sc->draw (); // image update
				sc->draw ( y_update, y_update+1); // force line only refresh ### y+1, y+1
			}
	};
	static void call_scan_stop (Scan* sc, gpointer data){ 
		sc->stop (((SPM_ScanControl*)data)->scan_flag == SCAN_FLAG_STOP 
			  && ((SPM_ScanControl*)data)->last_scan_dir == SCAN_DIR_TOPDOWN,
			  ((SPM_ScanControl*)data)->line);
	};

	void SetScanDir (GtkWidget *w) { 
		if (G_IS_OBJECT (w))
			scan_dir = (SCAN_DIR) ((long) g_object_get_data ( G_OBJECT (w), "SCANDIR")); 
		PI_DEBUG (DBG_L2, "SCM=" << scan_dir ); };
	void ClrScanDir (GtkWidget *w) { };
        int GetScanDir () {return last_scan_dir==SCAN_DIR_TOPDOWN?1:-1;};
	void SetRepeatMode (gboolean rmd) { repeat_on = rmd; };
	gboolean RepeatMode () { return repeat_on; };
	void SetMultiVoltMode (gboolean mvmd) { mvolt_on = mvmd; };
	gboolean MultiVoltMode () { return mvolt_on; };
	guint MultiVoltNumber () { 
		guint ret =  g_slist_length(multi_volt_list);
		return (guint)multi_volt_number < ret? (guint)multi_volt_number : ret;
	};
	double MultiVoltFromList (guint i) { 
		if (!multi_volt_list) return 0.2345;
		if (i < g_slist_length (multi_volt_list)){
			MultiVoltEntry *mve = (MultiVoltEntry*) g_slist_nth_data (multi_volt_list, i);
			return mve->volt ();
		}
		return 0.12345; // for safety not 0.
	};
	MultiVoltEntry* MultiVoltElement (guint i) { 
		if (i < (guint)multi_volt_number)
			return (MultiVoltEntry*) g_slist_nth_data (multi_volt_list, i);
		else 
			return NULL;
	};
	void compute_mvolt_list (GtkWidget *grid);
	GtkWidget *remote_param;

private:
	UnitObj *Unity; // Unit "1"
	UnitObj *Volt;

	Scan   *master_scan; // master "topo" scan -- needed as common parameter reference by probe
	Scan   *master_probescan; // master "probe" scan -- needed as common parameter reference by probe

	/* Scan and ProbeScan Lists: xp = X plus (-> dir), xm = X minus (<- dir) */
	GSList *xp_scan_list, *xp_2nd_scan_list, *xp_prbscan_list;
	GSList *xm_scan_list, *xm_2nd_scan_list, *xm_prbscan_list;

	/* xp/xm source mask (bit encoding of channels to aquire) */
	int    xp_srcs, xm_srcs;
	int    xp_2nd_srcs, xm_2nd_srcs;

	int YOriginTop; /* TRUE if the Y origin is the top (first) line (all SPM),
			   else FALSE (SPALEED uses the image center)
			*/
	
	int yline; /* current scanline moveto coordinare */
	
	int line, line2update; // current scan line and line to update in background
	int ix0off; // current X offset (in pixels) -- if in subscan mode
	SCAN_FLAG scan_flag; // scan status flag
	SCAN_DIR  scan_dir, last_scan_dir; // current and last scan direction
	gboolean  do_probe; // set if currently in probe mode
	gboolean  repeat_on; // scan repeat mode flag

	gboolean  mvolt_on; // multi volt mode flag
	double    mv_start, mv_end, mv_gap;
	gint      multi_volt_number; // number of volts to use
	GSList    *multi_volt_list;

	gboolean  sls_mode;
	int       sls_config[4];
	
	GSList*   SPMC_RemoteEntryList;

 public:
	gboolean  keep_multi_layer_info;
};

#endif
