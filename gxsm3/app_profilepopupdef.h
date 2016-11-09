/* Gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Copyright (C) 1999,2000,2001,2002,2003 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * additional features: Andreas Klust <klust@users.sf.net>
 * WWW Home: http://gxsm.sf.net
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

/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Definition of the Profile - Pop Up menu */

#if 0

static GnomeUIInfo profile_print_menu[] = {
  GNOMEUIINFO_ITEM_STOCK(N_("User command 1"), N_("Set in Prefs"), 
			 (gpointer) ProfileControl::file_print1_callback, GNOME_STOCK_MENU_PRINT),
  GNOMEUIINFO_ITEM_STOCK(N_("User command 2"), N_("Set in Prefs"), 
			 (gpointer) ProfileControl::file_print2_callback, GNOME_STOCK_MENU_PRINT),
  GNOMEUIINFO_ITEM_STOCK(N_("User command 3"), N_("Set in Prefs"), 
			 (gpointer) ProfileControl::file_print3_callback, GNOME_STOCK_MENU_PRINT),
  GNOMEUIINFO_ITEM_STOCK(N_("User command 4"), N_("Set in Prefs"), 
			 (gpointer) ProfileControl::file_print4_callback, GNOME_STOCK_MENU_PRINT),
  GNOMEUIINFO_ITEM_STOCK(N_("User command 5 (xmgrace)"), N_("View via xmgrace"), 
			 (gpointer) ProfileControl::file_print5_callback, GNOME_STOCK_MENU_PRINT),
  GNOMEUIINFO_ITEM_STOCK(N_("User command 6 (matplot)"), N_("View via matplotlib"), 
			 (gpointer) ProfileControl::file_print6_callback, GNOME_STOCK_MENU_PRINT),
  GNOMEUIINFO_END
};

static GnomeUIInfo profile_file_menu[] = {
  GNOMEUIINFO_ITEM_STOCK(N_("_Open"), N_("load profile"), 
			 (gpointer) ProfileControl::file_open_callback, GNOME_STOCK_MENU_OPEN),
  GNOMEUIINFO_ITEM_STOCK(N_("_Save"), N_("Save profile with automatically generated name"), 
			 (gpointer) ProfileControl::file_save_callback, GNOME_STOCK_MENU_SAVE),
  GNOMEUIINFO_ITEM_STOCK(N_("Save _as..."), N_("Save profile with a new name"), 
			 (gpointer) ProfileControl::file_save_as_callback, GNOME_STOCK_MENU_SAVE_AS),
  GNOMEUIINFO_ITEM_STOCK(N_("Save _Image"), N_("Save profile windo as image"), 
			 (gpointer) ProfileControl::file_save_image_callback, GNOME_STOCK_MENU_SAVE),
  GNOMEUIINFO_SUBTREE (N_("_Print"), profile_print_menu),
  GNOMEUIINFO_ITEM_STOCK(N_("Activate"), N_("Activate profile channel"), 
			 (gpointer) ProfileControl::file_activate_callback, GNOME_STOCK_MENU_PRINT),
  GNOMEUIINFO_ITEM_STOCK(N_("Close"), N_("Close this profile"), 
			 (gpointer) ProfileControl::file_close_callback, GNOME_STOCK_MENU_CLOSE),

  GNOMEUIINFO_END
};

