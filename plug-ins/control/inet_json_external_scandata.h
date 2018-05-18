/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Plugin Name: spm_scancontrol.h
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

#ifndef __INET_JSON_SCANDATA_H
#define __INET_JSON_SCANDATA_H

#include <config.h>


// Scan Control Class based on AppBase
// -> AppBase provides a GtkWindow and some window handling basics used by Gxsm
class Inet_Json_External_Scandata : public AppBase{
public:

        Inet_Json_External_Scandata(); // create window and setup it contents, connect buttons, register cb's...
	virtual ~Inet_Json_External_Scandata(); // unregister cb's
	
	void update(); // window update (inputs, etc. -- here currently not really necessary)

	GtkWidget *remote_param;
        static void connect_cb (GtkWidget *widget, Inet_Json_External_Scandata *self);

        static void read_cb (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void write_cb (GtkWidget *widget, Inet_Json_External_Scandata *self);

        void status_append (const gchar *msg);
      
private:
        BuildParam *bp;

        GtkWidget *input_rpaddress;
        GtkWidget *text_status;

        UnitObj *Unity; // Unit "1"
	
	GSList*   SPMC_RemoteEntryList;


        /* Socket Connection */
        GSocketConnection *connection;
        GSocketClient *client;
        GError *error;

public:
};

#endif
