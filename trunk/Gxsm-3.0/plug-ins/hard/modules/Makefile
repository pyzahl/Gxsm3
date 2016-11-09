# Makefile for additional kernel modules

# Path to linux kernel source:
KERNSRC   = /usr/src/linux
HPATH     = $(KERNSRC)/include

HOSTCC          =gcc
HOSTCFLAGS      =-Wall -Wstrict-prototypes -O2 -fomit-frame-pointer

CROSS_COMPILE   =

AS      =$(CROSS_COMPILE)as
LD      =$(CROSS_COMPILE)ld
CC      =$(CROSS_COMPILE)gcc -D__KERNEL__ -I$(HPATH)
CPP     =$(CC) -E
AR      =$(CROSS_COMPILE)ar
NM      =$(CROSS_COMPILE)nm
STRIP   =$(CROSS_COMPILE)strip
OBJCOPY =$(CROSS_COMPILE)objcopy
OBJDUMP =$(CROSS_COMPILE)objdump
MAKE    =make
GENKSYMS=/sbin/genksyms

all:  ccd.o dspspaemu.o dspbbspa.o dspspmemu.o pci32.o pc31.o

#CFLAGS = -Wall -Wstrict-prototypes -O2 -msoft-float -fomit-frame-pointer -I../../..
CFLAGS = -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer -I../../..

# use '-fno-strict-aliasing', but only if the compiler can take it
CFLAGS += $(shell if $(CC) -fno-strict-aliasing -S -o /dev/null -xc /dev/null >/dev/null 2>&1; then echo "-fno-strict-aliasing"; fi)

MODFLAGS += -DMODULE
MODFLAGS += -DMODVERSIONS -include $(HPATH)/linux/modversions.h

MATHFLAGS = -static -lm

ccd.o:	ccd.c ccd.h
	$(CC) $(CFLAGS) $(MODFLAGS) -DCCD_01 -c ccd.c

dspspaemu.o:	dspemu.c spaleed_emu.c
	$(CC) $(CFLAGS) $(MODFLAGS) -DEMULATE_A_SPALEED -c dspemu.c -o dspspaemu.o

dspbbspa.o:	dspemu.c spaleed_bb.c
	$(CC) $(CFLAGS) $(MODFLAGS) $(MATHFLAGS) -DEMULATE_DSP_BB_SPALEED -c dspemu.c -o dspbbspa.o

dspspmemu.o:	dspemu.c spm_emu.c
	$(CC) $(CFLAGS) $(MODFLAGS) -DEMULATE_DSP_SPM -c dspemu.c -o dspspmemu.o

pci32.o:pci32.c pci32.h dsp.h dbgstuff.h mbox.h
	$(CC) $(CFLAGS) $(MODFLAGS) -DCARD_PCI32 -c pci32.c 

pc31.o:pci32.c pci32.h pc31.h dsp.h dbgstuff.h mbox.h
	$(CC) $(CFLAGS) $(MODFLAGS) -DCARD_PC31 -c pci32.c -o pc31.o

install:
	@echo ---------------------------------------------------------------------------
	@echo Important Notice:
	@echo  - No automatic install of any hardware support modules!
	@echo  - Please set it up manually for your own needs, refer to the manual.
	@echo ---------------------------------------------------------------------------	
clean:
	rm *.o *~ core
