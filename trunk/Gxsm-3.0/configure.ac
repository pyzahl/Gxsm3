dnl Process this file with autoconf to produc a configure script.
# grep -rl gtk_object_ . | xargs sed -i s/gtk_object_/g_object_/g

AC_PREREQ(2.63.2)

m4_define(gxsm3_major_version, 3)
m4_define(gxsm3_minor_version, 41)
m4_define(gxsm3_micro_version, 0)
m4_define(gxsm3_version, gxsm3_major_version.gxsm3_minor_version.gxsm3_micro_version)

AC_INIT([gxsm3],
	[gxsm3_version],
	[http://sourceforge.net/tracker/?func=add&group_id=12992&atid=112992],
	[gxsm3],
	[http://gxsm.sf.net])

AC_DEFINE_UNQUOTED(GXSM_VERSION_NAME, "Red Force One","GXSM version identifier screen name")
AC_DEFINE_UNQUOTED(GXSM_WIDGET_PAD, 5, "default widget padding (replaces obsolete GNOME_PAD)")
AC_DEFINE_UNQUOTED(MENU_AT_POINTER(M,E), gtk_menu_popup_at_pointer (M, E), "for gtk-3.22 and up")
# AC_DEFINE_UNQUOTED(MENU_AT_POINTER(M,E), gtk_menu_popup (M, NULL, NULL, NULL, NULL, E->button.button, E->button.time), "for gtk versions older than 3.22")
AC_DEFINE_UNQUOTED(GXSM_RES_BASE_PATH_DOT,     "org.gnome.gxsm3",              "GSettings and Builder dot base path without leadiong or trailing slashes")
AC_DEFINE_UNQUOTED(GXSM_RES_BASE_PATH,         "org/gnome/gxsm3",              "GSettings and Builder base path without leadiong or trailing slashes")
AC_DEFINE_UNQUOTED(GXSM_RES_PREFERENCES_PATH,  "org/gnome/gxsm3/preferences",  "GSettings path for preferences")
AC_DEFINE_UNQUOTED(GXSM_RES_GL_PATH,           "org/gnome/gxsm3/gl",           "GSettings path for gl preferences")

## AC_DEFINE_UNQUOTED(GXSM_ENABLE_SCAN_USER_PLUGIN_PATH, "yes", "enable scanning in user path for plugins");

# Dependencies
GLIB_REQUIRED=2.48.1
 
# for gtk+3.0 >= 3.22
GTK_REQUIRED=3.22.0
GDK_REQUIRED=3.22.0

# for gtk+3.0 3.20.0
# GTK_REQUIRED=3.20.0
# GDK_REQUIRED=3.20.0
# AC_DEFINE_UNQUOTED(MENU_AT_POINTER(M,E), gtk_menu_popup (M, NULL, NULL, NULL, NULL, E->button.button, E->button.time), "for gtk versions older than 3.22")

GTKSOURCEVIEW_REQUIRED=3.12.0
# LIBPEAS_REQUIRED=1.7.0
LIBXML_REQUIRED=2.5.0
# ENCHANT_REQUIRED=1.2.0
# ISO_CODES_REQUIRED=0.35
PYGOBJECT_REQUIRED=3.0.0

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_SRCDIR(gxsm3/gxsm_main.C)
AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE(GXSM3_MAJOR_VERSION, gxsm3_major_version, [Gxsm3 major version])
AC_SUBST(GXSM3_MAJOR_VERSION, gxsm3_major_version)
AC_DEFINE(GXSM3_MINOR_VERSION, gxsm3_minor_version, [Gxsm3 minor version])
AC_SUBST(GXSM3_MINOR_VERSION, gxsm3_minor_version)
AC_DEFINE(GXSM3_MICRO_VERSION, gxsm3_micro_version, [Gxsm3 micro version])
AC_SUBST(GXSM3_MICRO_VERSION, gxsm3_micro_version)

AM_INIT_AUTOMAKE([1.11 tar-ustar dist-xz no-dist-gzip -Wno-portability subdir-objects])
AM_MAINTAINER_MODE([enable])
# AM_GNU_GETTEXT([external])

AM_SILENT_RULES([yes])

# Checks for programs
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_YACC
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_SYS_LARGEFILE
PKG_PROG_PKG_CONFIG
# needed on osx
AC_PROG_OBJC


# Internationalisation
GETTEXT_PACKAGE=gxsm3
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],[gettext package name])
IT_PROG_INTLTOOL([0.50.2])

# NetCDF
libnetcdf_LIBS="-lnetcdf -lnetcdf_c++"
AC_SUBST(libnetcdf_LIBS)

# NEW -- bullshit non compat
# AX_LIB_NETCDF4()


# Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT(disable-static)

dnl check for win32 platform
AC_MSG_CHECKING([for Win32 platform])
case "$host" in
  *-*-mingw*|*-*-cygwin*)
        archive_cmds='$CC -shared $libobjs $deplibs $compiler_flags -o $output_objdir/$soname ${wl}--enable-auto-image-base -Xlinker --output-def -Xlinker $lib.def && $DLLTOOL --dllname $soname --def $lib.def --output-delaylib $lib'
	# If the export-symbols file already is a .def file (1st line
	# is EXPORTS), use it as is; otherwise, prepend...
	archive_expsym_cmds='if test "x`$SED 1q $export_symbols`" = xEXPORTS; then
	  cp $export_symbols $output_objdir/$soname.def;
	else
	  echo EXPORTS > $output_objdir/$soname.def;
	  cat $export_symbols >> $output_objdir/$soname.def;
	fi~
	$CC -shared $output_objdir/$soname.def $libobjs $deplibs $compiler_flags -o $output_objdir/$soname ${wl}--enable-auto-image-base;
	$DLLTOOL --dllname $soname --def $output_objdir/$soname.def --output-delaylib $lib'
    platform_win32=yes
    ;;
  *)
    platform_win32=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")

AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    AC_CHECK_TOOL(WINDRES, windres)
    ACTIVE_PLUGINS="${ACTIVE_PLUGINS}, 'checkupdate'"
    ;;
  *)
    os_win32=no
    ;;
esac
AC_MSG_RESULT([$os_win32])
AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")

dnl check for osx platform
AC_MSG_CHECKING([for Mac OS X platform])
case "$host" in
  *-*-darwin*)
    platform_osx=yes
    ;;
  *)
    platform_osx=no
    ;;
esac
AC_MSG_RESULT([$platform_osx])
AM_CONDITIONAL(PLATFORM_OSX, test "$platform_osx" = "yes")

dnl check for native osx
gdk_targets=`$PKG_CONFIG --variable=targets gdk-3.0`

gdk_windowing="x11"

for target in $gdk_targets;
do
	case "$target" in
		quartz)
			gdk_windowing=quartz
			;;
		win32)
			gdk_windowing=win32
			;;
	esac
done

AC_MSG_CHECKING([for native Mac OS X])
if test "$gdk_windowing" = "quartz"; then
	os_osx=yes
	ACTIVE_PLUGINS="${ACTIVE_PLUGINS}, 'checkupdate'"
else
	os_osx=no
fi
AC_MSG_RESULT([$os_osx])
AM_CONDITIONAL(OS_OSX, test "$os_osx" = "yes")

AM_CONDITIONAL(GDK_WINDOWING_X11, test "$gdk_windowing" = "x11")

if test "$platform_osx" = "yes"; then
	AC_DEFINE([PLATFORM_OSX],[1],[Defined if platform is Mac OSX])
fi

AC_SUBST(DEFAULT_FONT)
AC_SUBST(DEFAULT_PRINT_FONT)

if test "$os_osx" = "yes"; then
	AC_DEFINE([OS_OSX],[1],[Defined if os is Mac OSX])

	PKG_CHECK_MODULES(GTK_MAC, gtk-mac-integration)

	AC_SUBST(GTK_MAC_CFLAGS)
	AC_SUBST(GTK_MAC_LIBS)
fi

dnl ===============================================================
dnl Expanded dirs
dnl ===============================================================

