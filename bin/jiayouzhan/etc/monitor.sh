#!/bin/sh

proc_num()                                          # 计算进程数  
{  
    num=`ps | grep $proc_name | grep -v grep | wc -l`  
    return $num  
}  

proc_name="/bin/GasStationCardManageSystem"             # 进程名  
proc_num 
number=$?

if [ $number -eq 0 ]             # 判断进程是否存在  
then   
    reboot                                          # 重启系统
fi  

proc_name="/bin/UdpMulticastClient"                 # 进程名  
proc_num 
number=$?
  
if [ $number -eq 0 ]             # 判断进程是否存在  
then   
    reboot                                          # 重启系统
fi
