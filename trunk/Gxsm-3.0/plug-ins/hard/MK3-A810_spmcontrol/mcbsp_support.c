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
#include "mcbsp_support.h"

#define Time1_us 84 // In the assembler function _wait, the value to set to obtain 1 us is 84. It is 7 CPU cycles * 84 at 589 MHz.
                    // *** =99 for 688 MHz

#define SPI_Tx_Bit 0x0080 // INT7
#define SPI_Rx_Bit 0x0100 // INT8
#define SPI_RxTx_Bit (SPI_Rx_Bit | SPI_Tx_Bit) // INT8 and INT7


extern ANALOG_VALUES    analog;


#define FPGA_NUM_32B_WORDS  8
int SPI_words_per_frame = 8;
int SPI_frame_element = 0;
int SPI_word_count = 0;
int SPI_enabled = 0;
int SPI_clkdiv = 99; // Serial Clock 1MHz
int use_FSG = MCBSP_MODE_CTRL_FSG;
unsigned int FSonTXC_MCBSP0_SRGR;
unsigned int FSonFSG_MCBSP0_SRGR;

unsigned int SPI_tx_data = 0;
unsigned int SPI_transmitted_words = 0;
unsigned int SPI_received_words = 0;

unsigned int SPI_debug_mode=0;

#pragma CODE_SECTION(SetSPIwords, ".text:slow")
void SetSPIwords(int n){
        if (n >= 1 && n <= 8)
                SPI_words_per_frame = n; 
        else
                SPI_words_per_frame = 8;
}

#pragma CODE_SECTION(SetSPIclock, ".text:slow")
// McBSP (SPI) CLKGDV=div,  CLK = 100MHz / (CLKGDV+1)
void SetSPIclock(int div){
        if (div >= 0 && div <= 255)
                SPI_clkdiv = div; 
        else
                SPI_clkdiv = 99; // 1 MHz fallback
}

#pragma CODE_SECTION(ResetMcBSP0, ".text:slow")
void ResetMcBSP0()
{
	MCBSP0_SPCR=(3<<CLKSTPBit);	// Put the McBSP0 in reset
        SPI_enabled = 0;
        SPI_words_per_frame = 8;
        SPI_word_count = 0;
        SPI_debug_mode = 0;
        SPI_transmitted_words = 0;
        SPI_received_words = 0;
}