# This macro expands DIR and assigns it to RET.
# If DIR is NONE, then it's replaced by DEFAULT.
# Based on AC_DEFINE_DIR
AC_DEFUN([GXSM3_FULLPATH], [
  test "x$prefix" = xNONE && prefix="$ac_default_prefix"
  test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
  ac_expand=[$]$1
  test "x$ac_expand" = xNONE && ac_expand="[$]$2"
  ac_expand=`eval echo [$]ac_expand`
  $3=`eval echo [$]ac_expand`
])

dnl FULL_LIBEXECDIR is used for X-GNOME-Bugzilla-ExtraInfoScript expansion
dnl in data/org.gnome.gxsm3.desktop.in.in
GXSM3_FULLPATH(libexecdir, NONE, FULL_LIBEXECDIR)
AC_SUBST(FULL_LIBEXECDIR)


dnl ================================================================
dnl Use gvfs metadata or the old xml file store.
dnl ================================================================

if test "x$os_win32" != "xno" || test "x$os_osx" != "xno"; then
	gvfs_metadata_default="no"
else
	gvfs_metadata_default="yes"
fi

AC_ARG_ENABLE([gvfs-metadata],
	AS_HELP_STRING([--enable-gvfs-metadata], [Enable using gvfs to store metadata]),
	[enable_gvfs_metadata=$enableval],
	[enable_gvfs_metadata=$gvfs_metadata_default])

AM_CONDITIONAL(ENABLE_GVFS_METADATA, test "x$enable_gvfs_metadata" != "xno")

if test "x$enable_gvfs_metadata" != "xno"; then
	AC_DEFINE([ENABLE_GVFS_METADATA], [1], [Define to enable using gvfs metadata])
fi

dnl ================================================================
dnl check update plugin
dnl ================================================================

AC_ARG_ENABLE([updater],
	AS_HELP_STRING([--disable-updater],[Disable check update plugin (default: enabled)]),
	[enable_updater=$enableval],
	[enable_updater=yes])

if test "x$enable_updater" != "xno"; then
	PKG_CHECK_MODULES(LIBSOUP, \
		libsoup-2.4,
		have_libsoup=yes,
		have_libsoup=no)

	if test "x$have_libsoup" = "xyes"; then
		if test "x$os_win32" = "xyes"; then
			enable_updater=yes
		elif test "x$os_osx" = "xyes"; then
			enable_updater=yes
		else
			enable_updater=no
		fi
	else
		enable_updater=no
	fi
fi

AM_CONDITIONAL(ENABLE_UPDATER, test x"$enable_updater" = "xyes")

dnl ================================================================
dnl Start of pkg-config checks
dnl ================================================================

############################### KEEPING

AC_ARG_ENABLE( one-plugin-dir,
[  --enable-one-plugin-dir  use a single plugin dir [default=no]],,
                enable_one_plugin_dir="no")

AC_ARG_ENABLE( user-plugin-dir,
[  --disable-user-plugin-dir disable per-user plugin dir],,
                enable_user_plugin_dir="yes")

AC_ARG_ENABLE(gtksourceview, AC_HELP_STRING([--enable-gtksourceview],
                                    [Syntax highlighting in Python wrapper console]),[],
                                    [enable_gtksourceview=yes])

dnl Subst proivate PACKAGE_ICON_DIR and set it in config.h.
if test "x${prefix}" = "xNONE"; then
        AC_DEFINE_UNQUOTED(PACKAGE_ICON_DIR, "${ac_default_prefix}/share/${PACKAGE}/icons",This is the path where icons will be instaled)
        PACKAGE_ICON_DIR="${ac_default_prefix}/share/${PACKAGE}/pixmaps"
else
        AC_DEFINE_UNQUOTED(PACKAGE_ICON_DIR, "${prefix}/share/${PACKAGE}/icons",This is the path where icons will be instaled)
        PACKAGE_ICON_DIR="${prefix}/share/${PACKAGE}/icons"
fi
AC_SUBST(PACKAGE_ICON_DIR)

