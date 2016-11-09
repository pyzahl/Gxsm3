#!/bin/bash
typeset -i N
N=0

clear
echo "Gxsm Remote Startup procedure for Dummies..."
echo
echo "1.) Press Run button in Gxsm Remote Control!"
echo
echo "2.) Press Enter when done."
read KEY 

cat remoteecho > remoteechofile &
echo | (cat >> remotehistory)
tail -f -n 1 remotehistory > remote &

while [ $N -eq 0 ]
do
    clear
    echo "Remote Control Script by PZ 12/2000"
    echo "==================================="
    echo "'Q': quit   'R': read from file"
    echo "----- Remote History --------------"
    tail -n 5 remotehistory
    echo "-----------------------------------"
    echo "----- Remote Echo -----------------"
    tail -n 5 remoteechofile
    echo "-----------------------------------"
    echo
    echo "Enter Gxsm Remote Command >"
    read KEY 
    case $KEY in
    Q) N=1 
    KEY="cmd quit"

    ;;
    R) 
    echo "Enter Commandfilename:"
    read CMDFILE
    cat $CMDFILE >> remotehistory
    KEY=""
    ;;
    esac
    echo $KEY | (cat >> remotehistory)
    sleep 1
done
echo Finished, killing jobs
kill `ps | grep tail | awk '{print $1}'`
kill `ps | grep cat | awk '{print $1}'`
