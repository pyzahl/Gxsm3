/* Gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Gxsm Hardware Interface Plugin
 * ==============================
 * hacked form Percy Zahls' demo_hwi* files
 * 
 * Author: Marcello Carla' <carla@fi.infn.it>
 * 
 * Copyright (C) 2008 Percy Zahl, Marcello Carla'
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

#ifndef __KMDSP_HWI_H
#define __KMDSP_HWI_H

//#include "include/dsp-pci32/xsm/dpramdef.h"
#include "SR-STD_spmcontrol/FB_spm_dataexchange.h" // Data exchange structs and consts
#include "kmdsp_hwi_control.h"


/*
 * general hardware abstraction class for Signal Ranger compatible hardware
 * - instrument independent device support part
 * =================================================================
 */
class kmdsp_hwi_dev : public XSM_Hardware{

public: 
	friend class kmdsp_SPM_Control;

	kmdsp_hwi_dev();
	virtual ~kmdsp_hwi_dev();

	/* Parameter  */
	virtual long GetMaxPointsPerLine(){ return  DAC_max_points; };
	virtual long GetMaxLines(){ return  DAC_max_points; };

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
	virtual gint RTQuery (gchar *property, double &val) { return FALSE; };
	virtual gint RTQuery (gchar *property, double &val1, double &val2) { return FALSE; };
	virtual gint RTQuery (gchar *property, double &val1, double &val2, double &val3) { return FALSE; };
	virtual gint RTQuery (gchar *property, gchar **val) { return FALSE; };

	virtual double GetUserParam (gint n, gchar *id=NULL);
	virtual gint   SetUserParam (gint n, gchar *id=NULL, double value=0.);
	

	virtual void ExecCmd(int Cmd) {};

// kmdsp format conversions and endianess adaptions

	int is_scanning() { return ScanningFlg; };
protected:
	int ScanningFlg;
	int KillFlg;
	int dsp; // connection to kmdsp

private:
	int DAC_max_points;
};


/*
 * SPM hardware abstraction class for use with Signal Ranger compatible hardware
 * ======================================================================
 */
class kmdsp_hwi_spm : public kmdsp_hwi_dev{
 public:
	kmdsp_hwi_spm();
	virtual ~kmdsp_hwi_spm();

	/* Hardware realtime monitoring -- all optional */
	/* default properties are
	 * "X" -> current realtime tip position in X, inclusive rotation and offset
	 * "Y" -> current realtime tip position in Y, inclusive rotation and offset
	 * "Z" -> current realtime tip position in Z
	 * "xy" -> X and Y
	 * "zxy" -> Z, X, Y
	 * "U" -> current bias
	 */
	virtual gint RTQuery (gchar *property, double &val) { return FALSE; };
	virtual gint RTQuery (gchar *property, double &val1, double &val2) { return FALSE; };
	virtual gint RTQuery (gchar *property, double &val1, double &val2, double &val3);
	virtual gint RTQuery (gchar *property, gchar **val) { return FALSE; };

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
	kmdsp_SPM_Control *dc;

	double x_scan, y_scan, z_scan;
	double x_offset, y_offset, z_offset;
	double bias, current, setpoint, scan_speed;
};

#endif

