Compiling the project
=====================

Dependencies
------------

Installation on a new Raspberry Pi image (Raspbian Jessie Lite)

Packaged dependencies
~~~~~~~~~~~~~~~~~~~~~

First, let's download the basic tools for compilation, plus the server's dependencies, from the official repo::

  $ sudo apt-get install build-essential git libqt5serialport5-dev librtaudio-dev libsndfile1-dev qt5-default qt5-qmake wiringpi libkf5archive-dev

Building the program
--------------------

Compilation
~~~~~~~~~~~

First, clone the repository on the Raspberry Pi, either directly from Github or using the Git CLI::
  
  $ git clone https://github.com/hixe33/boiteselec-server
  
Then, we need to generate the Makefile from the QMake file before finally compiling the sources::

  $ mkdir boiteselec-server/build
  $ qmake -config release -o boiteselec-server/build/Makefile boiteselec-server/src/be-server.pro
  $ cd boiteselec-server/build && make
  
Now you're all set to test the program !
  
Additional notes
~~~~~~~~~~~~~~~~

If there is any ``#include`` problem during compilation, don't forget to check where all the libraries are installed (``.h`` and ``.so`` files) and to update the QMake file (``src/be-server.pro``) according to their locations.
