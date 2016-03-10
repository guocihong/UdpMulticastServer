#!/bin/sh

# install additional modules
modprobe ft5x06_ts 2>/dev/null
modprobe goodix_touch 2>/dev/null
modprobe ov9650 2>/dev/null
modprobe uvcvideo 2>/dev/null
modprobe tvp5150_tiny210 2>/dev/null
modprobe rtl8192cu 2>/dev/null
modprobe ath9k_htc 2>/dev/null
modprobe rt73usb 2>/dev/null
modprobe rt2800usb 2>/dev/null
modprobe zd1211rw 2>/dev/null

modprobe s5pv210_door 2>/dev/null
modprobe s5pv210_buzzer 2>/dev/null
modprobe s5pv210_led 2>/dev/null
modprobe s5pv210_relay 2>/dev/null
modprobe s5pv210_wieg 2>/dev/null

