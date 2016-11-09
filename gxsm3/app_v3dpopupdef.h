/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
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

/* The menu definitions: File/Exit and Help/About are mandatory */

// need math prototypes...
#include "xsmmath.h"
#include "app_v3dcontrol.h"

/* Definition of the Scan - Pop Up menu */

#if 0
static GnomeUIInfo scan_v3d_mode_menu[] = {
  GNOMEUIINFO_ITEM_NONE(N_("Off"),N_("kill channel"), (gpointer) V3dControl::SetOff_callback),
  GNOMEUIINFO_ITEM_NONE(N_("Active"),N_("activate this channel"), (gpointer) V3dControl::Activate_callback),
  GNOMEUIINFO_ITEM_NONE(N_("On"),N_("set channel mode to On"), (gpointer) V3dControl::SetOn_callback),
  GNOMEUIINFO_ITEM_NONE(N_("Math"),N_("set channel mode to Math"), (gpointer) V3dControl::SetMath_callback),
  GNOMEUIINFO_ITEM_NONE(N_("X"),N_("set channel mode to X"), (gpointer) V3dControl::SetX_callback),
  GNOMEUIINFO_END
};

static GnomeUIInfo scan_v3d_file_menu[] = {
  GNOMEUIINFO_ITEM_STOCK(N_("_Open here"), N_("load scan into this channel"), 
			 (gpointer) V3dControl::view_file_openhere_callback, GNOME_STOCK_MENU_OPEN),
  GNOMEUIINFO_ITEM_STOCK(N_("_Save"), N_("Save this scans with automatically generated name"), 
			 (gpointer) V3dControl::view_file_save_callback, GNOME_STOCK_MENU_SAVE),
  GNOMEUIINFO_ITEM_STOCK(N_("Save _as..."), N_("Save this scan with a new name"), 
			 (gpointer) V3dControl::view_file_save_as_callback, GNOME_STOCK_MENU_SAVE_AS),
  GNOMEUIINFO_ITEM_STOCK(N_("Save GL view as Image..."), N_("Save the current 3D view as image"), 
			 (gpointer) V3dControl::view_file_save_image_callback, GNOME_STOCK_MENU_SAVE_AS),
  GNOMEUIINFO_MENU_PRINT_ITEM( (gpointer) V3dControl::view_file_print_callback, NULL ),
  GNOMEUIINFO_ITEM_STOCK(N_("Close"), N_("kill this scan"), 
			 (gpointer) V3dControl::view_file_kill_callback, GNOME_STOCK_MENU_SAVE_AS),
  GNOMEUIINFO_END
};

static GnomeUIInfo scan_v3d_view_modes_radiolist[] = {
  GNOMEUIINFO_RADIOITEM(N_("Quick"), N_("quick view"), (gpointer) V3dControl::view_view_quick_callback, NULL),
  GNOMEUIINFO_RADIOITEM(N_("Direct"), N_("direct view"), (gpointer) V3dControl::view_view_direct_callback, NULL),
  GNOMEUIINFO_RADIOITEM(N_("Logarithmic"), N_("logarithmic view"), (gpointer) V3dControl::view_view_log_callback, NULL),
  GNOMEUIINFO_RADIOITEM(N_("Horizont"), N_("horizontal view - subtract mean/line"), (gpointer) V3dControl::view_view_horizont_callback, NULL),
  GNOMEUIINFO_RADIOITEM(N_("Periodic"), N_("periodic view of greys/colors"), (gpointer) V3dControl::view_view_periodic_callback, NULL),
  GNOMEUIINFO_RADIOITEM(N_("Differential"), N_("differential view of greys/colors"), (gpointer) V3dControl::view_view_differential_callback, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo scan_v3d_view_menu[] = {
  GNOMEUIINFO_RADIOLIST(scan_v3d_view_modes_radiolist),
  GNOMEUIINFO_END
};

static GnomeUIInfo scan_v3d_gl_wheel_mode_radiolist[] = {
  GNOMEUIINFO_RADIOITEM(N_("Zoom"), N_("use mouse wheel for zoom"), (gpointer) V3dControl::view_GL_Wh_zoom_callback, NULL),
  GNOMEUIINFO_RADIOITEM(N_("H Scale"), N_("use mouse wheel for Height (Z) scale"), (gpointer) V3dControl::view_GL_Wh_Zskl_callback, NULL),
  GNOMEUIINFO_RADIOITEM(N_("Rotate X"), N_("use mouse wheel to rotate X"), (gpointer) V3dControl::view_GL_Wh_rotX_callback, NULL),
  GNOMEUIINFO_RADIOITEM(N_("Rotate Y"), N_("use mouse wheel to rotate Y"), (gpointer) V3dControl::view_GL_Wh_rotY_callback, NULL),
  GNOMEUIINFO_RADIOITEM(N_("Rotate Z"), N_("use mouse wheel to rotate Z"), (gpointer) V3dControl::view_GL_Wh_rotZ_callback, NULL),
  GNOMEUIINFO_END
};
static GnomeUIInfo scan_v3d_gl_wheel_mode_menu[] = {
	GNOMEUIINFO_ITEM_NONE(N_("Mouse Wheel for"),NULL, NULL),
	GNOMEUIINFO_RADIOLIST(scan_v3d_gl_wheel_mode_radiolist),
	GNOMEUIINFO_END
};

static GnomeUIInfo scan_v3d_glopt_menu[] = {
  GNOMEUIINFO_TOGGLEITEM(N_("Zero Plane/Box"), N_("show Z=Zero plane and box"), (gpointer) V3dControl::view_GL_nZP_callback, NULL),
  GNOMEUIINFO_TOGGLEITEM(N_("Mesh"), N_("toggle mesh/solid"), (gpointer) V3dControl::view_GL_Mesh_callback, NULL),
  GNOMEUIINFO_TOGGLEITEM(N_("Smooth"), N_("toggle Smooth Shading on/off"), (gpointer) V3dControl::view_GL_Smooth_callback, NULL),
  GNOMEUIINFO_TOGGLEITEM(N_("Tickmarks"), N_("toggle tickmarks on/off"), (gpointer) V3dControl::view_GL_Ticks_callback, NULL),
  GNOMEUIINFO_END
};


static GnomeUIInfo scan_v3d_popup_menu[] = {
  GNOMEUIINFO_ITEM_NONE(N_("Activate"),N_("activate this Channel"), (gpointer) V3dControl::Activate_callback),
  GNOMEUIINFO_ITEM_NONE(N_("AutoDisp"),N_("autoscale Area"), (gpointer) V3dControl::AutoDisp_callback),
  GNOMEUIINFO_ITEM_NONE(N_("Update All"),N_("force apply of all settings"), (gpointer) V3dControl::apply_all_callback),
  GNOMEUIINFO_SUBTREE (N_("Mode"), scan_v3d_mode_menu),
  GNOMEUIINFO_SUBTREE (N_("File"), scan_v3d_file_menu),
  GNOMEUIINFO_SUBTREE (N_("View"), scan_v3d_view_menu),
  GNOMEUIINFO_SUBTREE (N_("GL Options"), scan_v3d_glopt_menu),
  GNOMEUIINFO_SUBTREE (N_("3D Control"), scan_v3d_gl_wheel_mode_menu),
  GNOMEUIINFO_ITEM_NONE(N_("Scene Setup.."),N_("Open Scene MesaGL Preferences Dialog"), (gpointer) V3dControl::preferences_callback),
  GNOMEUIINFO_END
};

#endif
