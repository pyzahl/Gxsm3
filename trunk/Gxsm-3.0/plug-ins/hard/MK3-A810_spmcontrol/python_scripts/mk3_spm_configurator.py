#!/usr/bin/env python

## * Python User Control for
## * Configuration and SPM Approach Control tool for the FB_spm DSP program/GXSM2 Mk3-Pro/A810 based
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
#import gi
#gi.require_version('Gtk', '3.0')
#from gi.repository import Gtk

#version = "3.0.0"

import pygtk
pygtk.require('2.0')

import gobject, gtk
import time

import struct
import array

from numpy import *

import math
import re

import pydot
#import xdot
from scipy.optimize import leastsq

from mk3_spmcontol_class import *
from meterwidget import *
from scopewidget import *

# timeouts [ms]
timeout_update_patchrack              = 3000
timeout_update_patchmenu              = 3000
timeout_update_A810_readings          =  120
timeout_update_signal_monitor_menu    = 2000
timeout_update_signal_monitor_reading =  500
timeout_update_meter_readings         =   88

timeout_min_recorder          =   80
timeout_min_tunescope         =   20
timeout_tunescope_default     =  150

timeout_DSP_status_reading        =  100
timeout_DSP_signal_lookup_reading = 2100

# class MyDotWindow(xdot.DotWindow):
#
# 	def __init__(self):
# 		xdot.DotWindow.__init__(self)
# 		self.widget.connect('clicked', self.on_url_clicked)
#
# 	def on_url_clicked(self, widget, url, event):
# 		dialog = gtk.MessageDialog(
# 			parent = self, 
# 			buttons = gtk.BUTTONS_OK,
# 			message_format="%s clicked" % url)
# 		dialog.connect('response', lambda dialog, response: dialog.destroy())
# 		dialog.run()
# 		return True

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
		c = moduleg ["NodeTypes"];
		graph.add_subgraph (c)
		c.add_node ( pydot.Node( "Process Flow", style="filled", fillcolor="lightskyblue") )
		c.add_node ( pydot.Node( "Signal Source", style="filled", fillcolor="green") )
		c.add_node ( pydot.Node( "Modul Input", style="filled", fillcolor="gold") )
		c.add_node ( pydot.Node( "Modul Input=0", style="filled", fillcolor="grey") )
		c.add_node ( pydot.Node( "DISABLED (NULL Ptr)", style="filled", fillcolor="grey97") )
		c.add_node ( pydot.Node( "Signal Manipulation", style="filled", fillcolor="cyan") )
		c.add_node ( pydot.Node( "Unmanaged Node (PLL, ..)", style="filled", fillcolor="purple") )
		c.add_node ( pydot.Node( "Error Signal not found", style="filled", fillcolor="red") )

		mod_conf =  parent.mk3spm.get_module_configuration_list()
		for mod in mod_conf.keys():
			moduleg [mod] = pydot.Cluster(graph_name=mod,
						      style="filled",
						      color="lightgrey",
						      label = mod)
			c = moduleg [mod];
			print "AddSubgraph: "+mod
			graph.add_subgraph (c)
			signaln [mod] = pydot.Node( mod, style="filled", fillcolor="lightskyblue")
			c.add_node( signaln [mod] )

			record = ""
			fi=1
			for signal in parent.mk3spm.get_signal_lookup_list ():
				if signal[SIG_ADR] > 0:
					if signal[SIG_GRP] == mod:
						if re.search ("In\ [0-7]", signal[SIG_NAME]):
							if record == "":
								record = "<f0> "+re.escape (signal[SIG_NAME])
							else:
								record = record + "|<f%d> "%fi + re.escape (signal[SIG_NAME])
								fi=fi+1
							print "added to record: " + signal[SIG_NAME] + " ==> " + record
						signaln [signal[SIG_NAME]] = pydot.Node( signal[SIG_NAME], style="filled", fillcolor="green")
						c.add_node( signaln [signal[SIG_NAME]] )
#			if record != "":
#				signaln ["record_IN"] = pydot.Node ("record_IN", label=record, shape="record", style="filled", fillcolor="green")
#				c.add_node( signaln ["record_IN"] )

		for mod in mod_conf.keys():
			c = moduleg [mod];
			for mod_inp in mod_conf[mod]:
				[signal, data, offset] = parent.mk3spm.query_module_signal_input(mod_inp[0])
				# Null-Signal -- omit node connections, shade node
				if signal[SIG_VAR] == "analog.vnull":
					nodecolor="grey"
				else:
					nodecolor=mod_inp[4]
				print "INPUT NODE: ", mod_inp
				if mod_inp[2] >= 0 and signal[SIG_INDEX] != -1:
					inputn [mod_inp[1]] = pydot.Node( mod_inp[1], style="filled", fillcolor=nodecolor)
					c.add_node( inputn [mod_inp[1]] )
					print signal[SIG_NAME]," ==> ",mod_inp[1],"     (",mod_inp, signal, data, ")"
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
						print "DISABLED [p=0] ==> ",mod_inp[1],"  (",mod_inp, signal, data, ")"
						inputn [mod_inp[1]] = pydot.Node( mod_inp[1], style="filled", fillcolor="grey97")
					else:
						if mod_inp[4] == "purple":
							print "UNMANAGED NODE (PLL, etc.) ==> ", mod_inp[1],"  (",mod_inp, signal, data, ")"
							inputn [mod_inp[1]] = pydot.Node( mod_inp[1], style="filled", fillcolor=mod_inp[4])
						else:
							print "ERROR: ",mod_inp, signal, data, " ** can not find signal."
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

#		for inp in MIXER_RELATIONS.keys():
#			for s in MIXER_RELATIONS[inp]:
#                                print "processing MIXER_REL: ", s, inp
#				edge = pydot.Edge(inputn [inp], signaln [s], arrowhead="diamond", weight="2000")
#				graph.add_edge(edge)	
#		edge = pydot.Edge(signaln ["MIX 0"], signaln ["MIX out delta"], arrowhead="open", weight="100")
#		graph.add_edge(edge)	
#		edge = pydot.Edge(signaln ["MIX 1"], signaln ["MIX out delta"], arrowhead="open", weight="100")
#		graph.add_edge(edge)	
#		edge = pydot.Edge(signaln ["MIX 2"], signaln ["MIX out delta"], arrowhead="open", weight="100")
#		graph.add_edge(edge)	
#		edge = pydot.Edge(signaln ["MIX 3"], signaln ["MIX out delta"], arrowhead="open", weight="100")
#		graph.add_edge(edge)	

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

	#	graph.write_png('signal_graph.png')
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

	#	dotwindow = MyDotWindow()
	#	dotwindow.set_dotcode(graph.dot)

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

	[Xsignal, Xdata, Offset] = parent.mk3spm.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL1_INPUT_ID)
	[Ysignal, Ydata, Offset] = parent.mk3spm.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL2_INPUT_ID)

	label = "Oscilloscope/Recorder" + mode
	name=label
	self.ss = single_shot
	self.block_length = blen
	self.restart_func = self.nop
	self.trigger = 0
	self.trigger_level = 0
	if not parent.wins.has_key(name):
		win = gobject.new(gtk.Window,
				  type=gtk.WINDOW_TOPLEVEL,
				  title=label,
				  allow_grow=True,
				  allow_shrink=True,
				  border_width=0)
		parent.wins[name] = win
		v = gobject.new(gtk.VBox(spacing=0))

		scope = Oscilloscope( gobject.new(gtk.Label), v, "XT", label)
		scope.show()
		scope.set_chinfo([Xsignal[SIG_NAME], Ysignal[SIG_NAME]])
		win.add(v)
		if not self.ss:
			self.block_length = 512
			parent.mk3spm.set_recorder (self.block_length, self.trigger, self.trigger_level)

		table = gtk.Table(4, 2)
		table.set_row_spacings(5)
		table.set_col_spacings(5)
		v.pack_start (table)
		table.show()

		tr=0
		lab = gobject.new(gtk.Label, label="# Samples:")
		lab.show ()
		table.attach(lab, 2, 3, tr, tr+1)
		Samples = gtk.Entry()
		Samples.set_text("%d"%self.block_length)
		table.attach(Samples, 2, 3, tr+1, tr+2)
		Samples.show()

                [signal,data,offset] = parent.mk3spm.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL1_INPUT_ID)
                [lab1, menu1] = parent.make_signal_selector (DSP_SIGNAL_SCOPE_SIGNAL1_INPUT_ID, signal, "CH1: ", parent.global_vector_index)
#		lab = gobject.new(gtk.Label, label="CH1-scale:")
		lab1.show ()
		table.attach(lab1, 0, 1, tr, tr+1)
		Xscale = gtk.Entry()
		Xscale.set_text("1.")
		table.attach(Xscale, 0, 1, tr+1, tr+2)
		Xscale.show()

                [signal,data,offset] = parent.mk3spm.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL2_INPUT_ID)
                [lab2, menu1] = parent.make_signal_selector (DSP_SIGNAL_SCOPE_SIGNAL2_INPUT_ID, signal, "CH2: ", parent.global_vector_index)
