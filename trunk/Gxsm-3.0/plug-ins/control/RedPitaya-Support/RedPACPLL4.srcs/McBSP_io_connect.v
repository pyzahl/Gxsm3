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
// Description: get top most bits from config
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module McBSP_io_connect #(
)
(
    // inout logic [ 8-1:0] exp_p_io,
    inout  [8-1:0] exp_p_io,
    inout  [8-1:0] exp_n_io,
    input  McBSP_tx,   // TX: data transmit
    input  McBSP_fsx,  // optional, debug: data frame start FSX
    input  McBSP_frm,  // optional, debug: data frame
    output McBSP_clk,  // McBSP clock
    output McBSP_fs,   // McBSP FS (frame start)
    output McBSP_rx,   // McBSP RX (data receive)
    output McBSP_nrx, // optional: Nth/other RedPitaya slaves data TX forwarded on scheme TDB
    output [8-1:0] RP_exp_in
);

wire dummy;

IOBUF clk_iobuf (.O(McBSP_clk), .IO(exp_p_io[0:0]), .I(0),         .T(1) );
IOBUF fs_iobuf  (.O(McBSP_fs),  .IO(exp_p_io[1:1]), .I(0),         .T(1) );
IOBUF rx_iobuf  (.O(McBSP_rx),  .IO(exp_p_io[2:2]), .I(0),         .T(1) );
IOBUF tx_iobuf  (.O(dummy),     .IO(exp_p_io[3:3]), .I(McBSP_tx),  .T(0) );
IOBUF fsx_iobuf (.O(dummy),     .IO(exp_p_io[4:4]), .I(McBSP_fsx), .T(0) );
IOBUF frm_iobuf (.O(dummy),     .IO(exp_p_io[5:5]), .I(McBSP_frm), .T(0) );
IOBUF nrx_iobuf (.O(McBSP_nrx), .IO(exp_p_io[7:7]), .I(0),         .T(1) );

IOBUF exp_in_iobuf[8-1:0] (.O(RP_exp_in), .IO(exp_n_io), .I(8'b00000000),    .T(8'b11111111) );


endmodule
