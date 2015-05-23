#! /bin/sh
### BEGIN INIT INFO
# Provides: bluepill
# Required-Start:    $all
# Required-Stop:     $local_fs $remote_fs $network $syslog
# Default-Start: 2 3 4 5
# Default-Stop: 0 1 6
# Short-Description: bluepill daemon, providing process monitoring
# Description: bluepill is a monitoring tool. More info at http://github.com/arya/bluepill.
### END INIT INFO

# based on http://jonkinney.com/articles/2010/02/01/bluepill-init-script-for-monitoring-delayed-job-on-linux-opensuse/

set -e

. /lib/lsb/init-functions
name=$(basename $0)
real_name=$(basename $(readlink "$0" || echo "$0"))
 
if [ "$name" = "$real_name" ]; then
    echo "Link this script to /etc/init.d/project_name"
    exit 1
fi
 
# here comes default config
USER=root
APP_ROOT=/home/pi/$name/current
BLUEPILL_CONFIG=config/${name}.pill
test -f /etc/default/$name && . /etc/default/$name
CMD="sudo -i -u $USER -- sh -c 'cd $APP_ROOT && bluepill"
 
case "$1" in
    start)
        echo -n "Starting bluepill for user $name"
        eval "$CMD --no-privileged load $BLUEPILL_CONFIG'"
        ;;
    stop)
        echo "Shutting down monitored processes"
        eval "$CMD --no-privileged stop'"
 
        echo "Shutting down bluepill daemon"
        eval "$CMD --no-privileged quit'"
        ;;
    restart)
        ## Stop the service and regardless of whether it was
        ## running or not, start it again.
        $0 stop
        $0 start
        ;;
    status)
        eval "$CMD --no-privileged status'"
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
        ;;
esac