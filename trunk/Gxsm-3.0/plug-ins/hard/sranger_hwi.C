/* Gnome gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * Gxsm Hardware Interface Plugin Name: sranger_hwi.C
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
% PlugInDocuCaption: Signal Ranger Hardware Interface
% PlugInName: sranger_hwi
% PlugInAuthor: Percy Zahl
% PlugInAuthorEmail: zahl@users.sf.net
% PlugInMenuPath: Tools/SR-DSP Control

% PlugInDescription
This provides the Signal Ranger-STD and -SP2 hardware interface (HwI)
for GXSM.  It contains all hardware close and specific settings and
controls for feedback, scanning, all kind of probing (spectroscopy and
manipulations) and coarse motion control including the auto approach
controller. Invisible for the user it interacts with the SRanger DSP,
manages all DSP parameters and data streaming for scan and probe.



%The \GxsmEntry{SR DSP Control} Dialog is divided into several sections:
%1) Feedback & Scan: This folder contains all necessary options regarding feedback and scan.
%2) Trigger: For Multi-Volt and Current imaging
%3) Advanced: Different settings for advanced features of the Signal Ranger.
%4) STS: Parameter for dI/dV spectroscopy can be defined here.
%5) FZ: In this section information about force - distance curves can be set.
%6) LM: For manipulation of x, y, or z use this dialog.
%7) Lock-In: This slice contains the parameter set for the build in Lock-In.
%8) AX: Auxiliary
%9) Graphs: Choose sources and decide how to plot them.


% OptPlugInSection: SR-DSP Feedback and Scan Control
The Feedback \& Scan folder contains all necessary options regarding
feedback and scan.  Here the feedback parameters and tunneling/force
settings are adjusted. The second purpose of this control panel is the adjustment of
all scan parameters of the digital vector scan generator like speed.

\GxsmScreenShot{SR-DSP-Control-FB}{GXSM SR DSP Control window: Feedback and Scan Control page}
\GxsmScreenShot{SR-DSP-Control-Advanced}{GXSM SR DSP Control window: Advanced settings page}

\begin{description}

\item[Bias] Voltage that is applied to OUT6. Beware: The entered value
will be divided by the \GxsmPref{InstSPM}{BiasGain} value. Without an
additional amplification the \GxsmEntry{Instrument/BiasGain} value has
to be set to 1.

\item[Current(STM) / SetPoint(AFM)] Set point the DSP is trying to
reach. For a correct conversion of Current / SetPoint to incoming
voltages at IN5 the values of \GxsmPref{InstSPM}{nAmpere2Volt} and
\GxsmPref{InstSPM}{nNewton2Volt} have to be set correctly.

\item[CP, CI] Parameters for the feedback loop. CP is proportional to
the difference between the set point and the current value at IN5. CI
integrates this difference. Higher values for CP / CI mean a faster
loop.

\item[MoveSpd, ScanSpd] MoveSpd is valid for movements without taking
data (e.g. change xy-offset or moving to the starting point of an area
scan). ScanSpd is valid if the DSP is taking data.  PiezoDriveSetting:
Usually a high voltage amplifier has different gains. A change of the
gain can easily be mentioned in GXSM by activating the appropriate
factor. A change of this values acts instantly. The available gain
values can be defined in the \GxsmPref{InstSPM}{Analog/V1-V9}. After
changing the preferences GXSM needs to be restarted to take
effect. Remark: Piezo constants and other parameters have to be
charged. Please use the \GxsmPref{InstSPM}{Instrument/PiezoAV} for
this odd values.  
\end{description}

\GxsmHint{Good conservative start values for the feedback loop gains
CP and CI are 0.01 for both. Typically they can be increased up to
0.1, depending on the the system, tip and sample. In general CP can be
about 150\% of CI to gain more stability with same CI. }


% OptPlugInSection: Trigger -- Multi-Volt and -Current Control

It is possible to trigger automatic changes of Bias or
Current-Setpoint while scanning at up to four given X indices each, for
Bias and Current for forward and backward scan direction.

If the index given is reached, the DSP will trigger a bias or
current-setpoint change. The bias is never changed instantly, but is
ramped with an fixed speed of (54 V/s @ 10V max. Bias) from that
position to the new value. The index is actually down-counted to zero
for each scan line, thus index 0 means the last point of every scan
line.

The first line sets up the trigger points for X-forward (Xm) and
X-backward (Xp) for bias changes: Index-Xp, new Bias, Index-Xm, new
Bias.  The second to fourth line does the same in an equivalent
way. The next frame sets up the similar task for changes of the current
setpoint. Any Index never reached will just never trigger, thus an
index of -1 will disable it.

To enable or updated the trigger data table, toggle/enable the Trigger
Enable check-box. At startup, GXSM will always read the current DSP
persistent trigger table and trigger-enable state.

While the trigger is enabled, the Bias and Current settings are
locked. Any attempts to change it (i.e. the slider) will temporary
enforce your setting until the next trigger hits.

You can turn the Trigger feature on or off at any time.

\GxsmScreenShot{SR-DSP-Control-Trigger}{Trigger Control window}

% OptPlugInSection: Advanced Feedback and Probe Control

Advanced is a collection of different settings, to handle the advanced
capabilities of the Signal Ranger DSP. It is quite a mixture of
different tools.

\begin{description}
\item[FB Switch] This option enables or disables the feedback
loop. Disabling will keep the current Z-position.  

\item[Automatic Raster Probe] In general spectra can easily be taken
while doing an area scan. Activating this feature forces the DSP to
take spectra every line with an intended misalignment. A value of 1
means a spectrum will be taken every scan point. A value of 9 means a
distance of 9 area scan points between two spectra. The driven
spectrum depends on the probe control that is selected (STS, Z, PL).
Single spectra of a Raster Probe Scan easily can be handled by using
the \GxsmPopup{Events}{Show Probe} feature in the area plot. For a
conversion into a layer based image please use the
\GxsmMenu{Math/Probe/ImageExtract} plug-in.

\item[Show Expert Controls] This option hides or reveals some advanced options in
different folders. In this help file controls will be mentioned as
Expert Controls, if they are revealed by this option.  

\item[DynZoom] With the Signal Ranger DSP it is possible to dynamical zoom
while scanning.  

\item[PrePts] Problems with drift in x-direction can be reduced by scanning
a larger distance. The DSP adds equivalent points at both ends of a
line which will be ignored in the resulting scan data. The total
scan size will get larger in x by a factor of $(1 + 2 * PrePts / ScanPointsX)$.
Use the Pan-View plug-in to prevent trouble with the maximum scan size.
\end{description}



% OptPlugInSubSection: IV-Type Spectroscopy


\GxsmScreenShot{SR-DSP-Control-Advanced-Raster}{Advanced setup with Raster enabled}
\GxsmScreenShot{SR-DSP-Control-STS}{STS folder}

This is the dialog for scanning tunneling spectroscopy (STS). Sources
can be chosen in the Graphs Folder (Please have a look at the Graphs
section for details).

\begin{description}
\item[IV-Start-End] These are the start and the end values of the
spectrum. Optional a repetition counter is shown (Expert Control
option). This forces the Signal Ranger to do the STS spectrum n times.

\item[IV-dz] Lowering of the tip can improve the signal to noise ratio
especially at low voltages. IV-dz is the maximum lowering depth which is
reached at 0V. The lowering depth is equal to zero if the the I/V curve
meets the bias voltage. Lowering the tip means a decrease of the
tunneling gap. An automatic correction of the resistance is
implemented by means of several (\#) dI/dz spectra at the bias voltage
(Expert Control option).  

\item[Points] Number of points the spectrum will have. Please have a look at
\GxsmEntry{Int} for additional informations.  

\item[IV-Slope] Slope while collecting data.  

\item[Slope-Ramp] Slope while just moving.  

\item[Final-Delay] After running a spectrum this is the time the DSP waits
to enable the feedback again.  

\item[Recover] This is the time between two spectra where the feedback is
activated for a readjustment of the distance (Expert Control option).

\item[Status] Gives information about the ongoing spectrum:

\item[Tp] Total time the probe needs.  

\item[dU] Maximum difference of the voltages that will be applied.

\item[dUs] Stepsize of the data points.  

\item[Feedback On] Decides whether the feedback will be on or off while
taking a spectrum.  

\item[Dual] When activated the DSP will take two spectra. One spectrum is
running from Start to End directly followed by a spectrum from the End
to the Start value.  

\item[Int] When activated the DSP will average all fetched data between two
points. It can easily be seen, that decreasing the values of IV-Slope
or Points will increase the oversampling and therefore will improve
the quality of the spectrum.See note ($*$) below.

\item[Ramp] This option forces the DSP to stream all data to the PC
including the Slope-Ramps.  

\item[Save] When activated, GXSM will save spectra automatically to your
home directory.  

\item[Plot] When activated, GXSM will automatically show/update the plots
chosen in the Graphs dialog.
\end{description}


\GxsmNote{$*$ The sampling rate of the Signal Ranger is 22.1 kHz so the
time between two points of a spectrum leads directly to the number of
interim points that can be used for oversampling.
%
total time for the spectrum:
\[ ts = dU / IV-Slope \]
%
time per point:
\[ tp = ts / Points = dU / (IV-Slope * Points) \]
%
number of samples at one point:
\[ N = tp * 22100 Hz = 22100 Hz * dU / (IV-Slope * Points) \]
}


% OptPlugInSubSection: Vertical (Z) Manipulation
Manipulation in general is controlled or forced top motion in one or
more dimensions for any desired purpose.
This is the dialog for distance spectroscopy and forced Z/tip manipulation.

\GxsmScreenShotDual{SR-DSP-Control-Z}{GXSM SR DSP Control window, left: Z manipulation}{SR-DSP-Control-LM}{LM, lateral and Z manipulation}

\begin{description}
\item[Z-Start-End] These are the start and the end values of the spectrum in
respect to the current position.  

\item[Points] Number of points the spectrum will have. Please have a look at
\GxsmEntry{Int} for additional informations.

\item[Z-Slope] Slope while collecting data.

\item[Slope-Ramp] Slope while just moving.

\item[Final-Delay] After running a spectrum this is the time the DSP waits
to enable the feedback again.

\item[Status] Gives information about the ongoing spectrum:

\item[Tp] Total time the probe needs. 
\end{description}

Informations about the check options can be found in STS.



% OptPlugInSubSection: Lateral Manipulation 
With LM a lateral manipulation of the tip/sample is possible.
But also the Z-dimension can be manipulated at the same time if dZ set to a non zero value.

\begin{description}
\item[dxyz] Distance vector that will be covered.

\item[Points] While moving it is possible to collect data. Points defines the number of collected data points.

\item[LM-Slope] Speed of the tip/sample.

\item[Final-Delay] Timeout after lateral manipulation.

\item[Status] Gives information about the ongoing move.
\end{description}

Informations about the check options can be found in STS.




% OptPlugInSubSection: Tip Enhancements and Field based Manipulation
There are several possibilities to prepare a tip. One is to dip the
tip into the sample in a controlled manner (use \GxsmEntry{Z} for
this). Another option is applying a charge pulse using this \GxsmEntry{PL} dialog.

\GxsmScreenShot{SR-DSP-Control-PL}{GXSM SR DSP Control window: PL mode}

\begin{description}
\item[Duration] Determines the duration of the pulse.
\item[Volts] Applied voltage.

\item[Slope] Slope to reach \GxsmEntry{Volts}.

\item[Final Delay] Delay for relaxing the I/V-converter after pulsing.

\item[Repetitions] How many pulses are applied.

\item[Status] Gives information about the ongoing pulse. 
\end{description}

Informations about the check options can be found in STS.


% OptPlugInSubSection: Control of the digital (DSP) Lock-In
The Lock-In folder provides all settings concerning the build in
digital Lock-In. The Lock-In and with it the bias modulation is
automatically turned on if any Lock-In data channel is requested,
either for probing/STS (in Graphs) or as a scan data source
(Channelselector) for imaging.

There are a total of five digital correlation sums computed:

Averaged Input Signal (LockIn0), 

Phase-A/1st order (LockIn1stA),
Phase-B/1st order (LockIn1stB),

Phase-A/2nd order (LockIn2ndA),
Phase-B/2nd order (LockIn2ndB).

\GxsmNote{Please always select LockIn0 for STS.}

\GxsmScreenShot{SR-DSP-Control-LockIn}{GXSM SR DSP Control window: Lock-In settings}

\begin{description}
\item[AC-Amplitude] The amplitude of the overlaid Lock-In AC voltage.
\item[AC-Frequency] The base frequency of the Lock-In. There are four fixed frequency choices.
\item[AC-Phase-AB] Phase for A and B signal, applied for both, 1st and 2nd order.
\item[AC-Avg-Cycles] This sets the length for averaging, i.e. the corresponding time-constant.
\end{description}

For adjustments purpose only are the following parameters and the execute function here,
not needed to run the Lock-In for all other modes. 
The special probe mode implemented in this section can actually sweep the phase of the Lock-In,
it is useful to figure out the correct phase to use:

\begin{description}
\item[span] Full phase span to sweep.
\item[pts] Number data points to acquire while phase sweep.
\item[slope] Phase ramp speed. 
\end{description}

The digital Lock-In is restricted to a fixed length of base period
(choices are 128, 64, 32, 16 samples/per period with a fixed sample
rate of 22100 samples/s) and a fixed number of 8 periods for computing
the correlation sum: The total number of periods used for correlation
of data can be increased by setting AC-Avg-Cycles greater than one,
then overlapping sections of the 8 period long base window is used for
building the correlation sum. Thus the total integration length (time constant ) is

\[ \tau = \frac{\text{AC-Ave-Cycels} \cdot 8}{\text{Frq}} \]
\[ \text{Frq} = \frac{22100 \:\text{Hz}}{M = 128, 64, 32, 16} \].

There for the following discrete frequencies are available: 172.7$\:$Hz, 345.3$\:$Hz, 690.6$\:$Hz, 1381.2$\:$Hz.


The four correlation sums for A/B/1st/2nd are always computed in
parallel on the DSP if the Lock-In is enabled -- regardless what data
is requested. The correlation length is given by:

\[ N = 128 \cdot \text{AC-Ave-Cycels} \cdot 8\]
\[ \omega = 2 \pi \cdot \text{Frq} \]

Lock-In data calculations and reference signal generation is all in digital regime on the DSP in real-time. 
The modulation is applied to the Bias voltage by default automatically only if the Lock-In is active:
\[ U_{\text{ref}} = \text{AC-Amp} \cdot \sin(\omega t) + \text{Bias}\]

Averaged signal and Lock-In output signals calculated:
\[ U_{\text{LockIn0}} = \sum_{i=0}^{N-1}{U_{in,i}} \]
\[ U_{\text{LockIn1stA}} = \frac{2 \pi}{N} \sum_{i=0}^{N-1}{\text{AC-Amp} \cdot U_{in,i} \cdot \sin(i \frac{2\pi}{M} + \text{Phase-A})  }    \]
\[ U_{\text{LockIn1stB}} = \frac{2 \pi}{N} \sum_{i=0}^{N-1}{\text{AC-Amp} \cdot U_{in,i} \cdot \sin(i \frac{2\pi}{M} + \text{Phase-B})  }    \]
\[ U_{\text{LockIn2ndA}} = \frac{2 \pi}{N} \sum_{i=0}^{N-1}{\text{AC-Amp} \cdot U_{in,i} \cdot \sin(2 i \frac{2\pi}{M} + \text{Phase-A})  }    \]
\[ U_{\text{LockIn2ndA}} = \frac{2 \pi}{N} \sum_{i=0}^{N-1}{\text{AC-Amp} \cdot U_{in,i} \cdot \sin(2 i \frac{2\pi}{M} + \text{Phase-B})  }    \]

\GxsmNote{Implemented in FB\_spm\_probe.c, run\_lockin() (C) by P.Zahl 2002-2007. }

\GxsmNote{All Lock-In data is raw summed in 32bit integer variables by the DSP, they are not normalized at this time and moved to \Gxsmx via FIFO.
\Gxsmx applies the normalization before plotting. }


Informations about the check options can be found in STS.

\clearpage

% OptPlugInSubSection: Auxiliary Probe Control

\GxsmScreenShot{SR-DSP-Control-AX}{GXSM SR DSP Control window: AX (Auxiliary) settings}

This folder can be used for control and data acquisition from several
kind of simple instruments like a QMA or Auger/CMA.

\GxsmNote{Best is to setup a new user for this instrument and
configure the Bias-Gain so the ``Voltage'' corresponds to what you
need. As input you can select any channel, including Lock-In and
Counter. Here the Gate-Time is used to auto set the V-slope to match
V-range and points.}

\clearpage


% OptPlugInSubSection: Data Sources and Graphing Control
In the Graphs folder all available data channels are listed. If a
Source is activated, measured data will be transferred into the
buffer. Saving the buffer will automatically save all activated
sources.  Additionally it is possible to define a source as to be
displayed. 


\GxsmScreenShot{SR-DSP-Control-Graphs}{GXSM SR DSP Control window, Graphs page: Plot and Data sources setup.}


\GxsmHint{Beware: If a channel is not marked as a Source there will be no data
to be displayed even if X or Y is checked.}


\clearpage




% OptPlugInSection: SR-DSP Mover and Approach Control

GXSM with the SRanger also provides ramp like signal generation for
slip-stick type slider/mover motions which are often used for coarse
positioning aud tip approach. Set
\GxsmPref{User}{User/SliderControlType} to \GxsmEntry{mover} to get
the most configurable Mover Control dialog. If set to
\GxsmEntry{slider} (default setting) the dialog will be simplified for
Z/approach only. The different tabs are only for users convenience to
store different speed/step values, the output will always occur as
configured on the \GxsmEntry{Config} folder.

\GxsmScreenShotDual{SR-DSP-Mover}{GXSM SR generic coarse mover controller}{SR-DSP-Mover-Auto}{Auto approach controller}
\GxsmScreenShot{SR-DSP-MoverConfig}{GXSM SR DSP configuration of SRanger inertial driver engine.}


\clearpage


% OptPlugInSection: Extra Python SR-DSP Control and Configuration Scripts

The Python script \GxsmFile{SRanger/TiCC-project-files/FB\_spmcontrol/python\_scripts/sr\_spm\_control.py}
can be used for inspection and DSP/SPM software configuartion:

\GxsmScreenShot{SR-spm-control}{Main Menu}
\GxsmScreenShot{SR-spm-control-AIC-Offset}{AIC Offset Control}
\GxsmScreenShot{SR-spm-control-CR-ratemeter}{CoolRunner Rate Meter}
\GxsmScreenShot{SR-spm-control-gain}{AIC Gain Control}
\GxsmScreenShot{SR-spm-control-info}{SR-DSP/SPM software version info}
\GxsmScreenShot{SR-spm-control-settings}{SPM settings}
\GxsmScreenShot{SR-CR-Stage-LVDT-Control}{DSP Stage/LVDT Control}

\clearpage

% PlugInUsage
Set the \GxsmPref{Hardware}{Card} to ''SRanger:SPM''.

\GxsmNote{
Launch\\ \filename{/SRanger/TiCC-project-files/FB\_spmcontrol/FB\_spmcontrol.out}\\
on the SR before starting \Gxsm!
\\
Execute ``loadusb'' like this before starting \Gxsm or any other DSP tool:
\\
\GxsmFile{besocke@thundera:~/SRanger/loadusb\$ ./loadusb ../TiCC-project-files/FB\_spmcontrol/FB\_spmcontrol.out}
}

%% OptPlugInSources

%% OptPlugInDest

% OptPlugInNote
Special features and behaviors to be documented here!

% EndPlugInDocuSection
* -------------------------------------------------------------------------------- 
*/

