/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Plugin Name: pyremote.C
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
% PlugInDocuCaption: Python remote control
% PlugInName: pyremote
% PlugInAuthor: Stefan Schr\"oder
% PlugInAuthorEmail: stefan_fkp@users.sf.net
% PlugInMenuPath: Tools/Pyremote

% PlugInDescription
 This plugin is an interface to an embedded Python
 Interpreter.

% PlugInUsage
 Choose Pyremote from the \GxsmMenu{Tools} menu to execute your
 command script. In the appendix you will find a tutorial with
 examples and tips and tricks.

% OptPlugInSection: Reference

The following script shows you all commands that the emb-module supports:

\begin{alltt}
import emb
print dir(emb)
\end{alltt}

The result will look like this:

\begin{alltt}
['__doc__', '__name__', 'autodisplay', 'chmodea', 
'chmodem', 'chmoden', 'chmodeno', 'chmodex', 
'chview1d', 'chview2d', 'chview3d', 'da0', 'direct',
'echo', 'gnuexport', 'gnuimport', 'load', 'log', 
'logev', 'menupath', 'quick', 'save', 'saveas', 
'scaninit', 'scanline', 'scanupdate', 'scanylookup', 
'set', 'sleep', 'startscan', 'stopscan', 'unitbz', 
'unitev', 'units', 'unitvolt', 'waitscan', 'createscan']
\end{alltt}


The following list shows a brief explanation of the commands, together with
the signature (that is the type of arguments).

'()' equals no argument. E.g. \verb+startscan()+

'(N)' equals one Integer arument. E.g. \verb+chview1d(2)+

'(X)' equals one Float argument. No example.

'(S)' equals a string. Often numbers are evaluated as strings first. Like
in \verb+set("RangeX", "234.12")+

'(S,N)' equals two parameters. E.g. \verb+gnuexport("myfilename.nc", 1)+

\begin{tabular}{ll} \hline
Scan operation\\
\texttt{startscan() }   &       Start a scan.\\
\texttt{stopscan() }    &       Stop scanning.\\
\texttt{waitscan}       &       is commented out in app\_remote.C\\
\texttt{initscan() }    &       only initialize.\\
\texttt{scanupdate() }  &       Set hardware parameters on DSP.\\
\texttt{setylookup(N,X)}&       ?\\
\texttt{scanline}       &       Not implemented.\\ \hline
File operation\\
\texttt{save() }  &       Save all.\\
\texttt{saveas(S,N) }    &       Save channel N with filename S.\\
\texttt{load(S,N)   }    &       Load file S to channel N.\\
\texttt{gnuimport(S,N) } &       Import file S to channel N.\\
\texttt{gnuexport(S,N) } &       Export channel N to file S.\\ \hline
Channel operation\\
\texttt{chmodea(N)}      &       Set channel(N) as active.\\
\texttt{chmodex(N)}      &       Set channel(N) to X.\\
\texttt{chmodem(N)}      &       Set channel(N) to Math.\\
\texttt{chmoden(N),N}    &       Set channel(N) to mode(N).\\
\texttt{chmodeno(N)}     &       Set channel(N) to mode 'No'.\\
\texttt{chview1d(N)}     &       View channel(N) in 1d-mode.\\
\texttt{chview2d(N)}     &       View channel(N) in 2d-mode.\\
\texttt{chview3d(N)}     &       View channel(N) in 3d-mode.\\ \hline
Views\\
\texttt{autodisplay()}  &       Autodisplay.\\
\texttt{quick()}        &       Set active display to quickview.\\
\texttt{direct()}       &       Set active display to directview.\\
\texttt{log()}          &       Set active display to logview.\\ \hline
Units\\
\texttt{unitbz() }      &       Set units to BZ.\\
\texttt{unitvolt()}     &       Set units to Volt.\\
\texttt{unitev()}       &       Set units to eV.\\
\texttt{units()}        &       Set units to S.\\ \hline
Others\\
\texttt{createscan(N,N,N,N,A) }  &  Create scan from array.\\
\texttt{list() }  &  Get list of known parameters for get/set.\\
\texttt{set(S,S)}       &  Set parameter to value.\\
\texttt{get(S)}         &  Get parameter, returns floating point value in current user unit .\\
\texttt{gets(S)}         &  Get parameter, returns string with user unit.\\
\texttt{rtquery(S)}     &  Ask current HwI to run RTQuery with parameter S, return vector of three values depening on query.\\
\texttt{y\_current()}    &  Ask current HwI to run RTQuery what shall return the actual scanline of a scan in progress, undefined return otherwise.\\
\texttt{echo(S)  }  &       Print S to console.\\
\texttt{logev(S) }  &       Print S to logfile.\\
\texttt{sleep(N) }  &       Sleep N/10 seconds.\\
\texttt{da0(X)   }  &       Set Analog Output channel 0 to X Volt. (not implemented).\\
\texttt{menupath(S)}  &       Activate Menuentry 'S'\\
\end{tabular}

% OptPlugInSubSection: The set-command

The set command can modify the following parameters:

\begin{tabular}{ll}
\texttt{ACAmp} & \texttt{ACFrq} \\
\texttt{ACPhase} & \texttt{} \\
\texttt{CPShigh} & \texttt{CPSlow} \\
\texttt{Counter} & \texttt{Energy} \\
\texttt{Gatetime} & \texttt{Layers} \\
\texttt{LengthX} & \texttt{LengthY} \\
\texttt{Offset00X} & \texttt{Offset00Y} \\
\texttt{OffsetX} & \texttt{OffsetY} \\
\texttt{PointsX} & \texttt{PointsY} \\
\texttt{RangeX} & \texttt{RangeY} \\
\texttt{Rotation} & \texttt{} \\
\texttt{StepsX} & \texttt{StepsY} \\
\texttt{SubSmp} & \texttt{VOffsetZ} \\
\texttt{VRangeZ} & \texttt{ValueEnd} \\
\texttt{ValueStart} & \texttt{nAvg} \\
\end{tabular}

These parameters are case-sensitive.
To help the python remote programmer to figure out the correct
set-names of all remote enabled entry fields a nifty option
was added to the Help menu to show tooltips with the correct "remote set name"
if the mouse is hovering over the entry.

% OptPlugInSubSection: The get-command

The \texttt{get()} command can retrieve the value of the remote control parameters.
While \texttt{get()} retrieves the internal value as a floating points number, 
\texttt{gets()} reads the actual string from the text entry including units. 
The list of remote control accessible parameters can be retrieved with \texttt{list()}.

\begin{alltt}
import emb
print "OffsetX = ", emb.get("OffsetX")
emb.set("OffsetX", "12.0")
print "Now OffsetX = ", emb.get("OffsetX")

for i in emb.list():
    print i, " ", emb.get(i), " as string: ", emb.gets(i)
\end{alltt}

On my machine (without hardware attached) this prints:

