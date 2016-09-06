Interaction
===========

General informations
--------------------

To communicate, client and server use the OSC protocol.

By default, the server uses the IP address ``192.170.0.1``, with the port ``9988`` to send messages and ``9989`` to receive them.

Protocol
--------

**Note :**
  ``<int>`` designate an integer value, ``<str>`` a string one, and ``<bool>`` a boolean one.

Server
~~~~~~

The server can receive the following messages from the client :

``/box/update_threshold <int>``
  Change the threshold value to ``<int>``.

``/box/reset_threshold <bool>``
  Reset the threshold to its default value.

``/box/enable <int>``
  Switch the track number ``<int>``'s state (activate or deactivate it).

``/box/volume <int 1> <int 2>``
  Change the track number ``<int 1>``'s volume value to ``<int 2>``.

``/box/pan <int 1> <int 2>``
  Change the track number ``<int 1>``'s pan value to ``<int 2>``.

``/box/mute <int> <bool>``
  Mute (``<bool>`` = ``true``) or unmute (``false``) the track number ``<int>``.

``/box/solo <int> <bool>``
  "Solo" (``<bool>`` = ``true``) or "unsolo" (``false``) the track number ``<int>``.

``/box/master <int>``
  Change the master volume value to ``<int>``.

``/box/play <bool>``
  Play the song

``/box/stop <bool>``
  Stop the song

``/box/reset <bool>``
  Stop the song and reset its options to their default values

``/box/refresh_song <bool>``
  Refresh the song's informations

``/box/select_song <str>``
  Select another song

``/box/sync <bool>``
  Send the informations of the actual song and the current state of the player to the client

Client
~~~~~~

The client can receive the following messages from the server to access its informations :

``/box/beat <int>``
  The actual server's beat count value = ``<int>``.

``/box/enable_out <int>``
  The box number ``<int>`` has been activated.

``/box/enable_sync <int>``
  The numbers of the activated tracks, where ``<int>`` is a binary number indicating them.
  For example, for an 8-tracks song with its 2nd, 4th, 5th and 8th tracks activated, ``<int>`` = 10011010.

``/box/play <int>``
  The selected song started playing (and that its tempo's value is ``<int>``).

``/box/ready <bool>``
  The selected song is (``<bool>`` = ``true``) or not (``false``) loaded and ready to be played.

``/box/sensor <int>``
  The actual server's threshold value = ``<int>``.

``/box/songs_list <str>``
  The available songs list = ``<str>``. 
  ``<str>`` is the concatenation of the songs' filenames, separated by the character ``|``.

``/box/title <str>``
  The actual server song's name is ``<str>``.

``/box/tracks_count <int>``
  The selected song's tracks count is ``<int>``.

``/box/tracks_list <str>``
  Send the informations of the selected song's tracks, as ``<str>``.
  ``<str>` is the concatenation of the songs' tracks' names, separated by the character ``|``.