#include <sys/ioctl.h>

#include "config.h"
#include "gxsm/plugin.h"
#include "gxsm/xsmhard.h"
#include "gxsm/glbvars.h"

// Define HwI PlugIn reference name here, this is what is listed later within "Preferenced Dialog"
// i.e. the string selected for "Hardware/Card"!
#define THIS_HWI_PLUGIN_NAME "SRanger:SPM"

// Plugin Prototypes
static void sranger_hwi_init( void );
static void sranger_hwi_about( void );
static void sranger_hwi_configure( void );
static void sranger_hwi_query( void );
static void sranger_hwi_cleanup( void );

static void DSPControl_show_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data);
static void DSPMover_show_callback (GSimpleAction *simple, GVariant *parameter, gpointer user_data);

static void DSPControl_StartScan_callback ( gpointer );

static void DSPControl_SaveValues_callback ( gpointer );
static void DSPControl_LoadValues_callback ( gpointer );


// Fill in the GxsmPlugin Description here
GxsmPlugin sranger_hwi_pi = {
	NULL,                   // filled in and used by Gxsm, don't touch !
	NULL,                   // filled in and used by Gxsm, don't touch !
	0,                      // filled in and used by Gxsm, don't touch !
	NULL,                   // The Gxsm-App Class Ref.pointer (called "gapp" in Gxsm) is 
	// filled in here by Gxsm on Plugin load, 
	// just after init() is called !!!
	// ----------------------------------------------------------------------
	// Plugins Name, CodeStly is like: Name-M1S|M2S-BG|F1D|F2D|ST|TR|Misc
	"sranger_hwi-"
	"HW-INT-1S-SHORT",
	// Plugin's Category - used to autodecide on Pluginloading or ignoring
	// In this case of Hardware-Interface-Plugin here is the interface-name required
	// this is the string selected for "Hardware/Card"!
	THIS_HWI_PLUGIN_NAME,
	// Description, is shown by PluginViewer (Plugin: listplugin, Tools->Plugin Details)
	g_strdup ("SRanger hardware interface."),
	// Author(s)
	"Percy Zahl",
	// Menupath to position where it is appendet to -- not used by HwI PIs
	"windows-section,windows-section",
	// Menuentry -- not used by HwI PIs
	N_("SPM Control,Mover Control"), // N_(THIS_HWI_PLUGIN_NAME"-HwI"),
	// help text shown on menu
	N_("This is the "THIS_HWI_PLUGIN_NAME" - GXSM Hardware Interface."),
	// more info...
	"N/A",
	NULL,          // error msg, plugin may put error status msg here later
	NULL,          // Plugin Status, managed by Gxsm, plugin may manipulate it too
	// init-function pointer, can be "NULL", 
	// called if present at plugin load
	sranger_hwi_init,  
	// query-function pointer, can be "NULL", 
	// called if present after plugin init to let plugin manage it install itself
	sranger_hwi_query, // query can be used (otherwise set to NULL) to install
	// additional control dialog in the GXSM menu
	// about-function, can be "NULL"
	// can be called by "Plugin Details"
	sranger_hwi_about,
	// configure-function, can be "NULL"
	// can be called by "Plugin Details"
	sranger_hwi_configure,
	// run-function, can be "NULL", if non-Zero and no query defined, 
	// it is called on menupath->"plugin"
	NULL, // run should be "NULL" for Gxsm-Math-Plugin !!!
	// cleanup-function, can be "NULL"
	// called if present at plugin removal
	DSPControl_show_callback, // direct menu entry callback1 or NULL
	DSPMover_show_callback, // direct menu entry callback2 or NULL
	sranger_hwi_cleanup
};