\begin{alltt}
OffsetX =  0.0
Now OffsetX =  12.0
Counter   0.0  as string:  00000    
VOffsetZ   0.0  as string:  0 nm  
VRangeZ   500.0  as string:  500 nm  
Rotation   1.92285320764e-304  as string:  1.92285e-304 °  
TimeSelect   0.0  as string:  0    
Time   1.0  as string:  1    
LayerSelect   0.0  as string:  0    
Layers   1.0  as string:  1    
OffsetY   0.0  as string:  0.0 nm  
OffsetX   12.0  as string:  12.0 nm  
PointsY   1000.0  as string:  1000    
PointsX   1000.0  as string:  1000    
StepsY   0.519863986969  as string:  0.52 nm  
StepsX   0.519863986969  as string:  0.52 nm  
RangeY   64.9830993652  as string:  65.0 nm  
RangeX   64.9830993652  as string:  65.0 nm  
\end{alltt}

All entry fields with assigned id can now be queried.


% OptPlugInSubSection: Creating new scans

Pyremote can create new images from scratch using the 
\verb+createscan+ command. Its arguments are
pixels in x-direction, pixels in y-direction,
range in x-direction (in Angstrom), 
range in y-direction (in Angstrom) and finally
a flat, numeric array that must contain 
as many numbers as needed to fill the matrix.

This example creates a new scan employing sine to
show some pretty landscape.

\begin{alltt}
import array   # for array
import Numeric # for fromfunction
import math    # for sin
import emb     # gxsm embedded module

def dist(x,y):
   return ((Numeric.sin((x-50)/15.0) + Numeric.sin((y-50)/15.0))*100)
   
m = Numeric.fromfunction(dist, (100,100))
n = Numeric.ravel(m) # make 1-d
p = n.tolist()       # convert to list  

examplearray = array.array('l', p) # 
emb.createscan(100, 100, 10000, 10000, examplearray)
\end{alltt}


\GxsmScreenShot{GxsmPI_pyremote01}{An autogenerated image.}

This command can be easily extended to create an importer for arbitrary
file formats via python. The scripts directory contains an elaborate 
example how to use this facility to import the file format employed 
by Nanonis.


% OptPlugInSubSection: Menupath and Plugins

Any plugin, that has a menuentry can be
executed via the
\GxsmTT{menupath}-command. Several of them, however, open a dialog and ask
for a specific parameter, e.g. the diff-PI in \GxsmMenu{Math/Filter1D}.  
This can become annoying, when you want to batch process a greater number
of files. To execute a PI non-interactively it is possible to 
call a plugin from scripts with default parameters and no user interaction.

The \GxsmTT{diff}-PI can be called like this:

\begin{alltt}
import emb
print "Welcome to Python."
emb.logev('my logentry')
emb.startscan()
emb.action('diff_PI')
\end{alltt}

The \GxsmTT{diff}- and \GxsmTT{smooth}-function are, at the time of this
writing, the only Math-PI, that have such an 'action'-callback. Others 
will follow. See \GxsmFile{diff.C} to find out, how to extend your
favourite PI with action-capabilities.

The action-command can execute the following PI:

\begin{tabular}{ll}
\GxsmTT{diff\_PI} & kernel-size set to 5+1\\
\GxsmTT{smooth\_PI} & kernel-size set to 5+1\\
\GxsmTT{print\_PI} & defaults are read from gconf\\
\end{tabular}

% OptPlugInSubSection: DSP-Control

The DSP-Control is the heart of SPM activity. The following parameters
can be set with \GxsmTT{set}. (DSP2 commands are available in Gxsm 2 only)

\GxsmNote{Manual Hacker notes: list of DSP/DSP2 is depricated. All entry fields with hover-over entry id is now remote capable.}

\begin{tabular}{ll}
\GxsmTT{DSP\_CI} & \GxsmTT{DSP2\_CI} \\
\GxsmTT{DSP\_CP} & \GxsmTT{DSP2\_CP} \\
\GxsmTT{DSP\_CS} & \GxsmTT{DSP2\_CS} \\
\GxsmTT{DSP\_I} & \GxsmTT{DSP2\_I} \\
\GxsmTT{DSP\_MoveLoops} & \GxsmTT{DSP2\_MoveLoops} \\
\GxsmTT{DSP\_MoveSpd} & \GxsmTT{DSP2\_MoveSpd} \\
\GxsmTT{DSP\_NAvg} & \GxsmTT{DSP2\_NAvg} \\
\GxsmTT{DSP\_Pre} & \GxsmTT{DSP2\_Pre} \\
\GxsmTT{DSP\_ScanLoops} & \GxsmTT{DSP2\_ScanLoops} \\
\GxsmTT{DSP\_ScanSpd} & \GxsmTT{DSP2\_ScanSpd} \\
\GxsmTT{DSP\_SetPoint} & \GxsmTT{DSP2\_SetPoint} \\
\GxsmTT{DSP\_U} & \GxsmTT{DSP2\_U} \\
\end{tabular}

\GxsmNote{Manual Hacker notes: VP exectutes via hover-over ExecuteID and action command.}



% OptPlugInSubSection: Peakfinder

Another plugin allows remote control. The plugin-functions are commonly
executed by a call of the \GxsmTT{action}-command. It is
\GxsmFile{Peakfinder}:

DSP Peak Find Plugin Commandset for the SPA-LEED peak finder:\\

\begin{tabular}{lllll}
\multicolumn{5}{c}{Commands Plugin \filename{DSP Peak Find}:}\\ \\ \hline  
Cmd & Arg. & \multicolumn{2}{l}{Values} & Description\\ \hline
\hline
action & DSPPeakFind\_XY0\_1 &&& Get fitted XY Position\\
action & DSPPeakFind\_OffsetFromMain\_1 &&& Get Offset from Main\\
action & DSPPeakFind\_OffsetToMain\_1 &&& Put Offset to Main\\
action & DSPPeakFind\_EfromMain\_1 &&& Get Energy from Main\\
action & DSPPeakFind\_RunPF\_1 &&& Run Peak Finder\\
\hline
action & DSPPeakFind\_XXX\_N &&& run action XXX (see above)\\
       &                    &&& on PF Folder N\\
\end{tabular}

The call is equivalent to the example above.

% OptPlugInConfig

The plugin can be configured in the preferences. The script that will be executed
must be defined in the path-tab in item \GxsmFile{PyremoteFile}. 
The name must be a qualified python module name. A module name
is not a filename! Thus \verb+remote.py+ is not a valid entry,
but \verb+remote+ (the default) is. The module is found by searching the
directories listed in the environment variable PYTONPATH.

The module with GXSM internal commands 
is called \GxsmFile{emb}.

To find the Python-script \GxsmFile{remote.py}, the environment-variable
PYTHONPATH is evaluated. If it is not expliticly declared, GXSM will set
PYTHONPATH to your current working directory. This is equivalent to the
following call:

\begin{alltt}
$export PYTHONPATH='.'
$gxsm3
\end{alltt}

Thus, the script in your current working directory will be found.

If you want to put your script somewhere else than into the
current directory, modify the environment variable
\GxsmFile{PYTHONPATH}. Python
will look into all directories, that are stored there.

