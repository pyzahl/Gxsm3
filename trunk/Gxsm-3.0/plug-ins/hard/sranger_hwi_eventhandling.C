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

/* ignore this module for docuscan
% PlugInModuleIgnore
*/



#include <locale.h>
#include <libintl.h>

#include <time.h>

#include "glbvars.h"
#include "plug-ins/hard/modules/dsp.h"
#include <fcntl.h>
#include <sys/ioctl.h>

#include "gxsm/action_id.h"

#include "dsp-pci32/xsm/xsmcmd.h"

#include "sranger_hwi_control.h"
#include "sranger_hwi.h"
#include "../plug-ins/hard/modules/sranger_ioctl.h"

#define UTF8_DEGREE    "\302\260"
#define UTF8_MU        "\302\265"
#define UTF8_ANGSTROEM "\303\205"

#define CONV_16(X) X = sranger_hwi_hardware->int_2_sranger_int (X)
#define CONV_32(X) X = sranger_hwi_hardware->long_2_sranger_long (X)

extern GxsmPlugin sranger_hwi_pi;
extern sranger_hwi_dev *sranger_hwi_hardware;

// SR specific conversions and lookups

#define SRV2     (2.05/32767.)
#define SRV10    (10.0/32767.)
#define PhaseFac (1./16.)
#define BiasFac  (gapp->xsm->Inst->Dig2VoltOut (1.) * gapp->xsm->Inst->BiasGainV2V ())
#define ZAngFac  (gapp->xsm->Inst->Dig2ZA (1))
#define XAngFac  (gapp->xsm->Inst->Dig2XA (1))
#define YAngFac  (gapp->xsm->Inst->Dig2YA (1))

static int   msklookup[] = { 0x0000020, 0x0000040, 0x0000080, 0x0000100, 0x0000200, 0x0000010, 0x0000400, 0x0000800, 
			     0x0000001, 0x0000002, 
			     0x0000008, 0x0001000, 0x0002000, 0x0004000, 0x0008000, 
			     0x0000004,
			     0x0100000, 0x0200000, 0x0400000, 0x0800000, 0x1000000, 0x2000000, 0x4000000,
			     -1 
};

static int   expdi_lookup[] = { PROBEDATA_ARRAY_AIC0, PROBEDATA_ARRAY_AIC1, PROBEDATA_ARRAY_AIC2, PROBEDATA_ARRAY_AIC3,
				PROBEDATA_ARRAY_AIC4, PROBEDATA_ARRAY_AIC5_FBS, PROBEDATA_ARRAY_AIC6, PROBEDATA_ARRAY_AIC7,
				PROBEDATA_ARRAY_AIC5OUT_ZMON, PROBEDATA_ARRAY_AIC6OUT_UMON, 
				PROBEDATA_ARRAY_LCK0, PROBEDATA_ARRAY_LCK1A, PROBEDATA_ARRAY_LCK1B, PROBEDATA_ARRAY_LCK2A, PROBEDATA_ARRAY_LCK2B,
				PROBEDATA_ARRAY_COUNT,
				PROBEDATA_ARRAY_TIME, PROBEDATA_ARRAY_XS, PROBEDATA_ARRAY_YS, PROBEDATA_ARRAY_ZS,  PROBEDATA_ARRAY_U, PROBEDATA_ARRAY_PHI,
				PROBEDATA_ARRAY_SEC,
				0,0
};

const gchar* lablookup[]  = { "ADC0", "ADC1", "ADC2", "ADC3", "ADC4", "ADC5-I","ADC6","ADC7",
			      "Zmon", "Umon", 
			      "LockIn0", "LockIn1stA", "LockIn1stB", "LockIn2ndA", "LockIn2ndB",
			      "Count",
			      "Time", "XS",   "YS",   "ZS",   "Bias", "Phase", "VP-Section",
			      "BLOCK", NULL
};

const gchar* unitlookup[] = { "V",   "V",    "V",    "V",    "V",    "V",     "V",   "V",
			      UTF8_ANGSTROEM,   "V",    
			      "V", "dV", "dV",  "ddV", "ddV",
			      "CNT",
			      "ms", UTF8_ANGSTROEM, UTF8_ANGSTROEM, UTF8_ANGSTROEM, "V", "deg", "#",
			      "BS", NULL
};

// watch verbosity...
# define LOGMSGS0(X) std::cout << X
//# define LOGMSGS0(X)

# define LOGMSGS(X) std::cout << X
//# define LOGMSGS(X)

