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

#ifndef __COMEDI_HWI_H
#define __COMEDI_HWI_H

/*
 * general hardware abstraction class for comedi compatible hardware
 * - instrument independent device support part
 * =================================================================
 */
class comedi_hwi_dev : public XSM_Hardware{
public: 
	comedi_hwi_dev();
	virtual ~comedi_hwi_dev();

	virtual void StoreParameter(void){;};
	virtual void RestoreParameter(void){;};
	virtual void ExecCmd(int Cmd);
	virtual int  WaitExec(int data);

	/* Parameter  */
	virtual long GetMaxPointsPerLine(){ return max_points_per_line; };
	virtual long GetMaxLines(){ return 1L<<16; };   // no limit here
	virtual long GetMaxChannels(){ return 1L; }; // not used

	virtual void SetParameter(PARAMETER_SET &hps, int scanflg=FALSE);
	virtual void GetParameter(PARAMETER_SET &hps);
	virtual size_t ReadData(void *buf, size_t count);
	virtual int ReadScanData(int y_index, int num_srcs, Mem2d *m[MAX_SRCS_CHANNELS]);
	virtual int ReadProbeData(int nsrcs, int nprobe, int kx, int ky,
				  Mem2d *m, double scale=1.);
	virtual gchar* get_info();

protected:
	int ScanningFlg;
	int KillFlg;
	// add data shared w upper spm class here

private:
	int max_points_per_line;

	// COMEDI_DEVICE_TYPE *comedi_drv;
	// add private data/var here!
};


/*
 * SPM hardware abstraction class for use with comedi compatible hardware
 * ======================================================================
 */
class comedi_hwi_spm : public comedi_hwi_dev{
public:
	comedi_hwi_spm();
	virtual ~comedi_hwi_spm();

	virtual void PutParameter(void *src, int grp=0);

	virtual void SetDxDy(double dx, double dy);
	virtual void SetOffset(double x, double y);
	virtual void SetNx(long nx);
	virtual void SetAlpha(double alpha);

	virtual void MovetoXY (double x, double y);
	virtual void StartScan2D();
	virtual void ScanLineM(int yindex, int xdir, int muxmode, 
			       Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0=0 );
	virtual void EndScan2D();
	virtual void PauseScan2D();
	virtual void ResumeScan2D();
	virtual void KillScan2D(){ KillFlg=TRUE; };

private:
	DSP_Param dspPar;
};

#endif

