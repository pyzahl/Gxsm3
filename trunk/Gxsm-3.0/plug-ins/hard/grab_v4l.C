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
#include "gxsm3/plugin.h"
#include "gxsm3/xsmhard.h"

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
  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Detail])
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



