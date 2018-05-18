/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Plugin Name: inet_json_external_scandata.C
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
 * All "% PlugInXXX" commentary tags are mandatory
 * All "% OptPlugInXXX" tags are optional
 * --------------------------------------------------------------------------------
% BeginPlugInDocuSection
% PlugInDocuCaption: Inet JSON Scan Data Control
% PlugInName: inet_json_external_scandata
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: windows-section Inet JSON Scan External Data
RP data streaming

% PlugInDescription

% PlugInUsage

% OptPlugInRefs

% OptPlugInNotes

% OptPlugInHints

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */


#include <gtk/gtk.h>
#include <gio/gio.h>

#include "config.h"
#include "gxsm/plugin.h"

#include "gxsm/unit.h"
#include "gxsm/pcs.h"
#include "gxsm/xsmtypes.h"
#include "gxsm/glbvars.h"
#include "gxsm/action_id.h"

#include "plug-ins/control/inet_json_external_scandata.h"

// Plugin Prototypes - default PlugIn functions
static void inet_json_external_scandata_init (void); // PlugIn init
static void inet_json_external_scandata_query (void); // PlugIn "self-install"
static void inet_json_external_scandata_about (void); // About
static void inet_json_external_scandata_configure (void); // Configure plugIn, called via PlugIn-Configurator
static void inet_json_external_scandata_cleanup (void); // called on PlugIn unload, should cleanup PlugIn rescources

// other PlugIn Functions and Callbacks (connected to Buttons, Toolbar, Menu)
static void inet_json_external_scandata_show_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data);
static void inet_json_external_scandata_SaveValues_callback ( gpointer );

// Fill in the GxsmPlugin Description here -- see also: Gxsm/src/plugin.h
GxsmPlugin inet_json_external_scandata_pi = {
	NULL,                   // filled in and used by Gxsm, don't touch !
	NULL,                   // filled in and used by Gxsm, don't touch !
	0,                      // filled in and used by Gxsm, don't touch !
	NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
	"Inet_Json_External_Scandata",
	NULL,
	NULL,
	"Percy Zahl",
	"windows-section", // Menu-path/section
	N_("Inet JSON RP"), // Menu Entry -- overridden my set-window-geometry() call automatism
	N_("Open Inet JSON External Scan Data Control Window"),
	"Inet JSON External Scan Data Control Window", // help text
	NULL,          // error msg, plugin may put error status msg here later
	NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
	inet_json_external_scandata_init,  
	inet_json_external_scandata_query,  
	// about-function, can be "NULL"
	// can be called by "Plugin Details"
	inet_json_external_scandata_about,
	// configure-function, can be "NULL"
	// can be called by "Plugin Details"
	inet_json_external_scandata_configure,
	// run-function, can be "NULL", if non-Zero and no query defined, 
	// it is called on menupath->"plugin"
	NULL,
	// cleanup-function, can be "NULL"
	// called if present at plugin removal
	inet_json_external_scandata_show_callback, // direct menu entry callback1 or NULL
	NULL, // direct menu entry callback2 or NULL

	inet_json_external_scandata_cleanup
};

// Text used in Aboutbox, please update!!
static const char *about_text = N_("Inet JSON External Scan Data Control Plugin\n\n"
                                   "This plugin manages externa Scan Data Sources.\n"
	);

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
	inet_json_external_scandata_pi.description = g_strdup_printf(N_("Gxsm inet_json_external_scandata plugin %s"), VERSION);
	return &inet_json_external_scandata_pi; 
}

// data passed to "idle" function call, used to refresh/draw while waiting for data
typedef struct {
	GSList *scan_list; // scans to update
	GFunc  UpdateFunc; // function to call for background updating
	gpointer data; // additional data (here: reference to the current Inet_Json_External_Scandata object)
} IdleRefreshFuncData;

Inet_Json_External_Scandata *inet_json_external_scandata = NULL;

// Query Function, installs Plugin's in File/Import and Export Menupaths!

#define REMOTE_PREFIX "INET_JSON_EX_"

static void inet_json_external_scandata_query(void)
{
	if(inet_json_external_scandata_pi.status) g_free(inet_json_external_scandata_pi.status); 
	inet_json_external_scandata_pi.status = g_strconcat (
                                                 N_("Plugin query has attached "),
                                                 inet_json_external_scandata_pi.name, 
                                                 N_(": File IO Filters are ready to use"),
                                                 NULL);

	PI_DEBUG (DBG_L2, "inet_json_external_scandata_query:new" );
	inet_json_external_scandata = new Inet_Json_External_Scandata;

	PI_DEBUG (DBG_L2, "inet_json_external_scandata_query:res" );
	
	inet_json_external_scandata_pi.app->ConnectPluginToCDFSaveEvent (inet_json_external_scandata_SaveValues_callback);
}

