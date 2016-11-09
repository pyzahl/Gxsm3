/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Hardware Interface Plugin Name: grab_v4l.C
 * ===============================================
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
 * All "% PlugInXXX" commentary tags are mandatory
 * All "% OptPlugInXXX" tags are optional and can be removed or commented in
 * --------------------------------------------------------------------------------
% BeginPlugInDocuSection
% PlugInDocuCaption: Video4Linux Grabber (experimental, to be ported)
% PlugInName: grab_v4l
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: Hardware/video4linux-HwI

% PlugInDescription
This is an experimental hardware interface plugin.
Grabbing video data using the Video4Linux (v4l) device.

It's using v4l, so set Hardware/Device to the desired /dev/videoXX device!

% PlugInUsage
Set the \GxsmPref{Hardware}{Card} to ''grab\_v4l''.

% OptPlugInSources
v4l device, i.e. (S)-Video/TV/\dots

% OptPlugInDest
Usual scan destination channel.

% OptPlugInNote
Experimental.

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */

#include "config.h"
#include "gxsm/plugin.h"
#include "gxsm/xsmhard.h"

// Define HwI PlugIn reference name here, this is what is listed later within "Preferenced Dialog"
// i.e. the string selected for "Hardware/Card"!
#define THIS_HWI_PLUGIN_NAME "video4linux"

// Plugin Prototypes
static void grab_v4l_init( void );
static void grab_v4l_about( void );
static void grab_v4l_configure( void );
static void grab_v4l_cleanup( void );

extern "C" gint gxsm_v4l_open_video4l ();
extern "C" gint gxsm_v4l_close_video4l ();
extern "C" gint gxsm_v4l_maxwidth  ();
extern "C" gint gxsm_v4l_maxheight ();
extern "C" gint gxsm_v4l_win_width  ();
extern "C" gint gxsm_v4l_win_height ();
extern "C" gint gxsm_v4l_grab_video4l ();
extern "C" gint gxsm_v4l_get_pixel (int *r, int *g, int *b);

// Fill in the GxsmPlugin Description here
GxsmPlugin grab_v4l_pi = {
  NULL,                   // filled in and used by Gxsm, don't touch !
  NULL,                   // filled in and used by Gxsm, don't touch !
  0,                      // filled in and used by Gxsm, don't touch !
  NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
                          // filled in here by Gxsm on Plugin load, 
                          // just after init() is called !!!
  // ----------------------------------------------------------------------
  // Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
  "grab_v4l-"
  "HW-INT-1S-RGB",
  // Plugin's Category - used to autodecide on Pluginloading or ignoring
  // In this case of Hardware-Interface-Plugin here is the interface-name required
  // this is the string selected for "Hardware/Card"!
  THIS_HWI_PLUGIN_NAME,
  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
  "Video for Linux interface.",                   
  // Author(s)
  "Percy Zahl",
  // Menupath to position where it is appendet to -- not used by HwI PIs
  N_("Hardware/"),
  // Menuentry -- not used by HwI PIs
  N_(THIS_HWI_PLUGIN_NAME"-HwI"),
  // help text shown on menu
  N_("This is a "THIS_HWI_PLUGIN_NAME" - GXSM Hardware Interface"),
  // more info...
  "N/A",
  NULL,          // error msg, plugin may put error status msg here later
  NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
  // init-function pointer, can be "NULL", 
  // called if present at plugin load
  grab_v4l_init,  
  // query-function pointer, can be "NULL", 
  // called if present after plugin init to let plugin manage it install itself
  NULL, // query should be "NULL" for Gxsm-Math-Plugin !!!
  // about-function, can be "NULL"
  // can be called by "Plugin Details"
  grab_v4l_about,
  // configure-function, can be "NULL"
  // can be called by "Plugin Details"
  grab_v4l_configure,
  // run-function, can be "NULL", if non-Zero and no query defined, 
  // it is called on menupath->"plugin"
  NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
  // cleanup-function, can be "NULL"
  // called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

  grab_v4l_cleanup
};


// Text used in Aboutbox, please update!!
static const char *about_text = N_("GXSM grab_v4l Plugin\n\n"
                                   "Video for Linux frame grabber.");

/* Here we go... */

/*
 * GXSM V4L Hardware Interface Class
 * ============================================================
 */

class gxsm_v4l : public XSM_Hardware{
public:
	gxsm_v4l();
	virtual ~gxsm_v4l();
	
	virtual void PutParameter(void *src, int grp=0); /* universelle Parameterübergabe */
	virtual void ExecCmd(int Cmd);
	virtual int  WaitExec(int data);
	
	/* Parameter  */
	virtual long GetMaxPointsPerLine(){ return gxsm_v4l_maxwidth (); };
	virtual long GetMaxLines(){ return gxsm_v4l_maxheight (); };
	virtual long GetMaxChannels(){ return 1L; };
	virtual void SetDxDy(int dx, int dy);
	virtual void SetOffset(long x, long y);
	virtual void SetNx(long nx);
	virtual void SetAlpha(double alpha);
	
