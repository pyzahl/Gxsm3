#!/bin/bash
# This build_helper.sh script is part of Gxsm.
# Use it to conveniently build Gxsm.

# Use one of the following arguments.
# 'prep' shall run any commands to 'prepare' the system for the build.
# 'build' shall build the binaries.
# 'install' shall install the binaries system-wide (may require sudo).
# 'package' shall build a .deb-package.

if [ ! -f /etc/os-release ] ; then
   echo "There is no /etc/os-release. "
   echo "This is not a supported distro. Giving up."
   exit
fi

source /etc/os-release

arg=$1

echo "This is a $ID $VERSION_ID system."

if [[ $arg == "prep" ]] ; then

    if [[ $ID == "debian" && $VERSION_ID == "11" ]] ; then
	sudo apt install -y autotools-dev debhelper fonts-freefont-ttf \
	freeglut3-dev gnome-common gsettings-desktop-schemas-dev \
	gtk-doc-tools intltool libfftw3-dev libgail-3-dev \
	libgl1-mesa-dev libglew-dev libglm-dev libgsl-dev \
	libgtk-3-0 libgtksourceview-3.0-dev \
	libgtksourceviewmm-3.0-dev \
	libnetcdf-cxx-legacy-dev libnetcdf-dev \
	libnlopt-dev libopencv-core-dev libopencv-features2d-dev \
	libopencv-highgui-dev libopencv-objdetect-dev libquicktime-dev \
	libsoup2.4-dev subversion yelp-tools libpython3-dev \
        libvala-0.48-dev libenchant-2-dev python3-dev python3-numpy \
        libopencv-video-dev libopencv-calib3d-dev libpopt-dev
    elif [[ $ID == "ubuntu" && $VERSION_ID == "22.04" ]] ; then
	sudo apt install -y autotools-dev debhelper fonts-freefont-ttf \
	freeglut3-dev gnome-common gsettings-desktop-schemas-dev \
	gtk-doc-tools intltool libfftw3-dev libgail-3-dev \
	libgl1-mesa-dev libglew-dev libglm-dev libgsl-dev \
	libgtk-3-0 libgtksourceview-3.0-dev \
	libgtksourceviewmm-3.0-dev \
	libnetcdf-cxx-legacy-dev libnetcdf-dev \
	libnlopt-dev libopencv-core-dev libopencv-features2d-dev \
	libopencv-highgui-dev libopencv-objdetect-dev libquicktime-dev \
	libsoup2.4-dev subversion yelp-tools libpython3-dev \
        libvala-0.56-dev libenchant-2-dev python3-dev python3-numpy \
        libopencv-video-dev libopencv-calib3d-dev libpopt-dev
    else
        echo "This is not a supported distro. "
        echo "Please file an issue to support '$ID $VERSION_ID'"
        exit
    fi
    echo "Now you can use the 'build' argument."
elif [[ $arg == "build" ]] ; then
    ./autogen.sh
    ./configure
    make
elif [[ $arg == "install" ]] ; then
    sudo make install
elif [[ $arg == "package" ]] ; then
    dpkg-buildpackage -us -uc
else 
    echo "Missing command. Use 'prep'."
fi
