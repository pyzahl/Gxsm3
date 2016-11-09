CFLAGS=-ansi -Wall `/usr/bin/./pkg-config glib-2.0 --cflags`
LDFLAGS=`/usr/bin/pkg-config glib-2.0 --libs`
logviewer.o : logviewer.h logviewer.C WSxM_header.h WSxM_header.C
	g++ -o logviewer WSxM_header.C logviewer.C $(CFLAGS) $(LDFLAGS)
