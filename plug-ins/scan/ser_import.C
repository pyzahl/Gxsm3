/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Plugin Name: ser_impor.C
 * ========================================
 * 
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Percy Zahl <zahl@fkp.uni-hannover.de>
 * additional features: Andreas Klust <klust@fkp.uni-hannover.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

/* Please do not change the Begin/End lines of this comment section!
 * this is a LaTeX style section used for auto generation of the PlugIn Manual 
 * Chapter. Add a complete PlugIn documentation inbetween the Begin/End marks!
 * --------------------------------------------------------------------------------
% BeginPlugInDocuSection
% PlugInDocuCaption: Ser file Import
% PlugInName: ser_import
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: File/Import/Ser

% PlugInDescription
\label{plugins:ser_import}
The \GxsmEmph{Ser} plug-in supports reading of a simple ser volumetric data file format.


SER format description version 3

Ser format consist of three parts:
A) Header with fixed size of 178 Byte
B) Image frame data with variable byte size of: <BytePerPixel> x <Image width> x <Image height> x <Total amount of Images>
C) Trailer Optional. Byte size of 8 x <Total amount of Images>Ser format consist of three parts:


1_FileID
Format: String
Length: 14 Byte (14 ASCII characters)
Content: "LUCAM-RECORDER" (fix)

2_LuID
Format: Integer_32 (little-endian)
Length: 4 Byte
Content: Lumenera camera series ID (currently unused; default = 0)

3_ColorID
Format:Integer_32 (little-endian)
Length:
Content:4 Byte
0  = MONO
8  = BAYER_RGGB
9  = BAYER_GRBG
10 = BAYER_GBRG
11 = BAYER_BGGR
16 = BAYER_CYYM
17 = BAYER_YCMY
18 = BAYER_YMCY
19 = BAYER_MYYC
100 = RGB
101 = BGR

4_LittleEndian
Format:  Integer_32 (little-endian)
Length:  4 Byte
Content: 0 (FALSE) for big-endian byte order in 16 bit image data
         1 (TRUE) for little-endian byte order in 16 bit image data

5_ImageWidth
Format:  Integer_32 (little-endian)
Length:  4 Byte
Content: Width of every image in pixel


6_ImageHeight
Format:  Integer_32 (little-endian)
Length:  4 Byte
Content: Height of every image in pixel

7_PixelDepthPerPlane
Format:  Integer_32 (little-endian)
Length:  4 Byte
Content: True bit depth per pixel per plane

3_ColorID           NumberOfPlanes
MONO … BAYER_MYYC   1
RGB, BGR            3

7_PixelDepthPerPlane  BytesPerPixel
1..8                  1 * NumberOfPlanes
9..16                 2 * NumberOfPlanes

Pixel data organization:
8 bit unsigned integer    (7_PixelDepthPerPlane = 1..8)

3_ColorID              Pixel data [Byte]
MONO … BAYER_MYYC      [M]
RGB                    [R] [G] [B]
BGR                    [B] [G] [R]

16 bit unsigned integer (7_PixelDepthPerPlane = 9..16)

3_ColorID              Pixel data [Byte]
MONO … BAYER_MYYC      [M][M]
RGB                    [R][R] [G][G] [B][B]
BGR                    [B][B] [G][G] [R][R]

Byte order in 16 bit format (Lo / Hi byte) depends on 4_LittleEndian.
Image data organization:
Start pixel is the upper left pixel of the image.
Data of between 1 and 8 bits should be stored aligned with the most significant bit
(MSB). For example:
1-bit data
2-bit data
3-bit data
4-bit data
5-bit data
6-bit data
7-bit data
8-bit data
MSB ->LSB
b0000000
bb000000
bbb00000
bbbb0000
bbbbb000
bbbbbb00
bbbbbbb0
bbbbbbbb
Data between 9 and 16 bits should be stored aligned with the least significant bit
(LSB). For example:
9-bit data
10-bit data
11-bit data
12-bit data
13-bit data
14-bit data
15-bit data
MSB
->
LSB
0000000bbbbbbbbb
000000bbbbbbbbbb
00000bbbbbbbbbbb
0000bbbbbbbbbbbb
000bbbbbbbbbbbbb
00bbbbbbbbbbbbbb
0bbbbbbbbbbbbbbb16-bit data
bbbbbbbbbbbbbbbb

8_FrameCount
Format: Integer_32 (little-endian)
Length: 4 Byte
Content: Number of image frames in SER file

9_Observer
Format: String
Length: 40 Byte (40 ASCII characters {32…126 dec.}, fill unused characters with 0 dec.)
Content: Name of observer

10_Instrument
Format: String
Length: 40 Byte (40 ASCII characters {32…126 dec.}, fill unused characters with 0 dec.)
Content: Name of used camera

11_Telescope
Format: String
Length: 40 Byte (40 ASCII characters {32…126 dec.}, fill unused characters with 0 dec.)
Content: Name of used telescope

12_DateTime
Format: Date / Integer_64 (little-endian)
Length: 8 Byte
Content: Start time of image stream (local time)
If 12_DateTime <= 0 then 12_DateTime is invalid and the SER file does not contain a
Time stamp trailer.

13_DateTime_UTC
Format: Date / Integer_64 (little-endian)
Length: 8 Byte
Content: Start time of image stream in UTC



Image Data
Image data starts at File start offset decimal 178
Size of every image frame in byte is: 5_ImageWidth x 6_ImageHeigth x BytePerPixel



Trailer in detail
Trailer starts at byte offset: 178 + 8_FrameCount x 5_ImageWidth x 6_ImageHeigth x BytePerPixel.

Trailer contains Date / Integer_64 (little-endian) time stamps in UTC for every image frame.


According to Microsoft documentation the used time stamp has the following format:
“Holds IEEE 64-bit (8-byte) values that represent dates ranging from January 1 of the year 0001
through December 31 of the year 9999, and times from 12:00:00 AM (midnight) through
11:59:59.9999999 PM. Each increment represents 100 nanoseconds of elapsed time since the
beginning of January 1 of the year 1 in the Gregorian calendar. The maximum value represents
100 nanoseconds before the beginning of January 1 of the year 10000.”
According to the findings of Raoul Behrend, Université de Genève, the date record is not a 64 bits
unsigned integer as stated, but a 62 bits unsigned integer. He got no information about the use of
the two MSB.






Data Set File Format:

The data files for rectilinearly sampled scalar data are written in
the following format (all fields big-endian ser): 

Resolution (number of grid points in x, y and z direction): three
32-bit int values. Let's refer to them as numX, numY, numZ.

Size of saved border around volume: one 32-bit int value. This value
is not used in the provided data sets and is set to zero.

True size (extent in x, y and z direction in some unit of
measurement): three 32-bit float values. Treat these fields like a
sort of "3D aspect ratio" - usually, medical data sets are sampled as
a stack of slices, where the distances between slices is different
from the distances between pixels in a slice.

Data values: All numX*numY*numZ data values of the volume stored as
unsigned char values in the range (0 .. 256). The values are stored in
x, y, z order, i.e., x varies slowest, z varies fastest. In other
words, they are stored in the memory order of a standard C
three-dimensional array unsigned char values\[numX\]\[numY\]\[numZ\].

% PlugInUsage
The plug-in is called by \GxsmMenu{File/Import/Ser}. 
Only import direction is implemented.

%% OptPlugInKnownBugs
%Not yet tested.

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */

