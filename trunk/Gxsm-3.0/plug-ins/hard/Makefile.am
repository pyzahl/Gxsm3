libdir = $(plugindir)/hard
AM_CPPFLAGS = $(GXSM3_CFLAGS) -I$(includedir) -I$(top_builddir)/gxsm -I$(top_builddir)/include 

lib_LTLIBRARIES = \
	libsranger_mk2_hwi.la

#	libsranger_hwi.la \
#
#	libtc211_ccd.la \
#	libdemo_hwi.la \
#	libkmdsp_hwi.la \
#	libsranger_test_hwi.la \
#	libinnovative_dsp_hwi.la \
#	libLAN_rhk_hwi.la \
#	libcomedi_hwi.la \
#	libgrab_v4l.la
#

## --- needs port to new internal builder BuidParam and grid use only
##libsranger_hwi_la_SOURCES = sranger_hwi.C sranger_hwi.h \
##	sranger_hwi_dev.C sranger_hwi_spm.C \
##	sranger_hwi_control.C sranger_hwi_vectorgen.C sranger_hwi_eventhandling.C sranger_hwi_mover.C sranger_hwi_control.h
##
##libsranger_hwi_la_LDFLAGS = -module -export-dynamic -avoid-version


libsranger_mk2_hwi_la_SOURCES = sranger_mk2_hwi.C sranger_mk23common_hwi.h \
	sranger_mk2_hwi_dev.C sranger_mk2_hwi_spm.C sranger_mk2_hwi.h \
	sranger_mk3_hwi_dev.C sranger_mk3_hwi_spm.C sranger_mk3_hwi.h \
	sranger_mk2_hwi_control.C sranger_mk2_hwi_vectorgen.C sranger_mk2_hwi_eventhandling.C \
	sranger_mk2_hwi_mover.C sranger_mk3_hwi_pac.C sranger_mk2_hwi_control.h \
	MK2-A810_spmcontrol/FB_spm_dataexchange.h MK3-A810_spmcontrol/FB_spm_dataexchange.h

libsranger_mk2_hwi_la_LDFLAGS = -module -export-dynamic -avoid-version


#libdemo_hwi_la_SOURCES = demo_hwi.C demo_hwi.h \
#	demo_hwi_dev.C demo_hwi_spm.C \
#	demo_hwi_control.C demo_hwi_control.h
#libdemo_hwi_la_LDFLAGS = -module -export-dynamic -avoid-version


# libtc211_ccd_la_SOURCES = tc211_ccd.C tc211_ccd_c.h
# libtc211_ccd_la_LDFLAGS = -module -export-dynamic -avoid-version

#libkmdsp_hwi_la_SOURCES = kmdsp_hwi.C kmdsp_hwi.h \
#        kmdsp_hwi_dev.C kmdsp_hwi_spm.C \
#        kmdsp_hwi_control.C  kmdsp_hwi_control.h
#libkmdsp_hwi_la_LDFLAGS = -module -export-dynamic -avoid-version


#libsranger_test_hwi_la_SOURCES = sranger_test_hwi.C sranger_test_hwi.h \
#	sranger_test_hwi_dev.C sranger_test_hwi_spm.C \
#	sranger_test_hwi_control.C sranger_test_hwi_control.h
#libsranger_test_hwi_la_LDFLAGS = -module -export-dynamic -avoid-version

#libinnovative_dsp_hwi_la_SOURCES = innovative_dsp_hwi.C innovative_dsp_hwi.h \
#	innovative_dsp_hwi_dev.C innovative_dsp_hwi_spm.C innovative_dsp_hwi_spa.C
#libinnovative_dsp_hwi_la_LDFLAGS = -module -export-dynamic -avoid-version

#libLAN_rhk_hwi_la_SOURCES = LAN_rhk_hwi.C LAN_rhk_hwi_dev.C LAN_rhk_hwi_spm.C LAN_rhk_hwi.h
#libLAN_rhk_hwi_la_LDFLAGS = -module -export-dynamic -avoid-version

#libcomedi_hwi_la_SOURCES = comedi_hwi.C comedi_hwi_dev.C comedi_hwi_spm.C comedi_hwi.h
#libcomedi_hwi_la_LDFLAGS = -module -export-dynamic -avoid-version

#libgrab_v4l_la_SOURCES = grab_v4l.C grab_v4l_c.c
#libgrab_v4l_la_LDFLAGS = -module -export-dynamic -avoid-version



# gsettings_SCHEMAS is a list of all the schemas you want to install
gsettings_SCHEMAS = \
	org.gnome.gxsm3.hwi.sranger_mk23.gschema.xml \
	org.gnome.gxsm3.hwi.sranger_mk23_mover.gschema.xml \
	org.gnome.gxsm3.pcs.plugin-libsranger-mk2-hwi.gschema.xml \
	org.gnome.gxsm3.pcsadjustments.plugin-libsranger-mk2-hwi.gschema.xml

# include the appropriate makefile rules for schema handling
@GSETTINGS_RULES@
