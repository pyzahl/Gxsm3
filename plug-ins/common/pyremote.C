/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Plugin Name: pyremote.C
 * ========================================
 *
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Percy Zahl <zahl@fkp.uni-hannover.de>
 * additional features: Andreas Klust <klust@fkp.uni-hannover.de>
 * Large segments of code were borrowewd from gwyddion's
 * (http://gwyddion.net/) pygwy. These were originally authored by
 * David Necas (Yeti), Petr Klapetek and Jan Horak
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

/* Please do not change the Begin/End lines of this comment section!
 * this is a LaTeX style section used for auto generation of the PlugIn Manual
 * Chapter. Add a complete PlugIn documentation inbetween the Begin/End marks!
 * All "% PlugInXXX" commentary tags are mandatory
 * All "% OptPlugInXXX" tags are optional
 * --------------------------------------------------------------------------------
% BeginPlugInDocuSection
% PlugInDocuCaption: Python remote control
% PlugInName: pyremote
% PlugInAuthor: Stefan Schr\"oder
% PlugInAuthorEmail: stefan_fkp@users.sf.net
% PlugInMenuPath: Tools/Pyremote Console

% PlugInDescription
 This plugin is an interface to an embedded Python
 Interpreter.

% PlugInUsage
 Choose Pyremote Console from the \GxsmMenu{Tools} menu to enter a
 python console. A default command script is loaded when the console
 is started for the first time, but is not executed automatically. In
 the appendix you will find a tutorial with examples and tips and
 tricks.

 Python Venv is supported if setup:
\begin{alltt}
 Create using: $ python3 -m venv .gxsm3/pyaction/GxsmPythonVenv
\end{alltt}


% OptPlugInSection: Reference

The following script shows you all commands that the gxsm-module supports:

\begin{alltt}
# list core gxsm module functions:
print dir(gxsm)

# list buildin help on functions:
for h in gxsm.help ():
	print (h)

# list all by gxsm known entry (set/get) reference names:
for h in gxsm.list_refnames ():
	print ('{} \t=>\t {}'.format(h, gxsm.get(h)))

# list all action hooks to activate function
# via a script triggered button press
# or call plugin hook enabled math plugins:
for h in gxsm.list_actions ():
	print (h)
\end{alltt}

The result will look like this (with added notes):

\begin{alltt}

** gxsm python module:
['__doc__', '__loader__', '__name__', '__package__', '__spec__', 'action', 'add_layerinformation',
'add_marker_object', 'autodisplay', 'autosave', 'autoupdate',
'chfname', 'chmodea', 'chmodem', 'chmoden', 'chmodeno', 'chmodex', 'chview1d', 'chview2d', 'chview3d',
'createscan', 'createscanf', 'crop', 'da0', 'direct', 'echo', 'export', 'get', 'get_data_pkt',
'get_differentials', 'get_dimensions', 'get_geometry', 'get_object', 'get_slice',
'get_v_lookup', 'get_x_lookup', 'get_y_lookup',
'gets', 'help', 'import', 'list_actions', 'list_refnames', 'load', 'log', 'logev',
'moveto_scan_xy', 'progress', 'put_data_pkt', 'quick', 'rtquery', 'save', 'save_drawing', 'saveas',
'scaninit', 'scanline', 'scanupdate', 'scanylookup', 'set', 'set_scan_lookup', 'set_scan_unit',
'set_v_lookup', 'set_view_indices', 'set_x_lookup', 'set_y_lookup', 'signal_emit', 'sleep',
'startscan', 'stopscan', 'unitbz', 'unitev', 'units', 'unitvolt', 'waitscan', 'y_current']

***
(1) Gxsm3 python remote console -- gxsm.help on build in commands
 The following list shows a brief explanation of the commands, together with
 the signature (that is the type of arguments).

 '()' equals no argument. E.g. \verb+startscan()+

 '(N)' equals one Integer arument. E.g. \verb+chview1d(2)+

 '(X)' equals one Float argument. No example.

 '(S)' equals a string. Often numbers are evaluated as strings first. Like in \verb+set("RangeX", "234.12")+

 '(S,N)' equals two parameters. E.g. \verb+gnuexport("myfilename.nc", 1)+

--------------------------------------------------------------------------------
gxsm.help : List Gxsm methods: print gxsm.help ()
gxsm.set : Set Gxsm entry value, see list_refnames: gxsm.set ('refname','value as string')
gxsm.get : Get Gxsm entry as value, see list_refnames. gxsm.get ('refname')
gxsm.gets : Get Gxsm entry as string. gxsm.gets ('refname')
gxsm.list_refnames : List all available Gxsm entry refnames (Better: pointer hover over Gxsm-Entry
                     to see tooltip with ref-name). print gxsm.list_refnames ()
gxsm.action : Trigger Gxsm action (menu action or button signal), see list_actions: gxsm.action('action')
gxsm.list_actions : List all available Gxsm actions (Better: pointer hover over Gxsm-Button
                    to see tooltip with action-name): print gxsm.list_actions ()
gxsm.rtquery : Gxsm hardware Real-Time-Query: svec[3] = gxsm.rtquery('X|Y|Z|xy|zxy|o|f|s|i|U')
gxsm.y_current : RTQuery Current Scanline.
gxsm.moveto_scan_xy : Set tip position to Scan-XY: gxsm.moveto_scan_xy (x,y)
gxsm.createscan : Create Scan int: gxsm.createscan (ch,nx,ny,nv pixels, rx,ry in A, array.array('l', [...]), append)
gxsm.createscanf : Create Scan float: gxsm.createscan (ch,nx,ny,nv pixels, rx,ry in A, array.array('f', [...]), append)
gxsm.set_scan_unit : Set Scan X,Y,Z,L Dim Unit: gxsm.set_scan_unit (ch,'X|Y|Z|L|T','UnitId string','Label string')
gxsm.set_scan_lookup : Set Scan Lookup for Dim: gxsm.set_scan_lookup (ch,'X|Y|L',start,end)
gxsm.get_geometry : Get Scan Geometry: [rx,ry,x0,y0,alpha]=gxsm.get_geometry (ch)
gxsm.get_differentials : Get Scan Scaling: [dx,dy,dz,dl]=gxsm.get_differentials (ch)
gxsm.get_dimensions : Get Scan Dimensions: [nx,ny,nv,nt]=gxsm.get_dimensions (ch)
gxsm.get_data_pkt : Get Data Value at point: value=gxsm.get_data_pkt (ch, x, y, v, t)
gxsm.put_data_pkt : Put Data Value to point: gxsm.put_data_pkt (value, ch, x, y, v, t)
gxsm.get_slice : Get Data Slice from Scan Imagein ch, values are scaled by dz to unit: [nx,ny,array]=gxsm.get_slice (ch, v, t, yi, yn)
gxsm.get_x_lookup : Get Scan Data index to world mapping: x=gxsm.get_x_lookup (ch, i)
gxsm.get_y_lookup : Get Scan Data index to world mapping: y=gxsm.get_y_lookup (ch, i)
gxsm.get_v_lookup : Get Scan Data index to world mapping: v=gxsm.get_v_lookup (ch, i)
gxsm.set_x_lookup : Set Scan Data index to world mapping: x=gxsm.get_x_lookup (ch, i, v)
gxsm.set_y_lookup : Set Scan Data index to world mapping: y=gxsm.get_y_lookup (ch, i, v)
gxsm.set_v_lookup : Set Scan Data index to world mapping: v=gxsm.get_v_lookup (ch, i, v)
gxsm.get_object : Get Object Coordinates: [type, x,y,..]=gxsm.get_object (ch, n)
gxsm.add_marker_object : Put Marker Object at pixel coordinates or current tip pos (id='xy'|grp=-1):
                          gxsm.add_marker_object (ch, label=str|'xy', mgrp=0..5|-1, x=ix,y=iy, size=0..1)
gxsm.startscan : Start Scan.
gxsm.stopscan : Stop Scan.
gxsm.waitscan : Wait Scan. ret=gxsm.waitscan(blocking=true). ret=-1: no scan in progress, else current line index.
gxsm.scaninit : Scaninit.
gxsm.scanupdate : Scanupdate.
gxsm.scanylookup : Scanylookup.
gxsm.scanline : Scan line.
gxsm.autosave : Save: Auto Save Scans. gxsm.autosave (). Returns current scanline y index and file name(s) if scanning.
gxsm.autoupdate : Save: Auto Update Scans. gxsm.autoupdate (). Returns current scanline y index and file name(s) if scanning.
gxsm.save : Save: Auto Save Scans: gxsm.save ()
gxsm.saveas : Save File As: gxsm.saveas (ch, 'path/fname.nc')
gxsm.load : Load File: gxsm.load (ch, 'path/fname.nc')
gxsm.export : Export scan: gxsm.export (ch, 'path/fname.nc')
gxsm.import : Import scan: gxsm.import (ch, 'path/fname.nc')
gxsm.save_drawing : Save Drawing to file: gxsm.save_drawing (ch, time, layer, 'path/fname.png|pdf|svg')
gxsm.set_view_indices : Set Ch view time and layer indices: gxsm.set_view_indices (ch, time, layer)
gxsm.autodisplay : Autodisplay active channel: gxsm.autodisplay ()
gxsm.chfname : Get Ch Filename: filename = gxsm.chfname (ch)
gxsm.chmodea : Set Ch Mode to A: gxsm.chmodea (ch)
gxsm.chmodex : Set Ch Mode to X: gxsm.chmodex (ch)
gxsm.chmodem : Set Ch Mode to MATH: gxsm.chmodem (ch)
gxsm.chmoden : Set Ch Mode to Data Channel <X+N>: gxsm.chmoden (ch,n)
gxsm.chmodeno : Set View Mode to No: gxsm.chmodeno (ch)
gxsm.chview1d : Set View Mode to 1D: gxsm.chmode1d (ch)
gxsm.chview2d : Set View Mode to 2D: gxsm.chmode2d (ch)
gxsm.chview3d : Set View Mode to 3D: gxsm.chmode3d (ch)
gxsm.quick : Quick.
gxsm.direct : Direct.
gxsm.log : Log.
gxsm.crop : Crop (ch-src, ch-dst)
gxsm.unitbz : UnitBZ.
gxsm.unitvolt : UnitVolt.
gxsm.unitev : UniteV.
gxsm.units : UnitS.
gxsm.echo : Echo string to terminal. gxsm.echo('hello gxsm to terminal')
gxsm.logev : Write string to Gxsm system log file and log monitor: gxsm.logev ('hello gxsm to logfile/monitor')
gxsm.progress : Show/update gxsm progress info. fraction<0 init, 0..1 progress, >1 close: gxsm.progress ('Calculating...', fraction)
gxsm.add_layerinformation : Add Layerinformation to active scan. gxsm.add_layerinformation('Text',ch)
gxsm.da0 : Da0. -- N/A for SRanger
gxsm.signal_emit : Action-String.
gxsm.sleep : Sleep N/10s: gxsm.sleep (N)
*
--------------------------------------------------------------------------------
(2) Gxsm3 python remote console -- help on reference names
  used for gxsm.set and get, gets commands.
  Hint: hover the pointer over any get/set enabled Gxsm entry to see it`s ref-name!
  Example: gxsm.set ("dsp-fbs-bias", "1.0")
--------------------------------------------------------------------------------
script-control 	=>	 0.0
TimeSelect 	=>	 0.0
Time 	=>	 1.0
LayerSelect 	=>	 0.0
Layers 	=>	 1.0
Rotation 	=>	 0.0
ScanY 	=>	 0.0
ScanX 	=>	 0.0
OffsetY 	=>	 0.0
OffsetX 	=>	 0.0
PointsY 	=>	 400.0
PointsX 	=>	 400.0
StepsY 	=>	 2.0050125313283207
StepsX 	=>	 2.0050125313283207
RangeY 	=>	 800.0
RangeX 	=>	 800.0
dsp-pac-ph-bw-set 	=>	 8000.0
dsp-pac-ph-ci-gain 	=>	 -127.2
dsp-pac-ph-cp-gain 	=>	 -61.6
dsp-pac-ph-set 	=>	 -100.0
dsp-pac-res-gain 	=>	 1.0
dsp-pac-res-q-factor 	=>	 30000.0
dsp-pac-am-bw-set 	=>	 8.0
dsp-pac-am-ci-gain 	=>	 -77.5
dsp-pac-am-cp-gain 	=>	 8.1
dsp-pac-am-set 	=>	 0.1
dsp-pac-excitation-sine-freq 	=>	 32766.4
dsp-pac-excitation-sine-amp 	=>	 0.5
dsp-pac-tau 	=>	 1.5e-05
dsp-pac-res-amp-max 	=>	 5.0
dsp-pac-res-amp-min 	=>	 0.0
dsp-pac-res-amp-range 	=>	 1.25
dsp-pac-res-amp-ref 	=>	 0.0
dsp-pac-exci-amp-max 	=>	 1.0
dsp-pac-exci-amp-min 	=>	 -0.05
dsp-pac-exci-amp-range 	=>	 0.625
dsp-pac-exci-amp-ref 	=>	 0.0
dsp-pac-res-phase-max 	=>	 180.0
dsp-pac-res-phase-min 	=>	 -180.0
dsp-pac-res-phase-range 	=>	 7.16
dsp-pac-res-phase-ref 	=>	 0.0
dsp-pac-exci-freq-max 	=>	 41000.0
dsp-pac-exci-freq-min 	=>	 29000.0
dsp-pac-exci-freq-range 	=>	 187.0
dsp-pac-exci-freq-ref 	=>	 0.0
dsp-VP-Lim-Val-Dn 	=>	 1.0
dsp-VP-Lim-Val-Up 	=>	 1.0
dsp-X-Final-Delay 	=>	 0.01
dsp-AX-GateTime 	=>	 0.001
dsp-AX-Final-Delay 	=>	 0.01
dsp-AX-Slope-Ramp 	=>	 100.0
dsp-AX-V-Slope 	=>	 100.0
dsp-AX-rep 	=>	 1.0
dsp-AX-Points 	=>	 100.0
dsp-AX-V-End 	=>	 1.0
dsp-AX-V-Start 	=>	 0.0
dsp-LCK-AC-Repetitions 	=>	 1.0
dsp-LCK-AC-Final-Delay 	=>	 0.01
dsp-LCK-AC-Slope 	=>	 12.0
dsp-LCK-AC-Points 	=>	 720.0
dsp-ALCK-C-Phase-Span 	=>	 360.0
dsp-LCK-AC-avg-Cycles 	=>	 32.0
dsp-LCK-AC-Phase-B 	=>	 90.0
dsp-LCK-AC-Phase-A 	=>	 0.0
dsp-LCK-AC-Frequency 	=>	 1171.88
dsp-LCK-AC-Z-Amp 	=>	 0.0
dsp-LCK-AC-Bias-Amp 	=>	 0.02
dsp-LCK-CORRSUM-SHR 	=>	 0.0
dsp-LCK-CORRPRD-SHR 	=>	 14.0
dsp-Noise-Amplitude 	=>	 0.0
dsp-TK-Delay 	=>	 1.0
dsp-TK-Speed 	=>	 1000.0
dsp-TK-Mode 	=>	 -1.0
dsp-TK-Reps 	=>	 100.0
dsp-TK-Nodes 	=>	 12.0
dsp-TK-Points 	=>	 10.0
dsp-TK-rad2 	=>	 0.0
dsp-TK-rad 	=>	 2.0
dsp-GVP-GPIO-Lock-57 	=>	 57.0
dsp-GVP-Final-Delay 	=>	 0.01
dsp-gvp-pcjr44 	=>	 0.0
dsp-gvp-nrep44 	=>	 0.0
dsp-gvp-data44 	=>	 0.0
dsp-gvp-n44 	=>	 0.0
dsp-gvp-dt44 	=>	 0.0
dsp-gvp-dsig44 	=>	 0.0
dsp-gvp-dz44 	=>	 0.0
dsp-gvp-dy44 	=>	 0.0
dsp-gvp-dx44 	=>	 0.0
dsp-gvp-du44 	=>	 0.0
dsp-gvp-pcjr43 	=>	 0.0
dsp-gvp-nrep43 	=>	 0.0
dsp-gvp-data43 	=>	 0.0
dsp-gvp-n43 	=>	 0.0
dsp-gvp-dt43 	=>	 0.0
dsp-gvp-dsig43 	=>	 0.0
dsp-gvp-dz43 	=>	 0.0
dsp-gvp-dy43 	=>	 0.0
dsp-gvp-dx43 	=>	 0.0
dsp-gvp-du43 	=>	 0.0
dsp-gvp-pcjr42 	=>	 0.0
dsp-gvp-nrep42 	=>	 0.0
dsp-gvp-data42 	=>	 0.0
dsp-gvp-n42 	=>	 0.0
dsp-gvp-dt42 	=>	 0.0
dsp-gvp-dsig42 	=>	 0.0
dsp-gvp-dz42 	=>	 0.0
dsp-gvp-dy42 	=>	 0.0
dsp-gvp-dx42 	=>	 0.0
dsp-gvp-du42 	=>	 0.0
dsp-gvp-pcjr41 	=>	 0.0
dsp-gvp-nrep41 	=>	 0.0
dsp-gvp-data41 	=>	 0.0
dsp-gvp-n41 	=>	 0.0
dsp-gvp-dt41 	=>	 0.0
dsp-gvp-dsig41 	=>	 0.0
dsp-gvp-dz41 	=>	 0.0
dsp-gvp-dy41 	=>	 0.0
dsp-gvp-dx41 	=>	 0.0
dsp-gvp-du41 	=>	 0.0
dsp-gvp-pcjr40 	=>	 0.0
dsp-gvp-nrep40 	=>	 0.0
dsp-gvp-data40 	=>	 0.0
dsp-gvp-n40 	=>	 0.0
dsp-gvp-dt40 	=>	 0.0
dsp-gvp-dsig40 	=>	 0.0
dsp-gvp-dz40 	=>	 0.0
dsp-gvp-dy40 	=>	 0.0
dsp-gvp-dx40 	=>	 0.0
dsp-gvp-du40 	=>	 0.0
dsp-gvp-pcjr39 	=>	 0.0
dsp-gvp-nrep39 	=>	 0.0
dsp-gvp-data39 	=>	 0.0
dsp-gvp-n39 	=>	 0.0
dsp-gvp-dt39 	=>	 0.0
dsp-gvp-dsig39 	=>	 0.0
dsp-gvp-dz39 	=>	 0.0
dsp-gvp-dy39 	=>	 0.0
dsp-gvp-dx39 	=>	 0.0
dsp-gvp-du39 	=>	 0.0
dsp-gvp-pcjr38 	=>	 0.0
dsp-gvp-nrep38 	=>	 0.0
dsp-gvp-data38 	=>	 0.0
dsp-gvp-n38 	=>	 0.0
dsp-gvp-dt38 	=>	 0.0
dsp-gvp-dsig38 	=>	 0.0
dsp-gvp-dz38 	=>	 0.0
dsp-gvp-dy38 	=>	 0.0
dsp-gvp-dx38 	=>	 0.0
dsp-gvp-du38 	=>	 0.0
dsp-gvp-pcjr37 	=>	 0.0
dsp-gvp-nrep37 	=>	 0.0
dsp-gvp-data37 	=>	 0.0
dsp-gvp-n37 	=>	 0.0
dsp-gvp-dt37 	=>	 0.0
dsp-gvp-dsig37 	=>	 0.0
dsp-gvp-dz37 	=>	 0.0
dsp-gvp-dy37 	=>	 0.0
dsp-gvp-dx37 	=>	 0.0
dsp-gvp-du37 	=>	 0.0
dsp-gvp-pcjr36 	=>	 0.0
dsp-gvp-nrep36 	=>	 0.0
dsp-gvp-data36 	=>	 0.0
dsp-gvp-n36 	=>	 0.0
dsp-gvp-dt36 	=>	 0.0
dsp-gvp-dsig36 	=>	 0.0
dsp-gvp-dz36 	=>	 0.0
dsp-gvp-dy36 	=>	 0.0
dsp-gvp-dx36 	=>	 0.0
dsp-gvp-du36 	=>	 0.0
dsp-gvp-pcjr35 	=>	 0.0
dsp-gvp-nrep35 	=>	 0.0
dsp-gvp-data35 	=>	 0.0
dsp-gvp-n35 	=>	 0.0
dsp-gvp-dt35 	=>	 0.0
dsp-gvp-dsig35 	=>	 0.0
dsp-gvp-dz35 	=>	 0.0
dsp-gvp-dy35 	=>	 0.0
dsp-gvp-dx35 	=>	 0.0
dsp-gvp-du35 	=>	 0.0
dsp-gvp-pcjr34 	=>	 0.0
dsp-gvp-nrep34 	=>	 0.0
dsp-gvp-data34 	=>	 0.0
dsp-gvp-n34 	=>	 0.0
dsp-gvp-dt34 	=>	 0.0
dsp-gvp-dsig34 	=>	 0.0
dsp-gvp-dz34 	=>	 0.0
dsp-gvp-dy34 	=>	 0.0
dsp-gvp-dx34 	=>	 0.0
dsp-gvp-du34 	=>	 0.0
dsp-gvp-pcjr33 	=>	 0.0
dsp-gvp-nrep33 	=>	 0.0
dsp-gvp-data33 	=>	 0.0
dsp-gvp-n33 	=>	 0.0
dsp-gvp-dt33 	=>	 0.0
dsp-gvp-dsig33 	=>	 0.0
dsp-gvp-dz33 	=>	 0.0
dsp-gvp-dy33 	=>	 0.0
dsp-gvp-dx33 	=>	 0.0
dsp-gvp-du33 	=>	 0.0
dsp-gvp-pcjr32 	=>	 0.0
dsp-gvp-nrep32 	=>	 0.0
dsp-gvp-data32 	=>	 0.0
dsp-gvp-n32 	=>	 0.0
dsp-gvp-dt32 	=>	 0.0
dsp-gvp-dsig32 	=>	 0.0
dsp-gvp-dz32 	=>	 0.0
dsp-gvp-dy32 	=>	 0.0
dsp-gvp-dx32 	=>	 0.0
dsp-gvp-du32 	=>	 0.0
dsp-gvp-pcjr31 	=>	 0.0
dsp-gvp-nrep31 	=>	 0.0
dsp-gvp-data31 	=>	 0.0
dsp-gvp-n31 	=>	 0.0
dsp-gvp-dt31 	=>	 0.0
dsp-gvp-dsig31 	=>	 0.0
dsp-gvp-dz31 	=>	 0.0
dsp-gvp-dy31 	=>	 0.0
dsp-gvp-dx31 	=>	 0.0
dsp-gvp-du31 	=>	 0.0
dsp-gvp-pcjr30 	=>	 0.0
dsp-gvp-nrep30 	=>	 0.0
dsp-gvp-data30 	=>	 0.0
dsp-gvp-n30 	=>	 0.0
dsp-gvp-dt30 	=>	 0.0
dsp-gvp-dsig30 	=>	 0.0
dsp-gvp-dz30 	=>	 0.0
dsp-gvp-dy30 	=>	 0.0
dsp-gvp-dx30 	=>	 0.0
dsp-gvp-du30 	=>	 0.0
dsp-gvp-pcjr29 	=>	 0.0
dsp-gvp-nrep29 	=>	 0.0
dsp-gvp-data29 	=>	 0.0
dsp-gvp-n29 	=>	 0.0
dsp-gvp-dt29 	=>	 0.0
dsp-gvp-dsig29 	=>	 0.0
dsp-gvp-dz29 	=>	 0.0
dsp-gvp-dy29 	=>	 0.0
dsp-gvp-dx29 	=>	 0.0
dsp-gvp-du29 	=>	 0.0
dsp-gvp-pcjr28 	=>	 0.0
dsp-gvp-nrep28 	=>	 0.0
dsp-gvp-data28 	=>	 0.0
dsp-gvp-n28 	=>	 0.0
dsp-gvp-dt28 	=>	 0.0
dsp-gvp-dsig28 	=>	 0.0
dsp-gvp-dz28 	=>	 0.0
dsp-gvp-dy28 	=>	 0.0
dsp-gvp-dx28 	=>	 0.0
dsp-gvp-du28 	=>	 0.0
dsp-gvp-pcjr27 	=>	 0.0
dsp-gvp-nrep27 	=>	 0.0
dsp-gvp-data27 	=>	 0.0
dsp-gvp-n27 	=>	 0.0
dsp-gvp-dt27 	=>	 0.0
dsp-gvp-dsig27 	=>	 0.0
dsp-gvp-dz27 	=>	 0.0
dsp-gvp-dy27 	=>	 0.0
dsp-gvp-dx27 	=>	 0.0
dsp-gvp-du27 	=>	 0.0
dsp-gvp-pcjr26 	=>	 0.0
dsp-gvp-nrep26 	=>	 0.0
dsp-gvp-data26 	=>	 0.0
dsp-gvp-n26 	=>	 0.0
dsp-gvp-dt26 	=>	 0.0
dsp-gvp-dsig26 	=>	 0.0
dsp-gvp-dz26 	=>	 0.0
dsp-gvp-dy26 	=>	 0.0
dsp-gvp-dx26 	=>	 0.0
dsp-gvp-du26 	=>	 0.0
dsp-gvp-pcjr25 	=>	 0.0
dsp-gvp-nrep25 	=>	 0.0
dsp-gvp-data25 	=>	 0.0
dsp-gvp-n25 	=>	 0.0
dsp-gvp-dt25 	=>	 0.0
dsp-gvp-dsig25 	=>	 0.0
dsp-gvp-dz25 	=>	 0.0
dsp-gvp-dy25 	=>	 0.0
dsp-gvp-dx25 	=>	 0.0
dsp-gvp-du25 	=>	 0.0
dsp-gvp-pcjr24 	=>	 0.0
dsp-gvp-nrep24 	=>	 0.0
dsp-gvp-data24 	=>	 0.0
dsp-gvp-n24 	=>	 0.0
dsp-gvp-dt24 	=>	 0.0
dsp-gvp-dsig24 	=>	 0.0
dsp-gvp-dz24 	=>	 0.0
dsp-gvp-dy24 	=>	 0.0
dsp-gvp-dx24 	=>	 0.0
dsp-gvp-du24 	=>	 0.0
dsp-gvp-pcjr23 	=>	 0.0
dsp-gvp-nrep23 	=>	 0.0
dsp-gvp-data23 	=>	 0.0
dsp-gvp-n23 	=>	 0.0
dsp-gvp-dt23 	=>	 0.0
dsp-gvp-dsig23 	=>	 0.0
dsp-gvp-dz23 	=>	 0.0
dsp-gvp-dy23 	=>	 0.0
dsp-gvp-dx23 	=>	 0.0
dsp-gvp-du23 	=>	 0.0
dsp-gvp-pcjr22 	=>	 0.0
dsp-gvp-nrep22 	=>	 0.0
dsp-gvp-data22 	=>	 0.0
dsp-gvp-n22 	=>	 0.0
dsp-gvp-dt22 	=>	 0.0
dsp-gvp-dsig22 	=>	 0.0
dsp-gvp-dz22 	=>	 0.0
dsp-gvp-dy22 	=>	 0.0
dsp-gvp-dx22 	=>	 0.0
dsp-gvp-du22 	=>	 0.0
dsp-gvp-pcjr21 	=>	 0.0
dsp-gvp-nrep21 	=>	 0.0
dsp-gvp-data21 	=>	 0.0
dsp-gvp-n21 	=>	 0.0
dsp-gvp-dt21 	=>	 0.0
dsp-gvp-dsig21 	=>	 0.0
dsp-gvp-dz21 	=>	 0.0
dsp-gvp-dy21 	=>	 0.0
dsp-gvp-dx21 	=>	 0.0
dsp-gvp-du21 	=>	 0.0
dsp-gvp-pcjr20 	=>	 0.0
dsp-gvp-nrep20 	=>	 0.0
dsp-gvp-data20 	=>	 0.0
dsp-gvp-n20 	=>	 0.0
dsp-gvp-dt20 	=>	 0.0
dsp-gvp-dsig20 	=>	 0.0
dsp-gvp-dz20 	=>	 0.0
dsp-gvp-dy20 	=>	 0.0
dsp-gvp-dx20 	=>	 0.0
dsp-gvp-du20 	=>	 0.0
dsp-gvp-pcjr19 	=>	 0.0
dsp-gvp-nrep19 	=>	 0.0
dsp-gvp-data19 	=>	 0.0
dsp-gvp-n19 	=>	 0.0
dsp-gvp-dt19 	=>	 0.0
dsp-gvp-dsig19 	=>	 0.0
dsp-gvp-dz19 	=>	 0.0
dsp-gvp-dy19 	=>	 0.0
dsp-gvp-dx19 	=>	 0.0
dsp-gvp-du19 	=>	 0.0
dsp-gvp-pcjr18 	=>	 0.0
dsp-gvp-nrep18 	=>	 0.0
dsp-gvp-data18 	=>	 0.0
dsp-gvp-n18 	=>	 0.0
dsp-gvp-dt18 	=>	 0.0
dsp-gvp-dsig18 	=>	 0.0
dsp-gvp-dz18 	=>	 0.0
dsp-gvp-dy18 	=>	 0.0
dsp-gvp-dx18 	=>	 0.0
dsp-gvp-du18 	=>	 0.0
dsp-gvp-pcjr17 	=>	 0.0
dsp-gvp-nrep17 	=>	 0.0
dsp-gvp-data17 	=>	 0.0
dsp-gvp-n17 	=>	 0.0
dsp-gvp-dt17 	=>	 0.0
dsp-gvp-dsig17 	=>	 0.0
dsp-gvp-dz17 	=>	 0.0
dsp-gvp-dy17 	=>	 0.0
dsp-gvp-dx17 	=>	 0.0
dsp-gvp-du17 	=>	 0.0
dsp-gvp-pcjr16 	=>	 0.0
dsp-gvp-nrep16 	=>	 0.0
dsp-gvp-data16 	=>	 0.0
dsp-gvp-n16 	=>	 0.0
dsp-gvp-dt16 	=>	 0.0
dsp-gvp-dsig16 	=>	 0.0
dsp-gvp-dz16 	=>	 0.0
dsp-gvp-dy16 	=>	 0.0
dsp-gvp-dx16 	=>	 0.0
dsp-gvp-du16 	=>	 0.0
dsp-gvp-pcjr15 	=>	 0.0
dsp-gvp-nrep15 	=>	 0.0
dsp-gvp-data15 	=>	 0.0
dsp-gvp-n15 	=>	 0.0
dsp-gvp-dt15 	=>	 0.0
dsp-gvp-dsig15 	=>	 0.0
dsp-gvp-dz15 	=>	 0.0
dsp-gvp-dy15 	=>	 0.0
dsp-gvp-dx15 	=>	 0.0
dsp-gvp-du15 	=>	 0.0
dsp-gvp-pcjr14 	=>	 0.0
dsp-gvp-nrep14 	=>	 0.0
dsp-gvp-data14 	=>	 0.0
dsp-gvp-n14 	=>	 0.0
dsp-gvp-dt14 	=>	 0.0
dsp-gvp-dsig14 	=>	 0.0
dsp-gvp-dz14 	=>	 0.0
dsp-gvp-dy14 	=>	 0.0
dsp-gvp-dx14 	=>	 0.0
dsp-gvp-du14 	=>	 0.0
dsp-gvp-pcjr13 	=>	 0.0
dsp-gvp-nrep13 	=>	 0.0
dsp-gvp-data13 	=>	 0.0
dsp-gvp-n13 	=>	 0.0
dsp-gvp-dt13 	=>	 0.0
dsp-gvp-dsig13 	=>	 0.0
dsp-gvp-dz13 	=>	 0.0
dsp-gvp-dy13 	=>	 0.0
dsp-gvp-dx13 	=>	 0.0
dsp-gvp-du13 	=>	 0.0
dsp-gvp-pcjr12 	=>	 0.0
dsp-gvp-nrep12 	=>	 0.0
dsp-gvp-data12 	=>	 0.0
dsp-gvp-n12 	=>	 0.0
dsp-gvp-dt12 	=>	 0.0
dsp-gvp-dsig12 	=>	 0.0
dsp-gvp-dz12 	=>	 0.0
dsp-gvp-dy12 	=>	 0.0
dsp-gvp-dx12 	=>	 0.0
dsp-gvp-du12 	=>	 0.0
dsp-gvp-pcjr11 	=>	 0.0
dsp-gvp-nrep11 	=>	 0.0
dsp-gvp-data11 	=>	 0.0
dsp-gvp-n11 	=>	 0.0
dsp-gvp-dt11 	=>	 0.0
dsp-gvp-dsig11 	=>	 0.0
dsp-gvp-dz11 	=>	 0.0
dsp-gvp-dy11 	=>	 0.0
dsp-gvp-dx11 	=>	 0.0
dsp-gvp-du11 	=>	 0.0
dsp-gvp-pcjr10 	=>	 0.0
dsp-gvp-nrep10 	=>	 0.0
dsp-gvp-data10 	=>	 0.0
dsp-gvp-n10 	=>	 0.0
dsp-gvp-dt10 	=>	 0.0
dsp-gvp-dsig10 	=>	 0.0
dsp-gvp-dz10 	=>	 0.0
dsp-gvp-dy10 	=>	 0.0
dsp-gvp-dx10 	=>	 0.0
dsp-gvp-du10 	=>	 0.0
dsp-gvp-pcjr09 	=>	 0.0
dsp-gvp-nrep09 	=>	 0.0
dsp-gvp-data09 	=>	 0.0
dsp-gvp-n09 	=>	 0.0
dsp-gvp-dt09 	=>	 0.0
dsp-gvp-dsig09 	=>	 0.0
dsp-gvp-dz09 	=>	 0.0
dsp-gvp-dy09 	=>	 0.0
dsp-gvp-dx09 	=>	 0.0
dsp-gvp-du09 	=>	 0.0
dsp-gvp-pcjr08 	=>	 0.0
dsp-gvp-nrep08 	=>	 0.0
dsp-gvp-data08 	=>	 0.0
dsp-gvp-n08 	=>	 0.0
dsp-gvp-dt08 	=>	 0.0
dsp-gvp-dsig08 	=>	 0.0
dsp-gvp-dz08 	=>	 0.0
dsp-gvp-dy08 	=>	 0.0
dsp-gvp-dx08 	=>	 0.0
dsp-gvp-du08 	=>	 0.0
dsp-gvp-pcjr07 	=>	 0.0
dsp-gvp-nrep07 	=>	 0.0
dsp-gvp-data07 	=>	 0.0
dsp-gvp-n07 	=>	 0.0
dsp-gvp-dt07 	=>	 0.0
dsp-gvp-dsig07 	=>	 0.0
dsp-gvp-dz07 	=>	 0.0
dsp-gvp-dy07 	=>	 0.0
dsp-gvp-dx07 	=>	 0.0
dsp-gvp-du07 	=>	 0.0
dsp-gvp-pcjr06 	=>	 0.0
dsp-gvp-nrep06 	=>	 0.0
dsp-gvp-data06 	=>	 0.0
dsp-gvp-n06 	=>	 0.0
dsp-gvp-dt06 	=>	 0.0
dsp-gvp-dsig06 	=>	 0.0
dsp-gvp-dz06 	=>	 0.0
dsp-gvp-dy06 	=>	 0.0
dsp-gvp-dx06 	=>	 0.0
dsp-gvp-du06 	=>	 0.0
dsp-gvp-pcjr05 	=>	 0.0
dsp-gvp-nrep05 	=>	 0.0
dsp-gvp-data05 	=>	 0.0
dsp-gvp-n05 	=>	 0.0
dsp-gvp-dt05 	=>	 0.0
dsp-gvp-dsig05 	=>	 0.0
dsp-gvp-dz05 	=>	 0.0
dsp-gvp-dy05 	=>	 0.0
dsp-gvp-dx05 	=>	 0.0
dsp-gvp-du05 	=>	 0.0
dsp-gvp-pcjr04 	=>	 0.0
dsp-gvp-nrep04 	=>	 0.0
dsp-gvp-data04 	=>	 0.0
dsp-gvp-n04 	=>	 0.0
dsp-gvp-dt04 	=>	 0.0
dsp-gvp-dsig04 	=>	 0.0
dsp-gvp-dz04 	=>	 0.0
dsp-gvp-dy04 	=>	 0.0
dsp-gvp-dx04 	=>	 0.0
dsp-gvp-du04 	=>	 0.0
dsp-gvp-pcjr03 	=>	 0.0
dsp-gvp-nrep03 	=>	 1.0
dsp-gvp-data03 	=>	 0.0
dsp-gvp-n03 	=>	 0.0
dsp-gvp-dt03 	=>	 0.0
dsp-gvp-dsig03 	=>	 0.0
dsp-gvp-dz03 	=>	 0.0
dsp-gvp-dy03 	=>	 0.0
dsp-gvp-dx03 	=>	 0.0
dsp-gvp-du03 	=>	 0.0
dsp-gvp-pcjr02 	=>	 0.0
dsp-gvp-nrep02 	=>	 1.0
dsp-gvp-data02 	=>	 2.0
dsp-gvp-n02 	=>	 100.0
dsp-gvp-dt02 	=>	 0.2
dsp-gvp-dsig02 	=>	 0.0
dsp-gvp-dz02 	=>	 -100.0
dsp-gvp-dy02 	=>	 0.0
dsp-gvp-dx02 	=>	 0.0
dsp-gvp-du02 	=>	 -1.0
dsp-gvp-pcjr01 	=>	 0.0
dsp-gvp-nrep01 	=>	 1.0
dsp-gvp-data01 	=>	 0.0
dsp-gvp-n01 	=>	 2000.0
dsp-gvp-dt01 	=>	 1.0
dsp-gvp-dsig01 	=>	 0.0
dsp-gvp-dz01 	=>	 0.0
dsp-gvp-dy01 	=>	 0.0
dsp-gvp-dx01 	=>	 0.0
dsp-gvp-du01 	=>	 0.0
dsp-gvp-pcjr00 	=>	 0.0
dsp-gvp-nrep00 	=>	 1.0
dsp-gvp-data00 	=>	 1.0
dsp-gvp-n00 	=>	 100.0
dsp-gvp-dt00 	=>	 0.2
dsp-gvp-dsig00 	=>	 0.0
dsp-gvp-dz00 	=>	 100.0
dsp-gvp-dy00 	=>	 0.0
dsp-gvp-dx00 	=>	 0.0
dsp-gvp-du00 	=>	 1.0
dsp-TS-Repetitions 	=>	 1.0
dsp-TS-Points 	=>	 2048.0
dsp-TS-Duration 	=>	 1000.0
dsp-SP-Repetitions 	=>	 1.0
dsp-SP-Delay 	=>	 0.01
dsp-SP-Flag-V-on-X 	=>	 1.0
dsp-SP-Ramp-Time 	=>	 10.0
dsp-SP-Volts 	=>	 2.0
dsp-SP-Duration 	=>	 10.0
dsp-LP-Repetitions 	=>	 1.0
dsp-LP-Slope 	=>	 10000.0
dsp-LP-Laser-Delay 	=>	 10.0
dsp-LP-Tip-Retract 	=>	 0.0
dsp-LP-Trigger-Time 	=>	 10.0
dsp-LP-Trigger-Volts 	=>	 2.0
dsp-LP-FB-Time 	=>	 10.0
dsp-PL-Repetitions 	=>	 1.0
dsp-PL-Final-Delay 	=>	 0.01
dsp-PL-Initial-Delay 	=>	 0.01
dsp-PL-Step-dZ 	=>	 0.0
dsp-PL-Step 	=>	 0.0
dsp-PL-SetStart 	=>	 0.1
dsp-PL-dZ-ext 	=>	 0.0
dsp-PL-dZ 	=>	 0.0
dsp-PL-Volts 	=>	 2.0
dsp-PL-Res 	=>	 0.0
dsp-PL-Slope 	=>	 10000.0
dsp-PL-Duration 	=>	 10.0
dsp-Z-Final-Delay 	=>	 0.01
dsp-Z-Slope-Ramp 	=>	 100.0
dsp-Z-Reps 	=>	 1.0
dsp-Z-Slope 	=>	 100.0
dsp-Z-Points 	=>	 100.0
dsp-Z-end 	=>	 100.0
dsp-Z-start 	=>	 0.0
dsp-IV-Recover-Delay 	=>	 0.3
dsp-IV-Final-Delay 	=>	 0.01
dsp-IV-Line-Final-Delay 	=>	 1.0
dsp-IV-Line-slope 	=>	 100.0
dsp-IV-Line-dM 	=>	 0.0
dsp-IV-Line-dY 	=>	 0.0
dsp-IV-Line-dX 	=>	 50.0
dsp-IV-rep 	=>	 1.0
dsp-IV-Slope-Ramp 	=>	 50.0
dsp-IV-Slope 	=>	 10.0
dsp-IV-dz-rep 	=>	 0.0
dsp-IV-dz 	=>	 0.0
dsp-6-IV-Points05 	=>	 10.0
dsp-6-IV-End05 	=>	 1.0
dsp-6-IV-Start05 	=>	 -1.0
dsp-5-IV-Points04 	=>	 10.0
dsp-5-IV-End04 	=>	 1.0
dsp-5-IV-Start04 	=>	 -1.0
dsp-4-IV-Points03 	=>	 10.0
dsp-4-IV-End03 	=>	 1.0
dsp-4-IV-Start03 	=>	 -1.0
dsp-3-IV-Points02 	=>	 10.0
dsp-3-IV-End02 	=>	 1.0
dsp-3-IV-Start02 	=>	 -1.0
dsp-2-IV-Points01 	=>	 10.0
dsp-2-IV-End01 	=>	 1.0
dsp-2-IV-Start01 	=>	 -1.0
dsp-IV-Points00 	=>	 100.0
dsp-IV-End00 	=>	 1.0
dsp-IV-Start00 	=>	 -1.0
dsp-IV-Sections 	=>	 1.0
dsp-fbs-scan-ldc-dz 	=>	 0.0
dsp-fbs-scan-ldc-dy 	=>	 0.0
dsp-fbs-scan-ldc-dx 	=>	 0.0
dsp-fbs-vp-section 	=>	 2.0
dsp-adv-scan-slope-y 	=>	 0.0
dsp-adv-scan-slope-x 	=>	 0.0
dsp-adv-scan-xs2nd-z-offset 	=>	 0.0
dsp-adv-scan-dyn-zoom 	=>	 1.0
dsp-adv-scan-fwd-slow-down-2nd 	=>	 1.0
dsp-adv-scan-pre-pts 	=>	 0.0
dsp-adv-scan-fwd-slow-down 	=>	 1.0
dsp-adv-scan-fast-return 	=>	 1.0
dsp-adv-scan-rasterb 	=>	 0.0
dsp-adv-scan-raster 	=>	 0.0
dsp-adv-iir3-fo 	=>	 18000.0
dsp-adv-iir2-fo 	=>	 18000.0
dsp-adv-iir1-fo 	=>	 18000.0
dsp-adv-current-offset 	=>	 10.0
dsp-adv-current-crossover 	=>	 100.0
dsp-adv-iir-fo-max 	=>	 8000.0
dsp-adv-iir0-fo-min 	=>	 200.0
dsp-adv-dsp-freq-ref 	=>	 75000.0
dsp-fbs-scan-speed-scan 	=>	 4094.1
dsp-fbs-scan-speed-move 	=>	 1567.2
dsp-fbs-ci 	=>	 0.0
dsp-fbs-cp 	=>	 15.4
dsp-fbs-mx3-level 	=>	 0.0
dsp-fbs-mx3-gain 	=>	 0.5
dsp-fbs-mx3-set 	=>	 0.0
dsp-fbs-mx2-level 	=>	 0.0
dsp-fbs-mx2-gain 	=>	 0.5
dsp-fbs-mx2-set 	=>	 1.0
dsp-fbs-mx1-freq-level 	=>	 0.0
dsp-fbs-mx1-freq-gain 	=>	 -0.5
dsp-fbs-mx1-freq-set 	=>	 0.0
dsp-fbs-mx0-current-level 	=>	 0.0
dsp-fbs-mx0-current-gain 	=>	 0.5
dsp-fbs-mx0-current-set 	=>	 0.1
dsp-adv-dsp-zpos-ref 	=>	 0.0
dsp-fbs-motor 	=>	 0.0
dsp-fbs-bias3 	=>	 0.5
dsp-fbs-bias2 	=>	 0.5
dsp-fbs-bias1 	=>	 0.5
dsp-fbs-bias 	=>	 0.08567931456548372
dspmover-config-GPIO-delay 	=>	 250.0
dspmover-config-GPIO-tmp2 	=>	 0.0
dspmover-config-GPIO-tmp1 	=>	 0.0
dspmover-config-GPIO-scan 	=>	 0.0
dspmover-config-GPIO-direction 	=>	 15.0
dspmover-config-GPIO-reset 	=>	 0.0
dspmover-config-GPIO-off 	=>	 0.0
dspmover-config-GPIO-on 	=>	 0.0
dspmover-config-wave-out5-ch-z 	=>	 0.0
dspmover-config-wave-out5-ch-y 	=>	 0.0
dspmover-config-wave-out5-ch-x 	=>	 0.0
dspmover-config-wave-out4-ch-z 	=>	 0.0
dspmover-config-wave-out4-ch-y 	=>	 0.0
dspmover-config-wave-out4-ch-x 	=>	 0.0
dspmover-config-wave-out3-ch-z 	=>	 0.0
dspmover-config-wave-out3-ch-y 	=>	 0.0
dspmover-config-wave-out3-ch-x 	=>	 0.0
dspmover-config-wave-out2-ch-z 	=>	 0.0
dspmover-config-wave-out2-ch-y 	=>	 0.0
dspmover-config-wave-out2-ch-x 	=>	 0.0
dspmover-config-wave-out1-ch-z 	=>	 0.0
dspmover-config-wave-out1-ch-y 	=>	 0.0
dspmover-config-wave-out1-ch-x 	=>	 0.0
dspmover-config-wave-out0-ch-z 	=>	 5.0
dspmover-config-wave-out0-ch-y 	=>	 4.0
dspmover-config-wave-out0-ch-x 	=>	 3.0
dspmover-config-besocke-t2 	=>	 0.09
dspmover-config-besocke-t1 	=>	 0.1
dspmover-config-besocke-z-jump-ratio 	=>	 0.1
dspmover-config-IW-Phase 	=>	 55.0
dspmover-config-Wave-Offset 	=>	 0.0
dspmover-config-Wave-Space 	=>	 0.0
dspmover-z0-goto 	=>	 0.0
dspmover-z0-speed 	=>	 500.0
dspmover-z0-range 	=>	 500.0
dspmover-auto-axis-Z 	=>	 0.0
dspmover-auto-axis-Y 	=>	 0.0
dspmover-auto-axis-X 	=>	 0.0
dspmover-config-Auto-App-Retract-CI 	=>	 150.0
dspmover-config-Auto-App-Max-Settling-Time 	=>	 1000.0
dspmover-config-Auto-App-Delay 	=>	 50.0
dspmover-auto-gpio 	=>	 0.0
dspmover-auto-duration 	=>	 4.0
dspmover-auto-amplitude 	=>	 1.0
dspmover-auto-max-steps 	=>	 5.0
dspmover-lens-axis-Z 	=>	 0.0
dspmover-lens-axis-Y 	=>	 0.0
dspmover-lens-axis-X 	=>	 0.0
dspmover-lens-gpio 	=>	 0.0
dspmover-lens-duration 	=>	 5.0
dspmover-lens-amplitude 	=>	 1.0
dspmover-lens-max-steps 	=>	 100.0
dspmover-psd-axis-Z 	=>	 0.0
dspmover-psd-axis-Y 	=>	 0.0
dspmover-psd-axis-X 	=>	 0.0
dspmover-psd-gpio 	=>	 0.0
dspmover-psd-duration 	=>	 5.0
dspmover-psd-amplitude 	=>	 1.0
dspmover-psd-max-steps 	=>	 100.0
dspmover-rot-axis-Z 	=>	 0.0
dspmover-rot-axis-Y 	=>	 0.0
dspmover-rot-axis-X 	=>	 0.0
dspmover-rot-gpio 	=>	 2.0
dspmover-rot-duration 	=>	 3.0
dspmover-rot-amplitude 	=>	 1.0
dspmover-rot-max-steps 	=>	 2.0
dspmover-xy-axis-Z 	=>	 0.0
dspmover-xy-axis-Y 	=>	 0.0
dspmover-xy-axis-X 	=>	 0.0
dspmover-xy-gpio 	=>	 1.0
dspmover-xy-duration 	=>	 3.0
dspmover-xy-amplitude 	=>	 1.0
dspmover-xy-max-steps 	=>	 1000.0
SPMC_SLS_Yn 	=>	 0.0
SPMC_SLS_Ys 	=>	 0.0
SPMC_SLS_Xn 	=>	 0.0
SPMC_SLS_Xs 	=>	 0.0
rp-pacpll-RP-VERBOSE-LEVEL 	=>	 0.0
rp-pacpll-SCOPE-HEIGHT 	=>	 256.0
rp-pacpll-SCOPE-WIDTH 	=>	 1024.0
rp-pacpll-DFREQ-CONTROL-MONITOR 	=>	 0.0
rp-pacpll-CONTROL-DFREQ-FB-UPPER 	=>	 500.0
rp-pacpll-CONTROL-DFREQ-FB-LOWER 	=>	 -500.0
rp-pacpll-DFREQ-FB-CI 	=>	 -143.0
rp-pacpll-DFREQ-FB-CP 	=>	 -76.0
rp-pacpll-DFREQ-FB-SETPOINT 	=>	 0.0
rp-pacpll-DFREQ-MONITOR 	=>	 0.0
rp-pacpll-PHASE-HOLD-AM-NOISE-LIMIT 	=>	 0.0
rp-pacpll-DDS-FREQ-MONITOR 	=>	 32768.0
rp-pacpll-FREQ-FB-UPPER 	=>	 333000.0
rp-pacpll-FREQ-FB-LOWER 	=>	 32000.0
rp-pacpll-PHASE-FB-CI 	=>	 -150.0
rp-pacpll-PHASE-FB-CP 	=>	 -95.0
rp-pacpll-PHASE-FB-SETPOINT 	=>	 60.0
rp-pacpll-PHASE-MONITOR 	=>	 0.0
rp-pacpll-EXEC-AMPLITUDE-MONITOR 	=>	 0.0
rp-pacpll-EXEC-FB-UPPER 	=>	 500.0
rp-pacpll-EXEC-FB-LOWER 	=>	 -300.0
rp-pacpll-AMPLITUDE-FB-CI 	=>	 -90.0
rp-pacpll-AMPLITUDE-FB-CP 	=>	 -60.0
rp-pacpll-AMPLITUDE-FB-SETPOINT 	=>	 8.0
rp-pacpll-VOLUME-MONITOR 	=>	 0.0
rp-pacpll-TUNE-SPAN 	=>	 50.0
rp-pacpll-TUNE-DFREQ 	=>	 0.1
rp-pacpll-VOLUME-MANUAL 	=>	 0.0
rp-pacpll-AUX-SCALE 	=>	 0.011642
rp-pacpll-FREQUENCY-CENTER 	=>	 149470.0
rp-pacpll-FREQUENCY-MANUAL 	=>	 149470.0
rp-pacpll-QC-GAIN 	=>	 0.0
rp-pacpll-QC-PHASE 	=>	 0.0
rp-pacpll-PACATAU 	=>	 40.0
rp-pacpll-PACTAU 	=>	 40.0
rp-pacpll-PAC-DCTAU 	=>	 10.0
rp-pacpll-DC-OFFSET 	=>	 0.0
*
--------------------------------------------------------------------------------
(3) Gxsm3 python remote console -- help on action names used for gxsm.action command
  Hint: hover the pointer over any Gxsm Action enabled Button to see it`s action-name!
  Example: gxsm.action ("DSP_CMD_GOTO_Z0")
--------------------------------------------------------------------------------
MATH_FILTER2D_Smooth
MATH_FILTER1D_Diff
MATH_FILTER2D_Edge
MATH_FILTER2D_Normal_Z
MATH_FILTER2D_Despike
DSP_CMD_GOTO_Z0
DSP_CMD_HOME_Z0
DSP_CMD_AUTOCENTER_Z0
DSP_CMD_DOWN_Z0
DSP_CMD_UP_Z0
DSP_CMD_STOP_Z0
DSP_CMD_STOPALL
DSP_CMD_AUTOAPP
DSP_CMD_MOV-ZM_Auto
DSP_CMD_MOV-ZP_Auto
DSP_CMD_MOV-YM_Lens
DSP_CMD_MOV-XP_Lens
DSP_CMD_MOV-XM_Lens
DSP_CMD_MOV-YP_Lens
DSP_CMD_MOV-YM_PSD
DSP_CMD_MOV-XP_PSD
DSP_CMD_MOV-XM_PSD
DSP_CMD_MOV-YP_PSD
DSP_CMD_MOV-YM_Rot
DSP_CMD_MOV-XP_Rot
DSP_CMD_MOV-XM_Rot
DSP_CMD_MOV-YP_Rot
DSP_CMD_MOV-YM_XY
DSP_CMD_MOV-XP_XY
DSP_CMD_MOV-XM_XY
DSP_CMD_MOV-YP_XY
DSP_VP_ABORT_EXECUTE
DSP_VP_AX_EXECUTE
DSP_VP_AC_EXECUTE
DSP_VP_TK_EXECUTE
DSP_VP_GVP_EXECUTE
DSP_VP_RCL_V0
DSP_VP_STO_V0
DSP_VP_RCL_VPJ
DSP_VP_STO_VPJ
DSP_VP_RCL_VPI
DSP_VP_STO_VPI
DSP_VP_RCL_VPH
DSP_VP_STO_VPH
DSP_VP_RCL_VPG
DSP_VP_STO_VPG
DSP_VP_RCL_VPF
DSP_VP_STO_VPF
DSP_VP_RCL_VPE
DSP_VP_STO_VPE
DSP_VP_RCL_VPD
DSP_VP_STO_VPD
DSP_VP_RCL_VPC
DSP_VP_STO_VPC
DSP_VP_RCL_VPB
DSP_VP_STO_VPB
DSP_VP_RCL_VPA
DSP_VP_STO_VPA
DSP_VP_TS_EXECUTE
DSP_VP_SP_EXECUTE
DSP_VP_LP_EXECUTE
DSP_VP_PL_EXECUTE
DSP_VP_FZ_EXECUTE
DSP_VP_IV_EXECUTE

\end{alltt}


The following list shows a brief explanation of the commands, together with
the signature (that is the type of arguments).

'()' equals no argument. E.g. \verb+startscan()+

'(N)' equals one Integer arument. E.g. \verb+chview1d(2)+

'(X)' equals one Float argument. No example.

'(S)' equals a string. Often numbers are evaluated as strings first. Like
in \verb+set("RangeX", "234.12")+

'(S,N)' equals two parameters. E.g. \verb+gnuexport("myfilename.nc", 1)+

\begin{tabular}{ll} \hline
Scan operation\\
\texttt{startscan() }   &       Start a scan.\\
\texttt{stopscan() }    &       Stop scanning.\\
\texttt{waitscan}       &       is commented out in app\_remote.C\\
\texttt{initscan() }    &       only initialize.\\
\texttt{scanupdate() }  &       Set hardware parameters on DSP.\\
\texttt{setylookup(N,X)}&       ?\\
\texttt{scanline}       &       Not implemented.\\ \hline
File operation\\
\texttt{save() }  &       Save all.\\
\texttt{saveas(S,N) }    &       Save channel N with filename S.\\
\texttt{load(S,N)   }    &       Load file S to channel N.\\
\texttt{import(S,N) } &       Import file S to channel N.\\
\texttt{export(S,N) } &       Export channel N to file S.\\ \hline
Channel operation\\
\texttt{chmodea(N)}      &       Set channel(N) as active.\\
\texttt{chmodex(N)}      &       Set channel(N) to X.\\
\texttt{chmodem(N)}      &       Set channel(N) to Math.\\
\texttt{chmoden(N),N}    &       Set channel(N) to mode(N).\\
\texttt{chmodeno(N)}     &       Set channel(N) to mode 'No'.\\
\texttt{chview1d(N)}     &       View channel(N) in 1d-mode.\\
\texttt{chview2d(N)}     &       View channel(N) in 2d-mode.\\
\texttt{chview3d(N)}     &       View channel(N) in 3d-mode.\\ \hline
Views\\
\texttt{autodisplay()}  &       Autodisplay.\\
\texttt{quick()}        &       Set active display to quickview.\\
\texttt{direct()}       &       Set active display to directview.\\
\texttt{log()}          &       Set active display to logview.\\ \hline
Units\\
\texttt{unitbz() }      &       Set units to BZ.\\
\texttt{unitvolt()}     &       Set units to Volt.\\
\texttt{unitev()}       &       Set units to eV.\\
\texttt{units()}        &       Set units to S.\\ \hline
Others\\
\texttt{createscan(N,N,N,N,A) }  &  Create scan from array.\\
\texttt{list() }  &  Get list of known parameters for get/set.\\
\texttt{set(S,S)}       &  Set parameter to value.\\
\texttt{get(S)}         &  Get parameter, returns floating point value in current user unit .\\
\texttt{gets(S)}         &  Get parameter, returns string with user unit.\\
\texttt{action(S)}         &  Initiate Action (S): trigger menu actions and button-press events (refer to GUI tooltips in buttons and menu action list below).\\
\texttt{rtquery(S)}     &  Ask current HwI to run RTQuery with parameter S, return vector of three values depening on query.\\
\texttt{y\_current()}    &  Ask current HwI to run RTQuery what shall return the actual scanline of a scan in progress, undefined return otherwise.\\
\texttt{echo(S)  }  &       Print S to console.\\
\texttt{logev(S) }  &       Print S to logfile.\\
\texttt{sleep(N) }  &       Sleep N/10 seconds.\\
\texttt{add\_layerinformation(S,N)   }  &       Add Layerinformation string S to active scan, layer N.\\
\texttt{da0(X)   }  &       Set Analog Output channel 0 to X Volt. (not implemented).\\
\end{tabular}

% OptPlugInSubSection: The set-command

The set command can modify the following parameters:

\begin{tabular}{ll}
\texttt{ACAmp} & \texttt{ACFrq} \\
\texttt{ACPhase} & \texttt{} \\
\texttt{CPShigh} & \texttt{CPSlow} \\
\texttt{Counter} & \texttt{Energy} \\
\texttt{Gatetime} & \texttt{Layers} \\
\texttt{LengthX} & \texttt{LengthY} \\
\texttt{Offset00X} & \texttt{Offset00Y} \\
\texttt{OffsetX} & \texttt{OffsetY} \\
\texttt{PointsX} & \texttt{PointsY} \\
\texttt{RangeX} & \texttt{RangeY} \\
\texttt{Rotation} & \texttt{} \\
\texttt{StepsX} & \texttt{StepsY} \\
\texttt{SubSmp} & \texttt{VOffsetZ} \\
\texttt{VRangeZ} & \texttt{ValueEnd} \\
\texttt{ValueStart} & \texttt{nAvg} \\
\end{tabular}

These parameters are case-sensitive.
To help the python remote programmer to figure out the correct
set-names of all remote enabled entry fields a nifty option
was added to the Help menu to show tooltips with the correct "remote set name"
if the mouse is hovering over the entry.

% OptPlugInSubSection: The get-command

The \texttt{get()} command can retrieve the value of the remote control parameters.
While \texttt{get()} retrieves the internal value as a floating points number,
\texttt{gets()} reads the actual string from the text entry including units.
The list of remote control accessible parameters can be retrieved with \texttt{list()}.

\begin{alltt}
print "OffsetX = ", gxsm.get("OffsetX")
gxsm.set("OffsetX", "12.0")
print "Now OffsetX = ", gxsm.get("OffsetX")

for i in gxsm.list():
    print i, " ", gxsm.get(i), " as string: ", gxsm.gets(i)
\end{alltt}

On my machine (without hardware attached) this prints:

\begin{alltt}
OffsetX =  0.0
Now OffsetX =  12.0
Counter   0.0  as string:  00000
VOffsetZ   0.0  as string:  0 nm
VRangeZ   500.0  as string:  500 nm
Rotation   1.92285320764e-304  as string:  1.92285e-304 °
TimeSelect   0.0  as string:  0
Time   1.0  as string:  1
LayerSelect   0.0  as string:  0
Layers   1.0  as string:  1
OffsetY   0.0  as string:  0.0 nm
OffsetX   12.0  as string:  12.0 nm
PointsY   1000.0  as string:  1000
PointsX   1000.0  as string:  1000
StepsY   0.519863986969  as string:  0.52 nm
StepsX   0.519863986969  as string:  0.52 nm
RangeY   64.9830993652  as string:  65.0 nm
RangeX   64.9830993652  as string:  65.0 nm
\end{alltt}

All entry fields with assigned id can now be queried.


% OptPlugInSubSection: Creating new scans

Pyremote can create new images from scratch using the
\verb+createscan+ command. Its arguments are
pixels in x-direction, pixels in y-direction,
range in x-direction (in Angstrom),
range in y-direction (in Angstrom) and finally
a flat, numeric array that must contain
as many numbers as needed to fill the matrix.

This example creates a new scan employing sine to
show some pretty landscape.

\begin{alltt}
import array   # for array
import numpy # for fromfunction
import math    # for sin

def dist(x,y):
   return ((numpy.sin((x-50)/15.0) + numpy.sin((y-50)/15.0))*100)

m = numpy.fromfunction(dist, (100,100))
n = numpy.ravel(m) # make 1-d
p = n.tolist()       # convert to list

examplearray = array.array('l', p) #
gxsm.createscan(100, 100, 10000, 10000, examplearray)
\end{alltt}


\GxsmScreenShot{GxsmPI_pyremote01}{An autogenerated image.}

This command can be easily extended to create an importer for arbitrary
file formats via python. The scripts directory contains an elaborate
example how to use this facility to import the file format employed
by Nanonis.


% OptPlugInSubSection: Menupath and Plugins

Any plugin, that has a menuentry can be
executed via the
\GxsmTT{menupath}-action command. Several of them, however, open a dialog and ask
for a specific parameter, e.g. the diff-PI in \GxsmMenu{Math/Filter1D}.
This can become annoying, when you want to batch process a greater number
of files. To execute a PI non-interactively it is possible to
call a plugin from scripts with default parameters and no user interaction.

The \GxsmTT{diff}-PI can be called like this:

\begin{alltt}
print "Welcome to Python."
gxsm.logev('my logentry')
gxsm.startscan()
gxsm.action('diff_PI')
\end{alltt}

The \GxsmTT{diff}- and \GxsmTT{smooth}-function are, at the time of this
writing, the only Math-PI, that have such an 'action'-callback. Others
will follow. See \GxsmFile{diff.C} to find out, how to extend your
favourite PI with action-capabilities.

The action-command can execute the following PI:

\begin{tabular}{ll}
\GxsmTT{diff\_PI} & kernel-size set to 5+1\\
\GxsmTT{smooth\_PI} & kernel-size set to 5+1\\
\GxsmTT{print\_PI} & defaults are read from gconf\\
\end{tabular}

GXSM3 Menu Action Table Information -- remote menu action/math/... call via action key, see table below for list, example:

\begin{alltt}
gxsm.signal_emit("math-filter1d-section-Koehler")
\end{alltt}

\begin{tabular}{l|l||l}
Section ID & Menu Entry & Action Key\\
\hline\hline
math-filter2d-section & Stat Diff & math-filter2d-section-Stat-Diff\\
math-convert-section & to float & math-convert-section-to-float\\
math-arithmetic-section & Z Rescale & math-arithmetic-section-Z-Rescale\\
math-statistics-section & Add Trail & math-statistics-section-Add-Trail\\
math-transformations-section & OctoCorr & math-transformations-section-OctoCorr\\
math-arithmetic-section & Mul X & math-arithmetic-section-Mul-X\\
math-filter2d-section & Edge & math-filter2d-section-Edge\\
math-arithmetic-section & Max & math-arithmetic-section-Max\\
math-statistics-section & Stepcounter & math-statistics-section-Stepcounter\\
math-convert-section & to double & math-convert-section-to-double\\
math-filter1d-section & Koehler & math-filter1d-section-Koehler\\
math-statistics-section & Histogram & math-statistics-section-Histogram\\
math-background-section & Line: 2nd order & math-background-section-Line--2nd-order\\
math-filter2d-section & Despike & math-filter2d-section-Despike\\
math-transformations-section & Auto Align & math-transformations-section-Auto-Align\\
math-background-section & Plane Regression & math-background-section-Plane-Regression\\
math-statistics-section & Cross Correlation & math-statistics-section-Cross-Correlation\\
math-arithmetic-section & Z Usr Rescale & math-arithmetic-section-Z-Usr-Rescale\\
math-filter1d-section & Diff & math-filter1d-section-Diff\\
math-filter2d-section & T derive & math-filter2d-section-T-derive\\
math-background-section & Pass CC & math-background-section-Pass-CC\\
math-statistics-section & Auto Correlation & math-statistics-section-Auto-Correlation\\
math-transformations-section & Rotate 90deg & math-transformations-section-Rotate-90deg\\
math-arithmetic-section & Invert & math-arithmetic-section-Invert\\
math-background-section & Plane max prop & math-background-section-Plane-max-prop\\
math-convert-section & U to float & math-convert-section-U-to-float\\
math-transformations-section & Movie Concat & math-transformations-section-Movie-Concat\\
math-transformations-section & Shear Y & math-transformations-section-Shear-Y\\
math-statistics-section & NN-distribution & math-statistics-section-NN-distribution\\
math-background-section & Waterlevel & math-background-section-Waterlevel\\
math-transformations-section & Quench Scan & math-transformations-section-Quench-Scan\\
math-arithmetic-section & Div X & math-arithmetic-section-Div-X\\
math-statistics-section & Vacancy Line Analysis & math-statistics-section-Vacancy-Line-Analysis\\
math-transformations-section & Shear X & math-transformations-section-Shear-X\\
math-convert-section & to complex & math-convert-section-to-complex\\
math-convert-section & make test & math-convert-section-make-test\\
math-misc-section & Spectrocut & math-misc-section-Spectrocut\\
math-arithmetic-section & Log & math-arithmetic-section-Log\\
math-statistics-section & Average X Profile & math-statistics-section-Average-X-Profile\\
math-filter2d-section & Lineinterpol & math-filter2d-section-Lineinterpol\\
math-background-section & Z drift correct & math-background-section-Z-drift-correct\\
math-background-section & Line Regression & math-background-section-Line-Regression\\
math-statistics-section & SPALEED Simkz & math-statistics-section-SPALEED-Simkz\\
math-convert-section & to byte & math-convert-section-to-byte\\
math-statistics-section & Slope Abs & math-statistics-section-Slope-Abs\\
math-filter2d-section & Small Convol & math-filter2d-section-Small-Convol\\
math-convert-section & to long & math-convert-section-to-long\\
math-transformations-section & Multi Dim Transpose & math-transformations-section-Multi-Dim-Transpose\\
math-arithmetic-section & Sub X & math-arithmetic-section-Sub-X\\
math-background-section & Stop CC & math-background-section-Stop-CC\\
math-filter2d-section & FT 2D & math-filter2d-section-FT-2D\\
math-convert-section & to short & math-convert-section-to-short\\
math-transformations-section & Volume Transform & math-transformations-section-Volume-Transform\\
math-background-section & Gamma & math-background-section-Gamma\\
math-background-section & Plane 3 Points & math-background-section-Plane-3-Points\\
math-transformations-section & Affine & math-transformations-section-Affine\\
math-misc-section & Shape & math-misc-section-Shape\\
math-background-section & Sub Const & math-background-section-Sub-Const\\
math-transformations-section & Flip Diagonal & math-transformations-section-Flip-Diagonal\\
math-background-section & Timescale FFT & math-background-section-Timescale-FFT\\
math-misc-section & Layersmooth & math-misc-section-Layersmooth\\
math-filter1d-section & Lin stat diff & math-filter1d-section-Lin-stat-diff\\
math-filter2d-section & Smooth & math-filter2d-section-Smooth\\
math-transformations-section & Manual Drift Fix/Align & math-transformations-section-Manual-Drift-Fix-Align\\
math-statistics-section & Polar Histogramm & math-statistics-section-Polar-Histogramm\\
math-statistics-section & feature match & math-statistics-section-feature-match\\
math-statistics-section & Angular Analysis & math-statistics-section-Angular-Analysis\\
math-transformations-section & Merge V & math-transformations-section-Merge-V\\
math-filter2d-section & Local height & math-filter2d-section-Local-height\\
math-filter2d-section & IFT 2D & math-filter2d-section-IFT-2D\\
math-statistics-section & Baseinfo & math-statistics-section-Baseinfo\\
math-transformations-section & Scale Scan & math-transformations-section-Scale-Scan\\
math-filter2d-section & Curvature & math-filter2d-section-Curvature\\
math-arithmetic-section & Add X & math-arithmetic-section-Add-X\\
math-transformations-section & Mirror X & math-transformations-section-Mirror-X\\
math-transformations-section & Merge H & math-transformations-section-Merge-H\\
math-statistics-section & feature recenter & math-statistics-section-feature-recenter\\
math-transformations-section & Shift-Area & math-transformations-section-Shift-Area\\
math-misc-section & Workfuncextract & math-misc-section-Workfuncextract\\
math-filter1d-section & t dom filter & math-filter1d-section-t-dom-filter\\
math-statistics-section & SPALEED Sim. & math-statistics-section-SPALEED-Sim.\\
math-statistics-section & Slope Dir & math-statistics-section-Slope-Dir\\
math-transformations-section & Reverse Layers & math-transformations-section-Reverse-Layers\\
math-filter1d-section & Despike & math-filter1d-section-Despike\\
math-background-section & Rm Line Shifts & math-background-section-Rm-Line-Shifts\\
math-transformations-section & Rotate & math-transformations-section-Rotate\\
math-transformations-section & Mirror Y & math-transformations-section-Mirror-Y\\
math-arithmetic-section & Z Limiter & math-arithmetic-section-Z-Limiter\\
math-probe-section & AFM Mechanical Simulation & math-probe-section-AFM-Mechanical-Simulation\\
math-probe-section & Image Extract & math-probe-section-Image-Extract\\
\hline
file-import-section & PNG & file-import-section-PNG\\
file-export-section & PNG & file-export-section-PNG\\
file-import-section & WIP & file-import-section-WIP\\
file-export-section & WIP & file-export-section-WIP\\
file-import-section & primitive auto & file-import-section-primitive-auto\\
file-export-section & primitive auto & file-export-section-primitive-auto\\
file-import-section & UKSOFT & file-import-section-UKSOFT\\
file-export-section & UKSOFT & file-export-section-UKSOFT\\
file-import-section & PsiHDF & file-import-section-PsiHDF\\
file-export-section & PsiHDF & file-export-section-PsiHDF\\
file-import-section & WSxM & file-import-section-WSxM\\
file-export-section & WSxM & file-export-section-WSxM\\
file-import-section & RHK-200 & file-import-section-RHK-200\\
file-import-section & RHK SPM32 & file-import-section-RHK-SPM32\\
file-export-section & RHK SPM32 & file-export-section-RHK-SPM32\\
file-import-section & Nano Scope & file-import-section-Nano-Scope\\
file-import-section & Omicron Scala & file-import-section-Omicron-Scala\\
file-import-section & UK2k & file-import-section-UK2k\\
file-import-section & Vis5D & file-import-section-Vis5D\\
file-export-section & Vis5D & file-export-section-Vis5D\\
file-import-section & G-dat & file-import-section-G-dat\\
file-export-section & G-dat & file-export-section-G-dat\\
file-import-section & SDF & file-import-section-SDF\\
file-import-section & GME Dat & file-import-section-GME-Dat\\
file-export-section & GME Dat & file-export-section-GME-Dat\\
file-import-section & ASCII & file-import-section-ASCII\\
file-export-section & ASCII & file-export-section-ASCII\\
file-import-section & SPA4-d2d & file-import-section-SPA4-d2d\\
file-export-section & SPA4-d2d & file-export-section-SPA4-d2d\\
file-import-section & Quicktime & file-import-section-Quicktime\\
file-export-section & Quicktime & file-export-section-Quicktime\\
\end{tabular}


% OptPlugInSubSection: DSP-Control

The DSP-Control is the heart of SPM activity. The following parameters
can be set with \GxsmTT{set}. (DSP2 commands are available in Gxsm 2 only)

\GxsmNote{Manual Hacker notes: list of DSP/DSP2 is depricated. All entry fields with hover-over entry id is now remote capable.}

\begin{tabular}{ll}
\GxsmTT{DSP\_CI} & \GxsmTT{DSP2\_CI} \\
\GxsmTT{DSP\_CP} & \GxsmTT{DSP2\_CP} \\
\GxsmTT{DSP\_CS} & \GxsmTT{DSP2\_CS} \\
\GxsmTT{DSP\_I} & \GxsmTT{DSP2\_I} \\
\GxsmTT{DSP\_MoveLoops} & \GxsmTT{DSP2\_MoveLoops} \\
\GxsmTT{DSP\_MoveSpd} & \GxsmTT{DSP2\_MoveSpd} \\
\GxsmTT{DSP\_NAvg} & \GxsmTT{DSP2\_NAvg} \\
\GxsmTT{DSP\_Pre} & \GxsmTT{DSP2\_Pre} \\
\GxsmTT{DSP\_ScanLoops} & \GxsmTT{DSP2\_ScanLoops} \\
\GxsmTT{DSP\_ScanSpd} & \GxsmTT{DSP2\_ScanSpd} \\
\GxsmTT{DSP\_SetPoint} & \GxsmTT{DSP2\_SetPoint} \\
\GxsmTT{DSP\_U} & \GxsmTT{DSP2\_U} \\
\end{tabular}

\GxsmNote{Manual Hacker notes: VP exectutes via hover-over ExecuteID and action command.}



% OptPlugInSubSection: Peakfinder

Another plugin allows remote control. The plugin-functions are commonly
executed by a call of the \GxsmTT{action}-command. It is
\GxsmFile{Peakfinder}:

DSP Peak Find Plugin Commandset for the SPA-LEED peak finder:\\

\begin{tabular}{lllll}
\multicolumn{5}{c}{Commands Plugin \filename{DSP Peak Find}:}\\ \\ \hline
Cmd & Arg. & \multicolumn{2}{l}{Values} & Description\\ \hline
\hline
action & DSPPeakFind\_XY0\_1 &&& Get fitted XY Position\\
action & DSPPeakFind\_OffsetFromMain\_1 &&& Get Offset from Main\\
action & DSPPeakFind\_OffsetToMain\_1 &&& Put Offset to Main\\
action & DSPPeakFind\_EfromMain\_1 &&& Get Energy from Main\\
action & DSPPeakFind\_RunPF\_1 &&& Run Peak Finder\\
\hline
action & DSPPeakFind\_XXX\_N &&& run action XXX (see above)\\
       &                    &&& on PF Folder N\\
\end{tabular}

The call is equivalent to the example above.

% OptPlugInSubSection: Peakfinder

% OptPlugInConfig

The plugin can be configured in the preferences. The default script
that will be loaded when the console is enetred for the first time
is defined in the path-tab in item \GxsmFile{PyremoteFile}.
The name must be a qualified python module name. A module name
is not a filename! Thus \verb+remote.py+ is not a valid entry,
but \verb+remote+ (the default) is. The module is found by searching the
directories listed in the environment variable PYTHONPATH. If no file is
defined or a file matching the name cannot be found, a warning will be issued.

The module with GXSM internal commands
is called \GxsmFile{gxsm}.

To find the Python-script \GxsmFile{remote.py}, the environment-variable
PYTHONPATH is evaluated. If it is not expliticly declared, GXSM will set
PYTHONPATH to your current working directory. This is equivalent to the
following call:

\begin{alltt}
$export PYTHONPATH='.'
$gxsm3
\end{alltt}

Thus, the script in your current working directory will be found.

If you want to put your script somewhere else than into the
current directory, modify the environment variable
\GxsmFile{PYTHONPATH}. Python
will look into all directories, that are stored there.

\begin{alltt}
$export PYTHONPATH='/some/obscure/path/'
$gxsm
\end{alltt}

Or you can link it from somewhere else. Or you can create a one line script,
that executes another script or several scripts. Do whatever you like.


% OptPlugInFiles
 Python precompiles your remote.py to remote.pyc. You can safely remove the
file remote.pyc file at any time, Python will regenerate it upon start of
the interpreter.

% OptPlugInRefs
See the appendix for more information. Don't know Python? Visit
\GxsmTT{python.org}.

% OptPlugInKnownBugs

The error handling is only basic. Your script may run if you give
wrong parameters but not deliver the wanted results. You can crash
Gxsm or even X! E.g. by selecting an illegal channel. Remember that channel
counting in the scripts begins with 0. Gxsm's channel numbering begins with 1.

The embedded functions return $-1$ as error value. It's a good idea
to attach \texttt{print} to critical commands to check this.

The \verb+remote_echo+ command is implemented via debug printing.
Using Pythons \texttt{print} is recommended.

The view functions \GxsmFile{quick}, \GxsmFile{direct}, \GxsmFile{log}
change the viewmode, but not the button in the main window, don't be
confused.

The waitscan and da0 function are not yet implemented and likely will never
be.

The library detection during compilation is amateurish. Needs work.

Python will check for the
right type of your arguments. Remember, that all values in \GxsmTT{set} are strings
and have to be quoted. Additionaly care for the case sensitivity.

If you you want to pause script execution, use the embedded sleep command
\GxsmTT{gxsm.sleep()} and not \GxsmTT{time.sleep()}, because the function from
the time library will freeze GXSM totally during the sleep.
(This is not a bug, it's a feature.)

% OptPlugInNotes
TODO: Add more action-handlers in Math-PI.
% and clean up inconsistent use of spaces and tabs.

% OptPlugInHints
If you write a particularly interesting remote-script, please give it back
to the community. The GXSM-Forums always welcome input.

% EndPlugInDocuSection
 * --------------------------------------------------------------------------------
 */

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
//#include "numpy/arrayobject.h"
#include "/usr/lib/python3/dist-packages/numpy/core/include/numpy/arrayobject.h"
#include "/usr/lib/python3/dist-packages/numpy/core/include/numpy/ndarraytypes.h"
#include "/usr/lib/python3/dist-packages/numpy/core/include/numpy/ndarrayobject.h"

#include <sys/types.h>
#include <signal.h>

#include "glib/gstdio.h"

#include "config.h"
#include "glbvars.h"
#include "plugin.h"
#include "gnome-res.h"
#include "action_id.h"
#include "xsmtypes.h"



#include "gapp_service.h"
#include "xsm.h"
#include "unit.h"
#include "pcs.h"

#include "gxsm_app.h"
#include "gxsm_window.h"
#include "app_view.h"
#include "surface.h"

#include "pyremote.h"

#include "pyscript_templates.h"
#include "pyscript_templates_script_libs.h"

// number of script control EC's -- but must manually match schemata in .xml files!
#define NUM_SCV 10

#define DEFAULT_SLEEP_USECS 10000

// Plugin Prototypes
static void pyremote_init( void );
static void pyremote_about( void );
static void pyremote_configure( void );
static void pyremote_cleanup( void );

// Fill in the GxsmPlugin Description here
GxsmPlugin pyremote_pi = {
        NULL,                   // filled in and used by Gxsm, don't touch !
        NULL,                   // filled in and used by Gxsm, don't touch !
        0,                      // filled in and used by Gxsm, don't touch !
        NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is
                                // filled in here by Gxsm on Plugin load,
                                // just after init() is called !!!
        // ----------------------------------------------------------------------
        // Plugins Name, CodeStly is like: Name-M1S[ND]|M2S-BG|F1D|F2D|ST|TR|Misc
        (char *)"Pyremote",
        NULL,
        // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
        (char *)"Remote control",
        // Author(s)
        (char *) "Stefan Schroeder",
        // Menupath to position where it is appended to
        (char *)"tools-section",
        // Menuentry
        N_("Pyremote Console"),
        // help text shown on menu
        N_("Python Remote Control Console"),
        // more info...
        (char *)"See Manual.",
        NULL,          // error msg, plugin may put error status msg here later
        NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
        // init-function pointer, can be "NULL",
        // called if present at plugin load
        pyremote_init,
        // query-function pointer, can be "NULL",
        // called if present after plugin init to let plugin manage it install itself
        NULL, // query should be "NULL" for Gxsm-Math-Plugin !!!
        // about-function, can be "NULL"
        // can be called by "Plugin Details"
        pyremote_about,
        // configure-function, can be "NULL"
        // can be called by "Plugin Details"
        pyremote_configure,
        // run-function, can be "NULL", if non-Zero and no query defined,
        // it is called on menupath->"plugin"
        NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
        // cleanup-function, can be "NULL"
        // called if present at plugin removal
        NULL, // direct menu entry callback1 or NULL
        NULL, // direct menu entry callback2 or NULL

        pyremote_cleanup
};

// Forward declaration
class py_gxsm_console;

// GXSM PY REMOTE GUI/CONSOLE CLASS
py_gxsm_console *py_gxsm_remote_console = NULL;

// Text used in Aboutbox, please update!!a
static const char *about_text = N_("Gxsm Plugin\n\n"
                                   "Python Remote Control.");

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!!
GxsmPlugin *get_gxsm_plugin_info ( void ){
        pyremote_pi.description = g_strdup_printf(N_("Gxsm pyremote plugin %s"), VERSION);
        return &pyremote_pi;
}

// 5.) Start here with the plugins code, vars def., etc.... here.
// ----------------------------------------------------------------------
//
// TODO:
// More error-handling
// Cannot return int in run
// fktname editable in preferences.
// Add numeric interface (LOW)
// Add image interface (LOW)
// Add i/o possibility (LOW)

// about-Function
static void pyremote_about(void)
{
        const gchar *authors[] = { pyremote_pi.authors, NULL};
        gtk_show_about_dialog (NULL,
                               "program-name",  pyremote_pi.name,
                               "version", VERSION,
                               "license", GTK_LICENSE_GPL_3_0,
                               "comments", about_text,
                               "authors", authors,
                               NULL);
}

// configure-Function
static void pyremote_configure(void)
{
        if(pyremote_pi.app){
                pyremote_pi.app->message("Pyremote Plugin Configuration");
        }
}

// TODO: Remove me? Simple keep PyObject instance in py_gxsm_console?
typedef struct {
        PyObject *main_module;
} PyGxsmModuleInfo;

static PyGxsmModuleInfo py_gxsm_module;

typedef struct {
        gchar *cmd;
        int mode;
        PyObject *ret;
} PyRunThreadData;

void clear_run_data(PyRunThreadData* run_data) {
        run_data->cmd = NULL;
        run_data->mode = 0;
        run_data->ret = NULL;
}


static GMutex g_list_mutex;

static GMutex mutex;
#define WAIT_JOIN_MAIN {gboolean tmp; do{ g_usleep (10000); g_mutex_lock (&mutex); tmp=idle_data.wait_join; g_mutex_unlock (&mutex); }while(tmp);}
#define UNSET_WAIT_JOIN_MAIN g_mutex_lock (&mutex); idle_data->wait_join=false; g_mutex_unlock (&mutex)


typedef struct {
        remote_args ra;
        const gchar *string;
        PyObject *self;
        PyObject *args;
        gint ret;
        gboolean wait_join;
        double vec[4];
        gint64 i64;
        gchar  c;
} IDLE_from_thread_data;


class py_gxsm_console : public AppBase {
public:
        py_gxsm_console ():AppBase(){
                script_filename = NULL;
                gui_ready = false;

                clear_run_data( &user_script_data );
                reset_user_script_data = true;
                action_script_running = 0;

                message_list  = NULL;
                g_mutex_init (&mutex);

                closing = false;
        };
        virtual ~py_gxsm_console ();

        virtual void AppWindowInit(const gchar *title);

        void initialize(void);
        static PyObject* run_string(const char *cmd, int type, PyObject *g,
                                    PyObject *l,
                                    py_gxsm_console *console = NULL);
        void show_stderr(const gchar *str);
        void initialize_stderr_redirect(PyObject *d);
        // Decrement the reference to a copied __main__, to inform the
        // interpreter it can be destroyed. If clearGlobals is true,
        // we clear our provided dict. Note that this will stop any other
        // thread running Python within the interpreter!
        void destroy_environment(PyObject *d, bool clearGlobals=false);
        // Shallow copy __main__, for use running a script.
        PyObject* create_environment(const gchar *filename);

        static gpointer PyRunConsoleThread(gpointer data);
        static gpointer PyRunActionScriptThread(gpointer data);

        const char* run_command(const gchar *cmd, int mode,
                                bool reset_locals, bool reset_globals,
                                bool run_as_action_script);

        void push_message_async (const gchar *msg){
                g_mutex_lock (&g_list_mutex);
                if (msg)
                        message_list = g_slist_prepend (message_list, g_strdup(msg));
                else
                        message_list = g_slist_prepend (message_list, NULL); // push self terminate IDLE task mark
                g_mutex_unlock (&g_list_mutex);
                g_idle_add (pop_message_list_to_console, this); // keeps running and watching for async console data to display
        }

        static gboolean pop_message_list_to_console (gpointer user_data){
                py_gxsm_console *pygc = (py_gxsm_console*) user_data;

                g_mutex_lock (&g_list_mutex);
                if (!pygc->message_list){
                        g_mutex_unlock (&g_list_mutex);
                        return G_SOURCE_REMOVE;
                }
                GSList* last = g_slist_last (pygc->message_list);
                if (!last){
                        g_mutex_unlock (&g_list_mutex);
                        return G_SOURCE_REMOVE;
                }
                if (last -> data)  {
                        pygc->append ((const gchar*)last -> data);
                        g_free (last -> data);
                        pygc->message_list = g_slist_delete_link (pygc->message_list, last);
                        g_mutex_unlock (&g_list_mutex);
                        return G_SOURCE_REMOVE;
                } else { // NULL data mark found
                        pygc->message_list = g_slist_delete_link (pygc->message_list, last);
                        g_mutex_unlock (&g_list_mutex);
                        pygc->append ("--END IDLE--");
                        return G_SOURCE_REMOVE; // finish IDLE task
                }
        }

        void append (const gchar *msg);

        gchar *pre_parse_script (const gchar *script, int *n_lines=NULL, int r=0); // parse script for gxsm lib include statements

        static void open_file_callback_exec (GtkDialog *dialog,  int response, gpointer user_data);
        static void open_file_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data);
        static void open_action_script_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data);
        static void save_file_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data);
        static void save_file_as_callback_exec (GtkDialog *dialog,  int response, gpointer user_data);
        static void save_file_as_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data);
        static void configure_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data);
        static void restart_interpreter_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data);

        static void run_file (GtkButton *btn, gpointer user_data);
        static void kill (GtkToggleButton *btn, gpointer user_data);

        void create_gui(void);
        void run();

        static void command_execute(GtkEntry *entry, gpointer user_data);
        static void clear_output(GtkButton *btn, gpointer user_data);
        // static gboolean check_func(PyObject *m, gchar *name, gchar *filename);


        gchar *get_gxsm_script (const gchar *name){
                gchar *tmp_script = NULL;
                gchar* path = g_strconcat (g_get_home_dir (), "/.gxsm3/pyaction", NULL);
                gchar* tmp_script_filename = g_strconcat (path, "/", name, ".py", NULL);
                g_free (path);
                if (g_file_test (tmp_script_filename, G_FILE_TEST_EXISTS)){
                        GError *err = NULL;
                        if (!g_file_get_contents(tmp_script_filename,
                                                 &tmp_script,
                                                 NULL,
                                                 &err)) {
                                gchar *message = g_strdup_printf("Cannot read content of file "
                                                                 "'%s': %s",
                                                                 tmp_script_filename,
                                                                 err->message);
                                append (message);
                                gapp->warning (message);
                                g_free(message);
                        }
                } else {
                        gchar *message = g_strdup_printf("Action script/library %s not yet defined.\nPlease define action script using the python console.", tmp_script_filename);
                        g_message ("%s", message);
                        append(message);
                        gapp->warning (message);
                        g_free(message);
                }
                g_free (tmp_script_filename);
                return tmp_script;
        };

        void run_action_script (const gchar *name){
                gchar *tmp_script = get_gxsm_script (name);
                if (tmp_script){
                        gchar *tmp = g_strdup_printf ("%s #jobs[%d]+1", N_("\n>>> Executing action script: "), action_script_running);
                        append (tmp);
                        g_free (tmp);
                        append (name);
                        append ("\n");
                        // TODO: No way to know if it ran??
                        const gchar *output = run_command(tmp_script, Py_file_input,
                                false, false, true);
                        g_free (tmp_script);
                        append (output);
                        append (N_("\n<<< Action script starting?: "));
                        append (name);
                        append ("\n");
                }
        };

        void set_script_filename (const gchar *name = NULL){
                if (name){
                        if (script_filename)
                                g_free (script_filename);
                        if (strstr (name, ".py")){
                                script_filename = g_strdup (name);
                        } else {
                                gchar* path = g_strconcat (g_get_home_dir (), "/.gxsm3/pyaction", NULL);
                                // attempt to create folder if not exiting
                                if (!g_file_test (path, G_FILE_TEST_IS_DIR)){
                                        g_message ("creating action script folder %s", path);
                                        g_mkdir_with_parents (path, 0777);
                                }
                                script_filename = g_strconcat (path, "/", name, ".py", NULL);
                                g_free (path);
                                if (!g_file_test (script_filename, G_FILE_TEST_EXISTS)){
                                        if (strstr (script_filename, "gxsm3-script")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Example python gxsm3 script file " << script_filename << " was created.\n"
                                                        "# this is the gxsm developer test and work script - see the Gxsm3 manual for more information\n";
                                                example_file << template_demo_script;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-help")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script file " << script_filename << " was created.\n";
                                                example_file << template_help;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-data-access-template")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script file " << script_filename << " was created.\n";
                                                example_file << template_data_access;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-control-watchdog")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script file " << script_filename << " was created.\n";
                                                example_file << template_watchdog;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-control-gxsm-sok-server")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script file " << script_filename << " was created.\n";
                                                example_file << template_gxsm_sok_server;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-data-cfextract-simple")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script file " << script_filename << " was created.\n";
                                                example_file << template_data_cfextract_simple;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-data-cfextract-data")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script file " << script_filename << " was created.\n";
                                                example_file << template_data_cfextract_data;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-3d-animate")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Surface3D/GL Animation Example script file " << script_filename << " was created.\n\n";
                                                example_file << template_animate;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-movie-export")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script for Multi layer/time series Drawing Export " << script_filename << " was created.\n\n";
                                                example_file << template_movie_drawing_export;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-lib-utils")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script Library: " << script_filename << " was created.\n\n";
                                                example_file << template_library_utils;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-lib-control")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script Library: " << script_filename << " was created.\n\n";
                                                example_file << template_library_control;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-lib-scan")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script Library: " << script_filename << " was created.\n\n";
                                                example_file << template_library_scan;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-lib-probe")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script Library: " << script_filename << " was created.\n\n";
                                                example_file << template_library_probe;
                                                example_file.close();
                                        } else if (strstr (script_filename, "gxsm3-lib-analysis")){
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Gxsm Python Script Library: " << script_filename << " was created.\n\n";
                                                example_file << template_library_analysis;
                                                example_file.close();
                                        } else {
                                                // make sample
                                                std::ofstream example_file;
                                                example_file.open(script_filename);
                                                example_file << "# Example python action script file " << script_filename << " was created.\n"
                                                        "# - see the Gxsm3 manual for more information\n"
                                                        "gxsm.set (\"dsp-fbs-bias\",\"0.1\") # set Bias to 0.1V\n"
                                                        "gxsm.set (\"dsp-fbs-mx0-current-set\",\"0.01\") # Set Current Setpoint to 0.01nA\n"
                                                        "# gxsm.sleep (2)  # sleep for 2/10s\n";
                                                example_file.close();
                                        }
                                }
                        }
                }
                if (script_filename)
                        gtk_header_bar_set_subtitle (GTK_HEADER_BAR  (header_bar), script_filename);
                else
                        gtk_header_bar_set_subtitle (GTK_HEADER_BAR  (header_bar), "no valid file name");
        };
        void write_example_file (void);

        void fix_eols_to_unix (gchar *text);

        gboolean set_sc_label (const gchar *id, const gchar *l){
                // *id = g_strdup_printf ("py-sc%02d",i+1);
                if (strlen(id) < 7) return false;
                int i = atoi(&id[5]);
                if (i>0 && i <= NUM_SCV){
                        gtk_label_set_text (GTK_LABEL(sc_label[i-1]) ,l);
                        return true;
                } else return false;
        };

