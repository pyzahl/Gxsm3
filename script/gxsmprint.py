#! /usr/bin/python
#
# Prettyprint Gxsm-Netcdf-files
#
# You
# can use this program to easily print a greater number of nc-files
# and even do some image-manipulation while doing so.
# More info can be found in the Gxsm-Manual at gxsm.sf.net
#
# This file is based on the pilprint.py utility that
# comes with the PIL. 
#
# WARNING: Before using it, you have to correct a bug
# in the file PSDraw.py which resides in
# /usr/lib/pythonVERSION/site-packages/PIL
# You have to change gsize to gsave in this line
# self.fp.write("gsize\n%f %f translate\n" % (dx, dy))
#
# The PSdraw module was not supposed to print multiple-pages
# files, thus the resulting ps is corrupt and has to be
# processed with ps2ps.
#
# This program uses ps2ps from the ghostscript-package
# This program uses psnup for thumbnailing.
# 
VERSION = "gxsmprint 0.1/2004-09-03"
TMPFILENAME= "/tmp/gxsmprint.ps"
TMPFILENAME2= "/tmp/gxsmprint2.ps"
FONTSIZETITLE= 18
FONTSIZETEXT= 11

import Image
import PSDraw, Numeric, math
from Scientific.IO.NetCDF import *

# Page setup in Postscript-points
XLEFT  = 1.0*72
YBOTTOM= 1.0*72
XRIGHT = 7.5*72
YTOP   = 10.0*72
XSIZE = XRIGHT-XLEFT
YSIZE = YTOP-YBOTTOM
letter = ( XLEFT, YBOTTOM, XRIGHT, YTOP )

class gxsmprintconfig:
    conf={
	'monochrome' : 0,
	'invert'     : 0,	
	'maintitle'  : 1,
	'title'      : 1,
	'username'   : 1,
	'userinfo'   : 1,
	'basename'   : 1,
	'dateofscan' : 1,
	'range'      : 1,
	'step'       : 1,
	'dsp'        : 1,
	'dim'        : 1,
	'comment'    : 0,
	'up'         : '1',
	'target'     : 0,
	'quick'	     : 0,
	#target = 0 -> stdout
	#target = 1 -> file
	#target = 2 -> printer
	#target = 3 -> preview 
	'tofilename'     : 'result.ps',
	'toprintername'  : 'lpr',
	'topreviewname'  : 'gv'
	}

    def __init__(self):
	pass


class GxsmPrint:

	def description(self, file, image):
	     title = os.path.splitext(os.path.split(file)[1])[0]
	     format = " (%dx%d "
	     if image.format:
	         format = " (" + image.format + " %dx%d "
	     return title + format % image.size + image.mode + ")"
	 
	def nextitem(self, ps, s):
		if (self.currentline % 2)==0:
	 		ps.text(( XLEFT, YTOP-self.currentline/2*FONTSIZETEXT*1.2), s)
		else:
	        	ps.text(( XLEFT+3*72, YTOP-self.currentline/2*FONTSIZETEXT*1.2), s)
		self.currentline += 1
	     
	def getnc(self, nc, s):
	 	"Quick and dirty hack to fetch data from nc-file."
	 	h = ''
	 	try:
	 		t = nc.variables[s].typecode()
	 		if t=='c':  # Variable is a string
	 			g = nc.variables[s].getValue()
	 			for i in range(len(g)):
	 				h=h+g[i]
	 		elif t=='d': # Variable is a decimal
	 			g = nc.variables[s].getValue()
	 			h=str(g)
	 	except:
	 		h = 'Not available.'
	 	return h
	 
	def getunit(self, nc, s):
	 	g = nc.variables[s].unit
	 	return str(g)
	 
	def calc_steps(self, r):
	 	"Calculate stepsize between labels."
	 	s = math.pow(10, int(math.log10(r)))
		q = r/s              # number of labels
		if q < 5:
			while q < 5:
				s = s/10
				q = r/s
		if q > 15:
			while q > 15:
		 		s = s*2
				q = r/s
	 	return s

        def linearRegression(self, l, start=None, stop=None):
                if not start: start = 0
                if not stop : stop  = len(l)
                sumx = sumxq = sumy = sumyq = sumxy = 0.0
                n = len(l)      
                for x in range(start, stop):
                        sumx   += x  
                        sumxq  += x*x
                        y =  l[x]  
                        sumy   += y
                        sumyq  += y * y
                        sumxy  += y * x

                xmean = sumx / n
                ymean = sumy / n
                # a is slope, b is y-achsenabschnitt
                a = (sumxy - sumx * sumy / n) / (sumxq - sumx * sumx / n)
                b = ymean - a * xmean
                return (a,b)

        def linearizeLine(self, l):
                a,b = self.linearRegression(l, start=int(0.1*len(l)), stop=int(0.9*len(l)))
                newline = l
                for i in range(len(l)):
                        newline[i] -= a*i + b
                return newline

        def linearizeSet(self, s, x, y):
                newimagedata = [] 
                for i in range(y):
                        newimagedata += self.linearizeLine(list(s[i]))
                return newimagedata

	def __init__(self, file, fp, conf):
	 	im = Image.open(file)
