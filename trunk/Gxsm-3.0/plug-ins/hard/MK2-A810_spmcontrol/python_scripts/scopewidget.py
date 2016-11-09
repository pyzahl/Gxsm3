#!/usr/bin/env python

## * Python Module VU or general Meter/Instrument gtk widget
## *
## * Copyright (C) 2014 Percy Zahl
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

###########################################################################
## The original VU meter is a passive electromechanical device, namely
## a 200 uA DC d'Arsonval movement ammeter fed from a full wave
## copper-oxide rectifier mounted within the meter case. The mass of the
## needle causes a relatively slow response, which in effect integrates
## the signal, with a rise time of 300 ms. 0 VU is equal to +4 [dBu], or
## 1.228 volts RMS across a 600 ohm load. 0 VU is often referred to as "0
## dB".[1] The meter was designed not to measure the signal, but to let
## users aim the signal level to a target level of 0 VU (sometimes
## labelled 100%), so it is not important that the device is non-linear
## and imprecise for low levels. In effect, the scale ranges from -20 VU
## to +3 VU, with -3 VU right in the middle. Purely electronic devices
## may emulate the response of the needle; they are VU-meters inasmuch as
## they respect the standard.

import pygtk
pygtk.require('2.0')
import os		# use os because python IO is bugy

import gobject, gtk
import cairo
import time
import fcntl
from threading import Timer

#import GtkExtra
import struct
import array
import math
from gtk import TRUE, FALSE
from numpy  import *

wins = {}

sr_dev_base = "/dev/sranger"
global sr_dev_path
updaterate = 200

delaylinelength = 32

global SPM_STATEMACHINE
global analog_offset	 # at magic[10]+16
global AIC_gain          # at magic[8]+1+3*8
global AIC_in_buffer	 # at magic[6]

unit  = [ "dB", "dB" ]