# define LOGMSGS2(X) std::cout << X
//# define LOGMSGS2(X)

int dbg_stack_size=0;
int dbg_stack_mem=0;

int DSPControl::Probing_eventcheck_callback( GtkWidget *widget, DSPControl *dspc){
	int popped=0;
	GArray **garr;
	double DAC2Ulookup[]={ SRV10, SRV10,  SRV10,  SRV10,  SRV10,  SRV10,   SRV10,  SRV10,
			       ZAngFac, BiasFac, SRV10,  SRV10,  SRV10, SRV10,  SRV10, 
			       1.,
			       1e3/dspc->frq_ref, XAngFac, YAngFac, ZAngFac, BiasFac, PhaseFac, 1.,
			       0.,0.
	};

	// pop off all available data from stack
	while ((garr = dspc->pop_probedata_arrays ()) != NULL){
		++popped;
		
// atach event to active channel, if one exists -- raster mode ------------------------------

		ScanEvent *se = NULL;
		if (gapp->xsm->MasterScan){
			// find first section header and take this X,Y coordinates as reference -- start of probe event
			int sec = 0;
			int bi=-1;
			for (int j=0; j<dspc->last_probe_data_index; ++j){
				int bsi = g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_BLOCK], double, j);
				int s = (int) g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_SEC], double, j);
				if (j>0 && (bsi == bi && s >= sec)){
					sec = s;
					continue;
				}
				sec = s; bi = bsi;

				// find chunksize (total # data sources)
				int chunksize = 0;
				GPtrArray *glabarray = g_ptr_array_new ();
				GPtrArray *gsymarray = g_ptr_array_new ();

				for (int src=0; msklookup[src]>=0 && src < MAX_NUM_CHANNELS; ++src)
					if (dspc->Source & msklookup[src]){
						g_ptr_array_add (glabarray, (gpointer) lablookup[src]);
						g_ptr_array_add (gsymarray, (gpointer) unitlookup[src]);
						++chunksize;
					}
		
				if (chunksize > 0 && chunksize < MAX_NUM_CHANNELS){
					double dataset[MAX_NUM_CHANNELS];
					ProbeEntry *pe = new ProbeEntry ("Probe", time(0), glabarray, gsymarray, chunksize);
					se = new ScanEvent (
						gapp->xsm->Inst->Dig2X0A ((long) round (g_array_index (garr [PROBEDATA_ARRAY_X0], double, j)))
						+ gapp->xsm->Inst->Dig2XA ((long) round (g_array_index (garr [PROBEDATA_ARRAY_XS], double, j))),
						gapp->xsm->Inst->Dig2Y0A ((long) round (g_array_index (garr [PROBEDATA_ARRAY_Y0], double, j)))
						+ gapp->xsm->Inst->Dig2YA ((long) round (g_array_index (garr [PROBEDATA_ARRAY_YS], double, j))),
						gapp->xsm->Inst->Dig2ZA ((long) round (g_array_index (garr [PROBEDATA_ARRAY_ZS], double, j)))
						);
					gapp->xsm->MasterScan->mem2d->AttachScanEvent (se);
#if 0
					g_print ("j=%4d SE @ (%gA, %gA)\n", j,
						 gapp->xsm->Inst->Dig2X0A ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_X0], double, j)))
						 + gapp->xsm->Inst->Dig2XA ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_XS], double, j))),
						gapp->xsm->Inst->Dig2Y0A ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_Y0], double, j)))
						+ gapp->xsm->Inst->Dig2YA ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_YS], double, j)))
						);
#endif
					for (int i = 0; i < dspc->last_probe_data_index; i++){
						int j=0;
						for (int src=0; msklookup[src]>=0 && src < MAX_NUM_CHANNELS; ++src){
							if (dspc->Source & msklookup[src])
								dataset[j++] = DAC2Ulookup[src] * g_array_index (garr [expdi_lookup[src]], double, i);
						}
						pe->add ((double*)&dataset);
					}
					se->add_event (pe);
				}

				break;
			}
		}

		// if we have attached an scan event, so fill it with data now

		free_probedata_array_set (garr, dspc);	
	}

	if (popped > 0)
		if (gapp->xsm->MasterScan)
			gapp->xsm->MasterScan->view->update_events ();

	return 0;
}

#define XSM_DEBUG_PG(X)  std::cout << X << std::endl;
//#define XSM_DEBUG_PG(X) ;