// Text used in Aboutbox, please update!!
static const char *about_text = N_("GXSM sranger_hwi Plugin\n\n"
                                   "Signal Ranger Hardware Interface for SPM.");

/* Here we go... */

#include "sranger_hwi.h"
#include "sranger_hwi_control.h"

/*
 * PI global
 */

// #define PI_DEBUG(L, DBGTXT) std::cout << "** (" << __FILE__ << ": " << __FUNCTION__ << ") Gxsm-PI-DEBUG-MESSAGE **: " << std::endl << " - " << DBGTXT << std::endl

gchar *sranger_hwi_configure_string = NULL;   // name of the currently in GXSM configured HwI (Hardware/Card)
sranger_hwi_dev *sranger_hwi_hardware = NULL; // instance of the HwI derived XSM_Hardware class

const gchar *DSPControl_menupath  = "windows-section";
const gchar *DSPControl_menuentry = N_("SR-DSP Control");
const gchar *DSPControl_menuhelp  = N_("open the SR-DSP control window");

const gchar *DSPMover_menuentry = N_("SR-DSP Mover");
const gchar *DSPMover_menuhelp  = N_("open the SR-DSP mover control window");

DSPControl *DSPControlClass = NULL;
DSPMoverControl *DSPMoverClass = NULL;