#include <gtk/gtk.h>
#include "config.h"
#include "gxsm3/plugin.h"
#include "gxsm3/dataio.h"
#include "gxsm3/action_id.h"
#include "gxsm3/util.h"
#include "batch.h"
#include "fileio.c"

using namespace std;

// Plugin Prototypes
static void ser_import_init (void);
static void ser_import_query (void);
static void ser_import_about (void);
static void ser_import_configure (void);
static void ser_import_cleanup (void);

static void ser_import_filecheck_load_callback (gpointer data );
static void ser_import_filecheck_save_callback (gpointer data );

static void ser_import_import_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data);
static void ser_import_export_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data);

// Fill in the GxsmPlugin Description here
GxsmPlugin ser_import_pi = {
  NULL,                   // filled in and used by Gxsm, don't touch !
  NULL,                   // filled in and used by Gxsm, don't touch !
  0,                      // filled in and used by Gxsm, don't touch !
  NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
                          // filled in here by Gxsm on Plugin load, 
                          // just after init() is called !!!
  "Ser_Import",
  NULL,                   // PlugIn's Categorie, set to NULL for all, I just don't want this always to be loaded!
  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
  g_strdup ("Im/Export of the SER (camera video) data file format."),
  "Percy Zahl",
  "file-import-section,file-export-section",
  N_("SER,SER"),
  N_("SER import,SER export"),
  N_("SER data file (camera raw video or still) import and export filter."),
  NULL,          // error msg, plugin may put error status msg here later
  NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
  ser_import_init,
  ser_import_query,
  // about-function, can be "NULL"
  // can be called by "Plugin Details"
  ser_import_about,
  // configure-function, can be "NULL"
  // can be called by "Plugin Details"
  ser_import_configure,
  // run-function, can be "NULL", if non-Zero and no query defined, 
  // it is called on menupath->"plugin"
  NULL,
  // cleanup-function, can be "NULL"
  // called if present at plugin removal
  ser_import_import_callback, // direct menu entry callback1 or NULL
  ser_import_export_callback, // direct menu entry callback2 or NULL

  ser_import_cleanup
};

// Text used in Aboutbox, please update!!
static const char *about_text = N_("GXSM ser data file Import/Export Plugin\n\n");

static const char *file_mask = "*";

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
  ser_import_pi.description = g_strdup_printf(N_("Gxsm im_export plugin %s"), VERSION);
  return &ser_import_pi; 
}