void DSPControl::probedata_visualize (GArray *probedata_x, GArray *probedata_y, GArray *probedata_sec,
				      ProfileControl* &pc, ProfileControl* &pc_av, int plot_msk,
				      const gchar *xlab, const gchar *xua, double xmult,
				      const gchar *ylab, const gchar *yua, double ymult,
				      int current_i){

	UnitObj *UXaxis = new UnitObj(xua, " ", "g", xlab);
	UnitObj *UYaxis = new UnitObj(yua,  " ", "g", ylab);
	double xmin, xmax, x;
//	double ymin, ymax, y;
	
	XSM_DEBUG_PG ("Pobing_graph_callback Visualization X-limits" );

	// find min and max X limit
	xmin = xmax = g_array_index (probedata_x, double, 0);
//	ymin = ymax = g_array_index (probedata_y, double, 0);
	for(int i = 1; i < current_i; i++){
		x = g_array_index (probedata_x, double, i);
//		y = g_array_index (probedata_y, double, i);
		if (x > xmax) xmax = x;
		if (x < xmin) xmin = x;
//		if (y > ymax) ymax = y;
//		if (y < ymin) ymin = y;
	}
	xmax *= xmult;
	xmin *= xmult;

	if (xmin == xmax) return;
//	if (ymin == ymax) return;
//	if (ymin == NAN || ymax == NAN) return;

	XSM_DEBUG_PG ("Pobing_graph_callback Visualization U&T/Rz" );
	UXaxis->SetAlias (xlab);
	UYaxis->SetAlias (ylab);
	if (!pc){
		gchar   *title  = g_strdup_printf ("Vector Probe, Channel: %s", ylab);
		pc = new ProfileControl (title, current_i, 
					  UXaxis, UYaxis, 
					  xmin, xmax,
					  ylab);
		g_free (title);
	} else {
		pc->scan1d->mem2d->Resize (current_i, 1);
		pc->SetXrange (xmin, xmax);
	}

	gint spectra=0; // used to count spectra
	gint spectra_section=0; // used to count spectra
	gint spectra_index=0; // used to count data points within spectra
	gdouble spectra_average[current_i][2]; // holds averaged spectra; ..[0] source ..[1] value
	for(int i = 0; i < current_i; i++){
		spectra_average[i][0]=0;
		spectra_average[i][1]=0;
	}
	for(int i = 0; i < current_i; i++){
		if (g_array_index (probedata_sec, double, i) < spectra_section)
		{
			spectra++;
			spectra_section = 0;
			spectra_index = 0;
		} else {
			spectra_index++;
			spectra_section = (int) g_array_index (probedata_sec, double, i);
		}
		pc->SetPoint (i,
			      xmult * g_array_index (probedata_x, double, i),
			      ymult * g_array_index (probedata_y, double, i));
		if (PlotAvg & plot_msk){
			spectra_average[spectra_index][0] = spectra_average[spectra_index][0] + g_array_index (probedata_x, double, i);
			spectra_average[spectra_index][1] = spectra_average[spectra_index][1] + g_array_index (probedata_y, double, i);
		}
		if (PlotSec & plot_msk){
			spectra_average[spectra_index][0] = g_array_index (probedata_x, double, i);
			spectra_average[spectra_index][1] = g_array_index (probedata_y, double, i);
		}
	}

	pc->UpdateArea ();
	pc->show ();

	// Create graph for averaged data; you will find them in the pc-array above 
	if (spectra>0 && (PlotAvg & plot_msk || PlotSec & plot_msk)){
		XSM_DEBUG_PG ("Pobing_graph_callback Visualization new pc -- put Av/Sec data" );
		if (!pc_av){
			gchar   *title  = g_strdup_printf ("Vector Probe, Channel: %s %s", ylab, (PlotAvg & plot_msk)?"averaged":"cur. section");
			pc_av = new ProfileControl (title, spectra_index+1, 
						    UXaxis, UYaxis, xmin, xmax, ylab);
			g_free (title);
		} else {
			pc_av->scan1d->mem2d->Resize (spectra_index+1, 1);
			pc_av->SetXrange (xmin, xmax);
		}
		double norm = 1.;
		if (PlotAvg & plot_msk)
			norm = 1./(spectra+1.);
		for(int i = 0; i < spectra_index+1; i++)
			pc_av->SetPoint (i, 
					 xmult * norm * spectra_average[i][0], 
					 ymult * norm * spectra_average[i][1]/(spectra+1));
						
		pc_av->UpdateArea ();
		pc_av->show ();
	} else if (pc_av && !(PlotAvg & plot_msk || PlotSec & plot_msk)){
		delete pc_av;
		pc_av = NULL; // get rid of it now
	}


//	LOGMSGS2 ( "VIS::VP-data-stack size=" << dbg_stack_size << " mem=" << dbg_stack_mem << " d=" << (dbg_stack_size-dbg_stack_mem) << std::endl);
}



