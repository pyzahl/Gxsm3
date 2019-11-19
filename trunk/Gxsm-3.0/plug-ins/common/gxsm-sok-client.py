#!/usr/bin/env python3

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

import gobject, gtk
import cairo
import os		# use os because python IO is bugy
import time
import fcntl
from threading import Timer

#import GtkExtra
import struct
import array

import math

import socket




class AI_app(Gtk.Window):

    def __init__(self, sok):
        Gtk.Window.__init__(self, title="AI ---- GXSM")

        self.sok=sok
        self.i=0

        grid = Gtk.Grid()
        self.add(grid)

        self.button = Gtk.Button(label="Click Here")
        self.button.connect("clicked", self.on_button_clicked)
        grid.add(self.button)
        self.send = Gtk.Button(label="Send")
        self.send.connect("clicked", self.send_message)
        grid.add(self.send)
        self.send = Gtk.Button(label="Start")
        self.send.connect("clicked", self.start_scan_message)
        grid.add(self.send)
        self.send = Gtk.Button(label="Stop")
        self.send.connect("clicked", self.stop_scan_message)
        grid.add(self.send)
        self.send = Gtk.Button(label="Set Test")
        self.send.connect("clicked", self.set_test_message)
        grid.add(self.send)

    def on_button_clicked(self, widget):
        print("Hello World")
    
    def send_message(self, widget):
        self.sok.sendall(b'Count %d'%self.i)
        data = self.sok.recv(1024)
        print('Received', repr(data))
        self.i=self.i+1
    
    def start_scan_message(self, widget):
        self.sok.sendall(b'gxsm-action-start-scan')
        data = self.sok.recv(1024)
        print('Received', repr(data))
        self.i=self.i+1
    
    def stop_scan_message(self, widget):
        self.sok.sendall(b'gxsm-action-stop-scan')
        data = self.sok.recv(1024)
        print('Received', repr(data))
        self.i=self.i+1
    
    def set_test_message(self, widget):
        self.sok.sendall(b'gxsm-set-OffsetX,%d'%self.i)
        data = self.sok.recv(1024)
        print('Received', repr(data))
        self.i=self.i+10
    

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 65432        # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b'Hello GXSM3, Hello world -- AI is coming!')
    data = s.recv(1024)
    print('Received', repr(data))

    win = AI_app(s)
    win.connect("destroy", Gtk.main_quit)
    win.show_all()
    Gtk.main()