#		lab = gobject.new(gtk.Label, label="CH2-scale:")
		lab2.show ()
		table.attach(lab2, 1, 2, tr, tr+1)
		Yscale = gtk.Entry()
		Yscale.set_text("1.")
		table.attach(Yscale, 1, 2, tr+1, tr+2)
		Yscale.show()

		tr = tr+2
		labx0 = gobject.new(gtk.Label, label="X-off:")
		labx0.show ()
		table.attach(labx0, 0, 1, tr, tr+1)
		Xoff = gtk.Entry()
		Xoff.set_text("0")
		table.attach(Xoff, 0, 1, tr+1, tr+2)
		Xoff.show()
		laby0 = gobject.new(gtk.Label, label="Y-off:")
		laby0.show ()
		table.attach(laby0, 1, 2, tr, tr+1)
		Yoff = gtk.Entry()
		Yoff.set_text("0")
		table.attach(Yoff, 1, 2, tr+1, tr+2)
		Yoff.show()

		self.xdc = 0.
		self.ydc = 0.

		def update_scope(set_data, Xsignal, Ysignal, xs, ys, x0, y0, num, x0l, y0l):
			blck = parent.mk3spm.check_recorder_ready ()
			if blck == -1:
				n = self.block_length
				[xd, yd] = parent.mk3spm.read_recorder (n)

				if not self.run:
                                        if n < 128:
                                                print "CH1:"
                                                print xd
                                                print "CH2:"
                                                print yd
                                        else:
                                                save("mk3_recorder_xdata", xd)
                                                save("mk3_recorder_ydata", yd)

				# auto subsample if big
				nss = n
				nraw = n
				if n > 4096:
					ss = int(n/2048)
					end =  ss * int(len(xd)/ss)
					nss = (int)(n/ss)
					xd = mean(xd[:end].reshape(-1, ss), 1)
					yd = mean(yd[:end].reshape(-1, ss), 1)
					scope.set_info(["sub sampling: %d"%n + " by %d"%ss,
							"T = %g ms"%(n/150.)
							])
				else:
					scope.set_info([
							"T = %g ms"%(n/150.)
							])

				# change number samples?
				try:
					self.block_length = int(num())
					if self.block_length < 64:
						self.block_length = 64
					if self.block_length > 999999:
						print "MAX BLOCK LEN is 999999"
						self.block_length = 1024
				except ValueError:
					self.block_length = 512

				if self.block_length != n or self.ss:
					self.run = gtk.FALSE
					self.run_button.set_label("RESTART")

				if not self.ss:
					parent.mk3spm.set_recorder (self.block_length, self.trigger, self.trigger_level)
				#				v = value * signal[SIG_D2U]
				#				maxv = (1<<31)*math.fabs(signal[SIG_D2U])
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
						"CH1:": "%.2f"%xscale_div + " %s/div"%Xsignal[SIG_UNIT],
						"CH2:": "%.1f"%yscale_div + " %s/div"%Xsignal[SIG_UNIT],
						"Timebase:": "%g ms/div"%(nraw/150./20.) 
						})
				
				scope.set_data (xd, yd)

				if self.mode > 1:
					self.run_button.set_label("SINGLE")
			else:
				scope.set_info(["waiting for trigger or data [%d]"%blck])
				scope.queue_draw()


			return self.run

		def set_restart_callback (func):
			self.restart_func = func
			
		def stop_update_scope (win, event=None):
			print "STOP, hide."
			win.hide()
			self.run = gtk.FALSE
			return True

		def toggle_run_recorder (b):
			if self.run:
				self.run = gtk.FALSE
				self.run_button.set_label("RUN")
			else:
				self.restart_func ()
				self.run = gtk.TRUE
				self.run_button.set_label("STOP")

                                [Xsignal, Xdata, Offset] = parent.mk3spm.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL1_INPUT_ID)
                                [Ysignal, Ydata, Offset] = parent.mk3spm.query_module_signal_input(DSP_SIGNAL_SCOPE_SIGNAL2_INPUT_ID)
                                scope.set_chinfo([Xsignal[SIG_NAME], Ysignal[SIG_NAME]])

                                period = int(2.*self.block_length/150.)
				if period < timeout_min_recorder:
					period = timeout_min_recorder
				gobject.timeout_add (period, update_scope, scope, Xsignal, Ysignal, Xscale.get_text, Yscale.get_text, Xoff.get_text, Yoff.get_text, Samples.get_text, labx0.set_text, laby0.set_text)
				if self.mode < 2: 
					self.run = gtk.TRUE
				else:
					self.run_button.set_label("WAITING")
					self.run = gtk.FALSE


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
			print self.trigger, self.trigger_level

		def toggle_mode (b):
			if self.mode == 0:
				self.mode = 1
				self.mode_button.set_label("M: Auto")
			else:
				if self.mode == 1:
					self.mode = 2
					self.mode_button.set_label("M: Normal")
				else:
					if self.mode == 2:
						self.mode = 3
						self.mode_button.set_label("M: Single")
					else:
						self.mode = 0
						self.mode_button.set_label("M: Free")

		self.run_button = gtk.Button("STOP")
#		self.run_button.set_use_stock(gtk.TRUE)
		self.run_button.connect("clicked", toggle_run_recorder)
		self.hb = gobject.new(gtk.HBox())
		self.hb.pack_start (self.run_button)
		self.mode_button = gtk.Button("M: Free")
		self.mode=0 # 0 free, 1 auto, 2 nommal, 3 single
		self.mode_button.connect("clicked", toggle_mode)
		self.hb.pack_start (self.mode_button)
		self.mode_button.show ()
		table.attach(self.hb, 2, 3, tr, tr+1)
		tr = tr+1

		self.trigger_button = gtk.Button("TRIGGER-OFF")
		self.trigger_button.connect("clicked", toggle_trigger)
		self.trigger_button.show ()
		table.attach(self.trigger_button, 2, 3, tr, tr+1)

		self.run = gtk.FALSE
		win.connect("delete_event", stop_update_scope)
		toggle_run_recorder (self.run_button)
		
	parent.wins[name].show_all()

    def nop (self):
        return 0
			
# try 1..2 deg or 0.5V as value
class StepResponse():
	def __init__(self, parent, value=0.05, mode="Amp"):
		self.parent = parent
		self.mk3spm = parent.mk3spm
		self.wins = parent.wins
		self.mode = mode

		self.value = value
		self.setup (value)
		self.scope = SignalScope(self, " Step Response: "+mode, 1)
		self.scope.set_restart_callback (self.setup)

	def setup(self, value=0.):
		if value != 0.:
			self.value = value
		if self.mode == "Amp":
			# Ampl step
			s1 = self.parent.mk3spm.lookup_signal_by_name ("PLL Res Amp LP")
			s2 = self.parent.mk3spm.lookup_signal_by_name ("PLL Exci Amp LP")
			self.parent.mk3spm.change_signal_input (0, s1 , DSP_SIGNAL_SCOPE_SIGNAL1_INPUT_ID)
			self.parent.mk3spm.change_signal_input (0, s2 , DSP_SIGNAL_SCOPE_SIGNAL2_INPUT_ID)
			self.parent.mk3spm. set_PLL_amplitude_step (self.value)

		else:
			# Phase Step
			s1 = self.parent.mk3spm.lookup_signal_by_name ("PLL Exci Frq LP")
			s2 = self.parent.mk3spm.lookup_signal_by_name ("PLL Res Ph LP")
			self.parent.mk3spm.change_signal_input (0, s1 , DSP_SIGNAL_SCOPE_SIGNAL1_INPUT_ID)
			self.parent.mk3spm.change_signal_input (0, s2 , DSP_SIGNAL_SCOPE_SIGNAL2_INPUT_ID)
			self.parent.mk3spm. set_PLL_phase_step (self.value)

class TuneScope():

    def __init__(self, parent, fc=29680.6, span=2., Fres=0.01, int_ms = 2000.):
	Xsignal = parent.mk3spm.lookup_signal_by_name("PLL Res Amp LP")
	Ysignal = parent.mk3spm.lookup_signal_by_name("PLL Res Ph LP")
	label = "Tune Scope -- X: " + Xsignal[SIG_NAME] + "  Y: " + Ysignal[SIG_NAME]
	name  = "Tune Scope"
	if not parent.wins.has_key(name):
		win = gobject.new(gtk.Window,
				  type=gtk.WINDOW_TOPLEVEL,
				  title=label,
				  allow_grow=True,
				  allow_shrink=True,
				  border_width=0)
		parent.wins[name] = win
		v = gobject.new(gtk.VBox(spacing=0))

		self.Fc       = fc
		self.Fspan    = span
		self.Fstep    = Fres
		self.interval = int_ms
		self.pos      = -10
		self.points   = int (2 * round (self.Fspan/2./self.Fstep) + 1)
		self.ResAmp   = ones (self.points)
		self.ResAmp2F = ones (self.points)
		self.ResPhase = zeros (self.points)
		self.ResPhase2F = zeros (self.points)
		self.Fit      = zeros (self.points)
		self.Freq     = zeros (self.points)
		self.volumeSine  = 0.01
                self.mode2f = 0
                self.phase_prev1 = 0
                self.phase_prev2 = 0
                
		def Frequency (position):
			return self.Fc - self.Fspan/2. + position * self.Fstep

		self.FSineHz  = Frequency (0.)

		scope = Oscilloscope( gobject.new(gtk.Label), v, "XT", label)
		scope.set_scale ( { Xsignal[SIG_UNIT]: "mV", Ysignal[SIG_UNIT]: "deg", "Freq." : "Hz" })
		scope.set_chinfo(["Res Ampl LP","Res Phase LP","Model","Res Ampl 2F","Res Phase 2F"])
		win.add(v)

		table = gtk.Table(4, 2)
		table.set_row_spacings(5)
		table.set_col_spacings(5)
		v.pack_start (table)

		tr=0
		lab = gobject.new(gtk.Label, label="Ampl scale: V/div")
		table.attach(lab, 0, 1, tr, tr+1)
		self.Xscale = gtk.Entry()
		self.Xscale.set_text("0.1")
		table.attach(self.Xscale, 0, 1, tr+1, tr+2)
		lab = gobject.new(gtk.Label, label="Phase scale: deg/Div")
		table.attach(lab, 1, 2, tr, tr+1)

		self.Yscale = gtk.Entry()
		self.Yscale.set_text("20.")
		table.attach(self.Yscale, 1, 2, tr+1, tr+2)
		lab = gobject.new(gtk.Label, label="Fc [Hz]:")
		table.attach(lab, 2, 3, tr, tr+1)
		self.Fcenter = gtk.Entry()
		self.Fcenter.set_text("%g"%self.Fc)
		table.attach(self.Fcenter, 2, 3, tr+1, tr+2)

		lab = gobject.new(gtk.Label, label="Span [Hz]:")
		table.attach(lab, 3, 4, tr, tr+1)
		self.FreqSpan = gtk.Entry()
		self.FreqSpan.set_text("%g"%self.Fspan)
		table.attach(self.FreqSpan, 3, 4, tr+1, tr+2)


		tr = tr+2
		self.labx0 = gobject.new(gtk.Label, label="Amp off:")
		table.attach(self.labx0, 0, 1, tr, tr+1)
		self.Xoff = gtk.Entry()
		self.Xoff.set_text("0")
		table.attach(self.Xoff, 0, 1, tr+1, tr+2)

		self.laby0 = gobject.new(gtk.Label, label="Phase off:")
		table.attach(self.laby0, 1, 2, tr, tr+1)
		self.Yoff = gtk.Entry()
		self.Yoff.set_text("0") ## 180
		table.attach(self.Yoff, 1, 2, tr+1, tr+2)

		lab = gobject.new(gtk.Label, label="Volume Sine [V]:")
		table.attach(lab, 2, 3, tr, tr+1)
		self.Vs = gtk.Entry()
		self.Vs.set_text("%g"%self.volumeSine)
		table.attach(self.Vs, 2, 3, tr+1, tr+2)

		lab = gobject.new(gtk.Label, label="Interval [ms]:")
		table.attach(lab, 3, 4, tr, tr+1)
		self.Il = gtk.Entry()
		self.Il.set_text("%d"%self.interval)
		table.attach(self.Il, 3, 4, tr+1, tr+2)

		self.M2F = gtk.CheckButton ("Mode 1F+2F")
		table.attach(self.M2F, 0, 1, tr+2, tr+3)

		lab = gobject.new(gtk.Label, label="Freq. Res. [Hz]:")
		table.attach(lab, 1, 2, tr+2, tr+3)
		self.FreqRes = gtk.Entry()
		self.FreqRes.set_text("%g"%self.Fstep)
		table.attach(self.FreqRes, 2, 3, tr+2, tr+3)

		self.xdc = 0.
		self.ydc = 0.

		self.fitresults = { "A":0., "f0": 32768., "Q": 0. }
		self.fitinfo = ["", "", ""]
		self.resmodel = ""

		def run_fit(iA0, if0, iQ):
			class Parameter:
			    def __init__(self, initialvalue, name):
				self.value = initialvalue
				self.name=name
			    def set(self, value):
				self.value = value
			    def __call__(self):
				return self.value

			def fit(function, parameters, x, data, u):
				def fitfun(params):
					for i,p in enumerate(parameters):
						p.set(params[i])
					return (data - function(x))/u

				if x is None: x = arange(data.shape[0])
				if u is None: u = ones(data.shape[0],"float")
				p = [param() for param in parameters]
				return leastsq(fitfun, p, full_output=1)

			# define function to be fitted
			def resonance(f):
				A=1000.
				return (A/A0())/sqrt(1.0+Q()**2*(f/w0()-w0()/f)**2)

			self.resmodel = "Model:  A(f) = (1000/A0) / sqrt (1 + Q^2 * (f/f0 - f0/f)^2)"

			# read data
			## freq, vr, dvr=load('lcr.dat', unpack=True)
			freq = self.Freq + self.Fstep*0.5         ## actual adjusted frequency, aligned for fit ??? not exactly sure why.
			vr   = self.ResAmp 
			dvr  = self.ResAmp/100.  ## error est. 1%

			# the fit parameters: some starting values
			A0=Parameter(iA0,"A")
			w0=Parameter(if0,"f0")
			Q=Parameter(iQ,"Q")

			p=[A0,w0,Q]

			# for theory plot we need some frequencies
			freqs=linspace(self.Fc - self.Fspan/2, self.Fc + self.Fspan/2, 200)
			initialplot=resonance(freqs)