int DSPControl::Probing_graph_callback( GtkWidget *widget, DSPControl *dspc, int finish_flag){
// show and update data pv graph
	static gboolean busy=FALSE;

	XSM_DEBUG_PG ("Pobing_graph_callback" );
	if (busy) return -1;
	busy=TRUE;

	XSM_DEBUG_PG ("Pobing_graph_callback lock?" );

	while (	dspc->pv_lock ) gapp->check_events_self (); // quiet

	XSM_DEBUG_PG ("Pobing_graph_callback data-ready?" );

	if (!dspc->current_probe_data_index){
		busy=FALSE;
		return 0;
	}

	double DAC2Ulookup[]={ SRV10, SRV10,  SRV10,  SRV10,  SRV10,  SRV10,   SRV10,  SRV10,
			       ZAngFac, BiasFac, SRV10,  SRV10,  SRV10, SRV10,  SRV10,  
			       1.,
			       1e3/dspc->frq_ref, XAngFac, YAngFac, ZAngFac, BiasFac, PhaseFac, 1.,
			       0.,0.
	};
		

//xxxxxxxxxxxxx atach event to active channel, if one exists -- manual mode xxxxxxxxxxxxxxxxxxxxx

	XSM_DEBUG_PG ("Pobing_graph_callback MasterScan? Add Ev." );

	ScanEvent *se = NULL;
	if (gapp->xsm->MasterScan && finish_flag){
		// find first section header and take this X,Y coordinates as reference -- start of probe event
		XSM_DEBUG_PG ("Pobing_graph_callback MasterScan: adding Ev." );
		int sec = 0;
		int bi = -1;
		int j0=0;
		for (int j=0; j<dspc->current_probe_data_index; ++j){
			int bsi = g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_BLOCK], double, j);
			int s = (int) g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_SEC], double, j);
			if (j>0 && (bsi == bi && s >= sec)){
				sec = s;
				continue;
			}
			sec = s; bi = bsi;
			g_print ("j=%4d [s:%d, bsi:%d]\n", j, s, bsi);

			// for all sections add probe event!!!
			XSM_DEBUG_PG ("Pobing_graph_callback ScanEvent-update/add" );
			// find chunksize (total # data sources)
			int chunksize = 0;
			GPtrArray *glabarray = g_ptr_array_new ();
			GPtrArray *gsymarray = g_ptr_array_new ();
			
			for (int src=0; msklookup[src]>=0 && src < MAX_NUM_CHANNELS; ++src)
				if (dspc->Source & msklookup[src]){
					g_print ("%s[%s]\n",lablookup[src],unitlookup[src]);
					g_ptr_array_add (glabarray, (gpointer) lablookup[src]);
					g_ptr_array_add (gsymarray, (gpointer) unitlookup[src]);
					++chunksize;
				}
			
			if (chunksize > 0 && chunksize < MAX_NUM_CHANNELS){
				double dataset[MAX_NUM_CHANNELS];
				ProbeEntry *pe = new ProbeEntry ("Probe", time(0), glabarray, gsymarray, chunksize);

				se = new ScanEvent (
					gapp->xsm->Inst->Dig2X0A ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_X0], double, j)))
					+ gapp->xsm->Inst->Dig2XA ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_XS], double, j))),
					gapp->xsm->Inst->Dig2Y0A ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_Y0], double, j)))
					+ gapp->xsm->Inst->Dig2YA ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_YS], double, j))),
					gapp->xsm->Inst->Dig2ZA ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_ZS], double, j)))
					);
				gapp->xsm->MasterScan->mem2d->AttachScanEvent (se);
#if 0
				g_print ("j=%4d SE @ (%gA, %gA)\n", j,
					gapp->xsm->Inst->Dig2X0A ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_X0], double, j)))
					+ gapp->xsm->Inst->Dig2XA ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_XS], double, j))),
					gapp->xsm->Inst->Dig2Y0A ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_Y0], double, j)))
					+ gapp->xsm->Inst->Dig2YA ((long) round (g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_YS], double, j)))
					);
