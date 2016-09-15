Configuration Pi
================

Raspbian installation
---------------------

The installation will be done on a Raspberry Pi, powered by the `*Raspbian Jessie Lite* <https://www.raspberrypi.org/downloads/raspbian/>`_ image. 

You can follow the installation instructions for more information about how to write the image on an `SD Card <https://www.raspberrypi.org/documentation/installation/installing-images/README.md>`_.

The follow one of the two sections' instructions, depending on your equipment, and update your Raspberry Pi::

  # apt-get update && apt-get upgrade

If you have a screen and a USB keyboard
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you have a screen with a HDMI port, connect it to your Raspberry Pi and do the same with a USB keyboard. Then power your Raspberry Pi on and follow the onscreen instructions to perform the installation.

If you don't
~~~~~~~~~~~~

If you don't have both a screen and a keyboard, you can still access it through SSH.

To do that, once the image has been written on the card, mount its ``boot`` partition, open the ``commandline.txt`` file and add ``silentinstall`` at the beginning of the first line.

Then, connect the Pi to your network (or directly to your computer, creating a local network !) with an Ethernet cable and power it on. You can scan your network with the ``nmap`` command to find your Pi IP address and connect you to it through SSH (default password : ``raspberry``)::

  $ ssh pi@<your pi's IP>

Server's requirements
---------------------

SPI
~~~

For the Boîtes Électriques Server's executable to be able to run correctly, you need to activate the SPI interface on the Pi. To do that, just type::

  # raspi-config
  
Then go to: ``9 Advanced Options`` -> ``A5 SPI`` -> ``<Yes>`` -> ``<Ok>`` -> ``<Finish>``

Warning : this must be done before executing the server (``be-server``), else it won't run.
  
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

Bugs
----

If you have the following error::
 
  Setting locale failed
 
You can check the `following page <https://www.thomas-krenn.com/en/wiki/Perl_warning_Setting_locale_failed_in_Debian>`_, and run::
 
  # dpkg-reconfigure locales
  
Then select the correct locales.  