#			self.Fit = initialplot
			
			# uncertainty calculation
#			v0=10.0
#			uvr=sqrt(dvr*dvr+vr*vr*0.0025)/v0
			v0=1.
			uvr=sqrt(dvr*dvr+vr*vr*0.0025)/v0

			# do fit using Levenberg-Marquardt
#			p2,cov,info,mesg,success=fit(resonance, p, freq, vr/v0, uvr)
			p2,cov,info,mesg,success=fit(resonance, p, freq, vr, None)

			if success==1:
				print "Converged"
			else:
				self.fitinfo[0] = "Fit not converged."
				print "Not converged"
				print mesg

			# calculate final chi square
			chisq=sum(info["fvec"]*info["fvec"])

			dof=len(freq)-len(p)
			# chisq, sqrt(chisq/dof) agrees with gnuplot
			print "Converged with chi squared ",chisq
			print "degrees of freedom, dof ", dof
			print "RMS of residuals (i.e. sqrt(chisq/dof)) ", sqrt(chisq/dof)
			print "Reduced chisq (i.e. variance of residuals) ", chisq/dof
			print

			# uncertainties are calculated as per gnuplot, "fixing" the result
			# for non unit values of the reduced chisq.
			# values at min match gnuplot
			print "Fitted parameters at minimum, with 68% C.I.:"
			for i,pmin in enumerate(p2):
				self.fitinfo[i]  = "%-10s %12f +/- %10f"%(p[i].name,pmin,sqrt(cov[i,i])*sqrt(chisq/dof))
				self.fitresults[p[i].name] = pmin
				print self.fitinfo[i]
			print

			print "Correlation matrix"
			# correlation matrix close to gnuplot
			print "               ",
			for i in range(len(p)): print "%-10s"%(p[i].name,),
			print
			for i in range(len(p2)):
			    print "%10s"%p[i].name,
			    for j in range(i+1):
				print "%10f"%(cov[i,j]/sqrt(cov[i,i]*cov[j,j]),),
			    print

			finalplot=resonance(freqs)
			self.Fit = finalplot
#			print "----------------- Final:"
#			print self.Fit

                        save("mk3_tune_Freq", self.Freq)
                        save("mk3_tune_Fit", self.Fit)
                        save("mk3_tune_ResAmp", self.ResAmp)
                        save("mk3_tune_ResPhase", self.ResPhase)
                        save("mk3_tune_ResPhase2F", self.ResPhase2F)
                        save("mk3_tune_ResAmp2F", self.ResAmp2F)


		self.peakvalue = 0
		self.peakindex = 0

		def update_tune(set_data, Xsignal, Ysignal):
			Filter64Out = parent.mk3spm.read_PLL_Filter64Out ()
			cur_a = Filter64Out[iii_PLL_F64_ResAmpLP] * Xsignal[SIG_D2U]
			cur_ph = Filter64Out[iii_PLL_F64_ResPhaseLP] * Ysignal[SIG_D2U]
			print self.pos, Filter64Out, cur_a, cur_ph, self.phase_prev1, self.points
			
			# init phase 29681.6
			if self.pos < 0:
				while cur_ph <= -180:
					cur_ph += 360
				while cur_ph >= 180:
					cur_ph -= 360

				if self.mode2f == 0:
					self.phase_prev1 = cur_ph
				else:
					self.phase_prev2 = cur_ph

			# full phase unwrap
                        if self.mode2f == 0:
                                pre_ph = self.phase_prev1
                        else:
                                pre_ph = self.phase_prev2
                        
                        # P_UnWrapped(i)=P(i)-Floor(((P(i)-P(i-1))/2Pi)+0.5)*2Pi

			if cur_ph - pre_ph > 180.:
                                cur_ph = cur_ph - 360.
				print "Ph UnWrap pos=", cur_ph
			if cur_ph - pre_ph < -180.:
                                cur_ph = cur_ph + 360.
				print "Ph UnWrap neg=", cur_ph

			if self.pos >= 0 and self.pos < self.points:
				self.Freq[self.pos]     = self.FSineHz
                                if self.mode2f == 0:
                                        self.ResAmp[self.pos]   = cur_a
                                        self.ResPhase[self.pos] = cur_ph
                                        self.phase_prev1 = cur_ph
                                
                                        if self.peakvalue < cur_a:
                                                self.peakvalue = cur_a
                                                self.peakindex = self.pos
                                else:
                                        self.ResAmp2F[self.pos]   = cur_a
                                        self.ResPhase2F[self.pos] = cur_ph
                                        self.phase_prev2 = cur_ph
			else:
				self.peakvalue = 0

                        if self.M2F.get_active ():
                                if self.mode2f == 1:
                                        self.pos      = self.pos+1
                                        self.mode2f = 0
                                else:
                                        self.mode2f = 1
                        else:
                                self.pos      = self.pos+1
                                self.mode2f = 0
                                
                                
			if self.pos >= self.points:
				self.run = gtk.FALSE
				self.run_button.set_label("RESTART")
                                if self.peakvalue > 0.:
                                        run_fit (1000./self.peakvalue, Frequency (self.peakindex), 20000.)
				parent.mk3spm.adjust_PLL_sine (self.volumeSine, self.fitresults["f0"], self.mode2f)
                                self.mode2f = 0
				self.pos = -10
			else:
				self.Fstep   = self.Fspan/(self.points-1)
				self.FSineHz = Frequency (self.pos)
				parent.mk3spm.adjust_PLL_sine (self.volumeSine, self.FSineHz, self.mode2f)

			if self.peakindex > 0 and self.peakindex < self.points:
				scope.set_info(["tuning at %.3f Hz"%self.FSineHz + " [%d]"%self.pos,
						"cur peak at: %.3f Hz"%Frequency (self.peakindex),
						"Res Amp: %g V"%cur_a + " [ %g V]"%self.peakvalue,
						"Phase: %3.1f deg"%self.phase_prev1 + " [ %3.1f deg]"%self.phase_prev2,
						"Vol. Sine: %.3f V"%self.volumeSine,"","","","","",
						self.resmodel,
						self.fitinfo[0],self.fitinfo[1],self.fitinfo[2]
						])
			else:
				scope.set_info(["tuning at %.3f Hz"%self.FSineHz + " [%d]"%self.pos,
						"cur peak at: --",
						"Res Amp: %g V"%cur_a,
						"Phase: %3.1f deg"%self.phase_prev1,
						"Vol. Sine: %.3f V"%self.volumeSine,"","","","","",
						self.resmodel,
						self.fitinfo[0],self.fitinfo[1],self.fitinfo[2]
						])

			n = self.pos

			try:
				self.Fc = float(self.Fcenter.get_text())
				if self.Fc < 1000.:
					self.Fc=32766.
				if self.Fc > 75000.:
					self.Fc=32766.
			except ValueError:
				self.Fc=32766.

			try:
				self.volumeSine = float(self.Vs.get_text())
				if self.volumeSine < 0.:
					self.volumeSine=0.
				if self.volumeSine > 10.:
					self.volumeSine=1.
			except ValueError:
				self.volumeSine=1.

			tmp = 1.
			try:
				xscale_div = float(self.Xscale.get_text())
				if xscale_div == 0.:
					xscale_div = tmp
			except ValueError:
				xscale_div = tmp
				
			tmp = 1.
			try:
				yscale_div = float(self.Yscale.get_text())
				if yscale_div == 0.:
					yscale_div = tmp
			except ValueError:
				yscale_div = tmp

			try:
				self.xoff = float(self.Xoff.get_text())
				for i in range (0, n, 8):
					self.xdc = 0.9 * self.xdc + 0.1 * self.ResAmp[i]
				self.labx0.set_text ("X-DC = %g"%self.xdc)
			except ValueError:
				for i in range (0, n, 8):
					self.xoff = 0.9 * self.xoff + 0.1 * self.ResAmp[i]
				self.labx0.set_text ("Amp Avg=%g"%self.xoff)

			try:
				self.yoff = float(self.Yoff.get_text())
				for i in range (0, n, 8):
					self.ydc = 0.9 * self.ydc + 0.1 * self.ResPhase[i]
				self.laby0.set_text ("Phase Avg=%g"%self.ydc)
			except ValueError:
				for i in range (0, n, 8):
					self.yoff = 0.9 * self.yoff + 0.1 * self.ResPhase[i]
				self.laby0.set_text ("Y-off: %g"%self.yoff)

			xd = -(self.ResAmp - self.xoff) / xscale_div
			fd = -(self.Fit - self.xoff) / xscale_div
			yd = -(self.ResPhase - self.yoff) / yscale_div

			ud = -(self.ResAmp2F - self.xoff) / xscale_div
			vd = -(self.ResPhase2F - self.yoff) / yscale_div

                        scope.set_scale ( { 
					"CH1: (Ampl.)":"%.2f V/div"%xscale_div, 
					"CH2: (Phase)":"%.1f deg/div"%yscale_div, 
					"Freq.: ": "%g Hz/div"%(self.Fspan/20.)
					})
			x = 20.*self.pos/self.points - 10 + 0.1
			y = -(cur_a- self.xoff) / xscale_div + 0.25
