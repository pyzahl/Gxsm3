/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Gxsm Plugin Name: external_converter.C
 * ========================================
 * 
 * Copyright (C) 2006 The Free Software Foundation
 *
 * Authors: Thorsten Wagner <stm@users.sf.net>
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
% PlugInDocuCaption: external converter 
% PlugInName: external_converter
% PlugInAuthor: Thorsten Wagner
% PlugInAuthorEmail: stm@users.sf.net
% PlugInMenuPath: Tools/external_converter 
% PlugInDescription
Simple plugin to call an external converter
% PlugInUsage
Registers itself. Select source file,a destination folder, a suitable
suffix, the external converter (full path) and press okey. You can
also path some options to the external program
% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */

#include "external_converter.h"
#include <dirent.h>
#include <fnmatch.h>
#define UMASK (S_IRUSR | S_IWUSR | S_IXUSR )

using namespace std;

// Plugin Prototypes
static void external_converter_init (void);
static void external_converter_query (void);
static void external_converter_about (void);
static void external_converter_configure (void);
static void external_converter_cleanup (void);

static void external_convert_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data);


//
// Gxsm plugin discription
//
GxsmPlugin external_converter_pi = {
  NULL,                   // filled in and used by Gxsm, don't touch !
  NULL,                   // filled in and used by Gxsm, don't touch !
  0,                      // filled in and used by Gxsm, don't touch !
  NULL,                   // filled in just after init() is called !!!
  "external_converter",
  NULL,                      
  g_strdup ("Calls an external converter"),
  "Thorsten Wagner",
  "tools-section",
  N_("External Converter"),
  NULL,
  "No further info",
  NULL,
  NULL,
  external_converter_init,
  external_converter_query,
  external_converter_about,
  external_converter_configure,
  NULL,
  external_convert_callback,
  NULL,
  external_converter_cleanup
};

//
//Text used in the About Box
//
static const char *about_text = N_("Tool to call an external converter.");
                                   
	
//
// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
//
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
  external_converter_pi.description = g_strdup_printf(N_("Tool to call an external converter. Version: %s"), VERSION);
  return &external_converter_pi; 
}

//
// Query Function, installs Plugin's in the apropriate menupath
// !!!! make sure the "external_converter_cleanup()" function (see below) !!!!
// !!!! removes the correct menuentries !!!!
//
static void external_converter_query(void)
{
  if(external_converter_pi.status) g_free(external_converter_pi.status); 
  external_converter_pi.status = g_strconcat (
	  N_("Plugin query has attached "),
	  external_converter_pi.name, 
	  N_(": File external_converter is ready to use"),
	  NULL);
}

//
// init-Function
//

static void external_converter_init(void)
{
  PI_DEBUG(DBG_L2,"external_converter Plugin Init");
}

//
// about-Function
//
static void external_converter_about(void)
{
  const gchar *authors[] = { external_converter_pi.authors, NULL};
  gtk_show_about_dialog (NULL, 
			 "program-name",  external_converter_pi.name,
			 "version", VERSION,
			 N_("(C) 2006 the Free Software Foundation"),
			 "comments", about_text,
			 "authors", authors,
			 NULL,NULL,NULL
			 );
}

//
// configure-Function
//
static void external_converter_configure(void)
{
  if(external_converter_pi.app)
    external_converter_pi.app->message("External Converter Plugin Configuration");
}

//
// cleanup-Function, make sure the Menustrings are matching those above!!!
//
static void external_converter_cleanup(void)
{
  PI_DEBUG(DBG_L2,"External Converter Plugin Cleanup");
  gnome_app_remove_menus (GNOME_APP (external_converter_pi.app->getApp()),
			  N_("Tools/External Converter"), 1);
}


//Gtk+ Signal Funktion
static void external_convert_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data)
{
  external_converter_Control *Popup = new external_converter_Control();
  Popup->run();
  PI_DEBUG(DBG_L2,"External Converter dialog opened");
}


////////////////////////////////////////
// ANFANG external_converter
///////////////////////////////////////

gchar *select_mask;

external_converter::external_converter() : m_converted(0)
{
	PI_DEBUG(DBG_L2,"Generating converter object!");
}


void external_converter::concatenate_dirs(gchar* target, const gchar* add)
{
	/** assure that the directories are delimited with / */
	int len = strlen(target);
	if (len > 0 && target[len-1] != '/')
		strcat(target, "/");
	strcat(target, add);
}


