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
#include "jsmn.h"

struct JSON_parameter {
        const gchar *js_varname;
        double *value;
        gboolean ro;
};

struct JSON_signal {
        const gchar *js_varname;
        int size;
        double *value;
};

struct PACPLL_parameters {
        double dc_offset;
        double exec_amplitude_monitor;
        double dds_frequency_monitor;
        double volume_monitor;
        double phase_monitor;
        double cpu_load;
        double free_ram;
        double counter;

        double gain1;
        double shr_ch1;
        double gain2;
        double shr_ch2;
        double shr_ch34;
        double gain3;
        double gain4;
        double gain5;
        double pactau;
        double frequency_manual;
        double volume_manual;
        double operation;
        double pacverbose;
        double transport_decimation;
        double transport_mode;
        double transport_ch3;
        double transport_ch4;
        double transport_ch5;
        double tune_dfreq;
        double tune_span;
        double amplitude_fb_setpoint;
        double amplitude_fb_invert;
        double amplitude_fb_cp;
        double amplitude_fb_ci;
        double amplitude_fb_cp_db;
        double amplitude_fb_ci_db;
        double exec_fb_upper;
        double exec_fb_lower;
        double amplitude_controller;
        double phase_fb_setpoint;
        double phase_fb_invert;
        double phase_fb_cp;
        double phase_fb_ci;
        double phase_fb_cp_db;
        double phase_fb_ci_db;
        double freq_fb_upper;
        double freq_fb_lower;
        double phase_controller;

};

struct PACPLL_signals {
        double signal_ch1[1024];
        double signal_ch2[1024];
        double signal_ch3[1024];
        double signal_ch4[1024];
        double signal_ch5[1024];
};

PACPLL_parameters pacpll_parameters;
PACPLL_signals pacpll_signals;

JSON_parameter PACPLL_JSON_parameters[] = {
        { "DC_OFFSET", &pacpll_parameters.dc_offset, true },
        { "CPU_LOAD", &pacpll_parameters.cpu_load, true },
        { "COUNTER", &pacpll_parameters.counter, true },
        { "FREE_RAM", &pacpll_parameters.free_ram, true },
        { "EXEC_MONITOR", &pacpll_parameters.exec_amplitude_monitor, true },
        { "DDS_FREQ_MONITOR", &pacpll_parameters.dds_frequency_monitor, true },
        { "VOLUME_MONITOR", &pacpll_parameters.volume_monitor, true },
        { "PHASE_MONITOR", &pacpll_parameters.phase_monitor, true },

        { "GAIN1", &pacpll_parameters.gain1, false },
        { "SHR_CH1", &pacpll_parameters.shr_ch1, false },
        { "GAIN2", &pacpll_parameters.gain2, false },
        { "SHR_CH2", &pacpll_parameters.shr_ch2, false },
        { "SHR_CH34", &pacpll_parameters.shr_ch34, false },
        { "GAIN3", &pacpll_parameters.gain3, false },
        { "GAIN4", &pacpll_parameters.gain4, false },
        { "GAIN5", &pacpll_parameters.gain5, false },
        { "PACTAU", &pacpll_parameters.pactau, false },
        { "FREQUENCY_MANUAL", &pacpll_parameters.frequency_manual, false },
        { "VOLUME_MANUAL", &pacpll_parameters.volume_manual, false },
        { "OPERATION", &pacpll_parameters.operation, false },
        { "PACVERBOSE", &pacpll_parameters.pacverbose, false },
        { "TRANSPORT_DECIMATION", &pacpll_parameters.transport_decimation, false },
        { "TRANSPORT_MODE", &pacpll_parameters.transport_mode, false },
        { "TRANSPORT_CH3", &pacpll_parameters.transport_ch3, false },
        { "TRANSPORT_CH4", &pacpll_parameters.transport_ch4, false },
        { "TRANSPORT_CH5", &pacpll_parameters.transport_ch5, false },
        { "TUNE_DFREQ", &pacpll_parameters.tune_dfreq, false },
        { "TUNE_SPAN", &pacpll_parameters.tune_span, false },
        { "AMPLITUDE_FB_SETPOINT", &pacpll_parameters.amplitude_fb_setpoint, false },
        { "AMPLITUDE_FB_CP", &pacpll_parameters.amplitude_fb_cp, false },
        { "AMPLITUDE_FB_CI", &pacpll_parameters.amplitude_fb_ci, false },
        { "EXEC_FB_UPPER", &pacpll_parameters.exec_fb_upper, false },
        { "EXEC_FB_LOWER", &pacpll_parameters.exec_fb_lower, false },
        { "AMPLITUDE_CONTROLLER", &pacpll_parameters.amplitude_controller, false },
        { "PHASE_FB_SETPOINT", &pacpll_parameters.phase_fb_setpoint, false },
        { "PHASE_FB_CP", &pacpll_parameters.phase_fb_cp, false },
        { "PHASE_FB_CI", &pacpll_parameters.phase_fb_ci, false },
        { "FREQ_FB_UPPER", &pacpll_parameters.freq_fb_upper, false },
        { "FREQ_FB_LOWER", &pacpll_parameters.freq_fb_lower, false },
        { "PHASE_CONTROLLER", &pacpll_parameters.phase_controller, false },

        { NULL, NULL, true }
};

