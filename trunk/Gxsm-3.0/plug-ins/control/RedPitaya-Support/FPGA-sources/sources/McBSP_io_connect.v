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
    parameter USE_RP_DIGITAL_IO = 0
)
(
    // inout logic [ 8-1:0] exp_p_io,
    inout  [8-1:0] exp_p_io,
    inout  [8-1:0] exp_n_io,
    input  McBSP_clkr, // CLKR: clock return
    input  McBSP_tx,   // TX: data transmit
    input  McBSP_fsx,  // optional, debug: data frame start FSX
    input  McBSP_frm,  // optional, debug: data frame
(* X_INTERFACE_PARAMETER = "FREQ_HZ 20000000" *)
    output McBSP_clk,  // McBSP clock
    output McBSP_fs,   // McBSP FS (frame start)
    output McBSP_rx,   // McBSP RX (data receive)
    output McBSP_nrx,  // optional: Nth/other RedPitaya slaves data TX forwarded on scheme TDB
    output [8-1:0] RP_exp_in,
    output [4-1:0] McBSP_pass
    // output [4-1:0] McBSP_dbg
);

//  [Place 30-574] Poor placement for routing between an IO pin and BUFG. If this sub optimal condition is acceptable for this design, you may use the CLOCK_DEDICATED_ROUTE constraint in the .xdc file to demote this message to a WARNING. However, the use of this override is highly discouraged. These examples can be used directly in the .xdc file to override this clock rule.
//  set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets system_i/PS_data_transport/McBSP_io_connect_0/inst/clk_iobuf/O]
// => complains about using non clk dedicated IO pin for McBSP Clk.
// https://www.xilinx.com/support/documentation/sw_manuals/xilinx2012_2/ug953-vivado-7series-libraries.pdfhttps://www.xilinx.com/support/documentation/sw_manuals/xilinx2012_2/ug953-vivado-7series-libraries.pdf

//[Place 30-574] Poor placement for routing between an IO pin and BUFG. This is normally an ERROR but the CLOCK_DEDICATED_ROUTE constraint is set to FALSE allowing your design to continue. The use of this override is highly discouraged as it may lead to very poor timing results. It is recommended that this error condition be corrected in the design.
//
//	system_i/PS_data_transport/McBSP_io_connect_0/inst/clk_iobuf/IBUF (IBUF.O) is locked to IOB_X0Y68
//	 and system_i/PS_data_transport/McBSP_io_connect_0/inst/McBSP_clk_BUFG_inst (BUFG.I) is provisionally placed by clockplacer on BUFGCTRL_X0Y3


// Re: WARNING: [Constraints 18-550] Could not create 'IOSTANDARD' constraint because net 'McBSP_clk' is not directly connected to top level port. 'IOSTANDARD' is ignored by Vivado but preserved for

// IOBUF macro, .T(0) : Output direction. IO = I, O = I (passed)
// IOBUF macro, .T(1) : Input direction.  IO = Z (high imp), O = IO (passed), I=X

IOBUF clk_iobuf (.O(McBSP_clk),      .IO(exp_p_io[0]), .I(0),         .T(1) );
IOBUF fs_iobuf  (.O(McBSP_fs),       .IO(exp_p_io[1]), .I(0),         .T(1) );
IOBUF rx_iobuf  (.O(McBSP_rx),       .IO(exp_p_io[2]), .I(0),         .T(1) );
IOBUF tx_iobuf  (.O(McBSP_pass[0]),  .IO(exp_p_io[3]), .I(McBSP_tx),  .T(0) );
IOBUF fsx_iobuf (.O(McBSP_pass[1]),  .IO(exp_p_io[4]), .I(McBSP_fsx), .T(0) );
IOBUF frm_iobuf (.O(McBSP_pass[2]),  .IO(exp_p_io[5]), .I(McBSP_frm), .T(0) );
IOBUF clkr_iobuf(.O(McBSP_pass[3]),  .IO(exp_p_io[6]), .I(McBSP_clkr),.T(0) );
//OBUF tx_obuf  (.O(exp_p_io[3]), .I(McBSP_tx));
//OBUF fsx_obuf (.O(exp_p_io[4]), .I(McBSP_fsx));
//OBUF frm_obuf (.O(exp_p_io[5]), .I(McBSP_frm));
//OBUF clkr_obuf(.O(exp_p_io[6]), .I(McBSP_clkr));
IOBUF nrx_iobuf (.O(McBSP_nrx),      .IO(exp_p_io[7]), .I(0),         .T(1) );

if (USE_RP_DIGITAL_IO)
begin
    IOBUF exp_in_iobuf[8-1:0] (.O(RP_exp_in[8-1:0]), .IO(exp_n_io[8-1:0]), .I(8'b00000000),    .T(8'b11111111) );
end
else
begin
    assign RP_exp_in = 0;
end

// assign McBSP_dbg = { McBSP_clk, McBSP_fs, McBSP_rx, McBSP_nrx };


endmodule


/*

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_clk' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_clk' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_clk' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_clkr' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_clkr' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_clkr' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_frm' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_frm' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_frm' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_fs' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_fs' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_fs' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_fsx' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_fsx' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_fsx' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_nrx' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_nrx' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_nrx' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[0]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[0]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[0]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[1]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[1]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[1]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[2]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[2]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[2]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[3]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[3]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_pass[3]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_rx' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_rx' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_rx' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_tx' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_tx' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/McBSP_tx' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[0]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[0]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[0]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[1]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[1]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[1]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[2]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[2]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[2]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[3]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[3]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[3]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[4]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[4]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[4]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[5]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[5]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[5]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[6]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[6]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[6]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

[Constraints 18-550] Could not create 'DRIVE' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[7]' is not directly connected to top level port. Synthesis is ignored for DRIVE but preserved for implementation.

[Constraints 18-550] Could not create 'IBUF_LOW_PWR' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[7]' is not directly connected to top level port. Synthesis is ignored for IBUF_LOW_PWR but preserved for implementation.

[Constraints 18-550] Could not create 'SLEW' constraint because net 'system_i/PS_data_transport/McBSP_io_connect_0/RP_exp_in[7]' is not directly connected to top level port. Synthesis is ignored for SLEW but preserved for implementation.

*/