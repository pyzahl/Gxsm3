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
    parameter S_AXIS_SIGNAL_TDATA_WIDTH = 32, // actually used: LMS DATA WIDTH and Q
    parameter S_AXIS_SC_TDATA_WIDTH = 64,
    parameter M_AXIS_SC_TDATA_WIDTH = 64,
    parameter SC_DATA_WIDTH = 26,
    parameter LMS_DATA_WIDTH = 26,
    parameter LMS_Q_WIDTH = 22, // do not change
    parameter M_AXIS_XY_TDATA_WIDTH = 64,
    parameter M_AXIS_AM_TDATA_WIDTH = 48,
    parameter LCK_INT_WIDTH = 22+44-12,    // 22 -- 14 base, renormalized around dphi working range (12 bits) ::: 22+44 - renorm ~54 ?!??!?
    parameter LCK_INT_PH_WIDTH = 45, // 44 + 1 for phase
    parameter LCK_BUFFER_LEN2 = 14,
    parameter USE_DUAL_PAC = 1,
    parameter USE_DUAL_PAC_AND_LOCKIN = 0, // does not fit on RedPitaya Classic!
    parameter USE_LOCKIN   = 1
)
(
    //(* X_INTERFACE_PARAMETER = "FREQ_HZ 62500000" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_CLKEN aclk" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_BUSIF S_AXIS_SIGNAL:S_AXIS_SC:M_AXIS_SC:M_AXIS_XY:M_AXIS_AM2:M_AXIS_Aout:M_AXIS_Bout:M_AXIS_LockInX:M_AXIS_LockInY" *)
    input aclk,
    input wire [S_AXIS_SIGNAL_TDATA_WIDTH-1:0]  S_AXIS_SIGNAL_tdata,
    input wire                                  S_AXIS_SIGNAL_tvalid,

    input wire [S_AXIS_SC_TDATA_WIDTH-1:0]  S_AXIS_SC_tdata,
    input wire                              S_AXIS_SC_tvalid,
    input signed [31:0] tau, // Q22 tau phase
    input signed [31:0] Atau, // Q22 tau amplitude
    
    input [31:0] DDS_dphi,
    
    input lck_ampl,
    input lck_phase,
    
    output wire [M_AXIS_SC_TDATA_WIDTH-1:0] M_AXIS_SC_tdata, // (Sine, Cosine) vector pass through
    output wire                             M_AXIS_SC_tvalid,

    output wire [M_AXIS_XY_TDATA_WIDTH-1:0] M_AXIS_XY_tdata, // (Sine, Cosine) vector pass through
    output wire                             M_AXIS_XY_tvalid,

    output wire [M_AXIS_AM_TDATA_WIDTH-1:0] M_AXIS_AM2_tdata, // dbg
    output wire                             M_AXIS_AM2_tvalid,
    
    output wire [31:0] M_AXIS_Aout_tdata,
    output wire        M_AXIS_Aout_tvalid,
    output wire [31:0] M_AXIS_Bout_tdata,
    output wire        M_AXIS_Bout_tvalid,
    output wire [31:0] M_AXIS_LockInX_tdata,
    output wire        M_AXIS_LockInX_tvalid,
    output wire [31:0] M_AXIS_LockInY_tdata,
    output wire        M_AXIS_LockInY_tvalid,

    output wire sc_zero_x
    );


    reg signed [LMS_DATA_WIDTH-1:0] s=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] s1=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] s2=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] c=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] c1=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] c2=0; // Q22
    reg signed [LMS_DATA_WIDTH-1:0] m=0; // Q22 input signal: measured value
    reg signed [LMS_DATA_WIDTH-1:0] mA=0; // Q22 input signal: measured value
    reg signed [LMS_DATA_WIDTH-1:0] a=0;
    reg signed [LMS_DATA_WIDTH-1:0] Aa=0;
    reg signed [LMS_DATA_WIDTH-1:0] b=0;
    reg signed [LMS_DATA_WIDTH-1:0] Ab=0;
    reg signed [LMS_DATA_WIDTH-1:0] predict=0; 
    reg signed [LMS_DATA_WIDTH-1:0] Apredict=0; 
    reg signed [LMS_DATA_WIDTH-1:0] predict1=0; 
    reg signed [LMS_DATA_WIDTH-1:0] Apredict1=0; 
    reg signed [LMS_DATA_WIDTH-1:0] d_mu_e1x=0; 
    reg signed [LMS_DATA_WIDTH-1:0] Ad_mu_e1x=0; 
    reg signed [LMS_DATA_WIDTH-1:0] d_mu_e2=0; 
    reg signed [LMS_DATA_WIDTH-1:0] Ad_mu_e2=0; 
    reg [2*(LMS_DATA_WIDTH-1)+1-1:0] ampl2=0; 
    reg signed [LMS_DATA_WIDTH+2-1:0] x=0; 
    reg signed [LMS_DATA_WIDTH+2-1:0] y=0; 
    reg signed [LMS_DATA_WIDTH+2-1:0] tmpX=0; 
    reg signed [LMS_DATA_WIDTH+2-1:0] tmpY=0; 
    reg signed [LMS_DATA_WIDTH+2-1:0] tmpXA=0; 
    reg signed [LMS_DATA_WIDTH+2-1:0] tmpYA=0; 

    // Lock-In
    reg [32-1:0] dds_dphi [10-1:0]; // 22
    reg signed [LMS_DATA_WIDTH-1:0] LckX=0;
    reg signed [LMS_DATA_WIDTH-1:0] LckY=0;

    reg signed [45-1:0] LckdIntPhi1=0; // DDS PHASE WIDTH=44 + 1
    reg signed [45-1:0] LckdIntPhi2=0; // DDS PHASE WIDTH=44 + 1
    reg signed [LCK_INT_WIDTH-1:0] LckXInt=0;
    reg signed [LCK_INT_WIDTH-1:0] LckYInt=0;
    
    reg signed [LCK_INT_PH_WIDTH-1:0] LckDdphi1=0;
    reg signed [LCK_INT_WIDTH-1:0] LckXdphi1=0;
    reg signed [LCK_INT_WIDTH-1:0] LckYdphi1=0;
    reg signed [LCK_INT_PH_WIDTH-1:0] LckDdphi [(LCK_BUFFER_LEN2<<2)-1:0];
    reg signed [LCK_INT_WIDTH-1:0] LckXdphi [(LCK_BUFFER_LEN2<<2)-1:0];
    reg signed [LCK_INT_WIDTH-1:0] LckYdphi [(LCK_BUFFER_LEN2<<2)-1:0];
    
    reg signed [LMS_DATA_WIDTH-1:0] LckXSum=0;
    reg signed [LMS_DATA_WIDTH-1:0] LckYSum=0;
    reg [LCK_BUFFER_LEN2-1:0] Lck_i=0;
    reg [LCK_BUFFER_LEN2-1:0] Lck_N=0;

    reg sp=0;
    reg cp=0;
    reg sc_zero=0;
    
    assign M_AXIS_SC_tdata  = S_AXIS_SC_tdata; // pass
    assign M_AXIS_SC_tvalid = S_AXIS_SC_tvalid; // pass
   
    always @ (posedge aclk)
    begin
        
        //  special IIR DC filter DC error on average of samples at 0,90,180,270
        if (sc_zero)
            sc_zero <= 0; // reset zero indicator

        if (S_AXIS_SC_tvalid)
        begin
        // DDS step, may delay by 10
            dds_dphi[9] <= DDS_dphi;
            dds_dphi[8] <= dds_dphi[9];
            dds_dphi[7] <= dds_dphi[8];
            dds_dphi[6] <= dds_dphi[7];
            dds_dphi[5] <= dds_dphi[6];
            dds_dphi[4] <= dds_dphi[5];
            dds_dphi[3] <= dds_dphi[4];
            dds_dphi[2] <= dds_dphi[3];
            dds_dphi[1] <= dds_dphi[2];
            dds_dphi[0] <= dds_dphi[1];
        // Sin, Cos
            s <= {{(LMS_DATA_WIDTH-LMS_Q_WIDTH-1){ S_AXIS_SC_tdata[S_AXIS_SC_TDATA_WIDTH/2+SC_DATA_WIDTH-1]}}, S_AXIS_SC_tdata[S_AXIS_SC_TDATA_WIDTH/2+SC_DATA_WIDTH-1 : S_AXIS_SC_TDATA_WIDTH/2+SC_DATA_WIDTH-LMS_Q_WIDTH-1]};
            c <= {{(LMS_DATA_WIDTH-LMS_Q_WIDTH-1){ S_AXIS_SC_tdata[                        SC_DATA_WIDTH-1]}}, S_AXIS_SC_tdata[                        SC_DATA_WIDTH-1 :                         SC_DATA_WIDTH-LMS_Q_WIDTH-1]};
        // S,C Zero Detector
            if (s>$signed(0) && ~sp)
            begin
                sp <= 1;
                sc_zero <= 1;
            end
            else
            begin
                if (s<$signed(0) && sp)
                begin
                    sp <= 0;
                    sc_zero <= 1;
                end
            end
            if (c>$signed(0) && ~cp)
            begin
                cp <= 1;
                sc_zero <= 1;
            end
            else
            begin
                if (c<$signed(0) && cp)
                begin
                    cp <= 0;
                    sc_zero <= 1;
                end
            end
        end

        // ci1t=s;  // DelayForRef (delayRef, s, &InputRef1[0]);
        // cr1t=c;  // DelayForRef (delayRef, c, &InputRef2[0]);
        
        // Compute the prediction
        // ###0
        // temp = s * a + c * b + 0x200000;  // Q22 * Q22 : Q44
        predict <= (s * a + c * b + $signed(45'sh200000)) >>> 22; // Q22
//# DUAL PAC or PAC+LCK -- both needs 82 DSP units out of 80 available for total design
        if (USE_DUAL_PAC)
            Apredict <= (s * Aa + c * Ab + $signed(45'sh200000)) >>> 22; // Q22

        m  <= $signed(S_AXIS_SIGNAL_tdata[LMS_DATA_WIDTH-1:0])-predict1;
        if (USE_DUAL_PAC)
            mA <= $signed(S_AXIS_SIGNAL_tdata[LMS_DATA_WIDTH-1:0])-Apredict1;
        
        // Compute d_mu_e    
        // ### 1
        // errordet = signal-predict #// Q22 - Q22 : Q22
        // temll = (errordet * Tau_pac + 0x200000  #// Q22 * Q22 : Q44 
        // temll = temll >> 22  #// Q22
        //d_mu_e1 <= ((m1-predict1) * tau + 45'sh200000) >>> 22;
        d_mu_e1x <= m * tau;
        if (USE_DUAL_PAC)
            //Ad_mu_e1 <= ((m1-Apredict1) * Atau + 45'sh200000) >>> 22;
            Ad_mu_e1x <= mA * Atau;

        // Compute LMS
        // ### 2
        // temll = c * d_mu_e + 0x200000  #// Q22 * Q22 : Q44 // +0x20000 is for rounding!
        // temll = temll >> 22 #// Q22
        b <= b + ((c2 * d_mu_e2 + $signed(45'sh200000)) >>> 22);
        if (USE_DUAL_PAC)
            Ab <= Ab + ((c2 * Ad_mu_e2 + $signed(45'sh200000)) >>> 22);
        // temll = s * d_mu_e + 0x200000  #// Q22 * Q22 : Q44 
        // temll = temll >> 22  #// Q22
        a <= a + ((s2 * d_mu_e2 + 45'sh200000) >>> 22);
        if (USE_DUAL_PAC)
            Aa <= Aa + ((s2 * Ad_mu_e2 + $signed(45'sh200000)) >>> 22);

        // Rot45
        // R2 = sqrt(2)
        // ar = a*R2 - b*R2 = R2*(a-b)
        // br = a*R2 + b*R2 = R2*(a+b)
        // Q22*arctan (ar/br) # +/- pi/2    
        // --> phase = Q22 arctan ((a-b)/(a+b))
        // amp = sqrt (a*a + b*b); // SQRT( Q44 )
        // ph  = atan ((a-b)/(a+b)); // Q22
        //---- amplitude and phase moved to end

        /*
        // amplitude squared from 2nd A-PAC 
        ampl2 <= Aa*Aa + Ab*Ab; // 1Q44
        // x,y (for phase)from 1st PAC
        y <= a-b;
        x <= a+b;
        */
        
        predict1 <= predict;
        d_mu_e2 <= (d_mu_e1x + $signed(45'sh200000)) >>> 22;

        if (USE_DUAL_PAC)
        begin
            Apredict1 <= Apredict;
            Ad_mu_e2 <= (Ad_mu_e1x  + $signed(45'sh200000)) >>> 22;
        end

        c1 <= c;
        c2 <= c1;
        s1 <= s;
        s2 <= s1;
        
        if (USE_LOCKIN)
        begin
            // Classic LockIn and Correlation Integral over one period
            // STEP 1: Correlelation Product
            LckX <= (s * m + $signed(45'sh200000)) >>> 22; // Q22
            LckY <= (c * m + $signed(45'sh200000)) >>> 22; // Q22
    
            // STEP 2: Scale to Phase-Signal Volume
            LckDdphi1 <= dds_dphi[0];
            LckXdphi1 <= (LckX * dds_dphi[0]) >>> 12; // Q22 + Q44 [assume 30kHz range renorm by 12 now (4096) --  4166 is 30kHz spp] 22+44-12=54
            LckYdphi1 <= (LckY * dds_dphi[0]) >>> 12;
            // Store in ring buffer
            LckDdphi [Lck_i] <= LckDdphi1;
            LckXdphi [Lck_i] <= LckXdphi1;
            LckYdphi [Lck_i] <= LckYdphi1;
    
    /*      sequencial code
            x = signal*s;
            y = signal*c;
            // add new
            sumdphi  += dphi
            sumcorrx += x*dphi
            sumcorry += y*dphi
            // correct and compensate if phase window len changed
            while (sumdphi > 2*math.pi+dphi/2):
                sumdphi  -= corrdphi[circ(corri-corrlen)];
                sumcorrx -= corrx[circ(corri-corrlen)];
                sumcorry -= corry[circ(corri-corrlen)];
                corrlen--;
          
            corrdphi[corri] = dphi;
            corrx[corri] = x*dphi;
            corry[corri] = y*dphi;
            corrlen++;
            corri = circ(corri+1);
    */
            LckdIntPhi2 = LckdIntPhi1 + LckDdphi1; // one step ???? blocking works may be???
            // single shot unrolled LockIn moving window correlation integral
            if (LckdIntPhi2 > {(44){1'b1}} && (LckdIntPhi2 - LckDdphi [Lck_i-Lck_N]) > {(44){1'b1}} ) // phase > 2pi (+dphi/2) // 2pi =!= 2<<44
            begin
                LckdIntPhi1 <= LckdIntPhi2 - LckDdphi [Lck_i-Lck_N] - LckDdphi [Lck_i-Lck_N+1];
                LckXInt <= LckXInt - LckXdphi [Lck_i-Lck_N] - LckXdphi [Lck_i-Lck_N+1] + LckXdphi [Lck_i]; 
                LckYInt <= LckYInt - LckYdphi [Lck_i-Lck_N] - LckYdphi [Lck_i-Lck_N+1] + LckYdphi [Lck_i];
                Lck_N <= Lck_N - 1; //!!!!
            end else begin
                if (LckdIntPhi2 > {(44){1'b1}}) // phase > 2pi (+dphi/2) // 2pi =!= 2<<44
                begin
                    LckdIntPhi1 <= LckdIntPhi2 - LckDdphi [Lck_i-Lck_N];
                    LckXInt <= LckXInt - LckXdphi [Lck_i-Lck_N] + LckXdphi [Lck_i];
                    LckYInt <= LckYInt - LckYdphi [Lck_i-Lck_N] + LckYdphi [Lck_i];
                end else begin
                    LckXInt <= LckXInt + LckXdphi [Lck_i];
                    LckYInt <= LckYInt + LckYdphi [Lck_i];
                    Lck_N <= Lck_N + 1; //!!!!
                end
            end
    /* does not fit */
    /*
            // single shot unrolled LockIn moving window correlation integral
            if (LckdIntPhi + LckDdphi1 > {(44){1'b1}} && (LckdIntPhi + LckDdphi1 - LckDdphi [Lck_i-Lck_N]) > {(44){1'b1}} ) // phase > 2pi (+dphi/2) // 2pi =!= 2<<44
            begin
                LckdIntPhi <= LckdIntPhi + LckDdphi1 - LckDdphi [Lck_i-Lck_N] - LckDdphi [Lck_i-Lck_N+1];
                LckXInt <= LckXInt - LckXdphi [Lck_i-Lck_N] - LckXdphi [Lck_i-Lck_N+1] + LckXdphi [Lck_i]; 
                LckYInt <= LckYInt - LckYdphi [Lck_i-Lck_N] - LckYdphi [Lck_i-Lck_N+1] + LckYdphi [Lck_i];
                Lck_N <= Lck_N - 1; //!!!!
            end else begin
                if (LckdIntPhi + LckDdphi1 > {(44){1'b1}}) // phase > 2pi (+dphi/2) // 2pi =!= 2<<44
                begin
                    LckdIntPhi <= LckdIntPhi + LckDdphi1 - LckDdphi [Lck_i-Lck_N];
                    LckXInt <= LckXInt - LckXdphi [Lck_i-Lck_N] + LckXdphi [Lck_i];
                    LckYInt <= LckYInt - LckYdphi [Lck_i-Lck_N] + LckYdphi [Lck_i];
                end else begin
                    LckdIntPhi <= LckdIntPhi + LckDdphi1;
                    LckXInt <= LckXInt + LckXdphi [Lck_i];
                    LckYInt <= LckYInt + LckYdphi [Lck_i];
                    Lck_N <= Lck_N + 1; //!!!!
                end
            end
    */
            Lck_i <= Lck_i + 1;
    
            LckXSum <= LckXInt >>> (44-12); // DDS-dPhi is Q 44 normalize with 2pi == Q44 (remaining bits)
            LckYSum <= LckYInt >>> (44-12); // DDS-dPhi is Q 44
        end
          
        // prepare outputs by selection        
        
        if (USE_DUAL_PAC)
        begin
            //# For Dual PAC:     
            // amplitude squared from 2nd A-PAC 
            ampl2 <= Aa*Aa + Ab*Ab; // 1Q44
            // x,y (for phase)from 1st PAC
            y <= a-b;
            x <= a+b;
        end 
        else 
        begin
            /* select PAC or LockIn individually */
            if (USE_DUAL_PAC_AND_LOCKIN)
                ampl2 <= (lck_ampl?LckXSum:Aa)*(lck_ampl?LckXSum:Aa) + (lck_ampl?LckYSum:Ab)*(lck_ampl?LckYSum:Ab); // dual amplitude calculation does not fit in RedPitaya Classic (small FPGA)
            else
                ampl2 <= (lck_ampl?LckXSum:a)*(lck_ampl?LckXSum:a) + (lck_ampl?LckYSum:b)*(lck_ampl?LckYSum:b);
            y     <= (lck_phase?LckXSum:a) - (lck_phase?LckYSum:b);
            x     <= (lck_phase?LckXSum:a) + (lck_phase?LckYSum:b);
        end        
          
    end
    
    //assign amplitude = sqrt (a*a+b*b); // Q22
    //assign phase = 4194303*atan ((a-b)/(a+b)); // Q22

    // atan2 cordic input w 28
    assign M_AXIS_XY_tdata    = { {(M_AXIS_XY_TDATA_WIDTH/2-(LMS_DATA_WIDTH+2)){y[LMS_DATA_WIDTH+2-1]}}, y,  
                                  {(M_AXIS_XY_TDATA_WIDTH/2-(LMS_DATA_WIDTH+2)){x[LMS_DATA_WIDTH+2-1]}}, x };
    assign M_AXIS_XY_tvalid   = 1'b1;

    assign M_AXIS_AM2_tdata   = { {(M_AXIS_AM_TDATA_WIDTH-2*(LMS_DATA_WIDTH-1)){1'b0}}, ampl2[2*(LMS_DATA_WIDTH-1):0] }; 
    assign M_AXIS_AM2_tvalid  = 1'b1;

    // LMS A,B
    assign M_AXIS_Aout_tdata  = {{(32-LMS_DATA_WIDTH){a[LMS_DATA_WIDTH-1]}}, a}; // a
    assign M_AXIS_Aout_tvalid = 1'b1;
    assign M_AXIS_Bout_tdata  = {{(32-LMS_DATA_WIDTH){b[LMS_DATA_WIDTH-1]}}, b}; // b
    assign M_AXIS_Bout_tvalid = 1'b1;

    // LockIn X,Y Sum
    assign M_AXIS_LockInX_tdata  = {{(32-LMS_DATA_WIDTH-2){LckXSum[LMS_DATA_WIDTH+2-1]}}, LckXSum};
    assign M_AXIS_LockInX_tvalid = 1'b1;
    assign M_AXIS_LockInY_tdata  = {{(32-LMS_DATA_WIDTH-2){LckYSum[LMS_DATA_WIDTH+2-1]}}, LckYSum};
    assign M_AXIS_LockInY_tvalid = 1'b1;
    
    assign sc_zero_x = sc_zero;

endmodule