private:
        // Data linked to Python scripts
        PyRunThreadData user_script_data;
        bool reset_user_script_data;
        bool restart_interpreter;
        gint action_script_running;

        gboolean closing; // Indicates Python console closing

        GSList *message_list;
        gboolean gui_ready;

        GSettings *gsettings;
        GtkWidget *file_menu;

        const char *example_filename = "gxsm_pyremote_example.py";

        // Console GUI elemets
        PyObject *std_err;
        PyObject *dictionary;
        GtkWidget *console_output;
        GtkTextMark *console_mark_end;
        GtkWidget *console_file_content;
        gchar *script_filename;
        gboolean query_filename;
        gboolean fail;
        gdouble exec_value;
        gdouble sc_value[NUM_SCV];
        GtkWidget *sc_label[NUM_SCV];
};



///////////////////////////////////////////////////////////////
// BLOCK I -- generic, help, get/set data, actions
///////////////////////////////////////////////////////////////

/* stolen from app_remote.C */
static void Check_ec(Gtk_EntryControl* ec, remote_args* ra){
        ec->CheckRemoteCmd (ra); // only reading PCS is thread safe!
};

static void Check_conf(GnomeResPreferences* grp, remote_args* ra){
        if (grp && ra)
                gnome_res_check_remote_command (grp, ra);
};