void external_converter::create_full_path(gchar* target, const gchar* source_directory,
	const gchar* current_dir, const gchar* file)
{
	if (source_directory)
		strcpy(target, source_directory);
	else
		*target = '\0';

	if (current_dir)
		concatenate_dirs(target, current_dir);

	if (file)
		concatenate_dirs(target, file);
}


void external_converter::replace_suffix(gchar* target, gchar* new_suffix)
{
	int len = strlen(target);

	for (int i = len-1; i > -1; --i) {
		if (target[i] == EXT_SEP) {
			strcpy(target+i+1, new_suffix);
			return;
		}
	}
}



void external_converter::ConvertDir(external_converter_Data* work_it, const gchar* current_dir)
{
	gchar source_dir[MAX_PATH];

	create_full_path(source_dir, work_it->sourceDir, current_dir, 0);

	DIR* dir = opendir(source_dir);
	if (dir) {
		struct stat file_stat;
		dirent* current_file;
		gchar source_name[MAX_PATH];
		gchar target_name[MAX_PATH];

		while ((current_file = readdir(dir))) {
			create_full_path(source_name, work_it->sourceDir, current_dir, 
				current_file->d_name);
			if (lstat(source_name, &file_stat) != -1) {
				// go recursively in case of directories
				if (S_ISDIR(file_stat.st_mode) && *current_file->d_name != '.')
				{
					if (work_it->m_recursive) {
						if (work_it->m_create_subdirs) {
							create_full_path(target_name, work_it->destDir,
								current_dir, current_file->d_name);
						//	if (lstat(target_name, &file_stat) != -1
						//		&& mkdir(target_name, S_IRUSR | S_IWUSR
						//			 | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP)
						//			 == -1)
							if(strcmp(source_name, work_it->destDir))
							if (mkdir(target_name, S_IRUSR | S_IWUSR
									 | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP)
									 == -1)
							{
								std::cout << "Could not create directory "
									<< target_name << std::endl; 
							} else
								std::cout << "Create directory "
									 << target_name << std::endl;
						}
						create_full_path(source_name, 0, current_dir,
							 current_file->d_name);

						ConvertDir(work_it, source_name);
					}
				// otherwise check whether the file matches the mask
				// Note: S_ISREG didn't seem to work correctly on my
				// system - otherwise I would use it as well
				} else if (!fnmatch(work_it->convFilter, current_file->d_name,
					0))
				{
					create_full_path(source_name, work_it->sourceDir,
						current_dir, current_file->d_name);
					create_full_path(target_name, work_it->destDir,
						work_it->m_create_subdirs ? current_dir : 0, current_file->d_name);
					replace_suffix(target_name, work_it->writeFormat);

					if (!work_it->m_overwrite_target
						 && lstat(target_name, &file_stat) != -1)
					{
						std::cout << target_name << " already exists!"
							 << std::endl;
						continue;
					}

					++m_converted;
//					std::cout << std::endl << "Converting file " << m_converted << ": "
//						<< source_name << std::endl;
					std::cout << std::endl << "Converting file " << m_converted << std::endl;
					char command[1000];
					snprintf(command, 1000, "%s %s \"%s\" \"%s\"", work_it->converterPath,work_it->converterOptions,
						 source_name,target_name);
					printf("command: %s\n",command);					
					int errornum=0;
					errornum=system(command);
					if (errornum!=0) printf("Error! %i \n",errornum);
				}
			}
		}	
	}

	closedir(dir);
}




////////////////////////////////////////
// ENDE external_converter
///////////////////////////////////////

////////////////////////////////////////
// ANFANG external_converter_Control
///////////////////////////////////////

gint file_error(GtkWindow *window, const gchar *err_msg, const gchar *dir);

external_converter_Control::external_converter_Control ()
{ 
	frontenddata = new external_converter_Data(getenv("PWD"),getenv("PWD"),"*.nc", "top");
	XsmRescourceManager xrm("FilePathSelectionHistory","external_converter");
	xrm.Get ("SourceFile",&frontenddata->sourceFile, getenv("PWD"));
	xrm.Get ("DestFile", &frontenddata->destFile, getenv("PWD"));
	xrm.Get ("DestSuffix", &frontenddata->writeFormat, ".txt");
	xrm.Get ("ConverterPath", &frontenddata->converterPath, "/usr/local/bin/converter");
	xrm.Get ("ConverterOptions", &frontenddata->converterOptions, "PWD");
}


external_converter_Control::~external_converter_Control ()
{
  delete frontenddata;
}


