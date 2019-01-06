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

extern ANALOG_VALUES    analog;


/*
;***************************************************************************
; wait function (delay in A4) : 7 cycles by delay value
; 17 CPU cyles of overhead
;***************************************************************************
*/


#define Time1_us 7 // 7 cycles per ~1us    (150us : 3687 cycles at 24.576MHz x 2)

#define MAX_SPI_FRAME_LEN      8
int SPI_frame_element = 0;
int SPI_enabled = 0;

void ResetMcBSP0()
{
	MCBSP0_SPCR=(3<<CLKSTPBit);	// Put the McBSP0 in reset
        SPI_enabled = 0;
}

void InitMcBSP0_InSPIMode()
{
	// STEP 1: Power up the SP

	CFG_VDD3P3V_PWDN &= ~SetSPPD; // 0x80 : bit SP(mcbsp0,1)

	// STEP 2

	MCBSP0_SPCR=(3<<CLKSTPBit);	// Put the McBSP0 in reset
        // CLKSTP=3 Clock starts with rising edge with delay

	// STEP 3

	// Set the SPI (clock: Internal clock at SYSClk3 (CPU (589E6)/6): diviser /7: 14.024 MHz)

	//MCBSP0_SRGR=(1<<CLKSMBit) + (7<<FPERBit)+ (6<<CLKGDVBit);
	//MCBSP0_SRGR=(1<<CLKSMBit) + (7<<FPERBit)+ (99<<CLKGDVBit); // 1 MHz
	//MCBSP0_SRGR=(1<<CLKSMBit) + (7<<FPERBit)+ (49<<CLKGDVBit); //  2 MHz
	//MCBSP0_SRGR=(1<<CLKSMBit) + (7<<FPERBit)+ (24<<CLKGDVBit); //  4 MHz
	MCBSP0_SRGR=(1<<CLKSMBit) + (7<<FPERBit)+ (12<<CLKGDVBit); //  8 MHz
	//MCBSP0_SRGR=(1<<CLKSMBit) + (7<<FPERBit)+ (6<<CLKGDVBit); //  16 MHz
	//MCBSP0_SRGR=(1<<CLKSMBit) + (7<<FPERBit)+ (6<<CLKGDVBit); //  20 MHz
							// FSGM=0
							// CLKSP=GSYNC=0 (don't care in internal input clock mode)
							// CLKSM=1 (Internal input clock)
							// FSGM=0
							// FPER=7 (8-bits by frame)
							// FWID=0
							// CLKGDV=6 (divider at 7 to generate 14.024 MHz)

	// want 32bit
	MCBSP0_RCR=(1<<RDATDLYBit) + (5<<RWDLEN1Bit);	// 32-bits word (phase1), 1 words by frame, Receive 1-bit delay
	MCBSP0_XCR=(1<<XDATDLYBit) + (5<<XWDLEN2Bit);	// 32-bits word (phase1), 1 words by frame, Transmit 1-bit delay
							// RPHASE=XPHASE=0
							// RFRLEN1=XFRLEN1=0
							// RCOMPAND=0
							// RFIG=0
							// RDATDLY=XDATDLY=1
							// RFRLEN1=XFRLEN1=0 1 word of 8-bit
							// RWDLEN1=XWDLEN1=0 (8-bits) =5 (32-bits)
							// XWDLEN2=XWDLEN1=0 (8-bits) =5 (32-bits)
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

	// STEP 7: Init EDMA (no DMA)

	// STEP 8: XRST=RRST=1 enable Transmitter and enable Receiver.

	MCBSP0_SPCR |= (MCBSP_SPCR_XRST+MCBSP_SPCR_RRST);

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

#if 0 // missing code:
	DisableInts_SDB;
	IER &= ~SPI_RxTx_Bit; // Disable the SPI int RX/TX
	EnableInts_SDB;
	ICR=SPI_RxTx_Bit; // Clear flag
#endif
	// 11: FRST=1 (enable frame-syn generator)

	MCBSP0_SPCR |= MCBSP_SPCR_FRST;

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

        MCBSP0_DXR_32BIT = 0x10002000; // Test Word

}

// McBSP [SPI] TX (Transmit)
interrupt void McBSP1TX_INT()
{
        MCBSP0_DXR_32BIT = 0x00000030;
}

// McBSP [SPI] RX (Receive)
interrupt void McBSP1RX_INT()
{
        if (SPI_frame_element < MAX_SPI_FRAME_LEN)
                analog.McBSP_SPI[SPI_frame_element++] = MCBSP0_DRR_32BIT;
}

void TestMcBSP0()
{
        if (!SPI_enabled)
                return;

        start_McBSP_transfer();
}