static void CbAction_ra(remote_action_cb* ra, gpointer arglist){
        if(ra->cmd && ((gchar**)arglist)[1])
                if(! strcmp(((gchar**)arglist)[1], ra->cmd)){
                        if (ra->data)
                                (*ra->RemoteCb) (ra->widget, ra->data);
                        else
                                (*ra->RemoteCb) (ra->widget, arglist);
                        // see above and pcs.h
                }
};

static PyObject* remote_help(PyObject *self, PyObject *args);

/* This function will build and return a python tuple
   that contains all the objects (string name) that
   you can 'set' and 'get'.

   Example output of 'print gxsm.list()':

   ('Counter', 'VOffsetZ', 'VRangeZ', 'Rotation',
   'TimeSelect', 'Time', 'LayerSelect', 'Layers',
   'OffsetY', 'OffsetX', 'PointsY', 'PointsX',
   'StepsY', 'StepsX', 'RangeY', 'RangeX')

   when no hardware is attached.

*/
static PyObject* remote_listr(PyObject *self, PyObject *args)
{
        int slen = g_slist_length(gapp->RemoteEntryList ); // How many entries?

// This will be our return object with as many slots as input list has:
        PyObject *ret = PyTuple_New(slen);
        GSList* tmp =gapp->RemoteEntryList;
        for (int n=0; n<slen; n++){
                Gtk_EntryControl* ec = (Gtk_EntryControl*)tmp->data; // Look at data item in GSList.
                PyTuple_SetItem(ret, n, PyUnicode_FromString(ec->get_refname())); // Add Refname to Return-list
                tmp = g_slist_next(tmp);
        }

        return ret;
}

