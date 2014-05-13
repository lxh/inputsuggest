#!/bin/sh




while [ 1 ]
do
	ps aux | grep -v grep | grep config.ini | grep -e isdict -e isformat -e isindex >> mem.log
	sleep 5
done