#pragma CODE_SECTION(InitMcBSP0_InSPIMode, ".text:slow")
// wpf: words per frame: 1..8
// mode: b0: DLB (digital loop back test mode), b12: SPI mode (clock stop)
void InitMcBSP0_InSPIMode(int wpf, int mode)
{
        SetSPIwords(wpf);
        use_FSG = mode;
        
        // DisableInts_SDB;
        CSR = ~0x1 & CSR; // Disable INTs.
        
	// STEP 1: Power up the SP

	CFG_VDD3P3V_PWDN &= ~SetSPPD; // 0x80 : bit SP(mcbsp0,1)

	// STEP 2

        // ====== McBSP Serial Port Control Register (SPCR)
	//MCBSP0_SPCR=(0<<CLKSTPBit);	// Put the McBSP0 in reset, regular McBSP mode, clock is running

        // CLKSTP=3 Clock starts with rising edge with delay [SPI mode]
        // MCBSP_SPCR_DLB // DLBBit [digital loopback test]
	// MCBSP0_SPCR=(3<<CLKSTPBit) + (0 << DLBBit);	// Put the McBSP0 in reset // 3= SPI (Clock Stop Mode)
	MCBSP0_SPCR = (0 << CLKSTPBit) + (0 << DLBBit);	// Put the McBSP0 in reset, McBSP default mode

	// STEP 3

        // Configure:
        // ====== Pin Control Register PCR
        // CLKRPBit   0 = 0   1: rising edge!    0: receive data sampled on falling edge of CLKR (input)
        // CLKXPBit   1 = 0   rising clock pol (transmit on rising edge of CLKX)
        // FSRPBit    2 = 0   receive fram pol pos
        // FSXPBit    3 = 0   pos Frame Pol
        // SCLKME     7 = 0  (sample rate gen...)
        // CLKRMBit   8 = 0  CLKR is input (returned clock!! need this)
        // CLKXMBit   9 = 1  McBSP master and generates clkx for slave and for its receive clock 
        // FSRMBit   10 = 0  0: is derived from external source. 1: internally, FSR is output
        // FSXMBit   11 = 1  Frame Sync Generation by FSGM bit in SRGM, 0: is derived from external source.
        // RIOENBit  12 = 0
        // XIOENBit  13 = 0   DX, FSX, CLKX active

        // CLKX and FSX must be output
        // -> CLKXM=1, FSXM=1
        // CLKR and FSR must be input (reference clock/frame from clock/frame return over wire):
        // -> CLKRM=0, FSRM=0 
	MCBSP0_PCR = (0 << CLKRPBit) + (1 << PCR_CLKXMBit) + (1 << PCR_FSXMBit) + (0 << PCR_CLKXPBit) + (0 << PCR_CLKRPBit) + (0 << PCR_FSXPBit);
        
        // ===== Sample Rate Generator Register SRGR
	// Set the SPI (clock: Internal clock at SYSClk3 (CPU (589E6)/6): diviser /7: 14.024 MHz)
        // FSGM=0 0: Transmit FSX is generated on every DXR->XSR copy, 1: FSX is driven by sample rate generator frame sync signal (FSG)
        // CLKSP=GSYNC=0 (don't care in internal input clock mode)
        // CLKSM=1 (Internal input clock)
        // FPER= (8x32-bit words by frame)  8*32-1   -- ??? total frame length for all words ???
        // FWID=0 =1: 2 CLK WIDE FS Pulse
        // CLKGDV=6 (divider at 7 to generate 14.024 MHz) CLK = 100MHz / (CLKGDV+1)
        // (99<<CLKGDVBit) // 1 MHz clock
        // (49<<CLKGDVBit) // 2 MHz clock
        // (12<<CLKGDVBit) // 8 MHz clock **
        // (24<<CLKGDVBit) // 4 MHz clock
        // (6<<CLKGDVBit)  // 16 MHz clock,   (6<<CLKGDVBit); //  20 MHz ???
	//MCBSP0_SRGR=(1<<CLKSMBit) + ((8*32-1)<<FPERBit)+ (99<<CLKGDVBit) + (0 << FSGMBit) + (1 << FWIDBit); // 1MHz
	FSonTXC_MCBSP0_SRGR = (1<<CLKSMBit) + ((8*32-1+8)<<FPERBit)+ (SPI_clkdiv<<CLKGDVBit) + (0 << FSGMBit) + (1 << FWIDBit); // FSGMBit=0: Transmit FSX is generated on every DXR->XSR copy
	FSonFSG_MCBSP0_SRGR = FSonTXC_MCBSP0_SRGR + (1 << FSGMBit); // FSGMBit=1: FSX is driven by sample rate generator frame sync signal (FSG)
        MCBSP0_SRGR = use_FSG & MCBSP_MODE_FTXC ? FSonTXC_MCBSP0_SRGR : FSonFSG_MCBSP0_SRGR;
        
        // ===== McBSP Receive Control Register (RCR) and Transmit Control Register (XCR)
	// want 32bit, 4x 32bit words default, up to 8 words
	MCBSP0_RCR=(1<<RDATDLYBit) + (5<<RWDLEN1Bit) + ((SPI_words_per_frame-1) << RFRLEN1Bit);	// 32-bits word (phase1), 1 words by frame, Receive 1-bit delay
        MCBSP0_XCR=(1<<XDATDLYBit) + (5<<XWDLEN1Bit) + ((SPI_words_per_frame-1) << XFRLEN1Bit);	// 32-bits word (phase1), 1 words by frame, Transmit 1-bit delay
        // RPHASE=XPHASE=0
        // RFRLEN1=XFRLEN1=0
        // RCOMPAND=0
        // RFIG=0
        // RDATDLY=XDATDLY=1
        // RFRLEN1=XFRLEN1=0 1 word of RWDLEN1-bits
        // XFRLEN1=XFRLEN1=0 1 word of XWDLEN1-bits
        // RWDLEN1=XWDLEN1=0 (8-bits) =5 (32-bits)
        // XWDLEN1=XWDLEN1=0 (8-bits) =5 (32-bits)

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

        // should be OK as set above, but make sure
        MCBSP0_SPCR &= ~(3 << XINTMBit); // make sure it is =0 for XINT driven by XRDY (end of word) and end of frame
        MCBSP0_SPCR &= ~(3 << RINTMBit); // make sure it is =0 for RINT driven by RRDY (end of word) and end of frame
        
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

        // DisableInts_SDB;
        // CSR = ~0x1 & CSR; // Disable INTs 

	IER &= ~SPI_RxTx_Bit; // Disable RX/TX int

        // EnableInts_SDB;
        CSR = 1 | CSR; // Enable ints 

	ICR=SPI_RxTx_Bit; // Clear flag

	// 11: FRST=1 (enable frame-syn generator)

        if (use_FSG & (MCBSP_MODE_FSG | MCBSP_MODE_FTXC)){
                MCBSP0_SPCR |= MCBSP_SPCR_FRST;
        }
        // else: not yet! start at first transfer request
        
        SPI_word_count = 0;
        SPI_debug_mode = 0;
        SPI_enabled = 1;

        analog.McBSP_SPI[8] = MCBSP0_SPCR;
        analog.McBSP_SPI[9] = MCBSP0_PCR;
       
	IER |= SPI_RxTx_Bit; // Enable RX/TX int
}

