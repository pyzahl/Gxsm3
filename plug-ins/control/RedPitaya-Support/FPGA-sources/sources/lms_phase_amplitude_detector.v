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
# s,c signal in QSC
    def phasedetect (self, signal, s, c):
        #// Sin/Cos in Q27 coeff in Q27

        #// Apply loopback filter on ref

        self.ss=int ((1.-self.iirf)*self.ss+self.iirf*s)
        self.cc=int ((1.-self.iirf)*self.cc+self.iirf*c)

        S=s-self.ss  # DelayForRef (delayRef, s, &InputRef1[0]);
        C=c-self.cc  # DelayForRef (delayRef, c, &InputRef2[0]);

        #// Compute the prediction error
        sa = S * self.a
        cb = C * self.b
        predict_err = signal - (( sa + cb + SCQH ) >> NQSC)

        #// Compute d_mu_e  
        d_mu_e = ( predict_err * self.tau_pac + LMSQH ) >> NQLMS;

        #// Compute LMS
        self.b = self.b + (( C * d_mu_e + SCQH ) >> NQSC)
        self.a = self.a + (( S * d_mu_e + SCQH ) >> NQSC)


    def ampl (self):
        return math.sqrt (self.a*self.a + self.b*self.b)/QLMS

    def phase (self):
        return math.atan2 (self.a-self.b, self.a+self.b)
*/

