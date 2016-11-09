#!/usr/bin/env python

## * Python User Control for
## * Configuration and SPM Approach Control tool for the FB_spm DSP program/GXSM2
## * for the Signal Ranger STD/SP2 DSP board
## * 
## * Copyright (C) 2003 Percy Zahl
## *
## * Author: Percy Zahl <zahl@users.sf.net>
## * WWW Home: http://sranger.sf.net
## *
## * This program is free software; you can redistribute it and/or modify
## * it under the terms of the GNU General Public License as published by
## * the Free Software Foundation; either version 2 of the License, or
## * (at your option) any later version.
## *
## * This program is distributed in the hope that it will be useful,
## * but WITHOUT ANY WARRANTY; without even the implied warranty of
## * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## * GNU General Public License for more details.
## *
## * You should have received a copy of the GNU General Public License
## * along with this program; if not, write to the Free Software
## * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

version = "2.1.0"

import pygtk
pygtk.require('2.0')

import gobject, gtk
import os		# use os because python IO is bugy
import time

#import GtkExtra
import struct
import array
#import Numeric

import math
import emb

# if you want to play a sound when ready, here we go
#import pygame
#pygame.init()

# Set here the SRanger dev to be used:
sr_dev_path = "/dev/sranger_mk2_0"

magic_address = 0x5000

i_magic = 0
magic_fmt = "<HHHHHHHHHHHHHHHHHHHHHHHHHH"

i_magic_version = 1
i_magic_year    = 2
i_magic_date    = 3
i_magic_sid     = 4


i_statemachine = 5
fmt_statemachine = "<hhhhLLLhhhhhh"
fmt_statemachine_w = "<h"

i_AIC_in = 6
fmt_AIC_in = "<hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh"

i_AIC_out = 7
fmt_AIC_out = "<hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh"

i_feedback = 9
fmt_feedback = "<hhhhlhhhhhhlllhhhhllhhlh"
#               0123456789012345678901
#       fmt = "<hhhhlhhhhhhlllhhhhllll"

i_analog = 10
fmt_analog_out = "<hhhhhhhh"  #+0
fmt_analog_in_offset = "<hhhhhhhh" #+8
fmt_analog_out_offset = "<hhhhhhhh" #+16
fmt_analog_counter = "<LLLL" #+24

i_autoapp = 14
fmt_autoapp = "<hhhhhhhhLLhhhhhhhhhhh"


i_feedback_mixer = 18
fmt_feedback_mixer = "<hhhhhhhhhhhhhhhhhhhlhhhh"
#       fmt = "<hhhhhhhhhhhhhhhhhhhlhhhh"

global SPM_STATEMACHINE
global SPM_AUTOAPP

# need to get magics
sr = open (sr_dev_path, "r")
sr.seek (magic_address, 1)
magic = struct.unpack (magic_fmt, sr.read (struct.calcsize (magic_fmt)))

# os.lseek (sr.fileno(), magic[i_statemachine], 0)
# SPM_STATEMACHINE = struct.unpack (fmt_statemachine, os.read (sr.fileno(), struct.calcsize (fmt_statemachine)))

# center all offsets
emb.set ("DSP_Bias","0.1")
emb.set ("DSP_SCurrent","1.5")
emb.set ("DSP_CP", "0.024")
emb.set ("DSP_CI", "0.020")
emb.set ("OffsetX", "0")
emb.set ("OffsetY", "0")
emb.sleep (10)


#pygame.mixer.music.load("/usr/share/sounds/gnome/default/alerts/sonar.ogg")
#pygame.mixer.music.play()
#pygame.event.wait()


# start autoapp
emb.action ("DSP_CMD_AUTOAPP")

# wait and check for tip ready
process_flag = 1
while process_flag:
    emb.sleep (20)
    os.lseek (sr.fileno(), magic[i_autoapp], 0)
    SPM_AUTOAPP = struct.unpack (fmt_autoapp, os.read (sr.fileno(), struct.calcsize (fmt_autoapp)))
    process_flag = SPM_AUTOAPP[20]
    
#pygame.mixer.music.load("/usr/share/sounds/ubuntu/stereo/system-ready.ogg")
#pygame.mixer.music.play()

emb.sleep (10)
    
sr.close ()

emb.set ("DSP_Bias","0.1")
emb.set ("DSP_SCurrent","0.5")
emb.set ("DSP_CP","0.012")
emb.set ("DSP_CI","0.010")
emb.set ("OffsetX","0")
emb.set ("OffsetY","0")
emb.startscan ()



