#!/usr/bin/env python3

## GXSM NetCDF San Image Viwer and Tag Tool with GXSM remote inspection and control socket client functionality

import sys
import os		# use os because python IO is bugy
import time
import threading
import re
import socket
import json

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gio, Gtk, Gdk, GObject
import cairo


from netCDF4 import Dataset
import struct
import array
import math

import numpy as np
import matplotlib.cm as cm
import matplotlib.pyplot as plt
import matplotlib.cbook as cbook
from matplotlib.path import Path
from matplotlib.patches import PathPatch
from matplotlib.backends.backend_gtk3agg import (FigureCanvasGTK3Agg as FigureCanvas)
from matplotlib.figure import Figure
from matplotlib.backends.backend_gtk3cairo import FigureCanvasGTK3Cairo as FigureCanvas
from matplotlib.backends.backend_gtk3 import NavigationToolbar2GTK3 as NavigationToolbar
from matplotlib.patches import Rectangle
from matplotlib import cm

# Socket Client

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 65432        # The port used by the server


class SocketClient:
    def __init__(self, host, port):
        self.sok = None
        #with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        #   self.sok = client
        self.sok=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sok.connect ((host, port))
        self.send_as_json ({'echo': [{'message': 'Hello GXSM3! Establishing Socket Link.'}]})
        data = self.receive_json ()
        print('Received: ', data)
            
    def __del__(self):
        if not self.sok:
            print ('No connection')
            raise Exception('You have to connect first before receiving data')
        self.sok.close()

    def send_as_json(self, data):
        if not self.sok:
            print ('No connection')
            raise Exception('You have to connect first before receiving data')
        try:
            serialized = json.dumps(data)
        except (TypeError, ValueError):
            raise Exception('You can only send JSON-serializable data')

        print('Sending JSON: N={} D=[{}]'.format(len(serialized.encode('utf-8')),serialized))

        # send the length of the serialized data first
        sd = '{}\n{}'.format(len(serialized), serialized)
        # send the serialized data
        self.sok.sendall(sd.encode('utf-8'))
            
    def request_start_scan(self):
        self.send_as_json({'action': ['start-scan']})
        return self.receive()

    def request_stop_scan(self):
        self.send_as_json({'action': ['stop-scan']})
        return self.receive()

    def request_action(self, id):
        self.send_as_json({'action': [{'id':id}]})
        return self.receive()

    def request_action_v(self, id, value):
        self.send_as_json({'action': [{'id':id, 'value':value}]})
        return self.receive()

    def request_set_parameter(self, id, value):
        self.send_as_json({'command': [{'set': id, 'value': value}]})
        return self.receive()

    def request_get_parameter(self, id):
        self.send_as_json({'command': [{'get': id}]})
        return self.receive()

    def receive(self):
        if not self.sok:
            print ('No connection')
            raise Exception('You have to connect first before receiving data')
        return self.receive_json()

    def receive_json(self):
        print ('receive_json...\n')
        if not self.sok:
            print ('No connection')
            raise Exception('You have to connect first before receiving data')
        # try simple assume one message
        try:
            data = self.sok.recv (1024)
            if data:
                print ('Got Data: {}'.format(data))
                count, jsdata = data.split(b'\n')
                print ('N={} D={}'.format(count,jsdata))
                try:
                    deserialized = json.loads(jsdata)
                    print ('Deserialized: {}'.format(deserialized))
                    return deserialized
                except (TypeError, ValueError):
                    deserialized = json.loads({'JSON-Deserialize-Error'})
                    raise Exception('Data received was not in JSON format')
                    return deserialized
            else:
                pass
        except:
            pass
        
    def receive_json_long(self, socket):
        # read the length of the data, letter by letter until we reach EOL
        length_str = b''
        print ('Waiting for response.\n')
        char = self.sok.recv(1)
        while char != '\n':
            length_str += char
            char = self.sok.recv(1)
        total = int(length_str)
        print('receiving json bytes # ', total, ' [', length_str, ']\n')
        # use a memoryview to receive the data chunk by chunk efficiently
        view = memoryview(bytearray(total))
        next_offset = 0
        while total - next_offset > 0:
            recv_size = self.sok.recv_into(view[next_offset:], total - next_offset)
            next_offset += recv_size
        try:
            deserialized = json.loads(view.tobytes())
        except (TypeError, ValueError):
            raise Exception('Data received was not in JSON format')

        print('received JSON: {}\n', deserialized)
        
        return deserialized
    
    def recv_and_close(self):
        data = self.receive()
        self.close()
        return data
    
    def close(self):
        if self.s:
            self.sok.close()
            self.sok = None

    
