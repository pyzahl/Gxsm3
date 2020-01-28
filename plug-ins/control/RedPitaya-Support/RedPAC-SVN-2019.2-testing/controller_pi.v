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


module controller_pi #(
    parameter AXIS_TDATA_WIDTH = 32, // INPUT AXIS DATA WIDTH
    parameter M_AXIS_CONTROL_TDATA_WIDTH = 32, // SERVO CONTROL DATA WIDTH OF AXIS
    parameter CONTROL_WIDTH = 32, // SERVO CONTROL DATA WIDTH
    parameter M_AXIS_CONTROL2_TDATA_WIDTH = 32, // INTERNAL CONTROl DATA WIDTH MAPPED TO AXIS FOR READOUT not including extend
    parameter CONTROL2_WIDTH = 50, //64, // INTERNAL CONTROl DATA WIDTH not including extend **** COEFQ+AXIS_TDATA_WIDTH == CONTROL2_WIDTH
    parameter CONTROL2_OUT_WIDTH = 32, // max passed outside control width, must be <= CONTROL2_WIDTH
    parameter COEF_WIDTH = 32, // CP, CI WIDTH
    parameter QIN = 22, // Q In Signal
    parameter QCOEF = 31, // Q CP, CI's
    parameter QCONTROL = 31, // Q Controlvalue
    parameter CEXTEND = 4, // room for saturation check
    parameter DEXTEND = 1,  // data, erorr extend
    parameter AMCONTROL_ALLOW_NEG_SPECIAL = 1,
    parameter AUTO_RESET_AT_LIMIT = 0,
    parameter CONTROL2_WIDTH_X = CONTROL2_WIDTH-4 
)
(
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_CLKEN aclk" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_BUSIF S_AXIS:S_AXIS_reset:M_AXIS_PASS:M_AXIS_PASS2:M_AXIS_CONTROL:M_AXIS_CONTROL2" *)
    input aclk,
    input wire [AXIS_TDATA_WIDTH-1:0]  S_AXIS_tdata,
    input wire                         S_AXIS_tvalid,

    input wire signed [AXIS_TDATA_WIDTH-1:0] setpoint,
    input wire signed [COEF_WIDTH-1:0]  cp,
    input wire signed [COEF_WIDTH-1:0]  ci,
    input wire signed [M_AXIS_CONTROL_TDATA_WIDTH-1:0]  limit_upper,
    input wire signed [M_AXIS_CONTROL_TDATA_WIDTH-1:0]  limit_lower,
    
    input wire [M_AXIS_CONTROL_TDATA_WIDTH-1:0]  S_AXIS_reset_tdata,
    input wire                      S_AXIS_reset_tvalid,
    input enable,
    
    output wire [AXIS_TDATA_WIDTH-1:0] M_AXIS_PASS_tdata,
    output wire                        M_AXIS_PASS_tvalid,
    
    output wire [AXIS_TDATA_WIDTH-1:0] M_AXIS_PASS2_tdata,
    output wire                        M_AXIS_PASS2_tvalid,
    
    output wire [M_AXIS_CONTROL_TDATA_WIDTH-1:0] M_AXIS_CONTROL_tdata,  // may be less precision
    output wire                                  M_AXIS_CONTROL_tvalid,
    
    output wire [M_AXIS_CONTROL2_TDATA_WIDTH-1:0] M_AXIS_CONTROL2_tdata, // full precision without over flow bit
    output wire                                   M_AXIS_CONTROL2_tvalid,

    output wire signed [31:0] mon_signal,
    output wire signed [31:0] mon_error,
    output wire signed [31:0] mon_control,
    output wire signed [31:0] mon_control_lower32,
    output wire signed [31:0] mon_control_B,
    
    output wire status_max,
    output wire status_min
    );
    
    reg signed [CONTROL2_WIDTH_X+CEXTEND-1:0] upper, lower, reset;
    reg signed [CONTROL2_WIDTH_X+CEXTEND-1:0] control=0;
    reg signed [CONTROL2_WIDTH_X+CEXTEND-1:0] control_next=0;
    reg signed [CONTROL2_WIDTH_X+CEXTEND-1:0] control_cie=0;
    reg signed [CONTROL2_WIDTH_X+CEXTEND-1:0] control_cpe=0;
    reg signed [CONTROL2_WIDTH_X+CEXTEND-1:0] controlint=0;
    reg signed [CONTROL2_WIDTH_X+CEXTEND-1:0] controlint_next=0;
    reg signed [AXIS_TDATA_WIDTH+DEXTEND-1:0] error=0;
    reg signed [AXIS_TDATA_WIDTH+DEXTEND-1:0] error_next=0;
    reg signed [AXIS_TDATA_WIDTH-1:0] m=0;
    reg control_max;
    reg control_min;
    reg signed [COEF_WIDTH-1:0] cpX=0;
    reg signed [COEF_WIDTH-1:0] ciX=0;

    assign M_AXIS_PASS_tdata  = S_AXIS_tdata; // pass
    assign M_AXIS_PASS_tvalid = S_AXIS_tvalid; // pass
    assign M_AXIS_PASS2_tdata  = S_AXIS_tdata; // pass
    assign M_AXIS_PASS2_tvalid = S_AXIS_tvalid; // pass
   
    assign status_max = control_max;
    assign status_min = control_min;

