#!/bin/sh

LD_LIBRARY_PATH=:/usr/local/lib/arm-linux-gnueabihf/ 
export LD_LIBRARY_PATH

PRG=be-server
SCRIPT=$(dirname $(readlink -f "$0"))

while true
do
	ps cax | grep $PRG > /dev/null
	if [ $? -eq 0 ]; then
		echo "Server is running."
	else
		echo "Server is not running."
		$SCRIPT/$PRG $* &
	fi
	
	sleep 2
done
