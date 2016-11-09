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

#ifndef __XSMHARD_H
#define __XSMHARD_H

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <asm/page.h>
#include <fcntl.h>

#include "mem2d.h"
#include "xsmtypes.h"
#include "xsm_limits.h"

#include "include/dsp-pci32/xsm/dpramdef.h"

#define DSP_TIMEOUT	20  /* Zeit [s] für Automatisches Timeout */

#define MAXHP DSP_CTRL_REG_LEN

typedef enum { PRB_SHORT, PRB_FLOAT } PROBE_DATA_MODE;
typedef enum { SCAN_DATA_SWAP_SHORT, SCAN_DATA_MOVE2DPRAM_SHORT } SCAN_DATA_MODE;

/* Daten Übergabe Kontrollstruktur */

typedef struct{
	char   *name;
	double value;
} HARDWARE_PARAMETER;

typedef struct{
	int N;
	unsigned short Cmd;
	HARDWARE_PARAMETER hp[MAXHP];
} PARAMETER_SET;

/*
 * ============================================================
 * Allgemeines Basis Objekt zur Bedienung der XSM-HARDWARE 
 * ============================================================
 */

class XSM_Hardware{
 public:
	XSM_Hardware();
	virtual ~XSM_Hardware();

	virtual int update_gxsm_configurations (){ return 0; }; /* called after GUI build complete */

	virtual long GetPreScanLineOffset (){ return 0L; };

	/* query Hardware description and features */
	char* Info (int data){ return InfoString; }; /* Info */
	gchar* GetStatusInfo () { return AddStatusString; }; /* Status */

	/* Hardware Limits */
	virtual long GetMaxPointsPerLine (){ return 1L<<16; };
	virtual long GetMaxLines (){ return 1L<<16; };
	virtual long GetMaxChannels (){ return 1L; }; /* not used, may be obsoleted */

	/* Hardware realtime monitoring -- all optional */
	/* default properties are
	 * "X" -> current realtime tip position in X, inclusive rotation and offset
	 * "Y" -> current realtime tip position in Y, inclusive rotation and offset
	 * "Z" -> current realtime tip position in Z
	 * "xy" -> X and Y
	 * "zxy" -> Z, X, Y [mk2/3]
	 * "o" -> Z, X, Y-Offset [mk2/3]
	 * "f" -> feedback watch: f0, I, Irms as read on PanView [mk2/3]
	 * "s" -> status bits [FB,SC,VP,MV,(PAC)], [DSP load], [DSP load peak]  [mk2/3]
	 * "i" -> GPIO watch -- speudo real time, may be chached by GXSM: out, in, dir  [mk2/3]
	 * "U" -> current bias
	 */
	virtual gint RTQuery (const gchar *property, double &val) { return FALSE; };
	virtual gint RTQuery (const gchar *property, double &val1, double &val2) { return FALSE; };
	virtual gint RTQuery (const gchar *property, double &val1, double &val2, double &val3);
	virtual gint RTQuery (const gchar *property, gchar **val) { return FALSE; };
	virtual gint RTQuery () { return y_current; }; // actual progress on scan -- y-index mirror from FIFO read, etc. -- returns -1 if not available

	/* Methods for future use */
	virtual gchar* InqeueryUserParamId (gint n) { return NULL; };
	virtual gchar* InqeueryUserParamDescription (gint n) { return NULL; };
	virtual gchar* InqeueryUserParamUnit (gint n) { return NULL; };
	virtual double GetUserParam (gint n, const gchar *id=NULL) { return 0.; };
	virtual gint   SetUserParam (gint n, const gchar *id=NULL, double value=0.) { return FALSE; };
	
	/* 
	 * Generic scan parameters and scanhead controls
	 * All integer values are in DAC units
	 */

	/* set scan offset, HwI should move detector to this position, absolute coordinates, not rotated */
	virtual void SetOffset(double x, double y);

