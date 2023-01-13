#!/usr/bin/env python3

## * Python User Control for
## * Configuration and SPM Approach Control tool for the FB_spm DSP program/GXSM2 Mk2-Pro/A810 RTE based
## * for the Signal Ranger STD/SP2 DSP board
## * 
## * Copyright (C) 2010-13 Percy Zahl
## *
## * Author: Percy Zahl <zahl@users.sf.net>
## * WWW Home: http://sranger.sf.net
## *
## * This program is free software; you can redistribute it and/or modify
## * it under the terms of the GNU General Public License as published by
## * the Free Software Foundation; either version 2 of the License, or
## * (at your option) any later version.
## *
## * This program is distributed in the hope that it will be useful,
## * but WITHOUT ANY WARRANTY; without even the implied warranty of
## * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## * GNU General Public License for more details.
## *
## * You should have received a copy of the GNU General Public License
## * along with this program; if not, write to the Free Software
## * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

#### get numpy, scipy:
#### sudo apt-get install python-numpy python-scipy python-matplotlib ipython ipython-notebook python-pandas python-sympy python-nose
import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GLib

import time

import struct
import array

from numpy import *
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.ticker import LinearLocator, FormatStrFormatter
from matplotlib.animation import FuncAnimation

import math
import cmath
import re

import pickle
import pydot
#import xdot
from scipy.optimize import leastsq

from mk2_rtengine_controlclass import *
from meterwidget import *
from scopewidget import *

import datetime
import urllib3

from influxdb import InfluxDBClient

from colorsys import hls_to_rgb


urllib3.disable_warnings()
influxdb_client = InfluxDBClient(host='localhost', port=8086) ##, ssl=True, verify_ssl=False)

# timeouts [ms]
timeout_update_patchrack              = 3000
timeout_update_patchmenu              = 3000
timeout_update_A810_readings          =  120
timeout_update_signal_monitor_menu    = 2000
timeout_update_signal_monitor_reading =  500
timeout_update_meter_readings         =   88
timeout_update_process_list           =  100

timeout_min_recorder          =   80
timeout_min_tunescope         =   20
timeout_tunescope_default     =  150

timeout_DSP_status_reading        =  100
timeout_DSP_signal_lookup_reading = 2100


## Si Diode Curve 10 -- i.e for DT-400
## [ T(K), Volts, dV/dT (mV/K) ]
curve_10 = [
  [  1.40, 1.69812, -13.1 ],
  [  1.60, 1.69521, -15.9 ],
  [  1.80, 1.69177, -18.4 ],
  [  2.00, 1.68786, -20.7 ],
  [  2.20, 1.68352, -22.7 ],
  [  2.40, 1.67880, -24.4 ],
  [  2.60, 1.67376, -25.9 ],
  [  2.80, 1.66845, -27.1 ],
  [  3.00, 1.66292, -28.1 ],
  [  3.20, 1.65721, -29.0 ],
  [  3.40, 1.65134, -29.8 ],
  [  3.60, 1.64529, -30.7 ],
  [  3.80, 1.63905, -31.6 ],
  [  4.00, 1.63263, -32.7 ],
  [  4.20, 1.62602, -33.6 ],
  [  4.40, 1.61920, -34.6 ],
  [  4.60, 1.61220, -35.4 ],
  [  4.80, 1.60506, -36.0 ],
  [  5.00, 1.59782, -36.5 ],
  [  5.50, 1.57928, -37.6 ],
  [  6.00, 1.56027, -38.4 ],
  [  6.50, 1.54097, -38.7 ],
  [  7.00, 1.52166, -38.4 ],
  [  7.50, 1.50272, -37.3 ],
  [  8.00, 1.48443, -35.8 ],
  [  8.50, 1.46700, -34.0 ],
  [  9.00, 1.45048, -32.1 ],
  [  9.50, 1.43488, -30.3 ],
  [  10.0, 1.42013, -28.7 ],
  [  10.5, 1.40615, -27.2 ],
  [  11.0, 1.39287, -25.9 ],
  [  11.5, 1.38021, -24.8 ],
  [  12.0, 1.36809, -23.7 ],
  [  12.5, 1.35647, -22.8 ],
  [  13.0, 1.34530, -21.9 ],
  [  13.5, 1.33453, -21.2 ],
  [  14.0, 1.32412, -20.5 ],
  [  14.5, 1.31403, -19.9 ],
  [  15.0, 1.30422, -19.4 ],
  [  15.5, 1.29464, -18.9 ],
  [  16.0, 1.28527, -18.6 ],
  [  16.5, 1.27607, -18.2 ],
  [  17.0, 1.26702, -18.0 ],
  [  17.5, 1.25810, -17.7 ],
  [  18.0, 1.24928, -17.6 ],
  [  18.5, 1.24053, -17.4 ],
  [  19.0, 1.23184, -17.4 ],
  [  19.5, 1.22314, -17.4 ],
  [  20.0, 1.21440, -17.6 ],
  [  21.0, 1.19645, -18.5 ],
  [  22.0, 1.17705, -20.6 ],
  [  23.0, 1.15558, -21.7 ],
  [  24.0, 1.13598, -15.9 ],
  [  25.0, 1.12463, -7.72 ],
  [  26.0, 1.11896, -4.34 ],
  [  27.0, 1.11517, -3.34 ],
  [  28.0, 1.11212, -2.82 ],
  [  29.0, 1.10945, -2.53 ],
  [  30.0, 1.10702, -2.34 ],
  [  32.0, 1.10263, -2.08 ],
  [  34.0, 1.09864, -1.92 ],
  [  36.0, 1.09490, -1.83 ],
  [  38.0, 1.09131, -1.77 ],
  [  40.0, 1.08781, -1.74 ],
  [  42.0, 1.08436, -1.72 ],
  [  44.0, 1.08093, -1.72 ],
  [  46.0, 1.07748, -1.73 ],
  [  48.0, 1.07402, -1.74 ],
  [  50.0, 1.07053, -1.75 ],
  [  52.0, 1.06700, -1.77 ],
  [  54.0, 1.06346, -1.78 ],
  [  56.0, 1.05988, -1.79 ],
  [  58.0, 1.05629, -1.80 ],
  [  60.0, 1.05267, -1.81 ],
  [  65.0, 1.04353, -1.84 ],
  [  70.0, 1.03425, -1.87 ],
  [  75.0, 1.02482, -1.91 ],
  [  77.35, 1.02032, -1.92 ],
  [  80.0, 1.01525, -1.93 ],
  [  85.0, 1.00552, -1.96 ],
  [  90.0, 0.99565, -1.99 ],
  [  95.0, 0.98564, -2.02 ],
  [  100.0, 0.97550, -2.04 ],
  [  110.0, 0.95487, -2.08 ],
  [  120.0, 0.93383, -2.12 ],
  [  130.0, 0.91243, -2.16 ],
  [  140.0, 0.89072, -2.19 ],
  [  150.0, 0.86873, -2.21 ],
  [  160.0, 0.84650, -2.24 ],
  [  170.0, 0.82404, -2.26 ],
  [  180.0, 0.80138, -2.28 ],
  [  190.0, 0.77855, -2.29 ],
  [  200.0, 0.75554, -2.31 ],
  [  210.0, 0.73238, -2.32 ],
  [  220.0, 0.70908, -2.34 ],
  [  230.0, 0.68564, -2.35 ],
  [  240.0, 0.66208, -2.36 ],
  [  250.0, 0.63841, -2.37 ],
  [  260.0, 0.61465, -2.38 ],
  [  270.0, 0.59080, -2.39 ],
  [  273.15, 0.58327, -2.39 ],
  [  280.0, 0.56690, -2.39 ],
  [  290.0, 0.54294, -2.40 ],
  [  300.0, 0.51892, -2.40 ],
  [  305.0, 0.50688, -2.41 ],
  [  310.0, 0.49484, -2.41 ],
  [  320.0, 0.47069, -2.42 ],
  [  330.0, 0.44647, -2.42 ],
  [  340.0, 0.42221, -2.43 ],
  [  350.0, 0.39783, -2.44 ],
  [  360.0, 0.37337, -2.45 ],
  [  370.0, 0.34881, -2.46 ],
  [  380.0, 0.32416, -2.47 ],
  [  390.0, 0.29941, -2.48 ],
  [  400.0, 0.27456, -2.49 ],
  [  410.0, 0.24963, -2.50 ],
  [  420.0, 0.22463, -2.50 ],
  [  430.0, 0.19961, -2.50 ],
  [  440.0, 0.17464, -2.49 ],
  [  450.0, 0.14985, -2.46 ],
  [  460.0, 0.12547, -2.41 ],
  [  470.0, 0.10191, -2.30 ],
  [  475.0, 0.09062, -2.22 ],
  [  10000.0, 0.0, 0.0 ]
]


    
 


        
## [ T(K), Volts, dV/dT (mV/K) ]
def v2k(v):
    for dtv in curve_10:
        dv = v-dtv[1]
        if dv >= 0.0:
            return dtv[0] + dv*1000./dtv[2]

#np_v2k = np.vectorize(v2k)


# class MyDotWindow(xdot.DotWindow):
#
#     def __init__(self):
#         xdot.DotWindow.__init__(self)
#         self.widget.connect('clicked', self.on_url_clicked)
#
#     def on_url_clicked(self, widget, url, event):
#         dialog = Gtk.MessageDialog(
#             parent = self, 
#             buttons = Gtk.BUTTONS_OK,
#             message_format="%s clicked" % url)
#         dialog.connect('response', lambda dialog, response: dialog.destroy())
#         dialog.run()
#         return True

class Visualize():
    def __init__(self, parent, shownullsignals=0):
        # http://www.graphviz.org/Documentation.php
        graph = pydot.Dot(graph_type='digraph', compound='true', rankdir="LR", overlap="false") #, splines="ortho")

        moduleg = {}
        signaln = {}
        inputn = {}
        node_id = 0

        moduleg ["NodeTypes"] = pydot.Cluster(graph_name="Legend",
                              style="filled",
                              color="lightgrey",
                              label = "Node Types:")
        c = moduleg ["NodeTypes"]
        graph.add_subgraph (c)
        c.add_node ( pydot.Node( "Process Flow", style="filled", fillcolor="lightskyblue") )
        c.add_node ( pydot.Node( "Signal Source", style="filled", fillcolor="green") )
        c.add_node ( pydot.Node( "Modul Input", style="filled", fillcolor="gold") )
        c.add_node ( pydot.Node( "Modul Input=0", style="filled", fillcolor="grey") )
        c.add_node ( pydot.Node( "DISABLED (NULL Ptr)", style="filled", fillcolor="grey97") )
        c.add_node ( pydot.Node( "Signal Manipulation", style="filled", fillcolor="cyan") )
        c.add_node ( pydot.Node( "Unmanaged Node (PLL, ..)", style="filled", fillcolor="purple") )
        c.add_node ( pydot.Node( "Error Signal not found", style="filled", fillcolor="red") )

        mod_conf =  parent.mk2rte.get_module_configuration_list()
        for mod in mod_conf.keys():
            moduleg [mod] = pydot.Cluster(graph_name=mod,
                              style="filled",
                              color="lightgrey",
                              label = mod)
            c = moduleg [mod]
            print ("AddSubgraph: "+mod)
            graph.add_subgraph (c)
            signaln [mod] = pydot.Node( mod, style="filled", fillcolor="lightskyblue")
            c.add_node( signaln [mod] )

            record = ""
            fi=1
            for signal in parent.mk2rte.get_signal_lookup_list ():
                if signal[SIG_ADR] > 0:
                    if signal[SIG_GRP] == mod:
                        if re.search ("In\ [0-7]", signal[SIG_NAME]):
                            if record == "":
                                record = "<f0> "+re.escape (signal[SIG_NAME])
                            else:
                                record = record + "|<f%d> "%fi + re.escape (signal[SIG_NAME])
                                fi=fi+1
                            print ("added to record: " + signal[SIG_NAME] + " ==> " + record)
                        signaln [signal[SIG_NAME]] = pydot.Node( signal[SIG_NAME], style="filled", fillcolor="green")
                        c.add_node( signaln [signal[SIG_NAME]] )