dnl Subst PACKAGE_PIXMAPS_DIR and set it in config.h.
if test "x${prefix}" = "xNONE"; then
        AC_DEFINE_UNQUOTED(PACKAGE_PIXMAPS_DIR, "${ac_default_prefix}/share/pixmaps/${PACKAGE}",This is the path where pixmaps will be instaled)
        PACKAGE_PIXMAPS_DIR="${ac_default_prefix}/share/pixmaps/${PACKAGE}"
else
        AC_DEFINE_UNQUOTED(PACKAGE_PIXMAPS_DIR, "${prefix}/share/pixmaps/${PACKAGE}",This is the path where pixmaps will be instaled)
        PACKAGE_PIXMAPS_DIR="${prefix}/share/pixmaps/${PACKAGE}"
fi
AC_SUBST(PACKAGE_PIXMAPS_DIR)

dnl Subst PACKAGE_DATA_DIR and set it in config.h.
if test "x${prefix}" = "xNONE"; then
        AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}",This is the path where data will be instaled)
        PACKAGE_DATA_DIR="${ac_default_prefix}/share/${PACKAGE}"
else
        AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}",This is the path where data will be instaled)
        PACKAGE_DATA_DIR="${prefix}/share/${PACKAGE}"
fi
AC_SUBST(PACKAGE_DATA_DIR)

dnl Subst PACKAGE_DOC_DIR and set it in config.h.
if test "x${prefix}" = "xNONE"; then
        AC_DEFINE_UNQUOTED(PACKAGE_DOC_DIR, "${ac_default_prefix}/share/doc/${PACKAGE}",This is the path where docs will be instaled)
        PACKAGE_DOC_DIR="${ac_default_prefix}/share/doc/${PACKAGE}"
else
        AC_DEFINE_UNQUOTED(PACKAGE_DOC_DIR, "${prefix}/share/doc/${PACKAGE}",This is the path where docs will be instaled)
        PACKAGE_DOC_DIR="${prefix}/share/doc/${PACKAGE}"
fi
AC_SUBST(PACKAGE_DOC_DIR)

dnl Set PACKAGE_LOCALE_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${ac_default_prefix}/share/locale", [Define gxsm package local dir])
else
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${prefix}/share/locale", [Define gxsm package local dir])
fi

dnl Set PACKAGE_PLUGIN_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_PLUGIN_DIR, "${ac_default_prefix}/lib/${PACKAGE}-plugins", [Define gxsm package plugin search path])
else
  AC_DEFINE_UNQUOTED(PACKAGE_PLUGIN_DIR, "${prefix}/lib/${PACKAGE}-plugins", [Define gxsm package plugin search path])
fi

if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_PROFILEPLOT_DIR, "${ac_default_prefix}/share/${PACKAGE}/profileplot", [Define gxsm profileplot scripts])
else
  AC_DEFINE_UNQUOTED(PACKAGE_PROFILEPLOT_DIR, "${prefix}/share/${PACKAGE}/profileplot", [Define gxsm profileplot scripts])
fi

dnl Set PACKAGE_PALETTE_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_PALETTE_DIR, "${ac_default_prefix}/share/${PACKAGE}/palette", [Define gxsm package install prefix])
else
  AC_DEFINE_UNQUOTED(PACKAGE_PALETTE_DIR, "${prefix}/share/${PACKAGE}/palette", [Define gxsm package install prefix])
fi

#dnl Set PACKAGE_GRICMD_DIR in config.h.
#if test "x${prefix}" = "xNONE"; then
#  AC_DEFINE_UNQUOTED(PACKAGE_GRICMD_DIR, "${ac_default_prefix}/share/${PACKAGE}/gricmdfiles", [Define gxsm package path to gri cmd files])
#else
#  AC_DEFINE_UNQUOTED(PACKAGE_GRICMD_DIR, "${prefix}/share/${PACKAGE}/gricmdfiles", [Define gxsm package path to gri cmd files])
#fi
dnl Check if WORDS are big endian (as Motorola, Sparc -- not Intel)
AC_C_BIGENDIAN