/* 
 * PI essential members
 */

// Symbol "get_gxsm_plugin_info" is resolved by dlsym from Gxsm, used to get Plugin's info!! 
// Essential Plugin Function!!
GxsmPlugin *get_gxsm_plugin_info ( void ){ 
	sranger_hwi_pi.description = g_strdup_printf(N_("GXSM HwI sranger_hwi plugin %s"), VERSION);
	return &sranger_hwi_pi; 
}

// Symbol "get_gxsm_hwi_hardware_class" is resolved by dlsym from Gxsm for all HwI type PIs, 
// Essential Plugin Function!!
XSM_Hardware *get_gxsm_hwi_hardware_class ( void *data ) {
	PI_DEBUG (DBG_L2, "sranger_hwi HardwareInterface Init");
	sranger_hwi_configure_string = g_strdup ((gchar*)data);
	sranger_hwi_hardware = new sranger_hwi_spm ();
	return sranger_hwi_hardware;
}

// init-Function
static void sranger_hwi_init(void)
{
	PI_DEBUG (DBG_L2, "sranger_hwi Plugin Init");
	sranger_hwi_hardware = NULL;
}

// about-Function
static void sranger_hwi_about(void)
{
	const gchar *authors[] = { sranger_hwi_pi.authors, NULL};
	gtk_show_about_dialog (NULL, 
			       "program-name",  sranger_hwi_pi.name,
			       "version", VERSION,
			       "license", GTK_LICENSE_GPL_3_0,
			       "comments", about_text,
			       "authors", authors,
			       NULL
			       );
}

