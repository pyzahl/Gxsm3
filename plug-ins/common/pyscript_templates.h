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

const gchar *template_help = R"V0G0N(
############################################################################
#
# PLEASE EXECUTE ME TO PRINT THE GXSM3 PYTHON REMOTE COMMAND HELP LIST !!
#
############################################################################
S='***'
s='--------------------------------------------------------------------------------'
print (S)
print ('(1) Gxsm3 python remote console -- gxsm.help on build in commands')
print (s)
for h in gxsm.help ():
	print h
print ('*')

print (s)
print ('(2) Gxsm3 python remote console -- help on reference names')
print ('  used for gxsm.set and get, gets commands.')
print ('  Hint: hover the pointer over any get/set enabled Gxsm entry to see it`s ref-name!')
print ('  Example: gxsm.set ("dsp-fbs-bias", "1.0")')
print (s)

for h in gxsm.list_refnames ():
	print h
print ('*')

print (s)
print ('(3) Gxsm3 python remote console -- help on action names used for gxsm.action command')
print ('  Hint: hover the pointer over any Gxsm Action enabled Button to see it`s action-name!')
print ('  Example: gxsm.action ("DSP_CMD_GOTO_Z0")')
print (s)
	
for h in gxsm.list_actions ():
	print h
print ('*')
)V0G0N";


const gchar *template_data_access = R"V0G0N(
# pyremote example script for data acess/manipulation/analysis

import os
import array
import numpy
import math

homedir = os.environ['HOME']
basedir=homedir+"/mydatafolder"
print ("Loading Scans for Analysis...")
# CH0 : load scan
gxsm.load (0,basedir+"/"+"mydatafileU001.nc")
# CH1
gxsm.load (1,basedir+"/"+"mydatafileV001.nc")

# fetch dimensions
dims=gxsm.get_dimensions(0)
print (dims)
geo=gxsm.get_geometry(0)
print (geo)
diffs_u=gxsm.get_differentials(0)
print (diffs_u)
diffs_v=gxsm.get_differentials(1)
print (diffs_v)

# create empty scratch array
m = numpy.zeros((dims[1],dims[0]), dtype=float)

print ("calculating...")

# do some fancy math now

gxsm.progress ("Fancy Py Calculus in Porgress", -1) # Init GXSM Progress Display
# go over all pixels
for y in range (0,dims[1]):
    gxsm.progress ("y="+str(y), float(y)/dims[1]) # Update Progress
    for x in range (0, dims[0]):
        m[y][x]=0
        min=1000.
        # find minimul of all data layer in both scans...
        for v in range (0, dims[2]):
            u=gxsm.get_data_pkt (0, x, y, v, 0)*diffs_u[2]
            v=gxsm.get_data_pkt (1, x, y, v, 0)*diffs_v[2]
            if min > u:
                min = u
            if min > v:
                min = v
        m[y][x]=min

gxsm.progress ("Fancy Calc done", 2.) # Close Progress Info

# convert 2d array into single memory2d block
n = numpy.ravel(m) # make 1-d
mem2d = array.array('f', n.astype(float)) 

# CH2 : activate ch 2 and create scan with resulting image data from memory2d
gxsm.chmodea (2)
gxsm.createscanf (2,dims[0],dims[1], geo[0], geo[1], mem2d)

# be nice and auto update/autodisp
gxsm.chmodea (2)
gxsm.direct ()
gxsm.autodisplay ()

print ("update displays completed.")
)V0G0N";


const gchar *template_data_cfextract_simple = R"V0G0N(
# Real world working on data script. 
# You need to adjust input files, z range, settings below.
import os
import array
import numpy
import math

# use simulation file
homedir = os.environ['HOME']
basedir=homedir+"/BNLBox/Data/Pohl/5TTPO1Au-Simulation"
print ("Loading Scans for Analysis...")
# CH0 : load pre processed frequency shift data (const height force maps) in ch 0
gxsm.load (0,basedir+"/"+"2TTPO1PQAu1-sim-df-inlayer.nc")
#gxsm.load (0,basedir+"/"+"1TTPO-sim-df-inlayer.nc")
#gxsm.load (0,basedir+"/"+"1TTPO-sim-df-fzlim-inlayer.nc")

# fetch dimensions
dims=gxsm.get_dimensions(0)
print (dims)
geo=gxsm.get_geometry(0)
print (geo)
diffs_f=gxsm.get_differentials(0)
print (diffs_f)

# create empty scratch array
m = numpy.zeros((dims[1],dims[0]), dtype=float)

print ("calculating...")

vmax=dims[2]