# Application stuff
MENU_XML="""
<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <menu id="app-menu">
    <section>
      <attribute name="label" translatable="yes">Change label</attribute>
      <item>
        <attribute name="action">win.change_label</attribute>
        <attribute name="target">Unkown</attribute>
        <attribute name="label" translatable="yes">Unkown</attribute>
      </item>
      <item>
        <attribute name="action">win.change_label</attribute>
        <attribute name="target">Flat</attribute>
        <attribute name="label" translatable="yes">Flat</attribute>
      </item>
      <item>
        <attribute name="action">win.change_label</attribute>
        <attribute name="target">Lattice</attribute>
        <attribute name="label" translatable="yes">Lattice</attribute>
      </item>
      <item>
        <attribute name="action">win.change_label</attribute>
        <attribute name="target">Step</attribute>
        <attribute name="label" translatable="yes">Step</attribute>
      </item>
      <item>
        <attribute name="action">win.change_label</attribute>
        <attribute name="target">Step Bunch</attribute>
        <attribute name="label" translatable="yes">Step Bunch</attribute>
      </item>
      <item>
        <attribute name="action">win.change_label</attribute>
        <attribute name="target">Tiny Bump</attribute>
        <attribute name="label" translatable="yes">Tiny Bump</attribute>
      </item>
      <item>
        <attribute name="action">win.change_label</attribute>
        <attribute name="target">Danger</attribute>
        <attribute name="label" translatable="yes">Danger</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="action">win.maximize</attribute>
        <attribute name="label" translatable="yes">Maximize</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name="action">app.about</attribute>
        <attribute name="label" translatable="yes">_About</attribute>
      </item>
      <item>
        <attribute name="action">app.quit</attribute>
        <attribute name="label" translatable="yes">_Quit</attribute>
        <attribute name="accel">&lt;Primary&gt;q</attribute>
    </item>
    </section>
  </menu>
</interface>
"""

