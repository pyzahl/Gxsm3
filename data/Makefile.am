data_desktopdir = $(datadir)/applications
data_desktop_in_files = data/org.gnome.gxsm3.desktop.in
data_desktop_DATA = $(data_desktop_in_files:.desktop.in=.desktop)

data_appdatadir = $(datadir)/appdata
data_appdata_in_files = data/org.gnome.gxsm3.appdata.xml.in
data_appdata_DATA = $(data_appdata_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

data_servicedir = $(datadir)/dbus-1/services
data_service_in_files = data/org.gnome.gxsm3.service.in
data_service_DATA = $(data_service_in_files:.service.in=.service)

$(data_service_DATA): $(data_service_in_files) Makefile
	@sed -e "s|\@bindir\@|$(bindir)|" $<> $@

dist_man_MANS = data/gxsm3.1

data_pkgconfigdir = $(libdir)/pkgconfig
data_pkgconfig_DATA = data/gxsm3.pc

data_bugreportdir = $(libexecdir)/gxsm3
dist_data_bugreport_SCRIPTS = data/gxsm3-bugreport.sh

gsettings_SCHEMAS += data/org.gnome.gxsm3.gschema.xml
gsettings_ENUMS += data/org.gnome.gxsm3.enums.xml
data/org.gnome.gxsm3.enums.xml: $(top_srcdir)/gxsm3/*.h

if OS_OSX
dist_gsettingsschema_DATA += data/gxsm3-osx.gschema.override
endif

data_convertdir = $(datadir)/GConf/gsettings
dist_data_convert_DATA = data/gxsm3.convert

EXTRA_DIST += 					\
	$(data_desktop_in_files)		\
	$(data_appdata_in_files)		\
	$(data_service_in_files)		\
	data/org.gnome.gxsm3.gschema.xml.in	\
	data/gxsm3.pc.in

CLEANFILES +=	 			\
	$(data_appdata_DATA)		\
	$(data_desktop_DATA)		\
	$(data_service_DATA)		\
	$(data_pkgconfig_DATA)

DISTCLEANFILES +=	 		\
	$(data_desktop_DATA)		\
	$(data_service_DATA)		\
	$(data_pkgconfig_DATA)