#			scope.set_data (xd, yd, fd, array([[x,y],[x, y-0.5]]))
                        scope.set_data_with_uv (xd, yd, fd, ud, vd, array([[x,y],[x, y-0.5]]))


			return self.run

		def stop_update_tune (win, event=None):
			print "STOP, hide."
			win.hide()
			self.run = gtk.FALSE
			return True

		def toggle_run_tune (b):
			if self.run:
				self.run = gtk.FALSE
				self.run_button.set_label("RUN TUNE")
				run_fit (1000./self.peakvalue, Frequency (self.peakindex), 20000.)
			else:
				self.run = gtk.TRUE
				self.run_button.set_label("STOP TUNE")
				try:
					tmp = float(self.FreqSpan.get_text())
					if tmp != self.Fspan:
						if tmp > 0.1 and tmp < 75000:
							self.Fspan = tmp
				except ValueError:
					print "invalid entry"
				try:
					tmp = float(self.FreqRes.get_text())
					if tmp != self.Fstep:
						if tmp > 0.0001 and tmp < 100.:
							self.Fstep=tmp
							self.points   = int (2 * round (self.Fspan/2./self.Fstep) + 1)
							self.ResAmp   = ones (self.points)
							self.ResPhase2F = zeros (self.points)
							self.ResAmp2F   = ones (self.points)
							self.ResPhase = zeros (self.points)
							self.Fit      = zeros (self.points)
							self.Freq     = zeros (self.points)
                                                        self.mode2f   = 0
							self.pos      = -10
						else:
							print "invalid Fstep entry"
				except ValueError:
					print "invalid Fstep entry"

				self.interval = int (self.Il.get_text())
				self.fitinfo = ["","",""]
				self.resmodel = ""
				if self.interval < timeout_min_tunescope or self.interval > 30000:
					self.interval = timeout_tunescope_default
				gobject.timeout_add (self.interval, update_tune, scope, Xsignal, Ysignal)


		self.run_button = gtk.Button("STOP TUNE")
		self.run_button.connect("clicked", toggle_run_tune)
		table.attach(self.run_button, 3, 4, tr+2, tr+3)

		self.run = gtk.FALSE
		win.connect("delete_event", stop_update_tune)
		toggle_run_tune (self.run_button)
		
	parent.wins[name].show_all()




class SignalPlotter():
	# X: 7=time, plotting Monitor Taps 20,21,0,1
    def __init__(self, parent, length = 300., taps=[7,20,21,0,1]):
        self.monitor_taps = taps
        [value, uv1, Xsignal] = parent.mk3spm.get_monitor_signal (self.monitor_taps[1])
        [value, uv2, Ysignal] = parent.mk3spm.get_monitor_signal (self.monitor_taps[2])
        [value, uv3, Usignal] = parent.mk3spm.get_monitor_signal (self.monitor_taps[3])
        [value, uv4, Vsignal] = parent.mk3spm.get_monitor_signal (self.monitor_taps[4])

	label = "Signal Plotter -- Monitor Indexes 20,21,0,1"
	name  = "Signal Plotter"

	if not parent.wins.has_key(name):
		win = gobject.new(gtk.Window,
				  type=gtk.WINDOW_TOPLEVEL,
				  title=label,
				  allow_grow=True,
				  allow_shrink=True,
				  border_width=0)
		parent.wins[name] = win
		v = gobject.new(gtk.HBox(spacing=0))

		self.pos      = 0
                self.span     = length
		self.points   = 2000
		self.dt       = self.span/self.points
		self.Time = zeros (self.points)
		self.Tap1 = zeros (self.points)
		self.Tap2 = zeros (self.points)
		self.Tap3 = zeros (self.points)
		self.Tap4 = zeros (self.points)

                self.t0 = parent.mk3spm.get_monitor_data (self.monitor_taps[0])

		scope = Oscilloscope( gobject.new(gtk.Label), v, "XT", label)
		scope.set_scale ( { Xsignal[SIG_UNIT]: "mV", Ysignal[SIG_UNIT]: "deg", "time" : "s" })
		scope.set_chinfo(["MON1","MON2","MON3","MON4"])
		win.add(v)

		table = gtk.Table(4, 2)
		table.set_row_spacings(5)
		table.set_col_spacings(5)
		v.pack_start (table)

		tr=0
		lab = gobject.new(gtk.Label, label="CH1 scale: V/div")
		table.attach(lab, 0, 1, tr, tr+1)
		self.M1scale = gtk.Entry()
		self.M1scale.set_text("0.5")
		table.attach(self.M1scale, 0, 1, tr+1, tr+2)

		lab = gobject.new(gtk.Label, label="CH2 scale: V/Div")
		table.attach(lab, 1, 2, tr, tr+1)
		self.M2scale = gtk.Entry()
		self.M2scale.set_text("0.5")
		table.attach(self.M2scale, 1, 2, tr+1, tr+2)

		lab = gobject.new(gtk.Label, label="Span [s]:")
		table.attach(lab, 3, 4, tr, tr+1)
		self.Span = gtk.Entry()
		self.Span.set_text("%g"%self.span)
		table.attach(self.Span, 3, 4, tr+1, tr+2)


		tr = tr+2
		self.labx0 = gobject.new(gtk.Label, label="CH1 off:")
		table.attach(self.labx0, 0, 1, tr, tr+1)
		self.M1off = gtk.Entry()
		self.M1off.set_text("0")
		table.attach(self.M1off, 0, 1, tr+1, tr+2)

		self.laby0 = gobject.new(gtk.Label, label="CH2 off:")
		table.attach(self.laby0, 1, 2, tr, tr+1)
		self.M2off = gtk.Entry()
		self.M2off.set_text("0")
		table.attach(self.M2off, 1, 2, tr+1, tr+2)

		lab = gobject.new(gtk.Label, label="Interval [s]:")
		table.attach(lab, 3, 4, tr, tr+1)
		self.Il = gtk.Entry()
		self.Il.set_text("%d"%self.span)
		table.attach(self.Il, 3, 4, tr+1, tr+2)