#endif
				XSM_DEBUG_PG ("Pobing_graph_callback have ScanEvent!" );
				
				
				for (int i = j0; i < j; i++){
					int k=0;
					for (int src=0; msklookup[src]>=0 && src < MAX_NUM_CHANNELS; ++src){
						if (dspc->Source & msklookup[src])
							dataset[k++] = DAC2Ulookup[src] * g_array_index (dspc->garray_probedata [expdi_lookup[src]], double, i);
					}
					pe->add ((double*)&dataset);
				}
				se->add_event (pe);
			} else {
				XSM_DEBUG_PG ("Pobing_graph_callback -- EMPTY DATA, skipping." );
			}

			j0=j;

			gapp->xsm->MasterScan->view->update_events ();
		}
	}

// on-the-fly visualisation -- all graphs update and cleanup if unused
	for (int xmap=0; msklookup[xmap]>=0; ++xmap){
		if ((dspc->XSource & msklookup[xmap]) && (dspc->Source & msklookup[xmap]))
			for (int src=0; msklookup[src]>=0; ++src){
				if (xmap == src) continue;
				if ((dspc->PSource & msklookup[src]) && (dspc->Source & msklookup[src])){
					dspc->probedata_visualize (
						dspc->garray_probedata [expdi_lookup[xmap]], 
						dspc->garray_probedata [expdi_lookup[src]], 
						dspc->garray_probedata [PROBEDATA_ARRAY_SEC], 
						dspc->probe_pc_matrix[xmap][src], 
						dspc->probe_pc_matrix[MAX_NUM_CHANNELS+xmap][MAX_NUM_CHANNELS+src],
						msklookup[src],
						lablookup[xmap], unitlookup[xmap], DAC2Ulookup[xmap],
						lablookup[src], unitlookup[src], DAC2Ulookup[src],
						dspc->current_probe_data_index);
				} else { // clean up unused windows...
					if (dspc->probe_pc_matrix[xmap][src]){
						delete dspc->probe_pc_matrix[xmap][src]; // get rid of it now
						dspc->probe_pc_matrix[xmap][src] = NULL;
					}
					if (dspc->probe_pc_matrix[MAX_NUM_CHANNELS+xmap][MAX_NUM_CHANNELS+src]){
						delete dspc->probe_pc_matrix[MAX_NUM_CHANNELS+xmap][MAX_NUM_CHANNELS+src]; // get rid of it now
						dspc->probe_pc_matrix[MAX_NUM_CHANNELS+xmap][MAX_NUM_CHANNELS+src] = NULL;
					}
				}
			}
		else{   // remove not used ones now! pc[2*MAX_NUM_CHANNELS][2*MAX_NUM_CHANNELS]
			for (int src=0; src<MAX_NUM_CHANNELS; ++src){
				if (dspc->probe_pc_matrix[xmap][src]){
					delete dspc->probe_pc_matrix[xmap][src]; // get rid of it now
					dspc->probe_pc_matrix[xmap][src] = NULL;
				}
				if (dspc->probe_pc_matrix[MAX_NUM_CHANNELS+xmap][MAX_NUM_CHANNELS+src]){
					delete dspc->probe_pc_matrix[MAX_NUM_CHANNELS+xmap][MAX_NUM_CHANNELS+src]; // get rid of it now
						dspc->probe_pc_matrix[MAX_NUM_CHANNELS+xmap][MAX_NUM_CHANNELS+src] = NULL;
				}
			}
		}
	}

	XSM_DEBUG_PG ("Pobing_graph_callback DONE." );
	busy=FALSE;

//	LOGMSGS2 ( "GRAPH::VP-data-stack size=" << dbg_stack_size << " mem=" << dbg_stack_mem << " d=" << (dbg_stack_size-dbg_stack_mem) << std::endl);

	return 0;
}

// abort probe and stop fifo read, plot data until then
int DSPControl::Probing_abort_callback( GtkWidget *widget, DSPControl *dspc){
	dspc->Probing_graph_callback (widget, dspc);

	// can not simply cancel a DSP vector program in progress -- well can, but: this leaves it in an undefined state of all effected outputs incl.
	// ==> feedback state ON or OFF. SO AFTER THAT -- CHEC and eventually manually recover settings!
	// but aborting on your request
	dspc->Probing_exec_ABORT_callback (widget, dspc);
}