#            if record != "":
#                signaln ["record_IN"] = pydot.Node ("record_IN", label=record, shape="record", style="filled", fillcolor="green")
#                c.add_node( signaln ["record_IN"] )

        for mod in mod_conf.keys():
            c = moduleg [mod]
            for mod_inp in mod_conf[mod]:
                [signal, data, offset] = parent.mk2rte.query_module_signal_input(mod_inp[0])
                # Null-Signal -- omit node connections, shade node
                if signal[SIG_VAR] == "analog.vnull":
                    nodecolor="grey"
                else:
                    nodecolor=mod_inp[4]
                print ("INPUT NODE: ", mod_inp)
                if mod_inp[2] >= 0 and signal[SIG_INDEX] != -1:
                    inputn [mod_inp[1]] = pydot.Node( mod_inp[1], style="filled", fillcolor=nodecolor)
                    c.add_node( inputn [mod_inp[1]] )
                    print (signal[SIG_NAME]," ==> ",mod_inp[1],"     (",mod_inp, signal, data, ")")
                    if signal[SIG_VAR] != "analog.vnull" or shownullsignals:
                        if nodecolor == "cyan":
                            sigdir="back"
                            sigcol="magenta"
                        else:
                            sigdir="forward"
                            sigcol="red"
                        edge = pydot.Edge(signaln [signal[SIG_NAME]], inputn [mod_inp[1]], color=sigcol, dir=sigdir)
                        graph.add_edge(edge)
                        if re.search ("In\ [0-7]", signal[SIG_NAME]):
                            signal_f = "f%s"%re.sub("In ","",signal[SIG_NAME])
                            edge = pydot.Edge("record_IN:f0", inputn [mod_inp[1]], color=sigcol, dir=sigdir)

                else:
                    if mod_inp[3] == 1:
                        print ("DISABLED [p=0] ==> ",mod_inp[1],"  (",mod_inp, signal, data, ")")
                        inputn [mod_inp[1]] = pydot.Node( mod_inp[1], style="filled", fillcolor="grey97")
                    else:
                        if mod_inp[4] == "purple":
                            print ("UNMANAGED NODE (PLL, etc.) ==> ", mod_inp[1],"  (",mod_inp, signal, data, ")")
                            inputn [mod_inp[1]] = pydot.Node( mod_inp[1], style="filled", fillcolor=mod_inp[4])
                        else:
                            print ("ERROR: ",mod_inp, signal, data, " ** can not find signal.")
                            inputn [mod_inp[1]] = pydot.Node( mod_inp[1], style="filled", fillcolor="red")
                    c.add_node( inputn [mod_inp[1]] )

        # processing order
        edge = pydot.Edge(signaln ["DSP"], signaln ["Analog_IN"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["Analog_IN"], signaln ["PAC"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["PAC"], signaln ["Scope"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["PAC"], signaln ["Mixer"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["Mixer"], signaln ["RMS"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["Mixer"], signaln ["DBGX_Mixer"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["RMS"], signaln ["Counter"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["Counter"], signaln ["Control"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["Control"], signaln ["VP"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["VP"], signaln ["Scan"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["VP"], signaln ["LockIn"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["Scan"], signaln ["Z_Servo"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["Z_Servo"], signaln ["M_Servo"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["M_Servo"], signaln ["Analog_OUT"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["Analog_OUT"], signaln ["Coarse"], arrowhead="empty", color="green", weight="1000")
        graph.add_edge(edge)    

        # add fixed relations
        edge = pydot.Edge(signaln ["Bias Adjust"], signaln ["VP Bias"], arrowhead="open")
        graph.add_edge(edge)    

        ANALOG_RELATIONS = {
            "DIFF_IN0":"In 0",
            "DIFF_IN1":"In 1",
            "DIFF_IN2":"In 2",
            "DIFF_IN3":"In 3"
            }
        for inp in ANALOG_RELATIONS.keys():
            edge = pydot.Edge(inputn [inp], signaln [ANALOG_RELATIONS[inp]], arrowhead="invdot", weight="1000")
            graph.add_edge(edge)    

        # PAC / PLL assignment 
        edge = pydot.Edge(signaln ["In 4"], inputn ["PLL_INPUT"], arrowhead="empty", color="purple")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["PLL Exci Signal"], inputn ["OUTMIX_CH7"], arrowhead="empty", color="purple")
        graph.add_edge(edge)    

        MIXER_RELATIONS = {
            "MIXER0":["MIX 0", "MIX0 qfac15 LP", "IIR32 0"],
            "MIXER1":["MIX 1", "IIR32 1"],
            "MIXER2":["MIX 2", "IIR32 2"],
            "MIXER3":["MIX 3", "IIR32 3"]
            }

#        for inp in MIXER_RELATIONS.keys():
#            for s in MIXER_RELATIONS[inp]:
#                                print ("processing MIXER_REL: ", s, inp)
#                edge = pydot.Edge(inputn [inp], signaln [s], arrowhead="diamond", weight="2000")
#                graph.add_edge(edge)    
#        edge = pydot.Edge(signaln ["MIX 0"], signaln ["MIX out delta"], arrowhead="open", weight="100")
#        graph.add_edge(edge)    
#        edge = pydot.Edge(signaln ["MIX 1"], signaln ["MIX out delta"], arrowhead="open", weight="100")
#        graph.add_edge(edge)    
#        edge = pydot.Edge(signaln ["MIX 2"], signaln ["MIX out delta"], arrowhead="open", weight="100")
#        graph.add_edge(edge)    
#        edge = pydot.Edge(signaln ["MIX 3"], signaln ["MIX out delta"], arrowhead="open", weight="100")
#        graph.add_edge(edge)    

        edge = pydot.Edge(inputn ["ANALOG_AVG_INPUT"], signaln ["signal AVG-256"], arrowhead="invdot", weight="1000")
        graph.add_edge(edge)    
        edge = pydot.Edge(signaln ["signal AVG-256"], signaln ["signal RMS-256"], arrowhead="open", weight="100")
        graph.add_edge(edge)    

        OUTMIX_RELATIONS = {
            "OUTMIX_CH0":["OUTMIX_CH0_ADD_A","OUTMIX_CH0_SUB_B","OUTMIX_CH0_SMAC_A","OUTMIX_CH0_SMAC_B","Out 0"],
            "OUTMIX_CH1":["OUTMIX_CH1_ADD_A","OUTMIX_CH1_SUB_B","OUTMIX_CH1_SMAC_A","OUTMIX_CH1_SMAC_B","Out 1"],
            "OUTMIX_CH2":["OUTMIX_CH2_ADD_A","OUTMIX_CH2_SUB_B","OUTMIX_CH2_SMAC_A","OUTMIX_CH2_SMAC_B","Out 2"],
            "OUTMIX_CH3":["OUTMIX_CH3_ADD_A","OUTMIX_CH3_SUB_B","OUTMIX_CH3_SMAC_A","OUTMIX_CH3_SMAC_B","Out 3"],
            "OUTMIX_CH4":["OUTMIX_CH4_ADD_A","OUTMIX_CH4_SUB_B","OUTMIX_CH4_SMAC_A","OUTMIX_CH4_SMAC_B","Out 4"],
            "OUTMIX_CH5":["OUTMIX_CH5_ADD_A","OUTMIX_CH5_SUB_B","OUTMIX_CH5_SMAC_A","OUTMIX_CH5_SMAC_B","Out 5"],
            "OUTMIX_CH6":["OUTMIX_CH6_ADD_A","OUTMIX_CH6_SUB_B","OUTMIX_CH6_SMAC_A","OUTMIX_CH6_SMAC_B","Out 6"],
            "OUTMIX_CH7":["OUTMIX_CH7_ADD_A","OUTMIX_CH7_SUB_B","OUTMIX_CH7_SMAC_A","OUTMIX_CH7_SMAC_B","Out 7"],
            "OUTMIX_CH0_SMAC_A":["OUTMIX_CH0_ADD_A"],
            "OUTMIX_CH0_SMAC_B":["OUTMIX_CH0_SUB_B"],
            "OUTMIX_CH1_SMAC_A":["OUTMIX_CH1_ADD_A"],
            "OUTMIX_CH1_SMAC_B":["OUTMIX_CH1_SUB_B"],
            "OUTMIX_CH2_SMAC_A":["OUTMIX_CH2_ADD_A"],
            "OUTMIX_CH2_SMAC_B":["OUTMIX_CH2_SUB_B"],
            "OUTMIX_CH3_SMAC_A":["OUTMIX_CH3_ADD_A"],
            "OUTMIX_CH3_SMAC_B":["OUTMIX_CH3_SUB_B"],
            "OUTMIX_CH4_SMAC_A":["OUTMIX_CH4_ADD_A"],
            "OUTMIX_CH4_SMAC_B":["OUTMIX_CH4_SUB_B"],
            "OUTMIX_CH5_SMAC_A":["OUTMIX_CH5_ADD_A"],
            "OUTMIX_CH5_SMAC_B":["OUTMIX_CH5_SUB_B"],
            "OUTMIX_CH6_SMAC_A":["OUTMIX_CH6_ADD_A"],
            "OUTMIX_CH6_SMAC_B":["OUTMIX_CH6_SUB_B"],
            "OUTMIX_CH7_SMAC_A":["OUTMIX_CH7_ADD_A"],
            "OUTMIX_CH7_SMAC_B":["OUTMIX_CH7_SUB_B"],
            "OUTMIX_CH8":["OUTMIX_CH8_ADD_A","Wave Out 8"],
            "OUTMIX_CH9":["OUTMIX_CH9_ADD_A","Wave Out 9"]
            }
        for inp in OUTMIX_RELATIONS.keys():
            for inp2 in OUTMIX_RELATIONS[inp]:
                if re.search ("SMAC", inp):
                    edge = pydot.Edge(inputn [inp], inputn [inp2], arrowhead="invdot", color="blue", weight="3000")
                else:
                    if re.search ("Out", inp2):
                        edge = pydot.Edge(inputn [inp], signaln [inp2], weight="4000")
                    else:
                        edge = pydot.Edge(inputn [inp], inputn [inp2], arrowhead="obox", weight="2000")
                graph.add_edge(edge)    

        LOCKIN_RELATIONS = {
            "LOCKIN_A":["LockIn A-0","LockIn A-1st","LockIn A-2nd","LockIn Ref"],
            "LOCKIN_B":["LockIn B-0","LockIn B-1st","LockIn B-2nd","LockIn Ref"]
            }
        for inp in LOCKIN_RELATIONS.keys():
            for inp2 in LOCKIN_RELATIONS[inp]:
                edge = pydot.Edge(inputn [inp], signaln [inp2], arrowhead="ediamond", weight="2000")
                graph.add_edge(edge)    


        SERVO_RELATIONS = {
            "M_SERVO":["M Servo", "M Servo Neg"],
            "Z_SERVO":["Z Servo", "Z Servo Neg"]
            }
        for inp in SERVO_RELATIONS.keys():
            for s in SERVO_RELATIONS[inp]:
                edge = pydot.Edge(inputn [inp], signaln [s], arrowhead="dot", weight="1000")
                graph.add_edge(edge)    

    #    graph.write_png('signal_graph.png')
        if os.path.exists("/usr/share/gxsm/pixmaps"):
        # use directory /usr/share/gxsm/pixmaps
            graph.write_svg('/usr/share/gxsm/pixmaps/signal_graph.svg')
            graph.write_dot('/usr/share/gxsm/pixmaps/signal_graph.dot')
            os.system("xdot /usr/share/gxsm/pixmaps/signal_graph.dot&")
            # os.system("eog /usr/share/gxsm/pixmaps/signal_graph.svg &")
        else:
        # use working directory, possibly /gxsm-svn/Gxsm-2.0/plug-ins/hard/MK3-A810_spmcontrol/python_scripts
            graph.write_svg('signal_graph.svg')
            graph.write_dot('signal_graph.dot')
            os.system("xdot signal_graph.dot&")    
            # os.system("eog signal.svg &")

    #    dotwindow = MyDotWindow()
    #    dotwindow.set_dotcode(graph.dot)

        if 0:
            png_str = graph.create_png(prog='dot')

        # treat the dot output string as an image file
            sio = StringIO()
            sio.write(png_str)
            sio.seek(0)
            img = mpimg.imread(sio)

        # plot the image
            imgplot = plt.imshow(img, aspect='equal')
        # plt.show(block=False)
            plt.show()

class SignalScope():
    def __init__(self, parent, mode="", single_shot=0, blen=5000):
        [Xsignal, Xdata, Offset] = parent.mk2rte.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL1_INPUT_ID)
        [Ysignal, Ydata, Offset] = parent.mk2rte.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL2_INPUT_ID)

        label = "Oscilloscope/Recorder" + mode
        name=label
        self.ss = single_shot
        self.block_length = blen
        self.restart_func = self.nop
        self.trigger = 0
        self.trigger_level = 0
        if name not in wins:
            win = Gtk.Window()
            parent.wins[name] = win
            v = Gtk.VBox()
    
            scope = Oscilloscope( Gtk.Label(), v, "XT", label)
            scope.scope.set_wide (True)
            scope.show()
            scope.set_chinfo([Xsignal[SIG_NAME], Ysignal[SIG_NAME]])
            win.add(v)
            if not self.ss:
                self.block_length = 512
                parent.mk2rte.set_recorder (self.block_length, self.trigger, self.trigger_level)
    
            table = Gtk.Table(n_rows=4, n_columns=2)
            v.pack_start (table, True, True, 0)
            table.show()
    
            tr=0
            lab = Gtk.Label(label="# Samples:")
            lab.show ()
            table.attach(lab, 2, 3, tr, tr+1)
            Samples = Gtk.Entry()
            Samples.set_text("%d"%self.block_length)
            table.attach(Samples, 2, 3, tr+1, tr+2)
            Samples.show()
    
            [signal,data,offset] = parent.mk2rte.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL1_INPUT_ID)
            combo = parent.make_signal_selector (DSP_SIGNAL_SCOPE_SIGNAL1_INPUT_ID, signal, "CH1: ", parent.global_vector_index)
            table.attach(combo, 0, 1, tr, tr+1)
            Xscale = Gtk.Entry()
            Xscale.set_text("1.")
            table.attach(Xscale, 0, 1, tr+1, tr+2)
            Xscale.show()
    
            [signal,data,offset] = parent.mk2rte.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL2_INPUT_ID)
            combo = parent.make_signal_selector (DSP_SIGNAL_SCOPE_SIGNAL2_INPUT_ID, signal, "CH2: ", parent.global_vector_index)
            table.attach(combo, 1, 2, tr, tr+1)
            Yscale = Gtk.Entry()
            Yscale.set_text("1.")
            table.attach(Yscale, 1, 2, tr+1, tr+2)
            Yscale.show()
    
            tr = tr+2
            labx0 = Gtk.Label( label="X-off:")
            labx0.show ()
            table.attach(labx0, 0, 1, tr, tr+1)
            Xoff = Gtk.Entry()
            Xoff.set_text("0")
            table.attach(Xoff, 0, 1, tr+1, tr+2)
            Xoff.show()
            laby0 = Gtk.Label( label="Y-off:")
            laby0.show ()
            table.attach(laby0, 1, 2, tr, tr+1)
            Yoff = Gtk.Entry()
            Yoff.set_text("0")
            table.attach(Yoff, 1, 2, tr+1, tr+2)
            Yoff.show()
    
            self.xdc = 0.
            self.ydc = 0.
    
            def update_scope(set_data, Xsignal, Ysignal, xs, ys, x0, y0, num, x0l, y0l):
                blck = parent.mk2rte.check_recorder_ready ()
                if blck == -1:
                    n = self.block_length
                    [xd, yd] = parent.mk2rte.read_recorder (n)
    
                    if not self.run:
                        if n < 128:
                            print ("CH1:")
                            print (xd)
                            print ("CH2:")
                            print (yd)
                        else:
                            save("mk3_recorder_xdata", xd)
                            save("mk3_recorder_ydata", yd)
                            scope.set_flash ("Saved: mk3_recorder_[xy]data")
    
                    # auto subsample if big
                    nss = n
                    nraw = n
                    if n > 8192:
                        ### CHECK DOWN SAMPLING METHODE BELOW, SEAMS TO DROP POINTS EVEN IF MULTIPE OF 2
                        ss = int(n/8192)
                        end =  ss * int(len(xd)/ss)
                        nss = (int)(n/ss)
                        xd = mean(xd[:end].reshape(-1, ss), 1)
                        yd = mean(yd[:end].reshape(-1, ss), 1)
                        scope.set_info(["sub sampling: %d"%n + " by %d"%ss + " [nss=%d"%nss + ",end=%d]"%end,
                                "T = %g ms"%(n/150.),
                                mode])
                        scope.set_subsample_factor(ss)
                    else:
                        scope.set_info([
                                "T = %g ms"%(n/150.),
                                mode])
                        scope.set_subsample_factor(1)
    
                    # change number samples?
                    try:
                        self.block_length = int(num())
                        if self.block_length < 64:
                            self.block_length = 64
                        if self.block_length > 999999:
                            print ("MAX BLOCK LEN is 999999")
                            self.block_length = 1024
                    except ValueError:
                        self.block_length = 512
    
                    if self.block_length != n or self.ss:
                        self.run = False
                        self.run_button.set_label("RESTART")
    
                    if not self.ss:
                        parent.mk2rte.set_recorder (self.block_length, self.trigger, self.trigger_level)
                    #                v = value * signal[SIG_D2U]
                    #                maxv = (1<<31)*math.fabs(signal[SIG_D2U])
                    try:
                        xscale_div = float(xs())
                    except ValueError:
                        xscale_div = 1
    
                    try:
                        yscale_div = float(ys())
                    except ValueError:
                        yscale_div = 1
    
                    n = nss
                    try:
                        self.xoff = float(x0())
                        for i in range (0, n, 8):
                            self.xdc = 0.9 * self.xdc + 0.1 * xd[i] * Xsignal[SIG_D2U]
                        x0l("X-DC = %g"%self.xdc)
                    except ValueError:
                        for i in range (0, n, 8):
                            self.xoff = 0.9 * self.xoff + 0.1 * xd[i] * Xsignal[SIG_D2U]
                        x0l("X-off: %g"%self.xoff)
    
                    try:
                        self.yoff = float(y0())
                        for i in range (0, n, 8):
                            self.ydc = 0.9 * self.ydc + 0.1 * yd[i] * Ysignal[SIG_D2U]
                        y0l("Y-DC = %g"%self.ydc)
                    except ValueError:
                        for i in range (0, n, 8):
                            self.yoff = 0.9 * self.yoff + 0.1 * yd[i] * Ysignal[SIG_D2U]
                        y0l("Y-off: %g"%self.yoff)
    
                    if math.fabs(xscale_div) > 0:
                        xd = -(xd * Xsignal[SIG_D2U] - self.xoff) / xscale_div
                    else:
                        xd = xd * 0. # ZERO/OFF
                        
                    self.trigger_level = self.xoff / Xsignal[SIG_D2U]
    
                    if math.fabs(yscale_div) > 0:
                        yd = -(yd * Ysignal[SIG_D2U] - self.yoff) / yscale_div
                    else:
                        yd = yd * 0. # ZERO/OFF
    
                    scope.set_scale ( { 
                            "CH1:": "%g"%xscale_div + " %s"%Xsignal[SIG_UNIT],
                            "CH2:": "%g"%yscale_div + " %s"%Xsignal[SIG_UNIT],
                            "Timebase:": "%g ms"%(nraw/150./20.) 
                            })
                    
                    scope.set_data (xd, yd)
    
                    if self.mode > 1:
                        self.run_button.set_label("SINGLE")
                else:
                    scope.set_info(["waiting for trigger or data [%d]"%blck])
                    scope.queue_draw()
    
    
                return self.run
    
            def stop_update_scope (win, event=None):
                print ("STOP, hide.")
                win.hide()
                self.run = False
                return True
    
            def toggle_run_recorder (b):
                if self.run:
                    self.run = False
                    self.run_button.set_label("RUN")
                else:
                    self.restart_func ()
    
                    [Xsignal, Xdata, Offset] = parent.mk2rte.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL1_INPUT_ID)
                    [Ysignal, Ydata, Offset] = parent.mk2rte.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL2_INPUT_ID)
                    scope.set_chinfo([Xsignal[SIG_NAME], Ysignal[SIG_NAME]])
    
                    period = int(2.*self.block_length/150.)
                    if period < timeout_min_recorder:
                        period = timeout_min_recorder
    
                    if self.mode < 2: 
                        self.run_button.set_label("STOP")
                        self.run = True
                    else:
                        self.run_button.set_label("ARMED")
                        parent.mk2rte.set_recorder (self.block_length, self.trigger, self.trigger_level)
                        self.run = False
    
                    GLib.timeout_add (period, update_scope, scope, Xsignal, Ysignal, Xscale.get_text, Yscale.get_text, Xoff.get_text, Yoff.get_text, Samples.get_text, labx0.set_text, laby0.set_text)
    
            def toggle_trigger (b):
                if self.trigger == 0:
                    self.trigger = 1
                    self.trigger_button.set_label("TRIGGER POS")
                else:
                    if self.trigger > 0:
                        self.trigger = -1
                        self.trigger_button.set_label("TRIGGER NEG")
                    else:
                        self.trigger = 0
                        self.trigger_button.set_label("TRIGGER OFF")
                print (self.trigger, self.trigger_level)
    
            def toggle_mode (b):
                if self.mode == 0:
                    self.mode = 1
                    self.ss = 0
                    self.mode_button.set_label("T-Auto")
                else:
                    if self.mode == 1:
                        self.mode = 2
                        self.ss = 0
                        self.mode_button.set_label("T-Normal")
                    else:
                        if self.mode == 2:
                            self.mode = 3
                            self.ss = 1
                            self.mode_button.set_label("T-Single")
                        else:
                            self.mode = 0
                            self.ss = 0
                            self.mode_button.set_label("T-Free")
    
            self.run_button = Gtk.Button("STOP")
    #        self.run_button.set_use_stock(True)
            self.run_button.connect("clicked", toggle_run_recorder)
            self.hb = Gtk.HBox()
            self.hb.pack_start (self.run_button, True, True, 0)
            self.mode_button = Gtk.Button("M: Free")
            self.mode=0 # 0 free( 1 auto, 2 nommal, 3 single
            self.mode_button.connect("clicked", toggle_mode)
            self.hb.pack_start (self.mode_button, True, True, 0)
            self.mode_button.show ()
            table.attach(self.hb, 2, 3, tr, tr+1)
            tr = tr+1
    
            self.trigger_button = Gtk.Button("TRIGGER-OFF")
            self.trigger_button.connect("clicked", toggle_trigger)
            self.trigger_button.show ()
            table.attach(self.trigger_button, 2, 3, tr, tr+1)
    
            self.run = False
            win.connect("delete_event", stop_update_scope)
            toggle_run_recorder (self.run_button)
            
            parent.wins[name].show_all()
    
        def set_restart_callback (self, func):
            self.restart_func = func
                
    def nop (self):
        return 0
            


class SignalPlotter():
    # X: 7=time, plotting Monitor Taps 20,21,0,1
    def __init__(self, parent, length = 300., taps=[7,20,21,0,1], samplesperpage=2000):
        self.monitor_taps = taps
        [value, uv1, Xsignal] = parent.mk2rte.get_monitor_signal (self.monitor_taps[1])
        [value, uv2, Ysignal] = parent.mk2rte.get_monitor_signal (self.monitor_taps[2])
        [value, uv3, Usignal] = parent.mk2rte.get_monitor_signal (self.monitor_taps[3])
        [value, uv4, Vsignal] = parent.mk2rte.get_monitor_signal (self.monitor_taps[4])

        label = "Signal Plotter -- Monitor Indexes 20,21,0,1"
        name  = "Signal Plotter"
    
        if name not in wins:
            win = Gtk.Window()
            parent.wins[name] = win
            v = Gtk.HBox()
                
            self.pos      = 0
            self.span     = length
            self.points   = samplesperpage
            self.dt       = self.span/self.points
            self.Time = zeros (self.points)
            self.Tap1 = zeros (self.points)
            self.Tap2 = zeros (self.points)
            self.Tap3 = zeros (self.points)
            self.Tap4 = zeros (self.points)
            
            self.t0 = parent.mk2rte.get_monitor_data (self.monitor_taps[0])
    
            scope = Oscilloscope( Gtk.Label(), v, "XT", label)
            scope.set_scale ( { Xsignal[SIG_UNIT]: "mV", Ysignal[SIG_UNIT]: "deg", "time" : "s" })
            scope.set_chinfo(["MON1","MON2","MON3","MON4"])
            #scope.set_info(["to select CH1..4 taps","select Signals via Signal Monitor for:"," t, CH1..4 as Mon" + str(self.monitor_taps)])
            scope.set_flash ("configure CH1..4 via Signal Monitor: t, CH1..4:=" + str(self.monitor_taps))
    
            win.add(v)
    
            table = Gtk.Table(n_rows=4, n_columns=2)
            v.pack_start (table, True, True, 0)
    
            tr=0
            c=0
            lab = Gtk.Label( label="CH1: %s"%Xsignal[SIG_NAME] + ", Scale in %s/div"%Xsignal[SIG_UNIT])
                
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.M1scale = Gtk.Entry()
            self.M1scale.set_text("1.0")
            table.attach(self.M1scale, c, c+1, tr, tr+1)
            tr=tr+1
            
            lab = Gtk.Label( label="CH2: %s"%Ysignal[SIG_NAME] + ", Scale in %s/div"%Ysignal[SIG_UNIT])
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.M2scale = Gtk.Entry()
            self.M2scale.set_text("1.0")
            table.attach(self.M2scale, c, c+1, tr, tr+1)
            tr=tr+1
    
            lab = Gtk.Label( label="CH3: %s"%Usignal[SIG_NAME] + ", Scale in %s/div"%Usignal[SIG_UNIT])
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.M3scale = Gtk.Entry()
            self.M3scale.set_text("1.0")
            table.attach(self.M3scale, c, c+1, tr, tr+1)
            tr=tr+1
            
            lab = Gtk.Label( label="CH4: %s"%Vsignal[SIG_NAME] + ", Scale in %s/div"%Vsignal[SIG_UNIT])
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.M4scale = Gtk.Entry()
            self.M4scale.set_text("1.0")
            table.attach(self.M4scale, c, c+1, tr, tr+1)
            tr=tr+1
    
            tr = 0
            c = 1
            self.labx0 = Gtk.Label( label="Offset in %s/div"%Xsignal[SIG_UNIT])
            table.attach(self.labx0, c, c+1, tr, tr+1)
            tr=tr+1
            self.M1off = Gtk.Entry()
            self.M1off.set_text("0")
            table.attach(self.M1off, c, c+1, tr, tr+1)
            tr=tr+1
                
            self.laby0 = Gtk.Label( label="Offset in %s/div"%Ysignal[SIG_UNIT])
            table.attach(self.laby0, c, c+1, tr, tr+1)
            tr=tr+1
            self.M2off = Gtk.Entry()
            self.M2off.set_text("0")
            table.attach(self.M2off, c, c+1, tr, tr+1)
            tr=tr+1
    
            self.laby0 = Gtk.Label( label="Offset in %s/div"%Usignal[SIG_UNIT])
            table.attach(self.laby0, c, c+1, tr, tr+1)
            tr=tr+1
            self.M3off = Gtk.Entry()
            self.M3off.set_text("0")
            table.attach(self.M3off, c, c+1, tr, tr+1)
            tr=tr+1
    
            self.laby0 = Gtk.Label( label="Offset in %s/div"%Vsignal[SIG_UNIT])
            table.attach(self.laby0, c, c+1, tr, tr+1)
            tr=tr+1
            self.M4off = Gtk.Entry()
            self.M4off.set_text("0")
            table.attach(self.M4off, c, c+1, tr, tr+1)
            tr=tr+1
            
            c=0
            lab = Gtk.Label( label="Page length in s \n per "+str(self.points)+" samples")
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.Il = Gtk.Entry()
            self.Il.set_text("%d"%self.span)
            table.attach(self.Il, c, c+1, tr, tr+1)
                    
            self.xdc = 0.
            self.ydc = 0.
                
            def update_plotter():
    
                if self.pos >= self.points:
                    #self.run = False
                    #self.run_button.set_label("RESTART")
                    self.pos = 0
                    
                    save ("plotter_t-"+str(self.t0), self.Time)
                    save ("plotter_t1-"+str(self.t0), self.Tap1)
                    save ("plotter_t2-"+str(self.t0), self.Tap2)
                    save ("plotter_t3-"+str(self.t0), self.Tap3)
                    save ("plotter_t4-"+str(self.t0), self.Tap4)
                    scope.set_flash ("Saved: plotter_t#-"+str(self.t0))
                    # auto loop
                    self.t0 = parent.mk2rte.get_monitor_data (self.monitor_taps[0])
                        
                n = self.pos
                self.pos = self.pos+1
                        
                t = parent.mk2rte.get_monitor_data (self.monitor_taps[0])
                t = (t-self.t0) / 150000.
                self.Time[n] = t
                
                [value, uv1, signal1] = parent.mk2rte.get_monitor_signal (self.monitor_taps[1])
                self.Tap1[n] = uv1
                [value, uv2, signal2] = parent.mk2rte.get_monitor_signal (self.monitor_taps[2])
                self.Tap2[n] = uv2
                [value, uv3, signal3] = parent.mk2rte.get_monitor_signal (self.monitor_taps[3])
                self.Tap3[n] = uv3
                [value, uv4, signal4] = parent.mk2rte.get_monitor_signal (self.monitor_taps[4])
                self.Tap4[n] = uv4
        
                if n == 0:
                    print ("plotter data signals:")
                    print (signal1, signal2, signal3)
                    scope.set_chinfo([signal1[SIG_NAME],signal2[SIG_NAME],signal3[SIG_NAME],signal4[SIG_NAME]])
        
                print (n, t, uv1, uv2, uv3, uv4)
                
                try:
                    m1scale_div = float(self.M1scale.get_text())
                except ValueError:
                    m1scale_div = 1
                        
                try:
                    m2scale_div = float(self.M2scale.get_text())
                except ValueError:
                    m2scale_div = 1
        
                try:
                    m3scale_div = float(self.M3scale.get_text())
                except ValueError:
                    m3scale_div = 1
        
                try:
                    m4scale_div = float(self.M4scale.get_text())
                except ValueError:
                    m4scale_div = 1
        
                try:
                    m1off = float(self.M1off.get_text())
                except ValueError:
                    m1off = 0
                        
                try:
                    m2off = float(self.M2off.get_text())
                except ValueError:
                    m2off = 0
                        
                try:
                    m3off = float(self.M3off.get_text())
                except ValueError:
                    m3off = 0
                        
                try:
                    m4off = float(self.M4off.get_text())
                except ValueError:
                    m4off = 0
                        
                scope.set_scale ( { 
                    "XY1: (Tap1)":"%g mV"%m1scale_div, 
                    "XY2: (Tap2)":"%g mV"%m2scale_div, 
                    "time: ": "%g s/div"%(self.span/20.)
                })
        
                ### def set_data (self, Xd, Yd, Zd=zeros(0), XYd=[zeros(0), zeros(0)]):
        
                #  if t > self.span:
                #    td = 20. * (self.Time-t+self.span)/self.span
                # else:
                td = -10. + 20. * self.Time/self.span
                t1 = -(self.Tap1 - m1off) / m1scale_div
                t2 = -(self.Tap2 - m2off) / m2scale_div 
                t3 = -(self.Tap3 - m3off) / m3scale_div 
                t4 = -(self.Tap4 - m4off) / m4scale_div 
                #scope.set_data (zeros(0), zeros(0), zeros(0), XYd=[td, t1])
                #scope.set_data (t1, t2, zeros(0), XYd=[td, t1])
                scope.set_data_with_uv (t1, t2, t3, t4)
        
                return self.run
    
            def stop_update_plotter (win, event=None):
                print ("STOP, hide.")
                win.hide()
                self.run = False
                save ("plotter_t-"+str(self.t0), self.Time)
                save ("plotter_t1-"+str(self.t0), self.Tap1)
                save ("plotter_t2-"+str(self.t0), self.Tap2)
                save ("plotter_t3-"+str(self.t0), self.Tap3)
                save ("plotter_t4-"+str(self.t0), self.Tap4)
                scope.set_flash ("Saved: plotter_t#-"+str(self.t0))
                return True
    
            def toggle_run_plotter (b):
                if self.run:
                    self.run = False
                    self.run_button.set_label("RUN PLOTTER")
                    save ("plotter_t-"+str(self.t0), self.Time)
                    save ("plotter_t1-"+str(self.t0), self.Tap1)
                    save ("plotter_t2-"+str(self.t0), self.Tap2)
                    save ("plotter_t3-"+str(self.t0), self.Tap3)
                    save ("plotter_t4-"+str(self.t0), self.Tap4)
                    scope.set_flash ("Saved: plotter_t#-"+str(self.t0))
                else:
                    self.run = True
                    self.run_button.set_label("STOP PLOTTER")
                    self.t0 = parent.mk2rte.get_monitor_data (self.monitor_taps[0])
                    self.pos      = 0
                    self.span = float (self.Il.get_text())
                    self.dt   = self.span/self.points
                    self.Time = zeros (self.points)
                    self.Tap1 = zeros (self.points)
                    self.Tap2 = zeros (self.points)
                    self.Tap3 = zeros (self.points)
                    self.Tap4 = zeros (self.points)
                    GLib.timeout_add (int (self.dt*1000.), update_plotter)
    
            self.run_button = Gtk.Button("STOP TUNE")
            self.run_button.connect("clicked", toggle_run_plotter)
            table.attach(self.run_button, 1, 2, tr, tr+1)
    
            self.run = False
            win.connect("delete_event", stop_update_plotter)
            toggle_run_plotter (self.run_button)
            
            parent.wins[name].show_all()



class RecorderDeci():
    # X: 7=time, plotting Monitor Taps 20,21,0,1
    def __init__(self, parent):
        label = "Decimating Contineous Recorder"
        name  = "Deci Recorder"

        ### connect to influxdb, check db
        ### influxdb_client.drop_database('seismo') ### PURGE ###
        dblist=influxdb_client.get_list_database()
        create_db=True
        for db in dblist:
            print (db)
            if db['name'] == 'seismo':
                create_db=False
                break
            #[{'name': 'telegraf'}, {'name': '_internal'}, {'name': 'pyexample'}]
        if create_db:
            influxdb_client.create_database('seismo')
        influxdb_client.switch_database('seismo')
        ###
 
        if name not in wins:
            win = Gtk.Window()
            parent.wins[name] = win
            v = Gtk.HBox()
    
            #[Xsignal( Xdata, OffsetX] = parent.mk2rte.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL1_INPUT_ID)
            #[Ysignal, Ydata, OffsetY] = parent.mk2rte.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL2_INPUT_ID)
    
            scope = Oscilloscope( Gtk.Label(), v, "XT", label)
            scope.scope.set_subsample_factor (256)
            scope.scope.set_wide (True)
            scope.show()
            scope.set_chinfo(["CH1 AccZ", "CH2 VelZ", "CH2 AccX", "CH4 VelX"])
            #scope.set_chinfo([Xsignal[SIG_NAME], Ysignal[SIG_NAME]])
            #scope.set_scale ( { signalV[SIG_UNIT]: "V", "Temp": "K" })
    
            win.add(v)
    
            table = Gtk.Table(n_rows=8, n_columns=2)
            v.pack_start (table, True, True, 0)
            tr=0
            c=0

            lab = Gtk.Label( label="CH1: AccZ on IN0, Scale in 1mg/div")

            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.M1scale = Gtk.Entry()
            self.M1scale.set_text("0.1")
            table.attach(self.M1scale, c, c+1, tr, tr+1)
            tr=tr+1
            
            lab = Gtk.Label( label="CH2: VelZ, Scale in 1um/s/div")
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.M2scale = Gtk.Entry()
            self.M2scale.set_text("0.01")
            table.attach(self.M2scale, c, c+1, tr, tr+1)
            tr=tr+1
            
            lab = Gtk.Label( label="CH3: AccX on IN1, Scale in 1mg/div")
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.M3scale = Gtk.Entry()
            self.M3scale.set_text("0.1")
            table.attach(self.M3scale, c, c+1, tr, tr+1)
            tr=tr+1
            
            lab = Gtk.Label( label="CH4: VelX, Scale in 1um/s/div")
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.M4scale = Gtk.Entry()
            self.M4scale.set_text("0.01")
            table.attach(self.M4scale, c, c+1, tr, tr+1)
            tr=tr+1
            
            lab = Gtk.Label( label="Threshold Always")
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.T1 = Gtk.Entry()
            self.T1.set_text("0.2")
            table.attach(self.T1, c, c+1, tr, tr+1)
            tr=tr+1
            
            lab = Gtk.Label( label="Auto Threshold Factor")
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.TA = Gtk.Entry()
            self.TA.set_text("1.5")
            table.attach(self.TA, c, c+1, tr, tr+1)
            tr=tr+1
    
            self.lastevent = parent.mk2rte.read_recorder_deci (4096, 0, "", True)
            self.lasteventvel = self.lastevent
            self.logfile = '' #'mk2rte_S0_py_recorder_deci256.log'
            self.logcount = 5
            self.count = 100


            self.maxlist  = ones(256)*0.1
            self.maxlist1 = ones(256)*0.1
            self.thauto  = 0.2
            self.thauto1 = 0.2
            self.thi = 0
            self.thi1 = 0

            self.event_tics = 5*60

            self.t_last=[datetime.datetime.utcnow(), datetime.datetime.utcnow()]
            self.t_last_ft=[datetime.datetime.utcnow(), datetime.datetime.utcnow()]

            
            def update_recorder():
                try:
                    m1scale_div = float(self.M1scale.get_text())
                except ValueError:
                    m1scale_div = 1.0
                    
                try:
                    m2scale_div = float(self.M2scale.get_text())
                except ValueError:
                    m2scale_div = 1.0

                try:
                    m3scale_div = float(self.M3scale.get_text())
                except ValueError:
                    m3scale_div = 1.0

                try:
                    m4scale_div = float(self.M4scale.get_text())
                except ValueError:
                    m4scale_div = 1.0

                try:
                    m1th = float(self.T1.get_text())
                except ValueError:
                    m1th = 0.0

                try:
                    m1tha = float(self.TA.get_text())
                except ValueError:
                    m1tha = 1.1

    
                try:
                    m1th = float(self.T1.get_text())
                except ValueError:
                    m1th = 0.0

                dec_norm = 1.0
                dscale = 10./(dec_norm*32768.) 
                    
                rec  = dscale*parent.mk2rte.read_recorder_deci (4096, 0, self.logfile)
                rec1 = dscale*parent.mk2rte.read_recorder_deci (4096, 1)

                if rec.size != 4096:
                    print ("rec buffer size error. size=", rec.size)
                    return self.run
                        
                acc  = rec ## in mg  ------  [1000 V/g] 1V = 1mg ** MATCH PREAMPLIFIER SETTING
                acc1 = rec1
                if abs(acc.max()) < 9.0:
                    dt = 256.0/150000
                    vel = vel_integrate_acc (acc, dt)
                    vel1 = vel_integrate_acc (acc1, dt)
                else:
                    print ("Data error. ", abs(acc.max()) , abs(acc1.max()) ," Signal overload detected. Skipping.")
                    return self.run ## skip, data error
                
                scope.set_data (rec/m1scale_div, vel/m2scale_div, rec1/m3scale_div, vel1/m4scale_div)
    
                if m1th >= 0.0:
                    ma = acc[-512:].max()
                    mi = acc[-512:].min()
                    ma1 = acc1[-512:].max()
                    mi1 = acc1[-512:].min()
                    peak = max(abs(ma), abs(mi))
                    peak1 = max(abs(ma1), abs(mi1))
                    
                    t=datetime.datetime.utcnow()
                    scope.set_info(["max: "+str(ma), "min: "+str(mi), "thauto: "+str(self.thauto), "thauto1: "+str(self.thauto1)])
                    
                    if peak < 9.0 and peak1 < 9.0:
                        self.maxlist[self.thi]=peak
                        self.thi = (self.thi+1)%256
                        self.thauto = median(self.maxlist)*m1tha

                        self.maxlist1[self.thi1]=peak1
                        self.thi1 = (self.thi1+1)%256
                        self.thauto1 = median(self.maxlist1)*m1tha
                    
                        if peak >= m1th or peak >= self.thauto or peak1 >= m1th or peak1 >= self.thauto1 or self.logcount > 0:
                            #print(self.maxlist)
                            #print(sort(self.maxlist))
                            print (t, "### Auto Threashold " + str(self.thauto) + ", ",  str(self.thauto1) + " Triggered at: " + str(t) + " max: "+str(ma) + " min: "+str(mi) + " #" + str(self.logcount)+"\n")
                            if self.logcount > 0:
                                n=1024
                            else:
                                n=4096
                            #print ('pushing to influxdb...')
                            #print ('CH0-Z', acc.size, acc, vel.size, vel)
                            self.log_event_data (0, t, ma, mi, vel.max(), acc[-n:], vel[-n:], dt, self.thauto, 2)
                            self.log_rft_data (0, t, dt, acc, vel)
                            #print ('CH1-X', acc1.size, acc1, vel1.size, vel1)
                            self.log_event_data (1, t, ma1, mi1, vel1.max(), acc1[-n:], vel1[-n:], dt, self.thauto1, 2)
                            self.log_rft_data (1, t, dt, acc1, vel1)
                            #print ('done.')
                        else:
                            self.event_tics = self.event_tics-1
                            if self.event_tics < 1:
                                self.event_tics = 5*60
                                print ('pushing time stamp to influxdb...')
                                self.log_event_only (0, t, peak, mi, vel.max(), self.thauto)
                                self.log_event_only (1, t, peak1, mi1, vel1.max(), self.thauto1)
                                print ('done.')
                                
                        if peak > m1th or peak >= self.thauto or peak1 >= m1th or peak1 >= self.thauto1:
                            if 0:
                                self.logfile = 'mk3_S0_py_recorder_deci256.log'
                                if m1th > 0.0 and self.logcount == 0:
                                    with open(self.logfile, "a") as recorder:
                                            recorder.write("### Threashold Triggered at: " + str(t) + " max: "+str(ma) + " min: "+str(mi) + "\n")
                                            self.count = 0
                                            self.lastevent = acc
                                            self.lasteventvel = vel
                            else:
                                self.logfile = ''
                                if m1th > 0.0 and self.logcount == 0:
                                    self.lastevent = acc
                                    self.lasteventvel = vel
                                    self.count = 0
                            self.logcount = 25*5
                            self.count = self.count + 1
                            if self.count == 24:
                                    self.lastevent = rec
                            scope.set_flash ("Threashold Detected, Recording... " + str(self.count))
                            print ('Threashold Detected, Recording... ' + str(self.count))
                        else:
                            if self.logcount > 0:
                                    self.logcount = self.logcount - 1
                                    scope.set_flash ("Threashold Detected, Recording... " + str(self.logcount))
                                    print ('Threashold Detected, Recording... ' + str(self.count))
                                    #scope.set_data_with_uv (rec/m1scale_div, vel/m2scale_div, zeros(0), self.lastevent/m3scale_div, self.lasteventvel/m4scale_div)
                            else:
                                    if self.logfile != '':
                                            self.logfile = ''
                                            #scope.set_data_with_uv (rec/m1scale_div, vel/m2scale_div, zeros(0), zeros(0), zeros(0))
                return self.run

            def vel_integrate_acc (acc, dt):
                vel = zeros(acc.size)  ## in mm/s
                vint=0.0
                om = sum(acc, axis=0)/acc.size  # floating actual zero or center value
                i=0
                for a in acc:
                    om = om*0.99+0.01*a ## low pass
                    vint = vint + 9.807*(a-om)*dt
                    vel[i] = vint
                    i=i+1
                return vel

            def stop_recorder (win, event=None):
                print (("STOP, hide."))
                win.hide()
                self.run = False
                return True
    
            def toggle_run_recorder (b):
                if self.run:
                    self.run = False
                    self.run_button.set_label("RUN REC")
                    scope.set_flash ("Stopped.")
                else:
                    self.run = True
                    self.run_button.set_label("STOP REC")
                    scope.set_flash ("Starting...")
                    GLib.timeout_add (500, update_recorder)
    
            self.run_button = Gtk.Button(label="STOP REC")
            self.run_button.connect("clicked", toggle_run_recorder)
            table.attach(self.run_button, 0, 1, tr, tr+1)
    
            self.run = False
            win.connect("delete_event", stop_recorder)
            toggle_run_recorder (self.run_button)
            
        parent.wins[name].show_all()

    def get_id (self, ch):
        if ch == 0:
            return "Z-1L39"
        elif ch == 1:
            return "X-1L39"
        else:
            return "Z-10264"
        
    def log_event_only (self, ch, t, max, min, velmax, thauto):
        json_body = [
            {
                "measurement": "event",
                "tags": {
                    "user": "percy",
                    "accId": self.get_id (ch)
                },
                "time": str(t),
                "fields": {
                    "max": max,
                    "min": min,
                    "velmax": velmax,
                    "thauto": thauto
                }
            }
        ]
        influxdb_client.write_points(json_body)
                
    def log_event_data (self, ch, t, max, min, velmax, acc, vel, dt, thauto, decimation=16):
        json_body = [
            {
                "measurement": "event",
                "tags": {
                    "user": "percy",
                    "accId": self.get_id (ch)
                },
                "time": str(t),
                "fields": {
                    "max": max,
                    "min": min,
                    "velmax": velmax,
                    "thauto": thauto
                }
            }
        ]
        t = t - datetime.timedelta(seconds=dt*acc.size)
        deci=0
        for a,v in zip(acc, vel):
            t = t + datetime.timedelta(seconds=dt)
            if deci==0:
                am = a
                vm = abs(v)
            deci=deci+1
                        
            if abs(a) < 10.0:
                am = am*0.8+0.2*a
            if abs(v) < 10.0:
                vm = vm*0.8+0.2*abs(v)
                            
            if deci >= decimation and self.t_last[ch] < t:
                deci = 0
                json_body.append({
                    "measurement": "data",
                    "tags": {
                        "user": "percy",
                        "accId": self.get_id (ch)
                    },
                    "time": str(t),
                    "fields": {
                        "acc": am,
                        "vel": vm
                    }
                })
        influxdb_client.write_points(json_body)
        self.t_last[ch] = t

    def log_rft_data (self, ch, t, dt, acc, vel):
        if t < self.t_last_ft[ch]+datetime.timedelta(seconds=4096*dt):
            return
        self.t_last_ft[ch]=t
        HW = hanning(acc.size)
        acc_rft = abs(fft.rfft(HW*acc))/acc.size
        acc_rft = acc_rft[1:int(acc.size/2)+1]
        vel_rft = abs(fft.rfft(HW*vel))/vel.size
        vel_rft = vel_rft[1:int(vel.size/2)+1]

        N  = acc.size/2
        freqs = arange(0,N)/N/dt/2
                
        json_body = []
        i1=0
        iN=1
        NN=128
        NP=int(N/2)
        lb=1.025
        iNN=lb**NN
        for i in range(0,NN):
            i2=int(NP*(iN-1)/iNN)
            json_body.append({
                "measurement": "rft_data",
                "tags": {
                    "user": "percy",
                    "accId": self.get_id (ch),
                    "frq": '{:05.1f}'.format(freqs[int((i1+i2)/2)])
                },
                "time": str(t-datetime.timedelta(seconds=2048*dt)),
                "fields": {
                    "mag_acc": acc_rft[i1:i2+1].max(),
                    "mag_vel": vel_rft[i1:i2+1].max()
                }
            })
            iN=iN*lb
            i1=i2
        influxdb_client.write_points(json_body)

    
class DiodeTPlotter():
    # X: 7=time, plotting Monitor Taps 20,21,0,1
    def __init__(self, parent, length = 3600., taps=[7,1], samplesperpage=3600):
        self.monitor_taps = taps
    
        [value, uv, signalV] = parent.mk2rte.get_monitor_signal (self.monitor_taps[1])
    
        label = "Diode Temp Plotter -- Monitor Indexes 1 (MIX IN 1)"
        name  = "Diode Temp Plotter"
    
        if name not in wins:
            win = Gtk.Window()
            parent.wins[name] = win
            v = Gtk.HBox()
    
            self.pos      = 0
            self.span     = length
            self.points   = samplesperpage
            self.dt       = self.span/self.points
            self.Time = zeros (self.points)
            self.Tap4 = zeros (self.points)
    
            self.t0 = parent.mk2rte.get_monitor_data (self.monitor_taps[0])
    
            scope = Oscilloscope( Gtk.Label(), v, "XT", label)
            scope.set_scale ( { signalV[SIG_UNIT]: "V", "Temp": "K" } )
            scope.set_chinfo(["TEMP","MONV"])
            scope.set_flash ("configure CH4 via Signal Monitor: t, CH4:=" + str(self.monitor_taps[1]))
    
            win.add(v)
    
            table = Gtk.Table(n_rows=4, n_columns=2)
            v.pack_start (table, True, True, 0)
    
            tr=0
            c=0
    
            lab = Gtk.Label( label="CH1: Diode T in K")
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.M3scale = Gtk.Entry()
            self.M3scale.set_text("1.0")
            table.attach(self.M3scale, c, c+1, tr, tr+1)
            tr=tr+1
            
            lab = Gtk.Label( label="CH2: %s"%signalV[SIG_NAME] + ", Scale in %s/div"%signalV[SIG_UNIT])
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.M4scale = Gtk.Entry()
            self.M4scale.set_text("0.1")
            table.attach(self.M4scale, c, c+1, tr, tr+1)
            tr=tr+1
    
            tr = 0
            c = 1
            self.laby0 = Gtk.Label( label="Offset in K")
            table.attach(self.laby0, c, c+1, tr, tr+1)
            tr=tr+1
            self.M3off = Gtk.Entry()
            self.M3off.set_text("0")
            table.attach(self.M3off, c, c+1, tr, tr+1)
            tr=tr+1
    
            self.laby0 = Gtk.Label( label="Offset in V, 1.40K = 1.69812V")
            table.attach(self.laby0, c, c+1, tr, tr+1)
            tr=tr+1
            self.M4off = Gtk.Entry()
            self.M4off.set_text("1.69812")
            table.attach(self.M4off, c, c+1, tr, tr+1)
            tr=tr+1
            
            c=0
            lab = Gtk.Label( label="Page length in s \n per "+str(self.points)+" samples")
            table.attach(lab, c, c+1, tr, tr+1)
            tr=tr+1
            self.Il = Gtk.Entry()
            self.Il.set_text("%d"%self.span)
            table.attach(self.Il, c, c+1, tr, tr+1)
    
            self.xdc = 0.
            self.ydc = 0.
    
            def update_plotter():
    
                if self.pos >= self.points:
                    #self.run = False
                    #self.run_button.set_label("RESTART")
                    self.pos = 0
                                  
                    save ("plotter_dt-"+str(self.t0), self.Time)
                    save ("plotter_dv-"+str(self.t0), self.TapV)
                    save ("plotter_dk-"+str(self.t0), self.TapK)
                    scope.set_flash ("Saved: plotter_t#-"+str(self.t0))
                    # auto loop
                    self.t0 = parent.mk2rte.get_monitor_data (self.monitor_taps[0])
                    
                n = self.pos
                self.pos = self.pos+1
    
                t = parent.mk2rte.get_monitor_data (self.monitor_taps[0])
                t = (t-self.t0) / 150000.
                self.Time[n] = t
                
                [value, uv, signalV] = parent.mk2rte.get_monitor_signal_median (self.monitor_taps[1])
                self.TapV[n] = uv
                self.TapK[n] = v2k(uv)
    
                if n == 0:
                    print ("plotter data signals:")
                    print (signalV)
                    scope.set_chinfo(["Diode Temp in K", "Diode Volts"])
    
                print (n, t, uv, v2k(uv))
                
                try:
                    m3scale_div = float(self.M3scale.get_text())
                except ValueError:
                    m3scale_div = 1
    
                try:
                    m4scale_div = float(self.M4scale.get_text())
                except ValueError:
                    m4scale_div = 1
    
                try:
                    m3off = float(self.M3off.get_text())
                except ValueError:
                    m3off = 0
                    
                try:
                    m4off = float(self.M4off.get_text())
                except ValueError:
                    m4off = 0
                    
                scope.set_scale ( { 
                        "Temp"   : "[ {:.3f} K ]  {:g} K".format(v2k(uv),m3scale_div), 
                        "DVolt:" : "[ {:.4f} V ]  {:g}".format(uv,m4scale_div), 
                        "time: " : "{:g} s".format(self.span/20.)
                        })
    
                ### def set_data (self, Xd, Yd, Zd=zeros(0), XYd=[zeros(0), zeros(0)]):
    
                #  if t > self.span:
                #    td = 20. * (self.Time-t+self.span)/self.span
                # else:
                td = -10. + 20. * self.Time/self.span
                tk = -(self.TapK - m3off) / m3scale_div 
                tv = -(self.TapV - m4off) / m4scale_div 
                #scope.set_data (zeros(0), zeros(0), zeros(0), XYd=[td, t1])
                #scope.set_data (t1, t2, zeros(0), XYd=[td, t1])
                scope.set_data_with_uv (tk, tv, zeros(0), zeros(0))
    
                return self.run
    
            def stop_update_plotter (win, event=None):
                print ("STOP, hide.")
                win.hide()
                self.run = False
                save ("plotter_dt-"+str(self.t0), self.Time)
                save ("plotter_dv-"+str(self.t0), self.TapV)
                save ("plotter_dk-"+str(self.t0), self.TapK)
                scope.set_flash ("Saved: plotter_dt#-"+str(self.t0))
                return True
    
            def toggle_run_plotter (b):
                if self.run:
                    self.run = False
                    self.run_button.set_label("RUN PLOTTER")
                    save ("plotter_dt-"+str(self.t0), self.Time)
                    save ("plotter_dv-"+str(self.t0), self.TapV)
                    save ("plotter_dk-"+str(self.t0), self.TapK)
                    scope.set_flash ("Saved: plotter_dt#-"+str(self.t0))
                else:
                    self.run = True
                    self.run_button.set_label("STOP PLOTTER")
                    self.t0 = parent.mk2rte.get_monitor_data (self.monitor_taps[0])
                    self.pos      = 0
                    self.span = float (self.Il.get_text())
                    self.dt   = self.span/self.points
                    self.Time = zeros (self.points)
                    [value, uv, signalV] = parent.mk2rte.get_monitor_signal (self.monitor_taps[1])
                    self.TapV = zeros (self.points)+uv
                    self.TapK = zeros (self.points)+v2k(uv)
                    GLib.timeout_add (int (self.dt*1000.), update_plotter)
    
            self.run_button = Gtk.Button("STOP TUNE")
            self.run_button.connect("clicked", toggle_run_plotter)
            table.attach(self.run_button, 1, 2, tr, tr+1)
    
            self.run = False
            win.connect("delete_event", stop_update_plotter)
            toggle_run_plotter (self.run_button)
            
        parent.wins[name].show_all()



################################################################################
# CONFIGURATOR MAIN MENU/WINDOW
################################################################################

class Mk2_Configurator:
    def __init__(self):
        self.mk2rte = RTEcontrol (0)
        self.vector_index = 0
    
        if self.mk2rte.status () == 0:
            buttons = {
                "No MK2 found": self.dummy
            }
        else :
            #self.mk2rte.read_configurations ()
    
            buttons = {
            "0 A810 AD/DA Monitor": self.create_A810_ADDA_monitor,
            "1 A810 Wave Generator": self.wave_generator,
            #"2 SPM Signal Monitor": self.create_signal_monitor,
            #"3 SPM Signal Patch Rack": self.create_signal_patch_rack,
            "4 SPM Signal+DSP Manager": self.create_signal_manager,
            #"5 SPM Create Signal Graph": self.create_dotviz_graph,
            "6 SPM Signal Oscilloscope": self.create_oscilloscope_app,
            "6pSPM Signal Plotter": self.create_signalplotter_app,
            "6kSPM Diode Temp Plotter": self.create_diodetplotter_app,
            "6z Deci Recorder App": self.create_recorder_app,
            "7 MK2 Settings App": self.create_settings_app,
            #"7 PLL: Osc. Tune App": self.create_PLL_tune_app,
            #"8 PLL: Ampl. Step App": self.create_amp_step_app,
            #"9 PLL: Phase Step App": self.create_phase_step_app,
            ##           "GPIO Control": create_coolrunner_app,
            ##           "Rate Meter": create_ratemeter_app,
            ##           "FPGA Slider Control": create_coolrunner_slider_control_app,
            ##           "FPGA LVDT Stage Control": create_coolrunner_lvdt_stage_control_app,
            ##           "SR MK3 DSP Info": create_info,
            ##           "SR MK3 DSP SPM Settings": create_settings_edit,
            ##           "SR MK3 DSP Reset Peak Load": reset_dsp_load_peak,
            #           "SR MK3 read HR mask": self.mk2rte.read_hr,
            #           "SR MK3 set HR0": self.mk2rte.set_hr_0,
            #           "SR MK3 set HR1": self.mk2rte.set_hr_1,
            #           "SR MK3 set HR1slow": self.mk2rte.set_hr_1slow,
            #           "SR MK3 set HR1slow2": self.mk2rte.set_hr_1slow2,
            #           "SR MK3 set HRs2": self.mk2rte.set_hr_s2,
            #            "SR MK3 HARD RESET": self.mk2rte.issue_mk3_hard_reset,
            #"SR MK3 read GPIO settings": self.mk2rte.read_gpio,
            #"SR MK3 write GPIO settings": self.mk2rte.write_gpio,
            #"SR MK3 CLR DSP GP53,54,55": self.mk2rte.clr_dsp_gpio,
            #"SR MK3 SET DSP GP53,54,55": self.mk2rte.set_dsp_gpio,
            #"SR TEST GPIO PULSE": self.mk2rte.execute_pulse_cb,
    
    ##           "SR MK3 read D-FIFO128": readFIFO128,
            ##           "SR MK3 read D-FIFO-R": readFIFO,
            ##           "SR MK3 read PD-FIFO-R": readPFIFO,
               #"U SCO Config": self.sco_config,
            #"X Vector Index Selector": self.create_index_selector_matrix,
            #"Y User Values": self.create_user_values,
            #"Z SCAN DBG": self.print_(dbg,)
            }
        
        win = Gtk.Window()
        win.set_name("main window")
        #win.set_size_request(260, 540)
        win.connect("destroy", self.destroy)
        win.connect("delete_event", self.destroy)
    
        box1 = Gtk.VBox()
        win.add(box1)
        scrolled_window = Gtk.ScrolledWindow()
        scrolled_window.set_border_width(5)
        box1.pack_start(scrolled_window, True, True, 0)
        box2 = Gtk.VBox()
        box2.set_border_width(5)
        scrolled_window.add_with_viewport(box2)
    
        lab = Gtk.Label("SR dev:" + self.mk2rte.sr_path ())
        box2.pack_start(lab, True, True, 0)
        lab = Gtk.Label(self.mk2rte.get_spm_code_version_info())
        box2.pack_start(lab, True, True, 0)
    
        bk = buttons.keys()
        for appname in sorted(bk):
            print (appname)
            button = Gtk.Button(label=appname)
    
            if buttons[appname]:
                button.connect ("clicked", buttons[appname])
            else:
                button.set_sensitive (False)
            box2.pack_start (button, True, True, 0)
    
        separator = Gtk.HSeparator()
        box1.pack_start (separator, False, True, 0)
    
        box2 = Gtk.VBox()
        box1.pack_start (box2, False, True, 0)
        button = Gtk.Button (label='Close and Quit')
        button.connect ("clicked", self.do_exit)
        box2.pack_start (button, True, True, 0)
        win.show_all ()
        win.set_size_request(200,300)
    
        self.wins = {}

    # create SR DSP SPM settings dialog
    def create_settings_app(self, _button):
            def check_update(_set, _mask):
                if self.mk2rte.SPM_STATEMACHINE[2] & _mask:
                    _set (True)
                else:
                    _set (False)
                return 1

            def toggle_flag(w, mask):
                    sr = open (self.mk2rte.sr_dev_path, "wb")
                    if w.get_active():
                            os.lseek (sr.fileno(), self.mk2rte.magic[i_statemachine], 0)
                    else:
                            os.lseek (sr.fileno(), self.mk2rte.magic[i_statemachine]+1, 0)
                    os.write (sr.fileno(), struct.pack (fmt_statemachine_w, mask))
                    sr.close ()
                    
            def ext_FB_control_watch_value(value):
                    value = 0
                    sr = open (self.mk2rte.sr_dev_path, "wb")
                    os.lseek (sr.fileno(), self.mk2rte.magic[i_external]+2, 1)
                    os.write (sr.fileno(), struct.pack (fmt_external_w, int(value)))
                    sr.close ()
                    return 1
                    
            def ext_FB_control(_object, _value, _identifier):
                    value = 0
                    sr = open (self.mk2rte.sr_dev_path, "wb")
                    if _identifier=="ext_channel":
                        os.lseek (sr.fileno(), self.mk2rte.magic[i_external]+0, 1)
                        value = _object.get_value()
                    if _identifier=="ext_treshold":
                            os.lseek (sr.fileno(), self.mk2rte.magic[i_external]+1, 1)
                            value = _object.get_value()*3276.7
                    if _identifier=="ext_value":
                            os.lseek (sr.fileno(), self.mk2rte.magic[i_external]+2, 1)
                            value = _value
                    if _identifier=="ext_min":
                            os.lseek (sr.fileno(), self.mk2rte.magic[i_external]+3, 1)
                            value = _object.get_value()*16383
                    if _identifier=="ext_max":
                            os.lseek (sr.fileno(), self.mk2rte.magic[i_external]+4, 1)
                            value = _object.get_value()*16383
                    os.write (sr.fileno(), struct.pack (fmt_external_w, int(value)))
                    sr.close ()
                    return 1

            name = "SR DSP SPM Settings"
            if name not in wins:
                    win =  Gtk.Window()
                    wins[name] = win

                    box1 = Gtk.VBox()
                    win.add(box1)
                    box1.show()

                    box2 = Gtk.VBox()
                    box1.pack_start(box2, True, True, 0)
                    box2.show()

                    hbox = Gtk.HBox()
                    box2.pack_start(hbox, True, True, 0)
                    hbox.show()

                    lab =  Gtk.Label(label="* * * State Flags [state.mode] * * *\n -->  watching every %d ms" %updaterate)
                    hbox.pack_start(lab, True, True, 0)
                    lab.show()

                    separator = Gtk.HSeparator()
                    box2.pack_start(separator, True, True, 0)
                    separator.show()

                    hbox = Gtk.VBox()
                    box2.pack_start(hbox, True, True, 0)
                    hbox.show()

                    box2 = Gtk.VBox()
                    box1.pack_start(box2, True, True, 0)
                    box2.show()

                    box3 = Gtk.HBox()
                    hbox.pack_start(box3, True, True, 0)
                    box3.show()		

    #		adj = gtk.Adjustment(int(self.mk2rte.EXT_CONTROL[0]), 0, 7, 1, 1, 0)
                    adj = Gtk.Adjustment(int(self.mk2rte.EXT_CONTROL[0]), -32768, 32767, 1, 1, 0)
                    spinner = Gtk.SpinButton(adjustment=adj)
                    spinner.set_wrap(False)
                    spinner.set_numeric(True)
                    spinner.connect('value-changed', ext_FB_control, 0, "ext_channel")
                    spinner.show()

                    adj = Gtk.Adjustment(self.mk2rte.EXT_CONTROL[1]/3276.7, -9.9, 9.9, .1, 1, 0)
                    spinner2 = Gtk.SpinButton(adjustment=adj)
                    spinner2.set_digits(1)
                    spinner2.set_snap_to_ticks(True)
                    spinner2.set_wrap(False)
                    spinner2.set_numeric(True)
                    spinner2.connect('value-changed', ext_FB_control, 0, "ext_treshold")
                    spinner2.show()

                    label = Gtk.Label(label="is above")
                    label.show()

                    label2 = Gtk.Label(label="V")
                    label2.show()

                    check_button = Gtk.CheckButton("Feedback OFF if channel")
                    check_button.set_active(False)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0100)
                    check_button.connect('toggled', toggle_flag, 0x0100)
                    check_button.show()

                    box3.pack_start(check_button, True, True, 0)
                    box3.pack_start(spinner, True, True, 0)
                    box3.pack_start(label, True, True, 0)
                    box3.pack_start(spinner2, True, True, 0)
                    box3.pack_start(label2, True, True, 0)


    #		check_button = gtk.CheckButton("High Resolution Mode [0x0001]\n (Sigma Delta Style Resolution increase via bit0 for Z))")
    #		if self.mk2rte.SPM_STATEMACHINE[2] & 0x0001:
    #			check_button.set_active(True)
    #		check_button.connect('toggled', toggle_flag, 0x0001)
    #		GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0001)
    #		check_button.show()
    #		hbox.pack_start(check_button)

                    check_button = Gtk.CheckButton("BLK [0x0002]\n (DSP Heartbeat, RO))")
                    if self.mk2rte.SPM_STATEMACHINE[2] & 0x0001:
                            check_button.set_active(True)
    #		check_button.connect('toggled', toggle_flag, 0x0001)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0001)
                    check_button.show()
                    hbox.pack_start(check_button, True, True, 0)

                    check_button = Gtk.CheckButton("PID [0x0004]\n(Feedback Enable/Hold)")
                    if self.mk2rte.SPM_STATEMACHINE[2] & 0x0004:
                            check_button.set_active(True)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0004)
                    check_button.connect('toggled', toggle_flag, 0x0004)
                    check_button.show()
                    hbox.pack_start(check_button, True, True, 0)

                    check_button = Gtk.CheckButton("TEST A (normal OFF)  [0x0008]\n(disable vector rotation, using vec mul32)")
                    if self.mk2rte.SPM_STATEMACHINE[2] & 0x0008:
                            check_button.set_active(True)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0008)
                    check_button.connect('toggled', toggle_flag, 0x0008)
                    check_button.show()
                    hbox.pack_start(check_button, True, True, 0)

                    check_button = Gtk.CheckButton("Offset Compensation [0x0010]\n (obsolete for A810)")
                    if self.mk2rte.SPM_STATEMACHINE[2] & 0x0010:
                            check_button.set_active(True)
                    check_button.connect('toggled', toggle_flag, 0x0010)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0010)
                    check_button.show()
                    hbox.pack_start(check_button, True, True, 0)

                    check_button = Gtk.CheckButton("Z-Transformation [0x0020]\n (AIC[6] * (-1) -> AIC[5])")
                    if self.mk2rte.SPM_STATEMACHINE[2] & 0x0020:
                            check_button.set_active(True)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0020)
                    check_button.connect('toggled', toggle_flag, 0x0020)
                    check_button.show()
                    hbox.pack_start(check_button, True, True, 0)

                    check_button = Gtk.CheckButton("RANDOM NOISE GEN [0x0040] (add with AC_Amp on Bias!)\n")
                    if self.mk2rte.SPM_STATEMACHINE[2] & 0x0040:
                            check_button.set_active(True)
                    check_button.connect('toggled', toggle_flag, 0x0040)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0040)
                    hbox.pack_start(check_button, True, True, 0)
                    check_button.show()

                    check_button = Gtk.CheckButton("Offset Adding [0x0080]\n (enable offset adding to scan signals: AIC[0/1/2] not used)")
                    if self.mk2rte.SPM_STATEMACHINE[2] & 0x0080:
                            check_button.set_active(True)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0080)
                    check_button.connect('toggled', toggle_flag, 0x0080)
                    check_button.show()
                    hbox.pack_start(check_button, True, True, 0)

                    check_button = Gtk.CheckButton("TEST B (normal off) [0x0200]\n (disable Z0 solpe output/adding)")
                    if self.mk2rte.SPM_STATEMACHINE[2] & 0x0200:
                            check_button.set_active(True)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0200)
                    check_button.connect('toggled', toggle_flag, 0x0200)
                    check_button.show()
                    hbox.pack_start(check_button, True, True, 0)

                    check_button = Gtk.CheckButton("DIFF IN0-IN4 (normal off) [0x0800]\n (DSP differential input remap IN0-IN4 => IN0)")
                    if self.mk2rte.SPM_STATEMACHINE[2] & 0x0800:
                            check_button.set_active(True)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0800)
                    check_button.connect('toggled', toggle_flag, 0x0800)
                    check_button.show()
                    hbox.pack_start(check_button, True, True, 0)

                    check_button = Gtk.CheckButton("AIC Stop [0x2000]\n (Halt AICs -- stops data conversion and DMA transfer)")
                    if self.mk2rte.SPM_STATEMACHINE[2] & 0x2000:
                            check_button.set_active(True)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x2000)
                    check_button.connect('toggled', toggle_flag, 0x2000)
                    check_button.show()
                    hbox.pack_start(check_button, True, True, 0)

                    check_button = Gtk.CheckButton("Pass / Watch [0x0400]")
                    if self.mk2rte.SPM_STATEMACHINE[2] & 0x0400:
                            check_button.set_active(True)
                    GLib.timeout_add (updaterate, check_update, check_button.set_active, 0x0400)
                    check_button.connect('toggled', toggle_flag, 0x0400)
                    check_button.show()


                    extern_control_store = Gtk.ListStore(str, int)
                    extern_control_store.append(["Feedback", 0])
                    extern_control_store.append(["X Offset", 1])
                    extern_control_store.append(["log[AIC5]", 2])
                    extern_control_store.append(["FB delta", 3])
                    extern_control_store.append(["PRB Sine", 4])
                    extern_control_store.append(["PRB Phase", 5])
                    extern_control_store.append(["Bias Add(7)", 6])
                    extern_control_store.append(["EN_MOTOR Off[7]->M", 7])
                    extern_control_store.append(["PRB Phase->M", 8])
                    extern_control_store.append(["--", 9])
                    extern_control_store.append(["Ext Feeback Add: OUT7=Z_DSP+IN7",10])
                    extern_combo = Gtk.ComboBox.new_with_model (extern_control_store)
                    cell = Gtk.CellRendererText()
                    extern_combo.pack_start(cell, True)
                    extern_combo.add_attribute(cell, 'text', 0)

                    def on_ext_changed(widget):
                        print ('EXTERNAL MODE: ', widget.get_active())
                        ext_FB_control_watch_value(widget.get_active())
                        return
                    
                    extern_combo.connect("changed", on_ext_changed)
                    extern_combo.set_active(self.mk2rte.EXT_CONTROL[2])
                    extern_combo.show()
                            
                    label0 = Gtk.Label(label="to OUT7. Output voltage will be   ")
                    label0.show()

                    adj = Gtk.Adjustment(self.mk2rte.EXT_CONTROL[3]/16383., -2.0, 2.0, .1, 1, 0)
                    spinner = Gtk.SpinButton(adjustment=adj)
                    spinner.set_digits(1)
                    spinner.set_snap_to_ticks(True)
                    spinner.set_wrap(False)
                    spinner.set_numeric(True)
                    spinner.connect('value-changed', ext_FB_control, 0, "ext_min")
                    spinner.show()

                    label1 = Gtk.Label( label="   from ")
                    label1.show()

                    label2 = Gtk.Label( label="V (min/on)  to")
                    label2.show()

                    adj = Gtk.Adjustment(self.mk2rte.EXT_CONTROL[4]/16383., -2.0, 2.0, .1, 1, 0)
                    spinner2 = Gtk.SpinButton(adjustment=adj)
                    spinner2.set_digits(1)
                    spinner2.set_snap_to_ticks(True)
                    spinner2.set_wrap(False)
                    spinner2.set_numeric(True)
                    spinner2.connect('value-changed', ext_FB_control, 0, "ext_max")
                    spinner2.show()

                    label3 = Gtk.Label( label="V (max/off)      ")
                    label3.show()

                    box3 = Gtk.HBox()
                    hbox.pack_start(box3, True, True, 0)
                    box3.show()		
                    box3.pack_start(check_button, True, True, 0)
                    box3.pack_start(extern_combo, True, True, 0)
                    box3.pack_start(label0, True, True, 0)

                    box3 = Gtk.HBox()
                    hbox.pack_start(box3, True, True, 0)
                    box3.show()		
                    box3.pack_start(label1, True, True, 0)
                    box3.pack_start(spinner, True, True, 0)
                    box3.pack_start(label2, True, True, 0)
                    box3.pack_start(spinner2, True, True, 0)
                    box3.pack_start(label3, True, True, 0)

                    separator = Gtk.HSeparator()
                    box1.pack_start(separator, True, True, 0)
                    separator.show()

                    #status = Gtk.Label( label="---")
                    #GLib.timeout_add (updaterate, check_dsp_status, status.set_text)
                    #box1.pack_start(status, True, True, 0)
                    #status.show()

                    #status = Gtk.Label( label="---")
                    #GLib.timeout_add (updaterate, check_dsp_feedback, status.set_text)
                    #box1.pack_start(status, True, True, 0)
                    #status.show()

                    #separator = Gtk.HSeparator()
                    #box1.pack_start(separator, True, True, 0)
                    #separator.show()

                    #status = Gtk.Label( label="---")
                    #GLib.timeout_add (updaterate, check_dsp_scan, status.set_text)
                    #box1.pack_start(status)
                    #status.show()

                    #separator = Gtk.HSeparator()
                    #box1.pack_start(separator, True, True, 0)
                    #separator.show()

                    #box2 = Gtk.VBox(spacing=10)
                    #box2.set_border_width(10)
                    #box1.pack_start(box2, True, True, 0)
                    #box2.show()

                    button = Gtk.Button(stock='gtk-close')
                    button.connect("clicked", lambda w: win.hide())
                    box2.pack_start(button, True, True, 0)
                    button.show()
            wins[name].show()

    # create ratemeter dialog
    def create_ratemeter_app(self, _button):
            name = "SR DSP CR-RateMeter"
            if name not in wins:
                    win =  Gtk.Window()
                    wins[name] = win
                    win.connect("delete_event", delete_event)


                    box1 = gobject.new(gtk.VBox(spacing=5))
                    win.add(box1)
                    box1.show()

                    tr = 0

                    table = gtk.Table(4, 2)
                    table.set_row_spacings(5)
                    table.set_col_spacings(5)
                    box1.pack_start (table)
                    table.show()

                    lab = gobject.new(gtk.Label, label="Gatetime C1 [ms]:")
                    lab.show ()
                    table.attach(lab, 0, 1, tr, tr+1)

                    entry_GTm1 = gtk.Entry()
                    entry_GTm1.set_text("100")
                    table.attach(entry_GTm1, 1, 2, tr, tr+1)
                    entry_GTm1.show()

                    lab = gobject.new(gtk.Label, label="C2 [ms]:")
                    lab.show ()
                    table.attach(lab, 2, 3, tr, tr+1)
                    lab = gobject.new(gtk.Label, label="gate C2 max:")
                    lab.show ()
                    table.attach(lab, 2, 3, tr+1, tr+2)
                    lab = gobject.new(gtk.Label, label="873ms")
                    lab.show ()
                    table.attach(lab, 3, 4, tr+1, tr+2)

                    entry_GTm2 = gtk.Entry()
                    entry_GTm2.set_text("20")
                    table.attach(entry_GTm2, 3, 4, tr, tr+1)
                    entry_GTm2.show()

                    tr = tr+1

                    lab = gobject.new(gtk.Label, label="PeakHold [ms]:")
                    lab.show ()
                    table.attach(lab, 0, 1, tr, tr+1)

                    entry_holdTm = gtk.Entry()
                    entry_holdTm.set_text("0")
                    table.attach(entry_holdTm, 1, 2, tr, tr+1)
                    entry_holdTm.show()

                    tr = tr+1


                    check_button = gtk.CheckButton("Enable RateMeter")
                    if CR_COUNTER[13] and CR_COUNTER[0]:
                            check_button.set_active(True)
                    check_button.connect('toggled', enable_ratemeter, entry_GTm1.get_text, entry_GTm2.get_text, entry_holdTm.get_text)
                    box1.pack_start(check_button)
                    check_button.show()


                    separator = gobject.new(gtk.HSeparator())
                    box1.pack_start(separator, expand=False)
                    separator.show()

                    c1 = gobject.new(gtk.Label, label="-CNT1- ---")
                    c1.set_use_markup(True)
                    box1.pack_start(c1)
                    c1.show()

                    c2 = gobject.new(gtk.Label, label="-CNT2- ---")
                    c2.set_use_markup(True)
                    box1.pack_start(c2)
                    c2.show()

                    GLib.timeout_add (updaterate, check_dsp_ratemeter, c1.set_markup, c2.set_markup)

                    status = gobject.new(gtk.Label, label="-Status- ---")
                    GLib.timeout_add (updaterate, check_dsp_status, status.set_text)
                    box1.pack_start(status)
                    status.show()

                    separator = gobject.new(gtk.HSeparator())
                    box1.pack_start(separator, expand=False)
                    separator.show()

                    box2 = gobject.new(gtk.VBox(spacing=10))
                    box2.set_border_width(10)
                    box1.pack_start(box2, expand=False)
                    box2.show()

                    button = gtk.Button(stock='gtk-close')
                    button.connect("clicked", lambda w: win.hide())
                    box2.pack_start(button)
                    button.set_flags(gtk.CAN_DEFAULT)
                    button.grab_default()
                    button.show()
            wins[name].show()


        
    def create_index_selector_matrix(self, b):
        name="Index Selector Matrix"
        if name not in wins:
            win = Gtk.Window()
            self.wins[name] = win
            win.connect("delete_event", self.delete_event)

            box1 = Gtk.VBox()
            win.add(box1)

            box2 = Gtk.VBox()
            box1.pack_start(box2, True, True, 0)

            box1.pack_start (Gtk.Hseparator(), False, True, 0)

            # 8x8 matrix selector
            table = Gtk.Table(n_rows=8, n_columns=8)
            box1.pack_start(table, False, True, True, 0)
            
            for i in range (0,8):
                for j in range (0,8):
                    ij = 8*i+j
                    button = Gtk.Button ("%d"%ij)
                    button.connect ("clicked", self.do_set_index, ij)
                    table.attach (button, i, i+1, j, j+1)

            self.offsetindicator = Gtk.Label("Global Vector Index is [%d]"%self.vector_index)
            box1.pack_start(self.offsetindicator, False, True, 0)

        self.wins[name].show_all()

    def do_set_user_input(self, b, i, u_gettext):
        self.mk2rte.write_signal_by_name ("user signal array", float(u_gettext ()), i)
        
    def create_user_values(self, b):
        name="User Values Array"
        if name not in wins:
            win = Gtk.Window(
                      type=Gtk.WINDOW_TOPLEVEL,
                      title=name,
                      allow_grow=True,
                      allow_shrink=True,
                      border_width=5)
            self.wins[name] = win
            win.connect("delete_event", self.delete_event)

            box1 = Gtk.VBox()
            win.add(box1)

            box2 = Gtk.VBox()
            box2.set_border_width(5)
            box1.pack_start(box2, True, True, 0)

            box1.pack_start (Gtk.Hseparator(), False, True, 0)

            # 8x8 matrix selector
            table = Gtk.Table(n_rows=8, n_columns=8)
            table.set_row_spacings(1)
            table.set_col_spacings(1)
            box1.pack_start(table, False, True, 0)
            
            for j in range (0,2):
                for i in range (0,16):
                    ij = 16*j+i
                    ui = Gtk.Entry()
                    [u,uv, sig] = self.mk2rte.read_signal_by_name ("user signal array", ij)
                    ui.set_text("%f %s"%(uv, sig[SIG_UNIT]))
                    table.attach (ui, 2*j+1, 2*j+2, i, i+1)

                    button = Gtk.Button ("Set User Value [%d]"%ij)
                    button.connect ("clicked", self.do_set_user_input, ij, ui.get_text)
                    table.attach (button, 2*j+0, 2*j+1, i, i+1)

        self.wins[name].show_all()

    def print_dbg(self, b):
        def update ():
            self.mk2rte.check_dsp_scan ()
            return TRUE
        
        GLib.timeout_add(200, update)

    def do_set_index(self, b, ij):
        self.vector_index = ij
        self.offsetindicator.set_text("Global Vector Index is [%d]"%self.vector_index)

    def delete_event(self, win, event=None):
        win.hide()
        # don't destroy window -- just leave it hidden
        return True

    def do_exit (self, button):
        Gtk.main_quit()

    def destroy (self, *args):
        Gtk.main_quit()
    
    # update DSP setting parameter
    def int_param_update(self, _adj):
        param_address = _adj.get_data("int_param_address")
        param = int (_adj.get_data("int_param_multiplier")*_adj.value)
        self.mk2rte.write_parameter (param_address, struct.pack ("<l", param), 1) 

    def make_menu_item(self, signal_store, name, callback, value, identifier, func=lambda:0):
        print ("MK OPT MENU ITEM:", name, value, identifier)
        if (name != 'DISABLED'):
            signal_store.append([name, value[0]])
        else:
            signal_store.append([name, -1])
        return None

    def adjust_signal_callback(self, opt, SIG_LIST, _input_id):
        pos=opt.get_active()
        print ("SIGNAL POS:",pos)
        # prefix+"S%02d:"%signal[SIG_INDEX]+signal[SIG_NAME], self.mk3spm.change_signal_input, signal, _input_id, self.global_vector_index
        ## "DISABLED", self.mk3spm.disable_signal_input, 0, _input_id, self.global_vector_index)
        # --> connect("activate", callback, value, identifier, func)

        if pos>0:
            signal = SIG_LIST[pos-1]
            print ("Adjust Input", _input_id, " to signal:", signal)
            if signal[SIG_INDEX] >= 0:
                self.mk2rte.change_signal_input (0, signal, _input_id, self.global_vector_index)
            else:
                # "DISABLED"
                self.mk2rte.disable_signal_input (0, 0, _input_id, self.global_vector_index)
        else:
            print ("Can't do that.")

    def toggle_flag(self, w, mask):
        self.mk2rte.adjust_statemachine_flag(mask, w.get_active())


    ##### configurator tools
    def create_dotviz_graph (self, _button):
        vis = Visualize (self)
        
    def create_oscilloscope_app (self, _button):
        kao = SignalScope (self)

    def create_signalplotter_app (self, _button):
        ##### SignalPlotter __init__ (self, parent, length = 300., taps=[7,20,21,0,1], samplesperpage=2000):

        ## may customize defaults using this line:
        ## sigplotter = SignalPlotter (self, 300, [7,20,21,0,1], 2000)
        sigplotter = SignalPlotter (self)
    
    def create_recorder_app (self, _button):
        decirecorder = RecorderDeci (self)

    def create_diodetplotter_app (self, _button):
        dtplotter = DiodeTPlotter (self)
    
    def create_PLL_tune_app (self, _button):
        tune = TuneScope (self)

    def create_amp_step_app (self, _button):
        as_scope = StepResponse(self, 0.05, "Amp")

    def create_phase_step_app (self, _button):
        ph_scope = StepResponse(self, 1., "Phase")

    def scoset (self, button):
        print (self.s_offset.get_text())
        print (self.fsens.get_text())
        sc   = round (self.sco_s_offsetQ * float(self.s_offset.get_text()) )
        freq = float (self.fsens.get_text())
        fsv  = round (freq*self.sco_sensQ)    # CPN31 * 2pi freq / 150000.
        print (sc, fsv)
        scodata = self.mk2rte.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), fmt_sco1)
        if scodata[11] == 0:
            # (Sin - Sc) * fsv
            self.mk2rte.write_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), struct.pack("<ll", sc, fsv))
        else:
            CPN31 = 2147483647
            deltasRe = round (CPN31 * math.cos (2.*math.pi*freq/150000.))
            deltasIm = round (CPN31 * math.sin (2.*math.pi*freq/150000.))

            print ("deltasRe/Im=")
            print (deltasRe, deltasIm)