## xjt does build, but it is more confusing than useful on Win32,
## as it wants to run tar, mv and bzip2, which seldom are available
AM_CONDITIONAL(BUILD_XJT, test x$jpeg_ok = xyes && test x"$os_win32" = xno)


################
# Check for libz
################

if test -z "$LIBZ"; then
  AC_CHECK_LIB(z, gzsetparams,
    [AC_CHECK_HEADER(zlib.h,
      PSP='psp$(EXEEXT)'; LIBZ='-lz',
      [AC_MSG_WARN(*** PSP and PNG plug-ins will not be built (ZLIB header files not found) ***)])],
    [AC_MSG_WARN(*** PSP and PNG plug-ins will not be built (ZLIB library not found) ***)])
fi

AC_SUBST(LIBZ)
AC_SUBST(PSP)


##################
# Check for libpng
##################

AC_ARG_WITH(libpng,  [  --without-libpng        build without PNG support])
  if test x$with_libpng != xno && test -z "$LIBPNG" && test -n "$LIBZ"; then
    AC_CHECK_LIB(png, png_read_info,
      [AC_CHECK_HEADER(png.h,
        png_ok=yes,
        png_ok=no)],
      [AC_MSG_WARN(*** PNG plug-in will not be built (PNG library not found) ***)], -lz -lm)
    if test "$png_ok" = yes; then
      AC_MSG_CHECKING([for png_structp in png.h])
      AC_TRY_COMPILE([#include <png.h>],
        [png_structp pp; png_infop info; png_colorp cmap; png_create_read_struct;],
        png_ok=yes,
        png_ok=no)
      AC_MSG_RESULT($png_ok)
      if test "$png_ok" = yes; then
        PNG='png$(EXEEXT)'; LIBPNG="-lpng $LIBZ"
      else
        AC_MSG_WARN(*** PNG plug-in will not be built (PNG library is too old) ***)
      fi
    else
      AC_MSG_WARN(*** PNG plug-in will not be built (PNG header file not found) ***)
    fi
  fi

  if test x$with_libpng != xno && test -z "$LIBPNG"; then
     AC_MSG_ERROR([
*** Checks for PNG library failed. You can build without it by passing
*** --without-libpng to configure but you won't be able to use PNGs then.])
  fi

AC_SUBST(PNG)
AC_SUBST(LIBPNG)

#############################################################################
# GtkSourceView
if test "x$enable_gtksourceview" != "xno"; then
  PKG_CHECK_MODULES(GTKSOURCEVIEW,[gtksourceview-3.0],
                    [AC_DEFINE(HAVE_GTKSOURCEVIEW,1,
                               [Define if we have the GtkSourceView package.])],
                    [enable_gtksourceview=no; GTKSOURCEVIEW_WARN=" (needs GtkSourceView)"])
fi
if test "x$enable_gtksourceview" != "xno"; then
  GTKSOURCEVIEW_DEPENDENCY=gtksourceview-3.0
else
  GTKSOURCEVIEW_DEPENDENCY=
  GTKSOURCEVIEW_CFLAGS="$GTK_CFLAGS"
  GTKSOURCEVIEW_LIBS="$GTK_LIBS"
fi
AC_SUBST([GTKSOURCEVIEW_DEPENDENCY])

############################################################
# Checks for FFTW header and library.
############################################################

# Installed fftw library, if any.
FFTW_LIBS='-lfftw3'
AC_SUBST(FFTW_LIBS)

#have_fftw_header=no
#AC_CHECK_HEADERS(fftw3.h, [have_fftw_header=yes; break])
#if test "$have_fftw_header" = yes; then
#    AC_CHECK_LIB(fftw3, fftw_plan_dft_r2c_2d, FFTW_LIBS="-lfftw3")
#else
#    with_fftw=no
#fi
#
#if test "$with_fftw" = yes; then
  FFT_DIR=''
  AC_DEFINE(HAVE_FFTW, 1, [Define if the FFTW library is available.])
#fi

AC_DEFINE_UNQUOTED(XSM_DEBUG_OPTION, "yes", Global Enable/Disable of any debugging code)

AC_DEFINE_UNQUOTED(HAS_MESAGL, "yes", Having Mesa/GL -- now needed!)

# Info only
AC_DEFINE_UNQUOTED(COMPILEDBYNAME, "$USER@`hostname`; `date`", GXSM user@host CC-time info string)


# Quicktime library for Im/Export Plugin
AC_CHECK_HEADERS(lqt/quicktime.h)

# Now requires Python 2.5 or higher!
AC_CHECK_LIB(python2.5, main)
AC_CHECK_HEADERS(python2.5/Python.h)

# Now requires Python 2.6 or higher!
AC_CHECK_LIB(python2.6, main)
AC_CHECK_HEADERS(python2.6/Python.h)

# Now requires Python 2.7 or higher!
AC_CHECK_LIB(python2.7, main)
AC_CHECK_HEADERS(python2.7/Python.h)



dnl ---------------------------------------------------------
dnl Architecture specific stuff
dnl ---------------------------------------------------------

###########################
# Check target architecture
###########################

AC_MSG_CHECKING([for target architecture])
case x"$target" in
  xNONE | x)
    target_or_host="$host" ;;
  *)
    target_or_host="$target" ;;
esac
AC_MSG_RESULT([$target_or_host])

case "$target_or_host" in
  i*86-*-*)
    have_x86=yes
    AC_DEFINE(ARCH_X86, 1, [Define to 1 if you are compiling for ix86.])
    ;;
  x86_64-*-*)
    have_x86=yes
    AC_DEFINE(ARCH_X86, 1, [Define to 1 if you are compiling for ix86.])
    AC_DEFINE(ARCH_X86_64, 1, [Define to 1 if you are compiling for amd64.])
    ;;
  ppc-*-* | powerpc-* | G4 | G5)
    have_ppc=yes
    AC_DEFINE(ARCH_PPC, 1, [Define to 1 if you are compiling for PowerPC.])
    CFLAGS="$CFLAGS -O3 -mtune=G4 -mcpu=G4 -ffast-math -maltivec -mabi=altivec"
    CXXFLAGS="$CXXFLAGS -O3 -mtune=G4 -mcpu=G4 -ffast-math -maltivec -mabi=altivec"
    ;;
  ppc64-*-* | powerpc64-*)
    have_ppc=yes
    AC_DEFINE(ARCH_PPC, 1, [Define to 1 if you are compiling for PowerPC.])
    AC_DEFINE(ARCH_PPC64, 1, [Define to 1 if you are compiling for PowerPC64.])
    ;;
  *)
    ;;