\begin{alltt}
$export PYTHONPATH='/some/obscure/path/'
$gxsm
\end{alltt}

Or you can link it from somewhere else. Or you can create a one line script,
that executes another script or several scripts. Do whatever you like.


% OptPlugInFiles
 Python precompiles your remote.py to remote.pyc. You can safely remove the
file remote.pyc file at any time, Python will regenerate it upon start of
the interpreter.

% OptPlugInRefs
See the appendix for more information. Don't know Python? Visit 
\GxsmTT{python.org}.

% OptPlugInKnownBugs

The error handling is only basic. Your script may run if you give
wrong parameters but not deliver the wanted results. You can crash
Gxsm or even X! E.g. by selecting an illegal channel. Remember that channel
counting in the scripts begins with 0. Gxsm's channel numbering begins with 1.

The embedded functions return $-1$ as error value. It's a good idea
to attach \texttt{print} to critical commands to check this.

The \verb+remote_echo+ command is implemented via debug printing.
Using Pythons \texttt{print} is recommended.

The view functions \GxsmFile{quick}, \GxsmFile{direct}, \GxsmFile{log}  
change the viewmode, but not the button in the main window, don't be
confused.

The waitscan and da0 function are not yet implemented and likely will never
be. 

The library detection during compilation is amateurish. Needs work. 

Python will check for the
right type of your arguments. Remember, that all values in \GxsmTT{set} are strings
and have to be quoted. Additionaly care for the case sensitivity.

If you you want to pause script execution, use the embedded sleep command
\GxsmTT{emb.sleep()} and not \GxsmTT{time.sleep()}, because the function from
the time library will freeze GXSM totally during the sleep. 
(This is not a bug, it's a feature.)

% OptPlugInNotes
TODO: Add more action-handlers in Math-PI.
% and clean up inconsistent use of spaces and tabs.

% OptPlugInHints
If you write a particularly interesting remote-script, please give it back
to the community. The GXSM-Forums always welcome input.

% EndPlugInDocuSection
 * -------------------------------------------------------------------------------- 
 */


#include <gtk/gtk.h>
#include "config.h"
#include "gxsm3/plugin.h"
#include "gxsm3/glbvars.h"
#include "gxsm3/action_id.h"
#include "gxsm3/xsmtypes.h"


#if defined HAVE_PYTHON2_7_PYTHON_H
#    include <python2.7/Python.h>
#elif defined HAVE_PYTHON2_6_PYTHON_H
#    include <python2.6/Python.h>
#elif defined HAVE_PYTHON2_5_PYTHON_H
#    include <python2.5/Python.h>
#endif

 
#include <sys/types.h>
#include <signal.h>

      //#include "app_remote.h"
#include "pyremote.h"

      // Plugin Prototypes
static void pyremote_init( void );
static void pyremote_about( void );
static void pyremote_configure( void );
static void pyremote_cleanup( void );
static void pyremote_run(GtkWidget *w, void *data);

// Fill in the GxsmPlugin Description here
GxsmPlugin pyremote_pi = {
	NULL,                   // filled in and used by Gxsm, don't touch !
	NULL,                   // filled in and used by Gxsm, don't touch !
	0,                      // filled in and used by Gxsm, don't touch !
	NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
	// filled in here by Gxsm on Plugin load, 
	// just after init() is called !!!
	// ----------------------------------------------------------------------
	// Plugins Name, CodeStly is like: Name-M1S[ND]|M2S-BG|F1D|F2D|ST|TR|Misc
	(char *)"Pyremote",
	NULL,
	// Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
	(char *)"Remote control",
	// Author(s)
	(char *) "Stefan Schroeder",
	// Menupath to position where it is appended to
	(char *)"tools-section",
	// Menuentry
	N_("Pyremote"),
	// help text shown on menu
	N_("Python Remote Control."),
	// more info...
	(char *)"See Manual.",
	NULL,          // error msg, plugin may put error status msg here later
	NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
	// init-function pointer, can be "NULL", 
	// called if present at plugin load
	pyremote_init,  
	// query-function pointer, can be "NULL", 
	// called if present after plugin init to let plugin manage it install itself
	NULL, // query should be "NULL" for Gxsm-Math-Plugin !!!
	// about-function, can be "NULL"
	// can be called by "Plugin Details"
	pyremote_about,
	// configure-function, can be "NULL"
	// can be called by "Plugin Details"
	pyremote_configure,
	// run-function, can be "NULL", if non-Zero and no query defined, 
	// it is called on menupath->"plugin"
	pyremote_run, // run should be "NULL" for Gxsm-Math-Plugin !!!
	// cleanup-function, can be "NULL"
	// called if present at plugin removal
  NULL, // direct menu entry callback1 or NULL
  NULL, // direct menu entry callback2 or NULL

	pyremote_cleanup
	};

GtkWidget* create_remote_tools();

// Text used in Aboutbox, please update!!a
static const char *about_text = N_("Gxsm Plugin\n\n"
                                   "Python Remote Control.");

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
	pyremote_pi.description = g_strdup_printf(N_("Gxsm pyremote plugin %s"), VERSION);
	return &pyremote_pi; 
}

// 5.) Start here with the plugins code, vars def., etc.... here.
// ----------------------------------------------------------------------
//
// TODO: 
// More error-handling
// Cannot return int in run
// fktname editable in preferences.
// Add numeric interface (LOW)
// Add image interface (LOW)
// Add i/o possibility (LOW)

// init-Function
static void pyremote_init(void)
{
	/* Python will search for remote.py in the directories, defined 
	   by PYTHONPATH. */
	PI_DEBUG(DBG_L2, "pyremote Plugin Init");
	if (!getenv("PYTHONPATH")){
		PI_DEBUG(DBG_L2, "pyremote: PYTHONPATH is not set.");
		PI_DEBUG(DBG_L2, "pyremote: Setting to '.'");
		setenv("PYTHONPATH", ".", 0);
	}
}

// about-Function
static void pyremote_about(void)
{
	const gchar *authors[] = { pyremote_pi.authors, NULL};
	gtk_show_about_dialog (NULL, "program-name",  pyremote_pi.name,
					"version", VERSION,
					  "license", GTK_LICENSE_GPL_3_0,
					  "comments", about_text,
					  "authors", authors,
					  NULL,NULL,NULL
					  ));
}

// configure-Function
static void pyremote_configure(void)
{
	if(pyremote_pi.app){
		pyremote_pi.app->message("Pyremote Plugin Configuration");
		create_remote_tools();
	}
}

// cleanup-Function
static void pyremote_cleanup(void)
{
	PI_DEBUG(DBG_L2, "Pyremote Plugin Cleanup");
}

#ifdef Py_PYTHON_H
///////////////////////////////////////////////////////////////
// BLOCK I
// grep AddEntry2RemoteList src/*.C
// ACAmp
// ACFrq
// ACPhase
// CPShigh
// CPSlow
// Energy
// Gatetime
// Layers
// LengthX
// LengthY
// Offset00X
// Offset00Y
// OffsetX
// OffsetY
// PointsX
// PointsY
// RangeX
// RangeY
// Rotation
// StepsX
// StepsY
// SubSmp
// VOffsetZ
// VRangeZ
// ValueEnd
// ValueStart
// nAvg

