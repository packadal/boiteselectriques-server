#ifndef SERVER_H
#define SERVER_H

/**
 * @file Server.h
 * @brief Server interface
 */

#include "PlayThread.h"
#include "SaveManager.h"
#include "SerialManager.h"
#include "transmitter.hpp"

class SaveManager;

struct Settings {
  QString key;
  QVariant value;

  Settings(QString k, QVariant v) : key(k), value(v) {}
};

/**
 * @brief Main class
 *
 * Handles the events and dispatch the corresponding actions
 */
class Server : public QObject {
  Q_OBJECT

  friend class SaveManager;

private:
  PlayThread *m_player; /*< Audio play manager */
  QThread m_playThread;
  SaveManager m_saveManager;           /*< File handling manager */
  SerialManager m_serialManager{this}; /*< Interface with serial port */

  QSettings *m_options; /*< Config options */

  SongData m_song;   /*< Actual song's data */
  QString m_selSong; /*< Selected song's name */
  int m_threshold = 40;

  bool m_loaded{false}; /*< Indicate if a song has been loaded */

  int m_tempo;
  double m_trackDuration = 0.0;

  bool m_playSignalSent = false;

  Transmitter *m_transmitter;

  /**
   * @brief Setup WiringPi interface
   */
  void ledSetup();

  /*******************
   * EVENTS HANDLING *
   *******************/

  /**
   * @brief delete_song event handling
   * @param args the name of the song to delete
   *
   * Remove a local song file
   */
  void handle__box_deleteSong(QDataStream &args);
  /**
   * @brief update_threshold event handling
   * @param args New threshold value
   *
   * Change the threshold value
   */
  void handle__box_updateThreshold(QDataStream &args);
  /**
   * @brief reset_threshold event handling
   * @param args Nothing
   *
   * Reset the threshold value
   */
  void handle__box_resetThreshold(QDataStream &args);
  /**
   * @brief enable event handling
   * @param args Track number
   *
   * Switch a track state (enable/disable)
   */
  void handle__box_enable(QDataStream &args);
  /**
   * @brief volume event handling
   * @param args Track number and new volume (integer)
   *
   * Change the volume of a track
   */
  void handle__box_volume(QDataStream &args);
  /**
   * @brief pan event handling
   * @param args Track number and new pan (integer)
   *
   * Change the pan (left-right volume) of a track
   */
  void handle__box_pan(QDataStream &args);
  /**
   * @brief mute event handling
   * @param args Track number and new state (boolean)
   *
   * Mute or unmute a track
   */
  void handle__box_mute(QDataStream &args);
  /**
   * @brief solo event handling
   * @param args Track number and new state (boolean)
   *
   * "Solo" or "unsolo" a track
   */
  void handle__box_solo(QDataStream &args);
  /**
   * @brief play event handling
   * @param args Nothing
   *
   * Play the song
   */
  void handle__box_play(QDataStream &args);
  /**
   * @brief stop event handling
   * @param args Nothing
   *
   * Stop the song
   */
  void handle__box_stop(QDataStream &args);
  /**
   * @brief master event handling
   * @param args New volume (integer)
   *
   * Change the master volume
   */
  void handle__box_master(QDataStream &args);
  /**
   * @brief reset event handling
   * @param args Nothing
   *
   * Stop the song and reset its options to their default values
   */
  void handle__box_reset(QDataStream &args);
  /**
   * @brief refresh_song event handling
   * @param args Nothing
   *
   * Refresh the song data
   */
  void handle__box_refreshSong(QDataStream &args);
  /**
   * @brief select_song event handling
   * @param args Song's filename (char*)
   *
   * Select a new song
   */
  void handle__box_selectSong(QDataStream &args);
  /**
   * @brief sync event handling
   * @param args Nothing
   *
   * Send the informations of the actual song and the current state of the
   * player
   */
  void handle__box_sync(QDataStream &args);

  /**
   * @brief quits the application to ensure everything is reset
   * @param args unused
   */
  void handle__box_quit(QDataStream &args);

public:
  /**
   * @brief Constructor of the Server class
   * @param opt Configuration data
   */
  explicit Server(QSettings *opt);
  ~Server();

  /**
   * @brief Load or initialize the configuration options
   * @param c Pointer to the options data
   * @return If the config options have been loaded (true) or generated from the
   * default values (false)
   */
  bool initConf(QSettings *c);

  /**
   * @brief Activate the LED on pos n
   * @param n
   */
  void ledOn(int n);
  /**
   * @brief Activate the configured LED defined in config file
   */
  void ledOn();
  /**
   * @brief Deactivate the LED on pos n
   * @param n
   */
  void ledOff(int n);
  /**
   * @brief Deactivate the LED defined in config file
   */
  void ledOff();
  /**
   * @brief Make the LED defined in config file blink
   */
  void ledBlink();
  void sendSolo();

signals:
  /**
   * @brief Notify of a new threshold value
   * @param threshold New threshold value
   */
  void updateThreshold(int threshold);
  /**
   * @brief Notify the threshold reset
   */
  void resetThreshold();

  void playSong();

public slots:
  /**
   * @brief Reset the values to default
   */
  void reset();

  /**
   * @brief Change a track status
   * @param i Track number
   * @param val Sensor value (to validate the action)
   */
  void switchBox(unsigned int i, int val);

  /**
   * @brief Play the song
   */
  void play();

  /**
   * @brief Load the song
   * @return Loading status (0 = successful, 1 else)
   */
  bool load();

  /**
   * @brief Stop the song
   */
  void stop();

  /**
   * @brief Update the actual beat value
   * @param t Actual time (in seconds)
   */
  void updateBeat(double t);

  /**
   * @brief Update the total beat count
   * @param t Actual time (in seconds)
   */
  void updateBeatCount(double t);

  /**
   * @brief Actions to perform at the end of the song's loading
   */
  void onSongLoaded();

  /***************************
   * TRANSMISSIONS TO CLIENT *
   ***************************/

  // Perform eventual calculations and then call the private sending functions

  void sendTrackVolume(int track, int volume);
  void sendTrackPan(int track, int pan);

  /**
   * @brief Send the beat
   * @param beat
   */
  void sendBeat(double beat);
  /**
   * @brief Send the actual song's title
   */
  void sendSongTitle();
  /**
   * @brief Send the available songs' list
   */
  void sendSongsList();
  /**
   * @brief Send the song's tracks names, in the form of a string where the
   * different track names are separated by the '|' character
   */
  void sendTracksList();
  /**
   * @brief Notify the client of the loading state
   * @param isReady Server's loading state
   *
   * Send true if all the songs are loaded, false else
   */
  void sendReady(bool isReady);
  /**
   * @brief Send the server's threshold
   */
  void sendThreshold();
  /**
   * @brief Send the song's actual activated tracks
   */
  void sendActivatedTracks();
  /**
   * @brief Send the master volume
   */
  void sendMasterVolume();
  /**
   * @brief Notify the client of the song's playing start
   */
  void sendPlay();

  void sendImages();

  void sendMute();

protected:
  void updateTrackStatus();
};

#endif // SERVER_H
