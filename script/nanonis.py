# This python-script decodes a Nanonis-file
# used for surface analysis
# author: stefan_fkp@sf
# GPL 2.0
# 
import sys,  re, struct

try:
	import Numeric as numpy
	#download from http://numeric.scipy.org/
	#or debian: python-numeric
except:
	import numpy

class nanonis:
	def __init__(self):
		print "Creating new object."
		self.filename = ""
		self.header      = ""
		self.data = "" 
		self.NO_OF_CHANNELS = 0
		self.size        = (0,0)     # in pixels 
		self.type        = ('', '')  # only FLOAT accepted
		self.range       = (0,0)
		self.TYPE_STR    = ''
		self.TYPE_ENDIAN = ''
		self.TYPE_LEN    = ''
		self.map_to_0_256 = True

	def read(self, filename):
		self.filename = filename
		print "Reading " + self.filename
		f = open(filename, 'rb')
		lines = f.read()
		f.close()

		(self.header, self.data)= lines.split(':SCANIT_END:')

		headerarray = self.header.split('\n')
		
		for i in range(len(headerarray)):
			if headerarray[i].startswith(':SCAN_PIXELS'): # next line contains pixels
				self.size =  [int(x) for x in headerarray[i+1].split(' ') if x != '']
			if headerarray[i].startswith(':SCANIT_TYPE'): # next lines contains type
				print "Found type"
				self.type =  [x for x in headerarray[i+1].split(' ') if x != '']
			if headerarray[i].startswith(':SCAN_RANGE'): # next lines contains range
				self.range =  [x for x in headerarray[i+1].split(' ') if x != '']

		self.TOTAL = self.size[0] * self.size[1]

		print headerarray
		if( self.type[0] != 'FLOAT'):
			print "Type is not float, sorry."
			sys.exit()
		else:
			self.TYPE_STR = 'f'

		# This many bytes are taken by one item 
		self.TYPE_LEN = struct.calcsize(self.TYPE_STR)

		self.NO_OF_CHANNELS = len(self.data)/self.TOTAL/self.TYPE_LEN
		
		if( self.type[1] == 'LSBFIRST'):
			self.TYPE_ENDIAN = '>'
		else:
			self.TYPE_ENDIAN = '<'

	def get_data_for_channel(self, channel):
		daten_anfang = self.data.index('\x1a\x04')
		daten = self.data[daten_anfang+2:]
		offset = channel * self.TOTAL * self.TYPE_LEN
		feld = struct.unpack(self.TYPE_ENDIAN + str(self.TOTAL)+ self.TYPE_STR, 
				daten[offset + 0: offset + self.TOTAL*self.TYPE_LEN])
	
		a = numpy.array(feld, numpy.Float) # convert to numpy array to allow arithmetic on it
		if (self.map_to_0_256 == True):
			a = a - min(a)    #skalieren
			a = a/max(a)*256  #skalieren
		return(a)

	def dump_info_on_file(self):
		print "Size " + `self.size`
		print "Type " + `self.type`
		print "Range " + `self.range`
		print "Len data " + `len(self.data)`

	def convert_to_bitmap(self, newfilename):
		import Image
		# download from http://www.pythonware.com/downloads/index.htm
		#or debian: python-imaging

		filenameparts = newfilename.split('.')
		if (len(filenameparts) != 2):
			print "Too many dots in filename."
			return
		
		for i in range(datei1.NO_OF_CHANNELS):
			im = Image.new('L', self.size)
			im.putdata(self.get_data_for_channel(i), 1, 0) # 1=scale,0=offset
			newname = filenameparts[0] + `i` + "." + filenameparts[1]
			print "Writing to " + newname
			im.save(newname)

if __name__=="__main__":

	filename = ""
	target = ""

	if (len(sys.argv) == 1):
		import pygtk
		pygtk.require('2.0')
		import gtk
		dialog = gtk.FileChooserDialog("Select Nanonis-file",
					       None,
					       gtk.FILE_CHOOSER_ACTION_OPEN,
					       (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
						gtk.STOCK_OPEN, gtk.RESPONSE_OK))
		dialog.set_default_response(gtk.RESPONSE_OK)

		filter = gtk.FileFilter()
		filter.set_name("All files")
		filter.add_pattern("*")
		dialog.add_filter(filter)

		filter = gtk.FileFilter()
		filter.set_name("Nanonis")
		filter.add_pattern("*.sxm")
		dialog.add_filter(filter)

		response = dialog.run()
		if response == gtk.RESPONSE_OK:
		    print dialog.get_filename(), 'selected'
		elif response == gtk.RESPONSE_CANCEL:
		    print 'Closed, no files selected'
		    sys.exit()
		
		filename = dialog.get_filename()
		target = filename[:-4] + '.gif'
		dialog.destroy()
	
	if (len(sys.argv) == 3):
		# Conversion to other format
		filename = sys.argv[1]
		target   = sys.argv[2]
		
	if (len(sys.argv) == 2):
		filename = sys.argv[1]
		target = filename[:-4] + '.gif'

	datei1 = nanonis()
	datei1.read(filename)
	datei1.dump_info_on_file()
	datei1.convert_to_bitmap(target)
