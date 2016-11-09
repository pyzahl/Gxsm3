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

#ifndef __SRANGER_HWI_H
#define __SRANGER_HWI_H

//#include "include/dsp-pci32/xsm/dpramdef.h"
#include "SR-STD_spmcontrol/FB_spm_dataexchange.h" // SRanger data exchange structs and consts
#include "sranger_hwi_control.h"


/*
 * general hardware abstraction class for Signal Ranger compatible hardware
 * - instrument independent device support part
 * =================================================================
 */
class sranger_hwi_dev : public XSM_Hardware{

public: 
	friend class DSPControl;

	sranger_hwi_dev();
	virtual ~sranger_hwi_dev();

	/* Parameter  */
	virtual long GetMaxPointsPerLine(){ return  AIC_max_points; };
	virtual long GetMaxLines(){ return  AIC_max_points; };

	virtual int ReadScanData(int y_index, int num_srcs, Mem2d *m[MAX_SRCS_CHANNELS]);
	virtual int ReadProbeData(int nsrcs, int nprobe, int kx, int ky,
				  Mem2d *m, double scale=1.);
	virtual gchar* get_info();

	/* Hardware realtime monitoring -- all optional */
	/* default properties are
	 * "X" -> current realtime tip position in X, inclusive rotation and offset
	 * "Y" -> current realtime tip position in Y, inclusive rotation and offset
	 * "Z" -> current realtime tip position in Z
	 * "xy" -> X and Y
	 * "zxy" -> Z, X, Y
	 * "U" -> current bias
	 */
	virtual gint RTQuery (const gchar *property, double &val) { return FALSE; };
	virtual gint RTQuery (const gchar *property, double &val1, double &val2) { return FALSE; };
	virtual gint RTQuery (const gchar *property, double &val1, double &val2, double &val3) { return FALSE; };
	virtual gint RTQuery (const gchar *property, gchar **val) { return FALSE; };

	virtual double GetUserParam (gint n, gchar *id=NULL);
	virtual gint   SetUserParam (gint n, gchar *id=NULL, double value=0.);
	

	virtual void ExecCmd(int Cmd) {};

// SRanger format conversions and endianess adaptions
	void swap (guint16 *addr);
	void swap (gint16  *addr);
	void swap (gint32 *addr);
	void check_and_swap (gint16 &data) {
		if (swap_flg)
			swap (&data);
	};
	void check_and_swap (guint16 &data) {
		if (swap_flg)
			swap (&data);
	};
	void check_and_swap (gint32 &data) {
		if (swap_flg)
			swap (&data);
	};
	gint32 float_2_sranger_q15 (double x);
	gint32 int_2_sranger_int (gint32 x);
	gint32 long_2_sranger_long (gint32 x);

	int is_scanning() { return ScanningFlg; };
	int start_fifo_read (int y_start, 
			     int num_srcs0, int num_srcs1, int num_srcs2, int num_srcs3, 
			     Mem2d **Mob0, Mem2d **Mob1, Mem2d **Mob2, Mem2d **Mob3);

	int ReadLineFromFifo (int y_index);

	int ReadProbeFifo (int dspdev, int control=0);


	int probe_fifo_thread_active;
	int probe_time_estimate;
	int fifo_data_y_index;
	int fifo_data_num_srcs[4]; // 0: XP, 1: XM, 2: 2ND_XP, 3: 2ND_XM
	Mem2d **fifo_data_Mobp[4]; // 0: XP, 1: XM, 2: 2ND_XP, 3: 2ND_XM

	int probe_thread_dsp; // connection to SRanger used by probe thread

	inline void sr_read (int fh, void *d, size_t n){
		ssize_t ret;
		if ((ret = read (fh, d, n)) < 0){
			gchar *details = g_strdup_printf ("ret=%d", (int)ret);
			gapp->alert (N_("DSP read error"), N_("Error reading data from DSP."), details, 1);
			g_free (details);
		}
	};
	inline void sr_write (int fh, const void *d, size_t n){
		ssize_t ret;
		if ((ret = write (fh, d, n)) < 0){
			gchar *details = g_strdup_printf ("ret=%d",(int)ret);
			gapp->alert (N_("DSP read error"), N_("Error reading data from DSP."), details, 1);
			g_free (details);
		}
	};

protected:
	int ScanningFlg;
	int KillFlg;
	int dsp; // connection to SRanger
	int dsp_alternative; // 2nd connection to SRanger
	int thread_dsp; // connection to SRanger used by thread
	// add data shared w upper spm class here
	SPM_MAGIC_DATA_LOCATIONS magic_data;
	int swap_flg;

private:
	GThread *fifo_read_thread;
	GThread *probe_fifo_read_thread;
	int FifoRead (int start, int end, int &xi, int num_srcs, int len, SHT *buffer_w, LNG *buffer_l, SHT *fifo_w, LNG *fifo_l);
	gchar *productid;
	int AIC_max_points;
};


/*
 * SPM hardware abstraction class for use with Signal Ranger compatible hardware
 * ======================================================================
 */
class sranger_hwi_spm : public sranger_hwi_dev{
 public:
	sranger_hwi_spm();
	virtual ~sranger_hwi_spm();

	/* Hardware realtime monitoring -- all optional */
	/* default properties are
	 * "X" -> current realtime tip position in X, inclusive rotation and offset
	 * "Y" -> current realtime tip position in Y, inclusive rotation and offset
	 * "Z" -> current realtime tip position in Z
	 * "xy" -> X and Y
	 * "zxy" -> Z, X, Y
	 * "U" -> current bias
	 */
	virtual gint RTQuery (const gchar *property, double &val) { return FALSE; };
	virtual gint RTQuery (const gchar *property, double &val1, double &val2) { return FALSE; };
	virtual gint RTQuery (const gchar *property, double &val1, double &val2, double &val3);
	virtual gint RTQuery (const gchar *property, gchar **val) { return FALSE; };

	virtual void SetOffset(double x, double y);
	virtual void StartScan2D();
	virtual void ScanLineM(int yindex, int xdir, int muxmode,
			       Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0=0 );
	virtual void EndScan2D();
	virtual void PauseScan2D();
	virtual void ResumeScan2D();
	virtual void KillScan2D(){ KillFlg=TRUE; };

	virtual void ExecCmd(int Cmd);

	void reset_scandata_fifo (int stall=0); //!< reset scan data FIFO buffer
	void tip_to_origin (double x=0., double y=0.); //!< move tip to origin (default) or position in scan coordinate system

 protected:

 private:
	DSPControl *dc;
};

#endif