#		self.M2F = gtk.CheckButton ("Mode 1F+2F")
#		table.attach(self.M2F, 0, 1, tr+2, tr+3)

		self.xdc = 0.
		self.ydc = 0.

		def update_plotter():

                        if self.pos >= self.points:
                                #self.run = gtk.FALSE
				#self.run_button.set_label("RESTART")
				self.pos = 0
                                                          
                                save ("plotter_t-"+str(self.t0), self.Time)
                                save ("plotter_t1-"+str(self.t0), self.Tap1)
                                save ("plotter_t2-"+str(self.t0), self.Tap2)
                                save ("plotter_t3-"+str(self.t0), self.Tap3)
                                save ("plotter_t4-"+str(self.t0), self.Tap4)
                                # auto loop
                                self.t0 = parent.mk3spm.get_monitor_data (self.monitor_taps[0])
                                
			n = self.pos
                        self.pos = self.pos+1

                        t = parent.mk3spm.get_monitor_data (self.monitor_taps[0])
                        t = (t-self.t0) / 150000.
                        self.Time[n] = t
                        
                        [value, uv1, signal1] = parent.mk3spm.get_monitor_signal (self.monitor_taps[1])
                        self.Tap1[n] = uv1
                        [value, uv2, signal2] = parent.mk3spm.get_monitor_signal (self.monitor_taps[2])
                        self.Tap2[n] = uv2
                        [value, uv3, signal3] = parent.mk3spm.get_monitor_signal (self.monitor_taps[3])
                        self.Tap3[n] = uv3
                        [value, uv4, signal4] = parent.mk3spm.get_monitor_signal (self.monitor_taps[4])
                        self.Tap4[n] = uv4

                        if n == 0:
                            print "plotter data signals:"
                            print signal1, signal2, signal3
                            scope.set_chinfo([signal1[SIG_NAME],signal2[SIG_NAME],signal3[SIG_NAME],signal4[SIG_NAME]])

                        print n, t, uv1, uv2, uv3, uv4
                        
			try:
				m1scale_div = float(self.M1scale.get_text())
			except ValueError:
				m1scale_div = 1
				
			try:
				m2scale_div = float(self.M2scale.get_text())
			except ValueError:
				m2scale_div = 1

			try:
				m1off = float(self.M1off.get_text())
			except ValueError:
				m1off = 0
				
			try:
				m2off = float(self.M2off.get_text())
			except ValueError:
				m2off = 0
                                
                        scope.set_scale ( { 
					"XY1: (Tap1)":"%.2f mV/dev"%m1scale_div, 
					"XY2: (Tap2)":"%.2f mV/div"%m2scale_div, 
					"time: ": "%g s/div"%(self.span/20.)
					})

                        ### def set_data (self, Xd, Yd, Zd=zeros(0), XYd=[zeros(0), zeros(0)]):

                        #  if t > self.span:
                        #    td = 20. * (self.Time-t+self.span)/self.span
                        # else:
                        td = -10. + 20. * self.Time/self.span
                        t1 = -(self.Tap1 - m1off) / m1scale_div
                        t2 = -(self.Tap2 - m2off) / m2scale_div 
                        t3 = -(self.Tap3 - m2off) / m2scale_div 
                        t4 = -(self.Tap4 - m2off) / m2scale_div 
                        #scope.set_data (zeros(0), zeros(0), zeros(0), XYd=[td, t1])
                        #scope.set_data (t1, t2, zeros(0), XYd=[td, t1])
                        scope.set_data_with_uv (t1, t2, t3, t4)

			return self.run

		def stop_update_plotter (win, event=None):
			print "STOP, hide."
			win.hide()
			self.run = gtk.FALSE
                        save ("plotter_t-"+str(self.t0), self.Time)
                        save ("plotter_t1-"+str(self.t0), self.Tap1)
                        save ("plotter_t2-"+str(self.t0), self.Tap2)
                        save ("plotter_t3-"+str(self.t0), self.Tap3)
                        save ("plotter_t4-"+str(self.t0), self.Tap4)
			return True

		def toggle_run_plotter (b):
			if self.run:
				self.run = gtk.FALSE
				self.run_button.set_label("RUN PLOTTER")
                                save ("plotter_t-"+str(self.t0), self.Time)
                                save ("plotter_t1-"+str(self.t0), self.Tap1)
                                save ("plotter_t2-"+str(self.t0), self.Tap2)
                                save ("plotter_t3-"+str(self.t0), self.Tap3)
                                save ("plotter_t4-"+str(self.t0), self.Tap4)
			else:
				self.run = gtk.TRUE
				self.run_button.set_label("STOP PLOTTER")
                                self.t0 = parent.mk3spm.get_monitor_data (self.monitor_taps[0])
                                self.pos      = 0
				self.span = float (self.Il.get_text())
                                self.dt   = self.span/self.points
                                self.Time = zeros (self.points)
                                self.Tap1 = zeros (self.points)
                                self.Tap2 = zeros (self.points)
                                self.Tap3 = zeros (self.points)
                                self.Tap4 = zeros (self.points)
				gobject.timeout_add (int (self.dt*1000.), update_plotter)


		self.run_button = gtk.Button("STOP TUNE")
		self.run_button.connect("clicked", toggle_run_plotter)
		table.attach(self.run_button, 3, 4, tr+2, tr+3)

		self.run = gtk.FALSE
		win.connect("delete_event", stop_update_plotter)
		toggle_run_plotter (self.run_button)
		
	parent.wins[name].show_all()




################################################################################
# CONFIGURATOR MAIN MENU/WINDOW
################################################################################

class Mk3_Configurator:
    def __init__(self):
	self.mk3spm = SPMcontrol ()
	self.mk3spm.read_configurations ()
	self.vector_index = 0

	buttons = {
		   "1 810 AD/DA Monitor": self.create_A810_ADDA_monitor,
		   "2 SPM Signal Monitor": self.create_signal_monitor,
		   "3 SPM Signal Patch Rack": self.create_signal_patch_rack,
		   "4 SPM Signal+DSP Manager": self.create_signal_manager,
		   "5 SPM Create Signal Graph": self.create_dotviz_graph,
		   "6 SPM Signal Oscilloscope": self.create_oscilloscope_app,
		   "6pSPM Signal Plotter": self.create_signalplotter_app,
		   "7 PLL: Osc. Tune App": self.create_PLL_tune_app,
		   "8 PLL: Ampl. Step App": self.create_amp_step_app,
		   "9 PLL: Phase Step App": self.create_phase_step_app,
##		   "GPIO Control": create_coolrunner_app,
##		   "Rate Meter": create_ratemeter_app,
##		   "FPGA Slider Control": create_coolrunner_slider_control_app,
##		   "FPGA LVDT Stage Control": create_coolrunner_lvdt_stage_control_app,
##		   "SR MK3 DSP Info": create_info,
##		   "SR MK3 DSP SPM Settings": create_settings_edit,
##		   "SR MK3 DSP Reset Peak Load": reset_dsp_load_peak,
#                   "SR MK3 read HR mask": self.mk3spm.read_hr,
#                   "SR MK3 set HR0": self.mk3spm.set_hr_0,
#                   "SR MK3 set HR1": self.mk3spm.set_hr_1,
#                   "SR MK3 set HR1slow": self.mk3spm.set_hr_1slow,
#                   "SR MK3 set HR1slow2": self.mk3spm.set_hr_1slow2,
#                   "SR MK3 set HRs2": self.mk3spm.set_hr_s2,
###                   "SR MK3 read GPIO settings": self.mk3spm.read_gpio,
###                   "SR MK3 write GPIO settings": self.mk3spm.write_gpio,
##                   "SR MK3 read D-FIFO128": readFIFO128,
##                   "SR MK3 read D-FIFO-R": readFIFO,
##                   "SR MK3 read PD-FIFO-R": readPFIFO,
		   "U SCO Config": self.sco_config,
		   "X Vector Index Selector": self.create_index_selector_matrix,
		   "Y User Values": self.create_user_values,
		   "Z SCAN DBG": self.print_dbg,
		 }
	win = gobject.new(gtk.Window,
		     type=gtk.WINDOW_TOPLEVEL,
		     title='SR MK3-Pro SPM Configurator '+version,
		     allow_grow=True,
		     allow_shrink=True,
		     border_width=5)
	win.set_name("main window")
	win.set_size_request(260, 440)
	win.connect("destroy", self.destroy)
	win.connect("delete_event", self.destroy)

	box1 = gobject.new(gtk.VBox(False, 0))
	win.add(box1)
	scrolled_window = gobject.new(gtk.ScrolledWindow())
	scrolled_window.set_border_width(5)
	scrolled_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
	box1.pack_start(scrolled_window)
	box2 = gobject.new(gtk.VBox())
	box2.set_border_width(5)
	scrolled_window.add_with_viewport(box2)

	lab = gtk.Label("SR dev:" + self.mk3spm.sr_path ())
	box2.pack_start(lab)
	lab = gtk.Label(self.mk3spm.get_spm_code_version_info())
	box2.pack_start(lab)

	k = buttons.keys()
	k.sort()
	for i in k:
		button = gobject.new (gtk.Button, label=i)

		if buttons[i]:
			button.connect ("clicked", buttons[i])
		else:
			button.set_sensitive (False)
		box2.pack_start (button)

	separator = gobject.new (gtk.HSeparator())
	box1.pack_start (separator, expand=False)

	box2 = gobject.new (gtk.VBox(spacing=10))
	box2.set_border_width (5)
	box1.pack_start (box2, expand=False)
	button = gtk.Button (stock='gtk-close')
	button.connect ("clicked", self.do_exit)
	button.set_flags (gtk.CAN_DEFAULT)
	box2.pack_start (button)
	button.grab_default ()
	win.show_all ()

	self.wins = {}

    def create_index_selector_matrix(self, b):
		name="Index Selector Matrix"
		if not self.wins.has_key(name):
			win = gobject.new(gtk.Window,
					  type=gtk.WINDOW_TOPLEVEL,
					  title=name,
					  allow_grow=True,
					  allow_shrink=True,
					  border_width=5)
			self.wins[name] = win
			win.connect("delete_event", self.delete_event)

			box1 = gobject.new(gtk.VBox())
			win.add(box1)

			box2 = gobject.new(gtk.VBox(spacing=0))
			box2.set_border_width(5)
			box1.pack_start(box2)

			separator = gobject.new (gtk.HSeparator())
			box1.pack_start (separator, expand=False)

			# 8x8 matrix selector
			table = gtk.Table (8,8)
			table.set_row_spacings(1)
			table.set_col_spacings(1)
			box1.pack_start(table, expand=False)
			
			for i in range (0,8):
				for j in range (0,8):
					ij = 8*i+j
					button = gtk.Button ("%d"%ij)
					button.connect ("clicked", self.do_set_index, ij)
					table.attach (button, i, i+1, j, j+1)

			self.offsetindicator = gtk.Label("Global Vector Index is [%d]"%self.vector_index)
			box1.pack_start(self.offsetindicator, expand=False)

		self.wins[name].show_all()

    def do_set_user_input(self, b, i, u_gettext):
            self.mk3spm.write_signal_by_name ("user signal array", float(u_gettext ()), i)
                
    def create_user_values(self, b):
		name="User Values Array"
		if not self.wins.has_key(name):
			win = gobject.new(gtk.Window,
					  type=gtk.WINDOW_TOPLEVEL,
					  title=name,
					  allow_grow=True,
					  allow_shrink=True,
					  border_width=5)
			self.wins[name] = win
			win.connect("delete_event", self.delete_event)

			box1 = gobject.new(gtk.VBox())
			win.add(box1)

			box2 = gobject.new(gtk.VBox(spacing=0))
			box2.set_border_width(5)
			box1.pack_start(box2)

			separator = gobject.new (gtk.HSeparator())
			box1.pack_start (separator, expand=False)

			# 8x8 matrix selector
			table = gtk.Table (8,8)
			table.set_row_spacings(1)
			table.set_col_spacings(1)
			box1.pack_start(table, expand=False)
			
                        for j in range (0,2):
                                for i in range (0,16):
					ij = 16*j+i
                                        ui = gtk.Entry()
                                        [u,uv, sig] = self.mk3spm.read_signal_by_name ("user signal array", ij)
                                        ui.set_text("%f %s"%(uv, sig[SIG_UNIT]))
                                        table.attach (ui, 2*j+1, 2*j+2, i, i+1, gtk.FILL | gtk.EXPAND )

					button = gtk.Button ("Set User Value [%d]"%ij)
					button.connect ("clicked", self.do_set_user_input, ij, ui.get_text)
					table.attach (button, 2*j+0, 2*j+1, i, i+1)

		self.wins[name].show_all()

    def print_dbg(self, b):
            def update ():
                    self.mk3spm.check_dsp_scan ()
                    return TRUE
            
            gobject.timeout_add(200, update)

    def do_set_index(self, b, ij):
	    self.vector_index = ij
	    self.offsetindicator.set_text("Global Vector Index is [%d]"%self.vector_index)

    def delete_event(self, win, event=None):
	    win.hide()
	    # don't destroy window -- just leave it hidden
	    return True

    def do_exit (self, button):
	    gtk.main_quit()

    def destroy (self, *args):
	    gtk.main_quit()
	
    # update DSP setting parameter
    def int_param_update(self, _adj):
	    param_address = _adj.get_data("int_param_address")
	    param = int (_adj.get_data("int_param_multiplier")*_adj.value)
	    self.mk3spm.write_parameter (param_address, struct.pack ("<l", param), 1) 

    def make_menu_item(self, name, callback, value, identifier, func=lambda:0):
	    item = gtk.MenuItem(name)
	    item.connect("activate", callback, value, identifier, func)
	    item.show()
	    return item

    def toggle_flag(self, w, mask):
	    self.mk3spm.adjust_statemachine_flag(mask, w.get_active())


    ##### configurator tools
    def create_dotviz_graph (self, _button):
	    vis = Visualize (self)
	    
    def create_oscilloscope_app (self, _button):
	    kao = SignalScope (self)

    def create_signalplotter_app (self, _button):
	    sigplotter = SignalPlotter (self)
	
    def create_PLL_tune_app (self, _button):
	    tune = TuneScope (self)

    def create_amp_step_app (self, _button):
	    as_scope = StepResponse(self, 0.05, "Amp")

    def create_phase_step_app (self, _button):
	    ph_scope = StepResponse(self, 1., "Phase")

    def scoset (self, button):
            print self.s_offset.get_text()
            print self.fsens.get_text()
            sc   = round (self.sco_s_offsetQ * float(self.s_offset.get_text()) )
            freq = float (self.fsens.get_text())
            fsv  = round (freq*self.sco_sensQ)    # CPN31 * 2pi freq / 150000.
            print sc, fsv
            scodata = self.mk3spm.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), fmt_sco1)
            if scodata[11] == 0:
                    # (Sin - Sc) * fsv
                    self.mk3spm.write_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), struct.pack("<ll", sc, fsv))
            else:
                    CPN31 = 2147483647
                    deltasRe = round (CPN31 * math.cos (2.*math.pi*freq/150000.))
                    deltasIm = round (CPN31 * math.sin (2.*math.pi*freq/150000.))

                    print "deltasRe/Im="
                    print deltasRe, deltasIm
