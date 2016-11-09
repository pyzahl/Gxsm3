#!/bin/sh
# =*dI/dV / I/V*= Gxsm gri file Menuentry
#
# This script is part of GXSM2. gxsm.sf.net and is copyrighted under GPL.
#

DATAFILE=$1
if [ -z $2 ]; then
	TITLE="Gxsm Profile Plot"
else
	TITLE="$2"
fi

XLABEL=`cat $DATAFILE | grep Xlabel | cut -f2 -d"="`
YLABEL=`cat $DATAFILE | grep Ylabel | cut -f2 -d"="`
XUNIT=`cat $DATAFILE | grep " Xunit " | cut -f2 -d"=" | sed 's/ //g'`
YUNIT=`cat $DATAFILE | grep " Yunit " | cut -f2 -d"=" | sed 's/ //g'`

TMP1=`mktemp /tmp/GXSMPROFILEPLOT1.XXXXXX`.gri || exit 1
TMP2=`mktemp /tmp/GXSMPROFILEPLOT2.XXXXXX`.asc || exit 1
TMP3=`mktemp /tmp/GXSMPROFILEPLOT3.XXXXXX`.py || exit 1
OUT=`basename $0 .gri.sh`.ps

#####################################################
cat > "$TMP1" << EOF
#!/usr/bin/gri
set x name "U [V]"
set y name "(dI/dV) / (I/U)"

# {rpn 2 argv}

open "$TMP2"
read columns x y
close
draw curve
 
draw title "$TITLE  STS-calculation"
EOF
#####################################################
cat > "$TMP3" << EOF
#! /usr/bin/env python

import posix
import string
import math
import Numeric
import MLab

numavg=40
ylimit=10

def readgxsmprofile(filename):
      xydatafile = open(filename, 'r')
      i = 0
      for line in xydatafile.readlines():
            record = string.split(line)
            if record[0] == '#':
                  if record[1] == 'Anz':
                        points = int(record[3])
                        x = Numeric.array([],'d')
                        x.resize(points)
                        y = Numeric.array([],'d')
                        y.resize(points)
                  continue
            else:
                  getxy = '0'
                  for token in record[:]:
                        if token == 'InUnits:':
                              getxy = 'x'
                              continue
                        if getxy == 'x':
                              x[i] = float(token)
                              getxy = 'y'
                              continue
                        if getxy == 'y':
                              y[i] = float(token)
                              getxy = 'E'
                              i=i+1
                              break;
      return points, x, y

# make gausian smoothing window function

# use navg points
navg=numavg

def gaus(x):
      return math.exp(-x*x*0.01)

def gausc(i):
      return gaus(i-navg/2)

gausarr = Numeric.array([], 'd')
gausarr.resize(navg)
for i in range(navg):
      gausarr[i] = gausc(i)

# norm area to 1
sum=0
for q in gausarr:
      sum += q;

gausarr /= sum

# read xy data
numpoints, x, y = readgxsmprofile("$DATAFILE")

# do convolution with gausian window
ysmooth = Numeric.convolve(gausarr, y, 1)

dy = MLab.diff(ysmooth,1)
dx = MLab.diff(x,1)

dIdV = Numeric.array(dy/dx)
norm = Numeric.array(ysmooth/x)

dIdVNorm = Numeric.array(dIdV/norm[0:numpoints-1])

# limit Y range
for i in range(numpoints-1):
      if abs(dIdVNorm[i]) > ylimit:
	  dIdVNorm[i] *= ylimit/dIdVNorm[i];

for i in range(numpoints-1):
      print x[i], dIdVNorm[i]

EOF
###################################################
python "$TMP3" > "$TMP2"

gri -output "$OUT" "$TMP1" && gv $OUT

rm -f "$TMP1" "$TMP2" "$TMP3"