void start_McBSP_transfer(unsigned int index){
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
        SPI_tx_data = index;

        // check for error, reset
#if 0
        if (SPI_debug_mode & MCBSP_MODE_AUTO_RECOVER)
                if (MCBSP0_SPCR & (MCBSP_SPCR_XSYNCERR | MCBSP_SPCR_RSYNCERR)){
                        MCBSP0_SPCR &= ~(MCBSP_SPCR_XRST | MCBSP_SPCR_RRST); // put RT and TX in reset
                        wait(2*Time1_us);
                        MCBSP0_SPCR |= MCBSP_SPCR_XRST | MCBSP_SPCR_RRST;
                }
#endif

#if 1
        if (use_FSG & (MCBSP_MODE_CTRL_FSG |  MCBSP_MODE_FTXC)){
                SPI_word_count =  SPI_words_per_frame-1;
                SPI_tx_data = 0;
                MCBSP0_DXR_32BIT = index; //0xF0F00001; // copy Initial Word now
        } else {
                SPI_word_count =  SPI_words_per_frame; // done via TX_INT
        }
#endif
        
        SPI_word_count =  SPI_words_per_frame; // done via TX_INT

        if (use_FSG &  MCBSP_MODE_CTRL_FSG)
                // take FSG out of reset, start frame transmission, stopped again after last word is out!
                MCBSP0_SPCR |= MCBSP_SPCR_FRST; // takes one frame to start!

        // Debug

        //After the first write into MCBSP0_DXR_32BIT, check the IFR register if the INT7 flag is ON?
        //It seems that no interrupt is generated. Check the XRDY bit in the SPCR register to see if the Tx has been done.
        //Also, check the XINTM set-up is the SPCR register. It should be at zero.

     
        switch (SPI_debug_mode & 0x0f){
        case 0: break;
        case 1:
                analog.McBSP_SPI[8] = MCBSP0_SPCR & (  (1 << XSYNCERRBit) | (1 << XEMPTYBit) | (1 << XRDYBit)
                                                     | (1 << RSYNCERRBit) | (1 << RFULLBit)  | (1 << RRDYBit));
                analog.McBSP_SPI[10] = SPI_transmitted_words;
                analog.McBSP_SPI[11] = SPI_received_words;
                break;
        case 2:
                analog.McBSP_SPI[3] = MCBSP0_SPCR;
                analog.McBSP_SPI[4] = MCBSP0_MCR;
                analog.McBSP_SPI[5] = MCBSP0_SPCR & (MCBSP_SPCR_XSYNCERR | MCBSP_SPCR_RSYNCERR);
                //analog.McBSP_SPI[5] = MCBSP0_DRR_32BIT;
                analog.McBSP_SPI[6] = SPI_transmitted_words;
                analog.McBSP_SPI[7] = SPI_received_words;
                analog.McBSP_SPI[8] = MCBSP0_SPCR;
                analog.McBSP_SPI[10] = SPI_transmitted_words;
                analog.McBSP_SPI[11] = SPI_received_words;
                break;
        case 3:
                analog.McBSP_SPI[5] = MCBSP0_DRR_32BIT;
                analog.McBSP_SPI[6] = SPI_transmitted_words;
                analog.McBSP_SPI[7] = SPI_received_words;
                analog.McBSP_SPI[8] = MCBSP0_SPCR;
                analog.McBSP_SPI[10] = SPI_transmitted_words;
                analog.McBSP_SPI[11] = SPI_received_words;
                break;
        }
}

// McBSP [SPI] TX (Transmit)
interrupt void McBSP1TX_INT()
{
        if (SPI_word_count){
                if (SPI_tx_data){
                        MCBSP0_DXR_32BIT = SPI_tx_data;
                        SPI_tx_data = 0;
                        SPI_word_count = SPI_words_per_frame - 1;
                }else
                        MCBSP0_DXR_32BIT = SPI_word_count--;
                SPI_transmitted_words++;
                if (SPI_word_count == 0 && (use_FSG & MCBSP_MODE_CTRL_FSG))
                        MCBSP0_SPCR &= ~MCBSP_SPCR_FRST; // stop FSG
        } else {
                MCBSP0_DXR_32BIT = 0xF0F2F0F1; // dummy mark
        }
}

// McBSP [SPI] RX (Receive)
interrupt void McBSP1RX_INT()
{
        if (SPI_frame_element >= SPI_words_per_frame ){
                SPI_frame_element = 0;
                analog.McBSP_SPI[15] = MCBSP0_DRR_32BIT;
        } else {
                analog.McBSP_SPI[SPI_frame_element++] = MCBSP0_DRR_32BIT;
                SPI_received_words++;
        }
}

#pragma CODE_SECTION(DebugMcBSP0, ".text:slow")
void DebugMcBSP0(int level)
{
        if (!SPI_enabled)
                return;

        SPI_transmitted_words = 0;
        SPI_received_words = 0;
        SPI_debug_mode=level;
}
