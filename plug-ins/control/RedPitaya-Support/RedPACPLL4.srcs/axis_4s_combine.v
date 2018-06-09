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
    parameter SAXIS_45_DATA_WIDTH  = 24,
    parameter SAXIS_45_TDATA_WIDTH = 24,
    parameter SAXIS_6_DATA_WIDTH   = 32,
    parameter SAXIS_6_TDATA_WIDTH  = 32,
    parameter SAXIS_78_DATA_WIDTH  = 32,
    parameter SAXIS_78_TDATA_WIDTH = 32,
    parameter integer BRAM_DATA_WIDTH = 64,
    parameter integer BRAM_ADDR_WIDTH = 15,
    parameter integer DECIMATED_WIDTH = 32
)
(
    input a_clk,
    // input a_resetn,
    
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_1_TDATA_WIDTH-1:0]  S_AXIS1_tdata,
    input wire                          S_AXIS1_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_2_TDATA_WIDTH-1:0]  S_AXIS2_tdata,
    input wire                          S_AXIS2_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_3_TDATA_WIDTH-1:0]  S_AXIS3_tdata,
    input wire                            S_AXIS3_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_45_TDATA_WIDTH-1:0]  S_AXIS4_tdata,
    input wire                          S_AXIS4_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_45_TDATA_WIDTH-1:0]  S_AXIS5_tdata,
    input wire                          S_AXIS5_tvalid,
    // (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    // input wire [SAXIS_6_TDATA_WIDTH-1:0]  S_AXIS6_tdata,
    // input wire                          S_AXIS6_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_78_TDATA_WIDTH-1:0]  S_AXIS7_tdata, // M-MDC
    input wire                          S_AXIS7_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_78_TDATA_WIDTH-1:0]  S_AXIS8_tdata, // ---
    input wire                          S_AXIS8_tvalid,
    input wire signed [SAXIS_3_DATA_WIDTH-1:0] axis3_center,  
    input wire [8-1:0]   rp_digital_in,
    input wire [32-1:0]  operation, // 0..7 control bits 0: 1=start 1: 1=single shot/0=fifo loop 2, 4: 16=init/reset , [31:8] DATA ACCUMULATOR (64) SHR
    input wire [32-1:0]  ndecimate,
    input wire [32-1:0]  nsamples,
    input wire [32-1:0]  channel_selector,
    output wire          finished_state,
    output wire          init_state,
    output wire [32-1:0] writeposition,
    output wire [32-1:0] debug,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    output wire [64-1:0]  M_AXIS_aux_tdata,
    output wire           M_AXIS_aux_tvalid,


    // BRAM PORT A
    output wire                        bram_porta_clk,
    output wire [BRAM_ADDR_WIDTH-1:0]  bram_porta_addr,
    output wire [BRAM_DATA_WIDTH-1:0]  bram_porta_wrdata,
    // input  wire [BRAM_DATA_WIDTH-1:0]  bram_porta_rddata,
    // output wire                        bram_porta_rst,
    output wire                        bram_porta_en,
    output wire                        bram_porta_we
    );
    
    reg [2:0] dec_sms=3'd0;
    reg [2:0] dec_sms_next=3'd0;
    reg [2:0] bramwr_sms=3'd0;
    reg [2:0] bramwr_sms_next=1'b0;
    reg bram_wren, bram_wren_next;
    reg [BRAM_ADDR_WIDTH-1:0] bram_addr=0, bram_addr_next=0;
    reg [BRAM_DATA_WIDTH-1:0] bram_data, bram_data_next;

    reg [32-1:0] decimate_count, decimate_count_next; 
    reg [32-1:0] sample_count, sample_count_next;

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
    
    reg [BRAM_DATA_WIDTH-1: 0] wr_data;    

    reg mk3_pixel_clock=1'b0;
    reg mk3_pixel_clock_next=1'b0;
    reg mk3_pixel_clock_last=1'b0;
    reg mk3_line_clock=1'b0;
    reg mk3_line_clock_next=1'b0;
    reg mk3_line_clock_last=1'b0;

    assign init_state = operation[0];
    assign finished_state = finished;
    assign writeposition = { decimate_count[15:0], 1'b0, 1'b0, bram_addr[13:0] };
    assign debug = { {(2){1'b0}}, operation[12:8],  operation[7:0],     {(5){1'b0}}, finished_state,trigger,running,   {(1){1'b0}}, bramwr_sms, {(1){1'b0}}, dec_sms };

    // pass decimated ch2s out for auxillary use    
    assign M_AXIS_aux_tdata  = ch2s[64-1:0]; // Testing
    assign M_AXIS_aux_tvalid = running;
    
    assign bram_porta_clk = a_clk;
    // assign bram_porta_rst = ~a_resetn;
    assign bram_porta_we = bram_wren;
    assign bram_porta_en = bram_wren;
    assign bram_porta_addr = bram_addr;
    assign bram_porta_wrdata = bram_data;
        
    always @(posedge a_clk)
    begin
        finished <= finished_next;
        running  <= running_next;
        trigger  <= trigger_next;
        
        dec_sms <= dec_sms_next;
        bramwr_sms <= bramwr_sms_next;
        bram_addr <= bram_addr_next;
        bram_data <= bram_data_next;
        bram_wren <= bram_wren_next;
        
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
            dec_sms_next <= 3'd0;
            bramwr_sms_next <= 3'd0;
            bram_wren_next  <= 1'b0;
            running_next  <= 1'b0;
            trigger_next  <= 1'b0;
            finished_next <= 1'b0;
        end else begin
            if (operation[0] && ~running)
            begin
                running_next <= 1'b1;
                trigger_next <= 1'b1;
            end

            // BRAM STORE MACHINE
            case(bramwr_sms)
                0:    // Begin state
                begin
                    ; // idle
                end
                1:    // Store CH1
                begin
