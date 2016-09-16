Setup
=====

Installation
------------

Basic Installation
~~~~~~~~~~~~~~~~~~

Just clone the repository on the Raspberry Pi and follow the instructions of the ``COMPILING.rst`` page.

To test the program, copy the ``share/run.sh`` script to the executable (``be-server``) directory, and run it (don't forget to ``chmod +x`` it if it doesn't want to be executed.

WARNING : Be sure SPI has been activated, using the ``rapsi-config`` tool (cf ``PI.rst`` file for more details about how to set up the Raspberry Pi), or the server won't work.

.. _daemonized:

Daemonizing
~~~~~~~~~~~

WARNING : ``root`` access is required for the following section.

To make the server running in the background automatically on the RasPi startup, it's possible to daemonize it with a few steps.

First, copy the ``be-server`` executable file to ``/opt/boites-electriques/``, and the ``be-server.sh`` script (located in the repo's ``share/`` directory) to ``/etc/init.d/be-server``::

  $ sudo mkdir /opt/boites-electriques
  $ sudo cp build/be-server /opt/boites-electriques/be-server
  $ sudo cp share/be-server.sh /etc/init.d/be-server

You can then edit the ``/etc/init.d/be-server`` to change the following values:

``DAEMON`` 
  The full command.
  Default : ``"/opt/boites-electriques/be-server"``
  
``daemon_OPT`` 
  The program's arguments (here, the path to a specific configuration file).
  Default : ``""`` (empty, to use the default config file)

``DAEMONUSER``
  The user running the program.
  Default : ``"pi"`` (default ``sudo`` user of Raspbian)
  
``daemon_NAME``
  Name of the program.
  Default : ``"be-server"``

Then, we give the right permissions to the daemonizing script and reload the system's daemons (as root or with ``sudo``)::

  # chmod 0755 /etc/init.d/be-server
  # systemctl daemon-reload

We can then test the script::

  # systemctl start be-server
  [ ok ] Starting be-server (via systemctl): be-server.service
  
  # systemctl stop be-server
  [....] Stopping be-server (via systemctl): be-server.service

And add it to the startup scripts::
  
  # systemctl enable be-server

To remove it::

  # systemctl enable be-server

Configuration
-------------

General informations
~~~~~~~~~~~~~~~~~~~~

The server's configuration is saved in the ``config.txt`` file in the same folder as the executable.
This file is formatted with the (almost) standard `INI file format <https://en.wikipedia.org/wiki/INI_file>`_.

By default (if the server is ran without the ``--config`` option), the configuration file is ``~/.config/Rock & Chanson/Boites Electriques.ini``.

The missing fields in the file will automatically be filled at the execution.

The following options are available :
  
``[default]`` section
~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
  
``threshold``
  Default threshold value (integer).
  Default : ``200``
  
``master``
  Default master volume (integer).
  Default : ``50``
  
``volume``
  Default track's volume (integer).
  Default : ``50``

``pan``
  Default track's pan (integer).
  Default : ``0``

``activation``
  Default track's activation status (boolean : ``true`` or ``false``).
  Default : ``false``
  
``[files]`` section
~~~~~~~~~~~~~~~~~~~
  
``folder``
  Files save/load folder (string : path, ending with '``/``').
  Default : ``/home/pi/songs/``
  
``extension``
  Songs files' extension (string : ``*.<extension>``).
  Default : ``*.song``

``[gpio]`` section
~~~~~~~~~~~~~~~~~~

``led``
  LED's WiringPi identifier (to use with the ``gpio`` command).
  Default : ``6``

``[osc]`` section
~~~~~~~~~~~~~~~~~

``ip``
  Client's OSC IP address (integer).
  Default : ``192.170.0.17``
  
``receiver``
  Server's OSC receiver port (integer).
  Default : ``9988``
  
``sender``
  Server's OSC sender port (integer).
  Default : ``9989``
  
Run
---

To test the server, the location of KArchive binary is required to run the program correctly::
  
  $ export LD_LIBRARY_PATH=:/usr/local/lib/arm-linux-gnueabihf/ 
  $ ./be-server

Or, if the server has been daemonized_ (you can't give arguments to the command in that case)::
  
  $ sudo systemctl start be-server.service

The following options are available :

``-c``, ``--config <filepath>``
  Define the ``<filepath>`` file as the configuration file.