// configure-Function
static void sranger_hwi_configure(void)
{
	PI_DEBUG (DBG_L2, "sranger_hwi Plugin HwI-Configure");
	if(sranger_hwi_pi.app)
		sranger_hwi_pi.app->message("sranger_hwi Plugin Configuration");
}

// query-Function
static void sranger_hwi_query(void)
{
	g_print ("SR-HwI::sranger_hwi_query:: <%s>\n",sranger_hwi_configure_string);
	PI_DEBUG (DBG_L2, "sranger_hwi Plugin Query: " << sranger_hwi_configure_string);

//	SR DSP Control Window
// ==================================================
	DSPControlClass = new DSPControl;
	sranger_hwi_pi.app->ConnectPluginToStartScanEvent (DSPControl_StartScan_callback);

	g_print ("SR-HwI::sranger_hwi_query:: ConnectPluginToCDFSaveEvent\n");
	// connect to GXSM nc-fileio
	sranger_hwi_pi.app->ConnectPluginToCDFSaveEvent (DSPControl_SaveValues_callback);
	sranger_hwi_pi.app->ConnectPluginToCDFLoadEvent (DSPControl_LoadValues_callback);

//	SR DSP Mover Control Window
// ==================================================
	DSPMoverClass = new DSPMoverControl;

	sranger_hwi_pi.status = g_strconcat(N_("Plugin query has attached "),
					   sranger_hwi_pi.name, 
					   N_(": SR-DSPControl is created."),
					   NULL);
}