#                    deltasIm = round (freq*89953.58465492555767605362)   # CPN31 * 2pi f / 150000. 
#                    tmp=deltasIm*deltasIm
#                    print tmp
#                    tmp=long(tmp)>>32
#                    print tmp
#                    deltasRe = CPN31 - tmp
#                    print "deltasRe/Im approx="
#                    print deltasRe, deltasIm

                    self.mk3spm.write_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), struct.pack("<llll", sc, fsv, deltasRe, deltasIm))
                    self.mk3spm.write_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1) + 4*4, struct.pack("<ll", 2147483647, 0)) # reset
                    print "SCO="

            scodata = self.mk3spm.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), fmt_sco1)
            print scodata

    def scodbg (self, button):
            scodata = self.mk3spm.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), fmt_sco1)
            print scodata

    def scoman (self, button):
            scodata = self.mk3spm.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), fmt_sco1)
            if scodata[11] > 0:
                    self.mk3spm.write_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1) + 4*11, struct.pack("<l", 0)) # auto generator mode
            else:
                    self.mk3spm.write_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1) + 4*11, struct.pack("<l", 1)) # manual generator mode
            scodata = self.mk3spm.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), fmt_sco1)
            print scodata

    def sco_read (self, button, id=0):
            self.scoid=id
            [c, fsv] = self.mk3spm.read_o(i_sco1, self.scoid * struct.calcsize(fmt_sco1), "<ll")
            self.s_offset.set_text(str(c/self.sco_s_offsetQ))
            self.fsens.set_text(str(fsv/self.sco_sensQ))
            self.scoidlab.set_label('SCO '+str(id+1))
            
    def sco_config (self, _button):
            name="SCO Config"
            if not self.wins.has_key(name):
                    win = gobject.new(gtk.Window,
                                      type=gtk.WINDOW_TOPLEVEL,
                                      title=name,
                                      allow_grow=True,
                                      allow_shrink=True,
                                      border_width=5)
                    self.wins[name] = win
                    win.connect("delete_event", self.delete_event)
                    
                    grid = gobject.new(gtk.VBox())
                    win.add(grid)

                    self.scoidlab = gobject.new(gtk.Label, label="SCO ?")
                    grid.add(self.scoidlab)

                    self.sco_sensQ     = 89953.58465492555767605362/1024.
                    self.sco_s_offsetQ = 2147483648./10.
                    
                    [c, fsv] = self.mk3spm.read(i_sco1, "<ll")
                    
                    self.cl0 = gobject.new(gtk.Label, label="Sig Offset [V]:")
                    grid.add(self.cl0)
                    self.s_offset = gtk.Entry()
                    self.s_offset.set_text(str(c/self.sco_s_offsetQ))
                    grid.add (self.s_offset)

                    self.fs0 = gobject.new(gtk.Label, label="F Sens [Hz/V]:")
                    grid.add(self.fs0)
                    self.fsens = gtk.Entry()
                    self.fsens.set_text(str(fsv/self.sco_sensQ))
                    grid.add (self.fsens)

                    self.sco_read (0);
                    
                    bset = gtk.Button(stock='set')
                    bset.connect("clicked", self.scoset)
                    grid.add(bset)

                    bsco1 = gtk.Button(stock='Select SCO 1')
                    bsco1.connect("clicked", self.sco_read, 0)
                    grid.add(bsco1)
                    bsco2 = gtk.Button(stock='Select SCO 2')
                    bsco2.connect("clicked", self.sco_read, 1)
                    grid.add(bsco2)

                    bdbg = gtk.Button(stock='debug')
                    bdbg.connect("clicked", self.scodbg)
                    grid.add(bdbg)

                    bman = gtk.Button(stock='toggle man/auto')
                    bman.connect("clicked", self.scoman)
                    grid.add(bman)

                    button = gtk.Button(stock='gtk-close')
                    button.connect("clicked", lambda w: win.hide())
                    grid.add(button)
                    button.set_flags(gtk.CAN_DEFAULT)
                    button.grab_default()

                    self.wins[name].show_all()

                    
                    
    # Vector index or signal variable offset from signal head pointer
    def global_vector_index (self):
	    return self.vector_index

    def make_signal_selector (self, _input_id, sig, prefix, flag_nullptr_ok=0):
		opt = gtk.OptionMenu()
		menu = gtk.Menu()
		i = 0
		i_actual_signal = -1 # for anything not OK
		item = self.make_menu_item("???", self.mk3spm.change_signal_input, sig, -1)
		menu.append(item)
		SIG_LIST = self.mk3spm.get_signal_lookup_list()
		for signal in SIG_LIST:
			if signal[SIG_ADR] > 0:
				item = self.make_menu_item(prefix+"S%02d:"%signal[SIG_INDEX]+signal[SIG_NAME], self.mk3spm.change_signal_input, signal, _input_id, self.global_vector_index)
				menu.append(item)
				if signal[SIG_ADR] == sig[SIG_ADR]:
					i_actual_signal=i
				i=i+1
		if flag_nullptr_ok:
			item = self.make_menu_item("DISABLED", self.mk3spm.disable_signal_input, 0, _input_id, self.global_vector_index)
			menu.append(item)

		if i_actual_signal == -1:
			i_actual_signal = i

		menu.set_active(i_actual_signal+1)
		opt.set_menu(menu)
		opt.show()
		return [opt, menu]

    def update_signal_menu_from_signal (self, menu, tap):
	    [v, uv, signal] = self.mk3spm.get_monitor_signal (tap)
	    if signal[SIG_INDEX] <= 0:
		    menu.set_active(0)
		    return 1
	    menu.set_active(signal[SIG_INDEX]+1)
	    return 1

    # build offset editor dialog
    def create_A810_ADDA_monitor(self, _button):
		name="A810 AD/DA Monitor"
		if not self.wins.has_key(name):
			win = gobject.new(gtk.Window,
					  type=gtk.WINDOW_TOPLEVEL,
					  title=name,
					  allow_grow=True,
					  allow_shrink=True,
					  border_width=5)
			self.wins[name] = win
			win.connect("delete_event", self.delete_event)

			box1 = gobject.new(gtk.VBox())
			win.add(box1)

			box2 = gobject.new(gtk.VBox(spacing=0))
			box2.set_border_width(5)
			box1.pack_start(box2)

			table = gtk.Table (6, 17)
			table.set_row_spacings(5)
			table.set_col_spacings(4)
			box2.pack_start(table, False, False, 0)

			r=0
			lab = gobject.new(gtk.Label, label="A810 Channels")
			table.attach (lab, 0, 1, r, r+1)

			lab = gobject.new(gtk.Label, label="AD-IN Reading")
			lab.set_alignment(1.0, 0.5)
			table.attach (lab, 2, 3, 0, 1, gtk.FILL | gtk.EXPAND )

			lab = gobject.new(gtk.Label, label="DA-OUT Reading")
			lab.set_alignment(1.0, 0.5)
			table.attach (lab, 6, 7, r, r+1, gtk.FILL | gtk.EXPAND )

			level_max = 10000. # mV
			level_fac = 0.305185095 # mV/Bit

			r = r+1
			separator = gobject.new(gtk.HSeparator())
			separator.show()
			table.attach (separator, 0, 7, r, r+1, gtk.FILL | gtk.EXPAND )

			r = r+1
			# create table for CH0..7 in/out reading, hook tabs
			for tap in range(0,8):
				lab = gobject.new(gtk.Label, label="CH-"+str(tap))
				table.attach (lab, 0, 1, r, r+1)

				labin = gobject.new (gtk.Label, label="+####.# mV")
				labin.set_alignment(1.0, 0.5)
				table.attach (labin, 2, 3, r, r+1, gtk.FILL | gtk.EXPAND )

				labout = gobject.new (gtk.Label, label="+####.# mV")
				labout.set_alignment(1.0, 0.5)
				table.attach (labout, 6, 7, r, r+1, gtk.FILL | gtk.EXPAND )

				gobject.timeout_add(timeout_update_A810_readings, self.A810_reading_update, labin.set_text, labout.set_text, tap)
				r = r+1

			separator = gobject.new(gtk.HSeparator())
			box1.pack_start(separator, expand=False)
			separator.show()

			box2 = gobject.new(gtk.VBox(spacing=10))
			box2.set_border_width(10)
			box1.pack_start(box2, expand=False)

			button = gtk.Button(stock='gtk-close')
			button.connect("clicked", lambda w: win.hide())
			box2.pack_start(button)
			button.set_flags(gtk.CAN_DEFAULT)
			button.grab_default()

		self.wins[name].show_all()

    # LOG "VU" Meter
    def create_meterVU(self, w, tap, fsk):
	    [value, uv, signal] = self.mk3spm.get_monitor_signal (tap)
	    label = signal[SIG_NAME]
	    unit  = signal[SIG_UNIT]
	    name="meterVU-"+label
	    if not self.wins.has_key(name):
		    win = gobject.new(gtk.Window,
				      type=gtk.WINDOW_TOPLEVEL,
				      title=label,
				      allow_grow=True,
				      allow_shrink=True,
				      border_width=0)
		    self.wins[name] = win
		    win.connect("delete_event", self.delete_event)
		    v = gobject.new(gtk.VBox(spacing=0))
		    instr = Instrument( gobject.new(gtk.Label), v, "VU", label)
		    instr.show()
		    win.add(v)

		    def update_meter(inst, _tap, signal, fm):
