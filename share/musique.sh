#!/bin/bash

######
# Chercher tous les fichiers .song sur la clef USB
# Les copiers dans /home/ubuntu/songs
######

#export DISPLAY=":0"
#notify-send "Copying new songs... Please wait."

gpio mode 5 out
gpio write 5 1

cp -n /media/song_usb/*.song /home/ubuntu/songs
sync
umount /media/song_usb

gpio write 5 0

#notify-send "Copying done, you can safely remove the USB key"