// Query Function, installs Plugin's in File/Import and Export Menupaths!
// ----------------------------------------------------------------------
// Import Menupath is "File/Import/GME Dat"
// Export Menupath is "File/Export/GME Dat"
// ----------------------------------------------------------------------
// !!!! make sure the "ser_import_cleanup()" function (see below) !!!!
// !!!! removes the correct menuentries !!!!

static void ser_import_query(void)
{
	gchar **entry = g_strsplit (ser_import_pi.menuentry, ",", 2);

	if(ser_import_pi.status) g_free(ser_import_pi.status); 
	ser_import_pi.status = g_strconcat (
		N_("Plugin query has attached "),
		ser_import_pi.name, 
		N_(": File IO Filters are ready to use."),
		NULL);
	
	// clean up
	g_strfreev (entry);

// register this plugins filecheck functions with Gxsm now!
// This allows Gxsm to check files from DnD, open, 
// and cmdline sources against all known formats automatically - no explicit im/export is necessary.
	ser_import_pi.app->ConnectPluginToLoadFileEvent (ser_import_filecheck_load_callback);
	ser_import_pi.app->ConnectPluginToSaveFileEvent (ser_import_filecheck_save_callback);
}


// 5.) Start here with the plugins code, vars def., etc.... here.
// ----------------------------------------------------------------------
//


// init-Function
static void ser_import_init(void)
{
	PI_DEBUG (DBG_L2, ser_import_pi.name << "Plugin Init");
}

// about-Function
static void ser_import_about(void)
{
	const gchar *authors[] = { ser_import_pi.authors, NULL};
	gtk_show_about_dialog (NULL,
			       "program-name",  ser_import_pi.name,
			       "version", VERSION,
			       "license", GTK_LICENSE_GPL_3_0,
			       "comments", about_text,
			       "authors", authors,
			       NULL
			       );
}

// configure-Function
static void ser_import_configure(void)
{
	if(ser_import_pi.app)
		ser_import_pi.app->message("ser_import Plugin Configuration");
}

// cleanup-Function, make sure the Menustrings are matching those above!!!
static void ser_import_cleanup(void)
{
	PI_DEBUG (DBG_L2, "Plugin Cleanup done.");
}

// make a new derivate of the base class "Dataio"
class ser_ImExportFile : public Dataio{
public:
	ser_ImExportFile(Scan *s, const char *n) : Dataio(s,n){ };
	virtual FIO_STATUS Read(xsm::open_mode mode=xsm::open_mode::replace);
	virtual FIO_STATUS Write();
private:
	FIO_STATUS import(const char *fname);
};

FIO_STATUS ser_ImExportFile::Read(xsm::open_mode mode){
	FIO_STATUS ret;
	gchar *fname=NULL;

	fname = (gchar*)name;

	// name should have at least 4 chars: ".ext"
	if (fname == NULL || strlen(fname) < 4)
		return  FIO_NOT_RESPONSIBLE_FOR_THAT_FILE;
 
	// check for file exists and is OK !
	// else open File Dlg
	ifstream f;
	f.open(fname, ios::in | ios::binary);
	if(!f.good()){
		PI_DEBUG (DBG_L2, "Error at file open. File not good/readable.");
		return status=FIO_OPEN_ERR;
	}
	f.close();

	// Check all known File Types:
	if ((ret=import (fname)) !=  FIO_NOT_RESPONSIBLE_FOR_THAT_FILE)
		return ret;

	return  FIO_NOT_RESPONSIBLE_FOR_THAT_FILE;
}



/*
void setup_multidimensional_data_copy (const gchar *title, Scan *src, int &ti, int &tf, int &vi, int &vf, int *crop_window_xy=NULL, gboolean crop=FALSE){
	UnitObj *Pixel = new UnitObj("Pix","Pix");
	UnitObj *Unity = new UnitObj(" "," ");
	GtkWidget *dialog = gtk_dialog_new_with_buttons (N_(title),
							 NULL, 
							 (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
							 _("_OK"), GTK_RESPONSE_ACCEPT,
							 NULL); 
	BuildParam bp;
        gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), bp.grid);

        bp.grid_add_label ("New Time and Layer bounds:"); bp.new_line ();

	gint t_max = src->number_of_time_elements ()-1;
	if (t_max < 0) t_max=0;

        bp.grid_add_ec ("t-inintial",  Unity, &ti, 0, t_max, ".0f"); bp.new_line ();
        bp.grid_add_ec ("t-final",     Unity, &tf, 0, t_max, ".0f"); bp.new_line ();
        bp.grid_add_ec ("lv-inintial", Unity, &vi, 0, src->mem2d->GetNv (), ".0f"); bp.new_line ();
        bp.grid_add_ec ("lv-final",    Unity, &vf, 0, src->mem2d->GetNv (), ".0f"); bp.new_line ();

	gtk_widget_show_all (dialog);
	gtk_dialog_run (GTK_DIALOG(dialog));

	gtk_widget_destroy (dialog);

	delete Pixel;
	delete Unity;	
}
*/