	/* set scan step sizes, if dy is 0 or not given dy=dx is assumed */
	virtual void SetDxDy(double dx, double dy=0.);

	/* set scan dimensions, if ny not given or 0 ny=nx is assumed */
	virtual void SetNxNy(long nx, long ny=0L);

	/* set scan angle in degree, this affects the scan-coordinate system */
	virtual void SetAlpha(double alpha);

	/* perform a moveto aktion within the scan-coordinate system (rotated by alpha) */
	virtual void MovetoXY (double x, double y);

	/* set/get scan direction, +1: Top-Down, -1: Bottom-Up */
	virtual int ScanDirection (int dir);

	/* prepare to Start Scan */
	virtual void StartScan2D(){;};

	/* Scan a Line, negative yindex: scan initialization phase in progress... */
	virtual void ScanLineM(int yindex, int xdir, int muxmode, Mem2d *Mob[MAX_SRCS_CHANNELS], int ixy_sub[4]);

	/* End,Pause,Resume,Kill/Cancel Scan */
	virtual void EndScan2D(){;};
	virtual void PauseScan2D(){;};
	virtual void ResumeScan2D(){;};
	virtual void KillScan2D(){;};

	virtual void EventCheckOn(){;};
	virtual void EventCheckOff(){;};

	virtual size_t ReadData(void *buf, size_t count) { 
		return 0; 
	};
	virtual int ReadScanData(int y_index, int num_srcs, Mem2d *m[MAX_SRCS_CHANNELS]) {
		return 0; 
	};
	virtual int ReadProbeData(int nsrcs, int nprobe, int kx, int ky, Mem2d *m, double scale=1.) { 
		return 0; 
	};
	void SetFastScan(int f=0) { fast_scan=f; };
	virtual int IsFastScan() { return fast_scan; };
	void SetSuspendWatches(int f=0) { suspend_watches=f; };
	virtual int IsSuspendWatches() { return suspend_watches; };
	virtual gchar* get_info(){ 
		return g_strdup("*--GXSM XSM_Hardware base class --*\n"
				"This is just providing a simple emulation mode.\n"
				"No Hardware is connected!\n"
				"*--Features--*\n"
				"SCAN: Yes\n"
				"PROBE: No\n"
				"ACPROBE: No\n"
				"*--EOF--*\n"
			); 
	};

	void SetScanMode(int ssm=MEM_SET){ scanmode=ssm; };
	int  FreeOldData(){ return (scanmode == MEM_SET); };
	void Transform(double *x, double *y);
	void SetIdleFunc ( void (*ifunc)(gpointer), gpointer id){
		idlefunc_data = id;
		idlefunc = ifunc;
	};

	void CallIdleFunc(){ if (idlefunc) (*idlefunc)(idlefunc_data); };

	void add_user_event_now (const gchar *message_id, const gchar *info, double value[2], gint addflag=FALSE);
	void add_user_event_now (const gchar *message_id, const gchar *info, double v1, double v2, gint addflag=FALSE){
		double v[2] = {v1, v2};
		add_user_event_now (message_id, info, v, addflag); 
	};
	void add_user_event_now (const gchar *message, double v1, double v2, gint addflag=FALSE) {
		double v[2] = {v1, v2};
		const gchar *m_id = message;
		add_user_event_now (m_id, "N/A", v, addflag); 
	};
	
	gpointer idlefunc_data;
	void (*idlefunc)(gpointer);

	gchar *InfoString;
	gchar *AddStatusString;

 private:
	double Simulate(double x, double y);

	double sim_xyzS[3];
	double sim_xyz0[3];
	gint   sim_mode;

 protected:
	long   XAnz, Nx, Ny;
	double rx, ry, Dx, Dy;
	double Alpha;
	double rotmxx,rotmxy,rotmyx,rotmyy,rotoffx,rotoffy;

	int    fast_scan; // X scale is sinodial
	int    suspend_watches;
	int    scanmode;
	int    scan_direction;

	int    y_current;
};


#endif