esac



echo --- System Architecture detection ---
echo Host CPU  : $host_cpu
echo Target  : $target

#case "$host_cpu" in
case "$target" in
ppc|G4)
        echo PPC: tuning gcc for G4, enabling Altivec
        CFLAGS="$CFLAGS -arch ppc -O3 -mtune=G4 -mcpu=G4 -ffast-math -maltivec -mabi=altivec"
        CXXFLAGS="$CXXFLAGS -arch ppc -O3 -mtune=G4 -mcpu=G4 -ffast-math -maltivec -mabi=altivec"
        USE_ALTIVEC=yes
        AC_DEFINE([HAVE_ALTIVEC],1,[Processor support ALTIVEC])
        BUILD_ALTIVEC=1
        ;;
ppc64|G5)
        echo PPC64: tuning gcc for G5, enabling Altivec
        CFLAGS="$CFLAGS -arch ppc64 -m64 -O3 -mtune=G5 -mcpu=G5 -ffast-math -maltivec -mabi=altivec"
        CXXFLAGS="$CXXFLAGS -arch ppc64 -m64 -O3 -mtune=G5 -mcpu=G5 -ffast-math -maltivec -mabi=altivec"
        USE_ALTIVEC=yes
        AC_DEFINE([HAVE_ALTIVEC],1,[Processor support ALTIVEC])
        BUILD_ALTIVEC=1
        ;;
esac

