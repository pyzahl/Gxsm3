#/* SRanger and Gxsm - Gnome X Scanning Microscopy Project
# * universal STM/AFM/SARLS/SPALEED/... controlling and
# * data analysis software
# *
# * DSP tools for Linux
# *
# * Copyright (C) 1999-2011 Percy Zahl
# *
# * Authors: Percy Zahl <zahl@users.sf.net>
# * WWW Home:
# * DSP part:  http://sranger.sf.net
# * Gxsm part: http://gxsm.sf.net
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
# */
#
# --------------------------------------------------------------------------------
# Makefile to build the FB_spmcontrol.out DSP binary (COFF2 code), 
# you need to install the TI-C6x CC or newer to /opt.
# Get it from here: 
#
# Linux hosted 6x Code Generation Tools Linux-DSP-Tools-1.00.00.06 
# https://www-a.ti.com/downloads/sds_support/targetcontent/LinuxDspTools/download.html 
#
# --------------------------------------------------------------------------------

## export C6X_C_DIR="/opt/TI/C6000CGT7.0.4/include;/opt/TI/C6000CGT7.0.4/lib"

C6X_C_DIR   = /opt/TI/C6000CGT7.0.4/include
C6X_LIB_DIR = /opt/TI/C6000CGT7.0.4/lib
C6X_BIN_DIR = /opt/TI/C6000CGT7.0.4/bin



C            = $(C6X_BIN_DIR)/./cl6x
CL           = $(C6X_BIN_DIR)/./lnk6x
CS           = $(C6X_BIN_DIR)/./strip6x

##
## add below to CFLAGS  -DRECONFIGURE_DSP_688MHZ  for DSP turbo mode/overclocking
## !!!! WARNING: use on your own risc in any aspect. !!!!
##
CFLAGS       = -O3 -mv 6400+ --mem_model:const=far --mem_model:data=far -i $(C6X_C_DIR) -c -DDSP_CC
CLFLAGS      = --stack_size=0x1000 -i $(C6X_LIB_DIR) -cr
LIBS         = CMD_SR3.cmd SR3PRO_A810Driver.lib PAC_pll.lib

C_OBJECTS = FB_spm.obj dataprocess.obj \
	FB_spm_statemaschine.obj \
	FB_spm_areascan.obj FB_spm_offsetmove.obj \
	FB_spm_autoapproach.obj FB_spm_probe.obj \
	FB_spm_CoolRunner_puls.obj FB_spm_dsoszi.obj \
	spm_log.obj \
	ReadWrite_GPIO.obj \
	SR3_flash_support.obj \
	mcbsp_support.obj

ASM_OBJECTS = LogAsmSR3Pro.obj timerasm.obj vector.obj

FB_spmcontrol.out: $(C_OBJECTS) $(ASM_OBJECTS) $(LIBS)
	@echo
	@echo binding objects...
	$(CL) $(CLFLAGS) $(C_OBJECTS) $(ASM_OBJECTS) $(LIBS) -m FB_spmcontrol.map -o FB_spmcontrol.out
#	@echo
#	@echo stripping debug info from FB_spmcontrol.out...
#	@echo $(CS) FB_spmcontrol.out
	@echo
	@echo FB_spmcontrol.out is ready to load into the DSP.
	@echo

$(C_OBJECTS) : dataprocess.h  FB_spm_analog.h  FB_spm_dataexchange.h FB_spm.h  FB_spm_statemaschine.h dsp_signals.h

$(C_OBJECTS): %.obj: %.c
	$(C) -c $(CFLAGS) $<

$(ASM_OBJECTS): %.obj: %.asm
	$(C) -c $(CFLAGS) $<

# .PHONY : load
# load: FB_spmcontrol.out
# 	ln -sf ../../loadusb64/kernel.out .
# 	../../loadusb64/loadusb FB_spmcontrol.out

.PHONY : clean
clean :
	rm -f ${C_OBJECTS} ${ASM_OBJECTS} FB_spmcontrol.out FB_spmcontrol.map
