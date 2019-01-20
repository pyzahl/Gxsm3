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
// Module Name: McBSP_controller
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
 */

//////////////////////////////////////////////////////////////////////////////////


module McBSP_controller #(
    parameter WORDS_PER_FRAME = 8,
    parameter BITS_PER_WORD = 32,
    parameter SAXIS_TDATA_WIDTH = 32
)
(
    input a_clk,
    input mcbsp_clk,
    input mcbsp_frame_start,
    input mcbsp_data_rx,
    input mcbsp_data_nrx, // N-th RedPitaya Slave/Chained Link forwarding to McBSP Maser Slave -- optional
    output mcbsp_data_tx,
    output mcbsp_data_fsx, // optional/debug FSX start
    output mcbsp_data_frm, // optional/debug Frame
    // input a_resetn,
    
    output wire trigger,
    
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_TDATA_WIDTH-1:0]  S_AXIS1_tdata,
    input wire                          S_AXIS1_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_TDATA_WIDTH-1:0]  S_AXIS2_tdata,
    input wire                          S_AXIS2_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_TDATA_WIDTH-1:0]  S_AXIS3_tdata,
    input wire                          S_AXIS3_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_TDATA_WIDTH-1:0]  S_AXIS4_tdata,
    input wire                          S_AXIS4_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_TDATA_WIDTH-1:0]  S_AXIS5_tdata,
    input wire                          S_AXIS5_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_TDATA_WIDTH-1:0]  S_AXIS6_tdata,
    input wire                          S_AXIS6_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_TDATA_WIDTH-1:0]  S_AXIS7_tdata,
    input wire                          S_AXIS7_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_TDATA_WIDTH-1:0]  S_AXIS8_tdata,
    input wire                          S_AXIS8_tvalid

    );
    
    reg frame_start=0;
    reg [10-1:0] frame_bit_counter;
    reg [(WORDS_PER_FRAME * BITS_PER_WORD) -1 : 0] data;
    reg [(WORDS_PER_FRAME * BITS_PER_WORD) -1 : 0] data_in;
    reg [(WORDS_PER_FRAME * BITS_PER_WORD) -1 : 0] data_read;
    reg tx=0;

    reg rtrigger=1;

    always @(negedge mcbsp_clk) // read edge
    begin
        // Detect Frame Sync
        if (mcbsp_frame_start && ~frame_start)
        begin
            rtrigger <= 1;
            frame_start <= 1;
            frame_bit_counter <= 255; // ((WORDS_PER_FRAME * BITS_PER_WORD) - 1);
            // wait for aclk edge (very fast vs. mcbsp_clk)
            //@(posedge a_clk) // dose not synth.
            // Latch Axis Data at Frame Sync Pulse and initial data serialization
            data[(8 * BITS_PER_WORD) - 1 : (8 * BITS_PER_WORD)-32] <= S_AXIS1_tdata;
            data[(7 * BITS_PER_WORD) - 1 : (7 * BITS_PER_WORD)-32] <= S_AXIS2_tdata;
            data[(6 * BITS_PER_WORD) - 1 : (6 * BITS_PER_WORD)-32] <= S_AXIS3_tdata;
            data[(5 * BITS_PER_WORD) - 1 : (5 * BITS_PER_WORD)-32] <= S_AXIS4_tdata;
            data[(4 * BITS_PER_WORD) - 1 : (4 * BITS_PER_WORD)-32] <= S_AXIS5_tdata;
            data[(3 * BITS_PER_WORD) - 1 : (3 * BITS_PER_WORD)-32] <= S_AXIS6_tdata;
            data[(2 * BITS_PER_WORD) - 1 : (2 * BITS_PER_WORD)-32] <= S_AXIS7_tdata;
            // data[(1 * BITS_PER_WORD) - 1 : (1 * BITS_PER_WORD)-32] <= S_AXIS8_tdata;
            data[(1 * BITS_PER_WORD) - 1 : (1 * BITS_PER_WORD)-32] <= data_read[(8 * BITS_PER_WORD) - 1 : (8 * BITS_PER_WORD)-32]; // LOOP BACK TEST, index: col,row
        end else
        begin
            if (frame_start)
            begin
                rtrigger <= 0;
                // read data bit
                data_in[frame_bit_counter] <= mcbsp_data_rx;
        
                // completed?
                if (frame_bit_counter == 0)
                begin
                    frame_start <= 0;
                end else
                begin
                    // next bit
                    frame_bit_counter <= frame_bit_counter - 1;
                end
            end else
            begin
                data_read <= data_in; // buffer read data
            end   
        end 
    end

    always @(posedge mcbsp_clk) // setup edge
    begin
        if (frame_start)
        begin
            // setup data bit
            tx <= data[frame_bit_counter];
        end
    end

    assign mcbsp_data_tx = tx;
    assign trigger = rtrigger;

    assign mcbsp_data_frm = frame_start;
    assign mcbsp_data_fsx = rtrigger;


endmodule
