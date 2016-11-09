#!/bin/bash
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
NSETS=`cat $DATAFILE | grep " NSets " | cut -f2 -d"=" | sed 's/ //g'`

# NSETS + 3 -> X
# NSETS + 4... -> Y1,Y2,...Y_NSETS

set -i CX
set -i CY1
CX=$(($NSETS+3))

columns=''
for ((cy=1; cy <= NSETS ; cy++))
do
  columns="$columns -bxy $CX:$(($NSETS+$cy+3)) "
done         
#echo $columns

xmgrace  -graph 0 -pexec "title \"$TITLE\""  \
-pexec "xaxis label \"$XLABEL \[$XUNIT\] \"" -pexec "yaxis label \"$YLABEL \[$YUNIT\]\"" \
-block $DATAFILE $columns
