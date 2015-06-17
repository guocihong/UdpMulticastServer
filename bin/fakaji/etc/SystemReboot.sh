#!/bin/sh

mkdir -p /var/spool/cron/crontabs

echo "30 2 * * * reboot"  > /var/spool/cron/crontabs/root 
echo "*/1 * * * * /etc/monitor.sh"  >> /var/spool/cron/crontabs/root

/usr/sbin/crond start