int DSPControl::Probing_save_callback( GtkWidget *widget, DSPControl *dspc){
	int sec, bi;
// show and update data pv graph
	if (!dspc->current_probe_data_index) 
		return 0;
	

	double DAC2Ulookup[]={ SRV10, SRV10,  SRV10,  SRV10,  SRV10,  SRV10,   SRV10,  SRV10,
			       ZAngFac, BiasFac, SRV10,  SRV10,  SRV10,   SRV10,  SRV10,   
			       1.,
			       1e3/dspc->frq_ref, XAngFac, YAngFac, ZAngFac, BiasFac, PhaseFac, 1.,
			       0.
	};
	       
	const gchar *separator = "\t";

	std::ofstream f;

	// XsmRescourceManager xrm("FilingPathMemory");
	// gchar *path = xrm.GetStr ("Probe_DataSavePath", xsmres.DataPath);
	gchar *fntmp = g_strdup_printf ("%s/%s%03d-%s.vpdata",
					// path, 
					g_settings_get_string (gapp->get_as_settings (), "auto-save-folder-probe"), 
					gapp->xsm->data.ui.basename, ++gapp->xsm->counter, "VP");
	// g_free (path);

	time_t t;
	time(&t);

	f.open (fntmp);

	int ix=-999999, iy=-999999;
	if (gapp->xsm->MasterScan){
		gapp->xsm->MasterScan->World2Pixel (gapp->xsm->data.s.x0, gapp->xsm->data.s.y0, ix, iy, SCAN_COORD_ABSOLUTE);
	}
	f << std::setprecision(12)
	f << "# view via: xmgrace -graph 0 -pexec 'title \"GXSM Vector Probe Data: " << fntmp << "\"' -block " << fntmp  << " -bxy 2:4 ..." << std::endl;
	f << "# GXSM Vector Probe Data :: VPVersion=00.02 vdate=20070227" << std::endl;
	f << "# Date                   :: date=" << ctime(&t) << "#" << std::endl;
	f << "# FileName               :: name=" << fntmp << std::endl;
	f << "# GXSM-Main-Offset       :: X0=" << gapp->xsm->data.s.x0 << " Ang" <<  "  Y0=" << gapp->xsm->data.s.y0 << " Ang" 
	  << ", iX0=" << ix << " Pix iX0=" << iy << " Pix"
	  << std::endl;
	f << "# GXSM-DSP-Control-FB    :: Bias=" << dspc->bias << " V" <<  ", Current=" << dspc->current_set_point  << " nA" << std::endl; 
	f << "# GXSM-DSP-Control-STS   :: #IV=" << dspc->IV_repetitions << " " << std::endl; 

	gchar *tmp = g_strdup(gapp->xsm->data.ui.comment);
	gchar *cr;
	while (cr=strchr (tmp, '\n'))
		*cr = ' ';
	f << "# GXSM-Main-Comment      :: comment=\"" << tmp << "\"" << std::endl;
	g_free (tmp);
	f << "# Probe Data Number      :: N=" << dspc->current_probe_data_index << std::endl;
	f << "# Data Sources Mask      :: Source=" << dspc->Source << std::endl;
	f << "# X-map Sources Mask     :: XSource=" << dspc->XSource << std::endl;
	f << "#C " << std::endl;
	f << "#C VP Channel Map and Units lookup table used:=table [## msk expdi, lab, DAC2U, unit/DAC, Active]" << std::endl;

	for (int i=0; msklookup[i] >= 0; ++i)
		f << "# Cmap[" << i << "]" << separator 
		  << msklookup[i] << separator 
		  << expdi_lookup[i] << separator 
		  << lablookup[i] << separator 
		  << DAC2Ulookup[i] << separator 
		  << unitlookup[i] << "/DAC" << separator 
		  << (dspc->Source & msklookup[i] ? "Yes":"No") << std::endl;

	f << "#C " << std::endl;
	f << "#C Full Position Vector List at Section boundaries follows :: PositionVectorList" << std::endl;
	sec = 0; bi = -1;
	for (int j=0; j<dspc->current_probe_data_index; ++j){
		int bsi = g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_BLOCK], double, j);
		int s = (int) g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_SEC], double, j);
		if (bsi != bi){
			bi = bsi;
			f << "# S[" << s << "]  :: VP[" << j <<"]=(";
		} else
			continue;

		for (int i=13; msklookup[i]>=0; ++i){
			double ymult = DAC2Ulookup[i];
			if (i>13) 
				f << ", ";
			f << " \"" << lablookup[i] << "\"="
			  << (ymult * g_array_index (dspc->garray_probedata [expdi_lookup[i]], double, j))
			  << " " << unitlookup[i];
		}
		f << ")" << std::endl;
	}

	f << "#C " << std::endl;
	f << "#C Data Table             :: data=" << std::endl;

	for (int i = -1; i < dspc->current_probe_data_index; i++){
		if (i == -1)
			f << "#C Index" << separator;
		else
			f << i << separator;

		for (int xmap=0; msklookup[xmap]>=0; ++xmap)
			if ((dspc->XSource & msklookup[xmap]) && (dspc->Source & msklookup[xmap])){
				double xmult = DAC2Ulookup[xmap];
				if (i == -1)
					f << "\"" << lablookup[xmap] << " (" << unitlookup[xmap] << ")\"" << separator;
				else
					f << (xmult * g_array_index (dspc->garray_probedata [expdi_lookup[xmap]], double, i)) << separator;
			}

		for (int src=0; msklookup[src]>=0; ++src)
			if (dspc->Source & msklookup[src]){
				double ymult = DAC2Ulookup[src];
				if (i == -1)
					f << "\"" << lablookup[src] << " (" << unitlookup[src] << ")\"" << separator;
				else
					f << (ymult * g_array_index (dspc->garray_probedata [expdi_lookup[src]], double, i)) << separator;
			}

		if (i == -1)
			f << "Block-Start-Index";
		else
			f << g_array_index (dspc->garray_probedata [PROBEDATA_ARRAY_BLOCK], double, i);
		f << std::endl;
	}
	f << "#C " << std::endl;
	f << "#C END." << std::endl;
	f.close ();

	// update counter on main window!
	gapp->spm_update_all();
	gapp->SetStatus(N_("Saved VP data: "), fntmp);
	gchar *bbt = g_strdup_printf ("Save now - last: %s", fntmp);
	gtk_button_set_label (GTK_BUTTON (dspc->save_button), bbt);
	g_free (bbt);
	g_free (fntmp);

	
	return 0;
}