class AppWindow(Gtk.ApplicationWindow):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.gxsm = None
        self.cdf_image_data_filename = ''
        self.connect("destroy", self.quit)

        # This will be in the windows group and have the "win" prefix
        max_action = Gio.SimpleAction.new_stateful("maximize", None,
                                           GLib.Variant.new_boolean(False))
        max_action.connect("change-state", self.on_maximize_toggle)
        self.add_action(max_action)

        # Keep it in sync with the actual state
        self.connect("notify::is-maximized",
                            lambda obj, pspec: max_action.set_state(
                                               GLib.Variant.new_boolean(obj.props.is_maximized)))
        grid = Gtk.Grid()
        self.gridwidget = grid
        self.add(grid)

        lbl_variant = GLib.Variant.new_string("Label")
        lbl_action = Gio.SimpleAction.new_stateful("change_label", lbl_variant.get_type(),
                                               lbl_variant)
        lbl_action.connect("change-state", self.on_change_label_state)
        self.add_action(lbl_action)

        self.label = Gtk.Label(label=lbl_variant.get_string(),
                               margin=30)


        grid.attach(Gtk.Label(label='Category'),0,49,2,1)
        grid.attach(self.label,0,50,2,1)

        y=0
        button1 = Gtk.Button(label="Open NetCDF")
        button1.connect("clicked", self.on_file_clicked)
        grid.attach(button1, 0, y, 2, 1)
        y=y+1
        button2 = Gtk.Button(label="Set Folder")
        button2.connect("clicked", self.on_folder_clicked)
        grid.attach(button2, 0, y, 2, 1)
        y=y+1
        button3 = Gtk.Button(label="List NetCDF")
        button3.connect("clicked", self.list_CDF_clicked)
        grid.attach(button3, 0, y, 2, 1)
        y=y+1
        button4 = Gtk.Button(label="Reload")
        button4.connect("clicked", self.reload_clicked)
        grid.attach(button4, 0, y, 2, 1)
        y=y+1
        button5 = Gtk.Button(label="Next #")
        button5.connect("clicked", self.next_clicked)
        grid.attach(button5, 0, y, 2, 1)
        y=y+1
        button6 = Gtk.Button(label="GXSM ! Live")
        button6.connect("clicked", self.live_clicked)
        grid.attach(button6, 0, y, 2, 1)
        y=y+1
        button66 = Gtk.Button(label="GXSM ! Get NX")
        button66.connect("clicked", self.getNX_clicked)
        grid.attach(button66, 0, y, 2, 1)
        y=y+1
        button7 = Gtk.Button(label="GXSM ! Start")
        button7.connect("clicked", self.start_clicked)
        grid.attach(button7, 0, y, 2, 1)
        y=y+1
        button8 = Gtk.Button(label="GXSM ! Stop")
        button8.connect("clicked", self.stop_clicked)
        grid.attach(button8, 0, y, 2, 1)
        y=y+1

        buttonQ = Gtk.Button(label="P: Quick")
        buttonQ.connect("clicked", self.process_quick_clicked)
        grid.attach(buttonQ, 0, y, 2, 1)
        y=y+1

        button2x2 = Gtk.Button(label="2x2")
        button2x2.connect("clicked", self.subdivide2x2_clicked)
        grid.attach(button2x2, 0, y, 2, 1)
        y=y+1
        button3x3 = Gtk.Button(label="3x3")
        button3x3.connect("clicked", self.subdivide3x3_clicked)
        grid.attach(button3x3, 0, y, 2, 1)
        y=y+1
        button4x4 = Gtk.Button(label="4x4")
        button4x4.connect("clicked", self.subdivide4x4_clicked)
        grid.attach(button4x4, 0, y, 2, 1)
        y=y+1
        button5x5 = Gtk.Button(label="5x5")
        button5x5.connect("clicked", self.subdivide5x5_clicked)
        grid.attach(button5x5, 0, y, 2, 1)
        y=y+1

        y=10
        l = Gtk.Label(label="Parameters:")
        grid.attach(l, 0, y, 2, 1)
        y=y+1
        l = Gtk.Label(label="Bias")
        grid.attach(l, 0, y, 1, 1)
        self.bias = Gtk.Label(label="--")
        grid.attach(self.bias, 1, y, 1, 1)
        y=y+1
        l = Gtk.Label(label="Current")
        grid.attach(l, 0, y, 1, 1)
        self.current = Gtk.Label(label="--")
        grid.attach(self.current, 1, y, 1, 1)
        y=y+1
        l = Gtk.Label(label="Nx")
        grid.attach(l, 0, y, 1, 1)
        self.Nx = Gtk.Label(label="--")
        grid.attach(self.Nx, 1, y, 1, 1)
        y=y+1
        l = Gtk.Label(label="Ny")
        grid.attach(l, 0, y, 1, 1)
        self.Ny = Gtk.Label(label="--")
        grid.attach(self.Ny, 1, y, 1, 1)
        y=y+1
        l = Gtk.Label(label="XRange")
        grid.attach(l, 0, y, 1, 1)
        self.Xrange = Gtk.Label(label="--")
        grid.attach(self.Xrange, 1, y, 1, 1)
        y=y+1
        l = Gtk.Label(label="YRange")
        grid.attach(l, 0, y, 1, 1)
        self.Yrange = Gtk.Label(label="--")
        grid.attach(self.Yrange, 1, y, 1, 1)
        y=y+1
        l = Gtk.Label(label="dx")
        grid.attach(l, 0, y, 1, 1)
        self.dX = Gtk.Label(label="--")
        grid.attach(self.dX, 1, y, 1, 1)
        y=y+1
        l = Gtk.Label(label="dy")
        grid.attach(l, 0, y, 1, 1)
        self.dY = Gtk.Label(label="--")
        grid.attach(self.dY, 1, y, 1, 1)
        y=y+1
        l = Gtk.Label(label="dz")
        grid.attach(l, 0, y, 1, 1)
        self.dZ = Gtk.Label(label="--")
        grid.attach(self.dZ, 1, y, 1, 1)
        y=y+1
        l = Gtk.Label(label="ZR")
        grid.attach(l, 0, y, 1, 1)
        self.ZR = Gtk.Label(label="--")
        grid.attach(self.ZR, 1, y, 1, 1)

        y=97
        buttonCM = Gtk.Button(label="Color Map Swap")
        buttonCM.connect("clicked", self.color_map_swap_clicked)
        grid.attach(buttonCM, 0, y, 2, 1)
        y=y+1
        buttonC = Gtk.Button(label="Clear")
        buttonC.connect("clicked", self.clear_clicked)
        grid.attach(buttonC, 0, y, 2, 1)
        y=y+1
        self.position = Gtk.Label(label="Mouse-XY")
        grid.attach(self.position, 0, y, 2, 1)

        #ax = plt.Axes(fig,[0,0,1,1])
    
        self.fig = Figure(figsize=(6, 6), dpi=100)
        self.axy = self.fig.add_subplot(111)
        self.canvas = FigureCanvas(self.fig)  # a Gtk.DrawingArea
        self.canvas.set_size_request(800, 800)
        #self.gridwidget.attach(self.canvas, 3,0, 100,100)
        self.cbar = None

        self.colormap = cm.RdYlGn
        #self.colormap = cm.Greys  #magma
        #cmaps['Sequential'] = [
        #    'Greys', 'Purples', 'Blues', 'Greens', 'Oranges', 'Reds',
        #    'YlOrBr', 'YlOrRd', 'OrRd', 'PuRd', 'RdPu', 'BuPu',
        #    'GnBu', 'PuBu', 'YlGnBu', 'PuBuGn', 'BuGn', 'YlGn']

        #cmaps['Perceptually Uniform Sequential'] = [
        #    'viridis', 'plasma', 'inferno', 'magma', 'cividis']

        #cmaps['Sequential (2)'] = [
        #    'binary', 'gist_yarg', 'gist_gray', 'gray', 'bone', 'pink',
        #    'spring', 'summer', 'autumn', 'winter', 'cool', 'Wistia',
        #    'hot', 'afmhot', 'gist_heat', 'copper']

        
        self.evb = Gtk.EventBox()
        self.gridwidget.attach(self.evb, 3,0, 100,100)
        self.evb.add(self.canvas)

        self.evb.connect("motion-notify-event", self.on_mouse_move)
        self.evb.add_events(Gdk.EventMask.POINTER_MOTION_MASK | Gdk.EventMask.BUTTON_PRESS_MASK)
        #self.evb.set_events(Gdk.EventMask.POINTER_MOTION_MASK | Gdk.EventMask.BUTTON_PRESS_MASK)
        self.evb.connect("motion-notify-event", self.on_mouse_move)
        self.evb.connect("button-press-event", self.on_mouse_click)
        
        self.show_all()
        

    def on_mouse_move(self, widget, event):
        self.position.set_text ('MM: ({0:5.1f}, {1:5.1f})'.format( event.x, event.y))

    def on_mouse_click(self, widget, event):
        print("Key press on widget: ", widget)
        print("          Modifiers: ", event.state)
        print("                 XY: ", event.x, event.y)
        self.position.set_text ('MC: ({0:5.1f}, {1:5.1f})'.format( event.x, event.y))

        
    def quit(self, widget):
        #self.mouseThread.kill()
        return
        

    def on_file_clicked(self, widget):
        dialog = Gtk.FileChooserDialog(action=Gtk.FileChooserAction.OPEN)
        dialog.add_buttons(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
			   Gtk.STOCK_OPEN, Gtk.ResponseType.OK)
        #dialog.set_transient_for(self.main_widget)
        self.add_filters(dialog)
        #dialog.modal = True
        response = dialog.run()
        try:
            if response == Gtk.ResponseType.OK:
                print("Open clicked")
                print("File selected: " + dialog.get_filename())
                self.cdf_image_data_filename = dialog.get_filename()
                self.load_CDF ()
        finally:
            dialog.destroy()

    def add_filters(self, dialog):
        filter_py = Gtk.FileFilter()
        filter_py.set_name("Unidata NetCDF (GXSM)")
        filter_py.add_mime_type("application/x-netcdf")
        dialog.add_filter(filter_py)

        filter_any = Gtk.FileFilter()
        filter_any.set_name("Any files")
        filter_any.add_pattern("*")
        dialog.add_filter(filter_any)

    def on_folder_clicked(self, widget):
        dialog = Gtk.FileChooserDialog("Please choose a folder", self,
                                       Gtk.FileChooserAction.SELECT_FOLDER,
                                       (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                                        "Select", Gtk.ResponseType.OK))
        dialog.set_default_size(800, 400)

        response = dialog.run()
        if response == Gtk.ResponseType.OK:
            print("Select clicked")
            print("Folder selected: " + dialog.get_filename())
        elif response == Gtk.ResponseType.CANCEL:
            print("Cancel clicked")

        dialog.destroy()

    def reload_clicked(self, widget):
        if os.path.isfile(self.cdf_image_data_filename):
            self.load_CDF ()
        else:
            print ('Sorry.')

    def color_map_swap_clicked(self, widget):
        #self.colormap = cm.RdYlGn
        self.colormap = cm.Greys
        #self.colormap = cm.magma
        #cmaps['Sequential'] = [
        #    'Greys', 'Purples', 'Blues', 'Greens', 'Oranges', 'Reds',
        #    'YlOrBr', 'YlOrRd', 'OrRd', 'PuRd', 'RdPu', 'BuPu',
        #    'GnBu', 'PuBu', 'YlGnBu', 'PuBuGn', 'BuGn', 'YlGn']

        #cmaps['Perceptually Uniform Sequential'] = [
        #    'viridis', 'plasma', 'inferno', 'magma', 'cividis']

        #cmaps['Sequential (2)'] = [
        #    'binary', 'gist_yarg', 'gist_gray', 'gray', 'bone', 'pink',
        #    'spring', 'summer', 'autumn', 'winter', 'cool', 'Wistia',
        #    'hot', 'afmhot', 'gist_heat', 'copper']

            
    def next_clicked(self, widget):
        nnn = re.findall('[0-9]+',self.cdf_image_data_filename)
        tmp = self.cdf_image_data_filename.split(nnn[-1])
        test = tmp[0]+'{0:03d}'.format(int(nnn[-1])+1)+tmp[1]
        if os.path.isfile(test):
            self.cdf_image_data_filename = test
            self.load_CDF ()

    def live_clicked(self, widget):
        if self.gxsm == None:
            self.gxsm = SocketClient(HOST, PORT)
        fn = self.gxsm.request_get_parameter('LiveFilenames')
        print(fn)
        return
        
    def getNX_clicked(self, widget):
        if self.gxsm == None:
            self.gxsm = SocketClient(HOST, PORT)
        ret = self.gxsm.request_get_parameter('PointsX')
        print(ret['result'][0]['value'])
        return
        
    def start_clicked(self, widget):
        if self.gxsm == None:
            self.gxsm = SocketClient(HOST, PORT)
        fn = self.gxsm.request_start_scan()
        print(fn)
        return
        
    def stop_clicked(self, widget):
        if self.gxsm == None:
            self.gxsm = SocketClient(HOST, PORT)
        fn = self.gxsm.request_stop_scan()
        print(fn)
        return
        
    def clear_clicked(self, widget):
        for p in self.axy.patches[:]:
            p.remove()
        #foo = np.random.rand(9).reshape(3, 3)
        #extent = (0, foo.shape[1], foo.shape[0], 0)
        #self.axy.imshow(foo, extent=extent)
        self.fig.canvas.draw()
        
    def process_quick_clicked(self, widget):
        x = range (0,self.nx)
        for y in self.ImageData[:]:
            A = np.vstack([x, np.ones(len(x))]).T
            m,c = np.linalg.lstsq(A, y)[0]
            for i in x:
                y[i] = y[i] - ( m*i + c )

        self.im=self.axy.imshow(self.ImageData, interpolation='bilinear', cmap=self.colormap,
                                origin='lower', extent=[0, self.xr, 0, self.yr],
                                vmax=self.ImageData.max(), vmin=self.ImageData.min())
        self.fig.canvas.draw()

    def subdivide2x2_clicked(self, widget):
        self.subdivide_NxN(2,2)
    def subdivide3x3_clicked(self, widget):
        self.subdivide_NxN(3,3)
    def subdivide4x4_clicked(self, widget):
        self.subdivide_NxN(4,4)
    def subdivide5x5_clicked(self, widget):
        self.subdivide_NxN(5,5)

    def subdivide_NxN(self, N,M):
        #self.axy.cla ()
        self.ImageData = self.ImageData - self.ImageData.min()
        for p in self.axy.patches[:]:
            p.remove()
        
        for i,j in list(np.ndindex(N,M)):

            i0 = int (i*self.nx/N+1)
            j0 = int (j*self.ny/M+1)
            iN = int (self.nx/N-2)
            jN = int (self.ny/M-2)

            x0  = i*self.xr/N*1.01
            y0  = j*self.yr/M*1.01
            xw  = self.xr/N*0.98
            yw  = self.yr/M*0.98

            ImPatch = self.ImageData[j0:j0+jN , i0:i0+iN]
            vmin = ImPatch.min()
            self.ImageData[j0:j0+jN , i0:i0+iN] =  self.ImageData[j0:j0+jN , i0:i0+iN] - vmin
            #print (np.shape(ImPatch.shape))
            #self.im = self.axy.imshow(ImPatch, interpolation='bilinear', cmap=self.colormap,
            #                          origin='lower', extent=[x0, xw, y0, yw],
            #                          vmax=ImPatch.max(),
            #                          vmin=ImPatch.min())

            self.add_color_patch (N,M, i,j, 'red')

        self.im=self.axy.imshow(self.ImageData, interpolation='bilinear', cmap=self.colormap,
                                origin='lower', extent=[0, self.xr, 0, self.yr],
                                vmax=self.ImageData.max(), vmin=self.ImageData.min())

        self.fig.canvas.draw()

    def add_color_patch(self, N,M, i,j, color, alpha=0.1):
        x0  = i*self.xr/N*1.01
        y0  = j*self.yr/M*1.01
        xw  = self.xr/N*0.98
        yw  = self.yr/M*0.98
        self.axy.add_patch(Rectangle((x0,y0), xw, yw, facecolor=color, alpha=alpha))
        
        
    def load_CDF(self):
        print("NetCDF File: ", self.cdf_image_data_filename)
        self.rootgrp = Dataset (self.cdf_image_data_filename, "r")
        print(self.rootgrp['FloatField'])

        self.ImageData = self.rootgrp['FloatField'][0][0][:][:]
        print (self.ImageData)
        self.Xlookup = self.rootgrp['dimx'][:]
        self.Ylookup = self.rootgrp['dimy'][:]
        print('Bias    = ', self.rootgrp['sranger_mk2_hwi_bias'][0], self.rootgrp['sranger_mk2_hwi_bias'].var_unit)
        self.ub = self.rootgrp['sranger_mk2_hwi_bias'][0]
        self.bias.set_text ('{0:.4f} '.format(self.rootgrp['sranger_mk2_hwi_bias'][0])+self.rootgrp['sranger_mk2_hwi_bias'].var_unit)

        print('Current = ', self.rootgrp['sranger_mk2_hwi_mix0_set_point'][0], self.rootgrp['sranger_mk2_hwi_mix0_set_point'].var_unit)
        self.cur = self.rootgrp['sranger_mk2_hwi_mix0_set_point'][0]
        self.current.set_text ('{0:.4f} '.format(self.rootgrp['sranger_mk2_hwi_mix0_set_point'][0])+self.rootgrp['sranger_mk2_hwi_mix0_set_point'].var_unit)

        print('X Range = ', self.rootgrp['rangex'][0], self.rootgrp['rangex'].var_unit)
        self.xr = self.rootgrp['rangex'][0]
        self.Xrange.set_text ('{0:.1f} '.format(self.rootgrp['rangex'][0])+self.rootgrp['rangex'].var_unit)
        
        print('Y Range = ', self.rootgrp['rangey'][0], self.rootgrp['rangey'].var_unit)
        self.yr = self.rootgrp['rangey'][0]
        self.Yrange.set_text ('{0:.1f} '.format(self.rootgrp['rangey'][0])+self.rootgrp['rangey'].var_unit)

        print('Nx      = ', len(self.rootgrp['dimx']))
        self.nx = len(self.rootgrp['dimx'])
        self.Nx.set_text ('{} px'.format(self.nx))
        print('Ny      = ', len(self.rootgrp['dimy']))
        self.ny = len(self.rootgrp['dimy'])
        self.Ny.set_text ('{} px'.format(self.ny))

        print('dx      = ', self.rootgrp['dx'][0], self.rootgrp['dx'].var_unit)
        self.dx = self.rootgrp['dx'][0]
        self.dX.set_text ('{0:.4f} '.format(self.rootgrp['dx'][0])+self.rootgrp['dx'].var_unit)
        print('dy      = ', self.rootgrp['dy'][0], self.rootgrp['dy'].var_unit)
        self.dy = self.rootgrp['dy'][0]
        self.dY.set_text ('{0:.4f} '.format(self.rootgrp['dy'][0])+self.rootgrp['dy'].var_unit)
        print('dz      = ', self.rootgrp['dz'][0], self.rootgrp['dz'].var_unit)
        self.dz = self.rootgrp['dz'][0]
        self.dZ.set_text ('{0:.4f} '.format(self.rootgrp['dz'][0])+self.rootgrp['dz'].var_unit)
        self.ZR.set_text ('{0:.2f} '.format(self.ImageData.max()-self.ImageData.min())+self.rootgrp['dz'].var_unit)

        for p in self.axy.patches[:]:
            p.remove()
        
        # create/update figure and image
        self.im=self.axy.imshow(self.ImageData, interpolation='bilinear', cmap=self.colormap,
                           origin='lower', extent=[0, self.xr, 0, self.yr],
                           vmax=self.ImageData.max(), vmin=self.ImageData.min())
        self.axy.set_title('...'+self.cdf_image_data_filename[-45:])
        self.axy.set_xlabel('X in '+self.rootgrp['dx'].var_unit)
        self.axy.set_ylabel('Y in '+self.rootgrp['dy'].var_unit)
        if self.cbar:
            self.cbar.remove () #fig.delaxes(self.figure.axes[1])
        self.cbar = self.fig.colorbar(self.im, extend='both', shrink=0.9, ax=self.axy)
        self.cbar.set_label('Z in '+self.rootgrp['dz'].var_unit)
        self.fig.canvas.draw()
      
        self.rootgrp.close()

    def list_CDF_clicked(self, widget):
        print("NetCDF File: ", self.cdf_image_data_filename)
        self.rootgrp = Dataset (self.cdf_image_data_filename, "r")
        print(self.rootgrp.data_model)
        print(self.rootgrp.groups)
        print(self.rootgrp.dimensions)
        print(self.rootgrp.variables)

        print('========================================')
        print(self.rootgrp['FloatField'])

        #print(self.rootgrp['dimx'])
        #print(self.rootgrp['dimx'][:])
        self.Xlookup = self.rootgrp['dimx'][:]

        #print(self.rootgrp['dimy'])
        #print(self.rootgrp['dimy'][:])
        self.Ylookup = self.rootgrp['dimy'][:]
        
        #print(self.rootgrp['sranger_mk2_hwi_bias'])
        print('Bias = ', self.rootgrp['sranger_mk2_hwi_bias'][0], self.rootgrp['sranger_mk2_hwi_bias'].var_unit)

        #print(self.rootgrp['sranger_mk2_hwi_mix0_set_point'])
        print('Current = ', self.rootgrp['sranger_mk2_hwi_mix0_set_point'][0], self.rootgrp['sranger_mk2_hwi_mix0_set_point'].var_unit)

        #print(self.rootgrp['rangex'])
        print('X Range = ', self.rootgrp['rangex'][0], self.rootgrp['rangex'].var_unit)
        #print(self.rootgrp['rangey'])
        print('Y Range = ', self.rootgrp['rangey'][0], self.rootgrp['rangey'].var_unit)
        #print(self.rootgrp['dz'])
        print('dz = ', self.rootgrp['dz'][0], self.rootgrp['dz'].var_unit)

        self.rootgrp.close()

        
    def on_change_label_state(self, action, value):
        action.set_state(value)
        self.label.set_text(value.get_string())

    def on_maximize_toggle(self, action, value):
        action.set_state(value)
        if value.get_boolean():
            self.maximize()
        else:
            self.unmaximize()

