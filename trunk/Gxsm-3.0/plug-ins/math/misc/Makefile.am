libdir = $(plugindir)/math
AM_CPPFLAGS = $(GXSM3_CFLAGS) -I$(includedir) -I$(top_builddir)/gxsm

lib_LTLIBRARIES = libshape.la \
	libfindlocmax.la \
	libpsdadd.la \
	libspectrocut.la \
	liblayersmooth.la \
	libworkfuncextract.la

libfindlocmax_la_SOURCES = findlocmax.C
libfindlocmax_la_LDFLAGS = -module -export-dynamic -avoid-version

libpsdadd_la_SOURCES = psdadd.C
libpsdadd_la_LDFLAGS = -module -export-dynamic -avoid-version

libspectrocut_la_SOURCES = spectrocut.C
libspectrocut_la_LDFLAGS = -module -export-dynamic -avoid-version

liblayersmooth_la_SOURCES = layersmooth.C
liblayersmooth_la_LDFLAGS = -module -export-dynamic -avoid-version

libworkfuncextract_la_SOURCES = workfuncextract.C
libworkfuncextract_la_LDFLAGS = -module -export-dynamic -avoid-version

libshape_la_SOURCES = shape.C
libshape_la_LDFLAGS = -module -export-dynamic -avoid-version
