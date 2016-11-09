/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Hardware Interface Plugin Name: LAN_rhk_hwi.C
 * ==================================================
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
% PlugInDocuCaption: Local Area Network (Internet) RHK Controller Hardware Interface (to be ported)
% PlugInName: LAN_rhk_hwi
% PlugInAuthor: Farid El Gabaly, Juan de la Figuera
% PlugInAuthorEmail: johnnybegood@users.sourceforge.net
% PlugInMenuPath: Hardware/LAN_RHK:SPM-HwI

% PlugInDescription
This plugin/class provides a interface for SPM adquisition through a
Local Area Network using sockets. In particular, this plugin provides 
the necessary interface to interact with the standalone \GxsmFile{rhk\_controller} program and
thus use an \GxsmEmph{RHK STM-100} (the old stand-alone models before 
rev 7, not the new DSP based systems) with Gxsm.

It can be used as the foundation of a general SPM-through-the-internet plugin.

With this one, you can:
\begin{itemize}
\item read the settings of the RHK electronics (image size, offset, 
tunneling conditions, pixels in image)
\item adquire images with the proper settings (either topographic, current 
or any other channel).
\item change from XY to YX scanning
\end{itemize}

The current limitations are:
\begin{itemize}
\item Only forward image (it is not difficult to implement saving the backward
image)
\item Only a single channel can be adquiered. Again it is not too complex to change this.
\item No spectroscopy. Actually we have the code for doing IV curves, but we do not expect
we will implement it in the \GxsmFile{rhk\_controller} program anytime soon.
\end{itemize}

The ugliest code is due to Gxsm assuming that all settings (bias, scan size, etc) are set by Gxsm, not 
by the electronics itself (as is the case for the RHK electronics, which has its own 
scanning hardware). We have hacked the PutParameter call to also receive the data, and
this is why a non standard scanning plugin is needed (rhk\_scancontrol).

The \GxsmFile{rhk\_controller} program (available from the Gsxm CVS, module RHK\_controller) 
handles all the low
level details of data aquisition with an \GxsmEmph{RHK STM-100} \GxsmWWW{www.rhk-tech.com} 
with a simple DAQ card \GxsmEmph{I/O Tech Daqboard 2000}
\GxsmWWW{www.IOtech.com}. A couple of cable adapters are needed to connect
the electronics and the adquisition card: one was the \GxsmEmph{I/O
Tech DBK202}, to separate the analogic and digital inputs and outputs
of the andquisition card; the other was a home made one to adapt the
DBK202 to the RHK unit (if anyone needs the pcb drawing, please contact Farid).
The \GxsmFile{rhk\_controller} is run with simple ASCII commands to read the RHK 
settings, and select the adquisition channel, adquiere images and so on.
Look at the program itself for more information (it is a fairly simple
C program which uses lex/yacc to parse the commands), or connect to it with 
telnet localhost 5027 and type "help".
We use a Omicron Coarse approach controller which is interfaced by TTL with 
the DAQ card. The DSPMover plugin works with it.

To manage the \GxsmEmph{I/O Tech Daqboard 2000} under linux we use the
I/O tech linux driver. The Daqboard2000.tgz file from IOTech has:

\begin{itemize}
 \item the module with the device driver, \GxsmFile{db2k}
 \item the lib to acces the device driver from a user site, \GxsmFile{libdaqx}
 \item some examples
\end{itemize}

The version used for the \GxsmEmph{I/O Tech Daqboard 2000} driver is 0.1 and
is GPL licenced. This library is not thread-safe and calls way too many
''printk'' to print debug messages. A big
problem is that it tries to reserve the buffer memory before each adquisition.
We have a patch (in the src RHK\_controller) which eliminates the printk and
reserves the memory buffer only at installation (2Mb).

% PlugInUsage
Set the \GxsmPref{Hardware}{Card} to ''LAN\_RHK:SPM'', and the 
\GxsmPref{Device}{Hardware} to
''localhost:5027' if you are running \GxsmFile{rhk\_controller} in the same computer. 
Start the \GxsmFile{rhk\_controller} \GxsmEmph{before} the Gxsm program. The output of the
\GxsmFile{rhk\_controller} program should say ''someone connected'' when starting Gxsm.

%% OptPlugInSources

%% OptPlugInDest

% OptPlugInNote

You can see some data taken with it in \GxsmWebLink{hobbes.fmc.uam.es/loma}.

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */

#include <sys/ioctl.h>

#include "config.h"
#include "gxsm/plugin.h"
#include "gxsm/xsmhard.h"