# adjust Z range:
zdata=numpy.arange (19.5,8.5, -0.05)
print (zdata)
dz=-0.05

flv = 35.0 # Hz, frq. level to lock to
rhs = 1    # use righ hand side of f-min
lhs = 0   # use left hand side of f-min

interlevel=8 # interpolation level
zdata_fine=numpy.arange (zdata[0], zdata[zdata.size-1], -(zdata[0]-zdata[1])/interlevel)
#print (zdata_fine)

gxsm.progress ("CdF Image Extract", -1) # Init GXSM Progress Display
# primitiv slow test run
for y in range (0,dims[1]):
	gxsm.progress ("y="+str(y), float(y)/dims[1]) # Update Progress
	for x in range (0, dims[0]):
		m[y][x]=0
		zmin=1000.
		minz=1000.
		minf=1000.
		minv=0
		# Locate Minimum in df curve
		for v in range (0, vmax):
			df=gxsm.get_data_pkt (0, x, y, v, 0)*diffs_f[2]
			z=zdata[v]
			if zmin > z:
				zmin=z
			if minf > df:
				minf=df
				minz=z
				minv=v
			if df > flv+5:
				break
		m[y][x]=minz
		# find df at RHS, use interpol, replace z
		if rhs:
			m[y][x]=zmin
			for v in range (interlevel*minv, interlevel*vmax):
				vi=float(v)/interlevel
				df=gxsm.get_data_pkt (0, x, y, vi, 0)*diffs_f[2]
				z=zdata_fine[v]
				if df > flv:		
					m[y][x]=z
					break
		# find df at LHS, use interpol, replace z
		if lhs:# or m[y][x]==zmin:
			m[y][x]=zmin
			for v in range (interlevel*vmax, 0, -1):
				vi=float(v)/interlevel
				df=gxsm.get_data_pkt (0, x, y, vi, 0)*diffs_f[2]
				z=zdata_fine[v]
				if df > flv:		
					m[y][x]=z
					break
						
gxsm.progress ("CdF Image Extract", 2.) # Close Progress Info

# convert 2d array into single memory2d block
n = numpy.ravel(m) # make 1-d
mem2d = array.array('f', n.astype(float)) 

# CH2 : activate ch 2 and create scan with resulting image data from memory2d
gxsm.chmodea (2)
gxsm.createscanf (2,dims[0],dims[1],1, geo[0], geo[1], mem2d)
gxsm.add_layerinformation ("@ "+str(flv)+" Hz",10)

# be nice and auto update/autodisp
gxsm.chmodea (2)
gxsm.direct ()
gxsm.autodisplay ()

print ("update displays completed.")
)V0G0N";

const gchar *template_data_cfextract_data = R"V0G0N(
# Real world working on data script. 
# You need to adjust input files and settings below.
import os
import array
import numpy
import math

homedir = os.environ['HOME']
basedir=homedir+"/BNLBox/Data/Pohl/20170612-Au-TTPO-exorted-data"
print ("Loading Scans for Analysis...")
# CH0 : load pre processed frequency shift data (const height force maps) in ch 0
gxsm.load (0,basedir+"/"+"Au111-TTPO-V3004-3025-Xp-PLL-Exci-Frq-dirftcorrected-cropped-LOG12-f0-inlayer.nc")
# CH1 : load pre processed matching topo data (actual const height value, may deviate from const number if compliance mode is used) in ch 1
gxsm.load (1,basedir+"/"+"Au111-TTPO-V3004-3025-Xp-PLL-Topo-dirftcorrected-cropped317-inlayer.nc")

# fetch dimensions
dims=gxsm.get_dimensions(0)
print (dims)
geo=gxsm.get_geometry(0)
print (geo)
diffs_f=gxsm.get_differentials(0)
print (diffs_f)
diffs_z=gxsm.get_differentials(1)
print (diffs_z)

# create empty scratch array
m = numpy.zeros((dims[1],dims[0]), dtype=float)

print ("calculating...")

flv = -1.0 # Hz, frq. level to lock to
rhs = 1    # use righ hand side of f-min
lhs = 0   # use left hand side of f-min

interlevel=8 # interpolation level

