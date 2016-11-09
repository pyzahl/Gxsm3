#!/usr/bin/perl

# Example Script for programing Xxsm (C) PZ 1998-12

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
# do some things....
# testing....
R("cmd chmodeA 0");
R("cmd log");
R("cmd unitS");
R("cmd unitbz");

$px=800;
$phi=-24.3;

Set("Rotation", $phi);
$phi=$phi/57.292;

Set("PointsX", $px);
$sstart=2.0;
$send=5.0;
$slines=200;

Set("PointsY", $slines+1);
$ds=($send-$sstart)/$slines;

$bzx=100;
Set("LengthX", $bzx);
Set("LengthY", 0);
$Xoff=0;
$Yoff=0;
Set("OffsetX",0);
Set("Offset00X",$Xoff);
Set("Offset00Y",$Yoff);

R("scan init");

$Gate=10;
Set("Gatetime", $Gate);

for($l=0, $S=$sstart; $l<$slines; $S+=$ds, $l++){
   Set("Energy", $S);
   Echo();
   sleep(5);

   if($S < 4){
     R("action DSPPeakFind_EfromMain_1");
     R("action DSPPeakFind_RunPF_1");
     Echo();
     R("action DSPPeakFind_OffsetToMain_1");
     Echo();
     sleep(2);
   }

   Set("LengthX", $bzx);
   Set("LengthY", 0);
   Echo();
   sleep(2);

   Scan("line",$l);
   ScanN("setylookup", $l, $S, 0);
   Echo();
}

R("scan stop");
R("file save");

R("file save");

# move beam away

Set("Energy", 5);
Set("Offset00X", 10);

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