FIO_STATUS ser_ImExportFile::import(const char *fname){
        // SER HEADER: 178 BYTES
        struct SERheaderID {
                gchar FileID[16]; //1 "LUCAM-RECORDER"
        } lucam;
        struct SERheader {
                //gchar FileID[14]; //1 "LUCAM-RECORDER" -- alignment problem here!!
                guint32 LuID;     //2 serial = 0
                guint32 ColorID;  //3 MONO = 0, BAYER_RGGB=8, ..., RGB=100, BGR=101; see docu
                guint32 LittleEndian; //4 1=LE (TRUE), 0=BE
                guint32 ImageWidth;   //5   3088
                guint32 ImageHeight;  //6   2064
                guint32 PixelDepthPerPlane; //7   12
                // True bit depth per pixel per plane. [MONO...BAYER+*: NumPlanes=1, RGB,BGR: NumPlanes=3;] 1..8: 1Byte/Pixel*NumPLanes, 9..16: 2Byte/Pixel*NumPLanes
                guint32 FrameCount;   //8
                gchar Observer[40];   //9
                gchar Instrument[40]; //10
                gchar Telescope[40];  //11
                guint64 DateTime;     //12 Start time of image stream (local time) If 12_DateTime <= 0 then 12_DateTime is invalid and the SER file does not contain a Time stamp trailer.
                guint64 DateTimeUTC;  //13 in UTC
        } ser_header;
        int SER=0;
        
        // Trailer contains Date / Integer_64 (little-endian) time stamps in UTC for every image frame.
        /* According to Microsoft documentation the used time stamp has the following format:
           “Holds IEEE 64-bit (8-byte) values that represent dates ranging from January 1 of the year 0001
           through December 31 of the year 9999, and times from 12:00:00 AM (midnight) through
           11:59:59.9999999 PM. Each increment represents 100 nanoseconds of elapsed time since the
           beginning of January 1 of the year 1 in the Gregorian calendar. The maximum value represents
           100 nanoseconds before the beginning of January 1 of the year 10000.”
        */
                         
        struct header {
                guint32 dimensions_xyzb[4];
                double  extends_xyz[3];
                int     bin[3];
        } h;

	// Am I resposible for that file -- can only do a dimension sanity check
	ifstream f;
	GString *FileList=NULL;



        GSettings *global_settings = g_settings_new (GXSM_RES_BASE_PATH_DOT ".global");
        int sf = (int)g_settings_get_double (global_settings, "import-ser-start-frame");
        int ef = (int)g_settings_get_double (global_settings, "import-ser-end-frame");
        int tb = (int)g_settings_get_double (global_settings, "import-ser-tbin");
        int xb = (int)g_settings_get_double (global_settings, "import-ser-xbin");
        int yb = (int)g_settings_get_double (global_settings, "import-ser-ybin");
        int roi_ls = (int)g_settings_get_double (global_settings, "import-ser-roi-ls");
        int roi_le = (int)g_settings_get_double (global_settings, "import-ser-roi-le");
        g_clear_object (&global_settings);
        
        g_message ("SER import controls ** start: %d end: %d binning: %d/%d/%d  roi L %d .. %d", sf, ef, xb,yb,tb, roi_ls, roi_le);


        f.open(name, ios::in);
	if (!f.good())
	        return status=FIO_OPEN_ERR;

        // TRY and check for SER
        memset ((char*)&lucam, 0, 14); // zero
        f.read ((char*)&lucam, 14); // read 14 -- followed by 0,0 now!
	if (!f.good())
	        return status=FIO_OPEN_ERR;

	FileList = g_string_new ("Imported by GXSM from simple Binay/SER data file.\n");
        //g_string_append_printf (FileList, "");
        if (strncmp(lucam.FileID, "LUCAM-RECORDER", 14) == 0){
                g_message ("Checking for SER header: OK  ID=%14s", lucam.FileID);
                g_string_append_printf (FileList, "SER header: OK  ID=%14s\n", lucam.FileID);
                SER=1;
                f.read ((char*)&ser_header, sizeof (ser_header));
                g_string_append_printf (FileList, "ColorID=%d %s\n", ser_header.ColorID, ser_header.ColorID==0 ? "MONO": ser_header.ColorID < 100?"BAYER_***":"RGB/BGR");
                g_string_append_printf (FileList, "%sEndian\n", ser_header.LittleEndian?"Little":"Big");
                g_string_append_printf (FileList, "Image Width %d x Height %d\n", ser_header.ImageWidth, ser_header.ImageHeight);
                g_string_append_printf (FileList, "PixelDepthPerPlane: %d\n", ser_header.PixelDepthPerPlane);
                g_string_append_printf (FileList, "FrameCount:         %d\n", ser_header.FrameCount);
                // 0 terminate for sure:
                ser_header.Observer[39] = 0; 
                ser_header.Instrument[39] = 0;
                ser_header.Telescope[39] = 0;  //11
                g_string_append_printf (FileList, "Observer:   %s\n", ser_header. Observer);
                g_string_append_printf (FileList, "Instrument: %s\n", ser_header.Instrument);
                g_string_append_printf (FileList, "Telescope:  %s\n", ser_header.Telescope);
                g_string_append_printf (FileList, "DateTime:   %d\n", ser_header.DateTime);
                g_string_append_printf (FileList, "DateTimeUTC %d\n", ser_header.DateTimeUTC);

                h.dimensions_xyzb[0]=ser_header.ImageWidth;
                h.dimensions_xyzb[1]=ser_header.ImageHeight;
                h.dimensions_xyzb[2]=ser_header.FrameCount;
                h.dimensions_xyzb[3]=ser_header.PixelDepthPerPlane > 15? 16:8;
                h.extends_xyz[0]=ser_header.ImageWidth;
                h.extends_xyz[1]=ser_header.ImageHeight;
                h.extends_xyz[2]=ser_header.FrameCount;
                h.bin[0] = xb; // X
                h.bin[1] = yb; // Y
                h.bin[2] = tb; // TIME
                
        }else{
                f.close ();
                g_message ("Checking for SER header: FAILED ID=%14s", lucam.FileID);
                g_string_append_printf (FileList, "No SER header: ID=%14s\n", lucam.FileID);
                f.open(name, ios::in); // reopen from beginning
                g_string_append_printf (FileList, "Assuming plain raw dump IMX178M 3088x2064x16bit MONO, reading up to 1000 frames/end.\n");
                h.dimensions_xyzb[0]=3088;
                h.dimensions_xyzb[1]=2064;
                h.dimensions_xyzb[2]=1000;
                h.dimensions_xyzb[3]=16;
                h.extends_xyz[0]=3088;
                h.extends_xyz[1]=2064;
                h.extends_xyz[2]=1000;
                h.bin[0] = xb; // X
                h.bin[1] = yb; // Y 
                h.bin[2] = tb; // TIME
        }

        
        g_message ("Ser Read. Filename: %s\nDim-XYZB: %dpx %dpx %dpx %dbin\nExtends-XYZ: %f %f %f",
                   fname,
                   (int)h.dimensions_xyzb[0], (int)h.dimensions_xyzb[1], (int)h.dimensions_xyzb[2], h.dimensions_xyzb[3], 
                   (double)h.extends_xyz[0],  (double)h.extends_xyz[1],  (double)h.extends_xyz[2]);
        
	// read header
	if ( f.good ()){
                g_string_append_printf (FileList, "Original Filename: %s\nDim-XYZB: %d %d %d %d\nExtends-XYZ: %f %f %f", fname,
                                        (int)h.dimensions_xyzb[0], (int)h.dimensions_xyzb[1], (int)h.dimensions_xyzb[2], h.dimensions_xyzb[3], 
                                        (double)h.extends_xyz[0],  (double)h.extends_xyz[1],  (double)h.extends_xyz[2]);
                if ((double)h.dimensions_xyzb[0] * (double)h.dimensions_xyzb[1] * (double)h.dimensions_xyzb[2] > 1e12){
                        f.close ();
                        g_message ("SER Read Total Extends exceeding 1e12 -- stopping.");
                        return status=FIO_NOT_RESPONSIBLE_FOR_THAT_FILE; // possibly wrong. Arbitrary limits. Adjust if needed.
                }
	} else {
	        return status=FIO_OPEN_ERR;
        }

        // data[numX][numY][numZ]

	time_t t; // Scan - Startzeit eintragen 
	time(&t);
	gchar *tmp = g_strconcat ((ctime(&t)), " (Imported)", NULL); scan->data.ui.SetDateOfScan (tmp); g_free (tmp);
	scan->data.ui.SetName (fname);
	scan->data.ui.SetOriginalName (fname);
	scan->data.ui.SetType ("Ser Volume"); 


	// put some usefull values in the ui structure
	if(getlogin()){
		scan->data.ui.SetUser (getlogin());
	}
	else{
		scan->data.ui.SetUser ("unkonwn user");
	}

	// this is mandatory.
	// initialize scan structure -- this is a minimum example
	scan->data.s.ntimes  = 1;
	scan->data.s.nx = h.dimensions_xyzb[0]/h.bin[0];
	scan->data.s.ny = h.dimensions_xyzb[1]/h.bin[1];
	scan->data.s.nvalues = 1; //h.dimensions_xyzb[2];
	scan->data.s.dx = h.extends_xyz[0]/h.dimensions_xyzb[0]*h.bin[0]; // need Angstroems
	scan->data.s.dy = h.extends_xyz[1]/h.dimensions_xyzb[1]*h.bin[1];
	scan->data.s.dz = h.extends_xyz[2]/h.dimensions_xyzb[2];
	scan->data.s.rx = scan->data.s.dx * scan->data.s.nx;
	scan->data.s.ry = scan->data.s.dy * scan->data.s.ny;
	scan->data.s.rz = scan->data.s.dz * scan->data.s.nvalues;
	scan->data.s.x0 = 0.;
	scan->data.s.y0 = 0.;
	scan->data.s.alpha = 0.;

	// be nice and reset this to some defined state
	scan->data.display.z_high       = 0.;
	scan->data.display.z_low        = 0.;

	// set the default view parameters
	scan->data.display.bright = 32.;
	scan->data.display.contrast = 1.0;

	// FYI: (PZ)
	//  scan->data.display.vrange_z  = ; // View Range Z in base ZUnits
	//  scan->data.display.voffset_z = 0; // View Offset Z in base ZUnits
	//  scan->AutoDisplay([...]); // may be used too...
  
	UnitObj *u = gapp->xsm->MakeUnit ("um", "X");
	scan->data.SetXUnit(u);
	delete u;

	u = gapp->xsm->MakeUnit ("um", "Y");
	scan->data.SetYUnit(u);
	delete u;

	u = gapp->xsm->MakeUnit ("um", "Z");
	scan->data.SetZUnit(u);
	delete u;

        g_message (FileList->str);
	scan->data.ui.SetComment (FileList->str);
	g_string_free(FileList, TRUE); 
	FileList=NULL;



        // ser_header.PixelDepthPerPlane
        // ser_header.ColorID, ser_header.ColorID==0 ? "MONO": ser_header.ColorID < 100?"BAYER_***":"RGB/BGR"

        // not support all variation, add as needed!
        if (SER && ser_header.PixelDepthPerPlane == 8 && ser_header.ColorID == 100){ // RGB 24bit total in 3 planes
                // Read Img Data.
                size_t frame_buffer_size =  3*(size_t)h.dimensions_xyzb[0] * (size_t)h.dimensions_xyzb[1];
                guint8 *data;
                data = g_new (guint8, frame_buffer_size);

                g_message ("SER Raw RGB24 Import");

                gapp->progress_info_new ("SER Raw RGB24 Import", 2);
                gapp->progress_info_set_bar_fraction (0., 1);
                gapp->progress_info_set_bar_text (fname, 1);

                scan->data.s.nvalues = 3;
                scan->mem2d->Resize (scan->data.s.nx, scan->data.s.ny, ZD_RGBA);
                // convert data
                for (int index_time=0; index_time<ser_header.FrameCount; ++index_time){
                        gapp->progress_info_set_bar_fraction ((gdouble)index_time/(gdouble)ser_header.FrameCount, 1);
                        g_message ("Reading Frame: %d", index_time);
                        f.read ((char*)data, sizeof (guint8) * frame_buffer_size);
                        if (! f.good ()){
                                g_warning ("Ser data read EOF. #Frames read: %d", index_time);
                                break;
                        }
                        guint8 *p = data;
                        for (int row=0; row < scan->mem2d->GetNy (); ++row){
                                for (int col=0; col < scan->mem2d->GetNx (); ++col){
                                        for (int k=0; k<3; ++k){ // RGB
                                                double bin = 0.; // binning
                                                for (int ib = 0; ib<h.bin[0]; ib++)
                                                        for (int jb = 0; jb<h.bin[1]; jb++)
                                                                bin += (double)*(p+ib+jb*3*h.dimensions_xyzb[0]+k);
                                                p+=h.bin[0];
                                                scan->mem2d->PutDataPkt (bin, col, row, k);
                                        }
                                }
                                p+=(h.bin[1]-1)*3*h.dimensions_xyzb[0];
                        }
                        scan->append_current_to_time_elements (index_time, index_time);
                }
                g_free (data);

                // trailer may have frame times
                g_message ("SER reading trailer, frame times");
                gint64 frame_times[ser_header.FrameCount];
                f.read ((char*)frame_times, sizeof (guint64) * ser_header.FrameCount);
                if (f.good ()){
                        guint64 start=frame_times[0];
                        for (int index_time=0; index_time<ser_header.FrameCount; ++index_time){
                                g_message ("#%05d @ %llu %g s", index_time, frame_times[index_time], (double)(frame_times[index_time]-start)/10000000.);
                                scan->set_nth_time_element_frame_time (index_time, (double)(frame_times[index_time]-start)/10000000.);
                        }
                } else {
                        if (ser_header.FrameCount > 2){
                                g_message ("?? #%05d @ %llu ??", 0, frame_times[0]);
                                g_message ("?? #%05d @ %llu ??", 1, frame_times[1]);
                        }
                        g_message ("No or inclmplete SER trailer.\n");
                }
                
                gapp->progress_info_close ();
                scan->retrieve_time_element (0);
        } else if (SER && ser_header.PixelDepthPerPlane <= 16 && ser_header.ColorID == 0){ // MONO
                // Read Img Data.
                size_t frame_buffer_size =  (size_t)h.dimensions_xyzb[0] * (size_t)h.dimensions_xyzb[1];
                guint16 *data;

                g_message ("SER Raw MONO16 Import, Frame Count %d", ser_header.FrameCount);
                
                gapp->progress_info_set_bar_fraction (0., 1);
                gapp->progress_info_set_bar_text (fname, 1);

                data = g_new (guint16, frame_buffer_size);
                
                scan->data.s.nvalues = 1;
                if (roi_ls >= 0 && roi_le >= 0){
                        scan->data.s.ny = roi_le - roi_ls + 1;
                }
                scan->mem2d->Resize (scan->data.s.nx, scan->data.s.ny, 1, ZD_FLOAT);
                // convert data
                int fi=0;
                int tbin=0;
                for (int index_time=0; index_time<ser_header.FrameCount; ++index_time){
                        gapp->progress_info_set_bar_fraction ((gdouble)index_time/(gdouble)ser_header.FrameCount, 1);
                        g_message ("Reading Frame: %d", index_time);
                        f.read ((char*)data, sizeof (guint16) * frame_buffer_size);
                        if (! f.good ()){
                                g_warning ("Ser data read EOF. #Frames read: %d", index_time);
                                break;
                        }
                        if (sf > 0)
                                if (index_time < sf)
                                        continue;
                        if (ef > 0)
                                if (index_time > ef)
                                        break;
                                
                        guint16 *p = data;
                        if (roi_ls > 0)
                                p += (int)h.extends_xyz[0]*(roi_ls-1);
                        for (int row=0; row < scan->mem2d->GetNy (); ++row){
                                for (int col=0; col < scan->mem2d->GetNx (); ++col){
                                        double bin = 0.; // binning
                                        for (int ib = 0; ib<h.bin[0]; ib++)
                                                for (int jb = 0; jb<h.bin[1]; jb++)
                                                        bin += (double)*(p+ib+jb*h.dimensions_xyzb[0]);
                                        p+=h.bin[0];
                                        if (tbin == 0){
                                                //g_message ("[%d,%d] %g", col,row,bin);
                                                scan->mem2d->PutDataPkt (bin, col, row);
                                        }else
                                                scan->mem2d->data->Zadd (bin, col, row);
                                }
                                p+=(h.bin[1]-1)*h.dimensions_xyzb[0];
                        }
                        if (tb > 1){
                                tbin++;
                                if (tbin == tb){
                                        g_message ("Appending Frame: %d -> index %d, tbin=%d", index_time, fi, tbin);
                                        scan->append_current_to_time_elements (fi, index_time);
                                        fi++;
                                        tbin = 0;
                                }
                        }
                        else{
                                g_message ("Appending Frame: %d -> index %d", index_time, fi);
                                scan->append_current_to_time_elements (fi, index_time);
                                fi++;
                        }
                }
                g_free (data);

                // trailer may have frame times
                g_message ("SER reading trailer, frame times");
                gint64 frame_times[ser_header.FrameCount];
                f.read ((char*)frame_times, sizeof (guint64) * ser_header.FrameCount);

                fi=0;
                if (f.good ()){
                        guint64 start=frame_times[0];
                        for (int index_time=0; index_time<ser_header.FrameCount; ++index_time){
                                if (sf > 0)
                                        if (index_time < sf)
                                                continue;
                                if (ef > 0)
                                        if (index_time >= ef)
                                                break;
                                g_message ("[%d] #%05d @ %llu %g s", fi, index_time, frame_times[index_time], (double)(frame_times[index_time]-start)/10000000.);
                                if (index_time % tb == 0){
                                        scan->set_nth_time_element_frame_time (fi, (double)(frame_times[index_time]-start)/10000000.);
                                        fi++;
                                }
                        }
                } else {
                        if (ser_header.FrameCount > 2){
                                g_message ("?? #%05d @ %llu ??", 0, frame_times[0]);
                                g_message ("?? #%05d @ %llu ??", 1, frame_times[1]);
                        }
                        g_message ("No or inclmplete SER trailer.\n");
                }
        } else if (!SER){
                // Read Img Data.
                size_t frame_buffer_size =  (size_t)h.dimensions_xyzb[0] * (size_t)h.dimensions_xyzb[1];
                guint16 *data;
                data = g_new (guint16, frame_buffer_size);
                
                scan->data.s.nvalues = 1;
                scan->mem2d->Resize (scan->data.s.nx, scan->data.s.ny, 1, ZD_FLOAT);
                // convert data
                for (int frame=0;;++frame){
                        g_message ("Reading Frame: %d", frame);
                        f.read ((char*)data, sizeof (guint16) * frame_buffer_size);
                        if (! f.good ()){
                                g_warning ("Ser data read EOF. #Frames read: %d", frame);
                                break;
                        }
                        guint16 *p = data;
                        for (int row=0; row < scan->mem2d->GetNy (); ++row){
                                for (int col=0; col < scan->mem2d->GetNx (); ++col){
                                        double bin = 0.; // binning
                                        for (int ib = 0; ib<h.bin[0]; ib++)
                                                for (int jb = 0; jb<h.bin[1]; jb++)
                                                        bin += (double)*(p+ib+jb*h.dimensions_xyzb[0]);
                                        p+=h.bin[0];
                                        scan->mem2d->PutDataPkt (bin, col, row);
                                }
                                p+=(h.bin[1]-1)*h.dimensions_xyzb[0];
                        }
                }
                g_free (data);
                gapp->progress_info_close ();
                scan->retrieve_time_element (0);
        }
        f.close ();
       
	scan->data.orgmode = SCAN_ORG_CENTER;
	scan->mem2d->data->MkXLookup (-scan->data.s.rx/2., scan->data.s.rx/2.);
	scan->mem2d->data->MkYLookup (scan->data.s.ry/2., -scan->data.s.ry/2.);
	scan->mem2d->data->MkVLookup (scan->data.s.rz/2., -scan->data.s.rz/2.);
  
	return FIO_OK; 
}


