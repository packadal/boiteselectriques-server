Compiling the project
=====================

Dependencies
------------

Installation on a new Raspberry Pi image (Raspbian Jessie Lite)

Divers
~~~~~~

First, let's download the basic tools for compilation, plus the server's dependencies, from the official repo::

  $ sudo apt-get install build-essential git liboscpack-dev libqt5serialport5-dev librtaudio-dev libsndfile1-dev qt5-default qt5-qmake wiringpi

Extra CMake Modules
~~~~~~~~~~~~~~~~~~~

Then, we need the Extra CMake Modules to compile KArchive.
The problem is that the version in the official repositories is not enough up-to-date, so we need to compile a more recent verion::

  $ sudo apt-get install cmake
  $ git clone git://anongit.kde.org/extra-cmake-modules
  $ cd extra-cmake-modules
  $ mkdir build && cd build && cmake .. && make && sudo make install

KArchive
~~~~~~~~~~

Now, we can compile KArchive::

  $ wget http://download.kde.org/stable/frameworks/5.17/karchive-5.17.0.tar.xz
  $ tar -xf karchive-5.17.0.tar.xz
  $ cd karchive-5.17.0
  $ mkdir build && cd build && cmake .. && make && sudo make install

Additional notes
~~~~~~~~~~~~~~~~

If there is any problem with those dependencies, or if you can't download the ``liboscpack-dev`` from the official Raspbian repo, you can download the sources from `our git repo <https://github.com/hixe33/boiteselectriques-server-deps>`_

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

If there is any ``#include`` problem at compilation, don't forget to check where all the libraries are installed (``.h`` and ``.so`` files) and to update the QMake file (``src/be-server.pro``) according to their locations.