JSON_signal PACPLL_JSON_signals[] = {
        { "SIGNAL_CH1", 1024, pacpll_signals.signal_ch1 },
        { "SIGNAL_CH2", 1024, pacpll_signals.signal_ch2 },
        { "SIGNAL_CH3", 1024, pacpll_signals.signal_ch3 },
        { "SIGNAL_CH4", 1024, pacpll_signals.signal_ch4 },
        { "SIGNAL_CH5", 1024, pacpll_signals.signal_ch5 },
        { NULL, 0, NULL }
};

// Scan Control Class based on AppBase
// -> AppBase provides a GtkWindow and some window handling basics used by Gxsm
class Inet_Json_External_Scandata : public AppBase{
public:

        Inet_Json_External_Scandata(); // create window and setup it contents, connect buttons, register cb's...
	virtual ~Inet_Json_External_Scandata(); // unregister cb's
        int setup_scan (int ch, 
                        const gchar *titleprefix, 
                        const gchar *name,
                        const gchar *unit,
                        const gchar *label,
                        double d2u
                        );
	static void scan_start_callback (gpointer user_data);
	static void scan_stop_callback (gpointer user_data);

	static void pac_tau_parameter_changed (Param_Control* pcs, gpointer user_data);
	static void pac_frequency_parameter_changed (Param_Control* pcs, gpointer user_data);
	static void pac_volume_parameter_changed (Param_Control* pcs, gpointer user_data);
	static void tune_parameter_changed (Param_Control* pcs, gpointer user_data);
	static void amp_ctrl_parameter_changed (Param_Control* pcs, gpointer user_data);
	static void phase_ctrl_parameter_changed (Param_Control* pcs, gpointer user_data);
	static void amplitude_gain_changed (Param_Control* pcs, gpointer user_data);
        static void amplitude_controller_invert (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void amplitude_controller (GtkWidget *widget, Inet_Json_External_Scandata *self);
	static void phase_gain_changed (Param_Control* pcs, gpointer user_data);
        static void phase_controller_invert (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void phase_controller (GtkWidget *widget, Inet_Json_External_Scandata *self);

        static void choice_operation_callback (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void choice_transport_callback (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void choice_transport_ch3_callback (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void choice_transport_ch4_callback (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void choice_transport_ch5_callback (GtkWidget *widget, Inet_Json_External_Scandata *self);

        static void choice_auto_set_callback (GtkWidget *widget, Inet_Json_External_Scandata *self);

	void send_all_parameters ();
        
	void update (); // window update (inputs, etc. -- here currently not really necessary)
        void update_monitoring_parameters ();

        void update_graph ();

        void stream_data ();
        
        static void connect_cb (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void enable_scope (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void dbg_l1 (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void dbg_l2 (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void dbg_l4 (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void got_client_connection (GObject *object, GAsyncResult *result, gpointer user_data);
        static void on_message(SoupWebsocketConnection *ws,
                               SoupWebsocketDataType type,
                               GBytes *message,
                               gpointer user_data);
        static void on_closed (SoupWebsocketConnection *ws, gpointer user_data);
        
        void write_parameter (const gchar *paramater_id, double value);
        void write_parameter (const gchar *paramater_id, int value);

        static int json_dump(const char *js, jsmntok_t *t, size_t count, int indent) {
                int i, j, k;
                if (count == 0) {
                        return 0;
                }
                if (t->type == JSMN_PRIMITIVE) {
                        g_print("%.*s", t->end - t->start, js+t->start);
                        return 1;
                } else if (t->type == JSMN_STRING) {
                        g_print("'%.*s'", t->end - t->start, js+t->start);
                        return 1;
                } else if (t->type == JSMN_OBJECT) {
                        g_print("\n");
                        j = 0;
                        for (i = 0; i < t->size; i++) {
                                for (k = 0; k < indent; k++) g_print("  ");
                                j += json_dump(js, t+1+j, count-j, indent+1);
                                g_print(": ");
                                j += json_dump(js, t+1+j, count-j, indent+1);
                                g_print("\n");
                        }
                        return j+1;
                } else if (t->type == JSMN_ARRAY) {
                        j = 0;
                        g_print("\n");
                        for (k = 0; k < indent-1; k++) g_print("  ");
                        g_print("[");
                        for (i = 0; i < t->size; i++) {
                                j += json_dump(js, t+1+j, count-j, indent+1);
                                g_print(", ");
                        }
                        g_print("]\n");
                        return j+1;
                }
                return 0;
        };

        static JSON_parameter * check_parameter (const char *string, int len){
                //g_print ("[[check_parameter=%s]]", string);
                for (JSON_parameter *p=PACPLL_JSON_parameters; p->js_varname; ++p)
                        if (strncmp (string, p->js_varname, len) == 0)
                                return p;
                return NULL;
        };
        static JSON_signal * check_signal (const char *string, int len){
                for (JSON_signal *p=PACPLL_JSON_signals; p->js_varname; ++p){
                        //g_print ("[[check_signal[%d]=%.*s]=?=%s]", len, len, string,p->js_varname);
                        if (strncmp (string, p->js_varname, len) == 0)
                                return p;
                }
                return NULL;
        };
        static void dump_parameters (){
                for (JSON_parameter *p=PACPLL_JSON_parameters; p->js_varname; ++p)
                        g_print ("%s=%g\n", p->js_varname, *(p->value));
                for (JSON_signal *p=PACPLL_JSON_signals; p->js_varname; ++p){
                        g_print ("%s=[", p->js_varname);
                        for (int i=0; i<p->size; ++i)
                                g_print ("%g, ", p->value[i]);
                        g_print ("]\n");
                }
        };
        static int json_fetch(const char *js, jsmntok_t *t, size_t count, int indent){
                static JSON_parameter *jp=NULL;
                static JSON_signal *jps=NULL;
                static gboolean store_next=false;
                static int array_index=0;
                int i, j, k;
                if (count == 0) {
                        return 0;
                }
                if (indent == 0){
                        jp=NULL;
                        store_next=false;
                }
                if (t->type == JSMN_PRIMITIVE) {
                        //g_print("%.*s", t->end - t->start, js+t->start);
                        if (store_next){
                                if (jp){
                                        *(jp->value) = atof (js+t->start);
                                        //g_print ("ATOF[%.*s]=%g %14.8f\n", 20, js+t->start, *(jp->value), *(jp->value));
                                        jp=NULL;
                                        store_next = false;
                                } else if (jps){
                                        jps->value[array_index++] = atof (js+t->start);
                                        if (array_index >= jps->size){
                                                jps=NULL;
                                                store_next = false;
                                        }
                                }
                        }
                        return 1;
                } else if (t->type == JSMN_STRING) {
                        if (indent == 2){
                                jps=NULL;
                                jp=check_parameter ( js+t->start, t->end - t->start);
                                if (!jp){
                                        jps=check_signal ( js+t->start, t->end - t->start);
                                        if (jps) array_index=0;
                                }
                        }
                        if (indent == 3) if (strncmp (js+t->start, "value", t->end - t->start) == 0 && (jp || jps)) store_next=true;
                        //g_print("S[%d] '%.*s' [%s]", indent, t->end - t->start, js+t->start, jp || jps?"ok":"?");
                        return 1;
                } else if (t->type == JSMN_OBJECT) {
                        //g_print("\n O\n");
                        j = 0;
                        for (i = 0; i < t->size; i++) {
                                //for (k = 0; k < indent; k++) g_print("  ");
                                j += json_fetch(js, t+1+j, count-j, indent+1);
                                //g_print(": ");
                                j += json_fetch(js, t+1+j, count-j, indent+1);
                                //g_print("\n");
                        }
                        return j+1;
                } else if (t->type == JSMN_ARRAY) {
                        j = 0;
                        //g_print("\n A ");
                        //for (k = 0; k < indent-1; k++) g_print("  ");
                        //g_print("[");
                        for (i = 0; i < t->size; i++) {
                                j += json_fetch(js, t+1+j, count-j, indent+1);
                                //g_print(", ");
                        }
                        //g_print("]");
                        return j+1;
                }
                return 0;
        };

        void json_parse_message (const char *json_string);
        
        void status_append (const gchar *msg);
        void update_health ();
        
        void debug_log (const gchar *msg){
                if (debug_level > 4)
                        g_message (msg);
                if (debug_level > 2){
                        status_append (msg);
                        status_append ("\n");
                }
        };
        
private:
        int ch_freq;
        int ch_ampl;
        int data_decimation;
        int data_shr;
        int streampos;
        int x,y;
        
        PACPLL_parameters parameters;
        PACPLL_signals signals;

        BuildParam *bp;
        GSettings *inet_json_settings;
        
        gboolean run_scope;
        int transport;
        double gain_scale[5];
        double time_scale[5];
        GtkWidget *signal_graph;
       
        GtkWidget *input_rpaddress;
        GtkWidget *text_status;
	GtkWidget *red_pitaya_health;

        gint debug_level; 
        UnitObj *Unity, *Hz, *Deg, *VoltDeg, *Volt, *mVolt, *VoltHz, *dB, *Time, *uTime;

	GSList*   SPMC_RemoteEntryList;

        /* Socket Connection */
	GSocket *listener;
	gushort port;

	SoupSession *session;
	SoupMessage *msg;
	SoupWebsocketConnection *client;
        GIOStream *JSON_raw_input_stream;
	GError *client_error;
	GError *error;

        int block_message;

	GMutex mutex;

public:
};

#endif
