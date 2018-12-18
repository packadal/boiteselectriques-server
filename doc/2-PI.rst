Raspberry Pi configuration
==========================

Raspbian installation
---------------------

The installation will be done on a Raspberry Pi, powered by the `**Raspbian Jessie Lite** <https://www.raspberrypi.org/downloads/raspbian/>`_ image. 

You can follow the installation instructions for more information about how to write the image on an `SD Card <https://www.raspberrypi.org/documentation/installation/installing-images/README.md>`_.

Then follow one of the two sections' instructions, depending on your equipment, connect your Pi to the Internet and update your Raspberry Pi::

  # apt-get update && apt-get upgrade

Accessing the Raspberry Pi without a screen
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You first need to use the raspberry pi with a screen to enable remote connection.

# raspi-config

Then go to: ``5 Interfacing Options`` -> ``P2 SSH`` -> ``<Yes>`` -> ``<Ok>`` -> ``<Finish>``

This will allow you to connect remotely to the Raspberry Pi.

If you have a screen with a HDMI port, connect it to your Raspberry Pi and do the same with a USB keyboard. Then power your Raspberry Pi on and follow the onscreen instructions to perform the installation.

Then, connect the Pi to your network (or directly to your computer, creating a local network !) with an Ethernet cable and power it on. You can scan your network with the ``nmap`` command to find your Pi IP address and connect you to it through SSH (default password : ``raspberry``)::

  $ ssh pi@<your pi's IP>

Server's requirements
---------------------

SPI
~~~

For the Boîtes Électriques Server's executable to be able to run correctly, you need to activate the SPI interface on the Pi. To do that, just type::

  # raspi-config
  
Then go to: ``5 Interfacing Options`` -> ``P4 SPI`` -> ``<Yes>`` -> ``<Ok>`` -> ``<Finish>``

Warning : this must be done before executing the server (``be-server``), else it won't run.

FFMPEG
~~~~~~

To read the songs, the ``ffmpeg`` program is required. However, it is not directly available from the official Raspbian repo.

Fortunately, the deb-multimedia repo offers a Raspberry Pi-compatible package for ``fmpeg``. So, you must add this repo to your ``/etc/apt/sources.list``, by insering the fllowing line in the file::

  deb http://www.deb-multimedia.org jessie main non-free

Then, update your packages cache, import the repo's GPG key and install ``ffmpeg``::

  # apt-get update && apt-get install deb-multimedia-keyring
  # apt-get install ffmpeg

ALSA
~~~~

To be able to play the songs correctly, ALSA needs a bit of configuration. To do so, just open the ``/etc/asound.conf`` file and replace its content with this :

  defaults.ctl.!card 1;
  
If there is any problem to access the ``alsamixer``, remove (or simply move or rename) the ``/etc/asound.conf`` file.
  
Auto-load .song files from a USB key
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

WARNING: You first need to clone the server's repository on the Pi an open its folder (see ``3-COMPILING.rst`` for details about the procedure).

First, open the ``share/import_music.sh`` to setup the copy script. You can change the following variables:

``extension``
  Extension of the song files. Default: ``"song"``.

``destination``
  Folder where to copy the files. Default: ``"/home/pi/songs"``.

``user``
  Name of the user that owns the ``destination`` folder. Default: ``"pi"``.

``led``
  LED's ``wiringpi`` GPIO identifier (to signal the script is running). Default: ``"5"``

Once it's done (you can also leave the default values), copy the script to ``/usr/local/bin``, and the ``udev`` rule file to ``/etc/udev/rules.d``::

  # cp share/import_music.sh /usr/local/bin/
  # cp share/89-usbcopy.rules /etc/udev/rules.d/

The system should automatically detect the new rule, and will automatically copy the .song (or whatever you want) files from a USB key/drive to the folder you choosed.
Note that a LED should be activated during the whole operation, so don't touch the key or drive during that time.



Wifi bridge setup
-----------------

Network and tools
~~~~~~~~~~~~~~~~~

First, edit the ``/etc/network/interfaces`` file, and replace it with this::
  
  #config pont
  iface eth0 inet dhcp

  allow-hotplug wlan0
  iface wlan0 inet static
	address 192.170.0.1
	netmask 255.255.255.0
	network 192.170.0.0
  	broadcast 192.170.0.255

  iface default inet dhcp 

Then, we need Hostapd and DNSMasq to setup the bridge and attribute IPs automatically::

  $ sudo apt-get install hostapd dnsmasq
  
Hostapd configuration
~~~~~~~~~~~~~~~~~~~~~  
  
Edit the ``/etc/hostapd/hostapd.conf`` file and replace it with this::

  interface=wlan0
  driver=nl80211
  ssid=BoitesElectriquesPi
  hw_mode=g
  channel=6
  ieee80211n=1
  wmm_enabled=1
  ht_capab=[HT40][SHORT-GI-20][DSSS_CCK-40]
  macaddr_acl=0
  auth_algs=1
  beacon_int=100
  dtim_period=2
  max_num_sta=255
  rts_threshold=2347
  fragm_threshold=2346
  
Then ``/etc/default/hostapd``to make it run on startup::

  DAEMON_CONF="/etc/hostapd/hostapd.conf"  
 
Test it::

  # systemctl start hostapd
  
You can check the service status with the following command::

  # systemctl status hostapd
 
And enable it with ``systemctl``::

  # systemctl enable hostapd
  
DNSMasq
~~~~~~~  
  
Edit the ``/etc/dnsmasq.conf`` file and replace it with this::  

  interface=wlan0
  listen-address=192.170.0.1
  bind-interfaces
  server=8.8.8.8
  domain-needed
  bogus-priv
  dhcp-range=192.170.0.50,192.170.0.150,12h
 
Test it::

  # systemctl start dnsmasq
  
You can check the service status with the following command::

  # systemctl status dnsmasq
 
And enable it with ``systemctl``::

  # systemctl enable dnsmasq  

Now, you are ready to compile the server !

Bugs
----

If you have the following error::
 
  Setting locale failed
 
You can check the `following page <https://www.thomas-krenn.com/en/wiki/Perl_warning_Setting_locale_failed_in_Debian>`_, and run::
 
  # dpkg-reconfigure locales
  
Then select the correct locales.  