/* stolen from app_remote.C */
static void Check_ec(Gtk_EntryControl* ec, remote_args* ra){
	gdk_threads_enter();

	ec->CheckRemoteCmd (ra);

	gdk_threads_leave();
};

static void CbAction_ra(remote_action_cb* ra, gpointer arglist){
	gdk_threads_enter();
	if(ra->cmd && ((gchar**)arglist)[1])
		if(! strcmp(((gchar**)arglist)[1], ra->cmd)){
			if (ra->data)
				(*ra->RemoteCb) (ra->widget, ra->data);
			else
				(*ra->RemoteCb) (ra->widget, arglist);
			// see above and pcs.h
		}
	gdk_threads_leave();
};

/* This function will build and return a python tuple 
   that contains all the objects (string name) that 
   you can 'set' and 'get'. 

   Example output of 'print emb.list()':

   ('Counter', 'VOffsetZ', 'VRangeZ', 'Rotation',
   'TimeSelect', 'Time', 'LayerSelect', 'Layers',
   'OffsetY', 'OffsetX', 'PointsY', 'PointsX',
   'StepsY', 'StepsX', 'RangeY', 'RangeX')

   when no hardware is attached.

*/
static PyObject* remote_list(PyObject *self, PyObject *args)
{
	int slen = g_slist_length( gapp->RemoteEntryList ); // How many entries?

	// This will be our return object with as many slots as input list has:
	PyObject *ret = PyTuple_New(slen); 
	GSList* tmp = gapp->RemoteEntryList;
	for (int n=0; n<slen; n++)
		{
			Gtk_EntryControl* ec = (Gtk_EntryControl*)tmp->data; // Look at data item in GSList.
			PyTuple_SetItem(ret, n, PyString_FromString(ec->Get_Refname())); // Add Refname to Return-list 
			tmp = g_slist_next(tmp);
		}

	return ret;
}

static PyObject* remote_gets(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Getting as string ");
	gchar *parameter;

	if (!PyArg_ParseTuple(args, "s", &parameter))
		return Py_BuildValue("i", -1);

	int parameterlen = strlen(parameter);
	int slen = g_slist_length( gapp->RemoteEntryList ); 

	gchar *ret = NULL;

	GSList* tmp = gapp->RemoteEntryList;
	for (int n=0; n<slen; n++)
		{
			Gtk_EntryControl* ec = (Gtk_EntryControl*)tmp->data; 
    
			if (strncmp(parameter, ec->Get_Refname(), parameterlen) == 0)
				{
					ret = g_strdup(ec->Get_UsrString()); 
				}
			tmp = g_slist_next(tmp);
		}

	if (ret == NULL) // If the parameter doesn't exist.
		{ 
			ret = g_strdup("ERROR");
		}

	return Py_BuildValue("s", ret);
}


// Getting value in current user unit as plain double number -- could also/option get as string with unit
static PyObject* remote_get(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Getting ");
	gchar *parameter;
	remote_args ra;
	ra.qvalue = 0.;

	if (!PyArg_ParseTuple(args, "s", &parameter))
		return Py_BuildValue("i", -1);

	PI_DEBUG(DBG_L2, parameter << " query" );

	ra.qvalue = 0.;
	gchar *list[] = {(gchar *)"get", parameter, NULL};
	ra.arglist = list;

	g_slist_foreach(gapp->RemoteEntryList, (GFunc) Check_ec, (gpointer)&ra);
	PI_DEBUG(DBG_L2, parameter << " query result: " << ra.qvalue );

	return Py_BuildValue("f", ra.qvalue);
}


static PyObject* remote_set(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Setting ");
	remote_args ra;
	gchar *parameter, *value;

	if (!PyArg_ParseTuple(args, "ss", &parameter, &value))
		return Py_BuildValue("i", -1);

	PI_DEBUG(DBG_L2, parameter << " to " << value );

	ra.qvalue = 0.;
	gchar *list[] = { (char *)"set", parameter, value, NULL };
	ra.arglist = list;

	g_slist_foreach(gapp->RemoteEntryList, (GFunc) Check_ec, (gpointer)&ra);

	return Py_BuildValue("i", 0);
}

static PyObject* remote_action(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Action ") ;
	gchar *parameter, *value = (char *)"5.0";

	if (!PyArg_ParseTuple(args, "s|s", &parameter, &value))
		return Py_BuildValue("i", -1);

	PI_DEBUG(DBG_L2, parameter );

	PI_DEBUG(DBG_L2, "value:" << value);

	gchar *line3[] ={(char *)"action", parameter, value};

	g_slist_foreach(gapp->RemoteActionList, (GFunc) CbAction_ra, (gpointer)line3);

	return Py_BuildValue("i", 0);
}

// asks HwI via RTQuery for real time watches -- depends on HwI and it's capabilities/availabel options
/* Hardware realtime monitoring -- all optional */
/* default properties are
 * "X" -> current realtime tip position in X, inclusive rotation and offset
 * "Y" -> current realtime tip position in Y, inclusive rotation and offset
 * "Z" -> current realtime tip position in Z
 * "xy" -> X and Y
 * "zxy" -> Z, X, Y [mk2/3]
 * "o" -> Z, X, Y-Offset [mk2/3]
 * "f" -> feedback watch: f0, I, Irms as read on PanView [mk2/3]
 * "s" -> status bits [FB,SC,VP,MV,(PAC)], [DSP load], [DSP load peak]  [mk2/3]
 * "i" -> GPIO watch -- speudo real time, may be chached by GXSM: out, in, dir  [mk2/3]
 * "U" -> current bias
 */
static PyObject* remote_rtquery(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: RTQuery ") ;
	gchar *parameter;

	if (!PyArg_ParseTuple(args, "s", &parameter))
		return Py_BuildValue("i", -1);

	double u,v,w;
	gapp->xsm->hardware->RTQuery (parameter, u,v,w);

	return Py_BuildValue("fff", u,v,w);
}

// asks HwI via RTQuery for real time watches -- depends on HwI and it's capabilities/availabel options
static PyObject* remote_y_current(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: y_current ") ;
	gchar *parameter;

	gint y = gapp->xsm->hardware->RTQuery ();

	return Py_BuildValue("i", y);
}

///////////////////////////////////////////////////////////////
// BLOCK II
// startscan .    DONE
// stopscan .    DONE
// waitscan    DONE is commented out in app_remote
// initscan .    DONE
// scanupdate .   DONE
// setylookup N,X  DONE
// scanline  N,N,N  DONE
///////////////////////////////////////////////////////////////

static PyObject* remote_startscan(PyObject *self, PyObject *args)
{
	gdk_threads_enter();
	PI_DEBUG(DBG_L2, "pyremote: Starting scan");

	gapp->action_toolbar_callback(NULL, (void*)"Toolbar_Scan_Start");

	gdk_threads_leave();   

	return Py_BuildValue("i", 0);
}

