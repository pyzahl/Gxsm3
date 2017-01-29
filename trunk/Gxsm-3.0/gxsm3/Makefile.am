# This is the Makefile.am for the gxsm core
# glib-compile-resources gxsm3_resources.xml --target=gxsm3_resources.C --generate-source

bin_PROGRAMS = gxsm3

gxsm3_SOURCES =	\
gxsm_main.C \
gxsm3_resources.C gxsm3_resources.h \
gxsm_window.C gxsm_window.h \
\
plugin_ctrl.C plugin.h plugin_ctrl.h \
\
cairo_item.C cairo_item.h \
gxsm_conf.h \
gapp_service.C gapp_service.h \
gxsm_app.C gxsm_app.h \
xxsmlogo.xpm \
\
gxsm_menu-extension.c gxsm_menu-extension.h \
gnome-res.c gnome-res.h \
gtkmarshalers.c gtkmarshalers.h \
\
gxsm_resoucetable.C gxsm_resoucetable.h \
gxsmdruidlogo.xpm \
gxsmwatermark.xpm \
app_dnd.C \
gxsm_menucb.C \
app_spm.C \
app_channelselector.C \
app_vinfo.C app_vinfo.h \
app_vobj.C app_vobj.h \
app_view.C app_vpopupdef.h app_view.h \
app_monitor.C \
app_profile.C app_profile.h app_profilepopupdef.h \
app_v3dcontrol.C app_v3dcontrol.h app_v3dpopupdef.h \
lineprofile.C lineprofile.h \
unit.h \
pcs.C pcs.h \
\
meldungen.h \
glbvars.h \
xsmtypes.h \
xsmdebug.h \
xsm_limits.h \
xsm.C xsm.h \
instrument.C instrument.h \
surface.C surface.h \
scan.C scan.h \
topographic.C \
spascan.C \
\
xshmimg.C xshmimg.h \
view.C view.h \
vprofile.C \
vsurf3d.C \
\
xsmhard.C xsmhard.h \
\
bench.h \
mem2d.C mem2d.h zdata.h \
scan_event.C scan_event.h \
regress.C regress.h \
xsmmath.C xsmmath.h \
util.C \
clip.c clip.h \
\
monitor.C monitor.h \
dataio.C dataio.h \
meldungen.h \
epsfutils.C epsfutils.h

pixmapdir = $(datadir)/pixmaps

DEFS = @DEFS@ \
	$(CFLAGS) \
        -Wall -Wstrict-aliasing=0 -ggdb

AM_CPPFLAGS = -I.. \
	-I$(includedir) \
	${libnetcdf_CFLAGS} \
	-I../include $(GXSM3_CFLAGS) \
	-DGNOME_ICONDIR=\""$(pixmapdir)"\"


gxsm3_LDFLAGS = -export-dynamic $(GXSM3_LIBS)

gxsm3_LDADD =  \
	$(FFTW_LIBS) \
	${libnetcdf_LIBS} \
	-lGLEW -lGL

sysdir = $(datadir)/apps/System


# gsettings_SCHEMAS is a list of all the schemas you want to install
gsettings_SCHEMAS = \
	org.gnome.gxsm3.gschema.xml \
	org.gnome.gxsm3.window-geometry.gschema.xml \
	org.gnome.gxsm3.preferences.gschema.xml \
	org.gnome.gxsm3.gl.gschema.xml \
	org.gnome.gxsm3.pcs.mainwindow.gschema.xml \
	org.gnome.gxsm3.pcsadjustments.mainwindow.gschema.xml

# build gxsm2_resource.C
# glib-compile-resources gxsm3_resources.xml --target=gxsm3_resources.C --generate-source
resource_files = $(shell glib-compile-resources --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/gxsm3.gresources.xml)
gxsm3_resources.C: gxsm3.gresources.xml  $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-source --c-name gxsm3 $<
gxsm3_resources.h: gxsm3.gresources.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header --c-name gxsm3 $<

#gxsm3.gresource: gxsm3.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies gxsm3.gresource.xml)


# include the appropriate makefile rules for schema handling
@GSETTINGS_RULES@
