Setup
=====

Installation
------------

Configuration
-------------

General informations
~~~~~~~~~~~~~~~~~~~~

The server's configuration is saved in the ``config.txt`` file in the same folder as the executable.
This file is formatted with the (almost) standard `INI file format <https://en.wikipedia.org/wiki/INI_file>`_.

The following options are available :
  
``[default]`` section
~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
  
``threshold``
  Default threshold's value (integer)
  Default : 200
  
``master``
  Default master volume's value (integer)
  Default : 50
  
``volume``
  Default track's volume (integer)
  Default : 50

``pan``
  Default track's pan (integer)
  Default : 0

``activation``
  Default track's activation status (boolean : true or false)
  Default : false
  
``[files]`` section
~~~~~~~~~~~~~~~~~~~
  
``export_folder``
  Files save/load folder (string : path, ending with '/')
  Default : /home/pi/songs/
  
``extension``
  Songs files' extension (string : '*.<extension>')
  Default : *.song

``[osc]`` section
~~~~~~~~~~~~~~~~~

``ip``
  Client's OSC IP address (integer)
  Default : 192.170.0.17
  
``receiver``
  Server's OSC receiver port (integer)
  Default : 9988
  
``sender``
  Server's OSC sender port (integer)
  Default : 9989