static PyObject* remote_createscan(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Creating scan");

	/////
	PyObject *the_array;
	long* pbuf;
	int blen, i;

	long sizex, sizey, rangex, rangey;

	if(!PyArg_ParseTuple(args, "llllO", &sizex, &sizey, &rangex, &rangey, &the_array))
		return 0;
	if(PyObject_AsWriteBuffer(the_array, (void**)&pbuf, (Py_ssize_t*)&blen))
		return 0;
	blen /= sizeof(long);

	if ( blen != sizex*sizey ) {
		// Wrong number of arguments in List
		//return Py_BuildValue("i", -1);
		return 0;
	}
    
	gdk_threads_enter();

	/*for(i=0; i<blen; ++i)
	  pbuf[i] += 1;*/

	Scan *dst;
	gapp->xsm->ActivateFreeChannel(); 
	dst = gapp->xsm->GetActiveScan();

	dst->data.s.nx = sizex;
	dst->data.s.ny = sizey;
	dst->data.s.dx = 1; // unit?
	dst->data.s.dy = 1; // unit?
	dst->data.s.dz = 1;
	dst->data.s.rx = rangex;
	dst->data.s.ry = rangey;

	dst->data.s.x0 = 0.;
	dst->data.s.y0 = 0.;
	//  dst->data.s.alpha = 0.;

	dst->data.ui.SetUser ("User");

	gchar *tmp=g_strconcat ("PyCreate ",
				NULL);
	dst->data.ui.SetComment (tmp);
	g_free (tmp);

	dst->mem2d->Resize (dst->data.s.nx, dst->data.s.ny);

	/*Read*/
	for(gint i=0; i<dst->mem2d->GetNy(); i++){
                for(gint j=0; j<dst->mem2d->GetNx(); j++){
                        dst->mem2d->data->Z( (double) pbuf[i+sizex*j], j, i);
                }
        }
	dst->data.orgmode = SCAN_ORG_CENTER;
	dst->mem2d->data->MkXLookup (-dst->data.s.rx/2., dst->data.s.rx/2.);
	dst->mem2d->data->MkYLookup (-dst->data.s.ry/2., dst->data.s.ry/2.);
	gapp->spm_update_all();
	dst->draw();
	dst=NULL;

	gdk_threads_leave();   

	return Py_BuildValue("i", 0);
}

///////////////////////////////////////////////////////////////

static PyObject *remote_createscanf(PyObject * self, PyObject * args)
{
	PI_DEBUG(DBG_L2, "pyremote: Creating scanf");

	//
	PyObject *the_array;
	float *pbuf;
	int blen, i;

	long sizex, sizey, rangex, rangey;

	if (!PyArg_ParseTuple (args, "llllO", &sizex, &sizey, &rangex, &rangey, &the_array))
		return 0;
	if (PyObject_AsWriteBuffer(the_array, (void **) &pbuf, (Py_ssize_t*)&blen))
		return 0;
	blen /= sizeof(float);

	if (blen != sizex * sizey) {
		return 0;
	}

	gdk_threads_enter();

	Scan *dst;
	gapp->xsm->ActivateFreeChannel();
	dst = gapp->xsm->GetActiveScan();

	dst->data.s.nx = sizex;
	dst->data.s.ny = sizey;
	dst->data.s.dx = 1;  // unit?
	dst->data.s.dy = 1;  // unit?
	dst->data.s.dz = 1;
	dst->data.s.rx = rangex;
	dst->data.s.ry = rangey;

	dst->data.s.x0 = 0.;
	dst->data.s.y0 = 0.;

	dst->data.ui.SetUser("User");

	gchar *tmp = g_strconcat("PyCreate ", pbuf[0], NULL);
	dst->data.ui.SetComment(tmp);
	g_free(tmp);

	dst->mem2d->Resize(dst->data.s.nx, dst->data.s.ny, ZD_FLOAT);

	/*Read */
	for (gint i = 0; i < dst->mem2d->GetNy(); i++) {
		for (gint j = 0; j < dst->mem2d->GetNx(); j++) {
			dst->mem2d->data->Z((float) pbuf[i + sizex * j], j,
					    i);
		}
	}
	dst->data.orgmode = SCAN_ORG_CENTER;
	dst->mem2d->data->MkXLookup(-dst->data.s.rx / 2.,
				    dst->data.s.rx / 2.);
	dst->mem2d->data->MkYLookup(-dst->data.s.ry / 2.,
				    dst->data.s.ry / 2.);
	gapp->spm_update_all();
	dst->draw();
	dst = NULL;

	gdk_threads_leave();   

	return Py_BuildValue("i", 0);
}


static PyObject* remote_stopscan(PyObject *self, PyObject *args)
{
	gdk_threads_enter();


	PI_DEBUG(DBG_L2, "pyremote: Stopping scan");
	gapp->action_toolbar_callback(NULL, (void*)"Toolbar_Scan_Stop"); 

	gdk_threads_leave();   

	return Py_BuildValue("i", 0);
}

static PyObject* remote_waitscan(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Wait scan: commented out");
	//  if( gapp->xsm->ScanInProgress() ){
	//    PI_DEBUG(DBG_L2, "pyremote: Scan is in progress ");
	//  }
	//  else{
	//    PI_DEBUG(DBG_L2, "pyremote: Scan is finished ");
	//  }
	return Py_BuildValue("i", 0);
}

static PyObject* remote_scaninit(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Initializing scan");
	gapp->action_toolbar_callback(NULL, (void*)"Toolbar_Scan_Init"); 
	return Py_BuildValue("i", 0);
}

static PyObject* remote_scanupdate(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Updating scan (hardware)");
	gdk_threads_enter();
	gapp->action_toolbar_callback(NULL, (void*)"Toolbar_Scan_UpdateParam"); 
	gdk_threads_leave();   
	return Py_BuildValue("i", 0);
}

static PyObject* remote_scanylookup(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Scanylookup");
	int value1 = 0;
	double value2 = 0.0;
	if (!PyArg_ParseTuple(args, "ld", &value1, &value2))
		return Py_BuildValue("i", -1);
	gdk_threads_enter();
	PI_DEBUG(DBG_L2,  value1 << " and " << value2 );
	if(value1 && value2){
		gchar *cmd = NULL;
		cmd = g_strdup_printf ("2 %d %g", value1, value2);
		gapp->PutPluginData (cmd);
		gapp->action_toolbar_callback (NULL, (void*)"Toolbar_Scan_SetYLookup"); 
		g_free (cmd);
	}
	gdk_threads_leave();   
	return Py_BuildValue("i", 0);
}