#define DEFAULT_PROBE_LEN 256 // can increase automatically, just more efficient

void DSPControl::push_probedata_arrays (){
	GArray **garr = new GArray*[NUM_PROBEDATA_ARRAYS];
	pv_lock = TRUE;
	for (int i=0; i<NUM_PROBEDATA_ARRAYS; ++i){
		garr [i] = garray_probedata [i];
		garray_probedata [i] = g_array_sized_new (FALSE, TRUE, sizeof (double), DEFAULT_PROBE_LEN); // preallocated, can increase
	}
	probedata_list =  g_slist_prepend (probedata_list, garr); // push on list
	last_probe_data_index = current_probe_data_index;

//
//	++dbg_stack_size;
//	++dbg_stack_mem;

	++num_probe_events;
	pv_lock = FALSE;

//	LOGMSGS2 ( "PUSH::VP-data-stack size=" << dbg_stack_size << " mem=" << dbg_stack_mem << " d=" << (dbg_stack_size-dbg_stack_mem) << std::endl);
}

// return last dataset and removed it from list, does not touch data itself
GArray** DSPControl::pop_probedata_arrays (){
	pv_lock = TRUE;
	if (probedata_list){
		GSList *last = g_slist_last (probedata_list);
		if (last){
			GArray **garr = (GArray **) (last->data);
			probedata_list = g_slist_delete_link (probedata_list, last);
			return garr;
		}
//		--dbg_stack_size;
	}
	pv_lock = FALSE;
	return NULL;
}

void DSPControl::free_probedata_array_set (GArray** garr, DSPControl *dc){
	dc->pv_lock = TRUE;
	for (int i=0; i<NUM_PROBEDATA_ARRAYS; ++i)
		g_array_free (garr[i], TRUE);
	dc->pv_lock = FALSE;
//	--dbg_stack_mem;
}

void DSPControl::free_probedata_arrays (){
	if (!probedata_list) 
		return;
	g_slist_foreach (probedata_list, (GFunc) DSPControl::free_probedata_array_set, this);
	g_slist_free (probedata_list);
	probedata_list = NULL;	
	num_probe_events = 0;
	pv_lock = FALSE;
}

void DSPControl::init_probedata_arrays (){
	for (int i=0; i<NUM_PROBEDATA_ARRAYS; ++i){
		if (!garray_probedata[i]){
			garray_probedata [i] = g_array_sized_new (FALSE, TRUE, sizeof (double), DEFAULT_PROBE_LEN); // preallocated, can increase
		}
		else
			g_array_set_size (garray_probedata [i], 0);
	}
	current_probe_data_index = 0;
	nun_valid_data_sections = 0;
	pv_lock = FALSE;
}

