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

#ifndef __SURFACE_H
#define __SURFACE_H

#include "meldungen.h"
#include "xsmtypes.h"
#include "view.h"
#include "mem2d.h"
#include "xsmmath.h"
#include "scan.h"
#include "xsm.h"

#ifndef __XSMHARD_H
#include "xsmhard.h"
#endif

// moved to PI
//#include "xsm_mkicons.h"

/*
 * XSM Surface Scan Object Controller
 * Steuert Channelmechanismus
 * ============================================================
 */

typedef enum { MANUAL_SAVE_AS, AUTO_NAME_SAVE, AUTO_NAME_PARTIAL_SAVE, CHANGE_PATH_ONLY } AUTO_SAVE_MODE;

#define MAXSCANS (2*MAX_SRCS_CHANNELS)

class ProfileControl;
class Surface : public Xsm{
public:
  Surface ();
  virtual ~Surface();

  void hide();
  int draw();
  int load(const char *rname=NULL);
  int save(AUTO_SAVE_MODE automode, char *rname=NULL, int chidx=-1, int forceOverwrite=FALSE);
  int auto_append_in_time (double t);
  int gnuimport(const char *rname=NULL);
  int gnuexport(const char *rname=NULL);
  int  SetSDir(int Channel, int choice);
  int  SetView(int Channel, int choice);
  int  SetMode(int Channel, int choice, int force=FALSE);
  //  void SetRedraw(int flg=TRUE){ if(redrawflg=flg) SetVM(); };
  void SetRedraw(int flg=TRUE){ redrawflg=flg; };
  int  SetVM(int mode=0);
  Scan* NewScan(int vtype, int vflg, int ChNo, SCAN_DATA *vd);
  int  ActivateFreeChannel();
  int  ActivateChannel(int ActiveChannel);
  int  FindChan(int fid);

  void AutoDisplay(double hi=0., double lo=0.);

  void CleanupProfiles();
  int AddProfile(gchar *filename);
  int AddProfile(ProfileControl *pc);
  static void remove_profile(ProfileControl *pc, gpointer data);
  int RemoveProfile(ProfileControl *pc);
  void RemoveAllProfiles();

// moved to PI
//  void MkIcons(MkIconsData *mid);

  /* Surface Data Operations */
  void MathOperationNoDest(gboolean (*Op)(MATHOPPARAMSNODEST));
  void MathOperation(gboolean (*Op)(MATHOPPARAMS));
  void MathOperationS(gboolean (*Op)(MATHOPPARAMDONLY));
  void MathOperation_for_all_vt(gboolean (*Op)(MATHOPPARAMS));
  void MathOperationX(gboolean (*Op)(MATH2OPPARAMS), int IdScr2, gboolean size_matching=TRUE);

  void GetFromMem2d(Mem2d *m);

  Scan* GetActiveScan();
  Scan* GetMasterScan();
  void SetMasterScan(Scan *ms);

  /* Data */

  int  ActiveChannel;
  int  MasterChannel;
  int  ChannelMode[MAX_CHANNELS];
  int  ChannelScanMode[MAX_CHANNELS];
  int  ChannelView[MAX_CHANNELS];
  Scan *scan[MAX_CHANNELS];
  Scan *ActiveScan;
  Scan *MasterScan;

  GSList *ScanList;    /* List of Scan Objects */
  GSList *ProfileList; /* List of ProfileControl Objects */
  GSList *DelProfileList; /* List of ProfileControl Objects */

  int  StopScanFlg;

private:  
  int redrawflg;
};

#endif