void external_converter_Control::run()
{
  	GtkWidget *dialog;
  	GtkWidget *VarName;
  	GtkWidget *variable;
  	GtkWidget *help;
  	GtkWidget *hbox;
  	GtkWidget *table;
  
  DlgStart();
  dialog = gnome_dialog_new(N_("External Converter"), N_("Convert"), GNOME_STOCK_BUTTON_CANCEL, NULL); 

   gnome_dialog_set_close(GNOME_DIALOG(dialog), FALSE);
   gnome_dialog_close_hides(GNOME_DIALOG(dialog), FALSE);
   gnome_dialog_set_default(GNOME_DIALOG(dialog), 3);      


        table = gtk_table_new(7,3,FALSE);
	gtk_widget_show (table);

	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dialog)->vbox), table, TRUE, TRUE, GXSM_WIDGET_PAD);

	// Create entry for source file

	VarName = gtk_label_new (N_("Source File/Path"));
	gtk_widget_show (VarName);
	gtk_label_set_justify(GTK_LABEL(VarName), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (VarName), 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (VarName), 5, 0);
	
	SrcFile = variable = gnome_file_entry_new( NULL, "Choose the source file");
	gnome_file_entry_set_default_path( GNOME_FILE_ENTRY ( variable ), frontenddata->sourceFile);
	gnome_file_entry_set_directory_entry(GNOME_FILE_ENTRY (variable), FALSE);
	gnome_file_entry_set_filename (GNOME_FILE_ENTRY (variable), frontenddata->sourceFile);
	gtk_widget_show(variable);

	gtk_widget_set_size_request(variable,400,-1);
	gtk_table_attach_defaults(GTK_TABLE(table),VarName,0,1,0,1);		
	gtk_table_attach_defaults(GTK_TABLE(table),variable,1,3,0,1);		


	// Create entry for destination path


	VarName = gtk_label_new (N_("Destination Path"));
	gtk_widget_show (VarName);
	gtk_label_set_justify(GTK_LABEL(VarName), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (VarName), 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (VarName), 5, 0);

	DestinationPath = variable = gnome_file_entry_new( NULL, "Choose the destination directory");
	gnome_file_entry_set_default_path( GNOME_FILE_ENTRY ( variable ), frontenddata->destFile);
	gnome_file_entry_set_directory_entry(GNOME_FILE_ENTRY (variable), TRUE);
	gnome_file_entry_set_filename (GNOME_FILE_ENTRY (variable), frontenddata->destFile);
	gtk_widget_show(variable);

	gtk_widget_set_size_request(variable,400,-1);
	gtk_table_attach_defaults(GTK_TABLE(table),VarName,0,1,1,2);		
	gtk_table_attach_defaults(GTK_TABLE(table),variable,1,3,1,2);		



  VarName = gtk_label_new (N_("Suffix of Destination"));
  gtk_widget_show (VarName);
  gtk_label_set_justify(GTK_LABEL(VarName), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (VarName), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (VarName), 5, 0);

  DestinationSuffix = variable = gtk_entry_new ();
  gtk_widget_show (variable);
  gtk_entry_set_text (GTK_ENTRY (variable), frontenddata->writeFormat);

  help = gtk_button_new_with_label (N_("Help"));
  gtk_widget_show (help);
  g_signal_connect (G_OBJECT (help), "clicked", G_CALLBACK (show_info_callback),(void*)(N_("Enter the file format suffix without any special characters.")));

	g_signal_connect(G_OBJECT(dialog), "clicked", G_CALLBACK(external_converter_Control::dlg_clicked), this);
	gtk_table_attach_defaults(GTK_TABLE(table),VarName,0,1,2,3);		
	gtk_table_attach(GTK_TABLE(table),variable,1,2,2,3,GTK_FILL,GTK_FILL,0,0);		
	gtk_table_attach(GTK_TABLE(table),help,2,3,2,3,GTK_FILL,GTK_FILL,0,0);		
	gtk_widget_set_size_request(variable,297,-1);


	// Create entry for converter

	VarName = gtk_label_new (N_("Path to Converter"));
	gtk_widget_show (VarName);
	gtk_label_set_justify(GTK_LABEL(VarName), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (VarName), 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (VarName), 5, 0);

	ConverterPath = variable = gnome_file_entry_new( NULL, "Choose the path to the external converter");
	gnome_file_entry_set_default_path( GNOME_FILE_ENTRY ( variable ), frontenddata->destFile);
	gnome_file_entry_set_directory_entry(GNOME_FILE_ENTRY (variable), FALSE);
	gnome_file_entry_set_filename (GNOME_FILE_ENTRY (variable), frontenddata->converterPath);
	gtk_widget_show(variable);
	g_signal_connect(G_OBJECT(dialog), "clicked",G_CALLBACK(external_converter_Control::dlg_clicked),this);



	gtk_table_attach_defaults(GTK_TABLE(table),VarName,0,1,3,4);		
	gtk_table_attach_defaults(GTK_TABLE(table),variable,1,3,3,4);		



	VarName = gtk_label_new (N_("Converter Options"));
	gtk_widget_show (VarName);
	gtk_label_set_justify(GTK_LABEL(VarName), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (VarName), 0.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (VarName), 5, 0);

	ConverterOptions = variable = gtk_entry_new ();
	gtk_widget_show (variable);
	gtk_entry_set_text (GTK_ENTRY (variable), frontenddata->converterOptions);

	help = gtk_button_new_with_label (N_("Help"));
	gtk_widget_show (help);
	g_signal_connect (G_OBJECT (help), "clicked", G_CALLBACK (show_info_callback),(void*)(N_("Enter options for the external converter.")));

	g_signal_connect(G_OBJECT(dialog), "clicked", G_CALLBACK(external_converter_Control::dlg_clicked), this);


	gtk_table_attach_defaults(GTK_TABLE(table),VarName,0,1,4,5);		
	gtk_table_attach(GTK_TABLE(table),variable,1,2,4,5,GTK_FILL,GTK_FILL,0,0);		
	gtk_table_attach(GTK_TABLE(table),help,2,3,4,5,GTK_FILL,GTK_FILL,0,0);		
	gtk_widget_set_size_request(variable,297,-1);


	gtk_table_set_col_spacings(GTK_TABLE(table),6);
	gtk_table_set_row_spacings(GTK_TABLE(table),6);



	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_show(hbox);

	gtk_table_attach(GTK_TABLE(table),hbox,0,3,5,6,GTK_FILL,GTK_FILL,0,0);		

	GtkWidget* toggle_recursive = gtk_check_button_new_with_label("Recursive");
	
	gtk_box_pack_start(GTK_BOX (hbox), toggle_recursive, TRUE, TRUE, 0);
	gtk_widget_show(toggle_recursive);
	GtkWidget* toggle_overwrite_target = gtk_check_button_new_with_label("Overwrite Target File");
	g_signal_connect(G_OBJECT(toggle_recursive), "clicked",
		G_CALLBACK(external_converter_Control::recursive_click),
		this);
  
	gtk_box_pack_start(GTK_BOX (hbox), toggle_overwrite_target, TRUE, TRUE, 0);
	gtk_widget_show(toggle_overwrite_target);
	g_signal_connect(G_OBJECT(toggle_overwrite_target), "clicked",
		G_CALLBACK(external_converter_Control::overwrite_target_click),
		this);
  
	GtkWidget* toggle_create_subdirs = gtk_check_button_new_with_label("Create sub-directories");
	gtk_box_pack_start(GTK_BOX (hbox), toggle_create_subdirs, TRUE, TRUE, 0);
	gtk_widget_show(toggle_create_subdirs);
	g_signal_connect(G_OBJECT(toggle_create_subdirs), "clicked",
		G_CALLBACK(external_converter_Control::create_subdirs_click),
		this);


	gtk_table_set_row_spacing(GTK_TABLE(table),4,12);





	gtk_widget_show(dialog);
}