static PyObject* remote_lista(PyObject *self, PyObject *args)
{
        int slen = g_slist_length(gapp->RemoteActionList ); // How many entries?

        // This will be our return object with as many slots as input list has:
        PyObject *ret = PyTuple_New(slen);
        GSList* tmp =gapp->RemoteActionList;
        for (int n=0; n<slen; n++){
                remote_action_cb* ra = (remote_action_cb*)tmp->data; // Look at data item in GSList.
                PyTuple_SetItem(ret, n, PyUnicode_FromString(ra->cmd)); // Add Refname to Return-list
                tmp = g_slist_next(tmp);
        }

        return ret;
}

static PyObject* remote_gets(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Getting as string ");
        gchar *parameter;

        if (!PyArg_ParseTuple(args, "s", &parameter))
                return Py_BuildValue("i", -1);

        int parameterlen = strlen(parameter);
        int slen = g_slist_length(gapp->RemoteEntryList );

        gchar *ret = NULL;

        GSList* tmp =gapp->RemoteEntryList;
        for (int n=0; n<slen; n++)
        {
                Gtk_EntryControl* ec = (Gtk_EntryControl*)tmp->data;

                if (strncmp(parameter, ec->get_refname(), parameterlen) == 0)
                {
                        ret = g_strdup(ec->Get_UsrString());
                }
                tmp = g_slist_next(tmp);
        }

        if (ret == NULL) // If the parameter doesn't exist.
        {
                ret = g_strdup("ERROR");
        }

        return Py_BuildValue("s", ret);
}


// Getting value in current user unit as plain double number -- could also/option get as string with unit
static PyObject* remote_get(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Getting ");
        gchar *parameter;
        remote_args ra;
        ra.qvalue = 0.;

        if (!PyArg_ParseTuple(args, "s", &parameter))
                return Py_BuildValue("i", -1);

        PI_DEBUG(DBG_L2, parameter << " query" );

        ra.qvalue = 0.;
        gchar *list[] = {(gchar *)"get", parameter, NULL};
        ra.arglist = list;

        g_slist_foreach(gapp->RemoteEntryList, (GFunc) Check_ec, (gpointer)&ra);
        PI_DEBUG(DBG_L2, parameter << " query result: " << ra.qvalue );

        if (ra.qstr)
                return Py_BuildValue("s", ra.qstr);
        else
                return Py_BuildValue("f", ra.qvalue);
}

static gboolean main_context_set_entry_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE

        PI_DEBUG_GM (DBG_L2, "pyremote: main_context_set_entry_from_thread start %s %s %s", idle_data->ra.arglist[0], idle_data->ra.arglist[1], idle_data->ra.arglist[2] );
        // check PCS entries
        g_slist_foreach (gapp->RemoteEntryList, (GFunc) Check_ec, (gpointer)&(idle_data->ra));

        // check current active/open CONFIGURE elements
        g_slist_foreach (gapp->RemoteConfigureList, (GFunc) Check_conf, (gpointer)&(idle_data->ra));

        PI_DEBUG_GM (DBG_L2, "pyremote: main_context_set_entry_from_thread end");
        UNSET_WAIT_JOIN_MAIN;

        return G_SOURCE_REMOVE;
}

static PyObject* remote_set(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Setting ");
        gchar *parameter, *value;
        IDLE_from_thread_data idle_data;

        if (!PyArg_ParseTuple(args, "ss", &parameter, &value))
                return Py_BuildValue("i", -1);

        PI_DEBUG_GM (DBG_L2, "%s to %s", parameter, value );

        idle_data.ra.qvalue = 0.;
        gchar *list[] = { (char *)"set", parameter, value, NULL };
        idle_data.ra.arglist = list;
        idle_data.wait_join = true;

        PI_DEBUG_GM (DBG_L2, "IDLE START" );
        g_idle_add (main_context_set_entry_from_thread, (gpointer)&idle_data);
        PI_DEBUG_GM (DBG_L2, "IDLE WAIT JOIN" );
        WAIT_JOIN_MAIN;
        PI_DEBUG_GM (DBG_L2, "IDLE DONE" );

        return Py_BuildValue("i", 0);
}

static gboolean main_context_action_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gchar *parameter=NULL;
        gchar *value=NULL;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "s|s", &parameter, &value)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        PI_DEBUG(DBG_L2, "pyremote Action ** idle cb: value:" << parameter << ", " << (value?value:"N/A") );

        gchar *list[] = {(char *)"action", parameter, value, NULL};
        g_slist_foreach(gapp->RemoteActionList, (GFunc) CbAction_ra, (gpointer)list);
        idle_data->ret = 0;

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_action(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Action ") ;
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_action_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}