# Checks for header files.
AC_PATH_X
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([argz.h fcntl.h langinfo.h libintl.h limits.h locale.h malloc.h memory.h nl_types.h stddef.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_TYPE_UID_T

dnl Checks for library functions.
AC_FUNC_FNMATCH
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([__argz_count __argz_next __argz_stringify dcgettext floor getcwd gethostname getwd memchr memmove mempcpy memset mkdir munmap nl_langinfo pow putenv rint setenv setlocale sqrt stpcpy strcasecmp strchr strcspn strdup strerror strncasecmp strrchr strstr strtoul])

dnl Checks for OpenCV availability
dnl AC_SEARCH_LIBS([matchTemplate], [opencv_imgproc opencv_highgui opencv_ml opencv_video opencv_features2d opencv_calib3d opencv_objdetect opencv_contrib opencv_legacy opencv_flann], [
dnl  AC_DEFINE(HAS_OPENCV, 1, [Define if the OpenCV is available.])
dnl ], [
dnl  AC_MSG_ERROR([unable to find the opencv libraries. --> apt-get install libopencv-dev])
dnl ])


###############################

localedir=${datadir}/locale
AC_SUBST(localedir)

plugindir=${libdir}/gxsm3-plugins
AC_SUBST(plugindir)


# Dependencies

PKG_CHECK_MODULES(GXSM3, [
	libxml-2.0 >= $LIBXML_REQUIRED
	glib-2.0 >= $GLIB_REQUIRED
	gio-2.0 >= $GLIB_REQUIRED
	cairo-gobject
	gdk-pixbuf-2.0 >= gdk_pixbuf_minimum_version
	gmodule-2.0
	gmodule-export-2.0
	gail-3.0 gl netcdf fftw3
	gtk+-3.0 >= $GTK_REQUIRED
	gtksourceview-3.0 >= $GTKSOURCEVIEW_REQUIRED
	gsettings-desktop-schemas
])

GXSM3_CFLAGS="$CFLAGS $GXSM3_CFLAGS $X11_CFLAGS $UNIX_CFLAGS"
GXSM3_LIBS="$GXSM3_LIBS $X11_LIBS $UNIX_LIBS"

AC_SUBST(GXSM3_CFLAGS)
AC_SUBST(GXSM3_LIBS)

dnl ================================================================
dnl libgd
dnl ================================================================

### LIBGD_INIT([tagged-entry static])

dnl ================================================================
dnl GSettings stuff
dnl ================================================================

GLIB_GSETTINGS

dnl ================================================================
dnl Misc
dnl ================================================================
AC_PATH_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources)
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)
AC_PATH_PROG(GLIB_MKENUMS, glib-mkenums)

# AC_DEFINE(GDK_VERSION_MIN_REQUIRED, GDK_VERSION_3_8, [minimum gdk version])

GNOME_COMPILE_WARNINGS([maximum])

AC_ARG_ENABLE(deprecations,
              [AS_HELP_STRING([--enable-deprecations],
                              [warn about deprecated usages [default=no]])],
              [enable_deprecations=$enableval], [])

AC_MSG_CHECKING([whether to forbid deprecated symbols])
if test "$enable_deprecations" = "yes"; then
	AC_MSG_RESULT(yes)
	DISABLE_DEPRECATED_CFLAGS="\
-DGSEAL_ENABLE \
-DG_DISABLE_DEPRECATED \
-DGDK_DISABLE_DEPRECATED \
-DGTK_DISABLE_DEPRECATED \
-DGDK_PIXBUF_DISABLE_DEPRECATED \
-DGNOME_DISABLE_DEPRECATED \
-DPEAS_DISABLE_DEPRECATED"

	AC_SUBST(DISABLE_DEPRECATED_CFLAGS)
else
	AC_MSG_RESULT(no)
fi

# on win32 plugins must be able to resolve all symbols at link time
# so we create a lib
if test "$platform_win32" = yes; then
	GXSM3_IMPLIB='-L${libdir}/gxsm3 -lgxsm3'
	PLUGIN_LIBTOOL_FLAGS="-module -avoid-version -no-undefined"
	if test "$os_win32" = yes; then
		PLUGIN_LIBTOOL_FLAGS="${PLUGIN_LIBTOOL_FLAGS} -mwindows"
	fi
