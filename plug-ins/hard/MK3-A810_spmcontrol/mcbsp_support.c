/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 8 c-style: "K&R" -*- */

/* SRanger and Gxsm - Gnome X Scanning Microscopy Project
 * universal STM/AFM/SARLS/SPALEED/... controlling and
 * data analysis software
 *
 * DSP tools for Linux
 *
 * Copyright (C) 1999,2000,2001,2002 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * WWW Home:
 * DSP part:  http://sranger.sf.net
 * Gxsm part: http://gxsm.sf.net
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

#include "SR3_Reg.h"
#include "g_intrinsics.h"
#include "FB_spm_dataexchange.h"

#define Time1_us 84 // In the assembler function _wait, the value to set to obtain 1 us is 84. It is 7 CPU cycles * 84 at 589 MHz.
                    // *** =99 for 688 MHz

#define SPI_Tx_Bit 0x0080 // INT7
#define SPI_Rx_Bit 0x0100 // INT8
#define SPI_RxTx_Bit 0x0180 // INT8 and INT7


extern ANALOG_VALUES    analog;


#define MAX_SPI_FRAMES    8
int SPI_words_per_frame = 4;
int SPI_frame_element = 0;
int SPI_frame_count = 0;
int SPI_enabled = 0;

unsigned int SPI_transmitted_words = 0;
unsigned int SPI_received_words = 0;

unsigned int SPI_test_count=0;

#pragma CODE_SECTION(SetSPIwords, ".text:slow")
void SetSPIwords(int n){
        if (n >= 0 && n <= MAX_SPI_FRAMES)
                SPI_words_per_frame = n; 
        else
                SPI_words_per_frame = MAX_SPI_FRAMES;      
}

#pragma CODE_SECTION(ResetMcBSP0, ".text:slow")
void ResetMcBSP0()
{
	MCBSP0_SPCR=(3<<CLKSTPBit);	// Put the McBSP0 in reset
        SPI_enabled = 0;
        SPI_words_per_frame = 4; 
        SPI_frame_count = 0;
        SPI_test_count = 0;
}

#pragma CODE_SECTION(InitMcBSP0_InSPIMode, ".text:slow")
void InitMcBSP0_InSPIMode()
{
	// STEP 1: Power up the SP

	CFG_VDD3P3V_PWDN &= ~SetSPPD; // 0x80 : bit SP(mcbsp0,1)

	// STEP 2

	MCBSP0_SPCR=(3<<CLKSTPBit);	// Put the McBSP0 in reset
        // CLKSTP=3 Clock starts with rising edge with delay

	// STEP 3

        // Configure: Pin Control Register PCR
        // CLKRPBit   0 = 0   receive data sampled on falling edge
        // CLKXPBit   1 = 0   rising clock pol
        // FSRPBit    2 = 0   receive fram pol pos
        // FSXPBit    3 = 0   pos Frame Pol
        // DRSTATBit  4
        // DXSTATBit  5
        // CLKSTATBit 6
        // CLKRMBit 8   = 0  CLKR is input (returned clock!! need this)
        // CLKXMBit 9   = 1   
        // FSXMBit  11  = 1  Frame Sync by FSGM bit in SRGM, 0: external source ??
        // RIOENBit 12  = 0
        // XIOENBit 13  = 0   DX, FSX, CLKX active

        // CLKX and FSX must be output
        // -> CLKXM=1, FSXM=1
        // CLKR and FSR must be input (reference clock/frame from clock/frame return over wire):
        // -> CLKRM=0, FSRM=0 
        MCBSP0_PCR = (1 << PCR_CLKXMBit) + (1 << FSXMBit);
        

        
	// Set the SPI (clock: Internal clock at SYSClk3 (CPU (589E6)/6): diviser /7: 14.024 MHz)

	//MCBSP0_SRGR=(1<<CLKSMBit) + (7<<FPERBit)+ (6<<CLKGDVBit); // 8 clocks per frame (8bit)!
	//MCBSP0_SRGR=(1<<CLKSMBit) + (34<<FPERBit)+ (99<<CLKGDVBit) + (1 << FSGMBit); // 1 MHz, 34 clocks per frame (32 bit word)
	//MCBSP0_SRGR=(1<<CLKSMBit) + (34<<FPERBit)+ (49<<CLKGDVBit); //  2 MHz
	//MCBSP0_SRGR=(1<<CLKSMBit) + (34<<FPERBit)+ (24<<CLKGDVBit); //  4 MHz
	MCBSP0_SRGR=(1<<CLKSMBit) + (34<<FPERBit)+ (12<<CLKGDVBit) + (1 << FSGMBit); //  8 MHz
	//MCBSP0_SRGR=(1<<CLKSMBit) + (34<<FPERBit)+ (6<<CLKGDVBit); //  16 MHz
	//MCBSP0_SRGR=(1<<CLKSMBit) + (34<<FPERBit)+ (6<<CLKGDVBit); //  20 MHz
							// FSGM=0
							// CLKSP=GSYNC=0 (don't care in internal input clock mode)
							// CLKSM=1 (Internal input clock)
							// FSGM=0
							// FPER=7 (8-bits by frame)
							// FWID=0
							// CLKGDV=6 (divider at 7 to generate 14.024 MHz)

	// want 32bit, 4 frames default
	MCBSP0_RCR=(1<<RDATDLYBit) + (5<<RWDLEN1Bit) + (4 << RFRLEN1Bit);	// 32-bits word (phase1), 1 words by frame, Receive 1-bit delay
	MCBSP0_XCR=(1<<XDATDLYBit) + (5<<XWDLEN1Bit) + (4 << XFRLEN1Bit);	// 32-bits word (phase1), 1 words by frame, Transmit 1-bit delay
							// RPHASE=XPHASE=0
							// RFRLEN1=XFRLEN1=0
							// RCOMPAND=0
							// RFIG=0
							// RDATDLY=XDATDLY=1
							// RFRLEN1=XFRLEN1=0 1 word of RWDLEN1-bits
							// XFRLEN1=XFRLEN1=0 1 word of XWDLEN1-bits
							// RWDLEN1=XWDLEN1=0 (8-bits) =5 (32-bits)
							// XWDLEN1=XWDLEN1=0 (8-bits) =5 (32-bits)
	MCBSP0_PCR=(1<<CLKXMBit) + (1<<FSXMBit) + (0<<CLKXPBit) + (0<<CLKRPBit) + (0<<FSXPBit);
							// FSXM=1
							// FSRM=0
							// CLKXM=1 McBSP master and generates clkx for slave and for its receive clock
							// CLKRM=0 CLKR set as input
							// SCLKME=0
							// FSXP=0
							// FSRP=0
							// CLKXP=0 (transmit on rising edge of CLKX)
							// CLKRP=0 (receive on falling edge of CLKR)

	// STEP 4: wait 2 clks (we do 10)

	// Wait for the synchronization with CLKS
	// (10 cyles of internal clock (12.288MHz): 1 us)

	wait(Time1_us);			// wait 1 us

	// STEP 5: GRST=1 Sample-rate generator enable (wait the double of step 4)

	MCBSP0_SPCR |= MCBSP_SPCR_GRST;
	wait(2*Time1_us);

	// STEP 6: XRST=1, wait like step 5 and XRST=0

	MCBSP0_SPCR |= MCBSP_SPCR_XRST;
	wait(2*Time1_us);
	MCBSP0_SPCR &= ~MCBSP_SPCR_XRST;

        //MCBSP0_SPCR &= ~(3 << XINTMBit); // make sure it is =0 for XINT driven by XRDY (end of word) and end of frame
        //MCBSP0_SPCR &= ~(3 << RINTMBit); // make sure it is =0 for RINT driven by RRDY (end of word) and end of frame
        
	// STEP 7: Init EDMA (no DMA)

	// STEP 8: XRST=RRST=1 enable Transmitter and enable Receiver.

	MCBSP0_SPCR |= (MCBSP_SPCR_XRST+MCBSP_SPCR_RRST+MCBSP_SPCR_DLB); // DLBBit ?? [digital loopback]

	// 9: Nothing to do

	// 10: wait

	wait(2*Time1_us);
	// Init Mux1 and Mux2 for McBSP1 tx (int8) rx (int7)
	//INTC_INTMUX1=(INTC_INTMUX1 & 0x00FFFFFF) | 0x32000000;		// Add the MBXINT1 event (51:32h) on INT7
	//INTC_INTMUX2=(INTC_INTMUX2 & 0xFFFFFF00) | 0x00000033;		// Add the MBRINT1 event (50:33h) on INT8

	// tms320c6424.pdf p. 167
	// 48 MBXINT0 McBSP0 Transmit    = 0x30
	// 49 MBRINT0 McBSP0 Receive     = 0x31
	// 50 MBXINT1 McBSP1 Transmit    = 0x32
	// 51 MBRINT1 McBSP1 Receive     = 0x33

	// Init Mux1 and Mux2 for McBSP0 TX event MBXINT0 # d48 on (int7)  and RX MBRINT0 # d 49 on (int8)
	INTC_INTMUX1=(INTC_INTMUX1 & 0x00FFFFFF) | 0x30000000;		// Add the MBXINT1 event (51:30h) on INT7 ->TX
	INTC_INTMUX2=(INTC_INTMUX2 & 0xFFFFFF00) | 0x00000031;		// Add the MBRINT1 event (50:31h) on INT8 ->RX

        // DisableInts_SDB;
        CSR = ~0x1 & CSR; // Disable INTs 

	IER &= ~SPI_RxTx_Bit; // Disable the SPI int RX/TX

        // EnableInts_SDB;
        CSR = 1 | CSR; // Enable ints 

	ICR=SPI_RxTx_Bit; // Clear flag

	// 11: FRST=1 (enable frame-syn generator)

	MCBSP0_SPCR |= MCBSP_SPCR_FRST;

        SPI_words_per_frame = 4; 
        SPI_frame_count = 0;
        SPI_test_count = 0;
        SPI_enabled = 1;
}

/* During a serial transfer, there are typically periods of serial port inactivity between packets or transfers.
   The receive and transmit frame synchronization pulse occurs for every serial transfer. When the McBSP is
   not in the reset state and has been configured for the desired operation, a serial transfer can be initiated
   by programming (R/X)PHASE = 0 for a single-phase frame with the required number of elements
   programmed in (R/X)FRLEN1. The number of elements can range from 1 to 128 ((R/X)FRLEN1 = 00h to
   7Fh). The required serial element length is set in the (R/X)WDLEN1 field in the (R/X)CR. If a dual-phase
   frame is required for the transfer, RPHASE = 1 and each (R/X)FRLEN1/2 can be set to any value between
   0h and 7Fh.
*/