static gboolean main_context_idle_rtquery (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATIONS

        double u,v,w;
        gint64 ret = gapp->xsm->hardware->RTQuery ( idle_data->string, u,v,w);
        idle_data->vec[0]=u;
        idle_data->vec[1]=v;
        idle_data->vec[2]=w;
        idle_data->i64=ret;
        idle_data->ret = 0;

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static gint64 idle_rtquery(const gchar *m, double &x, double &y, double &z)
{
        PI_DEBUG(DBG_L2, "IDLE RTQuery ") ;
        IDLE_from_thread_data idle_data;
        idle_data.string = m;
        idle_data.wait_join = true;
        g_idle_add (main_context_idle_rtquery, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;

        x=idle_data.vec[0];
        y=idle_data.vec[1];
        z=idle_data.vec[2];
        return idle_data.i64;
}



// asks HwI via RTQuery for real time watches -- depends on HwI and it's capabilities/availabel options
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
static gboolean main_context_rtquery_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gchar *parameter=NULL;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "s", &parameter)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        double u,v,w;
        gint64 ret = gapp->xsm->hardware->RTQuery (parameter, u,v,w);
        idle_data->c=parameter[0];
        idle_data->vec[0]=u;
        idle_data->vec[1]=v;
        idle_data->vec[2]=w;
        idle_data->i64=ret;
        idle_data->ret = 0;

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_rtquery(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: RTQuery ") ;
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_rtquery_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;

        if (idle_data.c == 'm'){
                static char uu[10] = { 0,0,0,0, 0,0,0,0, 0,0};
                strncpy ((char*) uu, (char*)&idle_data.i64, 8);
                return Py_BuildValue("fffs", idle_data.vec[0], idle_data.vec[1], idle_data.vec[2], uu);
        } else
                return Py_BuildValue("fff",  idle_data.vec[0], idle_data.vec[1], idle_data.vec[2]);
}

// asks HwI via RTQuery for real time watches -- depends on HwI and it's capabilities/availabel options
static PyObject* remote_y_current(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: y_current ") ;
//gchar *parameter;

        gint y =gapp->xsm->hardware->RTQuery ();

        return Py_BuildValue("i", y);
}

static gboolean main_context_remote_moveto_scan_xy_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE

        if (gapp->xsm->hardware->MovetoXY
            (R2INT(gapp->xsm->Inst->XA2Dig(gapp->xsm->data.s.sx)),
             R2INT(gapp->xsm->Inst->YA2Dig(gapp->xsm->data.s.sy)))){
                if (gapp->xsm->ActiveScan)
                        gapp->xsm->ActiveScan->auto_display();
                return G_SOURCE_CONTINUE;
        }

        gapp->spm_update_all ();

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_moveto_scan_xy(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Moveto Scan XY ");
//remote_args ra;
        double x, y;

        if (!PyArg_ParseTuple(args, "dd", &x, &y))
                return Py_BuildValue("i", -1);

        PI_DEBUG(DBG_L2, x << ", " << y );

        if (x >= -gapp->xsm->data.s.rx/2 && x <=gapp->xsm->data.s.rx/2 &&
            y >= -gapp->xsm->data.s.ry/2 && y <=gapp->xsm->data.s.ry/2){

                gapp->xsm->data.s.sx = x;
                gapp->xsm->data.s.sy = y;

                IDLE_from_thread_data idle_data;
                idle_data.wait_join = true;
                g_timeout_add (50, main_context_remote_moveto_scan_xy_from_thread, (gpointer)&idle_data);
                WAIT_JOIN_MAIN;

                return Py_BuildValue("i", 0);
        } else {
                g_warning ("PyRemote: Set Scan XY: requested position (%g, %g) Ang out of current scan range.", x,y);
                return Py_BuildValue("i", -1);
        }
}


///////////////////////////////////////////////////////////////
// BLOCK II -- scan actions
///////////////////////////////////////////////////////////////

static gboolean main_context_emit_toolbar_action_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE

        PI_DEBUG_GM (DBG_L2, "pyremote: main_context_emit_toolbar_action  >%s<", idle_data->string);

        if (!strcmp (idle_data->string, "Toolbar_Scan_Start")){
                // CHECK FOR SCAN REPEAT OPTION SET, OVERRIDE AND DISABLE for this scanstart!
                GSettings *global_settings = g_settings_new (GXSM_RES_BASE_PATH_DOT ".global");
                g_settings_set_int (global_settings, "math-global-share-variable-repeatmode-override", 1);
                g_clear_object (&global_settings);
        }

        gapp->signal_emit_toolbar_action (idle_data->string);

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_startscan(PyObject *self, PyObject *args)
{
        PI_DEBUG_GM (DBG_L2, "pyremote: Starting scan");
        IDLE_from_thread_data idle_data;
        idle_data.string = "Toolbar_Scan_Start";
        idle_data.wait_join = true;
        g_idle_add (main_context_emit_toolbar_action_from_thread, (gpointer)&idle_data);
        PI_DEBUG_GM (DBG_L2, "pyremote: startscan idle job initiated");
        WAIT_JOIN_MAIN;
        PI_DEBUG_GM (DBG_L2, "pyremote: startscan idle job completed");
        return Py_BuildValue("i", 0);
}

static gboolean main_context_createscan_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;

        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        long ch;
        long sizex, sizey, sizev;
        double rangex, rangey;
        PyObject *obj;
        long append=0;

        idle_data->ret = -1;

        sizev=1; // try xyv
        if(!PyArg_ParseTuple (idle_data->args, "llllddOl", &ch, &sizex, &sizey, &sizev, &rangex, &rangey, &obj, &append)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        g_message ("Create Scan: %ld x %ld [x %ld], size %g x %g Ang from python array, append=%ld",sizex, sizey, sizev, rangex, rangey, append);

        Py_buffer view;
        gboolean rf=false;
        if (PyObject_CheckBuffer (obj)){
                if (PyObject_GetBuffer (obj, &view, PyBUF_SIMPLE)){
                        UNSET_WAIT_JOIN_MAIN;
                        return G_SOURCE_REMOVE;
                }
                rf=true;
        }
        if ( (long unsigned int)(view.len / sizeof(long)) != (long unsigned int)(sizex*sizey*sizev) ){
                g_message ("Create Scan: ERROR array len=%ld does not match nx x ny=%ld", view.len / sizeof(long), sizex*sizey);
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        g_message ("Create Scan: array len=%ld OK.", view.len / sizeof(long));


        //Scan *dst;
        //gapp->xsm->ActivateFreeChannel();
        //dst =gapp->xsm->GetActiveScan();
        Scan *dst =gapp->xsm->GetScanChannel (ch);

        if (dst){
                g_message ("Resize");
                dst->mem2d->Resize (sizex, sizey, sizev, ZD_FLOAT);

                dst->data.s.nx = sizex;
                dst->data.s.ny = sizey;
                dst->data.s.nvalues = sizev;
                dst->data.s.ntimes = 1;

                dst->data.s.dx = rangex/(sizex-1);
                dst->data.s.dy = rangey/(sizey-1);
                dst->data.s.dz = 1;
                dst->data.s.rx = rangex;
                dst->data.s.ry = rangey;

                // quick safety hack
                Scan *ref =gapp->xsm->GetScanChannel (0);
                if (ref){
                        dst->data.s.x0 = ref->data.s.x0;
                        dst->data.s.y0 = ref->data.s.y0;
                        dst->data.s.alpha = ref->data.s.alpha;
                }
                dst->data.ui.SetUser ("User");

                gchar *tmp=g_strconcat ("PyCreate ",
                                        NULL);
                dst->data.ui.SetComment (tmp);
                g_free (tmp);

                g_message ("Convert Data");
                /*Read*/

                long *buf = (long*)view.buf;

                long stridex = sizex;
                long strideyx = stridex*sizey;

                for(gint v=0; v<dst->mem2d->GetNv(); v++)
                        for(gint i=0; i<dst->mem2d->GetNy(); i++)
                                for(gint j=0; j<dst->mem2d->GetNx(); j++)
                                        dst->mem2d->data->Z( (double)buf[j+stridex*i+strideyx*v], j, i, v);
                dst->data.orgmode = SCAN_ORG_CENTER;
                dst->mem2d->data->MkXLookup (-dst->data.s.rx/2., dst->data.s.rx/2.);
                dst->mem2d->data->MkYLookup (-dst->data.s.ry/2., dst->data.s.ry/2.);
                dst->mem2d->data->MkVLookup (0, dst->data.s.nvalues-1);

                if (append > 0){
                        // append in time
                        //dst->GetDataSet(data);

                        double t=0., t0=0.;
                        if (!dst->TimeList) // reference to the first frame/image loaded
                                t0 = (double)dst->data.s.tStart;

                        t = (double)dst->data.s.tStart - t0;
                        dst->mem2d->add_layer_information (new LayerInformation ("name", 0., "PyScanCreate"));
                        dst->mem2d->add_layer_information (new LayerInformation ("t",t, "%.2f s"));
                        dst->mem2d->data->update_ranges (0);
                        dst->append_current_to_time_elements (-1, t);
                } else
                        dst->free_time_elements ();

                gapp->spm_update_all();
                dst->draw();

                if (rf)
                        PyBuffer_Release (&view);

                idle_data->ret = 0;
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        if (rf)
                PyBuffer_Release (&view);

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_createscan(PyObject *self, PyObject *args)
{
        PI_DEBUG_GM (DBG_L2, "pyremote: Creating scan");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;

        g_idle_add (main_context_createscan_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;

        return Py_BuildValue("i", idle_data.ret);
}

///////////////////////////////////////////////////////////////
static gboolean main_context_createscanf_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;

        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE

        PyObject *obj;

        long ch;
        long sizex, sizey, sizev;
        double rangex, rangey;
        long append=0;

        idle_data->ret = -1;

        sizev=1; // try xyv
        if(!PyArg_ParseTuple (idle_data->args, "llllddOl", &ch, &sizex, &sizey, &sizev, &rangex, &rangey, &obj, &append)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        g_message ("Create Scan Float: %ld x %ld [x %ld], size %g x %g Ang from python array, append=%ld",sizex, sizey, sizev, rangex, rangey, append);

        Py_buffer view;
        gboolean rf=false;

        if (PyObject_CheckBuffer (obj)){
                if (PyObject_GetBuffer (obj, &view, PyBUF_SIMPLE)){
                        UNSET_WAIT_JOIN_MAIN;
                        return G_SOURCE_REMOVE;
                }
                rf=true;
        }
        if ( (long unsigned int)(view.len / sizeof(float)) != (long unsigned int)(sizex*sizey*sizev) ) {
                g_message ("Create Scan: ERROR array len=%ld does not match nx x ny=%ld", view.len / sizeof(float), sizex*sizey);
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        g_message ("Create Scan: array len=%ld OK.", view.len / sizeof(float));

        //if (PyObject_AsWriteBuffer(the_array, (void **) &pbuf, (Py_ssize_t*)&blen))
        //        return Py_BuildValue("i", -1);
        //Scan *dst;
        //gapp->xsm->ActivateFreeChannel();
        //dst =gapp->xsm->GetActiveScan();

        Scan *dst =gapp->xsm->GetScanChannel (ch);
        if (dst){

                dst->mem2d->Resize (sizex, sizey, sizev, ZD_FLOAT);

                dst->data.s.nx = sizex;
                dst->data.s.ny = sizey;
                dst->data.s.nvalues = sizev;
                dst->data.s.ntimes = 1;

                dst->data.s.dx = rangex/(sizex-1);
                dst->data.s.dy = rangey/(sizey-1);
                dst->data.s.dz = 1;
                dst->data.s.rx = rangex;
                dst->data.s.ry = rangey;

                // quick safety hack
                Scan *ref =gapp->xsm->GetScanChannel (0);
                if (ref){
                        dst->data.s.x0 = ref->data.s.x0;
                        dst->data.s.y0 = ref->data.s.y0;
                        dst->data.s.alpha = ref->data.s.alpha;
                }
                dst->data.ui.SetUser("User");

                gchar *tmp = g_strconcat("PyCreate ", NULL);
                dst->data.ui.SetComment(tmp);
                g_free(tmp);

                /*Read */
                float *buf = (float*)view.buf;
                long stridex = sizex;
                long strideyx = stridex*sizey;
                for(gint v=0; v<dst->mem2d->GetNv(); v++)
                        for (gint i = 0; i < dst->mem2d->GetNy(); i++)
                                for (gint j = 0; j < dst->mem2d->GetNx(); j++)
                                        dst->mem2d->data->Z(buf[j + stridex * i + strideyx * v], j, i, v);

                dst->data.orgmode = SCAN_ORG_CENTER;
                dst->mem2d->data->MkXLookup(-dst->data.s.rx / 2.,
                                            dst->data.s.rx / 2.);
                dst->mem2d->data->MkYLookup(-dst->data.s.ry / 2.,
                                            dst->data.s.ry / 2.);
                dst->mem2d->data->MkVLookup (0, dst->data.s.nvalues-1);

                if (append > 0){
                        // append in time
                        //dst->GetDataSet(data);

                        double t=0., t0=0.;
                        if (!dst->TimeList) // reference to the first frame/image loaded
                                t0 = (double)dst->data.s.tStart;

                        t = (double)dst->data.s.tStart - t0;
                        dst->mem2d->add_layer_information (new LayerInformation ("name", 0., "PyScanCreate"));
                        dst->mem2d->add_layer_information (new LayerInformation ("t",t, "%.2f s"));
                        dst->mem2d->data->update_ranges (0);
                        dst->append_current_to_time_elements (-1, t);
                } else
                        dst->free_time_elements ();

                gapp->spm_update_all();
                dst->draw();
                dst = NULL;

                if (rf)
                        PyBuffer_Release (&view);

                idle_data->ret = 0;
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        if (rf)
                PyBuffer_Release (&view);

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}


static PyObject *remote_createscanf(PyObject * self, PyObject * args)
{
        PI_DEBUG(DBG_L2, "pyremote: Creating scanf");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;

        g_idle_add (main_context_createscanf_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;

        return Py_BuildValue("i", idle_data.ret);
}

static PyObject* remote_set_scan_unit(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: set scan zunit ");
        //remote_args ra;
        int ch;
        gchar *udim, *unitid, *ulabel;

        if (!PyArg_ParseTuple(args, "lsss", &ch, &udim, &unitid, &ulabel))
                return Py_BuildValue("i", -1);

        Scan *dst =gapp->xsm->GetScanChannel (ch);
        if (dst){

                UnitObj *u =gapp->xsm->MakeUnit (unitid, ulabel);
                g_message ("Set Scan Unit %c [%s] in %s", udim[0], u->Label(), u->Symbol());
                switch (udim[0]){
                case 'x': case 'X':
                        dst->data.SetXUnit(u); break;
                case 'y': case 'Y':
                        dst->data.SetYUnit(u); break;
                case 'z': case 'Z':
                        dst->data.SetZUnit(u); break;
                case 'l': case 'L': case 'v': case 'V':
                        dst->data.SetVUnit(u); break;
                case 't': case 'T':
                        dst->data.SetTimeUnit(u); break;
                default:
                        g_message ("Invalid Dimension Id given.");
                        break;
                }
                delete u;
        }
        else {
                g_message ("Invalid channel %d given.", ch);
                return Py_BuildValue("i", -1);
        }
        return Py_BuildValue("i", 0);
}

static PyObject* remote_set_scan_lookup(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: set scan lookup ");
//remote_args ra;
        int ch;
        gchar *udim;
        double start, end;

        if (!PyArg_ParseTuple(args, "lsdd", &ch, &udim, &start, &end))
                return Py_BuildValue("i", -1);

        Scan *dst =gapp->xsm->GetScanChannel (ch);
        if (dst){

                switch (udim[0]){
                case 'x': case 'X':
                        dst->mem2d->data->MkXLookup(start, end); break;
                case 'y': case 'Y':
                        dst->mem2d->data->MkYLookup(start, end); break;
                case 'l': case 'L': case 'v': case 'V':
                        dst->mem2d->data->MkVLookup(start, end); break;
                default:
                        g_message ("Invalid Dimension Id given.");
                        break;
                }
        }
        else
                return Py_BuildValue("i", -1);

        return Py_BuildValue("i", 0);
}


static PyObject* remote_getgeometry(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:getgeometry");

        long ch;

        if (!PyArg_ParseTuple (args, "l", &ch))
                return Py_BuildValue("ddddd", 0., 0., 0., 0., 0.);

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src)
                return Py_BuildValue("ddddd", src->data.s.rx, src->data.s.ry, src->data.s.x0, src->data.s.y0, src->data.s.alpha);
        else
                return Py_BuildValue("ddddd", 0., 0., 0., 0., 0.);
}

static PyObject* remote_getdifferentials(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:getdifferentials");

        long ch;

        if (!PyArg_ParseTuple (args, "l", &ch))
                return Py_BuildValue("dddd", 0., 0., 0., 0.);

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src)
                return Py_BuildValue("dddd", src->data.s.dx, src->data.s.dy, src->data.s.dz, src->data.s.dl);
        else
                return Py_BuildValue("dddd", 0., 0., 0., 0.);
}

static PyObject* remote_getdimensions(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:getdimensions");

        long ch;

        if (!PyArg_ParseTuple (args, "l", &ch))
                return Py_BuildValue("llll", -1, -1, -1, -1);

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src)
                return Py_BuildValue("llll", src->mem2d->GetNx (), src->mem2d->GetNy (), src->mem2d->GetNv (), src->number_of_time_elements ());
        else
                return Py_BuildValue("llll", -1, -1, -1, -1);
}

static PyObject* remote_getdatapkt(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:getdatapkt");

        long ch;
        double x, y, v, t;

        if (!PyArg_ParseTuple (args, "ldddd", &ch, &x, &y, &v, &t))
                return Py_BuildValue("d", 0.);

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src)
                if (t > 0.)
                        return Py_BuildValue("d", src->mem2d->GetDataPktInterpol (x,y,v, src, (int)(t)));
                else
                        return Py_BuildValue("d", src->mem2d->GetDataPktInterpol (x,y,v));
        else
                return Py_BuildValue("d", 0.);
}

static PyObject* remote_putdatapkt(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: putdatapkt");
        long ch;
        long x, y, v, t;
        double value;

        if (!PyArg_ParseTuple (args, "dlllll", &value, &ch, &x, &y, &v, &t))
                return Py_BuildValue("i", -1);

        Scan *dst =gapp->xsm->GetScanChannel (ch);
        if (dst){
                dst->mem2d->PutDataPkt (value, x,y,v);
                return Py_BuildValue("i", 0);
        } else
                return Py_BuildValue("i", -1);
}

//{"get_slice", remote_getslice, METH_VARARGS, "Get Slice/Image: [nx,ny,array]=gxsm.get_slice (ch, v, t, yi, yn)"},

static PyObject* remote_getslice(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: getslice");
        long ch,v,t,yi,yn;

        //PyObject *obj;

        if (!PyArg_ParseTuple (args, "lllll", &ch, &v, &t, &yi, &yn))
                return Py_BuildValue("i", -1);

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src && (yi+yn) <= src->mem2d->GetNy()){
                g_message ("remote_getslice from mem2d scan data in (dz scaled to unit) CH%d, Ys=%d Yf=%d", (int)ch, (int)yi, (int)(yi+yn));

                // PyObject* PyArray_SimpleNewFromData(int nd, npy_intp const* dims, int typenum, void* data);
                // PyObject *PyArray_FromDimsAndData(int n_dimensions, int dimensions[n_dimensions], int item_type, char *data);
                npy_intp dims[2];
                dims[0] = yn;
                dims[1] = src->mem2d->GetNx ();
                g_message ("Creating PyArray: shape %d , %d", (int)dims[0], (int)dims[1]);
                double *darr2 = (double*) malloc(sizeof(double) * dims[0]*dims[1]);
                double *dp=darr2;
                int yf = yi+yn;

                for (int y=yi; y<yf; ++y)
                        for (int x=0; x<dims[1]; ++x)
                                *dp++ = src->mem2d->data->Z(x,y)*src->data.s.dz;

                PyObject* pyarr = PyArray_SimpleNewFromData(2, dims, NPY_DOUBLE, (void*)darr2);
                PyArray_ENABLEFLAGS((PyArrayObject*) pyarr, NPY_ARRAY_OWNDATA);
                return Py_BuildValue("O", pyarr); // Python code will receive the array as numpy array.
        } else
                return Py_BuildValue("i", -1);

        return Py_BuildValue("i", 0);
}

static PyObject* remote_get_x_lookup(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:get_x_lookup");

        long ch, i;

        if (!PyArg_ParseTuple (args, "ll", &ch, &i))
                return Py_BuildValue("d", 0.);

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src)
                return Py_BuildValue("d", src->mem2d->data->GetXLookup(i));
        else
                return Py_BuildValue("d", 0.);
}

static PyObject* remote_get_y_lookup(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:get_x_lookup");

        long ch, i;

        if (!PyArg_ParseTuple (args, "ll", &ch, &i))
                return Py_BuildValue("d", 0.);

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src)
                return Py_BuildValue("d", src->mem2d->data->GetYLookup(i));
        else
                return Py_BuildValue("d", 0.);
}

static PyObject* remote_get_v_lookup(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:get_x_lookup");

        long ch, i;

        if (!PyArg_ParseTuple (args, "ll", &ch, &i))
                return Py_BuildValue("d", 0.);

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src)
                return Py_BuildValue("d", src->mem2d->data->GetVLookup(i));
        else
                return Py_BuildValue("d", 0.);
}

static PyObject* remote_set_global_share_parameter(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:set_global_share_parmeter");

        gchar *key;
        double x;

        if (!PyArg_ParseTuple (args, "sd", &key, &x))
                return Py_BuildValue("d", 0.);

        GSettings *global_settings = g_settings_new (GXSM_RES_BASE_PATH_DOT ".global");
        //g_settings_set_double (global_settings, "math-global-share-variable-radius", edge_radius);
        g_message ("Previous value of key global.%s = %g, new = %g", key, g_settings_get_double (global_settings, key), x);
        g_settings_set_double (global_settings, key, x);
        g_clear_object (&global_settings);

        return Py_BuildValue("d", x);
}


static PyObject* remote_set_x_lookup(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:get_x_lookup");

        long ch, i;
        double v;

        if (!PyArg_ParseTuple (args, "lld", &ch, &i, &v))
                return Py_BuildValue("d", 0.);

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src){
                src->mem2d->data->SetXLookup(i, v);
                return Py_BuildValue("d", src->mem2d->data->GetXLookup(i));
        } else
                return Py_BuildValue("d", 0.);
}

static PyObject* remote_set_y_lookup(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:get_x_lookup");

        long ch, i;
        double v;

        if (!PyArg_ParseTuple (args, "lld", &ch, &i, &v))
                return Py_BuildValue("d", 0.);

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src){
                src->mem2d->data->SetYLookup(i, v);
                return Py_BuildValue("d", src->mem2d->data->GetYLookup(i));
        } else
                return Py_BuildValue("d", 0.);
}

static PyObject* remote_set_v_lookup(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:get_x_lookup");

        long ch, i;
        double v;

        if (!PyArg_ParseTuple (args, "lld", &ch, &i, &v))
                return Py_BuildValue("d", 0.);

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src){
                src->mem2d->data->SetVLookup(i, v);
                return Py_BuildValue("d", src->mem2d->data->GetVLookup(i));
        } else
                return Py_BuildValue("d", 0.);
}


static PyObject* remote_getobject(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:getobject");

        long ch, nth;

        if (!PyArg_ParseTuple (args, "ll", &ch, &nth))
                return Py_BuildValue("s", "Invalid Parameters. [ll]: ch, nth");

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src){
                int n_obj = src->number_of_object ();
                if (nth < n_obj){
                        scan_object_data *obj_data = src->get_object_data (nth);
                        double xy[6] = {0.,0., 0.,0., 0.,0.};
                        if (obj_data){
                                obj_data->get_xy_i_pixel (0, xy[0], xy[1]);
                                if (obj_data->get_num_points () > 1){
                                        obj_data->get_xy_i_pixel (1, xy[2], xy[3]);
                                        if (obj_data->get_num_points () > 2){
                                                obj_data->get_xy_i_pixel (2, xy[4], xy[5]);
                                                return Py_BuildValue("sdddddd", obj_data->get_name (), xy[0], xy[1], xy[2], xy[3], xy[4], xy[5]);
                                        } else
                                                return Py_BuildValue("sdddd", obj_data->get_name (), xy[0], xy[1], xy[2], xy[3]);
                                } else
                                        return Py_BuildValue("sdd", obj_data->get_name (), xy[0], xy[1]);
                        }
                        else
                                return Py_BuildValue("sdd", "None", 0, 0);
                }
        }
        return Py_BuildValue("s", "None");
}

static gboolean main_context_getobject_action_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        long ch;
        gchar *objnameid;
        gchar *action;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple (idle_data->args, "lss", &ch, &objnameid, &action)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (src){
                int n_obj = src->number_of_object ();
                for (int i=0; i < n_obj; ++i){
                        scan_object_data *obj_data = src->get_object_data (i);
                        if (obj_data){
                                if (!strcmp (action, "REMOVE-ALL")){
                                        if (!strncmp (obj_data->get_name (), objnameid, strlen(objnameid))){ // part match?
                                                ViewControl *vc = src->view->Get_ViewControl ();
                                                vc->remove_object ((VObject *)obj_data, vc);
                                                n_obj = src->number_of_object ();
                                                --i;
                                                idle_data->ret = 0;
                                                continue;
                                        }
                                }
                                if (!strcmp (obj_data->get_name (), objnameid)){ // match?
                                        if (!strcmp (action, "GET-COORDS")){
                                                obj_data->SetUpScan ();
                                                idle_data->ret = 0;
                                        }
                                        else if (!strcmp (action, "SET-OFFSET")){
                                                obj_data->set_offset ();
                                                idle_data->ret = 0;
                                        }
                                        else if (!strncmp (action, "SET-LABEL-TO:",13)){
                                                obj_data->set_object_label (&action[13]);
                                                idle_data->ret = 0;
                                        }
                                        else if (!strcmp (action, "REMOVE")){
                                                ViewControl *vc = src->view->Get_ViewControl ();
                                                vc->remove_object ((VObject *)obj_data, vc);
                                                idle_data->ret = 0;
                                        }
                                        UNSET_WAIT_JOIN_MAIN;
                                        return G_SOURCE_REMOVE;
                                }
                        }
                }
        }
        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_getobject_action(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote:getobject_getcoords_setup_scan");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_getobject_action_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        if (idle_data.ret)
                return Py_BuildValue("s", "Invalid Parameters. [lls]: ch, objname-id, action=[GET_COORDS, SET-OFFSET, SET-LABEL-TO:{LABEL}, REMOVE]");
        else
                return Py_BuildValue("s", "OK");

}

static gboolean main_context_addmobject_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        long ch,grp,x,y;
        double size = 1.0;
        gchar *id;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple (idle_data->args, "lsllld", &ch, &id, &grp, &x, &y, &size)){
                //return Py_BuildValue("s", "Invalid Parameters. [ll]: ch, nth");
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        const gchar *marker_group[] = {
                "*Marker:red", "*Marker:green", "*Marker:blue", "*Marker:yellow", "*Marker:cyan", "*Marker:magenta",
                NULL };
        PI_DEBUG(DBG_L2, "pyremote:putobject");

        Scan *src =gapp->xsm->GetScanChannel (ch);
        if (!strncmp(id,"Rectangle",9)){
                VObject *vo;
                double xy[4];
                gfloat c[4] = { 1.,0.,0.,1.};
                int spc[2][2] = {{0,0},{0,0}};
                int sp00[2] = {1,1};
                src->Pixel2World ((int)round(x-size/2), (int)round(y-size/2), xy[0], xy[1]);
                src->Pixel2World ((int)round(x+size/2), (int)round(y+size/2), xy[2], xy[3]);
                (src->view->Get_ViewControl ())->AddObject (vo = new VObRectangle ((src->view->Get_ViewControl ())->canvas, xy, FALSE, VOBJ_COORD_ABSOLUT, id, 1.0));
                vo->set_obj_name (id);
                vo->set_custom_label_font ("Sans Bold 12");
                vo->set_custom_label_color (c);
                if (grp>0){
                        gfloat fillcolor[4];
                        gfloat outlinecolor[4];
                        for(int j=0; j<4; j++){
                                int sh = 24-(8*j);
                                fillcolor[j] = outlinecolor[j] = (gfloat)((grp&(0xff << sh) >> sh)) / 256.;
                        }
                        outlinecolor[3] = 0.0;
                        vo->set_color_to_custom (fillcolor, outlinecolor);
                }
                vo->set_on_spacetime  (sp00[0] ? FALSE:TRUE, spc[0]);
                vo->set_off_spacetime (sp00[1] ? FALSE:TRUE, spc[1]);
                vo->show_label (true);
                vo->lock_object (true);
                vo->remake_node_markers ();
        } else if (!strncmp(id,"Point",5)){
                VObject *vo;
                double xy[2];
                gfloat c[4] = { 1.,0.,0.,1.};
                int spc[2][2] = {{0,0},{0,0}};
                int sp00[2] = {1,1};
                src->Pixel2World ((int)round(x), (int)round(y), xy[0], xy[1]);
                (src->view->Get_ViewControl ())->AddObject (vo = new VObPoint ((src->view->Get_ViewControl ())->canvas, xy, FALSE, VOBJ_COORD_ABSOLUT, id, 1.0));
                vo->set_obj_name (id);
                vo->set_custom_label_font ("Sans Bold 12");
                vo->set_custom_label_color (c);
                if (grp>0){
                        gfloat fillcolor[4];
                        gfloat outlinecolor[4];
                        for(int j=0; j<4; j++){
                                int sh = 24-(8*j);
                                fillcolor[j] = outlinecolor[j] = (gfloat)((grp&(0xff << sh) >> sh)) / 256.;
                        }
                        outlinecolor[3] = 0.0;
                        vo->set_color_to_custom (fillcolor, outlinecolor);
                }
                vo->set_on_spacetime  (sp00[0] ? FALSE:TRUE, spc[0]);
                vo->set_off_spacetime (sp00[1] ? FALSE:TRUE, spc[1]);
                vo->show_label (true);
                vo->lock_object (true);
                vo->remake_node_markers ();
        } else if (grp == -1 || !strncmp(id,"xy",2)){
                if (size < 0. || size > 10.) size = 1.0;
                grp = 5;
                VObject *vo;
                double xy[2];
                gfloat c[4] = { 1.,0.,0.,1.};
                int spc[2][2] = {{0,0},{0,0}};
                int sp00[2] = {1,1};
                double px,py,pz;
                // gapp->xsm->hardware->RTQuery ("P", px, py, pz); // get Tip Position in pixels
                idle_rtquery ("P", px, py, pz); // get Tip Position in pixels
                src->Pixel2World ((int)round(px), (int)round(py), xy[0], xy[1]);
                gchar *lab = g_strdup_printf ("M%s XYZ=%g,%g,%g",id, px,py,pz);
                (src->view->Get_ViewControl ())->AddObject (vo = new VObPoint ((src->view->Get_ViewControl ())->canvas, xy, FALSE, VOBJ_COORD_ABSOLUT, lab, size));
                g_free (lab);
                vo->set_obj_name (marker_group[grp]);
                vo->set_custom_label_font ("Sans Bold 12");
                vo->set_custom_label_color (c);
                vo->set_on_spacetime  (sp00[0] ? FALSE:TRUE, spc[0]);
                vo->set_off_spacetime (sp00[1] ? FALSE:TRUE, spc[1]);
                vo->show_label (true);
                vo->lock_object (true);
                vo->remake_node_markers ();
        } else {
                if (size < 0. || size > 10.) size = 1.0;

                if (grp < 0 || grp > 6) grp=0; // silently set 0 if out of range

                if (src->view->Get_ViewControl ()){
                        VObject *vo;
                        double xy[2];
                        gfloat c[4] = { 1.,0.,0.,1.};
                        int spc[2][2] = {{0,0},{0,0}};
                        int sp00[2] = {1,1};
                        src->Pixel2World ((int)round(x), (int)round(y), xy[0], xy[1]);
                        gchar *lab = g_strdup_printf ("M%s",id);
                        (src->view->Get_ViewControl ())->AddObject (vo = new VObPoint ((src->view->Get_ViewControl ())->canvas, xy, FALSE, VOBJ_COORD_ABSOLUT, lab, size));
                        g_free (lab);
                        vo->set_obj_name (marker_group[grp]);
                        vo->set_custom_label_font ("Sans Bold 12");
                        vo->set_custom_label_color (c);
                        vo->set_on_spacetime  (sp00[0] ? FALSE:TRUE, spc[0]);
                        vo->set_off_spacetime (sp00[1] ? FALSE:TRUE, spc[1]);
                        vo->show_label (true);
                        vo->lock_object (true);
                        vo->remake_node_markers ();
                }
        }
        idle_data->ret = 0;

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}


static PyObject* remote_addmobject(PyObject *self, PyObject *args)
{
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_addmobject_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        if (idle_data.ret)
                return Py_BuildValue("s", "Invalid Parameters. [ll]: ch, nth");
        else
                return Py_BuildValue("s", "OK");
}