void DSPControl::add_probedata(double data[13]){ 
	int i,j;
	pv_lock = TRUE;
	for (i = PROBEDATA_ARRAY_AIC5OUT_ZMON, j=0; i <= PROBEDATA_ARRAY_END; ++i, ++j)
		g_array_append_val (garray_probedata[i], data[j]);

#ifdef TTY_DEBUG
	std::cout << "pvd[" << current_probe_data_index << "][M[zu]AIC[50123467]L[120]]: ";
	for (i = PROBEDATA_ARRAY_AIC5OUT_ZMON, j=0; i <= PROBEDATA_ARRAY_END; ++i, ++j)
		std::cout << data[j] << ", ";
	std::cout << std::endl;
#endif

	current_probe_data_index++;	
	pv_lock = FALSE;
}

void DSPControl::add_probevector(){ 
	int i,j, sec;
	double ds, val, multi, fixptm;
	pv_lock = TRUE;
	sec =  (int) (ds = g_array_index (garray_probedata [PROBEDATA_ARRAY_SEC], double, current_probe_data_index-1));
	g_array_append_val (garray_probedata [PROBEDATA_ARRAY_SEC], ds);
	g_array_append_val (garray_probedata [PROBEDATA_ARRAY_INDEX], current_probe_data_index);
	multi = dsp_vector_list[sec].dnx + 1;
	fixptm = 1./(1<<16);

	// Block Start Index
	val = g_array_index (garray_probedata[PROBEDATA_ARRAY_BLOCK], double, current_probe_data_index-1);
	g_array_append_val (garray_probedata[PROBEDATA_ARRAY_BLOCK], val);

#ifdef TTY_DEBUG
	std::cout << "+pv[" << current_probe_data_index << "] (m=" << multi << "): ";
#endif
	for (i = PROBEDATA_ARRAY_TIME, j=0; i < PROBEDATA_ARRAY_SEC; ++i, ++j){
		val = g_array_index (garray_probedata[i], double, current_probe_data_index-1);
		switch (i){
		case PROBEDATA_ARRAY_TIME:
			val += multi;
			break;
		case PROBEDATA_ARRAY_X0:
			val += dsp_vector_list[sec].f_dx0*multi*fixptm;
			break;
		case PROBEDATA_ARRAY_Y0:
			val += dsp_vector_list[sec].f_dy0*multi*fixptm;
			break;
		case PROBEDATA_ARRAY_PHI:
			val += dsp_vector_list[sec].f_dphi*multi*fixptm;
			break;
		case PROBEDATA_ARRAY_XS:
			val -= dsp_vector_list[sec].f_dx*multi*fixptm;
			break;
		case PROBEDATA_ARRAY_YS:
			val -= dsp_vector_list[sec].f_dy*multi*fixptm;
			break;
		case PROBEDATA_ARRAY_ZS:
			val -= dsp_vector_list[sec].f_dz*multi*fixptm;
			break;
		case PROBEDATA_ARRAY_U:
			val += dsp_vector_list[sec].f_du*multi*fixptm;
			break;
		default:
			break; // error!!!!
		}
		g_array_append_val (garray_probedata[i], val);
#ifdef TTY_DEBUG
		std::cout << val << ", ";
#endif
	}
	++nun_valid_data_sections;
	pv_lock = FALSE;

#ifdef TTY_DEBUG
	std::cout << sec << std::endl;
#endif
}

void DSPControl::set_probevector(double pv[9]){ 
	int i,j;
	pv_lock = TRUE;
	g_array_append_val (garray_probedata [PROBEDATA_ARRAY_INDEX], current_probe_data_index);
	for (i = PROBEDATA_ARRAY_TIME, j=0; i <= PROBEDATA_ARRAY_SEC; ++i, ++j)
		g_array_append_val (garray_probedata[i], pv[j]);

	double di = (double)current_probe_data_index;
	g_array_append_val (garray_probedata[PROBEDATA_ARRAY_BLOCK], di);
	
#ifdef TTY_DEBUG
	std::cout << "pv[" << current_probe_data_index << "]_[txyz0xyzSus]set: ";
	for (i = PROBEDATA_ARRAY_TIME, j=0; i <= PROBEDATA_ARRAY_SEC; ++i, ++j)
		std::cout << pv[j] << ", ";
	std::cout << std::endl;
#endif
	pv_lock = FALSE;
}