#            deltasIm = round (freq*89953.58465492555767605362)   # CPN31 * 2pi f / 150000. 
#            tmp=deltasIm*deltasIm
#            print (tmp)
#            tmp=long(tmp)>>32
#            print (tmp)
#            deltasRe = CPN31 - tmp
#            print ("deltasRe/Im approx=")
#            print (deltasRe, deltasIm)

            self.mk2rte.write_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), struct.pack("<llll", sc, fsv, deltasRe, deltasIm))
            self.mk2rte.write_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1) + 4*4, struct.pack("<ll", 2147483647, 0)) # reset
            print ("SCO=")

        scodata = self.mk2rte.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), fmt_sco1)
        print (scodata)

    def scodbg (self, button):
        scodata = self.mk2rte.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), fmt_sco1)
        print (scodata)

    def scoman (self, button):
        scodata = self.mk2rte.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), fmt_sco1)
        if scodata[11] > 0:
            self.mk2rte.write_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1) + 4*11, struct.pack("<l", 0)) # auto generator mode
        else:
            self.mk2rte.write_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1) + 4*11, struct.pack("<l", 1)) # manual generator mode
        scodata = self.mk2rte.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), fmt_sco1)
        print (scodata)

    def sco_read (self, button, id=0):
        self.scoid=id
        [c, fsv] = self.mk2rte.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), "<ll")
        self.s_offset.set_text(str(c/self.sco_s_offsetQ))
        self.fsens.set_text(str(fsv/self.sco_sensQ))
        self.scoidlab.set_label('SCO '+str(id+1))
        
    def sco_config (self, _button):
        name="SCO Config"
        if name not in wins:
            win = Gtk.Window()
            self.wins[name] = win
            win.connect("delete_event", self.delete_event)
            
            grid = Gtk.VBox()
            win.add(grid)

            self.scoidlab = Gtk.Label( label="SCO ?")
            grid.add(self.scoidlab)

            self.sco_sensQ     = 89953.58465492555767605362/1024.
            self.sco_s_offsetQ = 2147483648./10.
            
            [c, fsv] = self.mk2rte.read(i_sco1, "<ll")
            
            self.cl0 = Gtk.Label( label="Sig Offset [V]:")
            grid.add(self.cl0)
            self.s_offset = Gtk.Entry()
            self.s_offset.set_text(str(c/self.sco_s_offsetQ))
            grid.add (self.s_offset)

            self.fs0 = Gtk.Label( label="F Sens [Hz/V]:")
            grid.add(self.fs0)
            self.fsens = Gtk.Entry()
            self.fsens.set_text(str(fsv/self.sco_sensQ))
            grid.add (self.fsens)

            self.sco_read (0)
            
            bset = Gtk.Button(stock='set')
            bset.connect("clicked", self.scoset)
            grid.add(bset)

            bsco1 = Gtk.Button(stock='Select SCO 1')
            bsco1.connect("clicked", self.sco_read, 0)
            grid.add(bsco1)
            bsco2 = Gtk.Button(stock='Select SCO 2')
            bsco2.connect("clicked", self.sco_read, 1)
            grid.add(bsco2)

            bdbg = Gtk.Button(stock='debug')
            bdbg.connect("clicked", self.scodbg)
            grid.add(bdbg)

            bman = Gtk.Button(stock='toggle man/auto')
            bman.connect("clicked", self.scoman)
            grid.add(bman)

            button = Gtk.Button(stock='gtk-close')
            button.connect("clicked", lambda w: win.hide())
            grid.add(button)

            self.wins[name].show_all()

    def dummy (self, _button):
        return
    
    def wave_generator_stop (self, button):
        # immediate abort and stop wave gen if active -- will freeze output at last sample send!
        # self.mk2rte.write_wave_setup (0,1, 1, 1, 1, 1)
        
        # write zero wave to make sure it is off and forec restart with 4 zero samples
        wave = zeros (4, dtype('<i2'))
        self.mk2rte.write_wave (wave, 1, 1, 1)
        
    def wave_generator_start_sine (self, button):
        period    = float(self.wve_period.get_text())*1e-3 * 150000.0 # in samples
        channels  = 1
        wavelen   = int(channels * period+0.5)
        wavespeed = 1
        print (("WLi = %d"%wavelen))
        wtmp = wavelen
        while wtmp > 2000:
            wavespeed = wavespeed+1
            wtmp = wavelen / wavespeed
        wavelen = wtmp
    
        print ("WL = %d"%wavelen)
        print ("WS = %d"%wavespeed)
        print ("WLeff = %d"%(wavespeed*wavelen))
        
        wave = zeros (wavelen, dtype('<i2'))
        # compute wave form and upload
        A = 32768 * float(self.wve_ampl.get_text()) / 10
        for i in range(0, wavelen):
            wave[i] = int(A * math.sin(math.pi*2*i/wavelen))
            
        self.mk2rte.write_wave (wave, channels, wavespeed, int(self.wve_cycles.get_text()))

    def wave_generator_start_pulse (self, button):
        period    = float(self.wve_period.get_text())*1e-3 * 150000.0 # in samples
        pwd       = float(self.wve_pwd.get_text())*1e-3 * 150000.0 # in samples
        channels  = 1
        wavelen   = int(channels * period+0.5)
        wavespeed = 1
        print ("WLi = %d"%wavelen)
        wtmp = wavelen
        while wtmp > 2000:
            wavespeed = wavespeed+1
            wtmp = wavelen / wavespeed
        wavelen = wtmp
        pwd = pwd / wavespeed
    
        print ("WL = %d"%wavelen)
        print ("PW = %d"%pwd)
        print ("WS = %d"%wavespeed)
        print ("WLeff = %d"%(wavespeed*wavelen))
    
        wave = zeros (wavelen, dtype('<i2'))
        # compute wave form and upload
        A = 32768 * float(self.wve_ampl.get_text()) / 10
        for i in range(0, wavelen):
            if i < pwd:
                wave[i] = A
            else:
                wave[i] = 0
        
        self.mk2rte.write_wave (wave, channels, wavespeed, int(self.wve_cycles.get_text()))

              
    def wave_generator (self, _button):
        name="Arbitrary Wave Generator"
        if name not in wins:
            win = Gtk.Window()
            self.wins[name] = win
            win.connect("delete_event", self.delete_event)
            
            grid = Gtk.VBox()
            win.add(grid)

            self.wavegen = self.mk2rte.read(i_wavegen, fmt_wavegen)
            print (self.wavegen)
            
            self.wv_ampl = 1.0
            self.wvl_ampl = Gtk.Label( label="Amplitude in V:")
            grid.add(self.wvl_ampl)
            self.wve_ampl = Gtk.Entry()
            self.wve_ampl.set_text(str(self.wv_ampl))
            grid.add (self.wve_ampl)

            self.wv_pwd = 1.0
            self.wvl_pwd = Gtk.Label( label="Pulse Width in ms:")
            grid.add(self.wvl_pwd)
            self.wve_pwd = Gtk.Entry()
            self.wve_pwd.set_text(str(self.wv_pwd))
            grid.add (self.wve_pwd)

            self.wv_period = self.wavegen[ii_wavegen_wave_length]/self.wavegen[ii_wavegen_n_wave_channels]*self.wavegen[ii_wavegen_wave_speed]/150. # @150000/s
            self.wvl_period = Gtk.Label( label="Period in ms:")
            grid.add(self.wvl_period)
            self.wve_period = Gtk.Entry()
            self.wve_period.set_text(str(self.wv_period))
            grid.add (self.wve_period)

            self.wv_cycles = self.wavegen[ii_wavegen_max_wave_cycles]
            self.wvl_cycles = Gtk.Label( label="# Cycles:")
            grid.add(self.wvl_cycles)
            self.wve_cycles = Gtk.Entry()
            self.wve_cycles.set_text(str(self.wv_cycles))
            grid.add (self.wve_cycles)

            bstart = Gtk.Button(stock='Start Sine')
            bstart.connect("clicked", self.wave_generator_start_sine)
            grid.add(bstart)

            bstartp = Gtk.Button(stock='Start Pulse')
            bstartp.connect("clicked", self.wave_generator_start_pulse)
            grid.add(bstartp)

            bstop = Gtk.Button(stock='Stop')
            bstop.connect("clicked", self.wave_generator_stop)
            grid.add(bstop)

            button = Gtk.Button(stock='gtk-close')
            button.connect("clicked", lambda w: win.hide())
            grid.add(button)

            self.wins[name].show_all()

    # Vector index or signal variable offset from signal head pointer
    def global_vector_index (self):
        return self.vector_index

    def make_signal_selector (self, _input_id, sig, prefix, flag_nullptr_ok=0):
        signal_store = Gtk.ListStore(str, int)
        i = 0
        i_actual_signal = -1 # for anything not OK
        item = self.make_menu_item(signal_store, "???", self.mk2rte.change_signal_input, sig, -1)
        SIG_LIST = self.mk2rte.get_signal_lookup_list()
        for signal in SIG_LIST:
            if signal[SIG_ADR] > 0:
                item = self.make_menu_item(signal_store, prefix+"S%02d:"%signal[SIG_INDEX]+signal[SIG_NAME], self.mk2rte.change_signal_input, signal, _input_id, self.global_vector_index)
                if signal[SIG_ADR] == sig[SIG_ADR]:
                    i_actual_signal=i
                i=i+1
        if flag_nullptr_ok:
            item = self.make_menu_item(signal_store, "DISABLED", self.mk2rte.disable_signal_input, 0, _input_id, self.global_vector_index)

        if i_actual_signal == -1:
            i_actual_signal = i

        opt = Gtk.ComboBox.new_with_model(signal_store)
        opt.connect("changed", self.adjust_signal_callback, SIG_LIST, _input_id)
        renderer_text = Gtk.CellRendererText()
        opt.pack_start(renderer_text, True)
        opt.add_attribute(renderer_text, "text", 0)
        opt.set_active(i_actual_signal+1)
        opt.show()
        return opt

    def update_signal_menu_from_signal (self, menu, tap):
        [v, uv, signal] = self.mk2rte.get_monitor_signal (tap)
        if signal[SIG_INDEX] <= 0:
            menu.set_active(0)
            return 1
        menu.set_active(signal[SIG_INDEX]+1)
        return 1




    
    # build offset editor dialog
    def create_A810_ADDA_monitor(self, _button):
        name="A810/A16 AD/DA Monitor"
        if name not in wins:
            win = Gtk.Window()
            self.wins[name] = win
            win.connect("delete_event", self.delete_event)

            box1 = Gtk.VBox()
            win.add(box1)

            box2 = Gtk.VBox()
            box2.set_border_width(5)
            box1.pack_start(box2, True, True, 0)

            table = Gtk.Table(n_rows=20, n_columns=17)
            box2.pack_start(table, False, False, 0)

            r=0
            lab = Gtk.Label( label="A810 Channels ")
            table.attach (lab, 0, 1, r, r+1)

            lab = Gtk.Label( label="AD-IN Reading ")
            #lab.set_alignment(1.0, 0.5)
            table.attach (lab, 2, 3, 0, 1)

            lab = Gtk.Label( label="DA-OUT Reading ")
            #lab.set_alignment(1.0, 0.5)
            table.attach (lab, 6, 7, r, r+1)

            level_max = 10000. # mV
            level_fac = 0.305185095 # mV/Bit

            r = r+1
            separator = Gtk.HSeparator()
            separator.show()
            table.attach (separator, 0, 7, r, r+1)

            r = r+1
            # create table for CH0..7 in/out reading, hook tabs
            for tap in range(0, self.mk2rte.analog_channels):
                lab = Gtk.Label( label="CH-"+str(tap))
                table.attach (lab, 0, 1, r, r+1)

                labin = Gtk.Label(label="+####.# mV")
                #labin.set_alignment(1.0, 0.5)
                table.attach (labin, 2, 3, r, r+1)

                labout = Gtk.Label(label="+####.# mV")
                #labout.set_alignment(1.0, 0.5)
                table.attach (labout, 6, 7, r, r+1)

                GLib.timeout_add(timeout_update_A810_readings, self.A810_reading_update, labin.set_text, labout.set_text, tap)
                r = r+1

            
            box2 = Gtk.VBox()
            box1.pack_start(box2, False, True, 0)

            button = Gtk.Button(label='Close')
            button.connect("clicked", lambda w: win.hide())
            box2.pack_start(button, True, True, 0)

        self.wins[name].show_all()

    # LOG "VU" Meter
    def create_meterVU(self, w, tap, fsk):
        [value, uv, signal] = self.mk2rte.get_monitor_signal (tap)
        label = signal[SIG_NAME]
        unit  = signal[SIG_UNIT]
        name="meterVU-"+label
        if name not in wins:
            win = Gtk.Window()
            self.wins[name] = win
            win.connect("delete_event", self.delete_event)
            v = Gtk.VBox()
            instr = Instrument( Gtk.Label(), v, "VU", label)
            instr.show()
            win.add(v)

            def update_meter(inst, _tap, signal, fm):