static PyObject* remote_stopscan(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Stopping scan");
        IDLE_from_thread_data idle_data;
        idle_data.string = "Toolbar_Scan_Stop";
        idle_data.wait_join = true;
        g_idle_add (main_context_emit_toolbar_action_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", 0);
}

static PyObject* remote_waitscan(PyObject *self, PyObject *args)
{
        double x,y,z;
        long block = 0;
        PI_DEBUG_GM (DBG_L2, "pyremote: wait scan");
        if (PyArg_ParseTuple (args, "l", &block)){
                g_usleep(50000);
                if(idle_rtquery ("W",x,y,z) ){
                        if (block){
                                PI_DEBUG_GM (DBG_L2, "pyremote: wait scan (block=%d)-- blocking until ready.", (int) block);
                                while(idle_rtquery ("W",x,y,z) ){
                                        PI_DEBUG_GM (DBG_L2, "pyremote: wait scan blocking, line = %d",gapp->xsm->hardware->RTQuery () );
                                        g_usleep(100000);
                                }
                        }
                        return Py_BuildValue("i",gapp->xsm->hardware->RTQuery () ); // return current y_index of scan
                }else
                        return Py_BuildValue("i", -1); // no scan in progress
        } else {
                PI_DEBUG_GM (DBG_L2, "pyremote: wait scan -- default: blocking until ready.");
                while(idle_rtquery ("W",x,y,z) ){
                        g_usleep(100000);
                        PI_DEBUG_GM (DBG_L2, "pyremote: wait scan, default: block, line = %d",gapp->xsm->hardware->RTQuery () );
                }
        }
        return Py_BuildValue("i", -1); // no scan in progress
}

static PyObject* remote_scaninit(PyObject *self, PyObject *args)
{
        PI_DEBUG_GM (DBG_L2, "pyremote: Initializing scan");
        IDLE_from_thread_data idle_data;
        idle_data.string = "Toolbar_Scan_Init";
        idle_data.wait_join = true;
        g_idle_add (main_context_emit_toolbar_action_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", 0);
}

static PyObject* remote_scanupdate(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Updating scan (hardware)");
        IDLE_from_thread_data idle_data;
        idle_data.string = "Toolbar_Scan_UpdateParam";
        idle_data.wait_join = true;
        g_idle_add (main_context_emit_toolbar_action_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", 0);
}

static PyObject* remote_scanylookup(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Scanylookup");
        int value1 = 0;
        double value2 = 0.0;
        if (!PyArg_ParseTuple(args, "ld", &value1, &value2))
                return Py_BuildValue("i", -1);
        PI_DEBUG(DBG_L2,  value1 << " and " << value2 );
        if(value1 && value2){
                gchar *cmd = NULL;
                cmd = g_strdup_printf ("2 %d %g", value1, value2);
                gapp->PutPluginData (cmd);
                //gapp->signal_emit_toolbar_action ("Toolbar_Scan_SetYLookup");
                IDLE_from_thread_data idle_data;
                idle_data.string = "Toolbar_Scan_SetYLookup";
                idle_data.wait_join = true;
                g_idle_add (main_context_emit_toolbar_action_from_thread, (gpointer)&idle_data);
                WAIT_JOIN_MAIN;
                g_free (cmd);
        }
        return Py_BuildValue("i", 0);
}

static PyObject* remote_scanline(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Scan line");
        int value1 = 0, value2 = 0, value3 = 0;
        if (!PyArg_ParseTuple(args, "lll", &value1, &value2, &value3))
                return Py_BuildValue("i", -1);
        PI_DEBUG(DBG_L2,  value1 << " and " << value2 << " and " << value3);
        PI_DEBUG(DBG_L2, "pyremote: Warning toolbar NYI");
        if(value1){
                gchar *cmd = NULL;
                if(value2 && value3){
                        cmd = g_strdup_printf ("3 %d %d %d",
                                               value1,
                                               value2,
                                               value3);
                        gapp->PutPluginData (cmd);
                        //gapp->signal_emit_toolbar_action ("Toolbar_Scan_Partial_Line");
                        IDLE_from_thread_data idle_data;
                        idle_data.string = "Toolbar_Scan_Partial_Line";
                        idle_data.wait_join = true;
                        g_idle_add (main_context_emit_toolbar_action_from_thread, (gpointer)&idle_data);
                        WAIT_JOIN_MAIN;
                }
                else{
                        cmd = g_strdup_printf ("d %d",
                                               value1);
                        gapp->PutPluginData (cmd);
                        //gapp->signal_emit_toolbar_action ("Toolbar_Scan_Line");
                        IDLE_from_thread_data idle_data;
                        idle_data.string = "Toolbar_Scan_Line";
                        idle_data.wait_join = true;
                        g_idle_add (main_context_emit_toolbar_action_from_thread, (gpointer)&idle_data);
                        WAIT_JOIN_MAIN;
                }
                g_free (cmd);
        }
        return Py_BuildValue("i", 0);
}

///////////////////////////////////////////////////////////////
// BLOCK III  -- file IO
///////////////////////////////////////////////////////////////

#if 0 // TEMPLATE
static gboolean main_context_TEMPLATE_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        long channel = 0;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "l", &channel)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        gapp->xsm->ActivateChannel ((int)channel);
        idle_data->ret = 0;

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}
{
        IDLE_from_thread_data idle_data;
        idle_data.string = "Toolbar_Scan_Partial_Line";
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_TEMPLATE_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}
#endif

static gboolean main_context_autosave_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gapp->enter_thread_safe_no_gui_mode();
        gapp->auto_save_scans ();
        gapp->exit_thread_safe_no_gui_mode();

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_autosave(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Save All/Update");
        IDLE_from_thread_data idle_data;
        idle_data.wait_join = true;
        g_idle_add (main_context_autosave_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", (long)gapp->xsm->hardware->RTQuery ());
}


static gboolean main_context_autoupdate_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gapp->enter_thread_safe_no_gui_mode();
        gapp->auto_update_scans ();
        gapp->exit_thread_safe_no_gui_mode();

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_autoupdate(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Save All/Update");
        IDLE_from_thread_data idle_data;
        idle_data.wait_join = true;
        g_idle_add (main_context_autoupdate_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", (long)gapp->xsm->hardware->RTQuery ());
}


static gboolean main_context_save_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gapp->xsm->save(MANUAL_SAVE_AS, NULL, -1, TRUE);

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_save(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Save");
        IDLE_from_thread_data idle_data;
        idle_data.wait_join = true;
        g_idle_add (main_context_save_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", 0);
}


static gboolean main_context_saveas_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gchar* fname = NULL;
        long channel = 0;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "ls", &channel, &fname)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        if (fname){
                gapp->xsm->save (MANUAL_SAVE_AS, g_strdup(fname), channel, TRUE);
                //gapp->xsm->save(TRUE, fname, channel);
                idle_data->ret = 0;
        }

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_saveas(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Save As ");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_saveas_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static gboolean main_context_load_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gchar* fname = NULL;
        long channel = 0;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "ls", &channel, &fname)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        if (fname){
                gapp->xsm->ActivateChannel (channel);
                gapp->xsm->load (fname);
                idle_data->ret = 0;
        }

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_load(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Loading ");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_load_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static gboolean main_context_import_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gchar* fname = NULL;
        long channel = 0;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "ls", &channel, &fname)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        if (fname){
                gapp->xsm->ActivateChannel (channel);
                gapp->xsm->load (fname);
                idle_data->ret = 0;
        }

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}


static PyObject* remote_import(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Importing ");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_import_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static gboolean main_context_export_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gchar* fname = NULL;
        long channel = 0;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "ls", &channel, &fname)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        if (fname){
                gapp->xsm->ActivateChannel (channel);
                gapp->xsm->gnuexport (fname);
                idle_data->ret = 0;
        }

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_export(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Exporting ");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_export_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static gboolean main_context_save_drawing_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gchar* fname = NULL;
        long channel = 0;
        long time_index = 0;
        long layer_index = 0;

        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "llls", &channel, &time_index, &layer_index, &fname)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        if (fname){
                gapp->xsm->ActivateChannel (channel);
                ViewControl* vc =gapp->xsm->GetActiveScan()->view->Get_ViewControl();

                if (!vc){
                        UNSET_WAIT_JOIN_MAIN;
                        return G_SOURCE_REMOVE;
                }

                gapp->xsm->data.display.vlayer = layer_index;
                gapp->xsm->data.display.vframe = time_index;
                App::spm_select_layer (NULL, gapp);
                App::spm_select_time (NULL, gapp);

                gapp->xsm->GetActiveScan()->mem2d_time_element (time_index)->SetLayer (layer_index);
                vc->view_file_save_drawing (fname);
                idle_data->ret = 0;
        }

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_save_drawing (PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: save drawing ");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_save_drawing_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

///////////////////////////////////////////////////////////////
// BLOCK IV
///////////////////////////////////////////////////////////////

static gboolean main_context_set_view_indices_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        long channel = 0;
        long time_index = 0;
        long layer_index = 0;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "lll", &channel, &time_index, &layer_index)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        gapp->xsm->ActivateChannel (channel);
        gapp->xsm->data.display.vlayer = layer_index;
        gapp->xsm->data.display.vframe = time_index;
        App::spm_select_layer (NULL, gapp);
        App::spm_select_time (NULL, gapp);
        gapp->xsm->GetActiveScan()->mem2d_time_element (time_index)->SetLayer (layer_index);
        idle_data->ret = 0;

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}



static PyObject* remote_set_view_indices (PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: save drawing ");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_set_view_indices_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}



static gboolean main_context_autodisplay_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gapp->enter_thread_safe_no_gui_mode();
        gapp->xsm->ActiveScan->auto_display();
        gapp->exit_thread_safe_no_gui_mode();

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}


static PyObject* remote_autodisplay(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Autodisplay");
        if (!gapp->xsm->ActiveScan)
                return Py_BuildValue("i", -1);

        IDLE_from_thread_data idle_data;
        idle_data.wait_join = true;
        g_idle_add (main_context_autodisplay_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", 0);
}

static PyObject* remote_chfname(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Chfname ");
        long channel = 0;
        if (!PyArg_ParseTuple(args, "l", &channel))
                return Py_BuildValue("i", -1);
        int ch=channel;
        if (ch >= 100) ch -= 100;
        if (gapp->xsm->GetScanChannel(ch)){
                const gchar *tmp = gapp->xsm->GetScanChannel (ch)->storage_manager.get_filename();
                if (channel >= 100)
                        return Py_BuildValue ("s", tmp ? tmp : gapp->xsm->GetScanChannel (ch)->data.ui.originalname);
                else
                        return Py_BuildValue ("s", tmp ? tmp : gapp->xsm->GetScanChannel (ch)->data.ui.name);
        } else
                return Py_BuildValue ("s", "EE: invalid channel");
}

static gboolean main_context_chmodea_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        long channel = 0;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "l", &channel)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        gapp->xsm->ActivateChannel ((int)channel);
        idle_data->ret = 0;

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_chmodea(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Chmode a ");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_chmodea_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static gboolean main_context_chmodex_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        long channel = 0;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "l", &channel)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        gapp->xsm->SetMode ((int)channel, ID_CH_M_X);
        idle_data->ret = 0;

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_chmodex(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Chmode x ");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_chmodex_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static gboolean main_context_chmodem_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        long channel = 0;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "l", &channel)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        gapp->xsm->SetMode ((int)channel, ID_CH_M_MATH);
        idle_data->ret = 0;

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_chmodem(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Chmode m ");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_chmodem_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static gboolean main_context_chmoden_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        long channel = 0;
        long mode = 0;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "ll", &channel, &mode)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        idle_data->ret = 0;
        gapp->xsm->SetMode ((int)channel, ID_CH_M_X+mode);
        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_chmoden(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Chmode n ");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_chmoden_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static PyObject* remote_chmodeno(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Chmode no -- not available");
        return Py_BuildValue("i", 0);
#if 0 // not thread safe, may trigger GUI / dialog
        long channel = 0;
        if (!PyArg_ParseTuple(args, "l", &channel))
                return Py_BuildValue("i", -1);
        return Py_BuildValue ("i",gapp->xsm->SetView ((int)channel, ID_CH_V_NO));
#endif
}

static PyObject* remote_chview1d(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Chmode 1d -- not available");
        return Py_BuildValue("i", 0);
#if 0 // not thread safe, may trigger GUI / dialog
        PI_DEBUG(DBG_L2, "pyremote: Chview 1d.");
        long channel = 0;
        if (!PyArg_ParseTuple(args, "l", &channel))
                return Py_BuildValue("i", -1);
        return Py_BuildValue ("i",gapp->xsm->SetView (channel, ID_CH_V_PROFILE));
#endif
}

static PyObject* remote_chview2d(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Chview 2d -- not avialable");
        return Py_BuildValue("i", 0);
#if 0 // not thread safe, may trigger GUI / dialog
        long channel = 0;
        if (!PyArg_ParseTuple(args, "l", &channel))
                return Py_BuildValue("i", -1);
        return Py_BuildValue ("i",gapp->xsm->SetView ((int)channel, ID_CH_V_GREY));
#endif
}

static PyObject* remote_chview3d(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Chview 3d -- not available");
        return Py_BuildValue("i", 0);
#if 0 // not thread safe, may trigger GUI / dialog
        long channel = 0;
        if (!PyArg_ParseTuple(args, "l", &channel))
                return Py_BuildValue("i", -1);
        return Py_BuildValue ("i",gapp->xsm->SetView ((int)channel, ID_CH_V_SURFACE));
#endif
}

static gboolean main_context_setvm_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gapp->xsm->SetVM (idle_data->ret);
        idle_data->ret = 0;
        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}


static PyObject* remote_quick(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Quick");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.ret = SCAN_V_QUICK; // mode here
        idle_data.wait_join = true;
        g_idle_add (main_context_setvm_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static PyObject* remote_direct(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Direkt");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.ret = SCAN_V_DIRECT; // mode here
        idle_data.wait_join = true;
        g_idle_add (main_context_setvm_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static PyObject* remote_log(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Log");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.ret = SCAN_V_LOG; // mode here
        idle_data.wait_join = true;
        g_idle_add (main_context_setvm_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static gboolean main_context_math_crop_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        long chsrc = 0;
        long chdst = 1;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "ll", &chsrc, &chdst)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        if (!gapp->xsm->GetScanChannel (chdst))
                gapp->xsm->ActivateChannel (chdst);

        if (chsrc != chdst && gapp->xsm->GetScanChannel(chsrc) && gapp->xsm->GetScanChannel(chdst)){
                if (CropScan (gapp->xsm->GetScanChannel (chsrc), gapp->xsm->GetScanChannel (chdst)) == MATH_OK)
                        idle_data->ret = 0;

                gapp->enter_thread_safe_no_gui_mode();
                gapp->xsm->ActivateChannel (chdst);
                gapp->xsm->ActiveScan->auto_display();
                gapp->exit_thread_safe_no_gui_mode();
        }

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_crop(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Crop");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_math_crop_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

///////////////////////////////////////////////////////////////
// BLOCK V
// unitbz .   DONE
// unitvolt .  DONE
// unitev .  DONE
// units .  DONE
///////////////////////////////////////////////////////////////

static PyObject* remote_unitbz(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: unitbz");
        gapp->xsm->SetModeFlg(MODE_BZUNIT);
        gapp->xsm->ClrModeFlg(MODE_VOLTUNIT);
        return Py_BuildValue("i", 0);
}
static PyObject* remote_unitvolt(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: unitvolt");
        gapp->xsm->SetModeFlg(MODE_VOLTUNIT);
        gapp->xsm->ClrModeFlg(MODE_BZUNIT);
        return Py_BuildValue("i", 0);
}
static PyObject* remote_unitev(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: unitev");
        gapp->xsm->SetModeFlg(MODE_ENERGY_EV);
        gapp->xsm->ClrModeFlg(MODE_ENERGY_S);
        return Py_BuildValue("i", 0);
}
static PyObject* remote_units(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: units");
        gapp->xsm->SetModeFlg(MODE_ENERGY_S);
        gapp->xsm->ClrModeFlg(MODE_ENERGY_EV);
        return Py_BuildValue("i", 0);
}

///////////////////////////////////////////////////////////////
// BLOCK VI
// echo S      DONE
// logev S      DONE
// da0 X      DONE, commented out
// signal S      DONE
// more actions by plugins S  NYI
///////////////////////////////////////////////////////////////

static PyObject* remote_echo(PyObject *self, PyObject *args)
{

        PI_DEBUG(DBG_L2, "pyremote: Echo.");
        gchar* line1;
        if (!PyArg_ParseTuple(args, "s", &line1))
                return Py_BuildValue("i", -1);
        /*Change the Debuglevel to: print always.*/
        g_message ("%s", line1);
        return Py_BuildValue("i", 0);
}


static gboolean main_context_logev_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gchar* zeile;
        if (!PyArg_ParseTuple(idle_data->args, "s", &zeile)){
                idle_data->ret = -1;
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        if(zeile){
                gapp->monitorcontrol->LogEvent((char *)"RemoteLogEv", zeile);
        }else{
                gapp->monitorcontrol->LogEvent((char *)"RemoteLogEv", (char *)"--");
        }
        idle_data->ret = 0;
        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_logev(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Log ev.");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.ret = SCAN_V_LOG; // mode here
        idle_data.wait_join = true;
        g_idle_add (main_context_logev_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static gboolean main_context_progress_info_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        double d;
        gchar* info;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "sd", &info, &d)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        if(info){
                if (d < 0.)
                        gapp->progress_info_new (info, 1);
                else {
                        gapp->progress_info_set_bar_fraction (d, 1);
                        gapp->progress_info_set_bar_text (info, 1);
                }
                if (d > 1.){
                        gapp->progress_info_close ();
                }
                idle_data->ret = 0;
        }

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_progress_info(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: progress_info");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_progress_info_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}


static gboolean main_context_add_layer_information_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gchar* info;
        long layer = 0;
        long nth = -1;

        if (!PyArg_ParseTuple(idle_data->args, "sll", &info, &layer, &nth)){
                nth = -1;
                idle_data->ret = -1;
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        /*
          if (!PyArg_ParseTuple(idle_data->args, "sl", &info, &layer)){
          idle_data->ret = -1;
          UNSET_WAIT_JOIN_MAIN;
          return G_SOURCE_REMOVE;
          }
        */
        PI_DEBUG(DBG_L2, info << " to layer info, lv=" << layer << "  nth=" << nth);

        if (nth >= 0)
                if (gapp->xsm->ActiveScan)
                        if(info && layer>=0 && layer<gapp->xsm->GetActiveScan() -> mem2d->GetNv()){
                                LayerInformation *li = gapp->xsm->GetActiveScan() -> mem2d->get_layer_information_object ((int)nth);
                                g_message ("%s",info);
                                if (li) li->replace (info);
                        }
                        else
                                if (gapp->xsm->ActiveScan)
                                        if(info && layer>=0 && layer<gapp->xsm->GetActiveScan() -> mem2d->GetNv())
                                                gapp->xsm->GetActiveScan() -> mem2d->add_layer_information ((int)layer, new LayerInformation (info));

        idle_data->ret = 0;
        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_add_layer_information(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: add_layer_information to active scan channel");
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_add_layer_information_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

static PyObject* remote_da0(PyObject *self, PyObject *args)
{

        PI_DEBUG(DBG_L2, "pyremote: da0 ");
        double channel = 0;
        if (!PyArg_ParseTuple(args, "d", &channel))
                return Py_BuildValue("i", -1);
        if (channel){
                PI_DEBUG(DBG_L2, "Commented out.");
                //gapp->xsm->hardware->SetAnalog("da-name", channel);
        }
        return Py_BuildValue("i", 0);
}


static gboolean main_context_signal_emit_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        // NOT THREAD SAFE GUI OPERATION TRIGGER HERE
        gchar *action;
        idle_data->ret = -1;

        if (!PyArg_ParseTuple(idle_data->args, "s", &action)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }

        PI_DEBUG_GM (DBG_L2, "pyremote::remote_signal_emit (calling g_action_group_activate_action): %s", action);

        GActionMap *gm = G_ACTION_MAP (g_application_get_default ());
        //        g_message ("pyremote::remote_signal_emit get g_action_map: %s", gm ? "OK":"??");

        GAction *ga = g_action_map_lookup_action (gm, action);
        //        GAction *ga = g_action_map_lookup_action (G_ACTION_MAP (gapp->getApp ()), action);
        //      g_message ("pyremote::remote_signal_emit g_action_map_lookup_action: %s -> %s", action, ga?"OK":"??");

        if (ga){
                g_action_activate (ga, NULL);
                idle_data->ret = 0;
        } else {
                PI_DEBUG_GP_WARNING (DBG_L2, "==> action unknown: %s", action);
        }

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_signal_emit(PyObject *self, PyObject *args)
{
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_signal_emit_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}

#if 0
/* Taken from somewhere*/
static gboolean busy_sleep;
gint ret_false(gpointer r)
{
        // FIX-ME GTK4 ??? what is it at all
        //gtk_main_quit();
        *((int *)r) = FALSE;
        return FALSE;
}

void sleep_ms(int ms)
{
        if (busy_sleep) return;          /* Don't allow more than 1 sleep_ms */
        busy_sleep=TRUE;
        int wait=TRUE;
        g_timeout_add(ms,(GSourceFunc)ret_false, &wait); /* Start time-out function*/
        while (wait)
                while(g_main_context_pending (NULL)) g_main_context_iteration (NULL, FALSE);

        //gtk_main();                             /* wait */
        busy_sleep=FALSE;
}
#endif

static PyObject* remote_sleep(PyObject *self, PyObject *args)
{
        PI_DEBUG(DBG_L2, "pyremote: Sleep ");
        double d;
        if (!PyArg_ParseTuple(args, "d", &d))
                return Py_BuildValue("i", -1);
        if (d>0.){ // d in 1/10s
                g_usleep ((useconds_t)round(d*1e5)); // now in a thread and can simply sleep here!
                // sleep_ms((int)(round(d*100)));
        }
        return Py_BuildValue("i", 0);
}


static gboolean main_context_set_sc_label_from_thread (gpointer user_data){
        IDLE_from_thread_data *idle_data = (IDLE_from_thread_data *) user_data;
        gchar *id, *label;

        if (!PyArg_ParseTuple(idle_data->args, "ss", &id, &label)){
                UNSET_WAIT_JOIN_MAIN;
                return G_SOURCE_REMOVE;
        }
        if (py_gxsm_remote_console)
                py_gxsm_remote_console->set_sc_label (id, label);

        UNSET_WAIT_JOIN_MAIN;
        return G_SOURCE_REMOVE;
}

static PyObject* remote_set_sc_label(PyObject *self, PyObject *args)
{
        IDLE_from_thread_data idle_data;
        idle_data.self = self;
        idle_data.args = args;
        idle_data.wait_join = true;
        g_idle_add (main_context_set_sc_label_from_thread, (gpointer)&idle_data);
        WAIT_JOIN_MAIN;
        return Py_BuildValue("i", idle_data.ret);
}



///////////////////////////////////////////////////////////
/*
  PyMethodDef
  Structure used to describe a method of an extension type. This structure has four fields:

  Field   C Type  Meaning
  ml_name         char *  name of the method
  ml_meth         PyCFunction     pointer to the C implementation
  ml_flags        int     flag bits indicating how the call should be constructed
  ml_doc  char *  points to the contents of the docstring
*/

static PyMethodDef GxsmPyMethods[] = {
        // BLOCK I
        {"help", remote_help, METH_VARARGS, "List Gxsm methods: print gxsm.help ()"},
        {"set", remote_set, METH_VARARGS, "Set Gxsm entry value, see list_refnames: gxsm.set ('refname','value as string')"},
        {"get", remote_get, METH_VARARGS, "Get Gxsm entry as value, see list_refnames. gxsm.get ('refname')"},
        {"gets", remote_gets, METH_VARARGS, "Get Gxsm entry as string. gxsm.gets ('refname')"},
        {"list_refnames", remote_listr, METH_VARARGS, "List all available Gxsm entry refnames (Better: pointer hover over Gxsm-Entry to see tooltip with ref-name). print gxsm.list_refnames ()"},
        {"action", remote_action, METH_VARARGS, "Trigger Gxsm action (menu action or button signal), see list_actions: gxsm.action('action')"},
        {"list_actions", remote_lista, METH_VARARGS, "List all available Gxsm actions (Better: pointer hover over Gxsm-Button to see tooltip with action-name): print gxsm.list_actions ()"},
        {"rtquery", remote_rtquery, METH_VARARGS, "Gxsm hardware Real-Time-Query: svec[3] = gxsm.rtquery('X|Y|Z|xy|zxy|o|f|s|i|U') "},
        {"y_current", remote_y_current, METH_VARARGS, "RTQuery Current Scanline."},
        {"moveto_scan_xy", remote_moveto_scan_xy, METH_VARARGS, "Set tip position to Scan-XY: gxsm.moveto_scan_xy (x,y)"},

        // BLOCK II
        {"createscan", remote_createscan, METH_VARARGS, "Create Scan int: gxsm.createscan (ch,nx,ny,nv pixels, rx,ry in A, array.array('l', [...]), append)"},
        {"createscanf", remote_createscanf, METH_VARARGS, "Create Scan float: gxsm.createscan (ch,nx,ny,nv pixels, rx,ry in A, array.array('f', [...]), append)"},
        {"set_scan_unit", remote_set_scan_unit, METH_VARARGS, "Set Scan X,Y,Z,L Dim Unit: gxsm.set_scan_unit (ch,'X|Y|Z|L|T','UnitId string','Label string')"},
        {"set_scan_lookup", remote_set_scan_lookup, METH_VARARGS, "Set Scan Lookup for Dim: gxsm.set_scan_lookup (ch,'X|Y|L',start,end)"},
        //{"set_scan_lookup_i", remote_set_scan_llookup, METH_VARARGS, "Set Scan Lookup for Dim: gxsm.set_scan_lookup_i (ch,'X|Y|L',start,end)"},

        {"get_geometry", remote_getgeometry, METH_VARARGS, "Get Scan Geometry: [rx,ry,x0,y0,alpha]=gxsm.get_geometry (ch)"},
        {"get_differentials", remote_getdifferentials, METH_VARARGS, "Get Scan Scaling: [dx,dy,dz,dl]=gxsm.get_differentials (ch)"},
        {"get_dimensions", remote_getdimensions, METH_VARARGS, "Get Scan Dimensions: [nx,ny,nv,nt]=gxsm.get_dimensions (ch)"},
        {"get_data_pkt", remote_getdatapkt, METH_VARARGS, "Get Data Value at point: value=gxsm.get_data_pkt (ch, x, y, v, t)"},
        {"put_data_pkt", remote_putdatapkt, METH_VARARGS, "Put Data Value to point: gxsm.put_data_pkt (value, ch, x, y, v, t)"},
        {"get_slice", remote_getslice, METH_VARARGS, "Get Image Data Slice (Lines) from Scan in channel ch, yi ... yi+yn: [nx,ny,array]=gxsm.get_slice (ch, v, t, yi, yn)"},
        {"set_global_share_parameter", remote_set_global_share_parameter, METH_VARARGS, "Set Global Shared Parameter for Plugins, etc. in settings: gxsm.set_global_share_parameter (key, x)"},
        {"get_x_lookup", remote_get_x_lookup, METH_VARARGS, "Get Scan Data index to world mapping: x=gxsm.get_x_lookup (ch, i)"},
        {"get_y_lookup", remote_get_y_lookup, METH_VARARGS, "Get Scan Data index to world mapping: y=gxsm.get_y_lookup (ch, i)"},
        {"get_v_lookup", remote_get_v_lookup, METH_VARARGS, "Get Scan Data index to world mapping: v=gxsm.get_v_lookup (ch, i)"},
        {"set_x_lookup", remote_set_x_lookup, METH_VARARGS, "Set Scan Data index to world mapping: x=gxsm.get_x_lookup (ch, i, v)"},
        {"set_y_lookup", remote_set_y_lookup, METH_VARARGS, "Set Scan Data index to world mapping: y=gxsm.get_y_lookup (ch, i, v)"},
        {"set_v_lookup", remote_set_v_lookup, METH_VARARGS, "Set Scan Data index to world mapping: v=gxsm.get_v_lookup (ch, i, v)"},
        {"get_object", remote_getobject, METH_VARARGS, "Get Object Coordinates: [type, x,y,..]=gxsm.get_object (ch, n)"},
        {"add_marker_object", remote_addmobject, METH_VARARGS, "Put Marker/Rectangle Object at pixel coordinates or current tip pos (id='xy'|grp=-1, 'Rectangle[id]|grp=-2, 'Point[id]'): gxsm.add_marker_object (ch, label=str|'xy'|'Rectangle-id', mgrp=0..5|-1, x=ix,y=iy, size)"},
        {"marker_getobject_action", remote_getobject_action, METH_VARARGS, "Marker/Rectangle Object Action: gxsm.marker_getobject_action (ch, objnameid, action='REMOVE'|'REMOVE-ALL'|'GET_COORDS'|'SET-OFFSET')"},

        {"startscan", remote_startscan, METH_VARARGS, "Start Scan."},
        {"stopscan", remote_stopscan, METH_VARARGS, "Stop Scan."},
        {"waitscan", remote_waitscan, METH_VARARGS, "Wait Scan. ret=gxsm.waitscan(blocking=true). ret=-1: no scan in progress, else current line index."},
        {"scaninit", remote_scaninit, METH_VARARGS, "Scaninit."},
        {"scanupdate", remote_scanupdate, METH_VARARGS, "Scanupdate."},
        {"scanylookup", remote_scanylookup, METH_VARARGS, "Scanylookup."},
        {"scanline", remote_scanline, METH_VARARGS, "Scan line."},

        // BLOCK III
        {"autosave", remote_autosave, METH_VARARGS, "Save: Auto Save Scans. gxsm.autosave (). Returns current scanline y index and file name(s) if scanning."},
        {"autoupdate", remote_autoupdate, METH_VARARGS, "Save: Auto Update Scans. gxsm.autoupdate (). Returns current scanline y index and file name(s) if scanning."},
        {"save", remote_save, METH_VARARGS, "Save: Auto Save Scans: gxsm.save ()"},
        {"saveas", remote_saveas, METH_VARARGS, "Save File As: gxsm.saveas (ch, 'path/fname.nc')"},
        {"load", remote_load, METH_VARARGS, "Load File: gxsm.load (ch, 'path/fname.nc')"},
        {"export", remote_export, METH_VARARGS, "Export scan: gxsm.export (ch, 'path/fname.nc')"},
        {"import", remote_import, METH_VARARGS, "Import scan: gxsm.import (ch, 'path/fname.nc')"},
        {"save_drawing", remote_save_drawing, METH_VARARGS, "Save Drawing to file: gxsm.save_drawing (ch, time, layer, 'path/fname.png|pdf|svg')"},

        // BLOCK IV
        {"set_view_indices", remote_set_view_indices, METH_VARARGS, "Set Ch view time and layer indices: gxsm.set_view_indices (ch, time, layer)"},
        {"autodisplay", remote_autodisplay, METH_VARARGS, "Autodisplay active channel: gxsm.autodisplay ()"},
        {"chfname", remote_chfname, METH_VARARGS, "Get Ch Filename: filename = gxsm.chfname (ch)"},
        {"chmodea", remote_chmodea, METH_VARARGS, "Set Ch Mode to A: gxsm.chmodea (ch)"},
        {"chmodex", remote_chmodex, METH_VARARGS, "Set Ch Mode to X: gxsm.chmodex (ch)"},
        {"chmodem", remote_chmodem, METH_VARARGS, "Set Ch Mode to MATH: gxsm.chmodem (ch)"},
        {"chmoden", remote_chmoden, METH_VARARGS, "Set Ch Mode to Data Channel <X+N>: gxsm.chmoden (ch,n)"},
        {"chmodeno", remote_chmodeno, METH_VARARGS, "Set View Mode to No: gxsm.chmodeno (ch)"},
        {"chview1d", remote_chview1d, METH_VARARGS, "Set View Mode to 1D: gxsm.chmode1d (ch)"},
        {"chview2d", remote_chview2d, METH_VARARGS, "Set View Mode to 2D: gxsm.chmode2d (ch)"},
        {"chview3d", remote_chview3d, METH_VARARGS, "Set View Mode to 3D: gxsm.chmode3d (ch)"},
        {"quick", remote_quick, METH_VARARGS, "Quick."},
        {"direct", remote_direct, METH_VARARGS, "Direct."},
        {"log", remote_log, METH_VARARGS, "Log."},
        {"crop", remote_crop, METH_VARARGS, "Crop (ch-src, ch-dst)"},

        // BLOCK V
        {"unitbz", remote_unitbz, METH_VARARGS, "UnitBZ."},
        {"unitvolt", remote_unitvolt, METH_VARARGS, "UnitVolt."},
        {"unitev", remote_unitev, METH_VARARGS, "UniteV."},
        {"units", remote_units, METH_VARARGS, "UnitS."},

        // BLOCK VI
        {"echo", remote_echo, METH_VARARGS, "Echo string to terminal. gxsm.echo('hello gxsm to terminal') "},
        {"logev", remote_logev, METH_VARARGS, "Write string to Gxsm system log file and log monitor: gxsm.logev ('hello gxsm to logfile/monitor') "},
        {"progress", remote_progress_info, METH_VARARGS, "Show/update gxsm progress info. fraction<0 init, 0..1 progress, >1 close: gxsm.progress ('Calculating...', fraction) "},
        {"add_layerinformation", remote_add_layer_information, METH_VARARGS, "Add Layerinformation to active scan. gxsm.add_layerinformation('Text', ch, nth)"},
        {"da0", remote_da0, METH_VARARGS, "Da0. -- N/A for SRanger"},
        {"signal_emit", remote_signal_emit, METH_VARARGS, "Action-String. "},
        {"sleep", remote_sleep, METH_VARARGS, "Sleep N/10s: gxsm.sleep (N) "},
        {"set_sc_label", remote_set_sc_label, METH_VARARGS, "Set PyRemote SC label: gxsm.set_sc_label (id [1..8],'value as string')"},

        {NULL, NULL, 0, NULL}
};

static PyObject* remote_help(PyObject *self, PyObject *args)
{
        gint entries;
        for (entries=0; GxsmPyMethods[entries].ml_name != NULL; entries++);
        PyObject *ret = PyTuple_New(entries);
        for (int n=0; n < entries; n++){
                gchar *tmp = g_strdup_printf ("gxsm.%s : %s", GxsmPyMethods[n].ml_name, GxsmPyMethods[n].ml_doc);
                PyTuple_SetItem(ret, n, PyUnicode_FromString (tmp)); // Add Refname to Return-list
                g_free (tmp);
        }

        return ret;
}



int ok_button_callback( GtkWidget *widget, gpointer data)
{
        //    cout << getpid() << endl;
        kill (getpid(), SIGINT);
        //    cout << "pressed" <<endl;
        return 0;
}

py_gxsm_console::~py_gxsm_console (){
        PI_DEBUG_GM(DBG_L2, "Pyremote Plugin: destructor. Calls: Py_FinalizeEx()");
        closing = true;

        // TODO: Wait up to N seconds before exiting impolitely
        while( closing ) {  // Waiting for Python thread to close...
                 g_usleep(DEFAULT_SLEEP_USECS);
        }
        PI_DEBUG_GM(DBG_L2, "Pyremote Plugin: destructor completed.");
}


/* Python strout/err redirection helper module */

static PyObject* redirection_stdoutredirect(PyObject *self, PyObject *args)
{
        const char *string;
        if(!PyArg_ParseTuple(args, "s", &string))
                return NULL;

        g_print ("%s", string);
        if (py_gxsm_remote_console)
                py_gxsm_remote_console->push_message_async (string);

        Py_INCREF(Py_None);
        return Py_None;
}

static PyMethodDef RedirectionMethods[] = {
        {"stdoutredirect", redirection_stdoutredirect, METH_VARARGS,
         "stdout redirection helper"},
        {NULL, NULL, 0, NULL}
};

static struct PyModuleDef redirection_module_def = {
        PyModuleDef_HEAD_INIT,
        "redirection",     /* m_name */
        "GXSM Python Console STDI Redirection Module",  /* m_doc */
        -1,                  /* m_size */
        RedirectionMethods,  /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
};


static struct PyModuleDef gxsm_module_def = {
        PyModuleDef_HEAD_INIT,
        "gxsm",     /* m_name */
        "GXSM Python Remote Module",  /* m_doc */
        -1,                  /* m_size */
        GxsmPyMethods,       /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
};

static PyObject* PyInit_Gxsm(void)
{
        PI_DEBUG_GM (DBG_L1, "** PyInit_Gxsm => PyModuleCreate gxsm\n");
        // g_print ("** PyInit_Gxsm => PyModuleCreate gxsm\n");
        return PyModule_Create (&gxsm_module_def);
}

static PyObject* PyInit_Redirection(void)
{
        PI_DEBUG_GM (DBG_L1, "** PyInit_Redirection => PyModuleCreate redirection\n");
        // g_print ("** PyInit_Redirection => PyModuleCreate redirection\n");
        return PyModule_Create (&redirection_module_def);
}


void py_gxsm_console::initialize(void)
{
        PI_DEBUG_GM (DBG_L1, "pyremote Plugin :: py_gxsm_console::initialize **");

        if (!Py_IsInitialized()) {
                PI_DEBUG_GM (DBG_L1, "** Initializing Python interpreter, loading gxsm module and stdout redirection helper **");
                PI_DEBUG_GM (DBG_L1, "pyremote Plugin :: initialize -- PyImport_Append");

                push_message_async (N_("\n\nChecking for Gxsm Python Venv. -- You can setup a Gxsm Python dedicated virtual environmnet if desired:\n to create run $python3 -m venv ~/.gxsm3/pyaction/GxsmPythonVenv"));
                // create using: $ python3 -m venv .gxsm3/pyaction/GxsmPythonVenv
                
                // Testing Venv use
                gchar* venv_path = g_strconcat (g_get_home_dir (), "/.gxsm3/pyaction/GxsmPythonVenv", NULL);
                gchar* venv_cfg  = g_strconcat (venv_path, "/pyvenv.cfg", NULL);
                gchar* venv_python  = g_strconcat (venv_path, "/bin/python", NULL);
                // create using: $ python3 -m venv .gxsm3/pyaction/GxsmPythonVenv
                if (g_file_test (venv_cfg, G_FILE_TEST_EXISTS))
                        if (g_file_test (venv_python, G_FILE_TEST_EXISTS)){
                                //wchar_t *wc = Py_DecodeLocale(const char *arg, size_t *size)
                                push_message_async (N_("\n\nINFO: Found Gxsm Python Venv. Using:"  ));
                                push_message_async (venv_python);
                                PyStatus status;
                                PyPreConfig preconfig;
                                PyPreConfig_InitPythonConfig(&preconfig);
                                preconfig.utf8_mode = 1;
                                status = Py_PreInitialize(&preconfig);
                                Py_SetProgramName(Py_DecodeLocale(venv_python, NULL));

                                //status = Py_InitializeFromConfig(&config);

                        }

#if 0
                // This API is kept for backward compatibility: setting PyConfig.program_name should be used instead, see Python Initialization Configuration.
                
                PyStatus status;

                PyConfig config;
                PyConfig_InitPythonConfig(&config);
                config.isolated = 1;

                /* Decode command line arguments.
                   Implicitly preinitialize Python (in isolated mode). */
                status = PyConfig_SetBytesArgv(&config, argc, argv);
                if (PyStatus_Exception(status)) {
                        goto exception;
                }

                status = Py_InitializeFromConfig(&config);
                if (PyStatus_Exception(status)) {
                        goto exception;
                }
                PyConfig_Clear(&config);

                return Py_RunMain();

        exception:
                PyConfig_Clear(&config);
                if (PyStatus_IsExit(status)) {
                        return status.exitcode;
                }
                /* Display the error message and exit the process with
                   non-zero exit code */
                Py_ExitStatusException(status);
#endif

                
                // g_print ("pyremote Plugin :: initialize -- PyImport_Append\n");
                PyImport_AppendInittab ("gxsm", &PyInit_Gxsm);
                PyImport_AppendInittab ("redirection", &PyInit_Redirection);

                PI_DEBUG_GM (DBG_L2, "pyremote Plugin :: initialize --  PyInitializeEx(0)");
                // g_print ("pyremote Plugin :: initialize -- PyInitializeEx(0)\n");
                // Do not register signal handlers -- i.e. do not "crash" gxsm on errors!
                push_message_async ("\nInitializing Python ...\n");
                Py_InitializeEx (0);

                
                if (_import_array() < 0) {
                        PI_DEBUG_GM (DBG_L1, "pyremote Plugin :: initialize -- ImportModule gxsm, import array failed.");
                        PyErr_Print(); PyErr_SetString(PyExc_ImportError, "numpy.core.multiarray failed to import");
                }

                PI_DEBUG_GM (DBG_L2, "pyremote Plugin :: initialize -- ImportModule gxsm");
                // g_print ("pyremote Plugin :: initialize -- ImportModule gxsm\n");
                PyImport_ImportModule("gxsm");
                PyImport_ImportModule("redirection");

                PI_DEBUG_GM (DBG_L2, "pyremote Plugin :: initialize -- AddModule main\n");
                // g_print ("pyremote Plugin :: initialize -- AddModule main\n");
                py_gxsm_module.main_module = PyImport_AddModule("__main__");

                PI_DEBUG_GM (DBG_L2, "Get dict");
                push_message_async ("\nPython ready.\n");
        } else {
                g_message ("Python interpreter already initialized.");
                push_message_async ("\nPython is initialzie.\n");
        }
}

PyObject* py_gxsm_console::run_string(const char *cmd, int type, PyObject *g,
                                      PyObject *l, py_gxsm_console *console) {
        // This method assumes any Python C-API handling has been done before being
        // called (i.e. you have the GIL and are in a thread that Python is aware of).
        // Note: the console can be passed to push messages.
        if (console) {
                console->push_message_async ("\n<<< Starting Python. <<<\n");
                if (0){ // add verbose mode??
                        console->push_message_async ("\n<<< Running string in python. <<<\n");
                        console->push_message_async (cmd);
                }
        }

        PyErr_Clear(); // Clear any errors before running?
        PyObject *ret = PyRun_String(cmd, type, g, l);

        if (console) {
                console->push_message_async (ret ?
                                    "\n<<< Python interpreter finished processing string. <<<\n" :
                                    "\n<<< Python interpreter completed with Exception/Error. <<<\n");
                console->push_message_async (NULL); // terminate IDLE push task
        }

        if (!ret) {
                g_message ("Python interpreter completed with Exception/Error.");
                PyErr_Print();
        }
        return ret;
}


void py_gxsm_console::show_stderr(const gchar *str)
{
        GtkDialogFlags flags =  (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);
        GtkWidget *dialog = gtk_dialog_new_with_buttons (N_("Python interpreter result"),
                                                         window,
                                                         flags,
                                                         _("_CLOSE"), GTK_RESPONSE_CLOSE,
                                                         NULL);

        GtkWidget *text = gtk_text_view_new ();
        GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);
        gtk_container_add (GTK_CONTAINER (scroll), text);
        gtk_text_view_set_editable (GTK_TEXT_VIEW(text), FALSE);
        gtk_text_buffer_set_text (gtk_text_view_get_buffer(GTK_TEXT_VIEW(text)),
                                  str,
                                  -1);
        gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), scroll);

        gtk_widget_show (dialog);
        /*gint result =*/ gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);
}

void py_gxsm_console::initialize_stderr_redirect(PyObject *d)
{
        // new redirection of stdout/err capture
        run_string ("import redirection\n"
                    "import sys\n"
                    "class StdoutCatcher:\n"
                    "    def write(self, stuff):\n"
                    "        redirection.stdoutredirect(stuff)\n"
                    "    def flush(self):\n"
                    "        redirection.stdoutredirect('\\n')\n"
                    "class StderrCatcher:\n"
                    "    def write(self, stuff):\n"
                    "        redirection.stdoutredirect(stuff)\n"
                    "    def flush(self):\n"
                    "        redirection.stdoutredirect('\\n')\n"
                    "sys.stdout = StdoutCatcher()\n"
                    "sys.stderr = StderrCatcher()\n",
                    Py_file_input, d, d, this);
}

PyObject *py_gxsm_console::create_environment(const gchar *filename) {
        PyObject *d, *plugin_filename;
        wchar_t *argv[1];
        argv[0] = NULL;

        PI_DEBUG_GM (DBG_L2, "py_gxsm_console::create_environment **");

        d = PyDict_Copy (PyModule_GetDict (py_gxsm_module.main_module));
        // set __file__ variable for clearer error reporting
        plugin_filename = Py_BuildValue("s", filename);
        PyDict_SetItemString(d, "__file__", plugin_filename);
        PySys_SetArgv(0, argv);

        PI_DEBUG_GM(DBG_L2, "set up stderr redirection");
        initialize_stderr_redirect(d);

        if (d) {
                PI_DEBUG_GM(DBG_L2, "import gxsm module.");
                run_string("import gxsm\n", Py_file_input, d, d, this);
        }

        return d;
}

void py_gxsm_console::destroy_environment(PyObject *d, bool clearGlobals) {
        PI_DEBUG_GM (DBG_L2, "py_gxsm_console::destroy_environment **");

        if (clearGlobals) {
                PyDict_Clear(d);
        }
        Py_DECREF(d);
}

void py_gxsm_console::clear_output(GtkButton *btn, gpointer user_data)
{
        py_gxsm_console *pygc = (py_gxsm_console *)user_data;
        GtkTextBuffer *console_buf;
        GtkTextIter start_iter, end_iter;
        GtkTextView *textview;

        textview = GTK_TEXT_VIEW (pygc->console_output);
        console_buf = gtk_text_view_get_buffer(textview);
        gtk_text_buffer_get_bounds(console_buf, &start_iter, &end_iter);
        gtk_text_buffer_delete(console_buf, &start_iter, &end_iter);
}

int Stop(void *){
        PyErr_SetString (PyExc_KeyboardInterrupt, "Abort");
        return -1;  // Return <0 indicates exception/issue.
}

/*
 * killing the interpreter
 * see http://stackoverflow.com/questions/1420957/stopping-embedded-python
 * for possibly adding an exit flag.
 */
void py_gxsm_console::kill(GtkToggleButton *btn, gpointer user_data)
{
        py_gxsm_console *pygc = (py_gxsm_console *)user_data;

        if (pygc->user_script_data.cmd){ // User script running (or should be)
                pygc->append (N_("\n*** SCRIPT INTERRUPT REQUESTED: Setting PyErr SIGINT to abort script.\n"));
                PI_DEBUG_GM (DBG_L2,  "trying to interrup interpreter, sending SIGINT.");
                int r = Py_AddPendingCall(&Stop, NULL); // inject Stop routine
                if (r == -1) {
                        pygc->append("\nInterrupt request failed!\n");
                }

                // Try to force Stop() being called by calling CheckSignals().
                PyGILState_STATE gstate = PyGILState_Ensure ();
                PyErr_CheckSignals ();
                PyGILState_Release (gstate);

        } else {
                pygc->append (N_("\n*** SCRIPT INTERRUPT: No user script is currently running.\n"));
        }
}

// This is the 'main' Python interpreter thread, which runs user selected
// scripts and independent python commands fed from the console.
// Requested commands are read from py_gxsm_remote_console::user_script_data.
// If py_gxsm_remote_console::reset_user_script_data is true, we flush local
// data in Python.
gpointer py_gxsm_console::PyRunConsoleThread(gpointer user_data)
{
        // Note: user_data is NULL, since we can get what we need from our trusty
        // global py_gxsm_remote_console.

        // We save and restore the thread between Python calls, so any other
        // Python thread can use it.

        py_gxsm_console *pygc = py_gxsm_remote_console; //(py_gxsm_console *)user_data;
        PyRunThreadData *s = (PyRunThreadData*) &pygc->user_script_data;
        PI_DEBUG_GM (DBG_L2, "pyremote Plugin :: py_gxsm_console::PyConsoleThreadFunc");

        pygc->append("Welcome to the PyRemote Control for GXSM: Python Version is ");
        pygc->append(Py_GetVersion());

        // Start up the console!
        pygc->initialize();

        // create new environment
        PyObject *d = NULL;
        PyThreadState *thread_state = PyEval_SaveThread();
        while( pygc && !pygc->closing)  // Close when console is deleted...
        {
                // If run_data was set, run in python
                if( s->cmd )
                {
                        PyEval_RestoreThread (thread_state);
                        // Ensure we have our 'environment' as desired
                        if (d == NULL || pygc->reset_user_script_data) {
                                pygc->reset_user_script_data = false;
                                if (d != NULL)
                                        pygc->destroy_environment(d);
                                d = pygc->create_environment("__console__");
                                if (!d)
                                        break;  // Exit on environment failure
                        }

                        s->ret = pygc->run_string(s->cmd, s->mode, d, d,
                                                  pygc);
                        thread_state = PyEval_SaveThread ();

                        g_free (s->cmd);
                        s->cmd = NULL;
                        PI_DEBUG_GM (DBG_L2, "pyremote Plugin :: py_gxsm_console::PyConsoleThreadFunc PyRun completed");
                }
                g_usleep (DEFAULT_SLEEP_USECS);  // Hard-coded sleep time...
        }

        PI_DEBUG_GM(DBG_L2, "Pyremote Plugin: Python thread closing.");

        // Destroy environment, close python interpreter.
        PyEval_RestoreThread (thread_state);
        if (d != NULL)
                pygc->destroy_environment(d, true);
        Py_FinalizeEx();

        PI_DEBUG_GM(DBG_L2, "Pyremote Plugin: Python thread ended..");

        pygc->closing = false;
        return NULL;
}

// This handles any action scripts that are requested. It is spawned
// as its own thread. Commands are read from the provided PyRunThreadData.
gpointer py_gxsm_console::PyRunActionScriptThread(gpointer user_data)
{
        PI_DEBUG_GM(DBG_L2, "Pyremote Plugin: Python action script thread starting.");
        py_gxsm_console *pygc = py_gxsm_remote_console;
        PyRunThreadData *s = (PyRunThreadData *)user_data;

        // Register this thread with the Python interpreter, and get the GIL.
        PyGILState_STATE gstate = PyGILState_Ensure ();

        // create new environment
        PyObject *d = NULL;
        gchar* script_name = g_strdup_printf ("action_script_%d",
                                              pygc->action_script_running);
        d = pygc->create_environment (script_name);
        if (d != NULL) {
                pygc->action_script_running++;
                // TODO: Do we want extra info to show up in console?
                // Note that all messages are passed to console currently
                // (via redirection_stdouredirect).
                py_gxsm_console::run_string (s->cmd, s->mode, d, d,
                                             pygc);
                --pygc->action_script_running;

                PI_DEBUG_GM(DBG_L2, "Pyremote Plugin: Python action script thread ending.");
        }

        // Clean-up
        if (d != NULL) {
                pygc->destroy_environment (d, pygc->restart_interpreter);
                pygc->restart_interpreter = false;
        }
        delete s;

        // Release the GIL and ... unregister this thread?
        PyGILState_Release (gstate);
        return NULL;
}


const gchar* py_gxsm_console::run_command (const gchar *cmd, int mode,
        bool reset_locals, bool reset_globals, bool run_as_action_script)
{
        if (!cmd) {
                g_warning("No command.");
                return NULL;
        }

        PyRunThreadData *run_data = &user_script_data;
        if (run_as_action_script)
                // New a struct; RunActionScriptThread will delete
                run_data = new PyRunThreadData ();

        if (!run_data->cmd){
                PI_DEBUG_GM (DBG_L2, "pyremote Plugin :: py_gxsm_console::run_command *** starting console IDLE message pop job.");
                run_data->cmd = g_strdup (cmd);
                run_data->mode = mode;
                run_data->ret  = NULL;

                if (reset_locals)
                        reset_user_script_data = true;
                if (reset_globals)
                        restart_interpreter = true;

                if (run_as_action_script)  // Spawn new thread
                        g_thread_new (NULL, PyRunActionScriptThread, run_data);
                return NULL;
        } else {
                return "Busy";
        }
}

void py_gxsm_console::append (const gchar *msg)
{
        if (!msg) return;
        GtkTextBuffer *text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (console_output));
        GtkTextIter text_iter_end;
        gtk_text_buffer_get_end_iter (text_buffer, &text_iter_end);
        gtk_text_buffer_insert (text_buffer, &text_iter_end, msg, -1);
        gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (console_output),
                                      console_mark_end,
                                      0., FALSE, 0., 0.);
}

// simple parser to include script library.
// As this seams not possible with embedded python via "use ..." as gxsm.Fucntions() are not availble in external libraries.
// So this simple approach.
gchar *py_gxsm_console::pre_parse_script (const gchar *script, int *n_lines, int r){
        gchar *tmp;
        gchar *parsed_script = g_strdup_printf ("### parsed script. Level: %d\n", r);
        gchar **lines = NULL;
        gchar *to_parse = g_strdup (script);
        int i=0;

        // Max recursion = 10
        if (r > 10){
                gchar *message = g_strdup_printf("Pasing Error: Potential dead-loop of recursive inclusion detected:\n"
                                                 "Include Level > 10.\n"
                        );
                g_warning ("%s", message);
                append(message);
                gapp->warning (message);
                g_free(message);

                g_free (parsed_script);
                return to_parse;
        }

        do {
                ++i;
                if (lines) g_strfreev (lines);
                lines = g_strsplit (to_parse, "\n", 2);
                g_free (to_parse);
                to_parse = NULL;
                //g_print ("%05d: %s\n", i, lines[0]);

                if (g_strrstr(lines[0], "#GXSM_USE_LIBRARY")){
                        gchar *a = g_strrstr(lines[0], "<");
                        gchar *b = g_strrstr(lines[0], ">");
                        if (a && b){
                                *b='\0';
                                gchar *name=a+1;
                                //g_print ("Including Library <%s>\n", name);
                                gchar *lib_script = get_gxsm_script (name);
                                if (lib_script){
                                        int n_included=0;
                                        gchar *rtmp = pre_parse_script (lib_script, &n_included, r+1); // recursively parse
                                        g_free (lib_script);
                                        n_included += 8; // incl. comments below
                                        gchar *stat = g_strdup_printf ("Lines inlucded from <%s> at line %d: %d, Include Level: %d\n",
                                                                       name, i, n_included, r+1);

                                        tmp = g_strconcat (parsed_script, "\n\n",
                                                           "### BEGIN GXSM LIBRARY SCRIPT <", name, ">\n\n",
                                                           rtmp ? rtmp : "## PARSING ERROR: LIB-SCRIPT NOT FOUND", "\n",
                                                           "### ", stat,
                                                           "### END GXSM LIBRARY SCRIPT <", name, ">\n\n",
                                                           NULL);
                                        append (stat);
                                        g_message ("%s", stat);

                                        i += n_included;

                                        g_free (stat);
                                        g_free (parsed_script);
                                        g_free (rtmp);
                                        *b='>';
                                        parsed_script = tmp;
                                } else {
                                        gchar *message = g_strdup_printf("Action script/library parser error at line %d:\n"
                                                                         "%s\n"
                                                                         "Include file <%s> not found.",
                                                                         i,
                                                                         lines[0],
                                                                         name
                                                );
                                        g_message ("%s", message);
                                        append(message);
                                        gapp->warning (message);
                                        g_free(message);
                                }
                        } else {
                                g_warning ("Pasing Error here: %s", lines[0]);
                                gchar *message = g_strdup_printf("Action script/library parser syntax error at line %d:\n"
                                                                 "%s\n"
                                                                 "Gxsm Library script include example statement:\n"
                                                                 "#GXSM_USE_LIBRARY <gxsm3-lib-utils>\n",
                                                                 i,
                                                                 lines[0]);
                                g_message ("%s", message);
                                append(message);
                                gapp->warning (message);
                                g_free(message);
                        }
                } else {
                        tmp = g_strconcat (parsed_script, "\n", lines[0], NULL);
                        g_free (parsed_script);
                        parsed_script = tmp;
                }

                if (lines[1])
                        to_parse = g_strdup (lines[1]);
        } while (lines[0] && lines[1]);

        g_strfreev (lines);
        if (to_parse)
                g_free (to_parse);

        //g_print ("PARSED-SCRIPT\n");
        //g_print (parsed_script);

        if (n_lines)
                *n_lines = i;

        return parsed_script;
}

void py_gxsm_console::run_file(GtkButton *btn, gpointer user_data)
{
        py_gxsm_console *pygc = (py_gxsm_console *)user_data;
        GtkTextView *textview;
        GtkTextBuffer *console_file_buf;
        GtkTextIter start_iter, end_iter;
        gchar *script, *parsed_script;

        textview = GTK_TEXT_VIEW(pygc->console_file_content);
        console_file_buf = gtk_text_view_get_buffer(textview);

        if (pygc->user_script_data.cmd) {  // User script is running (or should be)
                pygc->append (N_("\n*** STOP -- User script is currently running. No recursive execution allowed for this console.\n"));
        } else {
                gtk_text_buffer_get_bounds(console_file_buf, &start_iter, &end_iter);
                script = gtk_text_buffer_get_text(console_file_buf,
                                                  &start_iter, &end_iter, FALSE);
                parsed_script = pygc->pre_parse_script (script);
                g_free (script);
                script = parsed_script;

                pygc->push_message_async (N_("\n>>> Executing parsed script >>>\n"));
                pygc->run_command (script, Py_file_input, true, false, false);
                g_free(script);
        }
}

void py_gxsm_console::fix_eols_to_unix(gchar *text)
{
        gchar *p = strchr(text, '\r');
        guint i, j;

        /* Unix */
        if (!p)
                return;

        /* Mac */
        if (p[1] != '\n') {
                do {
                        *p = '\n';
                } while ((p = strchr(p+1, '\r')));

                return;
        }

        /* MS-DOS */
        for (i = 0, j = 0; text[i]; i++) {
                if (text[i] != '\r') {
                        text[j] = text[i];
                        j++;
                }
        }
        text[j] = '\0';
}


void py_gxsm_console::open_file_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
        py_gxsm_console *pygc = (py_gxsm_console *)user_data;
        GtkWidget *file_chooser;
        GtkFileFilter *filter = gtk_file_filter_new();
        GtkTextBuffer *console_file_buf;
        GtkTextView *textview;

        gtk_file_filter_add_mime_type(filter, "text/x-python");
        gtk_file_filter_add_pattern(filter, "*.py");

        file_chooser = gtk_file_chooser_dialog_new(N_("Open Python script"),
                                                   pygc->window,
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   N_("_Cancel"), GTK_RESPONSE_CANCEL,
                                                   N_("_Open"), GTK_RESPONSE_ACCEPT,
                                                   NULL);

        gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(file_chooser), filter);

        if (!pygc-> query_filename ||
            gtk_dialog_run(GTK_DIALOG(file_chooser)) == GTK_RESPONSE_ACCEPT) {
                gchar *file_content;
                GError *err = NULL;

                if (pygc->query_filename) {
                        pygc->set_script_filename (gtk_file_chooser_get_filename
                                                   (GTK_FILE_CHOOSER(file_chooser)));
                }
                else {
                        /* this is a bit of a kludge, so i want to ensure that using the set
                           filename is chosen explicitly each time*/
                        pygc->query_filename = true;
                }
                if (!g_file_get_contents(pygc->script_filename,
                                         &file_content,
                                         NULL,
                                         &err)) {
                        gchar *message = g_strdup_printf("Cannot read content of file "
                                                         "'%s': %s",
                                                         pygc->script_filename,
                                                         err->message);
                        g_clear_error(&err);
                        pygc->append(message);
                        pygc->fail = true;
                        g_free(message);
                        pygc->set_script_filename (NULL);
                }
                else {
                        pygc->fix_eols_to_unix(file_content);

                        // read string which contain last command output
                        textview = GTK_TEXT_VIEW(pygc->console_file_content);
                        console_file_buf = gtk_text_view_get_buffer(textview);
                        // append input line
                        gtk_text_buffer_set_text(console_file_buf, file_content, -1);
                        g_free(file_content);
                        pygc->fail = false;
                }
        }
        gtk_widget_destroy(GTK_WIDGET(file_chooser));
}

void py_gxsm_console::open_action_script_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
        py_gxsm_console *pygc = (py_gxsm_console *)user_data;
        //GtkWidget *file_chooser;
        GtkTextBuffer *console_file_buf;
        GtkTextView *textview;
        GVariant *old_state=NULL, *new_state=NULL;
        gchar *file_content;
        GError *err = NULL;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_string (g_variant_get_string (parameter, NULL));

        XSM_DEBUG_GP (DBG_L1, "py_gxsm_console open_file action %s activated, state changes from %s to %s\n",
                      g_action_get_name (G_ACTION (action)),
                      g_variant_get_string (old_state, NULL),
                      g_variant_get_string (new_state, NULL));

        pygc->set_script_filename (g_variant_get_string (new_state, NULL));
        if (!g_file_get_contents(pygc->script_filename,
                                 &file_content,
                                 NULL,
                                 &err)) {
                gchar *message = g_strdup_printf("Cannot read content of file "
                                                 "'%s': %s",
                                                 pygc->script_filename,
                                                 err->message);
                g_clear_error(&err);
                pygc->append(message);
                pygc->fail = true;
                g_free(message);
                pygc->set_script_filename (NULL);
        }
        else {
                pygc->fix_eols_to_unix(file_content);

                // read string which contain last command output
                textview = GTK_TEXT_VIEW(pygc->console_file_content);
                console_file_buf = gtk_text_view_get_buffer(textview);
                // append input line
                gtk_text_buffer_set_text(console_file_buf, file_content, -1);
                g_free(file_content);
                pygc->fail = false;
        }
        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);
}

void py_gxsm_console::save_file_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
        py_gxsm_console *pygc = (py_gxsm_console *)user_data;
        GtkTextView *textview;
        GtkTextBuffer *buf;
        GtkTextIter start_iter, end_iter;
        gchar *script;
        FILE *f;

        if (pygc->script_filename == NULL) {
                pygc->save_file_as_callback (NULL, NULL, user_data);
        }
        else {
                textview = GTK_TEXT_VIEW (pygc->console_file_content);
                buf = gtk_text_view_get_buffer(textview);
                gtk_text_buffer_get_bounds(buf, &start_iter, &end_iter);
                script = gtk_text_buffer_get_text(buf, &start_iter, &end_iter, FALSE);
                f = g_fopen (pygc->script_filename, "wb");
                if (f) {
                        fwrite(script, 1, strlen(script), f);
                        fclose(f);
                        pygc->set_script_filename ();
                }
                else {
                        gchar *message = g_strdup_printf("Cannot open file '%s': %s",
                                                         pygc->script_filename,
                                                         g_strerror(errno));
                        pygc->append(message);
                        g_free(message);
                        pygc->set_script_filename ("invalid file");
                }
                g_free(script);
        }
}

void py_gxsm_console::save_file_as_callback (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
        py_gxsm_console *pygc = (py_gxsm_console *)user_data;
        GtkWidget *dialog;

        dialog = gtk_file_chooser_dialog_new(N_("Save Script as"),
                                             NULL,
                                             GTK_FILE_CHOOSER_ACTION_SAVE,
                                             N_("_Cancel"), GTK_RESPONSE_CANCEL,
                                             N_("_Save"), GTK_RESPONSE_ACCEPT,
                                             NULL);
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

        //gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), default_folder_for_saving);
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
                                          "Untitled document");

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
                pygc->script_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
                pygc->save_file_callback (NULL, NULL, user_data);
        }
        gtk_widget_destroy(dialog);
}