	virtual void MovetoXY(long x, long y);
	virtual void StartScan2D();
	virtual void ScanLineM(int yindex, int xdir, int muxmode, Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0=0 );
	virtual void EndScan2D();
	virtual gchar* get_info(){ 
		return g_strdup("*--GXSM HwI Plugin: v4l::XSM_Hardware --*\n"
				"video4linux on /dev/video0 is connected!\n"
				"*--Features--*\n"
				"SCAN: Yes\n"
				"*--ExecCMD options--*\n"
				"*--EOF--*\n"
			);
	};
	
private:
	DSP_Param dspPar;


};

/*
 * PI global
 */

gxsm_v4l *v4l_hardware = NULL;

/* Konstruktor: device open
 * ==================================================
 */
gxsm_v4l::gxsm_v4l():XSM_Hardware(){
	// open device?? or just on demand?
	gxsm_v4l_open_video4l();	
}

/* Destruktor:
 * ==================================================
 * Hardware "abtrennen"
 */
gxsm_v4l::~gxsm_v4l(){
	gxsm_v4l_close_video4l ();
}

/* Übergeordnete Parameterübergabefunktionen PC => PC31/DSP
 * ========================================================
 * virtual !
 */
void gxsm_v4l::PutParameter(void *src, int grp){
}

void gxsm_v4l::ExecCmd(int Cmd){
}
int  gxsm_v4l::WaitExec(int data){ return 0; }
void gxsm_v4l::MovetoXY(long x, long y){ rx=x; ry=y; }
void gxsm_v4l::SetDxDy(int dx, int dy){  
  Dx = dx; 
  Dy = dy;
}
void gxsm_v4l::SetOffset(long x, long y){
  rotoffx = x; rotoffy = y;
}
void gxsm_v4l::SetAlpha(double alpha){ 
  Alpha=M_PI*alpha/180.;
  rotmyy = rotmxx = cos(Alpha);
  rotmyx = -(rotmxy = sin(Alpha));
}
void gxsm_v4l::SetNx(long nx){ 
  Nx=nx;
}


void gxsm_v4l::StartScan2D(){ 
	// Grab Frame from V4L
	gxsm_v4l_grab_video4l ();
}

void gxsm_v4l::EndScan2D(){ 
	;
}




void gxsm_v4l::ScanLineM(int yindex, int xdir, int muxmode, Mem2d *Mob[MAX_SRCS_CHANNELS], int ix0 ){
	int r,g,b;
	if (yindex < 0){ // 2D capture
		if (yindex != -1) return; // XP/XM init cycle
		if(Mob[0])
			for(int j=0; j < gxsm_v4l_win_height () && j<Mob[0]->GetNy (); j++){
				for(int i=0; i<gxsm_v4l_win_width  () && i < Mob[0]->GetNx (); i++)
					Mob[0]->PutDataPkt ((double)i*j, i, j);
				if(!(j%32))
					gapp->check_events();
			}
		return;
	}

	if(yindex >=  gxsm_v4l_win_height ()) 
		return;

	if(Mob[0])
		for(int i=0; i < gxsm_v4l_win_width () && i < Mob[0]->GetNx (); i++)
			Mob[0]->PutDataPkt((double) gxsm_v4l_get_pixel (&r, &g, &b), i, yindex);

	if(!(yindex%32))
		gapp->check_events();
}


/* 
 * PI essential members
 */

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
  grab_v4l_pi.description = g_strdup_printf(N_("GXSM HwI grab_v4l plugin %s"), VERSION);
  return &grab_v4l_pi; 
}

// Symbol "get_gxsm_hwi_hardware_class" is resolved by dlsym from Gxsm for all HwI type PIs, 
// Essential Plugin Function!!
XSM_Hardware *get_gxsm_hwi_hardware_class ( void *data ) {
	return v4l_hardware;
}

// init-Function
static void grab_v4l_init(void)
{
	PI_DEBUG (DBG_L2, "grab_v4l Plugin Init");
	v4l_hardware = new gxsm_v4l ();
 }

// about-Function
static void grab_v4l_about(void)
{
	const gchar *authors[] = { grab_v4l_pi.authors, NULL};
	gtk_show_about_dialog (NULL, "program-name",  grab_v4l_pi.name,
					"version", VERSION,
					  "license", GTK_LICENSE_GPL_3_0,
					  "comments", about_text,
					  "authors", authors,
					  NULL
				));
}

// configure-Function
static void grab_v4l_configure(void)
{
	if(grab_v4l_pi.app)
		grab_v4l_pi.app->message("grab_v4l Plugin Configuration");
}

// cleanup-Function
static void grab_v4l_cleanup(void)
{
	PI_DEBUG (DBG_L2, "grab_v4l Plugin Cleanup");
	delete v4l_hardware;
	v4l_hardware = NULL;
}