FIO_STATUS ser_ImExportFile::Write(){
#if 0
	GtkWidget *dialog = gtk_message_dialog_new (NULL,
						    GTK_DIALOG_DESTROY_WITH_PARENT,
						    GTK_MESSAGE_INFO,
						    GTK_BUTTONS_OK,
						    N_("Ser export.")
						    );
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
#endif
	const gchar *fname;
	ofstream f;

	if(strlen(name)>0)
		fname = (const char*)name;
	else
		fname = gapp->file_dialog("File Export: Ser"," ",file_mask,"","Ser write");
	if (fname == NULL) return FIO_NO_NAME;

	// check if we like to handle this
	if (strncmp(fname+strlen(fname)-4,".asc",4))
		return FIO_NOT_RESPONSIBLE_FOR_THAT_FILE;

	f.open(name, ios::out);
	if (!f.good())
	        return status=FIO_OPEN_ERR;

	for (int row=0; row < scan->mem2d->GetNy (); ++row){
		for (int col=0; col < scan->mem2d->GetNx (); ++col)
			f << scan->mem2d->GetDataPkt (col, row) << " ";
		f << std::endl;
	}

	f.close ();

	return FIO_OK; 
}

// Plugin's Notify Cb's, registered to be called on file load/save to check file
// return via filepointer, it is set to Zero or passed as Zero if file has been processed!
// That's all fine, you should just change the Text Stings below...