#                [value, v, signal] = self.mk2rte.get_monitor_signal (_tap)
                value = self.mk2rte.get_monitor_data (_tap)


                if signal[SIG_VAR] == 'analog.rms_signal':
                    try:
                        float (fm ())
                        maxv = math.fabs( float (fm ()))
                    except ValueError:
                        maxv = (1<<31)*math.fabs(math.sqrt(signal[SIG_D2U]))
                
                    v = math.sqrt(math.fabs(value) * signal[SIG_D2U])
                else:
                    try:
                        float (fm ())
                        maxv = math.fabs( float (fm ()))
                    except ValueError:
                        maxv = (1<<31)*math.fabs(signal[SIG_D2U])
                
                    v = value * signal[SIG_D2U]

                maxdb = 20.*math.log10(maxv)
                # _labsv("%+06.2f " %v+signal[SIG_UNIT])

                if v >= 0:
                    p="+"
                else:
                    p="-"
                db = -maxdb + 20.*math.log10(math.fabs(v+0.001))

                inst.set_reading_auto_vu (db, p)
                inst.set_range(arange(0,maxv/10*11,maxv/10))

                return True

            GLib.timeout_add (timeout_update_meter_readings, update_meter, instr, tap, signal, fsk)
        self.wins[name].show_all()

    # Linear Meter
    def create_meterLinear(self, w, tap, fsk):
        [value, uv, signal] = self.mk2rte.get_monitor_signal (tap)
        label = signal[SIG_NAME]
        unit  = signal[SIG_UNIT]
        name="meterLinear-"+label
        if name not in wins:
            win = Gtk.Window()
            self.wins[name] = win
            win.connect("delete_event", self.delete_event)
            v = Gtk.VBox()
            try:
                float (fsk ())
                maxv = float (fsk ())
            except ValueError:
                maxv = (1<<31)*math.fabs(signal[SIG_D2U])

            instr = Instrument( Gtk.Label(), v, "Volt", label, unit)
            instr.set_range(arange(0,maxv/10*11,maxv/10))
            instr.show()
            win.add(v)

            def update_meter(inst, _tap, signal, fm):