static PyObject* remote_scanline(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Scan line");
	int value1 = 0, value2 = 0, value3 = 0;
	if (!PyArg_ParseTuple(args, "lll", &value1, &value2, &value3))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2,  value1 << " and " << value2 << " and " << value3);
	PI_DEBUG(DBG_L2, "pyremote: Warning toolbar NYI");
	gdk_threads_enter();
	if(value1){
		gchar *cmd = NULL;
		if(value2 && value3){
			cmd = g_strdup_printf ("3 %d %d %d",
					       value1, 
					       value2,
					       value3);
			gapp->PutPluginData (cmd);
			gapp->action_toolbar_callback (NULL,(void*)"Toolbar_Scan_Partial_Line");
		}
		else{
			cmd = g_strdup_printf ("d %d",
					       value1); 
			gapp->PutPluginData (cmd);
			gapp->action_toolbar_callback (NULL, (void*)"Toolbar_Scan_Line"); 
		}
		g_free (cmd);
	}
	gdk_threads_leave();   
	return Py_BuildValue("i", 0);
}

///////////////////////////////////////////////////////////////
// BLOCK III
// save  .      DONE
// saveas S,N       DONE
// load  S,N      DONE
// gnuimport S,N    DONE
// gnuexport S,N    DONE
// renamed import and export to avoid nameclash in python.
///////////////////////////////////////////////////////////////

static PyObject* remote_save(PyObject *self, PyObject *args)
{
	gdk_threads_enter();
	PI_DEBUG(DBG_L2, "pyremote: Save");
	//gapp->xsm->save (TRUE);
	//                              auto  all
	gapp->xsm->save(MANUAL_SAVE_AS, NULL, -1, TRUE);
	gdk_threads_leave();   
	return Py_BuildValue("i", 0);
}

static PyObject* remote_saveas(PyObject *self, PyObject *args)
{
	gdk_threads_enter();
	PI_DEBUG(DBG_L2, "pyremote: Save As ");
	gchar* zeile;
	long channel = 0;
	if (!PyArg_ParseTuple(args, "sl", &zeile, &channel))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2, zeile << " to Channel " << channel );
	if(zeile && channel){
		gapp->xsm->save(MANUAL_SAVE_AS, zeile, channel, TRUE);
		//gapp->xsm->save(TRUE, zeile, channel);
	}
	gdk_threads_leave();   
	return Py_BuildValue("i", 0);
}

static PyObject* remote_load(PyObject *self, PyObject *args)
{
	gdk_threads_enter();
	PI_DEBUG(DBG_L2, "pyremote: Loading ");
	gchar* zeile;
	long channel = 0;
	if (!PyArg_ParseTuple(args, "si", &zeile, &channel))
		return Py_BuildValue("i", -1);;
	PI_DEBUG(DBG_L2, zeile << " to Channel " << channel );
	if(zeile && channel){
		gapp->xsm->ActivateChannel( channel );
		gapp->xsm->load( zeile );
	}
	gdk_threads_leave();   
	return Py_BuildValue("i", 0);
}

static PyObject* remote_import(PyObject *self, PyObject *args)
{
	gdk_threads_enter();
	PI_DEBUG(DBG_L2, "pyremote: Importing ");
	gchar* zeile;
	long channel = 0;
	if (!PyArg_ParseTuple(args, "sl", &zeile, &channel))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2,  zeile << " to Channel " << channel);
	if(zeile && channel){
		gapp->xsm->ActivateChannel( channel );
		gapp->xsm->load( zeile );
	}
	gdk_threads_leave();   
	return Py_BuildValue("i", 0);
}

static PyObject* remote_export(PyObject *self, PyObject *args)
{
	gdk_threads_enter();
	PI_DEBUG(DBG_L2, "pyremote: Exporting ");
	gchar* zeile;
	long channel = 0;
	if (!PyArg_ParseTuple(args, "sl", &zeile, &channel))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2, zeile << " to Channel " << channel);
	if(zeile && channel){
		gapp->xsm->ActivateChannel( channel );
		gapp->xsm->gnuexport( zeile );
	}

	gdk_threads_leave();   
	return Py_BuildValue("i", 0);
}

///////////////////////////////////////////////////////////////
// BLOCK IV
// autodisp .      DONE
// chmodea N      DONE
// chmodex N      DONE
// chmodem N      DONE
// chmoden N,N      DONE
// chmodeno N      DONE
// chview1d N      DONE
// chview2d N      DONE
// chview3d N      DONE
// quick .      DONE
// direct .      DONE
// log .      DONE
///////////////////////////////////////////////////////////////

static PyObject* remote_autodisplay(PyObject *self, PyObject *args)
{
	gdk_threads_enter();
	PI_DEBUG(DBG_L2, "pyremote: Autodisplay");
	gapp->xsm->AutoDisplay();
	gdk_threads_leave();   
	return Py_BuildValue("i", 0);
}

static PyObject* remote_chmodea(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Chmode a ");
	long channel = 0;
	if (!PyArg_ParseTuple(args, "l", &channel))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2,  channel);
	if (channel) 
		gapp->xsm->ActivateChannel( channel );
	return Py_BuildValue("i", 0);
}

static PyObject* remote_chmodex(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Chmode x ");
	long channel = 0;
	if (!PyArg_ParseTuple(args, "l", &channel))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2, channel );
	if (channel)
		gapp->xsm->SetMode( channel, ID_CH_M_X );
	return Py_BuildValue("i", 0);
}

static PyObject* remote_chmodem(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Chmode m ");
	long channel = 0;
	if (!PyArg_ParseTuple(args, "l", &channel))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2,  channel );
	if (channel)
		gapp->xsm->SetMode( channel, ID_CH_M_MATH );
	return Py_BuildValue("i", 0);
}

static PyObject* remote_chmoden(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Chmode n ");
	long channel = 0;
	long mode = 0;
	if (!PyArg_ParseTuple(args, "ll", &channel, &mode))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2, channel << " to " << mode );
	if (channel && mode)
		gapp->xsm->SetMode(channel, ID_CH_M_X+mode);
	return Py_BuildValue("i", 0);
}

static PyObject* remote_chmodeno(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Chmode no ");
	long channel = 0;
	if (!PyArg_ParseTuple(args, "l", &channel))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2, channel );
	if (channel)
		gapp->xsm->SetView( channel, ID_CH_V_NO );
	return Py_BuildValue("i", 0);
}

static PyObject* remote_chview1d(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Chview 1d.");
	long channel;
	if (!PyArg_ParseTuple(args, "l", &channel))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2, channel );
	if (channel)
		gapp->xsm->SetView( channel, ID_CH_V_PROFILE );
	return Py_BuildValue("i", 0);
}

static PyObject* remote_chview2d(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Chview 2d");
	long channel;
	if (!PyArg_ParseTuple(args, "l", &channel))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2, channel );
	if (channel)
		gapp->xsm->SetView( channel, ID_CH_V_GREY );
	return Py_BuildValue("i", 0);
}

static PyObject* remote_chview3d(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Chview 3d.");
	long channel;
	if (!PyArg_ParseTuple(args, "l", &channel))
		return Py_BuildValue("i", -1);
	PI_DEBUG(DBG_L2, channel );
	if (channel)
		gapp->xsm->SetView( channel, ID_CH_V_SURFACE );
	return Py_BuildValue("i", 0);
}