#		print list(im.getdata())
	 	self.currentline = 0
	 
	 	if conf.conf['invert']:
	 	    import ImageChops
	 	    im = ImageChops.invert(im)
	 
	        if conf.conf['monochrome'] and im.mode not in ["1", "L"]:
	             im.draft("L", im.size)
	             im = im.convert("L")
	 

		if conf.conf['quick']:
		     nc = NetCDFFile(file, 'r')
                     foo = nc.variables['H']   
                     x,y = im.size
                     foo_array = foo[:]    

                     newimagedata = self.linearizeSet(list(foo_array[0][0]), x, y)
                     d=Numeric.ravel(newimagedata)      # linearize
                     d=d-min(d)              #  map to [0..256]
                     d=d*256./max(d) # we should catch div by 0

                     im.putdata( d )
		     nc.close()
	 	#
	 	# Here begins the action	
	 	#

		ps = PSDraw.PSDraw(fp)
	 
	        ps.begin_document()
	        ps.setfont("Helvetica-Narrow-Bold", FONTSIZETITLE )
	        title = self.description(file, im)
	        ps.text((XLEFT, YTOP+24), title)
	        ps.setfont("Helvetica-Narrow-Bold", FONTSIZETEXT )
	 
	 	#
	 	# Print information block.
	 	# 
 	 	nc = NetCDFFile(file, 'r')
 	 	if conf.conf['userinfo']:
 	 		self.nextitem(ps, 'Printed by: ' + str(os.getlogin()))
 	 		self.nextitem(ps, 'Printed on: ' + str(os.uname()))
	 	if conf.conf['title']:
 	 		self.nextitem(ps, 'Title: ' + self.getnc(nc, 'title'))
 	 	if conf.conf['basename']:
 	 		self.nextitem(ps, 'Basename: ' + self.getnc(nc, 'basename'))
 	 	if conf.conf['username']:
	 		self.nextitem(ps, 'Username: ' + self.getnc(nc, 'username'))
 	 	if conf.conf['dateofscan']:
 	 		self.nextitem(ps, 'Date of Scan: ' + self.getnc(nc, 'dateofscan'))
 	 	if conf.conf['range']:
	 		self.nextitem(ps, 'Range: ' + self.getnc(nc, 'rangex') + " " + self.getunit(nc, 'rangex') +\
 	 	 		' , ' + self.getnc(nc, 'rangey') + " " + self.getunit(nc, 'rangey'))
 	 		self.nextitem(ps, 'RangeZ: ' + self.getnc(nc, 'rangez') + " " + self.getunit(nc, 'rangez'))
 	 	if conf.conf['step']:
 	 		self.nextitem(ps, 'Step: ' + self.getnc(nc, 'dx') + " " + self.getunit(nc, 'dx') +\
	 			' , ' + self.getnc(nc, 'dy') + " " + self.getunit(nc, 'dy'))
 	 	if conf.conf['dsp']:
 	 		self.nextitem(ps, 'DSP (I/U/SP): ' + self.getnc(nc, 'dsp_itunnel') + self.getunit(nc, 'dsp_itunnel')
 	 				+ " " + self.getnc(nc, 'dsp_utunnel') + self.getunit(nc, 'dsp_utunnel')
 	 				+ " " + self.getnc(nc, 'dsp_SetPoint') + self.getunit(nc, 'dsp_SetPoint') )
 	 	if conf.conf['dim']:
 	 		self.nextitem(ps, 'dimx='+`nc.dimensions['dimx']`+
 	 			' dimy='+`nc.dimensions['dimy']`)
 	 	if conf.conf['comment']:
 	 		self.nextitem(ps, self.getnc(nc, 'comment'))
 	 	rangex=self.getnc(nc, 'rangex')
	 	rangey=self.getnc(nc, 'rangey')
 	 	nc.close()
	 
	 	#
	 	# Print image 
	 	#
	 	# here we calculate the dpi to make the image fit to page, default is 100
	 	ps.image(letter, im, im.size[0]/(letter[2]-letter[0]))
