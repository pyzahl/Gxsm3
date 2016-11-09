#!/bin/sh

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
OUT=`basename $0 .gri.sh`.ps

grep -v "#" "$DATAFILE" |\
cut -f 2 -d':' > "$TMP2"

cat > "$TMP1" << EOF
#!/usr/bin/gri
set x name "$XLABEL[$XUNIT]"
set y name "$YLABEL[$YUNIT]"


set page size letter
set page landscape
# set page translate 0 0
# set page factor 1.33
set y size 13
set x size 20
set y type log

# {rpn 2 argv}

open "$TMP2"
read columns x y
close
draw curve
 
draw title "$TITLE"
EOF
gri -output "$OUT" "$TMP1" && gv $OUT

rm -f "$TMP1" "$TMP2"