static PyObject* remote_quick(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Quick");
	gapp->xsm->SetVM(SCAN_V_QUICK);
	return Py_BuildValue("i", 0);
}

static PyObject* remote_direct(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Direkt");
	gapp->xsm->SetVM(SCAN_V_DIRECT);
	return Py_BuildValue("i", 0);
}

static PyObject* remote_log(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Log");
	gapp->xsm->SetVM(SCAN_V_LOG);
	return Py_BuildValue("i", 0);
}

///////////////////////////////////////////////////////////////
// BLOCK V
// unitbz .   DONE
// unitvolt .  DONE
// unitev .  DONE
// units .  DONE
///////////////////////////////////////////////////////////////

static PyObject* remote_unitbz(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: unitbz");
	gapp->xsm->SetModeFlg(MODE_BZUNIT);
	gapp->xsm->ClrModeFlg(MODE_VOLTUNIT);
	return Py_BuildValue("i", 0);
}
static PyObject* remote_unitvolt(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: unitvolt");
	gapp->xsm->SetModeFlg(MODE_VOLTUNIT);
	gapp->xsm->ClrModeFlg(MODE_BZUNIT);
	return Py_BuildValue("i", 0);
}
static PyObject* remote_unitev(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: unitev");
	gapp->xsm->SetModeFlg(MODE_ENERGY_EV);
	gapp->xsm->ClrModeFlg(MODE_ENERGY_S);
	return Py_BuildValue("i", 0);
}
static PyObject* remote_units(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: units");
	gapp->xsm->SetModeFlg(MODE_ENERGY_S);
	gapp->xsm->ClrModeFlg(MODE_ENERGY_EV);
	return Py_BuildValue("i", 0);
}

///////////////////////////////////////////////////////////////
// BLOCK VI
// echo S      DONE
// logev S      DONE
// da0 X      DONE, commented out
// menupath S      DONE
// more actions by plugins S  NYI
///////////////////////////////////////////////////////////////

static PyObject* remote_echo(PyObject *self, PyObject *args)
{

	PI_DEBUG(DBG_L2, "pyremote: Echo.");
	gchar* line1;
	if (!PyArg_ParseTuple(args, "s", &line1))
		return Py_BuildValue("i", -1);
	/*Change the Debuglevel to: print always.*/
	PI_DEBUG(DBG_EVER, line1 );
	return Py_BuildValue("i", 0);
}

static PyObject* remote_logev(PyObject *self, PyObject *args)
{

	PI_DEBUG(DBG_L2, "pyremote: Log ev.");
	gchar* zeile;
	if (!PyArg_ParseTuple(args, "s", &zeile))
		return Py_BuildValue("i", -1);
	if(zeile){
		gapp->monitorcontrol->LogEvent((char *)"RemoteLogEv", zeile);
	}else{
		gapp->monitorcontrol->LogEvent((char *)"RemoteLogEv", (char *)"--");
	}
	return Py_BuildValue("i", 0);
}

static PyObject* remote_da0(PyObject *self, PyObject *args)
{

	PI_DEBUG(DBG_L2, "pyremote: da0 ");
	double channel;
	if (!PyArg_ParseTuple(args, "d", &channel))
		return Py_BuildValue("i", -1);
	if (channel){
		PI_DEBUG(DBG_L2, "Commented out.");
		//gapp->xsm->hardware->SetAnalog("da-name", channel);
	}
	return Py_BuildValue("i", 0);
}

static PyObject* remote_menupath(PyObject *self, PyObject *args)
{
	gdk_threads_enter();

	PI_DEBUG(DBG_L2, "pyremote: Searching menupath ");
	gchar *menu1;
	if (!PyArg_ParseTuple(args, "s", &menu1)) {
		return Py_BuildValue("i", -1);
	}
	PI_DEBUG(DBG_L2, menu1 );
	GtkWidget *menushell;
	GtkWidget *menuitem;
	gint pos;
	menushell = gnome_app_find_menu_pos (gapp->gxsmmenu, menu1, &pos);
	--pos;
	if(!menushell) {
		return 0;
	}
	PI_DEBUG(DBG_L2, "pyremote: Menu Shell Found: " << pos << " Item=" << menushell);
    
	menuitem = (GtkWidget*)g_list_nth_data(GTK_MENU_SHELL (menushell) -> children, pos);
    
	if (!menuitem) {
		return 0;
	}
	PI_DEBUG(DBG_L2, "pyremote: Menu Item Found: " << menuitem);
    
	GdkEvent event;
	gint return_val;
	gtk_signal_emit (G_OBJECT(GTK_MENU_ITEM (menuitem) ), 
			 gtk_signal_lookup ("activate", G_OBJECT_TYPE (G_OBJECT(menuitem))),
			 &event, &return_val);
    
	//    PI_DEBUG(DBG_L2, "pyremote: Signal emitted to pos " << pos << " ! ret=" << return_val);

	gdk_threads_leave();
	return Py_BuildValue("i", 0);
}

/* Taken from somewhere*/
static gboolean busy_sleep;
gint ret_false()
{
	gtk_main_quit();
	return FALSE;
}

void sleep_ms(int ms)
{
	if (busy_sleep) return;          /* Don't allow more than 1 sleep_ms */
	busy_sleep=TRUE;
	gdk_threads_enter();
	gtk_timeout_add(ms,(GtkFunction)ret_false,0); /* Start time-out function*/
	gtk_main();                             /* wait */
	gdk_threads_leave();
	busy_sleep=FALSE;
}

static PyObject* remote_sleep(PyObject *self, PyObject *args)
{
	PI_DEBUG(DBG_L2, "pyremote: Sleep ");
	long l;
	if (!PyArg_ParseTuple(args, "l", &l))
		return Py_BuildValue("i", -1);
	if (l){
		sleep_ms(l*100);
	}
	return Py_BuildValue("i", 0);
}

///////////////////////////////////////////////////////////