static GnomeUIInfo profile_options_menu[] = {
  GNOMEUIINFO_TOGGLEITEM(N_("Y linregression"), N_("do lineregression on Y data"), 
			 (gpointer) ProfileControl::linreg_callback, NULL),
  GNOMEUIINFO_TOGGLEITEM(N_("Y PSD"), N_("show PSD of Y data"), 
			 (gpointer) ProfileControl::psd_callback, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_TOGGLEITEM(N_("Symbols"), N_("show Symbols"), 
			 (gpointer) ProfileControl::symbols_callback, NULL),
  GNOMEUIINFO_TOGGLEITEM(N_("Legend"), N_("show Legend"), 
			 (gpointer) ProfileControl::legend_callback, NULL),
  GNOMEUIINFO_TOGGLEITEM(N_("Ticmarks"), N_("show Ticmarks -- slower"), 
			 (gpointer) ProfileControl::tics_callback, NULL),
  GNOMEUIINFO_TOGGLEITEM(N_("no Gridlines"), N_("no Gridlines"), 
			 (gpointer) ProfileControl::nogrid_callback, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_ITEM_NONE(N_("Canvas Zoom In"), N_("Canvas Zoom In"), (gpointer) ProfileControl::zoom_in_callback),
  GNOMEUIINFO_ITEM_NONE(N_("Canvas Zoom Out"), N_("Canvas Zoom Out"), (gpointer) ProfileControl::zoom_out_callback),
  GNOMEUIINFO_ITEM_NONE(N_("Canvas Aspect Inc"), N_("Canvas Aspect Inc"), (gpointer) ProfileControl::aspect_inc_callback),
  GNOMEUIINFO_ITEM_NONE(N_("Canvas Aspect Dec"), N_("Canvas Aspect Dec"), (gpointer) ProfileControl::aspect_dec_callback),
  GNOMEUIINFO_ITEM_NONE(N_("make default"), N_("make default"), (gpointer)ProfileControl::canvas_size_store_callback),
  GNOMEUIINFO_END
};

static GnomeUIInfo profile_Yscaling_menu[] = {
  GNOMEUIINFO_TOGGLEITEM(N_("Y logarithmic"), N_("use logarithmic y scale"), 
			 (gpointer) ProfileControl::logy_callback, NULL),
  GNOMEUIINFO_SEPARATOR,

  GNOMEUIINFO_TOGGLEITEM(N_("Y hold"), N_("fix y range"), 
			 (gpointer)ProfileControl::yhold_callback, NULL),
  GNOMEUIINFO_TOGGLEITEM(N_("Y expand only"), N_("do only expansion on y range, hold old maximal range"), 
			 (gpointer)ProfileControl::yexpand_callback, NULL),
  GNOMEUIINFO_SEPARATOR,

  GNOMEUIINFO_ITEM_STOCK(N_("Y auto"), N_("Y autoscale "), 
			 (gpointer)ProfileControl::skl_Yauto_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("Y upper up"), N_("Y upper up "), 
			 (gpointer)ProfileControl::skl_Yupperup_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("Y upper down"), N_("Y upper down "), 
			 (gpointer)ProfileControl::skl_Yupperdn_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("Y lower up"), N_("Y lower up "), 
			 (gpointer)ProfileControl::skl_Ylowerup_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("Y lower down"), N_("Y lower down "), 
			 (gpointer)ProfileControl::skl_Ylowerdn_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("Y zoom in"), N_("Y zoom in "), 
			 (gpointer)ProfileControl::skl_Yzoomin_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("Y zoom out"), N_("Y zoom out "), 
			 (gpointer)ProfileControl::skl_Yzoomout_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("Y set bounds"), N_("manual Y set bound"), 
			 (gpointer)ProfileControl::skl_Yset_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_END
};

static GnomeUIInfo profile_Xscaling_menu[] = {
  GNOMEUIINFO_ITEM_STOCK(N_("X auto"), N_("X autoscale "), 
			 (gpointer)ProfileControl::skl_Xauto_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("X set bounds"), N_("manual X set bound"), 
			 (gpointer) ProfileControl::skl_Xset_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_END
};

static GnomeUIInfo profile_cursor_menu[] = {
  GNOMEUIINFO_TOGGLEITEM(N_("Cursor A"), N_("show cursor A"), 
			 (gpointer) ProfileControl::cur_Ashow_callback, NULL),
  GNOMEUIINFO_TOGGLEITEM(N_("Cursor B"), N_("show cursor B"), 
			 (gpointer) ProfileControl::cur_Bshow_callback, NULL),
  GNOMEUIINFO_ITEM_STOCK(N_("move A left"), N_("move cursor A left"), 
			 (gpointer) ProfileControl::cur_Aleft_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("move A next Max"), N_("find next right Maximum"), 
			 (gpointer) ProfileControl::cur_Armax_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("move A next Min"), N_("find next right Minimum"), 
			 (gpointer) ProfileControl::cur_Armin_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("move A prev Max"), N_("find next left Maximum"), 
			 (gpointer) ProfileControl::cur_Almax_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("move A prev Min"), N_("find next left Minimum"), 
			 (gpointer) ProfileControl::cur_Almin_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("move A right"), N_("move cursor A right"), 
			 (gpointer) ProfileControl::cur_Aright_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("move B left"), N_("move cursor B left"), 
			 (gpointer) ProfileControl::cur_Bleft_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("move B next Max"), N_("find next right Maximum"), 
			 (gpointer) ProfileControl::cur_Brmax_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("move B next Min"), N_("find next right Minimum"), 
			 (gpointer) ProfileControl::cur_Brmin_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("move B prev Max"), N_("find next left Maximum"), 
			 (gpointer) ProfileControl::cur_Blmax_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("move B prev Min"), N_("find next left Minimum"), 
			 (gpointer) ProfileControl::cur_Blmin_callback, GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_ITEM_STOCK(N_("move B right"), N_("move cursor B right"), 
			 (gpointer) ProfileControl::cur_Bright_callback,  GNOME_STOCK_MENU_BLANK),
  GNOMEUIINFO_END
};

static GnomeUIInfo profile_popup_menu[] = {
  GNOMEUIINFO_SUBTREE (N_("File"), profile_file_menu),
  GNOMEUIINFO_SUBTREE (N_("Options"), profile_options_menu),
  GNOMEUIINFO_SUBTREE (N_("Y Scaling"), profile_Yscaling_menu),
  GNOMEUIINFO_SUBTREE (N_("X Scaling"), profile_Xscaling_menu),
  GNOMEUIINFO_SUBTREE (N_("Cursor"), profile_cursor_menu),
  GNOMEUIINFO_END
};

#endif
