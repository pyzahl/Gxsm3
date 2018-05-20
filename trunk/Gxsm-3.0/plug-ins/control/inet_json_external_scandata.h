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
        double dds_frequency_monitor;
        double volume_monitor;
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
        double frequency;
        double volume;
        double operation;
        double pacverbose;
        double transport_decimation;
        double transport_mode;
        double transport_ch3;
        double transport_ch4;
        double transport_ch5;
        double tune_dfreq;
        double tune_fspan;
        double tune_frequency;
        double amplitude_fb_setpoint;
        double amplitude_fb_cp;
        double amplitude_fb_ci;
        double exec_fb_upper;
        double exec_fb_lower;
        double amplitude_controller;
        double phase_fb_setpoint;
        double phase_fb_cp;
        double phase_fb_ci;
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
        { "DDS_FREQ_MONITOR", &pacpll_parameters.dds_frequency_monitor, true },
        { "VOLUME_MONITOR", &pacpll_parameters.volume_monitor, true },

        { "GAIN1", &pacpll_parameters.gain1, false },
        { "SHR_CH1", &pacpll_parameters.shr_ch1, false },
        { "GAIN2", &pacpll_parameters.gain2, false },
        { "SHR_CH2", &pacpll_parameters.shr_ch2, false },
        { "SHR_CH34", &pacpll_parameters.shr_ch34, false },
        { "GAIN3", &pacpll_parameters.gain3, false },
        { "GAIN4", &pacpll_parameters.gain4, false },
        { "GAIN5", &pacpll_parameters.gain5, false },
        { "PACTAU", &pacpll_parameters.pactau, false },
        { "FREQUENCY", &pacpll_parameters.frequency, false },
        { "VOLUME", &pacpll_parameters.volume, false },
        { "OPERATION", &pacpll_parameters.operation, false },
        { "PACVERBOSE", &pacpll_parameters.pacverbose, false },
        { "TRANSPORT_DECIMATION", &pacpll_parameters.transport_decimation, false },
        { "TRANSPORT_MODE", &pacpll_parameters.transport_mode, false },
        { "TRANSPORT_CH3", &pacpll_parameters.transport_ch3, false },
        { "TRANSPORT_CH4", &pacpll_parameters.transport_ch4, false },
        { "TRANSPORT_CH5", &pacpll_parameters.transport_ch5, false },
        { "TUNE_DFREQ", &pacpll_parameters.tune_dfreq, false },
        { "TUNE_SPAN", &pacpll_parameters.tune_fspan, false },
        { "FREQUENCY", &pacpll_parameters.tune_frequency, false },
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
        { NULL, NULL }
};

