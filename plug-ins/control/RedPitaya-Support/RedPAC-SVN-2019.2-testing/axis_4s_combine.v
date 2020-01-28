`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
/* Gxsm - Gnome X Scanning Microscopy
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 * 
 * Copyright (C) 1999,2000,2001,2002,2003 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
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
// Create Date: 11/26/2017 08:20:47 PM
// Design Name: 
// Module Name: signal_combine
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
/*
 *  TRANSPORT FPGA MODULE to BRAM:
 *  S_AXIS1: M-AXIS-SIGNAL from LMS CH0 (input), CH1 (not used) -- pass through from ADC 14 bit
 *  S_AXIS2: M-AXIS-CONTROL Amplitude: output/monitor from Ampl. Controller (32 bit)
 *  S_AXIS3: M-AXIS-CONTROL Phase: output/monitor from Phase. Controller (48 bit)
 *  S_AXIS4: M-AXIS-CONTROL Phase pass, monitor Phase. from CORDIC ATAN X/Y (24 bit)
 *  S_AXIS5: M-AXIS-CONTROL Amplitude pass, monitor Amplitude from CORDIC SQRT (24 bit)
 *
 *  TRANSPORT MODE:
 *  BLOCK modes singel shot, fill after start, stop when full:
 *  0: S_AXIS1 CH1, CH2 decimated, aver aged (sum)
 *  1: S_AXIS4,5  decimated, averaged (sum)
 *  2: A_AXIS1 CH1 decimated, averaged (sum), ADDRreg
 *  3: TEST -- ch1n <= 32'h01234567, ch2n <= int_decimate_count;
 *  CONTINEOUS STEAM FIFO operation (loop, overwrite)
 *  ... modes to be added for FIFO contineous operation mode
 */

//////////////////////////////////////////////////////////////////////////////////


module axis_4s_combine #(
    parameter SAXIS_1_DATA_WIDTH  = 16,
    parameter SAXIS_1_TDATA_WIDTH = 32,
    parameter SAXIS_2_DATA_WIDTH  = 32,
    parameter SAXIS_2_TDATA_WIDTH = 32,
    parameter SAXIS_3_DATA_WIDTH  = 48,
    parameter SAXIS_3_TDATA_WIDTH = 48,
    parameter SAXIS_4_DATA_WIDTH  = 32,
    parameter SAXIS_4_TDATA_WIDTH = 32,
    parameter SAXIS_5_DATA_WIDTH  = 24,
    parameter SAXIS_5_TDATA_WIDTH = 24,
    parameter SAXIS_6_DATA_WIDTH   = 32,
    parameter SAXIS_6_TDATA_WIDTH  = 32,
    parameter SAXIS_78_DATA_WIDTH  = 32,
    parameter SAXIS_78_TDATA_WIDTH = 32,
    parameter MAXIS_TDATA_WIDTH = 32,
    parameter integer BRAM_DATA_WIDTH = 64,
    parameter integer BRAM_ADDR_WIDTH = 15,
    parameter integer DECIMATED_WIDTH = 32
)
(
    // (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_CLKEN a_clk" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_BUSIF S_AXIS1:S_AXIS2:S_AXIS3:S_AXIS4:S_AXIS5:S_AXIS6:S_AXIS7:S_AXIS8:M_AXIS_aux:M_AXIS_CH1:M_AXIS_CH2:M_AXIS_CH3:M_AXIS_CH4:M_AXIS_AMPL:M_AXIS_PHASE:M_AXIS_FCENTER:M_AXIS_FREQ31_0:M_AXIS_FREQ63_32" *)
    input a_clk,
    // input a_resetn
    
    input wire [SAXIS_1_TDATA_WIDTH-1:0]  S_AXIS1_tdata,
    input wire                            S_AXIS1_tvalid,
    input wire [SAXIS_2_TDATA_WIDTH-1:0]  S_AXIS2_tdata,
    input wire                            S_AXIS2_tvalid,
    input wire [SAXIS_3_TDATA_WIDTH-1:0]  S_AXIS3_tdata,
    input wire                            S_AXIS3_tvalid,
    input wire [SAXIS_4_TDATA_WIDTH-1:0]  S_AXIS4_tdata,
    input wire                            S_AXIS4_tvalid,
    input wire [SAXIS_5_TDATA_WIDTH-1:0]  S_AXIS5_tdata,
    input wire                            S_AXIS5_tvalid,
    input wire [SAXIS_6_TDATA_WIDTH-1:0]  S_AXIS6_tdata,
    input wire                            S_AXIS6_tvalid,
    input wire [SAXIS_78_TDATA_WIDTH-1:0]  S_AXIS7_tdata, // M-MDC
    input wire                             S_AXIS7_tvalid,
    input wire [SAXIS_78_TDATA_WIDTH-1:0]  S_AXIS8_tdata, // ---
    input wire                             S_AXIS8_tvalid,

    input wire signed [SAXIS_3_DATA_WIDTH-1:0] axis3_center,  
    input wire [8-1:0]   rp_digital_in,
    input wire [32-1:0]  operation, // 0..7 control bits 0: 1=start 1: 1=single shot/0=fifo loop 2, 4: 16=init/reset , [31:8] DATA ACCUMULATOR (64) SHR
    input wire [32-1:0]  ndecimate,
    input wire [32-1:0]  nsamples,
    input wire [32-1:0]  channel_selector,
    input wire           ext_trigger, // =0 for free run, auto trigger, 1= to hold trigger (until next pix, etc.)
    output wire          finished_state,
    output wire          init_state,
    output wire [32-1:0] writeposition,
    output wire [32-1:0] debug,
    
    output wire [64-1:0]  M_AXIS_aux_tdata,
    output wire           M_AXIS_aux_tvalid,

    // DOWN SAMPLED CH1-4
    output wire [MAXIS_TDATA_WIDTH-1:0]  M_AXIS_CH1_tdata,
    output wire                          M_AXIS_CH1_tvalid,
    output wire [MAXIS_TDATA_WIDTH-1:0]  M_AXIS_CH2_tdata,
    output wire                          M_AXIS_CH2_tvalid,
    output wire [MAXIS_TDATA_WIDTH-1:0]  M_AXIS_CH3_tdata,
    output wire                          M_AXIS_CH3_tvalid,
    output wire [MAXIS_TDATA_WIDTH-1:0]  M_AXIS_CH4_tdata,
    output wire                          M_AXIS_CH4_tvalid,
    output wire [MAXIS_TDATA_WIDTH-1:0]  M_AXIS_AMPL_tdata,
    output wire                          M_AXIS_AMPL_tvalid,
    output wire [MAXIS_TDATA_WIDTH-1:0]  M_AXIS_PHASE_tdata,
    output wire                          M_AXIS_PHASE_tvalid,
    output wire [MAXIS_TDATA_WIDTH-1:0]  M_AXIS_FCENTER_tdata,
    output wire                          M_AXIS_FCENTER_tvalid,
    output wire [MAXIS_TDATA_WIDTH-1:0]  M_AXIS_FREQ31_0_tdata,
    output wire                          M_AXIS_FREQ31_0_tvalid,
    output wire [MAXIS_TDATA_WIDTH-1:0]  M_AXIS_FREQ63_32_tdata,
    output wire                          M_AXIS_FREQ63_32_tvalid,

    // BRAM PUSH INTERFACE
    output wire [64-1:0] BR_ch1s,
    output wire [64-1:0] BR_ch2s,
    output wire BR_next,
    output wire BR_reset,
    input  wire BR_ready
    );
    
    reg [2:0] dec_sms=3'd0;
    reg [2:0] dec_sms_next=3'd0;

    reg [32-1:0] decimate_count=0, decimate_count_next=0; 
    reg [32-1:0] sample_count=0, sample_count_next=0;

    reg signed [64-1:0] ch1, ch2, ch3, ch4;
    reg signed [64-1:0] ch1n, ch2n, ch3n, ch4n;
    reg signed [64-1:0] ch1s, ch2s, ch3s, ch4s;
    reg signed [64-1:0] delta_freq;

    reg finished=1'b0;
    reg finished_next=1'b0;
    reg running=1'b0; 
    reg running_next=1'b0; 
    reg trigger=1'b0;
    reg trigger_next=1'b0;
    
    reg bram_reset=1;
    reg bram_next=0;
    
    reg mk3_pixel_clock=1'b0;
    reg mk3_pixel_clock_next=1'b0;
    reg mk3_pixel_clock_last=1'b0;
    reg mk3_line_clock=1'b0;
    reg mk3_line_clock_next=1'b0;
    reg mk3_line_clock_last=1'b0;

    assign init_state = operation[0];
    assign finished_state = finished;
    //assign writeposition = { decimate_count[15:0], 1'b0, 1'b0, bram_addr[13:0] };
    //assign debug = { {(2){1'b0}}, operation[12:8],  operation[7:0],     {(5){1'b0}}, finished_state,trigger,running,   {(1){1'b0}}, bramwr_sms, {(1){1'b0}}, dec_sms };

    // pass decimated ch2s out for auxillary use    
    assign M_AXIS_aux_tdata  = ch2s[64-1:0]; // Testing
    assign M_AXIS_aux_tvalid = running;
    
    // BRAM INTERFACE CONNECT
    assign BR_next  = bram_next;       
    assign BR_reset = bram_reset;       
    assign BR_ch1s  = ch1s;
    assign BR_ch2s  = ch2s;

    always @(posedge a_clk)
    begin
        finished <= finished_next;
        running  <= running_next;
               
        if (ext_trigger)
        begin
            trigger <= trigger_next;
        end
        
        dec_sms <= dec_sms_next;
        
        ch1 <= ch1n;
        ch2 <= ch2n;
        ch3 <= ch3n;
        ch4 <= ch4n;
        delta_freq <= $signed(S_AXIS3_tdata[SAXIS_3_DATA_WIDTH-1:0]) - $signed(axis3_center);
        //    ch2n <= $signed(S_AXIS3_tdata[SAXIS_3_DATA_WIDTH-1:0]) - $signed(axis3_center); // Freq (48) - Center (48) =>  64 sum

        decimate_count <= decimate_count_next; 
        sample_count <= sample_count_next;

        // detect MK3 pixel sync clock
        mk3_pixel_clock <= mk3_pixel_clock_next;
        if (rp_digital_in[0:0] != mk3_pixel_clock_last) // MK3 pixel clock
        begin
            mk3_pixel_clock_last <= rp_digital_in[0:0];
            mk3_pixel_clock_next <= 1'b1;      
        end
        else
        begin
            mk3_pixel_clock_next <= 1'b0;      
        end
        // MK3 line sync clock
        mk3_line_clock  <= mk3_line_clock_next;
        if (rp_digital_in[1:1] != mk3_line_clock_last) // MK3 line clock
        begin
            mk3_line_clock_last <= rp_digital_in[1:1];
            mk3_line_clock_next <= 1'b1;      
        end else begin
            mk3_line_clock_next <= 1'b0;      
        end

        if (operation[4]) // reset
        begin
            bram_reset    <= 1'b1;
            dec_sms_next  <= 3'd0;
            running_next  <= 1'b0;
            trigger_next  <= 1'b0;
            finished_next <= 1'b0;
        end else begin
            if (operation[0] && ~running)
            begin
                running_next <= 1'b1;
                trigger_next <= 1'b1;
            end

            // SERVE BRAM STORE MACHINE INTERFACE
            if (BR_ready)
            begin
                sample_count_next <= sample_count + 1;
                if (sample_count >= nsamples) // run mode 1 single shot, finish
                begin
                    dec_sms_next <= 3'd0;  // finished, needs reset to restart and re-arm.
                    if (operation[1]) // run mode 1 single shot, finish. Else start over (LOOP/FIFO mode)
                    begin
                        finished_next <= 1'b1; // set finish flag
                    end else begin
                        trigger_next <= 1'b1; // set trigger to start over right away
                    end
                end
            end

            
            // ===============================================================================
            // DECIMATING MACHINE
            case(dec_sms)
                0:    // Begin state, arm
                begin
                    sample_count_next <= 32'd0;
                    dec_sms_next <= 3'd1; // ready, now wait for start trigger, arming
                end
                1:    // Wait for trigger
                begin
                    if (trigger)
                    begin
                        bram_reset <= 1'b0; // take BRAM controller out of reset
                        bram_next  <= 0; // hold BRAM write next
                        finished_next <= 1'b0; // clear finish flag
                        dec_sms_next <= 3'd2;  // go
                        trigger_next <= 1'b0;
                    end
                end
                2:    // Initiate Measure, Average, Decimate, ...
                begin   
                    bram_next <= 0; // hold BRAM write next
                    decimate_count_next <= 32'd1; // first sample
                    case (channel_selector[7:0]) // channels selector
                        0: // S_AXIS1: 32: {16: ADC-IN1, 16: ADC-IN2 }
                        begin
                            if (S_AXIS1_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS1_tdata[15:0]);          // IN1 (16bit data, 14bit ADC) => 32
                                ch2n <= $signed(S_AXIS1_tdata[16+15:16+0]);    // IN2 (16bit data, 14bit ADC) => 32
                                dec_sms_next <= 3'd3;
                            end
                        end
                        1:
                        begin
                            if (S_AXIS4_tvalid && S_AXIS5_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS4_tdata[SAXIS_4_DATA_WIDTH-1:0]); // Phase (24) =>  32
                                ch2n <= $signed(S_AXIS5_tdata[SAXIS_5_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                dec_sms_next <= 3'd3;
                            end
                        end
                        2:
                        begin
                            if (S_AXIS6_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS6_tdata[15:0]);                    // IN1 AC Signal 16bit
                                ch2n <= $signed(S_AXIS6_tdata[31:16]);                   // M-DC
                                dec_sms_next <= 3'd3;
                            end
                        end
//                        2:
//                        begin
//                            if (S_AXIS1_tvalid && S_AXIS5_tvalid)
//                            begin
//                                ch1n <= $signed(S_AXIS1_tdata[15:0]);                    // IN1 Signal with
//                                ch2n <= $signed(S_AXIS5_tdata[SAXIS_5_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
//                                dec_sms_next <= 3'd3;
//                            end
//                        end
//                        3:
//                        begin
//                            if (S_AXIS1_tvalid && S_AXIS4_tvalid)
//                            begin
//                                ch1n <= $signed(S_AXIS1_tdata[15:0]);                    // IN1 Signal with
//                                ch2n <= $signed(S_AXIS4_tdata[SAXIS_4_DATA_WIDTH-1:0]); // Phase (24) =>  32
//                                dec_sms_next <= 3'd3;
//                            end
//                        end
                        4:
                        begin
                            if (S_AXIS2_tvalid && S_AXIS3_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS2_tdata[SAXIS_2_DATA_WIDTH-1:0]);                 // Amplitude Exec (32) =>  64 sum
                                ch2n <= delta_freq; // Freq (48) - Center (48) =>  64 sum
                                dec_sms_next <= 3'd3;
                            end
                        end
                        5:
                        begin
                            if (S_AXIS5_tvalid && S_AXIS2_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS5_tdata[SAXIS_5_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                ch2n <= $signed(S_AXIS2_tdata[SAXIS_2_DATA_WIDTH-1:0]);  // Amplitude Exec (32) =>  64 sum
//                            if (S_AXIS7_tvalid && S_AXIS8_tvalid)
//                                ch1n <= S_AXIS7_tdata[SAXIS_78_DATA_WIDTH-1:0]; // AX7
//                                ch2n <= S_AXIS8_tdata[SAXIS_78_DATA_WIDTH-1:0]; // AX8
                                dec_sms_next <= 3'd3;
                            end
                        end
                        6:
                        begin
                            if (S_AXIS4_tvalid && S_AXIS3_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS4_tdata[SAXIS_4_DATA_WIDTH-1:0]); // Phase (24) =>  32
                                ch2n <= delta_freq; // Freq (48) - Center (48) =>  64 sum
//                            ch1n <= ch1 + mk3_pixel_clock; // keep counting! SET SHR to 0!!!
//                            ch2n <= ch1 + mk3_line_clock;
                                ch3n <= $signed(S_AXIS5_tdata[SAXIS_5_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                ch4n <= $signed(S_AXIS2_tdata[SAXIS_2_DATA_WIDTH-1:0]);  // Amplitude Exec (32) =>  64 sum
                                dec_sms_next <= 3'd3;
                            end
                        end
                        //7:
                        //begin
                        //    if (S_AXIS7_tvalid && S_AXIS5_tvalid)
                        //    begin
                        //        ch1n <= $signed(S_AXIS7_tdata[SAXIS_78_DATA_WIDTH-1:0]); // M-MDC (32)
                        //        ch2n <= $signed(S_AXIS5_tdata[SAXIS_5_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                        //        dec_sms_next <= 3'd3;
                        //    end
                        //end
                        //8:
                        //begin
                        //    ch1n <= {rp_digital_in[7:0], rp_digital_in[7:0], rp_digital_in[7:0], rp_digital_in[7:0]}; // rp_digital_in; SET SHR to 0 or find it shifted!!!
                        //    ch2n <= mk3_pixel_clock;
                        //    dec_sms_next <= 3'd3;
                        //end
                    endcase
                end
                3:    // Measure, Average, Decimate, ...
                begin   
                    case (channel_selector[7:0]) // channel selector
                        0: // S_AXIS1: 32: {16: ADC-IN1, 16: ADC-IN2 }
                        begin
                            if (S_AXIS1_tvalid)
                            begin
                                ch1n <= ch1 + $signed(S_AXIS1_tdata[15:0]); // IN1
                                ch2n <= ch2 + $signed(S_AXIS1_tdata[16+15:16+0]); // IN2
                                decimate_count_next <= decimate_count + 1; // next sample
                            end
                        end
                        1:
                        begin
                            if (S_AXIS4_tvalid && S_AXIS5_tvalid)
                            begin
                                ch1n <= ch1 + $signed(S_AXIS4_tdata[SAXIS_4_DATA_WIDTH-1:0]); // Phase (24) =>  32
                                ch2n <= ch2 + $signed(S_AXIS5_tdata[SAXIS_5_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                decimate_count_next <= decimate_count + 1; // next sample
                            end
                        end
                        2:
                        begin
                            if (S_AXIS6_tvalid)
                            begin
                                ch1n <= ch1 + $signed(S_AXIS6_tdata[15:0]); // IN1 AC
                                ch2n <= ch2 + $signed(S_AXIS6_tdata[31:16]); // Amplitude (24) =>  32
                            end
                            decimate_count_next <= decimate_count + 1; // next sample
                        end
//                        2:
//                        begin
//                            if (S_AXIS1_tvalid && S_AXIS5_tvalid)
//                            begin
//                                ch1n <= ch1 + $signed(S_AXIS1_tdata[15:0]); // IN1
//                                ch2n <= ch2 + $signed(S_AXIS5_tdata[SAXIS_5_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
//                            end
//                            decimate_count_next <= decimate_count + 1; // next sample
//                        end
//                        3:
//                        begin
//                            if (S_AXIS1_tvalid && S_AXIS4_tvalid)
//                            begin
//                                ch1n <= ch1 + $signed(S_AXIS1_tdata[15:0]); // IN1
//                                ch2n <= ch2 + $signed(S_AXIS4_tdata[SAXIS_4_DATA_WIDTH-1:0]); // Phase (24) =>  32
//                            end
//                            decimate_count_next <= decimate_count + 1; // next sample
//                        end
                        4:
                        begin
                            if (S_AXIS2_tvalid && S_AXIS3_tvalid)
                            begin
                                ch1n <= ch1 + $signed(S_AXIS2_tdata[SAXIS_2_DATA_WIDTH-1:0]); // Amplitude Exec (32) =>  64 sum
                                ch2n <= ch2 + delta_freq; // Freq (48) - Lower (48) =>  64 sum
                                decimate_count_next <= decimate_count + 1; // next sample
                            end
                        end
                        5:
                        begin
                            if (S_AXIS5_tvalid && S_AXIS2_tvalid)
                            begin
                                ch1n <= ch1 + $signed(S_AXIS5_tdata[SAXIS_5_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                ch2n <= ch2 + $signed(S_AXIS2_tdata[SAXIS_2_DATA_WIDTH-1:0]);  // Amplitude Exec (32) =>  64 sum
                                decimate_count_next <= decimate_count + 1; // next sample
                            end
                        end
                        6:
                        begin
                            if (S_AXIS4_tvalid && S_AXIS3_tvalid)
                            begin
                                ch1n <= ch1 + $signed(S_AXIS4_tdata[SAXIS_4_DATA_WIDTH-1:0]); // Phase (24) =>  32
                                ch2n <= ch2 + delta_freq; // Freq (48) - Center (48) =>  64 sum
                                ch3n <= ch3 + $signed(S_AXIS5_tdata[SAXIS_5_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                ch4n <= ch4 + $signed(S_AXIS2_tdata[SAXIS_2_DATA_WIDTH-1:0]);  // Amplitude Exec (32) =>  64 sum
                                decimate_count_next <= decimate_count + 1; // next sample
                            end
                        end
                        //7:
                        //begin
                        //    if (S_AXIS7_tvalid && S_AXIS5_tvalid)
                        //    begin
                        //        ch1n <= ch1 + $signed(S_AXIS7_tdata[SAXIS_78_DATA_WIDTH-1:0]); // M-MDC (32)
                        //        ch2n <= ch2 + $signed(S_AXIS5_tdata[SAXIS_5_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                        //        decimate_count_next <= decimate_count + 1; // next sample
                        //    end
                        //end
                        //8:
                        //begin
                        //    decimate_count_next <= decimate_count + 1; // next sample
                        //end
                    endcase
    
                    if (decimate_count >= ndecimate && BR_ready) // BRAM write cycle must be comleted
                    begin
                        ch1s <= (ch1 >>> operation[31:8]);
                        ch2s <= (ch2 >>> operation[31:8]);
                        ch3s <= (ch3 >>> operation[31:8]);
                        ch4s <= (ch4 >>> operation[31:8]);
                        bram_next <= 1; // initate write data cycles
                        dec_sms_next <= 3'd2; // start over decimating with next value(s)
                    end
                end
            endcase
        end
    end
    
    // assign CH1..4 from box carr filtering as selected
    assign M_AXIS_CH1_tdata = ch1s[31:0];
    assign M_AXIS_CH1_tvalid = 1;
    assign M_AXIS_CH2_tdata = ch2s[31:0];
    assign M_AXIS_CH2_tvalid = 1;
    assign M_AXIS_CH3_tdata = ch3s[31:0];
    assign M_AXIS_CH3_tvalid = 1;
    assign M_AXIS_CH4_tdata = ch4s[31:0];
    assign M_AXIS_CH4_tvalid = 1;

    // assign raw unfiltered amplitude and phase
    assign M_AXIS_AMPL_tdata   = $signed(S_AXIS5_tdata[SAXIS_5_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
    assign M_AXIS_AMPL_tvalid  = 1;
    assign M_AXIS_PHASE_tdata  = $signed(S_AXIS4_tdata[SAXIS_4_DATA_WIDTH-1:0]); // Phase (24) =>  32
    assign M_AXIS_PHASE_tvalid = 1;

    // assign F CENTER used for d-freq signal, upper 32 bit from 48 (SAXIS_3_TDATA_WIDTH)
    assign M_AXIS_FCENTER_tdata = axis3_center[SAXIS_3_TDATA_WIDTH-1:SAXIS_3_TDATA_WIDTH-32];
    assign M_AXIS_FCENTER_tvalid = 1;
    
    // assign unfiltered absolute frequency
    assign M_AXIS_FREQ31_0_tdata = S_AXIS3_tdata[31:0];
    assign M_AXIS_FREQ31_0_tvalid = 1;
    assign M_AXIS_FREQ63_32_tdata = {{(64-SAXIS_3_TDATA_WIDTH){1'b0}}, S_AXIS3_tdata[SAXIS_3_TDATA_WIDTH-1:32]};
    assign M_AXIS_FREQ63_32_tvalid = 1;

    
endmodule