void py_gxsm_console::configure_callback (GSimpleAction *action, GVariant *parameter,
                                          gpointer user_data){
        //py_gxsm_console *pygc = (py_gxsm_console *) user_data;
        GVariant *old_state, *new_state;

        old_state = g_action_get_state (G_ACTION (action));
        new_state = g_variant_new_boolean (!g_variant_get_boolean (old_state));

        g_print ("Toggle action %s activated, state changes from %d to %d\n",
                 g_action_get_name (G_ACTION (action)),
                 g_variant_get_boolean (old_state),
                 g_variant_get_boolean (new_state));

        g_simple_action_set_state (action, new_state);
        g_variant_unref (old_state);

        if (g_variant_get_boolean (new_state)){
                ;
        }
}


void py_gxsm_console::restart_interpreter_callback(GSimpleAction *action, GVariant *parameter,
                                          gpointer user_data){
        py_gxsm_console *pygc = (py_gxsm_console *) user_data;

        const gchar *cmd = "";
        int mode = Py_file_input;
        pygc->run_command (cmd, mode, false, true, true);
}

static GActionEntry win_py_gxsm_action_entries[] = {
        { "pyfile-open", py_gxsm_console::open_file_callback, NULL, NULL, NULL },
        { "pyfile-save", py_gxsm_console::save_file_callback, NULL, NULL, NULL },
        { "pyfile-save-as", py_gxsm_console::save_file_as_callback, NULL, NULL, NULL },

        { "pyfile-action-use", py_gxsm_console::open_action_script_callback, "s", "'sf1'", NULL },

        { "pyremote-configure", py_gxsm_console::configure_callback, NULL, NULL, NULL },
        { "pyremote-restart-interpreter", py_gxsm_console::restart_interpreter_callback, NULL,
          NULL, NULL },
};

