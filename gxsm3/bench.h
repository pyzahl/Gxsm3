/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
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

/*@
 *@	File:   bench.h
 *@     $Header: /home/ventiotec/gxsm-cvs/Gxsm-2.0/src/bench.h,v 1.3 2003-07-29 20:01:03 zahl Exp $
 *@	Datum:	17.2.1999
 *@	Author:	P.Zahl  e-mail: zahl@dynamic.fkp.uni-hannover.de
 *@	Zweck:	for Benchmarks
 *@		Objekt stoppt die Zeit zwischen Konstruktor / start und stop
 *@             Ausgabe incl. Info. und Normierung auf NumOps über << operator
 *@
 *@	Funktionen:
 *@		
 *@		
 *@		
 *@	History:
 *@	=====================================================================
 *@	V1.00 Basisversion
 *@@	08.11.97 PZ: neu !
 *@@
 */

#ifndef __BENCH_H_
#define __BENCH_H_

#ifdef XSM_MATH_BENCHMARK
#include <iostream>
#include <string.h>

class bench{
public:
  bench(const char *I=NULL, unsigned long N=1L){
    dt = 0;
    count = 0;
    NumOps=N; 
    strcpy(Info, "XSM Benchmark: "); if(I) strcat(Info, I);
    t0 = g_get_monotonic_time ();
  }
  void start(){ dt=0.; t0 = g_get_monotonic_time (); };
  void resume(){ t0 = g_get_monotonic_time (); };
  void stop(){ t1 = g_get_monotonic_time (); count++; dt += t1-t0; }
  double getdt(){ return dt; };
  double getdt_normed(){ return dt/count; };
  void print () {
    g_message ("%s ==> t-exec: %ld us  #: %ld  Ops/s: %g",
	       Info,
	       dt/count,
	       count,
	       (double)NumOps*1e6/dt*count);
  };
private:
  gint64 t0;
  gint64 t1;
  gint64 dt;
  gint64 count;
  char Info[256];
  unsigned long NumOps;
};

#define BenchStart(B,I,N) bench B(I,N)
#define BenchStop(B)      B.stop(); B.print ()

#else /* Dummy */
#define BenchStart(B,I,N)
#define BenchStop(B)
#endif

#endif /* __BENCH_H */
