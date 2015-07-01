#!/bin/sh
cd /etc/init.d/ 
procs=`ls sems? 2> /dev/null`
cd -  > /dev/null
echo $procs
for proc in $procs; do
	#echo $proc
	/sbin/service $proc status  
	STATUS_OK=$?
	# if status return "running" or "stopped" then no action must be taken
	# otherwise something is wrong, so restart it
	#echo $STATUS_OK
	if [ $STATUS_OK -eq 2 ]; then
		logger -i -p user.info -t CT6 "$proc is down - service $proc restart"
		/sbin/service $proc restart	
	fi

done
