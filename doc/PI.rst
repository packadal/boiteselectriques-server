Configuration Pi
================

Distrib : Raspbian Jessie Lite
Install (pour install auto : ajouter silentinstall au début de boot/commandline.txt, puis connexion SSH). 

NOT SO SURE
---------------------------------------------------------------------------------------  
On passe sur ``testing`` pour avoir tous les packages : ``/etc/apt/sources.list``::
  
  deb http://mirrordirector.raspbian.org/raspbian/ **jessie** main contrib non-free rpi
  
Devient::
  
  deb http://mirrordirector.raspbian.org/raspbian/ **testing** main contrib non-free rpi


  $ sudo apt-get update && sudo apt-get dist-upgrade
----------------------------------------------------------------------------------------
  
Git::
  $ sudo apt-get install git
  $ git clone https://github.com/hixe33/boiteselec-server


Building::

  $ sudo apt-get install build-essential qt5-qmake qt5-default libqt5serialport5-dev libsndfile1-dev liboscpack-dev librtaudio-dev

WiringPi::

KF5Archive::
 
Activate SPI::
  
  # raspi-config
  
  -> 9 Advanced Options -> A5 SPI -> <Yes> -> <Ok> -> <Finish>
  
Bugs::

  Si Setting locale failed : `Solution <https://www.thomas-krenn.com/en/wiki/Perl_warning_Setting_locale_failed_in_Debian>`_
  # dpkg-reconfigure locales
  
SETUP WIFI BRIDGE::

Dans /etc/network/interfaces::
  
  #config pont
  iface eth0 inet dhcp

  allow-hotplug wlan0
  iface wlan0 inet static
	address 192.170.0.1
	netmask 255.255.255.0
	network 192.170.0.0
  	broadcast 192.170.0.255

  iface default inet dhcp 

Puis les utilitaires::

  $ sudo apt-get install hostapd dnsmasq (iptables ?)
  
Dans /etc/hostapd/hostapd.conf::

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
  
Dans /etc/dnsmasq.conf::

  interface=wlan0
  listen-address=192.170.0.1
  bind-interfaces
  server=8.8.8.8
  domain-needed
  bogus-priv
  dhcp-range=192.170.0.50,192.170.0.150,12h

Dans /etc/default/hostapd (pour hostapd au démarrage):

  DAEMON_CONF="/etc/hostapd/hostapd.conf"

On teste::
  
  sudo service dnsmasq start
  sudo service hostapd start
