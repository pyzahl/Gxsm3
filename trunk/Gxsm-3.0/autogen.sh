#!/bin/sh 

# This script does all the magic calls to automake/autoconf and
# friends that are needed to configure a cvs checkout.  As described in
# the file HACKING you need a couple of extra tools to run this script
# successfully.
#
# If you are compiling from a released tarball you don't need these
# tools and you shouldn't use this script.  Just call ./configure
# directly.


PROJECT="GXSM Gnome X Scanning Microscopy program"

echo
echo "Checking for symlink gxsm:"
if test -L "gxsm"; then
    echo "fine, symlink gxsm -> gxsm3 exists!"
else
    echo "creating symlink gxsm -> gxsm3"
    ln -s gxsm3 gxsm
fi
	    
test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

OLDDIR=`pwd`
cd $srcdir

AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
        echo "*** No autoreconf found, please intall it ***"
        exit 1
fi

INTLTOOLIZE=`which intltoolize`
if test -z $INTLTOOLIZE; then
        echo "*** No intltoolize found, please install the intltool package ***"
        exit 1
fi

GNOMEDOC=`which yelp-build`
if test -z $GNOMEDOC; then
        echo "*** The tools to build the documentation are not found,"
        echo "    please intall the yelp-tools package ***"
        exit 1
fi

GTKDOCIZE=`which gtkdocize`
if test -z $GTKDOCIZE; then
        echo "*** No GTK-Doc found, please install it ***"
        exit 1
fi

#
# CVS cannot save symlinks, we have to create them
# ourselves.
#
for i in gmetopng.1 rhkspm32topng.1 scalatopng.1
do
    if test -L "man/$i"; then
        echo "Fine, symlink for man-page $i exists!"
    else
        echo "creating symlink for $i"
        ln -s nctopng.1 man/$i
    fi
done

# end of creation of man-page symlinks.

gnome-autogen.sh $@

echo
echo "Now type 'make' to compile the $PROJECT."
echo "Speed-up hint: try 'make -j12' for fully utilizing a 12x multi-core/-threadded/CPU system for example ;-)"
echo