#                [value, v, signal] = self.mk2rte.get_monitor_signal (_tap)
                value = self.mk2rte.get_monitor_data (_tap)
                v = value * signal[SIG_D2U]
                try:
                    float (fm ())
                    maxv = float (fm ())
                except ValueError:
                    maxv = (1<<31)*math.fabs(signal[SIG_D2U])
                
                if v >= 0:
                    p="+"
                else:
                    p="-"
                inst.set_reading_auto_vu (v, p)
                inst.set_range(arange(0,maxv/10*11,maxv/10))
                return True

            GLib.timeout_add (timeout_update_meter_readings, update_meter, instr, tap, signal, fsk)
        self.wins[name].show_all()

    def update_signal_menu_from_signal (self, menu, tap):
        [v, uv, signal] = self.mk2rte.get_monitor_signal(tap)
        if signal[SIG_INDEX] <= 0:
            menu.set_active(0)
            return 1
        menu.set_active(signal[SIG_INDEX]+1)
        return 1

    def create_signal_monitor(self, _button):
        name="SPM Signal Monitor"
        if name not in wins:
            win = Gtk.Window(
                      type=Gtk.WINDOW_TOPLEVEL,
                      title=name,
                      allow_grow=True,
                      allow_shrink=True,
                      border_width=2)
            self.wins[name] = win
            win.connect("delete_event", self.delete_event)

            box1 = Gtk.VBox()
            win.add(box1)

            box2 = Gtk.VBox()
            box2.set_border_width(5)
            box1.pack_start(box2, True, True, 0)

            table = Gtk.Table(n_rows=11, n_columns=37)
            table.set_row_spacings(0)
            table.set_col_spacings(4)
            box2.pack_start(table, False, False, 0)

            r=0
            lab = Gtk.Label( label="Signal to Monitor")
            table.attach (lab, 0, 1, r, r+1)

            lab = Gtk.Label( label="DSP Signal Variable name")
            table.attach (lab, 1, 2, r, r+1)

            lab = Gtk.Label(label="DSP Sig Reading")
            lab.set_alignment(1.0, 0.5)
            table.attach (lab, 2, 3, r, r+1)

            lab = Gtk.Label( label="Value")
            lab.set_alignment(1.0, 0.5)
            table.attach (lab, 6, 7, r, r+1)

            lab = Gtk.Label( label="dB")
            lab.set_alignment(1.0, 0.5)
            table.attach (lab, 7, 8, r, r+1)
            lab = Gtk.Label( label="V")
            lab.set_alignment(1.0, 0.5)
            table.attach (lab, 8, 9, r, r+1)

            lab = Gtk.Label( label="Galvo Range")
            lab.set_alignment(1.0, 0.5)
            table.attach (lab, 9, 10, r, r+1)

            r = r+1
            separator = Gtk.HSeparator()
            separator.show()
            table.attach (separator, 0, 7, r, r+1)

            r = r+1
            # create full signal monitor
            for tap in range(0,num_monitor_signals):
                [value, uv, signal] = self.mk2rte.get_monitor_signal (tap)
                if signal[SIG_INDEX] < 0:
                    break
                combo = self.make_signal_selector (DSP_SIGNAL_MONITOR_INPUT_BASE_ID+tap, signal, "Mon[%02d]: "%tap)
                table.attach (combo, 0, 1, r, r+1)

                GLib.timeout_add (timeout_update_signal_monitor_menu, self.update_signal_menu_from_signal, menu1, tap)

                lab2 = Gtk.Label( label=signal[SIG_VAR])
                lab2.set_alignment(-1.0, 0.5)
                table.attach (lab2, 1, 2, r, r+1)

                labv = Gtk.Label( label=str(value))
                labv.set_alignment(1.0, 0.5)
                table.attach (labv, 2, 3, r, r+1)

                labsv = Gtk.Label (label="+####.# mV")
                labsv.set_alignment(1.0, 0.5)
                table.attach (labsv, 6, 7, r, r+1)
                GLib.timeout_add (timeout_update_signal_monitor_reading, self.signal_reading_update, lab2.set_text, labv.set_text, labsv.set_text, tap)

                full_scale = Gtk.Entry()
                full_scale.set_text("auto")
                table.attach (full_scale, 9, 10, r, r+1)

                image = Gtk.Image()
                if os.path.isfile("meter-icondB.png"):
                    imagefile="meter-icondB.png"
                else:
                    imagefile="/usr/share/gxsm/pixmaps/meter-icondB.png"
                image.set_from_file(imagefile)
                image.show()
                button = Gtk.Button()
                button.add(image)
                button.connect("clicked", self.create_meterVU, tap, full_scale.get_text)
                table.attach (button, 7, 8, r, r+1)

                image = Gtk.Image()
                if os.path.isfile("meter-iconV.png"):
                    imagefile="meter-iconV.png"
                else:
                    imagefile="/usr/share/gxsm/pixmaps/meter-iconV.png"
                image.set_from_file(imagefile)
                image.show()
                button = Gtk.Button()
                button.add(image)
                button.connect("clicked", self.create_meterLinear, tap, full_scale.get_text)
                table.attach (button, 8, 9, r, r+1)

                

                r = r+1

            separator = Gtk.HSeparator()
            box1.pack_start(separator, False, True, 0)
            separator.show()

            box2 = Gtk.VBox ()
            box2.set_border_width(10)
            box1.pack_start(box2, False,  True, 0)

            button = Gtk.Button(stock='gtk-close')
            button.connect("clicked", lambda w: win.hide())
            box2.pack_start(button, True, True, 0)

        self.wins[name].show_all()

    def update_signal_menu_from_mod_inp(self, _lab, mod_inp):
        [signal, data, offset] = self.mk2rte.query_module_signal_input(mod_inp)
        if offset > 0:
            _lab (" == "+signal[SIG_VAR]+" [%d] ==> "%(offset))
        else:
            _lab (" == "+signal[SIG_VAR]+" ==> ")
        return 1

    # Build MK3-A810 FB-SPMCONTROL Signal Patch Rack
    def create_signal_patch_rack(self, _button):
        name="SPM Patch Rack"
        if name not in wins:
            win = Gtk.Window()
            self.wins[name] = win
            win.connect("delete_event", self.delete_event)
            win.set_size_request(800, 500)

            box1 = Gtk.VBox()
            win.add(box1)
            scrolled_window = Gtk.ScrolledWindow()
            scrolled_window.set_border_width(3)
            box1.pack_start(scrolled_window, True, True, 0)
            scrolled_window.show()
            box2 = Gtk.VBox()
            box2.set_border_width(3)
            scrolled_window.add_with_viewport(box2)
            box1.pack_start(box2, True, True, 0)
 
            table = Gtk.Table(n_rows=6, n_columns=17)
            table.set_row_spacings(0)
            table.set_col_spacings(4)
            box2.pack_start(table, False, False, 0)

            r=0
            lab = Gtk.Label( label="SIGNAL")
            table.attach (lab, 0, 1, r, r+1)

            lab = Gtk.Label( label="--- variable --->")
            table.attach (lab, 1, 2, r, r+1)

            lab = Gtk.Label( label="MODULE INPUT")
            lab.set_alignment(0.5, 0.5)
            table.attach (lab, 2, 3, r, r+1)

            r = r+1
            separator = Gtk.HSeparator()
            separator.show()
            table.attach (separator, 0, 7, r, r+1)

            r = r+1
            # create full signal monitor
            for mod in DSP_MODULE_INPUT_ID_CATEGORIZED.keys():
                for mod_inp in DSP_MODULE_INPUT_ID_CATEGORIZED[mod]:
                    if mod_inp[0] > 0: # INPUT_ID_VALID
                        [signal, data, offset] = self.mk2rte.query_module_signal_input(mod_inp[0])
                        if mod_inp[2] >= 0: # address valid, zero allowed in special cases if mod_inp[3] == 1!
                            # lab = Gtk.Label( label=signal[SIG_NAME])
                            # lab.set_alignment(-1.0, 0.5)    
                            # table.attach (lab1, 0, 1, r, r+1)
                            
                            prefix = "Sig: "
                            combo = self.make_signal_selector (mod_inp[0], signal, prefix, mod_inp[3])
                            table.attach (combo, 0, 1, r, r+1)

                            lab = Gtk.Label( label=" == "+signal[SIG_VAR]+" ==> ")
                            #                    lab = Gtk.Label( label=" ===> ")
                            lab.set_alignment(-1.0, 0.5)    
                            table.attach (lab, 1, 2, r, r+1)
                            GLib.timeout_add (timeout_update_patchrack, self.update_signal_menu_from_mod_inp, lab.set_text, mod_inp[0])

                            lab = Gtk.Label( label=mod_inp[1])
                            lab.set_alignment(-1.0, 0.5)    
                            table.attach (lab, 2, 3, r, r+1)

                            r = r+1

            separator = Gtk.HSeparator()
            box1.pack_start(separator, False, True, 0)
            separator.show()

            box2 = Gtk.VBox()
            box1.pack_start(box2, False, True, 0)

            button = Gtk.Button(stock='gtk-close')
            button.connect("clicked", lambda w: win.hide())
            box2.pack_start(button, True, True, 0)

        self.wins[name].show_all()

    # Build MK3-A810 FB-SPMCONTROL Signal Manger / FLASH SUPPORT
    def create_signal_manager(self, _button, flashdbg=0):
        name="SPM Signal and DSP Core Manager"
        if name not in wins:
            win = Gtk.Window()
            self.wins[name] = win
            self.dsp_man_win=win

            win.connect("delete_event", self.delete_event)
            win.set_size_request(800, 450)

            scrolled_window = Gtk.ScrolledWindow()
            scrolled_window.set_border_width(5)
            win.add(scrolled_window)
            
            box1 = Gtk.VBox()
            scrolled_window.add_with_viewport(box1)

            box2 = Gtk.VBox()
            box1.pack_start(box2, False, True, 0)

            button = Gtk.Button("REVERT TO POWER UP DEFAULTS")
            button.connect("clicked", self.mk2rte.disable_signal_input, 0, 0, 0) # REVERT TO POWER-UP-DEFAULT
            box2.pack_start(button, True, True, 0)

            hb = Gtk.HBox()
            box2.pack_start(hb, True, True, 0)
            button = Gtk.Button("SAVE DSP-CONFIG TO FILE")
            button.connect("clicked", self.mk2rte.read_and_save_actual_module_configuration, "mk3_signal_configuration.pkl") # store to file
            hb.pack_start(button, True, True, 0)

            button = Gtk.Button("LOAD DSP-CONFIG FROM FILE")
            button.connect("clicked", self.mk2rte.load_and_write_actual_module_configuration, "mk3_signal_configuration.pkl") # restore from file
            hb.pack_start(button, True, True, 0)

            
            hb = Gtk.HBox()
            box2.pack_start(hb, True, True, 0)
            button = Gtk.Button("STORE TO FLASH")
            button.connect("clicked", self.mk2rte.manage_signal_configuration, DSP_SIGNAL_TABLE_STORE_TO_FLASH_ID)
            hb.pack_start(button, True, True, 0)
            
            button = Gtk.Button("RESTORE FROM FLASH")
            button.connect("clicked", self.mk2rte.manage_signal_configuration, DSP_SIGNAL_TABLE_RESTORE_FROM_FLASH_ID)
            hb.pack_start(button, True, True, 0)
            
            button = Gtk.Button("ERASE (INVALIDATE) FLASH TABLE")
            button.connect("clicked", self.mk2rte.manage_signal_configuration, DSP_SIGNAL_TABLE_ERASE_FLASH_ID)
            hb.pack_start(button, True, True, 0)
            
            if flashdbg:
                separator = Gtk.HSeparator()
                box1.pack_start(separator, False, True, 0)
                separator.show()
                
                lab = Gtk.Label(label="-- FLASH debug actions enabled --")
                box1.pack_start(lab,False, True, 0)
                lab.show()
                
                self.mk2rte.flash_dump(0, 2)
                lab = Gtk.Label(label=self.mk2rte.flash_dump(0, 2))
                box1.pack_start(lab, False, True, 0)
                lab.show()
                
                hb = Gtk.HBox()
                box2.pack_start(hb, True, True, 0)
                data = Gtk.Entry()
                data.set_text("0")
                hb.pack_start(data, True, True, 0)
                
                button = Gtk.Button("SEEK")
                button.connect("clicked", self.mk2rte.manage_signal_configuration, DSP_SIGNAL_TABLE_SEEKRW_FLASH_ID, data.get_text)
                hb.pack_start(button, True, True, 0)
                
                button = Gtk.Button("READ")
                button.connect("clicked", self.mk2rte.manage_signal_configuration, DSP_SIGNAL_TABLE_READ_FLASH_ID)
                hb.pack_start(button, True, True, 0)
                
                button = Gtk.Button("WRITE")
                button.connect("clicked", self.mk2rte.manage_signal_configuration, DSP_SIGNAL_TABLE_WRITE_FLASH_ID, data.get_text)
                hb.pack_start(button, True, True, 0)
                
                button = Gtk.Button("TEST0")
                button.connect("clicked", self.mk2rte.manage_signal_configuration, DSP_SIGNAL_TABLE_TEST0_FLASH_ID)
                hb.pack_start(button, True, True, 0)
                
                button = Gtk.Button("TEST1")
                button.connect("clicked", self.mk2rte.manage_signal_configuration, DSP_SIGNAL_TABLE_TEST1_FLASH_ID)
                hb.pack_start(button, True, True, 0)
                
                button = Gtk.Button("DUMP")
                button.connect("clicked", self.mk2rte.flash_dump)
                hb.pack_start(button, True, True, 0)
                

            hb = Gtk.HBox()
            box2.pack_start(hb, True, True, 0)
            data =  Gtk.Label( label="DSP CORE MANAGER:")
            hb.pack_start(data, True, True, 0)
            button = Gtk.Button("CLR RTL-PEAK")
            button.connect("clicked", self.mk2rte.reset_task_control_peak)
            hb.pack_start(button, True, True, 0)
            button = Gtk.Button("CLR N EXEC")
            button.connect("clicked", self.mk2rte.reset_task_control_nexec)
            hb.pack_start(button, True, True, 0)
            button = Gtk.Button("REST CLOCK")
            button.connect("clicked", self.mk2rte.reset_dsp_clock)
            hb.pack_start(button, True, True, 0)
            button = Gtk.Button("STATE?")
            button.connect("clicked", self.mk2rte.print_statemachine_status)
            hb.pack_start(button, True, True, 0)

            hb = Gtk.HBox()
            box2.pack_start(hb, True, True, 0)
            lbl =  Gtk.Label( label="DP MAX Tuning: ")
            hb.pack_start(lbl, True, True, 0)
            button = Gtk.Button("Default")
            button.connect("clicked", self.mk2rte.set_dsp_rt_DP_max_default)
            hb.pack_start(button, True, True, 0)
            button = Gtk.Button("3800")
            button.connect("clicked", self.mk2rte.set_dsp_rt_DP_max_hi1)
            hb.pack_start(button, True, True, 0)
            button = Gtk.Button("4000")
            button.connect("clicked", self.mk2rte.set_dsp_rt_DP_max_hi2)
            hb.pack_start(button, True, True, 0)
            button = Gtk.Button("4200!")
            button.connect("clicked", self.mk2rte.set_dsp_rt_DP_max_hi3)
            hb.pack_start(button, True, True, 0)
            button = Gtk.Button("4400!!")
            button.connect("clicked", self.mk2rte.set_dsp_rt_DP_max_hi4)
            hb.pack_start(button, True, True, 0)

            
                
            def confirm_experimental(dummy, func, param1, param2=None):
                label = Gtk.Label("Confirm Experimental DSP code.\nWARNING\nUse on own risc. Not for production yet.")
                dialog = Gtk.Dialog("DSP Experimental Feature",
                            None,
                            Gtk.DIALOG_MODAL | Gtk.DIALOG_DESTROY_WITH_PARENT,
                            (Gtk.STOCK_CANCEL, Gtk.RESPONSE_REJECT,
                             Gtk.STOCK_OK, Gtk.RESPONSE_ACCEPT))
                dialog.vbox.pack_start(label, True, True, 0)
                label.show()
                checkbox = Gtk.CheckButton("I agree.")
                dialog.action_area.pack_end(checkbox)
                checkbox.show()
                response = dialog.run()
                dialog.destroy()
                
                if response == Gtk.RESPONSE_ACCEPT and checkbox.get_active():
                    func(dummy, param1, param2)
                else:
                    print ("Function not activated, please confirm via checkbox.")
                    


            button = Gtk.Button(stock='gtk-close')
            button.connect("clicked", lambda w: win.hide())
            box2.pack_start(button, True, True, 0)

            separator = Gtk.HSeparator()
            box2.pack_start (separator, False, True, 0)

            self.load=0.0
            labtimestructs = Gtk.Label (label="DSP TIME")
            labtimestructs.set_alignment(0.0, 0.5)
            box2.pack_start (labtimestructs, False, True, 0)
            GLib.timeout_add (timeout_update_process_list, self.rt_dsprtosinfo_update, labtimestructs)
                    
            table = Gtk.Table(n_rows=7, n_columns=20)
            table.set_row_spacings(0)
            table.set_col_spacings(4)
            box2.pack_start(table, False, False, 0)

            r=0
            c=0
            hdrs=["PID","DSPTIME","TASK-TIME","PEAK","MISSED","FLAGS","Name of RT Task"]
            for h in hdrs:
                labh = Gtk.Label( label=h)
                if c == 0 or c == 6:
                    labh.set_alignment(0.0, 0.5)
                else:
                    if c == 5:
                        labh.set_alignment(0.5, 0.5)
                    else:
                        labh.set_alignment(1.0, 0.5)
                table.attach (labh, c, c+1, r, r+1)
                c=c+1
            for pid in range(0,NUM_RT_TASKS):
                r=r+1
                c=0
                labpid = Gtk.Label (label="RT{:03d}".format(pid))
                labpid.set_alignment(0.0, 0.5)
                table.attach (labpid, c, c+1, r, r+1)
                c=c+1
                labdsp_t = Gtk.Label (label="0")
                labdsp_t.set_alignment(1.0, 0.5)
                table.attach (labdsp_t, c, c+1, r, r+1)
                c=c+1
                labtask_t = Gtk.Label (label="0")
                labtask_t.set_alignment(1.0, 0.5)
                table.attach (labtask_t, c, c+1, r, r+1)
                c=c+1
                labtask_m = Gtk.Label (label="0")
                labtask_m.set_alignment(1.0, 0.5)
                table.attach (labtask_m, c, c+1, r, r+1)
                c=c+1
                labmissed = Gtk.Label (label="0")
                labmissed.set_alignment(1.0, 0.5)
                table.attach (labmissed, c, c+1, r, r+1)
                c=c+1
                labflag = Gtk.Label (label="0")
                labflag.set_alignment(0.5, 0.5)
                table.attach (labflag, c, c+1, r, r+1)
                c=c+1
                lab=Gtk.Label( label=dsp_rt_process_name[pid])
                lab.set_alignment(0.0, 0.5)
                table.attach(lab, c, c+1, r, r+1)
                c=c+1
                if pid > 0:
                    cfg=Gtk.Button("cfg")
                    cfg.connect("clicked", self.configure_rt, pid)
                    table.attach(cfg, c, c+1, r, r+1)

                GLib.timeout_add (timeout_update_process_list, self.rt_process_list_update, labdsp_t, labtask_t, labtask_m, labmissed, labflag, labpid, pid)

            r=r+2
            c=0
            hdrs=["PID","TIME NEXT","INTERVAL", "Timer Info", "NUM EXEC", "FLAGS", "Name of Idle Task"]
            for h in hdrs:
                labh = Gtk.Label( label=h)
                if c == 0 or c == 6:
                    labh.set_alignment(0.0, 0.5)
                else:
                    if c == 5:
                        labh.set_alignment(0.5, 0.5)
                    else:
                        labh.set_alignment(1.0, 0.5)
                table.attach (labh, c, c+1, r, r+1)
                c=c+1

            for pid in range(0,NUM_ID_TASKS):
                r=r+1
                c=0
                labpid = Gtk.Label (label="ID{:03d}".format(pid+1))
                labpid.set_alignment(0.0, 0.5)
                table.attach (labpid, c, c+1, r, r+1)
                c=c+1
                labtn = Gtk.Label (label="0")
                labtn.set_alignment(1.0, 0.5)
                table.attach (labtn, c, c+1, r, r+1)
                c=c+1
                labtask_ti = Gtk.Label (label="0")
                labtask_ti.set_alignment(1.0, 0.5)
                table.attach (labtask_ti, c, c+1, r, r+1)
                c=c+1
                lab_tmr_frq = Gtk.Label (label="-")
                lab_tmr_frq.set_alignment(1.0, 0.5)
                table.attach (lab_tmr_frq, c, c+1, r, r+1)
                c=c+1
                labtask_n = Gtk.Label (label="0")
                labtask_n.set_alignment(1.0, 0.5)
                table.attach (labtask_n, c, c+1, r, r+1)
                c=c+1
                labtask_f = Gtk.Label (label="0")
                labtask_f.set_alignment(0.5, 0.5)
                table.attach (labtask_f, c, c+1, r, r+1)
                c=c+1
                lab=Gtk.Label( label=dsp_id_process_name[pid])
                lab.set_alignment(0.0, 0.5)
                table.attach(lab, c, c+1, r, r+1)
                c=c+1
                cfg=Gtk.Button("cfg")
                cfg.connect("clicked", self.configure_id, pid)
                table.attach(cfg, c, c+1, r, r+1)

                GLib.timeout_add (timeout_update_process_list, self.id_process_list_update, labpid, labtn, labtask_ti, lab_tmr_frq, labtask_n, labtask_f, pid)
            
        self.wins[name].show_all()

    def rt_process_list_update(self, _dsp_t, _task_t, _task_m, _missed, _flag, _labpid, pid):
        flags = self.mk2rte.get_task_control_entry(ii_statemachine_rt_task_control, ii_statemachine_rt_task_control_flags, pid)
        missed_last = int(_missed.get_text())
        missed = self.mk2rte.get_task_control_entry(ii_statemachine_rt_task_control, ii_statemachine_rt_task_control_missed, pid)

        if flags == 0:
            _labpid.set_markup("<span color='grey'>RT{:03d}</span>".format(pid))
            _flag.set_markup("<span color='grey'>INACTIVE</span>")
        else:
            if missed_last != missed:
                _labpid.set_markup("<span color='red'>RT{:03d}</span>".format(pid))
            else:
                _labpid.set_markup("<span color='blue'>RT{:03d}</span>".format(pid))
            if flags & 0x10:
                _flag.set_markup("<span color='blue'>ALWAYS</span>")
            if flags & 0x20:
                _flag.set_markup("<span color='green'>on ODD CLK</span>")
            if flags & 0x40:
                _flag.set_markup("<span color='green'>on EVEN CLK</span>")
                
        _dsp_t.set_text("{:10d}".format(self.mk2rte.get_task_control_entry(ii_statemachine_rt_task_control, ii_statemachine_rt_task_control_time, pid)))
        _task_t.set_text("{:10d}".format(self.mk2rte.get_task_control_entry_task_time (ii_statemachine_rt_task_control, pid)))
        _task_m.set_text("{:10d}".format(self.mk2rte.get_task_control_entry_peak_time (ii_statemachine_rt_task_control, pid)))
        _missed.set_text("{:10d}".format(missed))
        return 1

    def configure_rt(self, bnt, pid):
        pname  = Gtk.Label(dsp_rt_process_name[pid])
        flags = self.mk2rte.get_task_control_entry(adr_statemachine_rt_task_control, ii_statemachine_rt_task_control_flags, pid)
        label = Gtk.Label("Set Flags:")
        dialog = Gtk.Dialog(title="Adjust RT{:03d} Task".format(pid), transient_for=self.dsp_man_win, flags=0)
        dialog.add_buttons (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OK, Gtk.ResponseType.OK)

        dialog.vbox.pack_start(pname, True, True, 0)
        pname.show()
        hb = Gtk.HBox()
        dialog.vbox.pack_end(hb, True, True, 0)
        hb.show()
        hb.pack_start(label, True, True, 0)
        label.show()
        cb_active = Gtk.RadioButton.new_with_label_from_widget(None, label="Active")
        hb.pack_end(cb_active, True, True, 0)
        cb_active.show()
        cb_odd = Gtk.RadioButton.new_with_label_from_widget(cb_active,  label="Odd")
        hb.pack_end(cb_odd, True, True, 0)
        cb_odd.show()
        cb_even = Gtk.RadioButton.new_with_label_from_widget(cb_active,  label="Even")
        hb.pack_end(cb_even, True, True, 0)
        cb_even.show()
        cb_sleep = Gtk.RadioButton.new_with_label_from_widget(cb_active,  label="Sleep")
        hb.pack_end(cb_sleep, True, True, 0)
        cb_sleep.show()

        if flags & 0x10:
            cb_active.set_active(True)
        elif flags & 0x20:
            cb_odd.set_active(True)
        elif flags & 0x40:
            cb_even.set_active(True)
        else:
            cb_sleep.set_active(True)
            
        response = dialog.run()
        dialog.destroy()
        
        if response == Gtk.ResponseType.OK:
            if cb_active.get_active():
                self.mk2rte.configure_rt_task(pid, "active")
            elif cb_odd.get_active():
                self.mk2rte.configure_rt_task(pid, "odd")
            elif cb_even.get_active():
                self.mk2rte.configure_rt_task(pid, "even")
            elif cb_sleep.get_active():
                self.mk2rte.configure_rt_task(pid, "sleep")
            
    def id_process_list_update(self, _labpid, _tn, _ti, _tmr_frq, _ne, _flags, pid):
        flags = self.mk2rte.get_task_control_entry(ii_statemachine_id_task_control, ii_statemachine_id_task_control_flags, pid)
        if flags == 0:
            _labpid.set_markup("<span color='grey'>ID{:03d}</span>".format(pid+1))
            _flags.set_markup("<span color='grey'>INACTIVE</span>")
        else:
            if flags & 0x10:
                _labpid.set_markup("<span color='blue'>ID{:03d}</span>".format(pid+1))
                _flags.set_markup("<span color='blue'>ALWAYS</span>")
            elif flags & 0x20:
                _labpid.set_markup("<span color='green'>ID{:03d}</span>".format(pid+1))
                _flags.set_markup("<span color='green'>TIMER</span>")
            elif flags & 0x40:
                _labpid.set_markup("<span color='yellow'>ID{:03d}</span>".format(pid+1))
                _flags.set_markup("<span color='yellow'>CLOCK</span>")
            else:
                _labpid.set_markup("<span color='red'>ID{:03d}</span>".format(pid+1))
                _flags.set_markup("<span color='red'>???</span>")
                
        _tn.set_text("{:10d}".format(self.mk2rte.get_task_control_entry(ii_statemachine_id_task_control, ii_statemachine_id_task_control_time_next, pid)))
        interval = self.mk2rte.get_task_control_entry(ii_statemachine_id_task_control, ii_statemachine_id_task_control_interval, pid)
        _ti.set_text("{:10d}".format(interval))
        if interval > 0:
            _tmr_frq.set_text("{:g}Hz".format(150000./interval)+" {:g}ms".format(interval/150.))
        else:
            _tmr_frq.set_text("-")
        _ne.set_text("{:10d}".format(self.mk2rte.get_task_control_entry(ii_statemachine_id_task_control, ii_statemachine_id_task_control_n_exec, pid)))
        return 1

    def configure_id(self, btn, pid):
        pname  = Gtk.Label(dsp_id_process_name[pid])
        flags = self.mk2rte.get_task_control_entry(ii_statemachine_id_task_control, ii_statemachine_id_task_control_flags, pid)
        label = Gtk.Label("Set flags:")
        dialog = Gtk.Dialog(title="Adjust ID{:03d} Task".format(pid+1), transient_for=self.dsp_man_win, flags=0)
        dialog.add_buttons (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OK, Gtk.ResponseType.OK)

        dialog.vbox.pack_start(pname, True, True, 0)
        pname.show()
        hb = Gtk.HBox()
        dialog.vbox.pack_end(hb, True, True, 0)
        hb.show()
        hb.pack_start(label, True, True, 0)
        label.show()
        cb_always = Gtk.RadioButton.new_with_label_from_widget(None, label="Always")
        hb.pack_end(cb_always, True, True, 0)
        cb_always.show()
        cb_timer = Gtk.RadioButton.new_with_label_from_widget(cb_always, label="Timer")
        hb.pack_end(cb_timer, True, True, 0)
        cb_timer.show()
        cb_clock = Gtk.RadioButton.new_with_label_from_widget(cb_always, label="Clock")
        hb.pack_end(cb_clock, True, True, 0)
        cb_clock.show()
        cb_sleep = Gtk.RadioButton.new_with_label_from_widget(cb_always, label="Sleep")
        hb.pack_end(cb_sleep, True, True, 0)
        cb_sleep.show()

        if flags & 0x10:
            cb_always.set_active(True)
        elif flags & 0x20:
            cb_timer.set_active(True)
        elif flags & 0x40:
            cb_clock.set_active(True)
        else:
            cb_sleep.set_active(True)
            
        response = dialog.run()
        dialog.destroy()
        
        if response == Gtk.ResponseType.OK:
            if cb_always.get_active():
                self.mk2rte.configure_id_task(pid, "always")
            elif cb_timer.get_active():
                self.mk2rte.configure_id_task(pid, "timer")
            elif cb_clock.get_active():
                self.mk2rte.configure_id_task(pid, "clock")
            elif cb_sleep.get_active():
                self.mk2rte.configure_id_task(pid, "sleep")

    def rtc_get(self, param):
        return self.mk2rte.get_rtos_parameter (param)
    
    def rt_dsprtosinfo_update(self, labtimestructs):
        self.load = 0.9*self.load + 0.1*self.rtc_get (ii_statemachine_DataProcessTime)/self.rtc_get (ii_statemachine_DataProcessReentryTime)
        d = int((self.rtc_get (ii_statemachine_DSP_minutes)-1)/(24*60))
        h = int((self.rtc_get (ii_statemachine_DSP_minutes)-1)/60 - 24*d);
        m = int((self.rtc_get (ii_statemachine_DSP_minutes)-1) - 24*60*d - h*60)
        s = 59 - self.rtc_get (ii_statemachine_DSP_seconds)
        labtimestructs.set_text("DSP TIME STRUCTS: DP TICKS {:10d}   {:10d}s   {:10d}m   {:02d}s\n"
                    .format(self.rtc_get (ii_statemachine_DSP_time),
                        self.rtc_get (ii_statemachine_DSP_seconds),
                        self.rtc_get (ii_statemachine_DSP_minutes)-1,
                        self.rtc_get (ii_statemachine_DSP_seconds))
                    +"DSP STATUS up  {:d} days {:d}:{:02d}:{:02d}, Average Load: {:g}\n".format(d,h,m,s, self.load)
                    +"DP Reentry Time:  {:10d}  Earliest: {:10d}  Latest:  {:10d}\n"
                    .format(
                        self.rtc_get (ii_statemachine_DataProcessReentryTime),
                        self.rtc_get (ii_statemachine_DataProcessReentryPeak & 0xffff),
                        self.rtc_get (ii_statemachine_DataProcessReentryPeak >> 16))
                    +"DP Time: {:10d}  Max: {:10d}  Idle Max: {:10d} Min: {:10d}\n"
                    .format(
                        self.mk2rte.get_task_control_entry_task_time(ii_statemachine_rt_task_control, 0),
                        self.mk2rte.get_task_control_entry_peak_time(ii_statemachine_rt_task_control, 0),
                        self.rtc_get (ii_statemachine_IdleTime_Peak >> 16),
                        self.rtc_get (ii_statemachine_IdleTime_Peak & 0xffff))
                    +"DP Continue Time Limit: {:10d}".format(self.rtc_get (ii_statemachine_DP_max_time_until_abort))
        )
        return 1


    # updates the right side of the offset dialog
    def A810_reading_update(self, _labin, _labout, tap):
        tmp = self.mk2rte.get_ADDA ()

        level_fac = 0.305185095 # mV/Bit
        Vin  = level_fac * tmp[0][tap]
        Vout = level_fac * tmp[1][tap]

        _labin("%+06.3f mV" %Vin)
        _labout("%+06.3f mV" %Vout)

        return 1

    # updates the right side of the offset dialog
    def signal_reading_update(self, _sig_var, _labv, _labsv, tap):
        [value, uv, signal] = self.mk2rte.get_monitor_signal (tap)
        if signal[SIG_INDEX] <= 0:
            _labv("N/A")
            _labsv("+####.## mV")
            return 1

        _sig_var(signal[SIG_VAR])
        _labv(str(value))

        if (signal[SIG_UNIT] == "X"):
            _labsv("0x%08X " %value)
        else:
            _labsv("%+06.4f " %uv+signal[SIG_UNIT])
        return 1



    ##### fire up default background updates then run main gtk loop
    def main(self):
        GLib.timeout_add (timeout_DSP_status_reading, self.mk2rte.get_status)    
        GLib.timeout_add (timeout_DSP_signal_lookup_reading, self.mk2rte.read_signal_lookup, 1)

        Gtk.main()


########################################

print (__name__)
if __name__ == "__main__":
    mk2 = Mk2_Configurator ()
    mk2.main ()

## END
