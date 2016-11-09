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

#ifndef __INNOVATIVE_DSP_HWI_H
#define __INNOVATIVE_DSP_HWI_H


#include "include/dsp-pci32/xsm/dpramdef.h"
#include "FB_spm_dataexchange.h" // SRanger data exchange structs and consts



/*
 * DSP Objekt für Steuerung über device /dev/pci32 kernelmodul
 * ============================================================
 */
class innovative_dsp_hwi_dev : public XSM_Hardware{
public: 
  innovative_dsp_hwi_dev(int ver=1);
  virtual ~innovative_dsp_hwi_dev();

  virtual void StoreParameter(void){;};
  virtual void RestoreParameter(void){;};
  virtual void ExecCmd(int Cmd);
  virtual int  WaitExec(int data);

  /* Parameter  */
  virtual long GetMaxPointsPerLine(){ return max_points_per_line; };
  virtual long GetMaxLines(){ return 1L<<16; };
  virtual long GetMaxChannels(){ return 1L; };

  virtual void SetParameter(PARAMETER_SET &hps, int scanflg=FALSE); /* universelle Parameterübergabe */
  virtual void GetParameter(PARAMETER_SET &hps); /* universelle Parameterübergabe */
  virtual size_t ReadData(void *buf, size_t count);
  virtual int ReadScanData(int y_index, int num_srcs, Mem2d *m[MAX_SRCS_CHANNELS]);
  virtual int ReadProbeData(int nsrcs, int nprobe, int kx, int ky, Mem2d *m, double scale=1.);
  virtual gchar* get_info(){ return get_DSP_softinfo();  };

protected:
  int ScanningFlg;
  gchar* get_DSP_softinfo();
  static void Evchk();
  static void NoEvchk();
  void EventCheckOn(){ EventCheck = Evchk; };
  void EventCheckOff(){ EventCheck = NoEvchk; };
  void (*EventCheck)();
  int KillFlg;

 private:
  void wait_dsp(){};
  void lock_dsp(){ ++in_use_count; };
  void release_dsp(){ --in_use_count; };
  int dsp_cmd, dsp_usr, dsp_data;
  int in_use_count;

  PROBE_DATA_MODE prb_data_mode;
  SCAN_DATA_MODE scan_data_mode;
  long max_points_per_line;
};

/* SPM Hardware auf /dev/dspdev -- xafm.out running on dsp 
 * used by AFM, STM, SARLS
 */

class innovative_dsp_hwi_spm : public innovative_dsp_hwi_dev{
 public:
  innovative_dsp_hwi_spm();
  virtual ~innovative_dsp_hwi_spm();

  virtual void PutParameter(void *src, int grp=0); /* universelle Parameterübergabe */

  virtual void SetDxDy(doouble dx, double dy);
  virtual void SetOffset(double x, double y);
  virtual void SetNx(long nx);
  virtual void SetAlpha(double alpha);

  virtual void MovetoXY (double x, double y);
  virtual void StartScan2D();
  virtual void ScanLineM(int yindex, int xdir, int muxmode, Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0=0 );
  virtual void EndScan2D();
  virtual void PauseScan2D();
  virtual void ResumeScan2D();
  virtual void KillScan2D(){ KillFlg=TRUE; };

 private:
  DSP_Param dspPar;

  void DSP_FbWerte();
  void DSP_SetTransferFkt();
  void DSP_SetRotParam();
  void DSP_SetMoveParam();
  void DSP_SetAppWerte();
};

/* SPA-LEED Hardware auf /dev/dspdev -- spa.out running on dsp */

class innovative_dsp_hwi_spa : public innovative_dsp_hwi_dev{
 public:
  innovative_dsp_hwi_spa();
  virtual ~innovative_dsp_hwi_spa();

  virtual void StoreParameter(void);
  virtual void RestoreParameter(void);
  virtual void PutParameter(void *src, int grp=0); /* universelle Parameterübergabe */

  virtual long GetMaxPointsPerLine(){ return (long)(DSP_DATA_REG_LEN<<2); };
  virtual void SetDxDy(double dx, double dy);
  virtual void SetOffset(double x, double y);
  virtual void SetNx(long nx);
  virtual void SetAlpha(double alpha);

  virtual void MovetoXY(double x, double y);
  virtual void StartScan2D();
  virtual void ScanLineM(int yindex, int xdir, int muxmode, Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0=0 );
  virtual void EndScan2D();
  virtual void KillScan2D(){ KillFlg=TRUE; };

 private:
  DSP_Param dspPar;

  void DSP_SpaWerte(int flg=FALSE);
};

#endif