/*
assign	w_convergent = i_data[(IWID-1):0]
			+ { {(OWID){1'b0}},
				i_data[(IWID-OWID)],
				{(IWID-OWID-1){!i_data[(IWID-OWID)]}}};
always @(posedge i_clk)
	o_convergent <= w_convergent[(IWID-1):(IWID-OWID)];
*/     
    always @ (posedge aclk)
    begin
        upper <= {{(CEXTEND){limit_upper[CONTROL_WIDTH-1]}}, limit_upper, {(CONTROL2_WIDTH_X-CONTROL_WIDTH){1'b0}}};  // sign extend and pad on right to control2 width
        lower <= {{(CEXTEND){limit_lower[CONTROL_WIDTH-1]}}, limit_lower, {(CONTROL2_WIDTH_X-CONTROL_WIDTH){1'b0}}};
        reset <= {{(CEXTEND){S_AXIS_reset_tdata[CONTROL_WIDTH-1]}}, S_AXIS_reset_tdata, {(CONTROL2_WIDTH_X-CONTROL_WIDTH){1'b0}}};
        // error <= error_next;
        if (error_next[AXIS_TDATA_WIDTH+DEXTEND-2] == 0 && error_next[AXIS_TDATA_WIDTH+DEXTEND-3] == 0 && error_next[AXIS_TDATA_WIDTH+DEXTEND-4] == 0 && error_next[AXIS_TDATA_WIDTH+DEXTEND-5] == 0)
        begin
            error <= { error_next[AXIS_TDATA_WIDTH+DEXTEND-1], error_next[AXIS_TDATA_WIDTH+DEXTEND-4-1-1:0]};
            cpX   <= cp;
            ciX   <= ci;
        end 
        else
        begin
            error <= error_next[AXIS_TDATA_WIDTH+DEXTEND-1:4] ;
            cpX   <= cp>>4;
            ciX   <= ci>>4;
        end
        // limit to range, in control mode
        if (enable && control_next > upper)
        begin
            if (AUTO_RESET_AT_LIMIT)
            begin
                control      <= reset;
                controlint   <= reset;
            end
            else
            begin
                control      <= upper;
                controlint   <= upper;
            end
            control_max  <= 1;
            control_min  <= enable? 0:1;
        end
        else 
        begin
            if (enable && control_next < lower)
            begin
                if (AUTO_RESET_AT_LIMIT)
                begin
                    control      <= reset;
                    controlint   <= reset;
                end
                else
                begin
                    control      <= lower;
                    controlint   <= lower;
                end
                control_max  <= enable? 0:1;
                control_min  <= 1;
            end 
            else
            begin
                if (AMCONTROL_ALLOW_NEG_SPECIAL)
                begin
                    if (error_next > $signed(0) && control_next < $signed(0)) // auto reset condition for amplitude control to preven negative phase, but allow active "damping"
                    begin
                        control      <= 0;
                        controlint   <= 0;
                    end
                    else
                    begin
                        control      <= control_next;
                        controlint   <= controlint_next;
                    end
                    control_max  <= enable? 0:1;
                    control_min  <= enable? 0:1;
                end 
                else
                begin
                    control      <= control_next;
                    controlint   <= controlint_next;
                    control_max  <= enable? 0:1;
                    control_min  <= enable? 0:1;
                end
            end
        end

        if (S_AXIS_tvalid)
        begin
            m <= $signed (S_AXIS_tdata); // {{(DEXTEND){S_AXIS_tdata[AXIS_TDATA_WIDTH-1]}},S_AXIS_tdata};
        end
        
        error_next <= setpoint - m; // Q AXIS_TDATA_WIDTH-1

        if (enable) // run controller
        begin
            // Q CONTROL2_WIDTH_X-1 (Q31)  ===  AXIS_TDATA_WIDTH-1 (Q31) + COEFQ (Q22) --- SHR
            //controlint_next <= controlint + ((ci*error) >>> (QCOEF+QIN-QCONTROL)); // saturation via extended range and limiter // Q31 x Q22
            //control_next    <= controlint + ((cp*error) >>> (QCOEF+QIN-QCONTROL)); // 
            control_cie <= ciX*error; // saturation via extended range and limiter // Q64.. += Q31 x Q22
            control_cpe <= cpX*error; // saturation via extended range and limiter // Q64.. += Q31 x Q22
            controlint_next <= controlint + control_cie; // saturation via extended range and limiter // Q64.. += Q31 x Q22
            control_next    <= controlint + control_cpe; // 
            //controlint_next <= controlint + ((ci*error) >>> (COEFQ+AXIS_TDATA_WIDTH-CONTROL2_WIDTH_X)); // saturation via extended range and limiter
            //control_next    <= controlint + ((cp*error) >>> (COEFQ+AXIS_TDATA_WIDTH-CONTROL2_WIDTH_X)); // make this shift "0"
            //controlint_next <= controlint + (ci*error); // >>> (COEFQ+AXIS_TDATA_WIDTH-CONTROL2_WIDTH_X)); // saturation via extended range and limiter
            //control_next    <= controlint + (cp*error); // >>> (COEFQ+AXIS_TDATA_WIDTH-CONTROL2_WIDTH_X)); // make this shift "0" -- d.h. COEFQ+AXIS_TDATA_WIDTH == CONTROL2_WIDTH_X
        end 
        else // pass reset value as control
        begin
            controlint_next <= reset;
            control_next    <= reset;
        end
    end
    
    assign M_AXIS_CONTROL_tdata   = {control[CONTROL2_WIDTH_X+CEXTEND-1], control[CONTROL2_WIDTH_X-2:CONTROL2_WIDTH_X-CONTROL_WIDTH]}; // strip extension
    assign M_AXIS_CONTROL_tvalid  = 1'b1;
    assign M_AXIS_CONTROL2_tdata  = {control[CONTROL2_WIDTH_X+CEXTEND-1], control[CONTROL2_WIDTH_X-2:CONTROL2_WIDTH_X-CONTROL2_OUT_WIDTH]};
    assign M_AXIS_CONTROL2_tvalid = 1'b1;

    assign mon_signal  = {m[AXIS_TDATA_WIDTH+DEXTEND-1], m[AXIS_TDATA_WIDTH-2:0]};
    assign mon_error   = {error[AXIS_TDATA_WIDTH+DEXTEND-1], error[AXIS_TDATA_WIDTH-2:0]};
    assign mon_control = {control[CONTROL2_WIDTH_X+CEXTEND-1], control[CONTROL2_WIDTH_X-2:CONTROL2_WIDTH_X-32]};
    assign mon_control_B = {control[CONTROL2_WIDTH_X+CEXTEND-1], control[CONTROL2_WIDTH_X-2:CONTROL2_WIDTH_X-32]};
    assign mon_control_lower32 = {{control[CONTROL2_WIDTH_X-32-1 : (CONTROL2_WIDTH_X>=64? CONTROL2_WIDTH_X-32-1-31:0)]}, {(CONTROL2_WIDTH_X>=64?0:(64-CONTROL2_WIDTH_X)){1'b0}}}; // signed, lower 31
    //assign mon_control_lower32 = {control[CONTROL2_WIDTH_X-32:0], {(32-(CONTROL2_WIDTH_X-32)){1'b0}}};

endmodule
