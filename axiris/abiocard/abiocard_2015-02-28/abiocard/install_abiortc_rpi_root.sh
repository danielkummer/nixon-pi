#!/bin/bash


#
# Check whether the script is being run by root
#

if [ "$(id -u)" != "0" ]; then
    echo "This script requires root permissions"
    exit 1
fi


#
# Copy
#

destdir=/opt/abiocard

mkdir -p $destdir

cp -f abiocardtime $destdir
chmod 4755 $destdir/abiocardtime


#
# Add lines to the crontab of root
#

addline1="@reboot /opt/abiocard/abiocardtime -u"
addline2="@daily /opt/abiocard/abiocardtime -u"

( crontab -l | grep "$addline1" ) || ( crontab -l; echo "$addline1"; ) | crontab -
( crontab -l | grep "$addline2" ) || ( crontab -l; echo "$addline2"; ) | crontab -

