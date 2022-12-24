#!/bin/bash
# Run this if you want to compile the Gxsm-Manual.
# This script will _NOT_ create the PDF, but it
# will install the packages necessary for building
# it.
# This file is part of Gxsm

if [[ $EUID != 0 ]] ; then
	echo "ERROR: This script must be run as root, because it wants to install packages."
	exit
fi

if [[ ! -f /etc/os-release ]] ; then
	echo "ERROR: This tool requires /etc/os-release. Sorry."
	exit
fi

. /etc/os-release

distrospecific="helpers/bootstrap.d/recipe_${ID}_${VERSION_ID}.sh"

if [[ -f $distrospecific ]] ; then
	. $distrospecific
else
	echo "There is no recipe for this Linux distribution ($ID, $VERSION_ID)"
	echo "Please file a bug-report at http://gxsm.sf.net"
fi

