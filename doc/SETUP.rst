Setup
=====

Installation
------------

Basic Installation
~~~~~~~~~~~~~~~~~~

Just clone the repository on the Raspberry Pi and follow the instructions of the ``COMPILING.rst`` page.

.. _daemonized:

Daemonizing
~~~~~~~~~~~

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

  # /etc/init.d/be-server start
  [ ok ] Starting be-server (via systemctl): be-server.service
  
  # /etc/init.d/be-server stop
  [....] Stopping be-server (via systemctl): be-server.service

And add it to the startup scripts::
  
  # update-rc.d be-server defaults

To remove it::

  # update-rc.d -f be-server remove

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

To start the server, just run ``./be-server``, or ``sudo /etc/init.d/be-server start`` if the server has been daemonized_.

The following options are available :

``-c``, ``--config <filepath>``
  Define the ``<filepath>`` file as the configuration file.
