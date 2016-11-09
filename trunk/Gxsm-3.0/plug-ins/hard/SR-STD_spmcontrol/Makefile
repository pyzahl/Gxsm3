#/* SRanger and Gxsm - Gnome X Scanning Microscopy Project
# * universal STM/AFM/SARLS/SPALEED/... controlling and
# * data analysis software
# *
# * DSP tools for Linux
# *
# * Copyright (C) 1999-2008 Percy Zahl
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
# you need to install the TI-C54x-CFG-V4112 CC or newer to /opt.
# Get it from here: 
#
# Linux hosted 54xx Code Generation Tools v4.1.1 
# https://www-a.ti.com/downloads/sds_support/targetcontent/LinuxDspTools/download.html 
#
# --------------------------------------------------------------------------------

C54X_C_DIR   = /opt/TI/cg54x_4_1_1/include
C54X_LIB_DIR = /opt/TI/cg54x_4_1_1/lib
C54X_BIN_DIR = /opt/TI/cg54x_4_1_1/bin

C            = $(C54X_BIN_DIR)/./cl500
CL           = $(C54X_BIN_DIR)/./lnk500
CS           = $(C54X_BIN_DIR)/./strip500
CFLAGS       = -O3 -i=$(C54X_C_DIR) -c -DDSP_CC
CLFLAGS      = -i=$(C54X_LIB_DIR) -c
LIBS         = FB_spm_lnk.cmd

C_OBJECTS = dataprocess.obj \
	FB_spm.obj FB_spm_statemaschine.obj \
	FB_spm_areascan.obj FB_spm_offsetmove.obj \
	FB_spm_autoapproach.obj FB_spm_mover_isr.obj FB_spm_probe.obj \
	FB_spm_CoolRunner_puls.obj FB_spm_dsoszi.obj

ASM_OBJECTS = spm_log.obj feedback.obj feedback_linear.obj vectors.obj 

FB_spmcontrol.out: $(C_OBJECTS) $(ASM_OBJECTS) $(LIBS)
	@echo
	@echo binding objects...
	$(CL) $(CLFLAGS) $(C_OBJECTS) $(ASM_OBJECTS) $(LIBS) -stack=0x200 -m=FB_spmcontrol.map -o=FB_spmcontrol.out
	@echo
	@echo stripping debug info from FB_spmcontrol.out...
	$(CS) FB_spmcontrol.out
	@echo
	@echo FB_spmcontrol.out is ready to load into the DSP.
	@echo

$(C_OBJECTS) : dataprocess.h  FB_spm_analog.h  FB_spm_dataexchange.h  FB_spm.h  FB_spm_statemaschine.h 

$(C_OBJECTS): %.obj: %.c
	$(C) -c $(CFLAGS) $<

$(ASM_OBJECTS): %.obj: %.asm
	$(C) -c $(CFLAGS) $<

.PHONY : load
load: FB_spmcontrol.out
	ln -sf ../../loadusb64/kernel.out .
	../../loadusb64/loadusb FB_spmcontrol.out

.PHONY : clean
clean :
	rm -f ${C_OBJECTS} ${ASM_OBJECTS} FB_spmcontrol.out FB_spmcontrol.map
