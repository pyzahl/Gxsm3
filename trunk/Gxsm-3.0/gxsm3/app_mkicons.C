// /* Gxsm - Gnome X Scanning Microscopy
//  * universal STM/AFM/SARLS/SPALEED/... controlling and
//  * data analysis software
//  * 
//  * Copyright (C) 1999,2000,2001,2002,2003 Percy Zahl
//  *
//  * Authors: Percy Zahl <zahl@users.sf.net>
//  * additional features: Andreas Klust <klust@users.sf.net>
//  * WWW Home: http://gxsm.sf.net
//  *
//  * This program is free software; you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation; either version 2 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program; if not, write to the Free Software
//  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
//  */
// 
// /* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */
// 
// #include <locale.h>
// #include <libintl.h>
// 
// #include "gxsm_app.h"
// #include "app_mkicons.h"
// 
// #include "unit.h"
// #include "pcs.h"
// #include "xsm_mkicons.h"
// 
// typedef union OptIndex {
//   struct { unsigned char oi, x, y, z; } s;
//   unsigned long   l; // default Eintrag
// };
// 
// char *Opt_Paper[]       = {"A4", "Letter", NULL};
// char *Opt_Resolution[]  = {"300dpi", "600dpi", "1200dpi", NULL};
// char *Opt_ERegression[] = {"no", "E 30% Rand", "E 5% Rand", NULL};
// char *Opt_LRegression[] = {"no", "lin.Reg.", NULL};
// char *Opt_ViewMode[]    = {"default", "quick", "direct", "logarithmic", "perodic", "horizontal", NULL};
// char *Opt_AutoScaling[] = {"default", "auto 5% Rand", "auto 20% Rand", "auto 30% Rand", NULL};
// char *Opt_Scaling[]     = {"no", "min-max", "Cps-lo-hi", NULL};
// 
// 
// #define MK_ICONS_KEYBASE "MkIcons"
// 
// MKICONS_OPTIONS OptionsList[] = {
// 	{ "Paper",   Opt_Paper,  "AL", 0 },
// 	{ "Resolution",   Opt_Resolution,  "36C", 1 },
// 	{ "E-Regression", Opt_ERegression, "-Ee", 0 },
// 	{ "L-Regression", Opt_LRegression, "-l",  0 },
// 	{ "View-Mode",    Opt_ViewMode,    "-qdlph", 0 },
// 	{ "Auto-Scaling", Opt_AutoScaling, "-123",  0 },
// 	{ "Scaling",      Opt_Scaling,     "-ac",  0 },
// 	{ NULL, 0 }
// };
// 
// 
// MkIconsControl::MkIconsControl (){
// 	// get defaults
// 
// 	XSM_DEBUG(DBG_L4, "MkIconsControl::MkIconsControl");
// 
// 	XsmRescourceManager xrm(MK_ICONS_KEYBASE);
// 	
// 	icondata = new MkIconsData(
// 		xrm.GetStr("SourcePath","."),
// 		xrm.GetStr("DestPath","/tmp/icons.ps"),
// 		xrm.GetStr("SourceMask","*.nc"),
// 		xrm.GetStr("Options","----------"),
// 		xrm.GetStr("IconFile","icons.ps")
// 		);
// 	icondata = new MkIconsData();
// 
// 	XSM_DEBUG(DBG_L4, "MkIconsControl::MkIconsControl OK.");
// }
// 
// MkIconsControl::~MkIconsControl (){
// 	// save defaults
// 
// 	XSM_DEBUG(DBG_L4, "MkIconsControl::~MkIconsControl");
// 	MKICONS_OPTIONS *opt;
// 	XsmRescourceManager xrm(MK_ICONS_KEYBASE);
// 	for(opt = OptionsList; opt->name; ++opt)
// 		xrm.Put(opt->name, opt->init);
// 	
// 	xrm.Put("SourcePath", icondata->pathname);
// 	xrm.Put("DestPath", icondata->outputname);
// 	xrm.Put("SourceMask", icondata->mask);
// 	xrm.Put("Options", icondata->options);
// 	xrm.Put("IconFile", icondata->name);
// 	
// 	XSM_DEBUG(DBG_L4, "MkIconsControl::~MkIconsControl done.");
// 	delete icondata;
// }
// 
// void MkIconsControl::run(){
// 	GtkWidget *dialog;
// 	GtkWidget *vbox;
// 	GtkWidget *hbox;
// 	GtkWidget *VarName;
// 	GtkWidget *variable;
// 	GtkWidget *help;
// 	GtkWidget *separator;
// 
// 	XSM_DEBUG(DBG_L4, "MkIconsControl::run");
// 	
// 	dialog = gtk_dialog_new_with_buttons (N_("GXSM make icons"),
// 					      NULL, // (GTK_WINDOW (gtk_widget_get_toplevel (widget))
// 					      (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
// 					      N_("1x1"), 1, N_("2x3"), 2, N_("4x6"), 4, N_("6x9"), 6,
// 					      _("_Cancel"), GTK_RESPONSE_CANCEL,
// 					      NULL); 
//         
// 	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
// 	gtk_widget_show (vbox);
// 	
// 	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
// 		     vbox, TRUE, TRUE, GXSM_WIDGET_PAD);
//         
// 
// 	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
// 	gtk_widget_show (hbox);
// 	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
// 	
// 	VarName = gtk_label_new (N_("Source Path"));
// 	gtk_widget_set_size_request (VarName, 100, -1);
// 	gtk_widget_show (VarName);
// 	gtk_label_set_justify(GTK_LABEL(VarName), GTK_JUSTIFY_LEFT);
// 	gtk_misc_set_alignment (GTK_MISC (VarName), 0.0, 0.5);
// 	gtk_misc_set_padding (GTK_MISC (VarName), 5, 0);
// 	gtk_box_pack_start (GTK_BOX (hbox), VarName, TRUE, TRUE, 0);
// 	
// 	SrcPath = variable = gtk_entry_new ();
// 	gtk_widget_show (variable);
// 	gtk_box_pack_start (GTK_BOX (hbox), variable, TRUE, TRUE, 0);
// 	gtk_entry_set_text (GTK_ENTRY (variable), icondata->pathname);
// 	
// 	help = gtk_button_new_with_label (N_("Help"));
// 	gtk_widget_show (help);
// 	gtk_box_pack_start (GTK_BOX (hbox), help, TRUE, TRUE, 0);
// 	g_signal_connect (G_OBJECT (help), "clicked",
// 			    G_CALLBACK (show_info_callback),
// 			    (void*)(N_("Set to full pathname of data source directory.")));
// 	
// 	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
// 	gtk_widget_show (hbox);
// 	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
//   
// 	VarName = gtk_label_new (N_("Selection Mask"));
// 	gtk_widget_set_size_request (VarName, 100, -1);
// 	gtk_widget_show (VarName);
// 	gtk_label_set_justify(GTK_LABEL(VarName), GTK_JUSTIFY_LEFT);
// 	gtk_misc_set_alignment (GTK_MISC (VarName), 0.0, 0.5);
// 	gtk_misc_set_padding (GTK_MISC (VarName), 5, 0);
// 	gtk_box_pack_start (GTK_BOX (hbox), VarName, TRUE, TRUE, 0);
//   
// 	SrcMask = variable = gtk_entry_new ();
// 	gtk_widget_show (variable);
// 	gtk_box_pack_start (GTK_BOX (hbox), variable, TRUE, TRUE, 0);
// 	gtk_entry_set_text (GTK_ENTRY (variable), icondata->mask);
// 
// 	help = gtk_button_new_with_label (N_("Help"));
// 	gtk_widget_show (help);
// 	gtk_box_pack_start (GTK_BOX (hbox), help, TRUE, TRUE, 0);
// 	g_signal_connect (G_OBJECT (help), "clicked",
// 			    G_CALLBACK (show_info_callback),
// 			    (void*)(N_("Select subset of files via wildcard.")));
// 	
// 	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
// 	gtk_widget_show (hbox);
// 	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
// 	
// 	VarName = gtk_label_new (N_("Icon Filename"));
// 	gtk_widget_set_size_request (VarName, 100, -1);
// 	gtk_widget_show (VarName);
// 	gtk_label_set_justify(GTK_LABEL(VarName), GTK_JUSTIFY_LEFT);
// 	gtk_misc_set_alignment (GTK_MISC (VarName), 0.0, 0.5);
// 	gtk_misc_set_padding (GTK_MISC (VarName), 5, 0);
// 	gtk_box_pack_start (GTK_BOX (hbox), VarName, TRUE, TRUE, 0);
//   
// 	IconName = variable = gtk_entry_new ();
// 	gtk_widget_show (variable);
// 	gtk_box_pack_start (GTK_BOX (hbox), variable, TRUE, TRUE, 0);
// 	gtk_entry_set_text (GTK_ENTRY (variable), icondata->outputname);
// 	
// 	help = gtk_button_new_with_label (N_("Help"));
// 	gtk_widget_show (help);
// 	gtk_box_pack_start (GTK_BOX (hbox), help, TRUE, TRUE, 0);
// 	g_signal_connect (G_OBJECT (help), "clicked",
// 			    G_CALLBACK (show_info_callback),
// 			    (void*)(N_("Full Pathname to Iconfile, openmode is append !")));
// 	
// 	separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
// 	/* The last 3 arguments to gtk_box_pack_start are:
// 	 * expand, fill, padding. */
// 	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, TRUE, 5);
// 	gtk_widget_show (separator);
// 	
// 	// make Option Menus
// 	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
// 	gtk_widget_show (hbox);
// 	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
// 
// 	GtkWidget *mvbox, *label, *wid, *menu, *menuitem;
// 	MKICONS_OPTIONS *opt;
// 	char **item;
// 	OptIndex OptI;
// 	int i,j;
// 
// 	XSM_DEBUG(DBG_L4, "MkIconsControl::run - Setting options");
// 
// 
// 	XsmRescourceManager xrm(MK_ICONS_KEYBASE);
// 	for(j=0, opt = OptionsList; opt->name; ++j, ++opt){
// 		// Init from rescources
// 		gchar *idefault=g_strdup_printf("%d", opt->init);
// 		xrm.Get(opt->name, &opt->init, idefault);
// 		g_free(idefault);
// 
// 		mvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
// 		gtk_widget_show (mvbox);
// 		gtk_box_pack_start (GTK_BOX (hbox), mvbox, TRUE, TRUE, 0);
// 		
// 		label = gtk_label_new (opt->name);
// 		gtk_widget_show (label);
// 		gtk_container_add (GTK_CONTAINER (mvbox), label);
// 		
// 		wid = gtk_option_menu_new ();
// 		gtk_widget_show (wid);
// 		gtk_container_add (GTK_CONTAINER (mvbox), wid);
// 		
// 		menu = gtk_menu_new ();
// 		
// 		// fill options in
// 		for(i=0, item=opt->list; *item; ++i, ++item){
// 			// make menuitem
// 			menuitem = gtk_menu_item_new_with_label (*item);
// 			gtk_widget_show (menuitem);
// 			gtk_menu_append (GTK_MENU (menu), menuitem);
// 			/* connect with signal-handler if selected */
// 			OptI.l = 0L;
// 			OptI.s.oi = j;
// 			OptI.s.x  = i;
// 			g_object_set_data(G_OBJECT (menuitem), "optindex", GINT_TO_POINTER (OptI.l));
// 			g_signal_connect (G_OBJECT (menuitem), "activate",
// 					    G_CALLBACK (MkIconsControl::option_choice_callback),
// 					    this);
// 		}
// 		gtk_option_menu_set_menu (GTK_OPTION_MENU (wid), menu);
// 		gtk_option_menu_set_history (GTK_OPTION_MENU (wid), opt->init);
// 	}
// 
// 	XSM_DEBUG(DBG_L4, "MkIconsControl::run - Setting options ... OK");
// 	
// 	XSM_DEBUG(DBG_L4, "MkIconsControl::run - Dlg show, run");
// 
// 	gtk_widget_show(dialog);
//   	dlg_clicked (gtk_dialog_run (GTK_DIALOG(dialog)));
// 	gtk_widget_destroy (dialog);
// 
// }
// 
// void MkIconsControl::option_choice_callback(GtkWidget *widget, MkIconsControl *mki){
// 	OptIndex i;
// 	i.l=(long)g_object_get_data( G_OBJECT (widget), "optindex");
// 	mki->icondata->options[i.s.oi] = OptionsList[i.s.oi].id[OptionsList[i.s.oi].init=i.s.x];
// 	XSM_DEBUG(DBG_L2, "MkIconsControl::option_choice_callback=" << mki->icondata->options );
// }
// 
// void MkIconsControl::dlg_clicked(gint response){
// 
// 	XSM_DEBUG(DBG_L4, "MkIconsControl::clicked - " << response);
// 
// 	g_free(icondata->pathname); 
// 	icondata->pathname = g_strdup(gtk_entry_get_text (GTK_ENTRY (SrcPath)));
// 	
// 	g_free(icondata->mask); 
// 	icondata->mask = g_strdup(gtk_entry_get_text (GTK_ENTRY (SrcMask)));
// 	
// 	g_free(icondata->outputname); 
// 	icondata->outputname = g_strdup(gtk_entry_get_text (GTK_ENTRY (IconName)));
// 
// 	switch(response){
// 	case 1:
// 		show_info_callback(NULL, N_("Generating 1x1 Icon Pages !"));
// 		icondata->nix=1;
// 		gapp->xsm->MkIcons(icondata);
// 		break;
// 	case 2:
// 		show_info_callback(NULL, N_("Generating 2x3 Icon Pages !"));
// 		icondata->nix=2;
// 		gapp->xsm->MkIcons(icondata);
// 		break;
// 	case 4: 
// 		show_info_callback(NULL, N_("Generating 4x6 Icon Pages !"));
// 		icondata->nix=4;
// 		gapp->xsm->MkIcons(icondata);
// 		break;
// 	case 6: 
// 		show_info_callback(NULL, N_("Generating 6x9 Icon Pages !"));
// 		icondata->nix=6;
// 		gapp->xsm->MkIcons(icondata);
// 		break;
// 	case GTK_RESPONSE_CANCEL: 
// 		break;
// 	}
// }
