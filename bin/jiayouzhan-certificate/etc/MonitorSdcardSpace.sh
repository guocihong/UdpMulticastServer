#!/bin/sh

SdcardAvailableSpace=`df | grep /dev/sdcard | awk {'print $4'}`
DirList=`ls -c -r /sdcard/log`

if [ $SdcardAvailableSpace -lt 1048576 ];then
    for DirName in $DirList 
    do
        rm -rf /sdcard/log/$DirName
        SdcardAvailableSpace=`df | grep /dev/sdcard | awk {'print $4'}`
        if [ $SdcardAvailableSpace -gt 2097152 ];then
            exit 0
        fi
    done
fi