else
	GXSM3_IMPLIB=""
	PLUGIN_LIBTOOL_FLAGS="-module -avoid-version"
fi
AC_SUBST(GXSM3_IMPLIB)
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

GXSM3_PLUGINS_DATA_DIR="$datadir/gxsm3/plugins"
AC_SUBST(GXSM3_PLUGINS_DATA_DIR)

GXSM3_PLUGINS_LIBS_DIR="$libdir/gxsm3/plugins"
AC_SUBST(GXSM3_PLUGINS_LIBS_DIR)

YELP_HELP_INIT

# Introspection

#GOBJECT_INTROSPECTION_REQUIRE

#GOBJECT_INTROSPECTION_CHECK([0.9.3])

#if test "$found_introspection" = "yes"; then
#	enable_introspection=yes
#	AC_DEFINE([ENABLE_INTROSPECTION], [1], [Define to enable GObject Introspection])
#else
#	enable_introspection=no
#fi

# Vala
#VAPIGEN_CHECK([0.25.1])

if test "$os_osx" = "yes"; then
	os_menus_ui=menus-osx.ui
	os_style_css=gxsm3-style-osx.css
else
	os_menus_ui=menus-default.ui
	os_style_css=

	OS_DEPENDENT_RESOURCE_FILES='<file preprocess="xml-stripblanks">gtk/menus-traditional.ui</file>'
fi

if test "x$os_style_css" != "x"; then
	OS_DEPENDENT_RESOURCE_FILES+="<file>css/gxsm3-style-os.css</file>"
	AC_CONFIG_LINKS(gxsm3/resources/css/gxsm3-style-os.css:gxsm3/resources/css/$os_style_css)
fi

AC_SUBST(OS_DEPENDENT_RESOURCE_FILES)
AC_CONFIG_LINKS(gxsm3/resources/gtk/menus.ui:gxsm3/resources/gtk/$os_menus_ui)

AC_CONFIG_FILES([
Makefile
gxsm3/Makefile
gxsm3/resources/gxsm3.gresource.xml
plug-ins/Makefile
plug-ins/common/Makefile
plug-ins/control/Makefile
plug-ins/math/Makefile
plug-ins/math/arithmetic/Makefile
plug-ins/math/background/Makefile
plug-ins/math/convert/Makefile
plug-ins/math/filter1d/Makefile
plug-ins/math/filter2d/Makefile
plug-ins/math/misc/Makefile
plug-ins/math/probe/Makefile
plug-ins/math/statistik/Makefile
plug-ins/math/transform/Makefile
plug-ins/scan/Makefile
plug-ins/hard/Makefile
palette/Makefile
profileplot/Makefile
tools/Makefile
thumbnailers/Makefile
pixmaps/Makefile
icons/Makefile
data/org.gnome.gxsm3.desktop.in
data/gxsm3.pc
data/org.gnome.gxsm3.gschema.xml
po/Makefile.in
osx/bundle/data/Info.plist])

AC_OUTPUT

echo "

Configuration:

	Source code location:	${srcdir}
	Compiler:		${CC}
	CFLAGS:			${CFLAGS}
	GXSM3_CFLAGS:		${GXSM3_CFLAGS}
	Spell Plugin enabled:	$enable_enchant
	Gvfs metadata enabled:	$enable_gvfs_metadata
	Deprecations enabled:	$enable_deprecations
	GObject Introspection:	$enable_introspection
	GDK Backend:            $gdk_windowing
	Python support:         $enable_python
	Vala support:           $enable_vala
"

if expr ${GXSM3_MINOR_VERSION} % 2 > /dev/null; then
	stable_version=`expr ${GXSM3_MINOR_VERSION} - 1`

cat << EOF
 *** IMPORTANT ***

This is not the lastest and producion approved version of Gxsm-3.

Thanks,
The Gxsm team

 *** END OF WARNING ***
EOF

fi