gxsm.progress ("CdF Image Extract", -1) # Init GXSM Progress Display
# primitiv slow test run
for y in range (0,dims[1]):
	gxsm.progress ("y="+str(y), float(y)/dims[1]) # Update Progress
	for x in range (0, dims[0]):
		m[y][x]=0
		zmin=1000.
		minz=1000.
		minf=1000.
		minv=0
		# Locate Minimum in df curve
		for v in range (0, dims[2]):
			df=gxsm.get_data_pkt (0, x, y, v, 0)*diffs_f[2]
			z=gxsm.get_data_pkt (1, x, y, v, 0)*diffs_z[2]
			if zmin > z:
				zmin=z
			if minf > df:
				minf=df
				minz=z
				minv=v
		m[y][x]=minz
		# find df at RHS, use interpol, replace z
		if rhs:
			m[y][x]=zmin
			for v in range (interlevel*minv, interlevel*dims[2]):
				vi=float(v)/interlevel
				df=gxsm.get_data_pkt (0, x, y, vi, 0)*diffs_f[2]
				z=gxsm.get_data_pkt (1, x, y, vi, 0)*diffs_z[2]
				if df > flv:		
					m[y][x]=z
					break
		# find df at LHS, use interpol, replace z
		if lhs:
			m[y][x]=zmin
			for v in range (interlevel*dims[2], 0, -1):
				vi=float(v)/interlevel
				df=gxsm.get_data_pkt (0, x, y, vi, 0)*diffs_f[2]
				z=gxsm.get_data_pkt (1, x, y, vi, 0)*diffs_z[2]
				if df > flv:		
					m[y][x]=z
					break
						
gxsm.progress ("CdF Image Extract", 2.) # Close Progress Info

# convert 2d array into single memory2d block
n = numpy.ravel(m) # make 1-d
mem2d = array.array('f', n.astype(float)) 

# CH2 : activate ch 2 and create scan with resulting image data from memory2d
gxsm.chmodea (2)
gxsm.createscanf (2,dims[0],dims[1],1, geo[0], geo[1], mem2d)
gxsm.add_layerinformation ("@ "+str(flv)+" Hz",10)

# be nice and auto update/autodisp
gxsm.chmodea (2)
gxsm.direct ()
gxsm.autodisplay ()

print ("update displays completed.")
)V0G0N";


const gchar *template_animate = R"V0G0N(
# Set a channel to Surface 3D view mode, remote will only apply to the current active GL Scen Setup/Control.

ti=0 # start at t-initial = 0
tf=200 # assume dt = 1 about 1/10s

phii=-120.0
phif=70.0

fovi=60.0
fovf=20.0
for t in range(ti, tf,1):
	gxsm.set("rotationz",str(phii+(phif-phii)*t/tf)) # adjust Z rotation (or any of the 3D view settings)
	gxsm.set("fov",str(fovi+(fovf-fovi)*t/tf)) # adjust Z rotation (or any of the 3D view settings)
	gxsm.set("animationindex",str(t+1)) # this will save the frame on every new index value!
	gxsm.sleep(0.01) # give GXSM a moment to update displays...
	#gxsm.sleep(1) # give GXSM a moment to update displays...
)V0G0N";


const gchar *template_movie_drawing_export = R"V0G0N(
# Save Drawing (many layers/times) as png image file series (via Cairo)

ch=0 # scratch channel used
output_filebasename = "/tmp/test" # export file path and file name base, a count is added

#gxsm.load (ch, "my_multilayer_or_time_data_file.nc") # or pre load manually in channel 1 and leave this commented out

[nx,ny,nv,nt]=gxsm.get_dimensions (ch)
for time in range (0,nt):
        # gxsm.set ("TimeSelect", str(time)) ## will also work as a generic example using entry set controls
	for layer in range (0,nv):
		print (time, layer);
		# gxsm.set ("LayerSelect", str(layer)) ## will also work as a generic example using entry set controls
		gxsm.set_view_indices (ch, time, layer) ## build in directly set both for ch is more efficient
		# gxsm.autodisplay () ## if you like, uncomment this, else manually set high/low limitis in viewcontrol to keep fixed
		gxsm.sleep(0.1) # give gxsm a moment to update
		gxsm.save_drawing (ch, time, layer, output_filebasename+'_T%d'%time+'_L%d.png'%layer )
)V0G0N";

const gchar *template_watchdog = R"V0G0N(
# Watch dog script. Watching via RTQuery system parameters:
# for example dF and if abs(dF) > limit DSP_CMD_STOPALL is issued (cancel auto approch, etc.)

limit = 15.
df=0.
while abs(df) < limit: 
	gxsm.sleep (10)
	fvec=gxsm.rtquery ("f")
	df = fvec[0]
	print "dF=",df
	gxsm.logev("Watchdog dF=%gHz"%df)

gxsm.action ("DSP_CMD_STOPALL")
gxsm.logev("Watchdog DSP_CMD_STOPALL ACTION as of dF=%gHz"%df)
print("Watchdog Abort")
)V0G0N";

/*
const gchar *template_name = R"V0G0N(
...py script ...
)V0G0N";
*/