#		ps.line( (XLEFT, YBOTTOM), (XRIGHT, YTOP))

	 	# 
	 	# Print labels
	 	#
 	 	rx=float(rangex)
 	 	xstep = self.calc_steps(rx)
 	 	i=0
 	 	ASPECT=float(im.size[1])/im.size[0]
 	 	while i <= rx:
 	 		y= YBOTTOM + (YSIZE/2)  - XSIZE * ASPECT/2 #lower
 	 		x= XLEFT + int( ((i/rx) * (XSIZE)))
 	 		ps.line( (x, y), (x, y-10) )
 	 		ps.text( (x-10,  y-20), str(i))
 	 
 	 		y= YBOTTOM + (YSIZE/2)  + XSIZE * ASPECT/2 #upper
 	 		x= XLEFT + int( ((i/rx) * (XSIZE)))
 	 		ps.line( (x, y), (x, y+10) )
 	 		ps.text( (x-10,  y+10), str(i))
 	 		i += xstep
 	 
 	 	ry=float(rangey)
 	 	ystep = self.calc_steps(ry)
 	 	i=0
 	 	while i <= ry:
 	 		y= YBOTTOM + (YSIZE/2)  - XSIZE * ASPECT/2 + \
 	 			+ int( ((i/ry) * (XSIZE) * ASPECT ))
 	 
 	 		x= XLEFT # left border
 	 		ps.line( (x, y), (x-10, y) )
 	 		ps.text( (x-40,  y), str(i))
 	 
 	 		x= XRIGHT  # right border
 	 		ps.line( (x, y), (x+10, y) )
 	 		ps.text( (x+10,  y), str(i))
 	 		i += ystep
		ps.end_document()

	def doprint(self):
		if conf.conf['target'] != 0:
	        	os.system('ps2ps ' + TMPFILENAME + ' ' + TMPFILENAME2)
	        	os.system('psnup -' + conf.conf['up'] + ' ' + TMPFILENAME2 + ' ' + TMPFILENAME)	
	 
		if conf.conf['target'] == 1: 
			os.system('mv -f ' + TMPFILENAME + ' ' + conf.conf['tofilename'] )
		elif conf.conf['target'] == 2: 
			os.system(conf.conf['toprintername'] + ' ' + TMPFILENAME)
		elif conf.conf['target'] == 3:
			os.system(conf.conf['topreviewname'] + ' ' + TMPFILENAME)

#############################
# main
#############################

import getopt, os, sys

try:
    opt, argv = getopt.getopt(sys.argv[1:], "qcdv:iu:p:P:N:o:")
except getopt.error, v:
    print v
    sys.exit(1)

if argv == []:
    print "Nothing to do."
    print "Usage: python gxsmprint.py files"
    print "Options are:\n-p \'lpr\'  print-command"
    print "-o \'file.ps\'  target-file"
    print "-v \'gv\' preview command"
    print "-N option  do not print this nc-variable"
    print "-i invert image"
    print "-u number  n-up printing"
    print "-q   quick-view"
    sys.exit(1)

conf = gxsmprintconfig()

for o, a in opt:
    if o == "-c":
        conf.conf['monochrome'] = 0
    elif o == "-o":
	conf.conf['target']  = 1
	conf.conf['tofilename'] = str(a)
    elif o == "-p":
	conf.conf['target']  = 2
	conf.conf['toprintername'] = a
    elif o == "-v":
	conf.conf['target']  = 3
	conf.conf['topreviewname'] = a
    elif o == "-N":
  	conf.conf[a] = 0
    elif o == "-i":
	conf.conf['invert'] = 1
    elif o == "-q":
	conf.conf['quick'] = 1
    elif o == "-u":
	conf.conf['up'] = str(a)
try:
	if conf.conf['target'] == 0:
		fp = sys.stdout
	else:
		fp = open(TMPFILENAME, 'w')
	for file in argv:
		g = GxsmPrint(file, fp, conf)
	fp.close()

	g.doprint()
except:
	print "Cannot print image."
	print "(%s:%s)" % (sys.exc_type, sys.exc_value)

# THE END ###################################################