#if 0

    //Set gain
    APP.setGain1 = function() {

        APP.gain1 = $('#gain1_set').val();

        var local = {};
        local['GAIN1'] = { value: APP.gain1 };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#gain1_value').text(APP.gain1);

    };

    APP.setShrCh1 = function() {

        APP.shr_ch1 = $('#shr_ch1_set').val();

        var local = {};
        local['SHR_CH1'] = { value: APP.shr_ch1 };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#shr_ch1_value').text(APP.shr_ch1);

    };

    //Set gain
    APP.setGain2 = function() {

        APP.gain2 = $('#gain2_set').val();

        var local = {};
        local['GAIN2'] = { value: APP.gain2 };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#gain2_value').text(APP.gain2);

    };
    
    APP.setShrCh2 = function() {

        APP.shr_ch2 = $('#shr_ch2_set').val();

        var local = {};
        local['SHR_CH2'] = { value: APP.shr_ch2 };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#shr_ch2_value').text(APP.shr_ch2);

    };

    APP.setShrCh34 = function() {

        APP.shr_ch34 = $('#shr_ch34_set').val();

        var local = {};
        local['SHR_CH34'] = { value: APP.shr_ch34 };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#shr_ch34_value').text(APP.shr_ch34);

    };

    //Set gain
    APP.setGain3 = function() {

        APP.gain3 = $('#gain3_set').val();

        var local = {};
        local['GAIN3'] = { value: APP.gain3 };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#gain3_value').text(APP.gain3);

    };

    //Set gain
    APP.setGain4 = function() {

        APP.gain4 = $('#gain4_set').val();

        var local = {};
        local['GAIN4'] = { value: APP.gain4 };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#gain4_value').text(APP.gain4);

    };

    //Set gain
    APP.setGain5 = function() {

        APP.gain5 = $('#gain5_set').val();

        var local = {};
        local['GAIN5'] = { value: APP.gain5 };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#gain5_value').text(APP.gain5);

    };

    // Set pactau
    APP.setPactau = function() {

        APP.pactau = $('#pactau_set').val();

        var local = {};
        local['PACTAU'] = { value: APP.pactau };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#pactau_value').text(APP.pactau);

    };

    // Set frequency
    APP.setFrequency = function() {

        APP.frequency = $('#frequency_set').val();

        var local = {};
        local['FREQUENCY'] = { value: APP.frequency };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#frequency_value').text(APP.frequency);

    };

    // Set volume
    APP.setVolume = function() {

        APP.volume = $('#volume_set').val();
	var volt = APP.volume/1000.;
	
        var local = {};
        local['VOLUME'] = { value: volt };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#volume_value').text(APP.volume);

    };

    // Set operation
    APP.setOperation = function() {

        APP.operation = $('#operation_set').val();

        console.log('Set OP to ' + APP.operation);

        var local = {};
        local['OPERATION'] = { value: APP.operation };
        APP.ws.send(JSON.stringify({ parameters: local }));
    };

    // Set pacverbose
    APP.setPACVerbose = function() {

        APP.pacverbose = $('#pacverbose_set').val();

        var local = {};
        local['PACVERBOSE'] = { value: APP.pacverbose };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#pacverbose_value').text(APP.pacverbose);
    };

    APP.setTransportDecimation = function() {

        APP.TransportDecimation = $('#decimation_set').val();

        var local = {};
        local['TRANSPORT_DECIMATION'] = { value: APP.TransportDecimation };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#decimation_value').text(APP.TransportDecimation);
    };
    
    APP.setTransportMode = function() {


        APP.transport_mode = $('#transport_mode_set').val();

        var local = {};
        local['TRANSPORT_MODE'] = { value: APP.transport_mode };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#transport_mode_value').text(APP.transport_mode);
    };
    
    APP.setTransportCh3 = function() {
	
        APP.transport_ch3 = $('#transport_ch3_set').val();

        var local = {};
        local['TRANSPORT_CH3'] = { value: APP.transport_ch3 };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#transport_ch3_value').text(APP.transport_ch3);
    };
    
    APP.setTransportCh4 = function() {

        APP.transport_ch4 = $('#transport_ch4_set').val();

        var local = {};
        local['TRANSPORT_CH4'] = { value: APP.transport_ch4 };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#transport_ch4_value').text(APP.transport_ch4);
    };

    APP.setTransportCh5 = function() {

        APP.transport_ch5 = $('#transport_ch5_set').val();

        var local = {};
        local['TRANSPORT_CH5'] = { value: APP.transport_ch5 };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#transport_ch5_value').text(APP.transport_ch5);
    };

    
    APP.setTunedf = function() {
	APP.dfreq = $('#df_set').val();
        var local = {};
        local['TUNE_DFREQ'] = { value: APP.dfreq };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#df_value').text(APP.dfreq);
    };
    
    APP.setTunefs = function() {
	APP.fspan = $('#frequency_span_set').val();
	APP.tune_f = APP.frequency-APP.fspan/2;

        var local = {};
        local['TUNE_SPAN'] = { value: APP.fspan };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#frequency_span_value').text(APP.fspan);
    };
    
    // Tune in next freq
    APP.tuneNext = function() {
	APP.tune_f += 1.0*APP.dfreq;
	
	if (APP.tune_f > 1.0*APP.frequency+APP.fspan/2){
	    APP.dfreq = -Math.abs (1.0*APP.dfreq);
	    APP.tune_f = 1.0*APP.frequency + 1.0*APP.fspan/2;
            $('#df_value').text(APP.dfreq);
	}
	if (APP.tune_f < 1.0*APP.frequency-APP.fspan/2){
	    APP.dfreq = Math.abs (1.0*APP.dfreq);
	    APP.tune_f = 1.0*APP.frequency - 1.0*APP.fspan/2;
            $('#df_value').text(APP.dfreq);
	}
	
        var local = {};
        local['FREQUENCY'] = { value: APP.tune_f };
        APP.ws.send(JSON.stringify({ parameters: local }));
	
        $('#frequency_value').text(APP.tune_f);
    };
    



    // Set Ampl Controller
    APP.setAMPLITUDE_FB_SETPOINT = function() {

        APP.AMPLITUDE_FB_SETPOINT = $('#ampl_setpoint_set').val(); // mV

        var local = {};
        local['AMPLITUDE_FB_SETPOINT'] = { value: APP.AMPLITUDE_FB_SETPOINT };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#ampl_setpoint_value').text(APP.AMPLITUDE_FB_SETPOINT);

    };

    APP.setAMPLITUDE_FB_CP = function() {

        //write_pll_variable32 (dsp_pll.icoef_Amp, pll.signum_ci_Amp * CPN(29)*pow (10.,pll.ci_gain_Amp/20.));
        // = ISign * CPN(29)*pow(10.,Igain/20.);
		
        //write_pll_variable32 (dsp_pll.pcoef_Amp, pll.signum_cp_Amp * CPN(29)*pow (10.,pll.cp_gain_Amp/20.));
        // = PSign * CPN(29)*pow(10.,Pgain/20.);

        APP.AMPLITUDE_FB_CP = ($('#ampl_cp_gain_inv').is(':checked')?-1:1) * Math.pow(10.,$('#ampl_cp_gain_set').val()/20);
        console.log('AMPL FB CP = ' + $('#ampl_cp_gain_set') + ' => ' + APP.AMPLITUDE_FB_CP);

        var local = {};
        local['AMPLITUDE_FB_CP'] = { value: APP.AMPLITUDE_FB_CP };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#ampl_cp_gain_value').text(APP.AMPLITUDE_FB_CP);

    };

    APP.setAMPLITUDE_FB_CI = function() {

        APP.AMPLITUDE_FB_CI = ($('#ampl_ci_gain_inv').is(':checked')?-1:1) * Math.pow(10.,$('#ampl_ci_gain_set').val()/20);

        var local = {};
        local['AMPLITUDE_FB_CI'] = { value: APP.AMPLITUDE_FB_CI };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#ampl_ci_gain_value').text(APP.AMPLITUDE_FB_CI);

    };

    APP.setEXEC_FB_UPPER = function() {

        APP.EXEC_FB_UPPER = $('#exec_upper_set').val();

        var local = {};
        local['EXEC_FB_UPPER'] = { value: APP.EXEC_FB_UPPER };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#exec_upper_value').text(APP.EXEC_FB_UPPER);

    };

    APP.setEXEC_FB_LOWER = function() {

        APP.EXEC_FB_LOWER = $('#exec_lower_set').val();

        var local = {};
        local['EXEC_FB_LOWER'] = { value: APP.EXEC_FB_LOWER };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#exec_lower_value').text(APP.EXEC_FB_LOWER);

    };

    APP.setAmplLoopControl = function() {
	APP.AMPLITUDE_CONTROL = $("#ampl_control_on").is(':checked');
        console.log('Amplitude Loop Control: ', APP.AMPLITUDE_CONTROL);
        var local = {};
        local['AMPLITUDE_CONTROLLER'] = { value: APP.AMPLITUDE_CONTROL };
        APP.ws.send(JSON.stringify({ parameters: local }));
    };
    
    // Set Phase Controller 
    APP.setPHASE_FB_SETPOINT = function() {

        APP.PHASE_FB_SETPOINT = $('#phase_setpoint_set').val();

        var local = {};
        local['PHASE_FB_SETPOINT'] = { value: APP.PHASE_FB_SETPOINT };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#phase_setpoint_value').text(APP.PHASE_FB_SETPOINT);

    };

    APP.setPHASE_FB_CP = function() {

        APP.PHASE_FB_CP = ($('#phase_cp_gain_inv').is(':checked')?-1:1) * Math.pow(10.,$('#phase_cp_gain_set').val()/20);

        var local = {};
        local['PHASE_FB_CP'] = { value: APP.PHASE_FB_CP };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#phase_cp_gain_value').text(APP.PHASE_FB_CP);

    };

    APP.setPHASE_FB_CI = function() {

        APP.PHASE_FB_CI = ($('#phase_ci_gain_inv').is(':checked') ? -1:1) * Math.pow(10.,$('#phase_ci_gain_set').val()/20);

        var local = {};
        local['PHASE_FB_CI'] = { value: APP.PHASE_FB_CI };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#phase_ci_gain_value').text(APP.PHASE_FB_CI);

    };

    APP.setFREQ_FB_UPPER = function() {

        APP.FREQ_FB_UPPER = $('#exec_upper_set').val();

        var local = {};
        local['FREQ_FB_UPPER'] = { value: APP.FREQ_FB_UPPER };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#exec_upper_value').text(APP.FREQ_FB_UPPER);

    };

    APP.setFREQ_FB_LOWER = function() {

        APP.FREQ_FB_LOWER = $('#exec_lower_set').val();

        var local = {};
        local['FREQ_FB_LOWER'] = { value: APP.FREQ_FB_LOWER };
        APP.ws.send(JSON.stringify({ parameters: local }));

        $('#exec_lower_value').text(APP.FREQ_FB_LOWER);

    };

    APP.setPhaseLoopControl = function() {
	APP.PHASE_CONTROL = $("#phase_control_on").is(':checked');
        console.log('Phase Loop Control: ', APP.PHASE_CONTROL);
        var local = {};
        local['PHASE_CONTROLLER'] = { value: APP.PHASE_CONTROL };
        APP.ws.send(JSON.stringify({ parameters: local }));
    };
    
