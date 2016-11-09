#!/usr/bin/perl

# Example Script for programing Gxsm (C) PZ 2001-1

# Hi CD!
# Instructions to run:
# 1) put this perl-script in your datadir and cd there! 
#    Make it execuatable! (I've done this for you)
# 2) ensure remote and remoteecho Fifos are present, eg.
#    create with shell cmd: "mkfifo remote remoteecho"
# 3) ensure Gxsm has the right Settings:
#    Preferences: Path/RemoteFifo    = remote
#    Preferences: Path/RemoteFifoOut = remoteecho
# 4) run gxsm
# 5) make all settings, start scan, etc...
#-------- Start here if all settings are correct -----------
# 6) Menupath: activate Tools/Remote Control (dt.: Fernsteuerung)
# 7) Press "Run-Button", this will block Gxsm until Remote-Script is started!
#    (Gxsm waits for Fifos to be connected)
# 8) Use a new xterm, cd to your datapath and run the per script like:
#    ./autosave.perl
#-------- Finish the forever running script by pressing "Stop"
#         in the remote Controlwindow
#      -> do not kill the script, Gxsm way wait forever <-
#    Have fun...
#                 PZ.

use Time::Local;
use IO::Handle;

# if you have no fifo, then
# create fifo before with "mkfifo remote"
$remotefifo = "remote";
$remotefifoecho = "remoteecho";

# now open the fifo used for remotecontrol
open(REMOTE, "> $remotefifo\0")
or die "sysopen $remotefifo: $!";
# use autoflush !!
REMOTE->autoflush(1);

open(REMOTEECHO, "< $remotefifoecho\0")
or die "sysopen $remotefifoecho: $!";
# use autoflush !!

# --------------- Users program starts here ------------------------

# run forver...
for(;;){
# Sync with Gxsm and sleep 900s zzz... Oops, do file save and loop.
   Echo();
   sleep(900);
   R("file save");
}

# say goodbye
R("cmd quit");
R("byebye");
# and close remote session
close(REMOTE);

# some usefull subs

# Send Remote Commands in some ways...
# Syntax Example: Set("energy",72,0);

# Syntax Example: R("set energy 72");
sub R {
    REMOTE->printf("$_[0]\n");
    printf("$_[0]\n");
}

# Syntax Example: Set("energy",72);
sub Set{
    REMOTE->printf("set $_[0] %f\n",$_[1]);
    printf("set $_[0] %f\n",$_[1]);
}

# Syntax Example: Scan("line",133);
sub Scan {
    REMOTE->printf("scan $_[0] $_[1]\n");
    printf("scan $_[0] $_[1]\n");
}

sub ScanN {
    REMOTE->printf("scan $_[0] $_[1] $_[2] $_[3]\n");
    printf("scan $_[0] $_[1] $_[2] $_[3]\n");
}

# Syntax Example: Rx("file","name.nc",0);
sub Rx {
    REMOTE->printf("$_\n");
    printf("$_\n");
}

# wait for echo
sub Echo {
   $dummy="";
   REMOTE->printf("cmd echo ready-echo\n");
   printf("ping send !\n");
   REMOTEECHO->read($dummy, 11);
   printf("got echo !\n");
}
