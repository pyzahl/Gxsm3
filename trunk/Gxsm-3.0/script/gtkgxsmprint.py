#!/usr/bin/env python

# gtkgxsmprint.py
#
# This program is a frontend for gxsmprint.py, which  
# is a program for printing Gxsm-NetCDF-Files.
# 

import gtk,os

PATHTOPYTHON='python'
PATHTOGXSMPRINT='./gxsmprint.py'

class GtkGxsmPrint:

    def do_print(self,widget):
	print "OK, printing."
	if self.r1.get_active():
		opts = " -o %s " % (self.e2.get_text())
	if self.r2.get_active():
		opts = " -p %s " % (self.e3.get_text()) 
	if self.r3.get_active():
		opts = " -v %s " % (self.e4.get_text()) 
	if self.b1.get_active():
		opts += ' -N userinfo '
	if self.b2.get_active():
		opts += ' -N basename '
	if self.combo.entry.get_text() != '1':
		opts += ' -u ' + self.combo.entry.get_text()
	opts += ' ' + self.e1.get_text()
	os.system (PATHTOPYTHON + ' ' +\
		   PATHTOGXSMPRINT + ' %s ' % opts)
	print "Ready."

    def callback(self, widget, data=None):
	pass

    def delete_event(self, widget, event, data=None):
        gtk.mainquit()
        return gtk.FALSE

    def __init__(self):
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_title("GtkGxsmprint")
        self.window.connect("delete_event", self.delete_event)
        self.window.set_border_width(20)
        vbox = gtk.VBox(gtk.TRUE, 2)
        self.window.add(vbox)

########################
        button = gtk.Label("Enter filenames")
        vbox.pack_start(button, gtk.TRUE, gtk.TRUE, 2)
        button.show()

        self.e1 = gtk.Entry(100)
	self.e1.set_text("*.nc")
        vbox.pack_start(self.e1, gtk.TRUE, gtk.TRUE, 2)
        self.e1.show()
########################
        self.r1 = gtk.RadioButton(None, "Print to this file:")
        self.r1.connect("toggled", self.callback, "radio button 1")
        self.r1.set_active(gtk.TRUE)
        vbox.pack_start(self.r1, gtk.TRUE, gtk.TRUE, 0)
        self.r1.show()

        self.e2 = gtk.Entry(100)
	self.e2.set_text("result.ps")
        vbox.pack_start(self.e2, gtk.TRUE, gtk.TRUE, 2)
        self.e2.show()
########################
        self.r2 = gtk.RadioButton(self.r1, "Print using this command:")
        self.r2.connect("toggled", self.callback, "radio button 2")
        vbox.pack_start(self.r2, gtk.TRUE, gtk.TRUE, 0)
        self.r2.show()

        self.e3 = gtk.Entry(100)
	self.e3.set_text("lpr")
        vbox.pack_start(self.e3, gtk.TRUE, gtk.TRUE, 2)
        self.e3.show()
########################
        self.r3 = gtk.RadioButton(self.r1, "Preview with:")
        self.r3.connect("toggled", self.callback, "radio button 3")
        vbox.pack_start(self.r3, gtk.TRUE, gtk.TRUE, 0)
        self.r3.show()

        self.e4 = gtk.Entry(100)
	self.e4.set_text("gv")
        vbox.pack_start(self.e4, gtk.TRUE, gtk.TRUE, 2)
        self.e4.show()
########################
        button = gtk.Label("Exclude this information:")
        vbox.pack_start(button, gtk.TRUE, gtk.TRUE, 2)
        button.show()
########################
        self.b1 = gtk.CheckButton("userinfo")
        self.b1.connect("toggled", self.callback, "userinfo")
        vbox.pack_start(self.b1, gtk.TRUE, gtk.TRUE, 2)
        self.b1.show()
########################
        self.b2 = gtk.CheckButton("basename")
        self.b2.connect("toggled", self.callback, "basename")
        vbox.pack_start(self.b2, gtk.TRUE, gtk.TRUE, 2)
        self.b2.show()
#
        button = gtk.Label("Number of images on one page:")
        vbox.pack_start(button, gtk.TRUE, gtk.TRUE, 2)
        button.show()
	self.combo = gtk.Combo()
	slist = [ "1", "2", "4", "6", "8", "9", "12", "16" ]
	self.combo.set_popdown_strings(slist)
	self.combo.entry.connect("activate", self.callback, "combo")
        vbox.pack_start(self.combo, gtk.TRUE, gtk.TRUE, 2)
	self.combo.show()

########################
        button = gtk.Button("Print")
        button.connect("clicked", self.do_print)
        vbox.pack_start(button, gtk.TRUE, gtk.TRUE, 2)
	button.show()
########################
        button = gtk.Button("Quit")
        button.connect("clicked", lambda wid: gtk.mainquit())
        vbox.pack_start(button, gtk.TRUE, gtk.TRUE, 2)
        button.show()
########################
        vbox.show()
        self.window.show()

def main():
    gtk.main()
    return 0       

if __name__ == "__main__":
    GtkGxsmPrint()
    main()