static void inet_json_external_scandata_SaveValues_callback ( gpointer gp_ncf ){

	//NcFile *ncf = (NcFile *) gp_ncf;
	//NcDim* spmscd  = ncf->add_dim("inet_json_external_scandata_dim", strlen(tmp));
	//NcVar* spmsc   = ncf->add_var("inet_json_external_scandata", ncChar, spmscd);
	//spmsc->add_att("long_name", "inet_json_external_scandata: scan direction");
	//spmsc->put(tmp, strlen(tmp));
	//g_free (tmp);
}


// 5.) Start here with the plugins code, vars def., etc.... here.
// ----------------------------------------------------------------------
//


// init-Function
static void inet_json_external_scandata_init(void)
{
  PI_DEBUG (DBG_L2, "inet_json_external_scandata Plugin Init" );
}

// about-Function
static void inet_json_external_scandata_about(void)
{
        const gchar *authors[] = { inet_json_external_scandata_pi.authors, NULL};
        gtk_show_about_dialog (NULL,
                               "program-name",  inet_json_external_scandata_pi.name,
                               "version", VERSION,
                               "license", GTK_LICENSE_GPL_3_0,
                               "comments", about_text,
                               "authors", authors,
                               NULL
                               );
}

// configure-Function
static void inet_json_external_scandata_configure(void)
{
	if(inet_json_external_scandata_pi.app)
		inet_json_external_scandata_pi.app->message("inet_json_external_scandata Plugin Configuration");
}

// cleanup-Function, make sure the Menustrings are matching those above!!!
static void inet_json_external_scandata_cleanup(void)
{
	// delete ...
	if( inet_json_external_scandata )
		delete inet_json_external_scandata ;

	if(inet_json_external_scandata_pi.status) g_free(inet_json_external_scandata_pi.status); 
}

static void inet_json_external_scandata_show_callback(GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	PI_DEBUG (DBG_L2, "inet_json_external_scandata Plugin : show" );
	if( inet_json_external_scandata )
		inet_json_external_scandata->show();
}


Inet_Json_External_Scandata::Inet_Json_External_Scandata ()
{
        GtkWidget *tmp;
	
	GSList *EC_list=NULL;
	GSList **RemoteEntryList = new GSList *;
	*RemoteEntryList = NULL;

        input_rpaddress = NULL;
        text_status = NULL;

        /* create a new connection, init */

        error = NULL;
        connection = NULL;
        client = g_socket_client_new();

        
	PI_DEBUG (DBG_L2, "inet_json_external_scandata Plugin : building interface" );

	Unity    = new UnitObj(" "," ");

        // Window Title
	AppWindowInit("Inet JSON External Scan Data Control for RP");

        bp = new BuildParam (v_grid);
        bp->set_no_spin (true);
        //bp->set_default_ec_change_notice_fkt (VObject::ec_properties_changed, this);

        bp->new_grid_with_frame ("Inet Setup");
        input_rpaddress = bp->grid_add_input ("RedPitaya Address");
        //gtk_entry_set_text (GTK_ENTRY (input_rpaddress), "http://rp-f05603.local/pacpll/?type=run");
        gtk_entry_set_text (GTK_ENTRY (input_rpaddress), "130.199.243.200");

        tmp =bp->grid_add_check_button ( N_("Connect"), "Check to initiate connection, uncheck to close connection.", 1,
                                         G_CALLBACK (Inet_Json_External_Scandata::connect_cb), this);
        
        bp->new_line ();
        tmp=bp->grid_add_button ( N_("Read"), "TEST READ", 1,
                                  G_CALLBACK (Inet_Json_External_Scandata::read_cb), this);
        tmp=bp->grid_add_button ( N_("Write"), "TEST WRITE", 1,
                                  G_CALLBACK (Inet_Json_External_Scandata::write_cb), this);

        bp->new_line ();
        text_status = gtk_text_view_new ();
 	gtk_text_view_set_editable (GTK_TEXT_VIEW (text_status), FALSE);
        //gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_status), GTK_WRAP_WORD_CHAR);

        bp->grid_add_widget (text_status, 3);
        
        bp->pop_grid ();
        bp->new_line ();

        bp->show_all ();
 
        // save List away...
	//g_object_set_data( G_OBJECT (window), "INETJSONSCANCONTROL_EC_list", EC_list);

        set_window_geometry ("inet-json-rp-control"); // needs rescoure entry and defines window menu entry as geometry is managed
}

Inet_Json_External_Scandata::~Inet_Json_External_Scandata (){
       	delete Unity;
}