static void ser_import_filecheck_load_callback (gpointer data ){
	gchar **fn = (gchar**)data;
	if (*fn){
		PI_DEBUG (DBG_L2, 
			  "Check File: ser_import_filecheck_load_callback called with >"
			  << *fn << "<" );

		Scan *dst = gapp->xsm->GetActiveScan();
		if(!dst){ 
			gapp->xsm->ActivateFreeChannel();
			dst = gapp->xsm->GetActiveScan();
		}
		ser_ImExportFile fileobj (dst, *fn);

		FIO_STATUS ret = fileobj.Read(); 
		if (ret != FIO_OK){ 
			// I'am responsible! (But failed)
			if (ret != FIO_NOT_RESPONSIBLE_FOR_THAT_FILE)
				*fn=NULL;
			// no more data: remove allocated and unused scan now, force!
//			gapp->xsm->SetMode(-1, ID_CH_M_OFF, TRUE); 
			PI_DEBUG (DBG_L2, "Read Error " << ((int)ret) << "!!!!!!!!" );
		}else{
			// got it!
			*fn=NULL;

			// Now update gxsm main window data fields
			gapp->xsm->ActiveScan->GetDataSet(gapp->xsm->data);
			gapp->spm_update_all();
			dst->draw();
		}
	}else{
		PI_DEBUG (DBG_L2, "ser_import_filecheck_load: Skipping" );
	}
}