class Scope(gtk.DrawingArea):

    def __init__(self, parent):
      
        self.par = parent
        super(Scope, self).__init__()
 
        self.set_size_request(550, 600)
	if os.path.isfile("scope-frame.png"):
		imagefile="scope-frame.png"
	else:
		imagefile="/usr/share/gxsm/pixmaps/scope-frame.png"
        self.vuscopesurface = cairo.ImageSurface.create_from_png(imagefile)
        cr = cairo.Context (self.vuscopesurface)
        cr.set_source_surface(self.vuscopesurface, 0,0)  
        cr.paint()

        cr.save ()

        # center of screen is 0,0
        # full scale is +/-10
        cr.translate (275, 275)
        cr.scale (25., 25.)

        # ticks -10 .. 0 .. +10
        cr.set_source_rgba(0.27, 0.48, 0.69, 0.25) # TICKS
        cr.set_line_width(0.03)

        ticks = arange(-10,11,1)
        for tx in ticks:
            cr.move_to (tx, -10.)
            cr.line_to (tx, 10.)
            cr.stroke()
        for ty in ticks:
            cr.move_to (-10., ty)
            cr.line_to (10., ty)
            cr.stroke()

        cr.set_source_rgba(0.37, 0.58, 0.79, 0.35) # ZERO AXIS
        cr.set_line_width(0.05)
        cr.move_to (0., -10.)
        cr.line_to (0., 10.)
        cr.stroke()
        cr.move_to (-10., 0.)
        cr.line_to (10., 0.)
        cr.stroke()
        
        cr.restore ()

        self.connect("expose-event", self.expose)

    def expose(self, widget, event):
        cr = widget.window.cairo_create()
        cr.set_source_surface (self.vuscopesurface)
        cr.paint()
        cr.stroke()

        cr.save ()

        self.par.count = self.par.count + 1
         
        # center of screen is 0,0
        # full scale is +/-10
        cr.translate (275, 275)
        cr.scale (25., 25.)
        cr.set_line_width(0.04)
        
        alpha = 0.85
        dx = 20. / size(self.par.Xdata)
        x = -10.
        cr.set_source_rgba(1., 0.925, 0., alpha) # YELLOW
        yp = self.par.Xdata[0]
        for y in self.par.Xdata:
            cr.move_to(x, yp)
            x = x+dx
            yp = y
            cr.line_to(x, y)
            cr.stroke()
        
        if size(self.par.Ydata) > 1:
            dx = 20. / size(self.par.Ydata)
            x = -10.
            cr.set_source_rgba(1., 0.075, 0., alpha) # RED
            yp = self.par.Ydata[0]
            for y in self.par.Ydata:
                cr.move_to(x, yp)
                x = x+dx
                yp = y
                cr.line_to(x, y)
                cr.stroke()

        if size(self.par.Zdata) > 1:
            dx = 20. / size(self.par.Zdata)
            x = -10.
            cr.set_source_rgba(0., 1., 0., alpha) # GREEN
            yp = self.par.Zdata[0]
            for y in self.par.Zdata:
                cr.move_to(x, yp)
                x = x+dx
                yp = y
                cr.line_to(x, y)
                cr.stroke()

        if size (self.par.XYdata[0]) > 1:
            cr.set_source_rgba (0., 1., 0., alpha) # GREEN
            xyp = self.par.XYdata[0]
            for xy in self.par.XYdata:
                cr.move_to (xyp[0], xyp[1])
                cr.line_to (xy[0], xy[1])
                cr.stroke ()
                xyp=xy

        cr.restore ()

        x0=25
        x1=150
        x2=250
        x3=350
        x4=450
        x5=525
        ylineOS0 = 45
        yline1 = 560
        yline2 = 575
        yline3 = 590

        cr.select_font_face("Droid Sans") #, cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
        cr.set_font_size(12)
        cr.set_line_width(0.8)

        cr.set_source_rgba(0., 1., 0., 1.) # GREEN

        if self.par.count == 1:
            reading = "Enable/Disable PAC/PLL processing to run/hold scope!"
            cr.move_to(x1, ylineOS0)
            cr.text_path(reading)
            cr.stroke()

        y=0
        for txt in self.par.info:
            if txt == "":
                y=y+2
                continue
            cr.move_to(x0, ylineOS0+y*25)
            cr.text_path(txt)
            cr.stroke()
            y=y+1

        y=0
        for txt in self.par.chinfo:
            if y==0:
                cr.set_source_rgba(1., 0.925, 0., alpha) # YELLOW
            else:
                if y == 1:
                    cr.set_source_rgba(1., 0.075, 0., alpha) # RED
                else:
                    cr.set_source_rgba(0., 1., 0., alpha) # GREEN
            reading = txt + ": CH%d"%(y+1)
            (rx, ry, width, height, dx, dy) = cr.text_extents(reading)
            cr.move_to(x5-width, ylineOS0+y*25)
            cr.text_path(reading)
            cr.stroke()
            y=y+1
            
        cr.set_source_rgba(0.03, 0., 0.97, 1.) # BLUE
            
        # record count
        reading = "#: %d"%self.par.count
        cr.move_to(x1, yline1)
        cr.text_path(reading)
        cr.stroke()

        # CH1: (X-sig) scale/div
        y=yline1
        for s in self.par.scale.keys():
            # (x, y, width, height, dx, dy) = cr.text_extents(reading)
            cr.move_to (x3, y)
            cr.text_path (s)
            cr.stroke ()
            reading = self.par.scale[s]
            (rx, ry, width, height, dx, dy) = cr.text_extents(reading)
            cr.move_to(x5-width, y)
            cr.text_path(reading)
            cr.stroke()
            y=y+15

        
class Oscilloscope(gtk.Label):
    def __init__(self, parent, vb, ft="XT", l="Scope"):
        
        #Initialize the Widget
        gtk.Widget.__init__(self)
        self.set_use_markup(True)
        self.frametype=ft
        self.label = l
        self.info = []
        self.chinfo = []
        self.scale = { 
            "CH1-scale": "1 V/div",
            "CH2-scale": "1 V/div",
            "Timebase":  "20 ms/div" 
            }

        self.Xdata = zeros(2)
        self.Ydata = zeros(0)
        self.Zdata = zeros(0)
        self.XYdata = array([[],[]])
        self.scope = Scope(self)
        self.count = 0
        vb.pack_start(self, False, False, 0)
        vb.pack_start(self.scope, False, False, 0)
        vb.show_all()
        self.show_all()
        
    def set_data (self, Xd, Yd, Zd=zeros(0), XYd=[zeros(0), zeros(0)]):
        self.Xdata = Xd
        self.Ydata = Yd
        self.Zdata = Zd
        self.XYdata = XYd
        self.scope.queue_draw()

    def set_info (self, infolist):
        self.info = infolist
        
    def set_chinfo (self, infolist):
        self.chinfo = infolist
        
    def set_scale (self, s):
        self.scale = s
        
    def set_label (self, str):
        self.label = str

    def queue_draw (self):
        self.scope.queue_draw()
        
        
## END
