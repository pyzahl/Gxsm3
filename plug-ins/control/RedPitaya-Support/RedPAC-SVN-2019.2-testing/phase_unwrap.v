`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
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
// 
// Create Date: 11/26/2017 09:10:43 PM
// Design Name: 
// Module Name: lms_phase_amplitude_detector
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
//////////////////////////////////////////////////////////////////////////////////


module phase_unwrap #(
    parameter S_AXIS_TDATA_WIDTH = 24, // INPUT AXIS DATA WIDTH  3Q21
    parameter M_AXIS_TDATA_WIDTH = 32  // OUTPUT AXIS DATA WIDTH 11Q21
)
(
    // (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_CLKEN aclk" *)
    input aclk,

    input wire [S_AXIS_TDATA_WIDTH-1:0]  S_AXIS_tdata,
    input wire                           S_AXIS_tvalid,

    input enable,

    output wire [M_AXIS_TDATA_WIDTH-1:0] M_AXIS_tdata,
    output wire                          M_AXIS_tvalid
    );
    
    reg signed [S_AXIS_TDATA_WIDTH-1:0] p0=0;
    reg signed [S_AXIS_TDATA_WIDTH-1:0] p1=0;
    reg signed [S_AXIS_TDATA_WIDTH-1:0] p2=0;
    reg signed [S_AXIS_TDATA_WIDTH:0] dp01=0;
    reg signed [M_AXIS_TDATA_WIDTH-1:0] phase_wrap=0;
    reg signed [M_AXIS_TDATA_WIDTH-1:0] phase_unwrapped=0;

    always @ (posedge aclk)
    begin
        p0 <= $signed (S_AXIS_tdata);
        p1 <= p0;
        p2 <= p1;
        
        dp01 <= p0-p1;
        
        if (enable)
        begin
            if (dp01 > 25'sd6588397)                      // > pi : (1<<21)*pi = 6588397.3
            begin
                phase_wrap <= phase_wrap - 25'sd13176795; // 2pi : (1<<21)*pi = 13176794.6
            end
            if (dp01 < -25'sd6588397)                     // > pi  (1<<21)*pi = 6588397.3
            begin
                phase_wrap <= phase_wrap + 25'sd13176795;
            end
        end
        else
        begin
            phase_wrap <= 0;
        end

        phase_unwrapped <= p2 + phase_wrap; 
        
    end
    
    assign M_AXIS_tdata   = phase_unwrapped;
    assign M_AXIS_tvalid  = S_AXIS_tvalid;

endmodule