module lms_phase_amplitude_detector #(
    parameter S_AXIS_SIGNAL_TDATA_WIDTH = 32, // actually used: LMS DATA WIDTH and Q
    parameter S_AXIS_SC_TDATA_WIDTH = 64,
    parameter M_AXIS_SC_TDATA_WIDTH = 64,
    parameter SC_DATA_WIDTH  = 25,  // SC 25Q24
    parameter SC_Q_WIDTH     = 24,  // SC 25Q24
    parameter LMS_DATA_WIDTH = 26,  // LMS 26Q22
    parameter LMS_Q_WIDTH    = 22,  // LMS 26Q22
    parameter M_AXIS_XY_TDATA_WIDTH = 64,
    parameter M_AXIS_AM_TDATA_WIDTH = 48,
    parameter LCK_INT_WIDTH = 22+44-12,    // 22 -- 14 base, renormalized around dphi working range (12 bits) ::: 22+44 - renorm ~54 ?!??!?
    parameter LCK_INT_PH_WIDTH = 45, // 44 + 1 for phase
    parameter LCK_BUFFER_LEN2 = 14,
    parameter USE_DUAL_PAC = 1,
    parameter USE_DUAL_PAC_AND_LOCKIN = 0, // does not fit on RedPitaya Classic!
    parameter COMPUTE_LOCKIN = 0
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

    wire signed [2*LMS_DATA_WIDTH-1:0] LMSQHALF             = { {(2*LMS_DATA_WIDTH-LMS_Q_WIDTH){1'b0}}, 1'b1, {(LMS_Q_WIDTH-1){1'b0}} }; // for simple rounding Q 1/2 ("=0.5")  2^(44-1)
    wire signed [SC_DATA_WIDTH+LMS_DATA_WIDTH-1:0] SCQHALF  = { {(SC_DATA_WIDTH+LMS_DATA_WIDTH-SC_Q_WIDTH){1'b0}}, 1'b1, {(SC_Q_WIDTH-1){1'b0}} }; // for simple rounding Q 1/2 ("=0.5")

    reg signed [31:0] Rtau=0; // Q22 tau phase
    reg signed [31:0] RAtau=0; // Q22 tau amplitude

    reg signed [LMS_DATA_WIDTH-1:0] signal=0;  // 26Q22   LMS  Q22
    reg signed [LMS_DATA_WIDTH-1:0] signal_1=0; // 26Q22
    reg signed [SC_DATA_WIDTH-1:0] s=0; // Q SC (25Q24)
    reg signed [SC_DATA_WIDTH-1:0] s1=0;
    reg signed [SC_DATA_WIDTH-1:0] s2=0;
    reg signed [SC_DATA_WIDTH-1:0] s3=0;
    reg signed [SC_DATA_WIDTH-1:0] c=0; 
    reg signed [SC_DATA_WIDTH-1:0] c1=0;
    reg signed [SC_DATA_WIDTH-1:0] c2=0;
    reg signed [SC_DATA_WIDTH-1:0] c3=0;
    reg signed [LMS_DATA_WIDTH-1:0] a=0;    // Q LMS (26Q22)
    reg signed [LMS_DATA_WIDTH-1:0] Aa=0;
    reg signed [LMS_DATA_WIDTH-1:0] b=0;
    reg signed [LMS_DATA_WIDTH-1:0] Ab=0;
    reg signed [SC_DATA_WIDTH+LMS_DATA_WIDTH-1:0] sa_1=0; // Q SC + Q LMS   (51Q46)
    reg signed [SC_DATA_WIDTH+LMS_DATA_WIDTH-1:0] cb_1=0;
    reg signed [SC_DATA_WIDTH+LMS_DATA_WIDTH-1:0] Asa_1=0;
    reg signed [SC_DATA_WIDTH+LMS_DATA_WIDTH-1:0] Acb_1=0;
    reg signed [LMS_DATA_WIDTH-1:0] predict_err_2=0; 
    reg signed [LMS_DATA_WIDTH-1:0] Apredict_err_2=0; 
    reg signed [LMS_DATA_WIDTH+LMS_DATA_WIDTH-1:0] d_mu_e_3=0; 
    reg signed [LMS_DATA_WIDTH+LMS_DATA_WIDTH-1:0] Ad_mu_e_3=0;
    reg signed [SC_DATA_WIDTH+LMS_DATA_WIDTH-1:0] Sd_mu_e_4=0; 
    reg signed [SC_DATA_WIDTH+LMS_DATA_WIDTH-1:0] ASd_mu_e_4=0;
    reg signed [SC_DATA_WIDTH+LMS_DATA_WIDTH-1:0] Cd_mu_e_4=0; 
    reg signed [SC_DATA_WIDTH+LMS_DATA_WIDTH-1:0] ACd_mu_e_4=0;
    
    reg [2*(LMS_DATA_WIDTH-1)+1-1:0] ampl2=0; // Q LMS Squared 
    reg [2*(LMS_DATA_WIDTH-1)+1-1:0] a2=0; 
    reg [2*(LMS_DATA_WIDTH-1)+1-1:0] b2=0; 
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
        Rtau  <= tau;  // buffer to register -- Q22 tau phase
        RAtau <= Atau; // buffer to register -- Q22 tau amplitude
        
    //  special IIR DC filter DC error on average of samples at 0,90,180,270
        if (sc_zero)
        begin
            sc_zero <= 0; // reset zero indicator
        end
        
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
        
    // Signal Input
        signal   <= S_AXIS_SIGNAL_tdata[LMS_DATA_WIDTH-1:0];
        signal_1 <= signal;
        
    // Sin, Cos
        //s <= {{(LMS_DATA_WIDTH-LMS_Q_WIDTH-1){ S_AXIS_SC_tdata[S_AXIS_SC_TDATA_WIDTH/2+SC_DATA_WIDTH-1]}}, S_AXIS_SC_tdata[S_AXIS_SC_TDATA_WIDTH/2+SC_DATA_WIDTH-1 : S_AXIS_SC_TDATA_WIDTH/2+SC_DATA_WIDTH-LMS_Q_WIDTH-1]};   // 26Q22 truncated
        //c <= {{(LMS_DATA_WIDTH-LMS_Q_WIDTH-1){ S_AXIS_SC_tdata[                        SC_DATA_WIDTH-1]}}, S_AXIS_SC_tdata[                        SC_DATA_WIDTH-1 :                         SC_DATA_WIDTH-LMS_Q_WIDTH-1]};   // 26Q22 truncated
        c <= S_AXIS_SC_tdata[                        SC_DATA_WIDTH-1 :                       0];  // 25Q24 full dynamic range, proper rounding   24: 0
        s <= S_AXIS_SC_tdata[S_AXIS_SC_TDATA_WIDTH/2+SC_DATA_WIDTH-1 : S_AXIS_SC_TDATA_WIDTH/2];  // 25Q24 full dynamic range, proper rounding   56:32
    // and pipelined delayed s,c
        c1 <= c;
        c2 <= c1;
        c3 <= c2;
        s1 <= s;
        s2 <= s1;
        s3 <= s2;
        
    // S,C Zero Detector
        if (s > $signed(0) && ~sp)
        begin
            sp <= 1;
            sc_zero <= 1;
        end else
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
        end else
        begin
            if (c<$signed(0) && cp)
            begin
                cp <= 0;
                sc_zero <= 1;
            end
        end

        // Compute the prediction
        // ### 1 pipeline level
        // predict <= (s * a + c * b + "0.5") >>> 22; // Q22
        sa_1 <= s * a; // Q SC + Q LMS 
        cb_1 <= c * b; // Q24 + Q22 => Q46
        
        // ### 2
        predict_err_2 <= signal_1 - ((sa_1 + cb_1 + SCQHALF) >>> SC_Q_WIDTH); // sum, round normalize Q SC + Q LMS to Q LMS: 46 to 22
        //# DUAL PAC or PAC+LCK -- both needs 82 DSP units out of 80 available for total design
        if (USE_DUAL_PAC)
        begin
            // Apredict <= (s * Aa + c * Ab + $signed(45'sh200000)) >>> 22; // Q22
            //== Apredict1 <= (s * Aa + c * Ab + QHALF) >>> LMS_Q_WIDTH; // Q44
            Asa_1 <= s * Aa; // Q SC + Q LMS 
            Acb_1 <= c * Ab; //  Q24 + Q22 => Q46
            Apredict_err_2 <= signal_1 - ((Asa_1 + Acb_1 + SCQHALF) >>> SC_Q_WIDTH); // Q44
        end
        
        // Compute d_mu_e    
        // ### 3
        // error = signal-predict #// Q22 - Q22 : Q22
        // d_mu_e1 <= ((signal-predict1) * tau + "0.5") >>> 22;
        //== d_mu_e3 <= ((signal1 - predict1) * Rtau + QHALF) >>> LMS_Q_WIDTH;
        //d_mu_e_3 <= (predict_err_2 * Rtau + LMSQHALF) >>> LMS_Q_WIDTH;
        //d_mu_e_3 <= (predict_err_2 * Rtau + LMSQHALF); // ) >>> LMS_Q_WIDTH;
        d_mu_e_3 <= predict_err_2 * Rtau + LMSQHALF; // ** test w o rounding
        if (USE_DUAL_PAC)
        begin
            //Ad_mu_e1 <= ((m1-Apredict1) * Atau + 45'sh200000) >>> 22;
            Ad_mu_e_3 <= (Apredict_err_2 * RAtau + LMSQHALF); //) >>> LMS_Q_WIDTH;
        end
        
        // Compute LMS
        // ### 4
        // b <= b + ((c2 * d_mu_e2 + "0.5") >>> 22);
        // b <= b + ((c3 * d_mu_e_3 + SCQHALF) >>> SC_Q_WIDTH);
        // b <= b + ((c3 * $signed(d_mu_e_3[LMS_Q_WIDTH+LMS_Q_WIDTH-1:LMS_Q_WIDTH]) + SCQHALF) >>> SC_Q_WIDTH);
        // Cd_mu_e_4 <= c3 * $signed(d_mu_e_3[LMS_Q_WIDTH+LMS_Q_WIDTH-1 : LMS_Q_WIDTH]) + SCQHALF;
        Cd_mu_e_4 <= c3 * $signed(d_mu_e_3[LMS_Q_WIDTH+LMS_Q_WIDTH-1 : LMS_Q_WIDTH]);
        //b <= b + $signed(Cd_mu_e_4[LMS_Q_WIDTH+SC_Q_WIDTH-1 : SC_Q_WIDTH]);
        b <= b + ((Cd_mu_e_4 + SCQHALF) >>> SC_Q_WIDTH);
        if (USE_DUAL_PAC)
        begin
            // Ab <= Ab + ((c3 * $signed(Ad_mu_e_3[LMS_Q_WIDTH+LMS_Q_WIDTH-1:LMS_Q_WIDTH]) + SCQHALF) >>> SC_Q_WIDTH);
            ACd_mu_e_4 <= c3 * $signed(Ad_mu_e_3[LMS_Q_WIDTH+LMS_Q_WIDTH-1 : LMS_Q_WIDTH]);
            Ab <= Ab + ((ACd_mu_e_4 + SCQHALF) >>> SC_Q_WIDTH);
        end
        
        // a <= a + ((s2 * d_mu_e2 + "0.5") >>> 22);
        // a <= a + ((s3 * d_mu_e_3 + SCQHALF) >>> SC_Q_WIDTH);
        // a <= a + ((s3 * $signed(d_mu_e_3[LMS_Q_WIDTH+LMS_Q_WIDTH-1:LMS_Q_WIDTH]) + SCQHALF) >>> SC_Q_WIDTH);
        Sd_mu_e_4 <= s3 * $signed(d_mu_e_3[LMS_Q_WIDTH+LMS_Q_WIDTH-1 : LMS_Q_WIDTH]);
        a <= a + ((Sd_mu_e_4+SCQHALF) >>> SC_Q_WIDTH);
        if (USE_DUAL_PAC)
        begin
            // Aa <= Aa + ((s3 * $signed(Ad_mu_e_3[LMS_Q_WIDTH+LMS_Q_WIDTH-1:LMS_Q_WIDTH]) + SCQHALF) >>> SC_Q_WIDTH);
            ASd_mu_e_4 <= s3 * $signed(Ad_mu_e_3[LMS_Q_WIDTH+LMS_Q_WIDTH-1 : LMS_Q_WIDTH]);
            Aa <= Aa + ((ASd_mu_e_4+SCQHALF) >>> SC_Q_WIDTH);
        end
        
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
        
        if (COMPUTE_LOCKIN)
        begin
            // Classic LockIn and Correlation Integral over one period
            // STEP 1: Correlelation Product
            LckX <= (s * signal + SCQHALF) >>> SC_Q_WIDTH; // Q22
            LckY <= (c * signal + SCQHALF) >>> SC_Q_WIDTH; // Q22
    
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
            end else 
                begin
                if (LckdIntPhi2 > {(44){1'b1}}) // phase > 2pi (+dphi/2) // 2pi =!= 2<<44
                begin
                    LckdIntPhi1 <= LckdIntPhi2 - LckDdphi [Lck_i-Lck_N];
                    LckXInt <= LckXInt - LckXdphi [Lck_i-Lck_N] + LckXdphi [Lck_i];
                    LckYInt <= LckYInt - LckYdphi [Lck_i-Lck_N] + LckYdphi [Lck_i];
                end else
                begin
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
            a2 <= Aa*Aa;
            b2 <= Ab*Ab;
            //== ampl2 <= Aa*Aa + Ab*Ab; // 1Q44
            ampl2 <= a2 + b2; // 1Q44
            // x,y (for phase)from 1st PAC
            y <= a-b;
            x <= a+b;
        end else 
        begin
            a2 <= a*a;
            b2 <= b*b;
            //== ampl2 <= Aa*Aa + Ab*Ab; // 1Q44
            ampl2 <= a2 + b2; // 1Q44
            // x,y (for phase)from 1st PAC
            y <= a-b;
            x <= a+b;
        end
        
        if (COMPUTE_LOCKIN)
        begin
            // select PAC or LockIn individually
            if (USE_DUAL_PAC_AND_LOCKIN) // select from Dual PAC or LockIn options
            begin
                a2 <= (lck_ampl?LckXSum:Aa)*(lck_ampl?LckXSum:Aa);
                b2 <= (lck_ampl?LckYSum:Ab)*(lck_ampl?LckYSum:Ab);
            end else
            begin // select from Single PAC (same tau for PH and AM) or LockIn
                a2 <= (lck_ampl?LckXSum:a)*(lck_ampl?LckXSum:a);
                b2 <= (lck_ampl?LckYSum:b)*(lck_ampl?LckYSum:b);
            end
            ampl2 <= a2 + b2; // 1Q44
            // x,y (for phase)from PH PAC or LockIn
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

    assign M_AXIS_AM2_tdata   = ampl2[M_AXIS_AM_TDATA_WIDTH-1:0]; 
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
