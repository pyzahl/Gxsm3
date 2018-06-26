`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 11/26/2017 12:54:20 AM
// Design Name: 
// Module Name: VolumeAdjuster
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


module ScaleAndAdjust #(
    parameter S_AXIS_DATA_WIDTH = 64,
    parameter START_BIT         = 31,
    parameter M_AXIS_DATA_WIDTH = 16,
    parameter GAIN_DATA_WIDTH   = 32,
    parameter GAIN_DATA_Q       = 16
)
(
   input a_clk,
   (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
   input wire [S_AXIS_DATA_WIDTH-1:0]  S_AXIS_tdata,
   input wire                          S_AXIS_tvalid,
   input wire [GAIN_DATA_WIDTH-1:0]    gain,
   //input wire [7:0]                     pre_shr,
   (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
   output [M_AXIS_DATA_WIDTH-1:0]       M_AXIS_tdata,
   output                               M_AXIS_tvalid
);

    reg signed [S_AXIS_DATA_WIDTH-START_BIT-1:0] x1=0;
    reg signed [S_AXIS_DATA_WIDTH-START_BIT-1:0] x=0;
    reg signed [GAIN_DATA_WIDTH-1:0] v1=0;
    reg signed [GAIN_DATA_WIDTH-1:0] v=0;
    //reg signed [M_AXIS_DATA_WIDTH-1:0] w=0;
    reg signed [GAIN_DATA_WIDTH+S_AXIS_DATA_WIDTH-START_BIT-1:0] y=0;
    always @ (posedge a_clk)
    begin
       if (S_AXIS_tvalid)
       begin
           x <= x1;
           v <= v1;
           x1 <= {S_AXIS_tdata[S_AXIS_DATA_WIDTH-1], S_AXIS_tdata[START_BIT-1 : 0]};
           v1 <= {gain[GAIN_DATA_WIDTH-1 : 0]};
           // w <= x <<< pre_shr;
           // y <= v*w;
           y <= v*x; // Volume Q15 or Q(VAXIS_DATA_Q-1)
       end
    end
    assign M_AXIS_tdata  = {y[GAIN_DATA_WIDTH+S_AXIS_DATA_WIDTH-START_BIT-1], y[M_AXIS_DATA_WIDTH+GAIN_DATA_Q-1 : GAIN_DATA_Q]}; //-GAIN_DATA_Q-
    assign M_AXIS_tvalid = S_AXIS_tvalid;
   
endmodule