void Inet_Json_External_Scandata::update(){
	if (G_IS_OBJECT (window))
		g_slist_foreach((GSList*)g_object_get_data( G_OBJECT (window), "INETJSONSCANCONTROL_EC_list"),
				(GFunc) App::update_ec, NULL);
}

void Inet_Json_External_Scandata::connect_cb (GtkWidget *widget, Inet_Json_External_Scandata *self){
        if (!self->text_status) return;
        if (!self->input_rpaddress) return;

        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))){

                /* connect to the host */
                self->connection = g_socket_client_connect_to_host (self->client,
                                                                    gtk_entry_get_text (GTK_ENTRY (self->input_rpaddress)), //(gchar*)"localhost",
                                                                    80, /* port HTTP */
                                                                    NULL,
                                                                    &self->error);

                if (self->error != NULL)
                        {
                                g_warning (self->error->message);
                                self->status_append ("ERROR:\n");
                                self->status_append (self->error->message);
                                self->status_append ("\n");
                        }
                else
                        {
                                g_message ("Connection to");
                                g_message (gtk_entry_get_text (GTK_ENTRY (self->input_rpaddress)));
                                g_message ("established.\n");
                                self->status_append ("Connection to ");
                                self->status_append (gtk_entry_get_text (GTK_ENTRY (self->input_rpaddress)));
                                self->status_append (" established.\n");
                        }
        } else {
                /* close connection */
                //self->connection = g_socket_client_connect_finish (self->connection,
                //                                                  GAsyncResult *result,
                //                                                   &self->error);
                if (g_io_stream_close ( G_IO_STREAM (self->connection), //GIOStream *stream,
                                       NULL,
                                        &self->error)){
                        self->status_append ("Connection closed.\n");
                } else {
                        self->status_append ("ERROR: Closing connection failed.\n");
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), true);
                }

                if (self->error != NULL) {
                        g_warning (self->error->message);
                        self->status_append ("ERROR:\n");
                        self->status_append (self->error->message);
                        self->status_append ("\n");
                } else {
                        g_message ("Connection closed.");
                }
        }
}

void Inet_Json_External_Scandata::read_cb (GtkWidget *widget, Inet_Json_External_Scandata *self){
        GInputStream * istream = g_io_stream_get_input_stream (G_IO_STREAM (self->connection));
        gchar buffer[4096]; for (int i=0; i<4096; ++i) buffer[i]=0;
        gssize num = g_input_stream_read (istream,
                                          (void *)buffer,
                                          100,
                                          NULL,
                                          &self->error);   
        if (self->error != NULL) {
                g_warning (self->error->message);
                self->status_append (self->error->message);
        } else {
                self->status_append (buffer);  
        }
}

void Inet_Json_External_Scandata::write_cb (GtkWidget *widget, Inet_Json_External_Scandata *self){
        GOutputStream * ostream = g_io_stream_get_output_stream (G_IO_STREAM (self->connection));
        const gchar *buffer="Hello RedPitaya!";
        g_output_stream_write  (ostream,
                                (void*)buffer, /* your message goes here */
                                strlen (buffer), /* length of your message */
                                NULL,
                                &self->error);
        if (self->error != NULL) {
                g_warning (self->error->message);
                self->status_append (self->error->message);
        }else {
                self->status_append ("Write OK: '");  
                self->status_append (buffer);  
                self->status_append ("'\n");  
        }
}


void Inet_Json_External_Scandata::status_append (const gchar *msg){

	GtkTextBuffer *console_buf;
	GtkTextIter start_iter, end_iter;
	GtkTextView *textview;
	GString *output;
	GtkTextMark *end_mark;

	if (!msg) {
		g_warning("No message to append");
		return;
	}

	// read string which contain last command output
	console_buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text_status));
	gtk_text_buffer_get_bounds (console_buf, &start_iter, &end_iter);

	// get output widget content
	output = g_string_new (gtk_text_buffer_get_text (console_buf,
                                                         &start_iter, &end_iter,
                                                         FALSE));

	// append input line
	output = g_string_append (output, msg);
	gtk_text_buffer_set_text (console_buf, output->str, -1);
	g_string_free (output, TRUE);

	// scroll to end
	gtk_text_buffer_get_end_iter (console_buf, &end_iter);
	end_mark = gtk_text_buffer_create_mark (console_buf, "cursor", &end_iter,
                                                FALSE);
	g_object_ref (end_mark);
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (text_status),
                                      end_mark, 0.0, FALSE, 0.0, 0.0);
	g_object_unref (end_mark);
}


// Menu Call Back Fkte
#if 0
/* stolen from app_remote.C */
static void via_remote_list_Check_ec(Gtk_EntryControl* ec, remote_args* ra){
	ec->CheckRemoteCmd (ra);
};
#endif