#endif


// Scan Control Class based on AppBase
// -> AppBase provides a GtkWindow and some window handling basics used by Gxsm
class Inet_Json_External_Scandata : public AppBase{
public:

        Inet_Json_External_Scandata(); // create window and setup it contents, connect buttons, register cb's...
	virtual ~Inet_Json_External_Scandata(); // unregister cb's
	
	void update(); // window update (inputs, etc. -- here currently not really necessary)

	GtkWidget *remote_param;
        static void connect_cb (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void dbg_l1 (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void dbg_l2 (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void dbg_l4 (GtkWidget *widget, Inet_Json_External_Scandata *self);
        static void got_client_connection (GObject *object, GAsyncResult *result, gpointer user_data);
        static void on_message(SoupWebsocketConnection *ws,
                               SoupWebsocketDataType type,
                               GBytes *message,
                               gpointer user_data);
        static void on_closed (SoupWebsocketConnection *ws, gpointer user_data);
        
        static void write_cb (GtkWidget *widget, Inet_Json_External_Scandata *self);

        static int jsoneq (const char *json, jsmntok_t *tok, const char *s) {
                if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
                    strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
                        return 0;
                }
                return -1;
        };
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

        void debug_log (const gchar *msg){
                if (debug_level > 4)
                        g_message (msg);
                if (debug_level > 2){
                        status_append (msg);
                        status_append ("\n");
                }
        };
        
private:
        BuildParam *bp;
       
        GtkWidget *input_rpaddress;
        GtkWidget *text_status;

        gint debug_level; 
        UnitObj *Unity; // Unit "1"
	
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


	GMutex mutex;

public:
};

#endif