static void DSPControl_show_callback(GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if ( DSPControlClass )
		DSPControlClass->show();
}

static void DSPMover_show_callback(GSimpleAction *simple, GVariant *parameter, gpointer user_data){
	if ( DSPMoverClass )
		DSPMoverClass->show();
}

static void DSPControl_StartScan_callback( gpointer ){
//	g_print ("SR-HwI::DSPControl_StartScan_callback");
	if ( DSPControlClass )
		DSPControlClass->update();
}

static void DSPControl_SaveValues_callback ( gpointer ncf ){
//	g_print ("SR-HwI::SPControl_SaveValues_callback\n");
	if ( DSPControlClass )
		DSPControlClass->save_values ((NcFile *) ncf);
}

static void DSPControl_LoadValues_callback ( gpointer ncf ){
//	g_print ("SR-HwI::SPControl_LoadValues_callback\n");
	if ( DSPControlClass )
		DSPControlClass->load_values ((NcFile *) ncf);
}

// cleanup-Function
static void sranger_hwi_cleanup(void)
{
	g_print ("SR-HwI::sranger_hwi_cleanup -- Plugin Cleanup --DSPCoCl\n");
	if( DSPControlClass )
		delete DSPControlClass ;
	DSPControlClass = NULL;

	if( DSPMoverClass )
		delete DSPMoverClass ;
	DSPMoverClass = NULL;

	g_print ("SR-HwI::sranger_hwi_cleanup -- Plugin Cleanup --sr_hwi\n");
	if (sranger_hwi_hardware)
		delete sranger_hwi_hardware;
	sranger_hwi_hardware = NULL;

	g_print ("SR-HwI::sranger_hwi_cleanup -- Plugin Cleanup --Info\n");
	g_free (sranger_hwi_configure_string);
	sranger_hwi_configure_string = NULL;

	g_print ("SR-HwI::sranger_hwi_cleanup -- Plugin Cleanup done.\n");
}

