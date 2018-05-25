`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
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
//////////////////////////////////////////////////////////////////////////////////


module axis_sc28_to_14 #(
    parameter SRC_BITS = 26,
    parameter AUX_BITS = 16,
    parameter SAXIS_DATA_WIDTH = 32,
    parameter SAXIS_TDATA_WIDTH = 64, // two channels s,c
    parameter SAXIS_AUX_TDATA_WIDTH = 16,
    parameter SAXIS_AUX_DATA_WIDTH = 16,
    parameter ADC_WIDTH = 14,
    parameter MAXIS_DATA_WIDTH = 16,
    parameter MAXIS_TDATA_WIDTH = 32
)
(
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input a_clk,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_TDATA_WIDTH-1:0]  S_AXIS_tdata,
    input wire                          S_AXIS_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input wire [SAXIS_AUX_TDATA_WIDTH-1:0]  S_AXIS_aux_tdata,
    input wire                              S_AXIS_aux_tvalid, // if valid is set, this data is choosen for channel 2
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    output [MAXIS_TDATA_WIDTH-1:0]      M_AXIS_tdata,
    output                              M_AXIS_tvalid
);
    assign M_AXIS_tdata = {
        {(MAXIS_DATA_WIDTH-ADC_WIDTH){S_AXIS_tdata[SRC_BITS-1]}}, 
                                      S_AXIS_tdata[SRC_BITS-1:SRC_BITS-ADC_WIDTH],
        S_AXIS_aux_tvalid ?
        {{(MAXIS_DATA_WIDTH-ADC_WIDTH){S_AXIS_aux_tdata[AUX_BITS-1]}}, 
                                      S_AXIS_tdata[AUX_BITS-1:AUX_BITS-ADC_WIDTH]}
        :
        {{(MAXIS_DATA_WIDTH-ADC_WIDTH){S_AXIS_tdata[SAXIS_TDATA_WIDTH-SAXIS_DATA_WIDTH+SRC_BITS-1]}}, 
                                       S_AXIS_tdata[SAXIS_TDATA_WIDTH-SAXIS_DATA_WIDTH+SRC_BITS-1:SAXIS_TDATA_WIDTH-SAXIS_DATA_WIDTH+SRC_BITS-ADC_WIDTH]}
    };
    assign M_AXIS_tvalid = S_AXIS_tvalid;

endmodule