void py_gxsm_console::AppWindowInit(const gchar *title){

        PI_DEBUG(DBG_L2, "pyremote Plugin :: AppWindoInit() -- building Console AppWindow.");

        //        SET_PCS_GROUP("plugin_libpyremote");
        //        gsettings = g_settings_new (GXSM_RES_BASE_PATH_DOT".plugin.common.libpyremote");

        app_window = gxsm3_app_window_new (GXSM3_APP (gapp->get_application ()));
        window = GTK_WINDOW (app_window);

        header_bar = gtk_header_bar_new ();
        gtk_widget_show (header_bar);
        // hide close, min, max window decorations
        gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), false);

        g_action_map_add_action_entries (G_ACTION_MAP (app_window),
                                         win_py_gxsm_action_entries, G_N_ELEMENTS (win_py_gxsm_action_entries),
                                         this);
        PI_DEBUG (DBG_L2,  "pyremote Plugin :: setup file menu" );

        // create window PopUp menu  ---------------------------------------------------------------------
        file_menu = gtk_menu_new_from_model (G_MENU_MODEL (gapp->get_plugin_pyremote_file_menu ()));
        g_assert (GTK_IS_MENU (file_menu));

        GtkIconSize tmp_toolbar_icon_size = GTK_ICON_SIZE_LARGE_TOOLBAR;

        // attach popup file menu button --------------------------------
        GtkWidget *header_menu_button = gtk_menu_button_new ();
        gtk_button_set_image (GTK_BUTTON (header_menu_button), gtk_image_new_from_icon_name ("document-open-symbolic", tmp_toolbar_icon_size));
        gtk_menu_button_set_popup (GTK_MENU_BUTTON (header_menu_button), file_menu);
        gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_menu_button);
        gtk_widget_show (header_menu_button);

        PI_DEBUG (DBG_L2,  "pyremote Plugin ::  header button" );

        // attach execute action buttons --------------------------------
        GtkWidget *header_action_button = gtk_button_new ();
        gtk_button_set_image (GTK_BUTTON (header_action_button), gtk_image_new_from_icon_name ("system-run-symbolic", tmp_toolbar_icon_size));
        gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_action_button);
        gtk_widget_show (header_action_button);
        g_signal_connect (header_action_button, "clicked", G_CALLBACK(py_gxsm_console::run_file), this);
        gtk_widget_set_tooltip_text (header_action_button, N_("Execute Script"));

        header_action_button = gtk_button_new ();
        gtk_button_set_image (GTK_BUTTON (header_action_button), gtk_image_new_from_icon_name ("edit-clear-all-symbolic", tmp_toolbar_icon_size));
        gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_action_button);
        gtk_widget_show (header_action_button);
        g_signal_connect (header_action_button, "clicked", G_CALLBACK(py_gxsm_console::clear_output), this);
        gtk_widget_set_tooltip_text (header_action_button, N_("Clear Console Output"));

        header_action_button = gtk_button_new ();
        gtk_button_set_image (GTK_BUTTON (header_action_button), gtk_image_new_from_icon_name ("system-shutdown-symbolic", tmp_toolbar_icon_size));
        gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), header_action_button);
        gtk_widget_show (header_action_button);
        g_signal_connect (header_action_button, "clicked", G_CALLBACK(py_gxsm_console::kill), this);
        gtk_widget_set_tooltip_text (header_action_button, N_("Abort Script"));

        PI_DEBUG (DBG_L2,  "pyremote Plugin :: setup titlbar" );

        gtk_window_set_title (GTK_WINDOW (window), title);
        gtk_header_bar_set_title ( GTK_HEADER_BAR (header_bar), title);
        if (script_filename)
                gtk_header_bar_set_subtitle (GTK_HEADER_BAR  (header_bar), script_filename);
        else
                gtk_header_bar_set_subtitle (GTK_HEADER_BAR  (header_bar), "no script file name.");
        gtk_window_set_titlebar (GTK_WINDOW (window), header_bar);

        g_signal_connect (G_OBJECT(window),
                          "delete_event",
                          G_CALLBACK(App::close_scan_event_cb),
                          this);

        v_grid = gtk_grid_new ();
        gtk_container_add (GTK_CONTAINER (window), v_grid);
        g_object_set_data (G_OBJECT (window), "v_grid", v_grid);

        gtk_widget_show_all (GTK_WIDGET (window));

        PI_DEBUG(DBG_L2, "pyremote Plugin :: AppWindoInit() -- building Console AppWindow -- calling GUI builder.");
        create_gui ();

        gui_ready = true;
        PI_DEBUG(DBG_L2, "pyremote Plugin :: AppWindoInit() -- Console AppWindow ready.");

        set_window_geometry ("python-console");
}

void py_gxsm_console::create_gui ()
{
        GtkWidget *console_scrolledwin, *file_scrolledwin, *vpaned, *hpaned_scpane, *frame, *sc_grid;
        GtkWidget *entry_input;

        GtkTextView *file_textview, *output_textview;
        //PangoFontDescription *font_desc;

        BuildParam *bp;
        BuildParam *bp_sc;
        UnitObj *null_unit;

#ifdef HAVE_GTKSOURCEVIEW
        GtkSourceLanguageManager *manager;
        GtkSourceBuffer *sourcebuffer;
        GtkSourceLanguage *language;
#endif
        PI_DEBUG(DBG_L2, "pyremote Plugin :: create_gui() -- building GUI elements.");

        sc_grid = gtk_grid_new ();
        bp = new BuildParam (v_grid, NULL, gapp->RemoteEntryList);

        // create static structure;
        exec_value = 50.0; // mid value

        // create GUI

        PI_DEBUG(DBG_L2, "pyremote Plugin :: create_gui() -- building GUI elements..");

        // window
        hpaned_scpane = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
        gtk_widget_set_vexpand (hpaned_scpane, TRUE);
        gtk_widget_set_hexpand (hpaned_scpane, TRUE);
        bp->grid_add_widget (hpaned_scpane, 100);

        vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
        gtk_widget_set_hexpand (vpaned, TRUE);
        gtk_widget_set_vexpand (vpaned, TRUE);

        gtk_paned_pack1(GTK_PANED(hpaned_scpane), vpaned, TRUE, TRUE);
        gtk_paned_pack2(GTK_PANED(hpaned_scpane), sc_grid, TRUE, TRUE);
        //bp->grid_add_widget (vpaned, 100);
        bp->new_line ();

        file_scrolledwin = gtk_scrolled_window_new(NULL, NULL);
        gtk_paned_pack1(GTK_PANED(vpaned), file_scrolledwin, TRUE, FALSE);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(file_scrolledwin),
                                            GTK_SHADOW_IN);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(file_scrolledwin),
                                       GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_AUTOMATIC);
        console_scrolledwin = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(console_scrolledwin),
                                       GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_AUTOMATIC);
        gtk_paned_pack2(GTK_PANED(vpaned), console_scrolledwin, TRUE, TRUE);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(console_scrolledwin),
                                            GTK_SHADOW_IN);

        PI_DEBUG(DBG_L2, "pyremote Plugin :: create_gui() -- building GUI elements...");

        // console output
        console_output = gtk_text_view_new();
        output_textview = GTK_TEXT_VIEW (console_output);
        /* create an auto-updating 'always at end' marker to scroll */
        GtkTextBuffer *text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (console_output));
        GtkTextIter text_iter_end;
        gtk_text_buffer_get_end_iter (text_buffer, &text_iter_end);

        console_mark_end = gtk_text_buffer_create_mark (text_buffer,
                                                        NULL,
                                                        &text_iter_end,
                                                        FALSE);

        gtk_container_add(GTK_CONTAINER(console_scrolledwin), console_output);
        gtk_text_view_set_editable(output_textview, FALSE);

        // source view file buffer
        sourcebuffer = gtk_source_buffer_new (NULL);
        console_file_content = gtk_source_view_new_with_buffer (sourcebuffer);
        file_textview = GTK_TEXT_VIEW(console_file_content);
        gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(file_textview), TRUE);
        gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(file_textview), TRUE);
        manager = gtk_source_language_manager_get_default();

        language = gtk_source_language_manager_get_language(manager, "pygwy");
        if (!language)
                language = gtk_source_language_manager_get_language(manager, "python");
        gtk_source_buffer_set_language(sourcebuffer, language);
        gtk_source_buffer_set_highlight_syntax(sourcebuffer, TRUE);

#if 0
        console_file_content = gtk_text_view_new();
        file_textview = GTK_TEXT_VIEW(console_file_content);
#endif

        PI_DEBUG(DBG_L2, "pyremote Plugin :: create_gui() -- building GUI elements....");
        // set font
        //font_desc = pango_font_description_from_string ("Monospace 8");
        //gtk_widget_override_font (console_file_content, font_desc);
        //gtk_widget_override_font (console_output, font_desc);
        //pango_font_description_free (font_desc);

        gtk_container_add (GTK_CONTAINER(file_scrolledwin),
                           console_file_content);
        gtk_text_view_set_editable (file_textview, TRUE);

        frame = gtk_frame_new (N_("Command"));
        entry_input = gtk_entry_new ();
        gtk_container_add (GTK_CONTAINER(frame), entry_input);
        gtk_entry_set_invisible_char (GTK_ENTRY(entry_input), 9679);
        gtk_widget_grab_focus (GTK_WIDGET(entry_input));
        bp->grid_add_widget (frame, 80);

        null_unit = new UnitObj(" "," ");
        bp->grid_add_ec ("Script Control", null_unit, &exec_value, 0.0, 100.0, "4g", 1., 10., "script-control");

        gapp->RemoteEntryList = bp->get_remote_list_head ();

        bp_sc = new BuildParam (sc_grid, NULL, gapp->RemoteEntryList);
        for(int i=0; i<NUM_SCV; ++i){
                sc_value[i] = 0.0;
                gchar *l = g_strdup_printf ("SC%d",i+1);
                gchar *id = g_strdup_printf ("py-sc%02d",i+1);
                bp_sc->grid_add_ec (l, null_unit, &sc_value[i], -1e10, 1e10, "g", 1., 10., id);
                sc_label[i] = bp_sc->label;
                g_free (l);
                g_free (id);
                bp_sc->new_line ();
        }
        gapp->RemoteEntryList = bp_sc->get_remote_list_head ();

        g_signal_connect(entry_input, "activate",
                         G_CALLBACK(py_gxsm_console::command_execute), this);

        PI_DEBUG(DBG_L2, "pyremote Plugin :: console_create_gui() -- building GUI elements.... hooking up");

        // connect on window close()
        g_signal_connect (window, "delete-event",
                          G_CALLBACK(gtk_widget_hide_on_delete), this);
        gtk_text_view_set_wrap_mode (output_textview, GTK_WRAP_WORD_CHAR);

        gtk_widget_show_all (v_grid);
        gtk_widget_show_all (sc_grid);

        gtk_window_resize (GTK_WINDOW(window), 600, 500);
        //gtk_paned_set_position (GTK_PANED(vpaned), 50);
        //gtk_paned_set_position (GTK_PANED(vpaned), -1); // 1:1 -- unset default
        gtk_paned_set_position (GTK_PANED(hpaned_scpane), 300);
        PI_DEBUG(DBG_L2, "pyremote Plugin :: console_create_gui() -- building GUI elements.... completed.");
}


// Small idiotic function to create a file with some pyremote example commands if none is found.
void  py_gxsm_console::write_example_file(void)
{
        std::ofstream example_file;
        example_file.open(example_filename);
        example_file << "\n#Since no default file / script was found, here are some\n"
                "# things you can do with the python interface.\n\n"
                "# Please visit/install gxsm3-lib-utils.py, ... before executing this:\n"
                "# Simply visit Menu/Libraries/Lib Utils first.\n"
                "\n"
                "#GXSM_USE_LIBRARY <gxsm3-lib-scan>\n"
                "\n"
                "# You can also create the more extensive default/example tools collection: File->Use default.\n"
                "# - see the manual for more information\n"
                "# Load scan support object from library\n"
                "\n"
                "scan = scan_control()\n"
                "\n"
                "# Execute to try these\n"
                "c = gxsm.get(\"script-control\")\n"
                "print \"Control value: \", c\n"
                "if c < 50:\n"
                "    print \"control value below limit - aborting!\"\n"
                "    print \"you can set the value in the window above\"\n"
                "    # you could set Script_Control like any other variable - see below\n"
                "else:\n"
                "    gxsm.set (\"RangeX\",\"200\")\n"
                "    gxsm.set (\"RangeY\",\"200\")\n"
                "    gxsm.set (\"PointsX\",\"100\")\n"
                "    gxsm.set (\"PointsY\",\"100\")\n"
                "    gxsm.set (\"dsp-fbs-bias\",\"0.1\")\n"
                "    gxsm.set (\"dsp-fbs-mx0-current-set\",\"1.5\")\n"
                "    gxsm.set (\"dsp-fbs-cp\", \"25\")\n"
                "    gxsm.set (\"dsp-fbs-ci\", \"20\")\n"
                "    gxsm.set (\"OffsetX\", \"0\")\n"
                "    gxsm.set (\"OffsetY\", \"0\")\n"
                "#    gxsm.action (\"DSP_CMD_AUTOAPP\")\n"
                "    gxsm.sleep (10)\n"
                "\n"
                "    #gxsm.startscan ()\n"
                "\n"
                "    scan.run_set ([-0.6, -0.4, -0.3])  # simply executes a scan for every bias voltage given in the list.\n"
                "\n"
                "    gxsm.moveto_scan_xy (100.,50.)\n";
        example_file.close();
}

void py_gxsm_console::run()
{
        PI_DEBUG_GM(DBG_L2, "pyremote Plugin :: console_run()");

        // are we up already? just make sure to (re) present window, else create
        if (gui_ready){
                gtk_window_present (GTK_WINDOW(window));
                return;
        } else {
                AppWindowInit (pyremote_pi.help);
        }

        script_filename = NULL;
        fail = false;

        script_filename = g_strdup_printf("%s.py", xsmres.PyremoteFile);
        query_filename = false;

        PI_DEBUG_GM(DBG_L1, "Pyremote console opening >%s< ", script_filename);
        open_file_callback (NULL, NULL, this);

        // put some small sommand example if no file is found
        if (fail) {
                PI_DEBUG(DBG_L1, "Pyremote console opening failed. Generating example.");
                query_filename = false;
                write_example_file();
                script_filename = g_strdup(example_filename);
                open_file_callback (NULL, NULL, this);
        }

        append("\n\n");
        append("WARNING: ================================================================================\n");
        append("WARNING: GXSM3 back port of beta GXSM4 work in progress: embedded python running in it's own thread.\n");
        append("WARNING: pending compete testing of a few less used python functions and thead save validation. \n");
        append("WARNING: ================================================================================\n\n");

        PI_DEBUG_GM(DBG_L2, "pyremote Plugin :: console_run() -- startup finished and ready. Standing by.");
}

void py_gxsm_console::command_execute(GtkEntry *entry, gpointer user_data)
{
        py_gxsm_console *pygc = (py_gxsm_console *)user_data;
        gchar *input_line;
        const gchar *command;
        GString *output;

        input_line = g_strconcat(">>> ", gtk_entry_buffer_get_text (GTK_ENTRY_BUFFER (gtk_entry_get_buffer (GTK_ENTRY (entry)))), "\n", NULL);
        output = g_string_new(input_line);
        command = gtk_entry_buffer_get_text (GTK_ENTRY_BUFFER (gtk_entry_get_buffer (GTK_ENTRY (entry))));
        output = g_string_append(output,
                                 pygc->run_command(command, Py_single_input,
                                         false, false, false));

        pygc->append((gchar *)output->str);
        g_string_free(output, TRUE);

        gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
}


// -------------------------------------------------- GXSM PLUGIN INTERFACE

static void run_action_script_callback (gpointer data){
        const gchar *name = *(gchar**)data;
        py_gxsm_remote_console->run_action_script (name);
}

// init-Function
static void pyremote_init(void)
{
        /* Python will search for remote.py in the directories, defined
           by PYTHONPATH. */
        PI_DEBUG_GM(DBG_L2, "pyremote Plugin Init");
        if (!getenv("PYTHONPATH")){
                PI_DEBUG_GM(DBG_L2, "pyremote: PYTHONPATH is not set.");
                PI_DEBUG_GM(DBG_L2, "pyremote: Setting to '.'");
                setenv("PYTHONPATH", ".", 0);
        }

        if (!py_gxsm_remote_console){
                py_gxsm_remote_console = new py_gxsm_console ();

                gapp->ConnectPluginToRemoteAction (run_action_script_callback);
        }

        py_gxsm_remote_console->run();
        g_thread_new (NULL, py_gxsm_console::PyRunConsoleThread, NULL);
}


// cleanup-Function
static void pyremote_cleanup(void)
{
        PI_DEBUG_GM(DBG_L2, "Pyremote Plugin Cleanup");
        if (py_gxsm_remote_console){
                PI_DEBUG(DBG_L3, "Pyremote Plugin: savinggeometry forced now.");
                py_gxsm_remote_console->SaveGeometry (); // some what needed and now it running the destruictor also. Weird.
                PI_DEBUG(DBG_L3, "Pyremote Plugin: closing up remote control console: delete py_gxsm_remote_console.");
                delete py_gxsm_remote_console;
        }
        py_gxsm_remote_console = NULL;
}
