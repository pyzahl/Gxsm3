#!/usr/bin/env python2.2

## * Python UTF character explorer
## * 
## * Copyright (C) 2004 Percy Zahl
## *
## * Author: Percy Zahl <zahl@users.sf.net>
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


import pygtk
pygtk.require('2.0')

import gobject, gtk

#import GtkExtra
import struct
import array
from gtk import TRUE, FALSE

def delete_event(win, event=None):
	win.hide()
	# don't destroy window -- just leave it hidden
	return gtk.TRUE


def do_exit(button):
        gtk.main_quit()

def destroy(*args):
        gtk.main_quit()

def do_print(b, txt, info):
	info.set_text (txt)
	print txt
	
def create_main_window():
	win = gobject.new(gtk.Window,
	             type=gtk.WINDOW_TOPLEVEL,
                     title='UTF-8 Charcter Explorer Tool (C) 2004 by P.Zahl -- GPL',
                     allow_grow=gtk.TRUE,
                     allow_shrink=gtk.TRUE,
                     border_width=5)
	win.set_name("main window")
	win.connect("destroy", destroy)
	win.connect("delete_event", destroy)

	box1 = gobject.new(gtk.VBox())
	win.add(box1)

	info = gobject.new(gtk.Label, label='Click a character!')

	scr = gobject.new(gtk.ScrolledWindow())
	scr.set_size_request(600,500)
	box1.pack_start(scr)
	
	table = gtk.Table (17, 16) # expands itself in y...
	table.set_row_spacings(5)
	table.set_col_spacings(5)
	scr.add_with_viewport(table)	

	page_start = 0
	page_end   = 6
	
	esc = 193+page_start
	v=0
	utf=page_start*0x0100
	for page in range(page_start,page_end):
	    if esc == 193:
		a=0
		b=3
	    else:
		a=0
		b=4
	    for k in range(a,b):
		if esc == 193:
		    va=0
		    vb=8
		else:
		    va=8
		    vb=12
		for vv in range(va,vb):
		    if esc > 193:
			lab = gobject.new(gtk.Label, label= '%(P)d \\%(U)o' % {"P":page,"U":esc})
		    else:
			lab = gobject.new(gtk.Label, label= 'ASCII')
		    table.attach (lab, 0, 1, v, v+1)
		    for h in range(0,16):
			c = vv*16+h
			if esc > 193:
			    utf8_txt = '%(U)c%(#)c' % {"U":esc,"#":c}
			    esc_code = 'Page: %(P)d  Escaped: \\%(E)o\\%(N)o   Hex: 0x%(E)02X%(N)02X   U%(U)04X' % {"P":page, "E":esc, "N":c, "U":utf} + '   UTF8-char: ' + utf8_txt
			else:
			    if utf >= 0x20:
				utf8_txt = '%(#)c' % {"#":utf}
			    else:
				utf8_txt = '%(U)02x' % {"U":utf}
			    esc_code = 'Escaped: \\%(N)o   Hex: 0x%(N)02X   U%(N)04X' % {"N":utf} + '   UTF8/ASCII-char: ' + utf8_txt
			but = gobject.new(gtk.Button, label=utf8_txt)
			table.attach (but, h+1, h+2, v, v+1)
			but.connect("clicked", do_print, esc_code, info)
			utf += 1
		    v += 1
		esc += 1
					
	separator = gobject.new(gtk.HSeparator())
	box1.pack_start(separator, expand=gtk.FALSE)

	box1.pack_start(info, expand=gtk.FALSE)

	separator = gobject.new(gtk.HSeparator())
	box1.pack_start(separator, expand=gtk.FALSE)
	box2 = gobject.new(gtk.VBox(spacing=10))
	box2.set_border_width(5)
	box1.pack_start(box2, expand=gtk.FALSE)
	button = gtk.Button(stock='gtk-close')
	button.connect("clicked", do_exit)
	button.set_flags(gtk.CAN_DEFAULT)
	box2.pack_start(button)
	button.grab_default()
	win.show_all()
	
create_main_window()
gtk.main()
print ("Byby.")

