/*
- For the defines see SR3_Ref.h file
- The function wait() must be already in your project. If not you can use this function in ASM:



2) For the TX/RX int number for McBSP0, see page 167/243 of the attached file. The TX is number 50d and Rx is 49d. For the rest of the code and to change from McBSP1 to McBSP0, you will have to change the register from *BSP1 to *BSP0.

For adding the interrupt vectors for the TX/RX of BSP0. You have to edit your vector.asm:

;INT7 (MBXINT1)
_VectINT7: 

STW .D2T2 B10,*B15--[2] ; An absolute branch (or call) is used to avoid
 || MVKL .S2     _McBSP1TX_INT,B10 ; far trampoline sections when the ISR is far (more
MVKH .S2     _McBSP1TX_INT,B10 ; than 21-bits address from the current PC)
B .S2     B10 ; Note that the B15 (software stack) is necessary
     LDW .D2T2   *++B15[2],B10
NOP     4
NOP
NOP

;INT8 (MBRINT1)
_VectINT8: 

STW .D2T2 B10,*B15--[2] ; An absolute branch (or call) is used to avoid
 || MVKL .S2     _McBSP1RX_INT,B10 ; far trampoline sections when the ISR is far (more
MVKH .S2     _McBSP1RX_INT,B10 ; than 21-bits address from the current PC)
B .S2     B10 ; Note that the B15 (software stack) is necessary
     LDW .D2T2   *++B15[2],B10
NOP     4
NOP
NOP

In your C file:

interrupt void McBSP1TX_INT()

{

}

interrupt void McBSP1RX_INT()

{

}

 


;***************************************************************************
; wait function (delay in A4) : 7 cycles by delay value
; 17 CPU cyles of overhead
;***************************************************************************

        .global _wait   
            	
_wait:

	 MV      .L1     A4,A0              
	
waitLoop_1:
           ADD    .S1     0xffffffff,A0,A0     
   [ A0]   BNOP    .S1     waitLoop_1,5      

	; Return

	B	.S2		B3
	NOP			5

- You have to add the INT7 and INT8 in your file vector.asm (probably you will only use the Rx INT in your case)
*/


#include "SR3_Reg.h"

void InitMcBSP0_InSPIMode()
{
	// STEP 1: Power up the SP

	CFG_VDD3P3V_PWDN &= ~SetSPPD; // 0x80 : bit SP(mcbsp0,1)

	// STEP 2

	MCBSP1_SPCR=(3<<CLKSTPBit);	// Put the McBSP0 in reset
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
	MCBSP0_RCR=(1<<RDATDLYBit);	// 8-bits word (phase1), 1 words by frame, Receive 1-bit delay
	MCBSP0_XCR=(1<<XDATDLYBit);	// 8-bits word (phase1), 1 words by frame, Transmit 1-bit delay
							// RPHASE=XPHASE=0
							// RFRLEN1=XFRLEN1=0
							// RCOMPAND=0
							// RFIG=0
							// RDATDLY=XDATDLY=1
							// RFRLEN1=XFRLEN1=0 1 word of 8-bit
							// RWDLEN1=XWDLEN1=0 (8-bits)
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

	DisableInts_SDB;
	IER &= ~SPI_RxTx_Bit; // Disable the SPI int RX/TX
	EnableInts_SDB;
	ICR=SPI_RxTx_Bit; // Clear flag

	// 11: FRST=1 (enable frame-syn generator)

	MCBSP0_SPCR |= MCBSP_SPCR_FRST;

}