void external_converter_Control::recursive_click(GtkWidget* widget, gpointer userdata)
{
	((external_converter_Control*) userdata)->frontenddata->m_recursive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}


void external_converter_Control::overwrite_target_click(GtkWidget* widget, gpointer userdata)
{
	((external_converter_Control*) userdata)->frontenddata->m_overwrite_target = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}


void external_converter_Control::create_subdirs_click(GtkWidget* widget, gpointer userdata)
{
	((external_converter_Control*) userdata)->frontenddata->m_create_subdirs = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}




void external_converter_Control::dlg_clicked(GnomeDialog * dialog, gint button_number, external_converter_Control *mic){

	g_free(mic->frontenddata->sourceFile); 
	mic->frontenddata->sourceFile = g_strdup(gnome_file_entry_get_full_path(GNOME_FILE_ENTRY (mic->SrcFile),TRUE));
	if(mic->frontenddata->sourceFile == NULL && !button_number)
		if(file_error(GTK_WINDOW(dialog),"Source file does not exist",NULL)){
			gnome_dialog_close(dialog);
			goto finish;
		}
	g_free(mic->frontenddata->destFile); 
	mic->frontenddata->destFile = g_strdup(gnome_file_entry_get_full_path(GNOME_FILE_ENTRY (mic->DestinationPath),FALSE));
	if(access(mic->frontenddata->destFile,F_OK) && !button_number)
		if(file_error(GTK_WINDOW(dialog),"Destination Folder does not exist",mic->frontenddata->destFile)){
			gnome_dialog_close(dialog); 
			goto finish;
		}

	g_free(mic->frontenddata->writeFormat);
	mic->frontenddata->writeFormat = g_strdup(gtk_entry_get_text (GTK_ENTRY (mic->DestinationSuffix)));	

	g_free(mic->frontenddata->converterPath); 
	mic->frontenddata->converterPath = g_strdup(gnome_file_entry_get_full_path(GNOME_FILE_ENTRY (mic->ConverterPath),FALSE));
	if(access(mic->frontenddata->converterPath,F_OK) && !button_number)
		if(file_error(GTK_WINDOW(dialog),"Converter does not exist",NULL)){
			gnome_dialog_close(dialog); 
			goto finish;
		}
	
	g_free(mic->frontenddata->converterOptions);
	mic->frontenddata->converterOptions = g_strdup(gtk_entry_get_text (GTK_ENTRY (mic->ConverterOptions)));	

  switch(button_number){
  case 0:	{gnome_dialog_close(dialog);
		//show_info_callback(NULL, N_("Converting ..."));
	struct stat file_stat;
	int len=0;
	gchar source_name[MAX_PATH];

	lstat(mic->frontenddata->sourceFile, &file_stat);


	if (!S_ISDIR(file_stat.st_mode))
		{
		//file
		char command[1000];
		len = strlen(mic->frontenddata->sourceFile);
		gchar* fname = g_strndup(mic->frontenddata->sourceFile,1000);
		gchar* buffer;
		while(buffer=strstr(fname,"/")){
			fname = g_strndup(buffer+1,1000);
			}
		buffer = g_strndup(fname,strlen(fname)-strlen(strrchr(fname, '.')));
		//printf("Buffer %s fname %s\n",buffer,fname);				
		len = strlen(mic->frontenddata->destFile);
		// make sure that the directory is with terminating '/' 
		if (len > 0) {
			if(mic->frontenddata->destFile[len-1] != '/') strcat(mic->frontenddata->destFile, "/");
			}
		
		printf("Starting conversion\n");		
		//check if input file is a file or directory
		snprintf(command, 1000, "%s %s \"%s\" \"%s%s.%s\" &", mic->frontenddata->converterPath, mic->frontenddata->converterOptions,
				 mic->frontenddata->sourceFile,mic->frontenddata->destFile, buffer, mic->frontenddata->writeFormat);
		printf("Command:%s\n",command);
		system(command);
		}
	else 
		{
		//directory
		mic->frontenddata->sourceDir=mic->frontenddata->sourceFile;
		mic->frontenddata->destDir=mic->frontenddata->destFile;
		external_converter *converter_obj;
  		converter_obj = new external_converter();

	        converter_obj->ConvertDir(mic->frontenddata, 0);
		}
   break;}

   case 1:	{gnome_dialog_close(dialog); 
				break;}
	}
finish:
	XsmRescourceManager xrm("FilePathSelectionHistory","external_converter");
	xrm.Put ("SourceFile", mic->frontenddata->sourceFile);
	xrm.Put ("DestFile", mic->frontenddata->destFile);
	xrm.Put ("DestSuffix", mic->frontenddata->writeFormat);
	xrm.Put ("ConverterPath", mic->frontenddata->converterPath);
	xrm.Put ("ConverterOptions", mic->frontenddata->converterOptions);

mic->DlgDone();

}

////////////////////////////////////////
//ANFANG converterData
///////////////////////////////////////

external_converter_Data :: external_converter_Data(const gchar *src, const gchar *dst, const gchar *conv, const gchar *write) : m_recursive(false), m_overwrite_target(false), m_create_subdirs(false)
{
	sourceDir   =  g_strdup(src);
	destDir     =  g_strdup(dst); 
	sourceFile   =  g_strdup(src);
	destFile     =  g_strdup(dst); 
        convFilter  =  g_strdup(conv);
        writeFormat =  g_strdup(write);
}
	
external_converter_Data :: ~external_converter_Data() 
{
		g_free(sourceDir);
		g_free(destDir);
		g_free(convFilter);
		g_free(writeFormat);
}

////////////////////////////////////////
// ENDE converterData
////////////////////////////////////////


