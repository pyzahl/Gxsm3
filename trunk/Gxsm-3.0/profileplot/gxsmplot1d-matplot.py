#!/usr/bin/python

#
# This script is part of GXSM2. gxsm.sf.net and is copyrighted under GPL.
#

from pylab import *
import sys
import string

f = open(sys.argv[1], 'r')
alllines = f.readlines()
f.close()

j = []
k = []

PLOT_TITLE = "Gxsm plot"

for i in alllines:
  if string.find(i,'Title') != -1:
      PLOT_TITLE = string.split(i, '= ')[1][:-1]
  if string.find(i,'Xlabel') != -1:
      XLABEL1 = string.split(i, '= ')[1][:-1]
  if string.find(i,'Xalias') != -1:
      XLABEL2 = string.split(i, '= ')[1][:-1]
  if string.find(i,'Ylabel') != -1:
      YLABEL1 = string.split(i, '= ')[1][:-1]
  if string.find(i,'Yalias') != -1:
      YLABEL2 = string.split(i, '= ')[1][:-1]
  try:
    n = float(i.split()[0])
    m = float(i.split()[-1])
    j.append(n)
    k.append(m)
  except:
    pass
  
subplot(111)
plot(j, k, 'k')
grid('on')

if PLOT_TITLE == 'title not set':
  PLOT_TITLE = '-=Gxsm plot=-'
title(PLOT_TITLE)
xlabel(XLABEL1 + '[' + XLABEL2 + ']')
ylabel(YLABEL1 + '[' + YLABEL2 + ']')

show()
