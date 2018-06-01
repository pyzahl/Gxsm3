`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: GXSM 
// Author: Percy Zahl

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
/*
# s,c signal in Q27
def phasedetect (signal, s, c, Tau_pac):
    global LMS_a, LMS_b
    a=LMS_a
    b=LMS_b
    #// Sin/Cos in Q27 coeff in Q27
	
    #// Apply loopback filter on ref
	
    ci1t=s  # DelayForRef (delayRef, s, &InputRef1[0]);
    cr1t=c  # DelayForRef (delayRef, c, &InputRef2[0]);
	
    #// Compute the prediction
	
    temll = ci1t * a  #// Q22 * Q22 : Q44
    temll = temll + cr1t * b + 0x200000  #// Q22 * Q22 : Q44
    predict = temll >> 22 #// Q22

    #// Compute d_mu_e	
	
    errordet = signal-predict #// Q22 - Q22 : Q22
    temll = errordet * Tau_pac + 0x200000  #// Q22 * Q22 : Q44 
    temll = temll >> 22  #// Q22
    d_mu_e = temll;
	
    #// Compute LMS
	
    temll = cr1t * d_mu_e + 0x200000  #// Q22 * Q22 : Q44 
    temll = temll >> 22 #// Q22
    b = b+temll

    temll = ci1t * d_mu_e + 0x200000  #// Q22 * Q22 : Q44 
    temll = temll >> 22  #// Q22
    a = a+temll

    LMS_a = a
    LMS_b = b
    
    
    def rot45 (a,b):
        s=c=SQRT2
        return (a*c - b*s, a*s + b*c)
    
    def phase (a, b):
        if abs (b) > 0:
            return Q22*arctan (a/b) # +/- pi/2
        else:
            if a > 0:
                return Q22
            else:
                return -Q22
  
*/


module lms_phase_amplitude_detector #(
    parameter S_AXIS_SIGNAL_TDATA_WIDTH = 32,
    parameter S_AXIS_SIGNAL_DATA_WIDTH = 16,
    parameter S_AXIS_SIGNAL_SIGNIFICANT_DATA_WIDTH = 14,
    parameter S_AXIS_SC_TDATA_WIDTH = 64,
    parameter M_AXIS_SC_TDATA_WIDTH = 64,
    parameter SC_DATA_WIDTH = 26,
    parameter LMS_DATA_WIDTH = 26,
    parameter LMS_Q_WIDTH = 22, // do not change
    parameter M_AXIS_XY_TDATA_WIDTH = 64,
    parameter M_AXIS_AM_TDATA_WIDTH = 48
)
(
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input aclk,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [S_AXIS_SIGNAL_TDATA_WIDTH-1:0]  S_AXIS_SIGNAL_tdata,
    input wire                                  S_AXIS_SIGNAL_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [S_AXIS_SC_TDATA_WIDTH-1:0]  S_AXIS_SC_tdata,
    input wire                              S_AXIS_SC_tvalid,
    input signed [31:0] tau, // Q22
    input signed [31:0] Atau, // Q22
    input signed [31:0] dc, // Q22

    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    output wire [M_AXIS_SC_TDATA_WIDTH-1:0] M_AXIS_SC_tdata, // (Sine, Cosine) vector pass through
    output wire                             M_AXIS_SC_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    output wire [M_AXIS_XY_TDATA_WIDTH-1:0] M_AXIS_XY_tdata, // (Sine, Cosine) vector pass through
    output wire                             M_AXIS_XY_tvalid,

    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    output wire [S_AXIS_SIGNAL_TDATA_WIDTH-1:0]  M_AXIS_SIGNAL_tdata,
    output wire                                  M_AXIS_SIGNAL_tvalid,

    
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    output wire [M_AXIS_AM_TDATA_WIDTH-1:0] M_AXIS_AM2_tdata, // dbg
    output wire                             M_AXIS_AM2_tvalid,
    
    output wire [31:0] dbg1,
    output wire [31:0] dbg2,
    output wire [31:0] dbg3,
    output wire [31:0] dbg4,
    output wire [31:0] Aout,
    output wire [31:0] Bout
);
    reg signed [LMS_DATA_WIDTH-1:0] s=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] s1=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] s2=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] c=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] c1=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] c2=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] m=0; // Q22 input signal: measured value
    reg signed [LMS_DATA_WIDTH-1:0] m1=0; // Q22 input signal: measured value
    reg signed [LMS_DATA_WIDTH-1:0] a=0;
    reg signed [LMS_DATA_WIDTH-1:0] Aa=0;
    reg signed [LMS_DATA_WIDTH-1:0] b=0;
    reg signed [LMS_DATA_WIDTH-1:0] Ab=0;
    reg signed [LMS_DATA_WIDTH-1:0] predict=0; 
    reg signed [LMS_DATA_WIDTH-1:0] Apredict=0; 
    reg signed [LMS_DATA_WIDTH-1:0] predict1=0; 
    reg signed [LMS_DATA_WIDTH-1:0] Apredict1=0; 
    reg signed [LMS_DATA_WIDTH-1:0] d_mu_e1=0; 
    reg signed [LMS_DATA_WIDTH-1:0] Ad_mu_e1=0; 
    reg signed [LMS_DATA_WIDTH-1:0] d_mu_e2=0; 
    reg signed [LMS_DATA_WIDTH-1:0] Ad_mu_e2=0; 
    reg [2*(LMS_DATA_WIDTH-1)+1-1:0] ampl2=0; 
    reg signed [LMS_DATA_WIDTH+2-1:0] x=0; 
    reg signed [LMS_DATA_WIDTH+2-1:0] y=0; 
    
    assign M_AXIS_SC_tdata  = S_AXIS_SC_tdata; // pass
    assign M_AXIS_SC_tvalid = S_AXIS_SC_tvalid; // pass
   
    assign M_AXIS_SIGNAL_tdata  = S_AXIS_SIGNAL_tdata; // pass
    assign M_AXIS_SIGNAL_tvalid = S_AXIS_SIGNAL_tvalid; // pass

    always @ (posedge aclk)
    begin
        if (S_AXIS_SIGNAL_tvalid)
        begin
//            m <= {{(LMS_DATA_WIDTH-S_AXIS_SIGNAL_SIGNIFICANT_DATA_WIDTH){S_AXIS_SIGNAL_tdata[S_AXIS_SIGNAL_SIGNIFICANT_DATA_WIDTH-1]}},
//                                                                        {S_AXIS_SIGNAL_tdata[S_AXIS_SIGNAL_SIGNIFICANT_DATA_WIDTH-1 : 0]}};
//                 (26-22-1=3)S, td_adc0[14-1:0=14], (26-14-1=11)0 = 3+14+11 = 24
//                 26 =  3 + 14 + (26-(26-22-1)-14)=9
            m <= {{(LMS_DATA_WIDTH-LMS_Q_WIDTH-1){S_AXIS_SIGNAL_tdata[S_AXIS_SIGNAL_SIGNIFICANT_DATA_WIDTH-1]}}, // signum bit 13 extend to LMS_DATA_WIDTH
                                                 {S_AXIS_SIGNAL_tdata[S_AXIS_SIGNAL_SIGNIFICANT_DATA_WIDTH-1 : 0]}, // 14bit ADC data bits 13..0
                  {(LMS_DATA_WIDTH-(LMS_DATA_WIDTH-LMS_Q_WIDTH-1)-S_AXIS_SIGNAL_SIGNIFICANT_DATA_WIDTH){1'b0}} // fill 0
                 };
        end
        if (S_AXIS_SC_tvalid)
            begin
            s <= {{(LMS_DATA_WIDTH-LMS_Q_WIDTH-1){ S_AXIS_SC_tdata[S_AXIS_SC_TDATA_WIDTH/2+SC_DATA_WIDTH-1]}}, S_AXIS_SC_tdata[S_AXIS_SC_TDATA_WIDTH/2+SC_DATA_WIDTH-1 : S_AXIS_SC_TDATA_WIDTH/2+SC_DATA_WIDTH-LMS_Q_WIDTH-1]};
            c <= {{(LMS_DATA_WIDTH-LMS_Q_WIDTH-1){ S_AXIS_SC_tdata[                        SC_DATA_WIDTH-1]}}, S_AXIS_SC_tdata[                        SC_DATA_WIDTH-1 :                         SC_DATA_WIDTH-LMS_Q_WIDTH-1]};
        end

        // ci1t=s;  // DelayForRef (delayRef, s, &InputRef1[0]);
        // cr1t=c;  // DelayForRef (delayRef, c, &InputRef2[0]);
        
        // Compute the prediction
        // ###0
        // temp = s * a + c * b + 0x200000;  // Q22 * Q22 : Q44
        predict <= (s * a + c * b + 45'sh200000) >>> 22; // Q22
        Apredict <= (s * Aa + c * Ab + 45'sh200000) >>> 22; // Q22
        
        // Compute d_mu_e    
        // ### 1
        // errordet = signal-predict #// Q22 - Q22 : Q22
        // temll = (errordet * Tau_pac + 0x200000  #// Q22 * Q22 : Q44 
        // temll = temll >> 22  #// Q22
        d_mu_e1 <= ((m1-predict1) * tau + 45'sh200000) >>> 22;
        Ad_mu_e1 <= ((m1-Apredict1) * Atau + 45'sh200000) >>> 22;
        
        // Compute LMS
        // ### 2
        // temll = c * d_mu_e + 0x200000  #// Q22 * Q22 : Q44 
        // temll = temll >> 22 #// Q22
        b <= b + ((c2 * d_mu_e2 + 45'sh200000) >>> 22);
        Ab <= Ab + ((c2 * Ad_mu_e2 + 45'sh200000) >>> 22);
        
        // temll = s * d_mu_e + 0x200000  #// Q22 * Q22 : Q44 
        // temll = temll >> 22  #// Q22
        a <= a + ((s2 * d_mu_e2 + 45'sh200000) >>> 22);
        Aa <= Aa + ((s2 * Ad_mu_e2 + 45'sh200000) >>> 22);
        
        // Rot45
        // R2 = sqrt(2)
        // ar = a*R2 - b*R2 = R2*(a-b)
        // br = a*R2 + b*R2 = R2*(a+b)
        // Q22*arctan (ar/br) # +/- pi/2    
        // --> phase = Q22 arctan ((a-b)/(a+b))
        // amp = sqrt (a*a + b*b); // SQRT( Q44 )
        // ph  = atan ((a-b)/(a+b)); // Q22

        // amplitude from 2nd A-PAC 
        ampl2 <= Aa*Aa + Ab*Ab; // 1Q44
        y <= a-b;
        x <= a+b;
        
        predict1 <= predict;
        Apredict1 <= Apredict;
        d_mu_e2 <= d_mu_e1;
        Ad_mu_e2 <= Ad_mu_e1;
        
        m1 <= m-dc;
        c1 <= c;
        c2 <= c1;
        s1 <= s;
        s2 <= s1;
    end
    
    //assign amplitude = sqrt (a*a+b*b); // Q22
    //assign phase = 4194303*atan ((a-b)/(a+b)); // Q22

    // atan2 cordic input w 28
    assign M_AXIS_XY_tdata    = { {(M_AXIS_XY_TDATA_WIDTH/2-(LMS_DATA_WIDTH+2)){y[LMS_DATA_WIDTH+2-1]}}, y,  
                                  {(M_AXIS_XY_TDATA_WIDTH/2-(LMS_DATA_WIDTH+2)){x[LMS_DATA_WIDTH+2-1]}}, x };
    assign M_AXIS_XY_tvalid = 1'b1;

    assign M_AXIS_AM2_tdata  = { {(M_AXIS_AM_TDATA_WIDTH-2*(LMS_DATA_WIDTH-1)){1'b0}}, ampl2[2*(LMS_DATA_WIDTH-1):0] }; 
    assign M_AXIS_AM2_tvalid = 1'b1;
    
    //assign M_AXIS_XX_tdata  = { {(2){predict1[LMS_DATA_WIDTH-1]}},predict1[LMS_DATA_WIDTH-1:LMS_DATA_WIDTH-14], {(2){m[LMS_DATA_WIDTH-1]}},m[LMS_DATA_WIDTH-1-8:LMS_DATA_WIDTH-14-8]}; 
    //assign M_AXIS_XX_tvalid = 1'b1;

    assign Aout = {{(32-LMS_DATA_WIDTH){a[LMS_DATA_WIDTH-1]}}, a}; // a
    assign Bout = {{(32-LMS_DATA_WIDTH){b[LMS_DATA_WIDTH-1]}}, b}; // b
    assign dbg1 = {{(32-LMS_DATA_WIDTH){m[LMS_DATA_WIDTH-1]}}, m}; // m
    assign dbg2 = {{(32-LMS_DATA_WIDTH){m[LMS_DATA_WIDTH-1]}}, m1}; // m1
    assign dbg3 = {{(32-LMS_DATA_WIDTH-2){x[LMS_DATA_WIDTH+2-1]}}, x}; // x
    assign dbg4 = {{(32-LMS_DATA_WIDTH-2){y[LMS_DATA_WIDTH+2-1]}}, y}; // y


endmodule