static void ser_import_filecheck_save_callback (gpointer data ){
	gchar **fn = (gchar**)data;
	if (*fn){
		Scan *src;
		PI_DEBUG (DBG_L2,
			  "Check File: ser_import_filecheck_save_callback called with >"
			  << *fn << "<" );

		ser_ImExportFile fileobj (src = gapp->xsm->GetActiveScan(), *fn);

		FIO_STATUS ret;
		ret = fileobj.Write(); 

		if(ret != FIO_OK){
			// I'am responsible! (But failed)
			if (ret != FIO_NOT_RESPONSIBLE_FOR_THAT_FILE)
				*fn=NULL;
			PI_DEBUG (DBG_L2, "Write Error " << ((int)ret) << "!!!!!!!!" );
		}else{
			// write done!
			*fn=NULL;
		}
	}else{
		PI_DEBUG (DBG_L2, "ser_import_filecheck_save: Skipping >" << *fn << "<" );
	}
}

// Menu Call Back Fkte

static void ser_import_import_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gchar **help = g_strsplit (ser_import_pi.help, ",", 2);
	gchar *dlgid = g_strconcat (ser_import_pi.name, "-import", NULL);
	gchar *fn = gapp->file_dialog_load (help[0], NULL, file_mask, NULL);
	g_strfreev (help); 
	g_free (dlgid);
	if (fn){
	        ser_import_filecheck_load_callback (&fn );
                g_free (fn);
	}
}

static void ser_import_export_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	gchar **help = g_strsplit (ser_import_pi.help, ",", 2);
	gchar *dlgid = g_strconcat (ser_import_pi.name, "-export", NULL);
	gchar *fn = gapp->file_dialog_save (help[1], NULL, file_mask, NULL);
	g_strfreev (help); 
	g_free (dlgid);
	if (fn){
	  	ser_import_filecheck_save_callback (&fn );
                g_free (fn);
	}
}