static PyMethodDef EmbMethods[] = {
	// BLOCK I
	{"set", remote_set, METH_VARARGS, "Set."},
	{"get", remote_get, METH_VARARGS, "Get."},
	{"gets", remote_gets, METH_VARARGS, "Get string."},
	{"list", remote_list, METH_VARARGS, "List."},
	{"action", remote_action, METH_VARARGS, "Action."},
	{"rtquery", remote_rtquery, METH_VARARGS, "RTQuery."},
	{"y_current", remote_y_current, METH_VARARGS, "RTQuery Current Scanline."},

	// BLOCK II
	{"createscan", remote_createscan, METH_VARARGS, "Create Scan."},
	{"createscanf", remote_createscanf, METH_VARARGS, "Create Scan float."},

	{"startscan", remote_startscan, METH_VARARGS, "Start Scan."},
	{"stopscan", remote_stopscan, METH_VARARGS, "Stop Scan."},
	{"waitscan", remote_waitscan, METH_VARARGS, "Wait Scan."},
	{"scaninit", remote_scaninit, METH_VARARGS, "Scaninit."},
	{"scanupdate", remote_scanupdate, METH_VARARGS, "Scanupdate."},
	{"scanylookup", remote_scanylookup, METH_VARARGS, "Scanylookup."},
	{"scanline", remote_scanline, METH_VARARGS, "Scan line."},

	// BLOCK III
	{"save", remote_save, METH_VARARGS, "Save."},
	{"saveas", remote_saveas, METH_VARARGS, "Save As."},
	{"load", remote_load, METH_VARARGS, "Load."},
	{"gnuexport", remote_export, METH_VARARGS, "Export."},
	{"gnuimport", remote_import, METH_VARARGS, "Import."},

	// BLOCK IV
	{"autodisplay", remote_autodisplay, METH_VARARGS, "Autodisplay."},
	{"chmodea", remote_chmodea, METH_VARARGS, "Chmode A."},
	{"chmodex", remote_chmodex, METH_VARARGS, "Chmode X."},
	{"chmodem", remote_chmodem, METH_VARARGS, "Chmode M."},
	{"chmoden", remote_chmoden, METH_VARARGS, "Chmode N."},
	{"chmodeno", remote_chmodeno, METH_VARARGS, "Chmode No."},
	{"chview1d", remote_chview1d, METH_VARARGS, "Chview 1d."},
	{"chview2d", remote_chview2d, METH_VARARGS, "Chview 2d."},
	{"chview3d", remote_chview3d, METH_VARARGS, "Chview 3d."},
	{"quick", remote_quick, METH_VARARGS, "Quick."},
	{"direct", remote_direct, METH_VARARGS, "Direct."},
	{"log", remote_log, METH_VARARGS, "Log."},

	// BLOCK V
	{"unitbz", remote_unitbz, METH_VARARGS, "UnitBZ."},
	{"unitvolt", remote_unitvolt, METH_VARARGS, "UnitVolt."},
	{"unitev", remote_unitev, METH_VARARGS, "UniteV."},
	{"units", remote_units, METH_VARARGS, "UnitS."},

	// BLOCK VI
	{"echo", remote_echo, METH_VARARGS, "Echo. "},
	{"logev", remote_logev, METH_VARARGS, "Logev. "},
	{"da0", remote_da0, METH_VARARGS, "Da0. "},
	{"menupath", remote_menupath, METH_VARARGS, "Menupath. "},
	{"sleep", remote_sleep, METH_VARARGS, "Sleep. "},

	{NULL, NULL, 0, NULL}
};


int ok_button_callback( GtkWidget *widget, gpointer data)
{
	//    cout << getpid() << endl;
	kill (getpid(), SIGINT); 
	//    cout << "pressed" <<endl;
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////7
GtkWidget* create_gui_elements()
{
	GtkWidget *mywin;
	// GtkWidget *clist;
	GtkWidget *button1;
	// GtkWidget *vbox;
	// GtkWidget *hbox
	GtkWidget *label1;

	mywin = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW (mywin), "Press to Interrupt Python Script");
	label1 = gtk_label_new ("Press button to interrupt after current script-command.");
	button1 = gtk_button_new_with_label ("Interrupt");

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(mywin)->action_area),button1);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(mywin)->vbox),label1);
	g_signal_connect (G_OBJECT (button1), "clicked",G_CALLBACK (ok_button_callback), NULL);
	g_signal_connect_object (G_OBJECT (button1), "clicked",
				   G_CALLBACK (gtk_widget_destroy),
				   G_OBJECT (mywin));
	gtk_widget_show_all (mywin);

	return(mywin);
}

GtkWidget* create_remote_tools(){
#if 0
	GtkWidget *tb;
	GtkWidget *toolbar = gtk_toolbar_new ();
	gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), GTK_ORIENTATION_VERTICAL);
	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_BOTH);
	gtk_container_set_border_width (GTK_CONTAINER (toolbar), 5);
	gtk_widget_show (toolbar);
	//        gtk_box_pack_start (GTK_BOX (gdk_get_default_root_window ()), toolbar, FALSE, FALSE, 0);
	//        gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar), TRUE);       
	//       gnome_appbar_new (FALSE, FALSE, GNOME_PREFERENCES_USER);
	//        gnome_ (GNOME_APP (gapp->app), toolbar);


        tb = gtk_button_new_with_label ("REMOTE SCRIPT TEST 1");
	//        g_signal_connect (G_OBJECT (tb), "toggled", G_CALLBACK(ProfileControl::linreg_callback), this);
        gtk_widget_show (tb);
        gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), tb, "tool1", "Tool1");

        return toolbar;
#endif
        return NULL;
}

typedef struct{
	void *data;
        int job;
        int progress;
} Embpy_Env;


gpointer embpy_thread (void *env){
        Embpy_Env* job = (Embpy_Env*)env;
	
	PyObject *pName, *pModule; //, *pDict;
	
	XInitThreads();

	//	gdk_threads_enter();
	//	gdk_threads_flush();
	//	gdk_threads_leave();

	Py_Initialize();
	Py_InitModule("emb", EmbMethods);
	
	pName = PyString_FromString(xsmres.PyremoteFile);
	
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	
	if (pModule == NULL) {
		PyErr_Print();
		fprintf(stderr, "Failed to execute Module. \n");
		pyremote_pi.app->message("Failed to execute Module.\n"
					 "Reason: No remote.py in working path found\n"
					 "or error in script, see terminal output."
					 );
	}
	
	Py_Finalize();

	job->job = -1; // done indicator
        return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// run-Function
static void pyremote_run( GtkWidget *w, void *data )
{
	GtkWidget *remote_window; 
	//    remote_window = create_gui_elements();

	static GThread*  pythr = NULL;
	static Embpy_Env pyjob;

	if (pythr){
		if (pyjob.job == 0)
			return;
		else    // error message -- a python script is running
			pyremote_pi.app->message ("Sorry:\n A Python remote script is currently running.");
	}

	pyjob.data = data;
	pyjob.job  = 0;
	pyjob.progress = 0;

	pythr = g_thread_new ("embpy_thread", embpy_thread, &pyjob);

#if 0
	PyObject *pName, *pModule; //, *pDict;

	Py_Initialize();
	Py_InitModule("emb", EmbMethods);

	pName = PyString_FromString(xsmres.PyremoteFile);

	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule == NULL) {
		PyErr_Print();
		fprintf(stderr, "Failed to execute Module. \n");
		pyremote_pi.app->message("Failed to execute Module.\n"
					 "Reason: No remote.py in working path found\n"
					 "or error in script, see terminal output."
					 );
	}

	Py_Finalize();
#endif

	//    gtk_widget_destroy(remote_window);
	return;
}
#endif

#ifndef Py_PYTHON_H
static void pyremote_run( GtkWidget *w, void *data )
{
	PI_DEBUG_ERROR(DBG_L1, "Python module was disabled at build time, sorry.");
	pyremote_pi.app->message("Sorry, Python remote module was disabled at build time.\n"
				 "Reason: missing python support libraries\n"
				 "Note: V2.5, 2.6, 2.7 are supported currently.\n"
				 "Suggestion: sudo apt-get install python2.7-dev");

	return;
}
#endif

