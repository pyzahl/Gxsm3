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

(test -f configure.ac) || {
        echo "*** ERROR: Directory '$srcdir' does not look like the top-level project directory ***"
        exit 1
}

# shellcheck disable=SC2016
PKG_NAME=$(autoconf --trace 'AC_INIT:$1' configure.ac)

if [ "$#" = 0 -a "x$NOCONFIGURE" = "x" ]; then
        echo "*** WARNING: I am going to run 'configure' with no arguments." >&2
        echo "*** If you wish to pass any to it, please specify them on the" >&2
        echo "*** '$0' command line." >&2
        echo "" >&2
fi

aclocal --install || exit 1
# gtkdocize --copy || exit 1
intltoolize --force --copy --automake || exit 1
# glib-gettextize --force --copy || exit 1
autoreconf --verbose --force --install || exit 1

cd "$olddir"
if [ "$NOCONFIGURE" = "" ]; then
        $srcdir/configure "$@" || exit 1

        if [ "$1" = "--help" ]; then
                exit 0
        else
                echo "Now type 'make' to compile $PKG_NAME" || exit 1
        fi
else
        echo "Skipping configure process."
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

# gnome-autogen.sh $@

echo
echo "Now type 'make' to compile the $PROJECT."
echo "Speed-up hint: try 'make -j12' for fully utilizing a 12x multi-core/-threadded/CPU system for example ;-)"
echo