// Define HwI PlugIn reference name here, this is what is listed later within "Preferenced Dialog"
// i.e. the string selected for "Hardware/Card"!
#define THIS_HWI_PLUGIN_NAME "LAN_RHK:SPM"

// Plugin Prototypes
static void LAN_rhk_hwi_init( void );
static void LAN_rhk_hwi_about( void );
static void LAN_rhk_hwi_configure( void );
static void LAN_rhk_hwi_cleanup( void );

// Fill in the GxsmPlugin Description here
GxsmPlugin LAN_rhk_hwi_pi = {
  NULL,                   // filled in and used by Gxsm, don't touch !
  NULL,                   // filled in and used by Gxsm, don't touch !
  0,                      // filled in and used by Gxsm, don't touch !
  NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
                          // filled in here by Gxsm on Plugin load, 
                          // just after init() is called !!!
  // ----------------------------------------------------------------------
  // Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
  "LAN_rhk_hwi-"
  "HW-INT-1S-SHORT",
  // Plugin's Category - used to autodecide on Pluginloading or ignoring
  // In this case of Hardware-Interface-Plugin here is the interface-name required
  // this is the string selected for "Hardware/Card"!
  THIS_HWI_PLUGIN_NAME,
  // Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
  "LAN_RHK hardware interface.",
  // Author(s)
  "Percy Zahl",
  // Menupath to position where it is appendet to -- not used by HwI PIs
  N_("Hardware/"),
  // Menuentry -- not used by HwI PIs
  N_(THIS_HWI_PLUGIN_NAME"-HwI"),
  // help text shown on menu
  N_("This is the "THIS_HWI_PLUGIN_NAME" - GXSM Hardware Interface"),
  // more info...
  "N/A",
  NULL,          // error msg, plugin may put error status msg here later
  NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
  // init-function pointer, can be "NULL", 
  // called if present at plugin load
  LAN_rhk_hwi_init,  
  // query-function pointer, can be "NULL", 
  // called if present after plugin init to let plugin manage it install itself
  NULL, // query should be "NULL" for Gxsm-Math-Plugin !!!
  // about-function, can be "NULL"
  // can be called by "Plugin Details"
  LAN_rhk_hwi_about,
  // configure-function, can be "NULL"
  // can be called by "Plugin Details"
  LAN_rhk_hwi_configure,
  // run-function, can be "NULL", if non-Zero and no query defined, 
  // it is called on menupath->"plugin"
  NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
  // cleanup-function, can be "NULL"
  // called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

  LAN_rhk_hwi_cleanup
};


// Text used in Aboutbox, please update!!
static const char *about_text = N_("GXSM LAN_rhk_hwi Plugin\n\n"
                                   "LAN-RHK Hardware Interface for SPM.");

/* Here we go... */

#include "LAN_rhk_hwi.h"

/*
 * PI global
 */

LAN_rhk_hwi_dev *LAN_rhk_hwi_hardware = NULL;

/* 
 * PI essential members
 */

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
  LAN_rhk_hwi_pi.description = g_strdup_printf(N_("GXSM HwI LAN_rhk_hwi plugin %s"), VERSION);
  return &LAN_rhk_hwi_pi; 
}

// Symbol "get_gxsm_hwi_hardware_class" is resolved by dlsym from Gxsm for all HwI type PIs, 
// Essential Plugin Function!!
XSM_Hardware *get_gxsm_hwi_hardware_class ( void *data ) {
	return LAN_rhk_hwi_hardware;
}

// init-Function
static void LAN_rhk_hwi_init(void)
{
	PI_DEBUG (DBG_L2, "LAN_rhk_hwi Plugin Init");
	LAN_rhk_hwi_hardware = new LAN_rhk_hwi_spm ();
 }

// about-Function
static void LAN_rhk_hwi_about(void)
{
	const gchar *authors[] = { LAN_rhk_hwi_pi.authors, NULL};
	gtk_show_about_dialog (NULL, "program-name",  LAN_rhk_hwi_pi.name,
					"version", VERSION,
					  "license", GTK_LICENSE_GPL_3_0,
					  "comments", about_text,
					  "authors", authors,
					  NULL
				));
}

// configure-Function
static void LAN_rhk_hwi_configure(void)
{
	if(LAN_rhk_hwi_pi.app)
		LAN_rhk_hwi_pi.app->message("LAN_rhk_hwi Plugin Configuration");
}

// cleanup-Function
static void LAN_rhk_hwi_cleanup(void)
{
	PI_DEBUG (DBG_L2, "LAN_rhk_hwi Plugin Cleanup");
	delete LAN_rhk_hwi_hardware;
	LAN_rhk_hwi_hardware = NULL;
}

