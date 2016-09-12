Compiling the project
=====================

Dependencies
------------

Installation on a new Raspberry Pi image (Raspbian Jessie)

    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo add-apt-repository ppa:ubuntu-sdk-team/ppa
    sudo apt-get update
    sudo apt-get dist-upgrade
    sudo apt-get install git g++-4.8 ubuntu-sdk libportaudiocpp0 libportaudio-dev libavfilter-dev libavformat-dev libavcodec-dev cmake
  

Build latest CMake and KDE Frameworks 5 Archive : 

* CMake
```
    mkdir ~/kf5
    cd ~/kf5 
    wget http://www.cmake.org/files/v3.0/cmake-3.0.0.tar.gz
    tar -zxf cmake-3.0.0.tar.gz
    cd cmake-3.0.0
    (mkdir build; cd build; cmake ..; make; make install)
```    
* Extra CMake modules
```
    git clone git://anongit.kde.org/extra-cmake-modules
    cd extra-cmake-modules
    (mkdir build; cd build; cmake ..; make; make install)
```    
* KArchive
```
    wget http://download.kde.org/stable/frameworks/5.0.0/karchive-5.0.0.tar.xz
    tar -xf karchive-5.0.0.tar.xz
    cd karchive-5.0.0.tar.xz
    (mkdir build; cd build; cmake ..; make; make install)
```

And build.

Building the program
--------------------

First, clone the repository on the Raspberry Pi, either directly from Github or using the Git CLI::
  
  $ git clone https://github.com/hixe33/boiteselec-server
  
Then, we need to generate the Makefile before finally compiling the sources::

  $ mkdir boiteselec-server/build
  $ qmake -config release -o boiteselec-server/build/Makefile boiteselec-server/src/be-server.pro
  $ cd boiteselec-server/build
  $ make
