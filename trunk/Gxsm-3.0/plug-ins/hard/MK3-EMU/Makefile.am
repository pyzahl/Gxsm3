# This is the Makefile.am for the DSP MK3 EMU project
#

DEFS = @DEFS@ \
        -Wall \
        -DCOMPILEDBYNAME=\""$(USER)@`hostname`, `date`"\" \
	-DDSPEMU -DDSP_CC

AM_CPPFLAGS = -I.. -I$(includedir) `pkg-config --cflags glib-2.0 gtk+-3.0`

bin_PROGRAMS = mk3dspemu

#noinst_PROGRAMS = mk3dspemu
#NEED:  sudo apt-get install gcc-multilib g++-multilib
# build on matter what in m32 bit mode: CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32
# export PKG_CONFIG_PATH=/usr/lib32/pkgconfig:$PKG_CONFIG_PATH
#autotools with no helper
#Build-depend on dpkg-dev (>= 1.16.0).
#Set DEB_HOST_MULTIARCH in debian/rules by calling: DEB_HOST_MULTIARCH ?= $(shell dpkg-architecture -qDEB_HOST_MULTIARCH)
#Add --libdir=\$${prefix}/lib/$(DEB_HOST_MULTIARCH) to each invocation of configure in debian/rules.
#Any occurrences of /usr/lib in debian/rules should be replaced with /usr/lib/$(DEB_HOST_MULTIARCH).
#Once you have built the package and verified that the shared library package contains only the expected files, and that your -dev package still works, mark this shared library package Multi-Arch: same in debian/control.
#Mark the common package(which is "all") Multi-Arch: foreign in debian/control, if it may be needed by the shared library package.

mk3dspemu_CFLAGS = $(GTK_CFLAGS) `pkg-config --cflags glib-2.0 gtk+-3.0`
mk3dspemu_CXXFLAGS = $(GTK_CXXFLAGS) `pkg-config --cflags glib-2.0 gtk+-3.0`
mk3dspemu_LDFLAGS = -lrt `pkg-config --libs glib-2.0 gtk+-3.0`
mk3dspemu_SOURCES = 				\
	FB_spm.c FB_spm.h \
	FB_spm_dataexchange.h \
	g_intrinsics.c g_intrinsics.h \
	FB_spm_statemaschine.c FB_spm_statemaschine.h \
	dataprocess.c dataprocess.h \
	pac_pll_emu.c \
	FB_spm_offsetmove.c FB_spm_offsetmove.h \
	FB_spm_areascan.c FB_spm_areascan.h \
	FB_spm_CoolRunner_puls.c FB_spm_CoolRunner_puls.h \
	FB_spm_probe.c FB_spm_probe.h \
	FB_spm_autoapproach.c FB_spm_autoapproach.h \
	FB_spm_dsoszi.c \
	SR3PRO_A810Driver.h \
	ReadWrite_GPIO.c ReadWrite_GPIO.h \
        SR3_flash_support.c SR3_flash_support.h \
	spm_log.c spm_log.h \
	dspemu_app.c dspemu_app.h 		\
	dspemu_app_window.c dspemu_app_window.h 	\
	dspemu_app_prefs.c dspemu_app_prefs.h 	\
	resources.c

BUILT_SOURCES = 				\
	resources.c 				\
	gschemas.compiled

resources.c: mk3dspemu.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/mk3dspemu.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) $(srcdir)/mk3dspemu.gresource.xml \
		--target=$@ --sourcedir=$(srcdir) --generate-source

gsettings_SCHEMAS = 				\
	org.gtk.mk3dspemu.gschema.xml

@GSETTINGS_RULES@

gschemas.compiled: org.gtk.mk3dspemu.gschema.xml
	$(GLIB_COMPILE_SCHEMAS) .

EXTRA_DIST = 					\
	window.ui 				\
	app-menu.ui 				\
	prefs.ui 				\
	dspemu_app.gresource.xml 		\
	org.gtk.dspemu_app.gschema.xml

CLEANFILES = 					\
	gschemas.compiled

-include $(top_srcdir)/git.mk
