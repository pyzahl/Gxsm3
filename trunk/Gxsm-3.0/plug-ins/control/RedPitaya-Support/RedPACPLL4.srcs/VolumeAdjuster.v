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

//SineOut=(int)(((long long)cr1 * (long long)volumeSine)>>22); // Volume Q22


module VolumeAdjuster(
    input signed [31:0] tdata_in,
    input signed [31:0] vol, // Q22
    output signed [31:0] tdata_out
   );
/*
   reg signed [31:0] y;
   always @*
   begin
        y = (vol*tdata_in) >>> 22; // 31+22
   end
*/
   assign tdata_out = (vol*tdata_in) >>> 22;
endmodule
