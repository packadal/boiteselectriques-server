#!/bin/bash

######
# Copy the $extension files from the root folder (/) 
# of a USB key to the $destination folder
#
# $1 : Device's kernel name
######

extension="song"
destination="/home/pi/songs"
user="pi"
led="5"

gpio mode $led out
gpio write $led 1

mkdir /media/song_$1

mount /dev/$1 /media/song_$1

echo "mounted" >> /home/pi/truc.txt

cp -n /media/song_$1/*.$extension $destination
sudo chown -R $user $destination
sync

umount /media/song_$1
rmdir /media/song_$1

gpio write $led 0

