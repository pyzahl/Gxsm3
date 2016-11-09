#!/usr/bin/perl

# Example Script for programing Xxsm (C) PZ 1998-12

use Time::Local;
use IO::Handle;

# if you have no fifo, then
# create fifo before with "mkfifo remote"
$remotefifo = "remote";

# now open the fifo used for remotecontrol
open(REMOTE, "> $remotefifo\0")
or die "sysopen $remotefifo: $!";
# use autoflush !!
REMOTE->autoflush(1);

# --------------- Users program starts here ------------------------
# do some things....
# testing....
R("cmd chmodeA 0");
R("cmd log");
R("set PointsX 300");
R("set RangeX 150");
R("scan init");
R("scan line 150");
R("scan line 299");
R("scan stop");
R("cmd chmodeA 0");
R("cmd autodisp");
R("scan init");
for($i=0; $i<8; $i++){
  for($l=$i; $l<300-8; $l+=8){
    Scan("line", $l);
  }
#  R("cmd sleepms 2000");
sleep(1);
}
R("scan stop");

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

# Syntax Example: Rx("file","name.nc",0);
sub Rx {
    REMOTE->printf("$_\n");
    printf("$_\n");
}