#			    [value, v, signal] = self.mk3spm.get_monitor_signal (_tap)
			    value = self.mk3spm.get_monitor_data (_tap)


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

			    return gtk.TRUE

		    gobject.timeout_add (timeout_update_meter_readings, update_meter, instr, tap, signal, fsk)
	    self.wins[name].show_all()

    # Linear Meter
    def create_meterLinear(self, w, tap, fsk):
	    [value, uv, signal] = self.mk3spm.get_monitor_signal (tap)
	    label = signal[SIG_NAME]
	    unit  = signal[SIG_UNIT]
	    name="meterLinear-"+label
	    if not self.wins.has_key(name):
		    win = gobject.new(gtk.Window,
				      type=gtk.WINDOW_TOPLEVEL,
				      title=label,
				      allow_grow=True,
				      allow_shrink=True,
				      border_width=0)
		    self.wins[name] = win
		    win.connect("delete_event", self.delete_event)
		    v = gobject.new(gtk.VBox(spacing=0))
		    try:
			    float (fsk ())
			    maxv = float (fsk ())
		    except ValueError:
			    maxv = (1<<31)*math.fabs(signal[SIG_D2U])

		    instr = Instrument( gobject.new(gtk.Label), v, "Volt", label, unit)
		    instr.set_range(arange(0,maxv/10*11,maxv/10))
		    instr.show()
		    win.add(v)

		    def update_meter(inst, _tap, signal, fm):
#			    [value, v, signal] = self.mk3spm.get_monitor_signal (_tap)
			    value = self.mk3spm.get_monitor_data (_tap)
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
			    return gtk.TRUE

		    gobject.timeout_add (timeout_update_meter_readings, update_meter, instr, tap, signal, fsk)
	    self.wins[name].show_all()

    def update_signal_menu_from_signal (self, menu, tap):
	    [v, uv, signal] = self.mk3spm.get_monitor_signal(tap)
	    if signal[SIG_INDEX] <= 0:
		    menu.set_active(0)
		    return 1
	    menu.set_active(signal[SIG_INDEX]+1)
	    return 1

    def create_signal_monitor(self, _button):
		name="SPM Signal Monitor"
		if not self.wins.has_key(name):
			win = gobject.new(gtk.Window,
					  type=gtk.WINDOW_TOPLEVEL,
					  title=name,
					  allow_grow=True,
					  allow_shrink=True,
					  border_width=2)
			self.wins[name] = win
			win.connect("delete_event", self.delete_event)

			box1 = gobject.new(gtk.VBox())
			win.add(box1)

			box2 = gobject.new(gtk.VBox(spacing=0))
			box2.set_border_width(5)
			box1.pack_start(box2)

			table = gtk.Table (11, 37)
			table.set_row_spacings(0)
			table.set_col_spacings(4)
			box2.pack_start(table, False, False, 0)

			r=0
			lab = gobject.new(gtk.Label, label="Signal to Monitor")
			table.attach (lab, 0, 1, r, r+1)

			lab = gobject.new(gtk.Label, label="DSP Signal Variable name")
			table.attach (lab, 1, 2, r, r+1)

			lab = gobject.new(gtk.Label, label="DSP Sig Reading")
			lab.set_alignment(1.0, 0.5)
			table.attach (lab, 2, 3, r, r+1, gtk.FILL | gtk.EXPAND )

			lab = gobject.new(gtk.Label, label="Value")
			lab.set_alignment(1.0, 0.5)
			table.attach (lab, 6, 7, r, r+1, gtk.FILL | gtk.EXPAND )

			lab = gobject.new(gtk.Label, label="dB")
			lab.set_alignment(1.0, 0.5)
			table.attach (lab, 7, 8, r, r+1, gtk.FILL | gtk.EXPAND )
			lab = gobject.new(gtk.Label, label="V")
			lab.set_alignment(1.0, 0.5)
			table.attach (lab, 8, 9, r, r+1, gtk.FILL | gtk.EXPAND )

			lab = gobject.new(gtk.Label, label="Galvo Range")
			lab.set_alignment(1.0, 0.5)
			table.attach (lab, 9, 10, r, r+1, gtk.FILL | gtk.EXPAND )

			r = r+1
			separator = gobject.new(gtk.HSeparator())
			separator.show()
			table.attach (separator, 0, 7, r, r+1, gtk.FILL | gtk.EXPAND )

			r = r+1
			# create full signal monitor
			for tap in range(0,num_monitor_signals):
				[value, uv, signal] = self.mk3spm.get_monitor_signal (tap)
				if signal[SIG_INDEX] < 0:
					break
				[lab1, menu1] = self.make_signal_selector (DSP_SIGNAL_MONITOR_INPUT_BASE_ID+tap, signal, "Mon[%02d]: "%tap)
				lab1.set_alignment(-1.0, 0.5)
				table.attach (lab1, 0, 1, r, r+1)

				gobject.timeout_add (timeout_update_signal_monitor_menu, self.update_signal_menu_from_signal, menu1, tap)

				lab2 = gobject.new(gtk.Label, label=signal[SIG_VAR])
				lab2.set_alignment(-1.0, 0.5)
				table.attach (lab2, 1, 2, r, r+1)

				labv = gobject.new(gtk.Label, label=str(value))
				labv.set_alignment(1.0, 0.5)
				table.attach (labv, 2, 3, r, r+1)

				labsv = gobject.new (gtk.Label, label="+####.# mV")
				labsv.set_alignment(1.0, 0.5)
				table.attach (labsv, 6, 7, r, r+1, gtk.FILL | gtk.EXPAND )
				gobject.timeout_add (timeout_update_signal_monitor_reading, self.signal_reading_update, lab2.set_text, labv.set_text, labsv.set_text, tap)

				full_scale = gtk.Entry()
				full_scale.set_text("auto")
				table.attach (full_scale, 9, 10, r, r+1, gtk.FILL | gtk.EXPAND )

				image = gtk.Image()
				if os.path.isfile("meter-icondB.png"):
				    imagefile="meter-icondB.png"
				else:
				    imagefile="/usr/share/gxsm/pixmaps/meter-icondB.png"
				image.set_from_file(imagefile)
				image.show()
				button = gtk.Button()
				button.add(image)
				button.connect("clicked", self.create_meterVU, tap, full_scale.get_text)
				table.attach (button, 7, 8, r, r+1, gtk.FILL | gtk.EXPAND )

				image = gtk.Image()
				if os.path.isfile("meter-iconV.png"):
				    imagefile="meter-iconV.png"
				else:
				    imagefile="/usr/share/gxsm/pixmaps/meter-iconV.png"
				image.set_from_file(imagefile)
				image.show()
				button = gtk.Button()
				button.add(image)
				button.connect("clicked", self.create_meterLinear, tap, full_scale.get_text)
				table.attach (button, 8, 9, r, r+1, gtk.FILL | gtk.EXPAND )

				

				r = r+1

			separator = gobject.new(gtk.HSeparator())
			box1.pack_start(separator, expand=False)
			separator.show()

			box2 = gobject.new(gtk.VBox(spacing=10))
			box2.set_border_width(10)
			box1.pack_start(box2, expand=False)

			button = gtk.Button(stock='gtk-close')
			button.connect("clicked", lambda w: win.hide())
			box2.pack_start(button)
			button.set_flags(gtk.CAN_DEFAULT)
			button.grab_default()

		self.wins[name].show_all()

    def update_signal_menu_from_mod_inp(self, _lab, mod_inp):
		[signal, data, offset] = self.mk3spm.query_module_signal_input(mod_inp)
		if offset > 0:
			_lab (" == "+signal[SIG_VAR]+" [%d] ==> "%(offset))
		else:
			_lab (" == "+signal[SIG_VAR]+" ==> ")
		return 1

    # Build MK3-A810 FB-SPMCONTROL Signal Patch Rack
    def create_signal_patch_rack(self, _button):
		name="SPM Patch Rack"
		if not self.wins.has_key(name):
			win = gobject.new(gtk.Window,
					  type=gtk.WINDOW_TOPLEVEL,
					  title=name,
					  allow_grow=True,
					  allow_shrink=True,
					  border_width=2)
			self.wins[name] = win
			win.connect("delete_event", self.delete_event)
			win.set_size_request(720, 500)

			box1 = gobject.new(gtk.VBox(False, 0))
			win.add(box1)
			scrolled_window = gobject.new(gtk.ScrolledWindow())
			scrolled_window.set_border_width(3)
			scrolled_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
			box1.pack_start(scrolled_window)
			scrolled_window.show()
			box2 = gobject.new(gtk.VBox())
			box2.set_border_width(3)
			scrolled_window.add_with_viewport(box2)
			box1.pack_start(box2)

			table = gtk.Table (6, 17)
			table.set_row_spacings(0)
			table.set_col_spacings(4)
			box2.pack_start(table, False, False, 0)

			r=0
			lab = gobject.new(gtk.Label, label="SIGNAL")
			table.attach (lab, 0, 1, r, r+1, gtk.FILL | gtk.EXPAND )

			lab = gobject.new(gtk.Label, label="--- variable --->")
			table.attach (lab, 1, 2, r, r+1)

			lab = gobject.new(gtk.Label, label="MODULE INPUT")
			lab.set_alignment(0.5, 0.5)
			table.attach (lab, 2, 3, r, r+1)

			r = r+1
			separator = gobject.new(gtk.HSeparator())
			separator.show()
			table.attach (separator, 0, 7, r, r+1, gtk.FILL | gtk.EXPAND )

			r = r+1
			# create full signal monitor
			for mod in DSP_MODULE_INPUT_ID_CATEGORIZED.keys():
				for mod_inp in DSP_MODULE_INPUT_ID_CATEGORIZED[mod]:
					if mod_inp[0] > 0: # INPUT_ID_VALID
						[signal, data, offset] = self.mk3spm.query_module_signal_input(mod_inp[0])
						if mod_inp[2] >= 0: # address valid, zero allowed in special cases if mod_inp[3] == 1!
							# lab = gobject.new(gtk.Label, label=signal[SIG_NAME])
							# lab.set_alignment(-1.0, 0.5)	
							# table.attach (lab1, 0, 1, r, r+1)
							
							prefix = "Sig: "
							[lab, menu] = self.make_signal_selector (mod_inp[0], signal, prefix, mod_inp[3])
							lab.set_alignment(-1.0, 0.5)
							table.attach (lab, 0, 1, r, r+1)

							lab = gobject.new(gtk.Label, label=" == "+signal[SIG_VAR]+" ==> ")
							#					lab = gobject.new(gtk.Label, label=" ===> ")
							lab.set_alignment(-1.0, 0.5)	
							table.attach (lab, 1, 2, r, r+1)
							gobject.timeout_add (timeout_update_patchrack, self.update_signal_menu_from_mod_inp, lab.set_text, mod_inp[0])

							lab = gobject.new(gtk.Label, label=mod_inp[1])
							lab.set_alignment(-1.0, 0.5)	
							table.attach (lab, 2, 3, r, r+1)

							r = r+1

			separator = gobject.new(gtk.HSeparator())
			box1.pack_start(separator, expand=False)
			separator.show()

			box2 = gobject.new(gtk.VBox(spacing=5))
			box2.set_border_width(10)
			box1.pack_start(box2, expand=False)

			button = gtk.Button(stock='gtk-close')
			button.connect("clicked", lambda w: win.hide())
			box2.pack_start(button)
			button.set_flags(gtk.CAN_DEFAULT)
			button.grab_default()

		self.wins[name].show_all()

    # Build MK3-A810 FB-SPMCONTROL Signal Manger / FLASH SUPPORT
    def create_signal_manager(self, _button, flashdbg=1):
		name="SPM Signal and DSP Core Manager"
		if not self.wins.has_key(name):
			win = gobject.new(gtk.Window,
					  type=gtk.WINDOW_TOPLEVEL,
					  title=name,
					  allow_grow=True,
					  allow_shrink=True,
					  border_width=2)
			self.wins[name] = win
			win.connect("delete_event", self.delete_event)
			win.set_size_request(720, 250)

			box1 = gobject.new(gtk.VBox(False, 0))
			win.add(box1)