void start_McBSP_transfer(){
        if (!SPI_enabled)
                return;

        //(R/X)PHASE = 0, specifying a single-phase frame
        //(R/X)FRLEN1 = 0b, specifying one element per frame
        //(R/X)WDLEN1 = 000b, specifying eight bits per element
        //(R/X)FRLEN2 = (R/X)WDLEN2 = Value is ignored
        // CHECK:  XRSTBit in SPCR     // Transmitter reset bit resets or enables the transmitter.
        // CHECK:  XRDYBit in SPCR     // Transmitter ready bit.
        // CHECK: RSYNCERRBit in SPCR  // Receive synchronization error bit.
        // MAY RESET: FRSTBit in SPCR  // Frame-sync generator reset bit.

        SPI_frame_element = 0; // reset receiver frame element count
        SPI_frame_count =  SPI_words_per_frame-1;

        // check for error, reset
#if 0
        if (MCBSP0_SPCR & (MCBSP_SPCR_XSYNCERR | MCBSP_SPCR_RSYNCERR)){
                MCBSP0_SPCR |= MCBSP_SPCR_XRST;
                wait(2*Time1_us);
                MCBSP0_SPCR &= ~MCBSP_SPCR_XRST;
        }
#endif
        // 11: FRST=1 (enable frame-syn generator)
        //MCBSP0_SPCR &= ~MCBSP_SPCR_FRST;
	//MCBSP0_SPCR |= MCBSP_SPCR_FRST;

	MCBSP0_RCR=(1<<RDATDLYBit) + (5<<RWDLEN1Bit) + (SPI_frame_count << RFRLEN1Bit);	// 32-bits word (phase1), 1 words by frame, Receive 1-bit delay
	MCBSP0_XCR=(1<<XDATDLYBit) + (5<<XWDLEN1Bit) + (SPI_frame_count << XFRLEN1Bit);	// 32-bits word (phase1), 1 words by frame, Transmit 1-bit delay
        MCBSP0_DXR_32BIT = 0xF0F00001; // Initial Word

        // Debug
        if (SPI_test_count){
                analog.McBSP_SPI[3] = SPI_transmitted_words;
                analog.McBSP_SPI[4] = SPI_received_words;
                analog.McBSP_SPI[5] = MCBSP0_PCR;
                analog.McBSP_SPI[6] = MCBSP0_SPCR;
                analog.McBSP_SPI[7] = MCBSP0_DRR_32BIT;
        }
}

// McBSP [SPI] TX (Transmit)
interrupt void McBSP1TX_INT()
{
        if (SPI_frame_count){
                MCBSP0_DXR_32BIT = SPI_frame_count--;
                SPI_transmitted_words++;
        }
}

// McBSP [SPI] RX (Receive)
interrupt void McBSP1RX_INT()
{
        if (SPI_frame_element < SPI_words_per_frame ){
                analog.McBSP_SPI[SPI_frame_element++] = MCBSP0_DRR_32BIT;
                SPI_received_words++;
        }
        //else
                // 11: FRST=0 (disable frame-syn generator)
                // MCBSP0_SPCR &= ~MCBSP_SPCR_FRST;
}

void TestMcBSP0()
{
        if (!SPI_enabled)
                return;

        SPI_words_per_frame = 2;
        SPI_transmitted_words = 0;
        SPI_received_words = 0;
        SPI_test_count=1;
}
