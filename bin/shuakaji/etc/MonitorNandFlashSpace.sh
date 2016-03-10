#!/bin/sh

NandFlashAvailableSpace=`df | grep /dev/mtdblock4 | awk {'print $4'}`
DirList=`ls -c -r /sdcard/log`

if [ $NandFlashAvailableSpace -lt 153600 ];then
    for DirName in $DirList 
    do
        rm -rf /sdcard/log/$DirName
        NandFlashAvailableSpace=`df | grep /dev/mtdblock4 | awk {'print $4'}`
        if [ $NandFlashAvailableSpace -gt 256000 ];then
            exit 0
        fi
    done
fi