##			scrolled_window = gobject.new(gtk.ScrolledWindow())
##			scrolled_window.set_border_width(3)
##			scrolled_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
##			box1.pack_start(scrolled_window)
##			scrolled_window.show()
##			box2 = gobject.new(gtk.VBox())
##			box2.set_border_width(3)
##			scrolled_window.add_with_viewport(box2)
##			box1.pack_start(box2)

##			table = gtk.Table (6, 17)
##			table.set_row_spacings(0)
##			table.set_col_spacings(4)
##			box2.pack_start(table, False, False, 0)
##			separator = gobject.new(gtk.HSeparator())
##			box1.pack_start(separator, expand=False)
##			separator.show()

			box2 = gobject.new(gtk.VBox(spacing=5))
			box2.set_border_width(10)
			box1.pack_start(box2, expand=False)

			button = gtk.Button("REVERT TO POWER UP DEFAULTS")
			button.connect("clicked", self.mk3spm.disable_signal_input, 0, 0, 0) # REVERT TO POWER-UP-DEFAULT
			box2.pack_start(button)

			hb = gobject.new(gtk.HBox(spacing=10))
			box2.pack_start(hb)
			button = gtk.Button("SAVE DSP-CONFIG TO FILE")
			button.connect("clicked",  self.mk3spm.read_and_save_actual_module_configuration, "mk3_signal_configuration.pkl") # store to file
			hb.pack_start(button)

			button = gtk.Button("LOAD DSP-CONFIG FROM FILE")
			button.connect("clicked",  self.mk3spm.load_and_write_actual_module_configuration, "mk3_signal_configuration.pkl") # restore from file
			hb.pack_start(button)

                        
			hb = gobject.new(gtk.HBox(spacing=10))
			box2.pack_start(hb)
			button = gtk.Button("STORE TO FLASH")
			button.connect("clicked", self.mk3spm.manage_signal_configuration, DSP_SIGNAL_TABLE_STORE_TO_FLASH_ID)
			hb.pack_start(button)
			
			button = gtk.Button("RESTORE FROM FLASH")
			button.connect("clicked", self.mk3spm.manage_signal_configuration, DSP_SIGNAL_TABLE_RESTORE_FROM_FLASH_ID)
			hb.pack_start(button)
			
			button = gtk.Button("ERASE (INVALIDATE) FLASH TABLE")
			button.connect("clicked", self.mk3spm.manage_signal_configuration, DSP_SIGNAL_TABLE_ERASE_FLASH_ID)
			hb.pack_start(button)
			
			if flashdbg:
				separator = gobject.new(gtk.HSeparator())
				box1.pack_start(separator, expand=False)
				separator.show()
				
				lab = gobject.new(gtk.Label(), label="-- FLASH debug actions enabled --")
				box1.pack_start(lab, expand=False)
				lab.show()
				
				self.mk3spm.flash_dump(0, 2)
				lab = gobject.new(gtk.Label(), label=self.mk3spm.flash_dump(0, 2))
				box1.pack_start(lab, expand=False)
				lab.show()
				
				hb = gobject.new(gtk.HBox(spacing=10))
				box2.pack_start(hb)
				data = gtk.Entry()
				data.set_text("0")
				hb.pack_start(data)
				
				button = gtk.Button("SEEK")
				button.connect("clicked", self.mk3spm.manage_signal_configuration, DSP_SIGNAL_TABLE_SEEKRW_FLASH_ID, data.get_text)
				hb.pack_start(button)
				
				button = gtk.Button("READ")
				button.connect("clicked", self.mk3spm.manage_signal_configuration, DSP_SIGNAL_TABLE_READ_FLASH_ID)
				hb.pack_start(button)
				
				button = gtk.Button("WRITE")
				button.connect("clicked", self.mk3spm.manage_signal_configuration, DSP_SIGNAL_TABLE_WRITE_FLASH_ID, data.get_text)
				hb.pack_start(button)
				
				button = gtk.Button("TEST0")
				button.connect("clicked", self.mk3spm.manage_signal_configuration, DSP_SIGNAL_TABLE_TEST0_FLASH_ID)
				hb.pack_start(button)
				
				button = gtk.Button("TEST1")
				button.connect("clicked", self.mk3spm.manage_signal_configuration, DSP_SIGNAL_TABLE_TEST1_FLASH_ID)
				hb.pack_start(button)
				
				button = gtk.Button("DUMP")
				button.connect("clicked", self.mk3spm.flash_dump)
				hb.pack_start(button)
				

			hb = gobject.new(gtk.HBox(spacing=10))
                        box2.pack_start(hb)
                        data =  gobject.new(gtk.Label, label="DSP CORE MANAGER:")
                        hb.pack_start(data)
                        button = gtk.Button("STATE?")
                        button.connect("clicked", self.mk3spm.print_statemachine_status)
                        hb.pack_start(button)
                        button = gtk.Button("590MHz")
                        button.connect("clicked", self.mk3spm.dsp_adjust_speed, 590)
                        hb.pack_start(button)
                        button = gtk.Button("688MHz")

                        def confirm_688(dummy):
                                label = gtk.Label("Confirm DSP reconfiguration to 688 MHz.\nWARNING\nUse on own risc.")
                                dialog = gtk.Dialog("DSP Clock Reconfiguration",
                                                    None,
                                                    gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                                    (gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                                                     gtk.STOCK_OK, gtk.RESPONSE_ACCEPT))
                                dialog.vbox.pack_start(label)
                                label.show()
                                checkbox = gtk.CheckButton("I agree.")
                                dialog.action_area.pack_end(checkbox)
                                checkbox.show()
                                response = dialog.run()
                                dialog.destroy()
                                
                                print response
                                if response == gtk.RESPONSE_ACCEPT and checkbox.get_active():
                                        self.mk3spm.dsp_adjust_speed(0, 688)
                        
                        button.connect("clicked", confirm_688)
                        hb.pack_start(button)
				
			button = gtk.Button(stock='gtk-close')
			button.connect("clicked", lambda w: win.hide())
			box2.pack_start(button)
			button.set_flags(gtk.CAN_DEFAULT)
			button.grab_default()
                        
		self.wins[name].show_all()



    # updates the right side of the offset dialog
    def A810_reading_update(self, _labin, _labout, tap):
	    tmp = self.mk3spm.get_ADDA ()

	    level_fac = 0.305185095 # mV/Bit
	    Vin  = level_fac * tmp[0][tap]
	    Vout = level_fac * tmp[1][tap]

	    _labin("%+06.3f mV" %Vin)
	    _labout("%+06.3f mV" %Vout)

	    return 1

    # updates the right side of the offset dialog
    def signal_reading_update(self, _sig_var, _labv, _labsv, tap):
	    [value, uv, signal] = self.mk3spm.get_monitor_signal (tap)
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
	    gobject.timeout_add (timeout_DSP_status_reading, self.mk3spm.get_status)	
	    gobject.timeout_add (timeout_DSP_signal_lookup_reading, self.mk3spm.read_signal_lookup, 1)

	    gtk.main()


########################################

print __name__
if __name__ == "__main__":
	mk3 = Mk3_Configurator ()
	mk3.main ()

## END