class Application(Gtk.Application):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, application_id="org.example.myapp",
                         flags=Gio.ApplicationFlags.HANDLES_COMMAND_LINE,
                         **kwargs)
        self.window = None

        self.add_main_option("test", ord("t"), GLib.OptionFlags.NONE,
                             GLib.OptionArg.NONE, "Command line test", None)

    def do_startup(self):
        Gtk.Application.do_startup(self)

        action = Gio.SimpleAction.new("about", None)
        action.connect("activate", self.on_about)
        self.add_action(action)

        action = Gio.SimpleAction.new("quit", None)
        action.connect("activate", self.on_quit)
        self.add_action(action)

        builder = Gtk.Builder.new_from_string(MENU_XML, -1)
        self.set_app_menu(builder.get_object("app-menu"))

    def do_activate(self):
        # We only allow a single window and raise any existing ones
        if not self.window:
            # Windows are associated with the application
            # when the last one is closed the application shuts down
            self.window = AppWindow(application=self, title="GXSM NetCDF Data and Scan Image Tagging Viewer")

        self.window.present()

    def do_command_line(self, command_line):
        options = command_line.get_options_dict()
        # convert GVariantDict -> GVariant -> dict
        options = options.end().unpack()

        if "test" in options:
            # This is printed on the main instance
            print("Test argument recieved: %s" % options["test"])

        self.activate()
        return 0

    def on_about(self, action, param):
        about_dialog = Gtk.AboutDialog(transient_for=self.window, modal=True)
        about_dialog.present()

    def on_quit(self, action, param):
        self.quit()

if __name__ == "__main__":
    app = Application()
    app.run(sys.argv)