//                    bram_data_next  <= {ch1s[63], ch1s[BRAM_DATA_WIDTH-2:0]};
                    bram_data_next  <= ch1s;
                    bramwr_sms_next <= 3'd2;
                end
                2:    // Write
                begin
                    bram_wren_next  <= 1'b1;
                    bramwr_sms_next <= 3'd3;
                end
                3:    // Store CH2
                begin
                    bram_wren_next  <= 1'b0;
                    bram_addr_next <= bram_addr + 1;
//                    bram_data_next  <= {ch2s[63], ch2s[BRAM_DATA_WIDTH-2:0]};
                    bram_data_next  <= ch2s;
                    bramwr_sms_next <= 3'd4;
                end
                4:    // Write
                begin
                    bram_wren_next  <= 1'b1;
                    bramwr_sms_next <= 3'd5;
                end
                5:    // Store completed
                begin
                    bram_addr_next <= bram_addr + 1;
                    bram_wren_next  <= 2'd0;

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
                    bramwr_sms_next <= 3'd0; // idle next
                end
            endcase
            // ===============================================================================
            // DECIMATING MACHINE
            case(dec_sms)
                0:    // Begin state, arm
                begin
                    bram_addr_next <= {(BRAM_ADDR_WIDTH){1'b0}};
                    sample_count_next <= 32'd0;
                    dec_sms_next <= 3'd1; // ready, now wait for start trigger, arming
                end
                1:    // Wait for trigger
                begin
                    if (trigger)
                    begin
                        finished_next <= 1'b0; // clear finish flag
                        dec_sms_next <= 3'd2;  // go
                        trigger_next <= 1'b0;
                    end
                end
                2:    // Initiate Measure, Average, Decimate, ...
                begin   
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
                                ch1n <= $signed(S_AXIS4_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Phase (24) =>  32
                                ch2n <= $signed(S_AXIS5_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                dec_sms_next <= 3'd3;
                            end
                        end
                        2:
                        begin
                            if (S_AXIS1_tvalid && S_AXIS5_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS1_tdata[15:0]);                    // IN1 Signal with
                                ch2n <= $signed(S_AXIS5_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                dec_sms_next <= 3'd3;
                            end
                        end
                        3:
                        begin
                            if (S_AXIS1_tvalid && S_AXIS4_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS1_tdata[15:0]);                    // IN1 Signal with
                                ch2n <= $signed(S_AXIS4_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Phase (24) =>  32
                                dec_sms_next <= 3'd3;
                            end
                        end
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
                                ch1n <= $signed(S_AXIS5_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
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
                                ch1n <= $signed(S_AXIS4_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Phase (24) =>  32
                                ch2n <= delta_freq; // Freq (48) - Center (48) =>  64 sum
//                            ch1n <= ch1 + mk3_pixel_clock; // keep counting! SET SHR to 0!!!
//                            ch2n <= ch1 + mk3_line_clock;
                                dec_sms_next <= 3'd3;
                            end
                        end
                        7:
                        begin
                            if (S_AXIS7_tvalid && S_AXIS5_tvalid)
                            begin
                                ch1n <= $signed(S_AXIS7_tdata[SAXIS_78_DATA_WIDTH-1:0]); // M-MDC (32)
                                ch2n <= $signed(S_AXIS5_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                dec_sms_next <= 3'd3;
                            end
                        end
                        8:
                        begin
                            ch1n <= {{(64-8){1'b0}}, rp_digital_in[7:0]}; // rp_digital_in; SET SHR to 0!!!
                            ch2n <= mk3_pixel_clock;
                            dec_sms_next <= 3'd3;
                        end
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
                                ch1n <= ch1 + $signed(S_AXIS4_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Phase (24) =>  32
                                ch2n <= ch2 + $signed(S_AXIS5_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                decimate_count_next <= decimate_count + 1; // next sample
                            end
                        end
                        2:
                        begin
                            if (S_AXIS1_tvalid && S_AXIS5_tvalid)
                            begin
                                ch1n <= ch1 + $signed(S_AXIS1_tdata[15:0]); // IN1
                                ch2n <= ch2 + $signed(S_AXIS5_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                            end
                            decimate_count_next <= decimate_count + 1; // next sample
                        end
                        3:
                        begin
                            if (S_AXIS1_tvalid && S_AXIS4_tvalid)
                            begin
                                ch1n <= ch1 + $signed(S_AXIS1_tdata[15:0]); // IN1
                                ch2n <= ch2 + $signed(S_AXIS4_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Phase (24) =>  32
                            end
                            decimate_count_next <= decimate_count + 1; // next sample
                        end
                        4:
                        begin
                            if (S_AXIS2_tvalid && S_AXIS3_tvalid)
                            begin
                                ch1n <= ch1 + $signed(S_AXIS2_tdata[SAXIS_2_DATA_WIDTH-1:0]); // Amplitude (32) =>  64 sum
                                ch2n <= ch2 + delta_freq; // Freq (48) - Lower (48) =>  64 sum
                                decimate_count_next <= decimate_count + 1; // next sample
                            end
                        end
                        5:
                        begin
                            if (S_AXIS5_tvalid && S_AXIS2_tvalid)
                            begin
                                ch1n <= ch1 + $signed(S_AXIS5_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                ch2n <= ch2 + $signed(S_AXIS2_tdata[SAXIS_2_DATA_WIDTH-1:0]);  // Amplitude Exec (32) =>  64 sum
                                decimate_count_next <= decimate_count + 1; // next sample
                            end
                        end
                        6:
                        begin
                            if (S_AXIS4_tvalid && S_AXIS3_tvalid)
                            begin
                                ch1n <= ch1 +  $signed(S_AXIS4_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Phase (24) =>  32
                                ch2n <= ch2 + delta_freq; // Freq (48) - Center (48) =>  64 sum
                                decimate_count_next <= decimate_count + 1; // next sample
                            end
                        end
                        7:
                        begin
                            if (S_AXIS7_tvalid && S_AXIS5_tvalid)
                            begin
                                ch1n <= ch1 + $signed(S_AXIS7_tdata[SAXIS_78_DATA_WIDTH-1:0]); // M-MDC (32)
                                ch2n <= ch2 + $signed(S_AXIS5_tdata[SAXIS_45_DATA_WIDTH-1:0]); // Amplitude (24) =>  32
                                decimate_count_next <= decimate_count + 1; // next sample
                            end
                        end
                        8:
                        begin
                            decimate_count_next <= decimate_count + 1; // next sample
                        end
                    endcase
    
                    if (decimate_count >= ndecimate && bramwr_sms_next == 0) // BRAM write cycle must be comleted
                    begin
                        ch1s <= (ch1 >>> operation[31:8]);
                        ch2s <= (ch2 >>> operation[31:8]);
                        ch3s <= (ch3 >>> operation[31:8]);
                        ch4s <= (ch4 >>> operation[31:8]);
                        dec_sms_next <= 2'd2; // start over decimating with next value(s)
                        bramwr_sms_next <= 3'd1; // initate write data cycles
                    end
                end
            endcase
        end
    end
endmodule
