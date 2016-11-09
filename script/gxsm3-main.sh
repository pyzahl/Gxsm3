#!/bin/sh

## Demo Startup script ##

# make a new data directory for today's session
cd ~/Data/2004-Data
mkdir `date --iso-8601`
cd `date --iso-8601`

# prepare a dummy py-remote script
export PYTHONPATH=.
cp ../remote-template.py remote.py

# setup a link (or copy) to extra scan info (any text file)
ln -s ~/SRanger/TiCC-project-files/FB_spmcontrol_testing/gxsm_extra_scan_info .

# launch gxsm3 now
gxsm3

