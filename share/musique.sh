#!/bin/bash

######
# Copy the $extension files from the root folder (/) 
# of a USB key to the $destination folder
#
# $1 : USB key's kernel name
######

extension="song"
destination="/home/pi/songs"

#export DISPLAY=":0"
#notify-send "Copying new songs... Please wait."

gpio mode 5 out
gpio write 5 1

cp -n /dev/$1/*.$extension $destination
sync
umount /media/song_usb

gpio write 5 0

#notify-send "Copying done, you can safely remove the USB key"
