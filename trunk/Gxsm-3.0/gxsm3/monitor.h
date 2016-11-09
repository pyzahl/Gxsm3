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

#ifndef __MONITOR_H
#define __MONITOR_H

#include <fstream>

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <gtk/gtk.h>

#define MAXMONITORFIELDS 10

class Monitor{
public:
  Monitor(const gchar *name=NULL);
  virtual ~Monitor();

  virtual void run() {};
  virtual void stop() {};

  virtual void Messung(float val=0., gchar *txt=NULL);
  virtual void LogEvent(const gchar *Action, const gchar *Entry);

  void PutEvent(const gchar *Action, const gchar *Entry);

  gint Load(gchar *fname);
  gint Save(gchar *fname);

  gint AppLine();
  gint GetLine();

  void SetLogName(char *name);

  gchar *Fields[MAXMONITORFIELDS];
protected:
  double dt;
  char   *logname;
};

#endif




