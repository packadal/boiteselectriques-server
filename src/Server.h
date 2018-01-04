#ifndef SERVER_H
#define SERVER_H

/**
 * @file Server.h
 * @brief Server interface
 */

#include "PlayThread.h"
#include "SaveManager.h"
#include "SerialManager.h"

#include "osc/oscmessagegenerator.h"
#include "osc/oscreceiver.h"
#include "osc/oscsender.h"

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
  Q_PROPERTY(int tempo READ getTempo WRITE setTempo)

  friend class SaveManager;

 private:
  PlayThread* m_player;                /*< Audio play manager */
  SaveManager m_saveManager;           /*< File handling manager */
  SerialManager m_serialManager{this}; /*< Interface with serial port */

  QSettings* m_options; /*< Config options */

  SongData m_song;   /*< Actual song's data */
  QString m_selSong; /*< Selected song's name */
  int m_threshold;
  int m_nbChannels;
  QString m_currentFile;

  bool m_loaded{false};  /*< Indicate if a song has been loaded */
  bool m_playing{false}; /*< Indicate if a song is playing */

  int m_tempo;
  double m_beatCount;
  // Optimization : Comparison with the previous beat
  int m_previousBeat{-1};

  OscReceiver* m_receiver; /*< Receiving interface with OSC protocol */
  OscSender* m_sender;     /*< Sending interface with OSC protocol */

  /**
   * @brief Setup WiringPi interface
   */
  void ledSetup();

  /***************************
   * TRANSMISSIONS TO CLIENT *
   ***************************/

  // Only send the corresponding data using OSC protocol

  /**
   * @brief Send the actual threshold value to the client
   * @param t Threshold value
   */
  void sendMsgThreshold(int t);
  /**
   * @brief Send the activated tracks' numbers to the client
   * @param tracks Tracks numbers
   *
   * Called every 8 beats, to keep the client synchronized
   */
  void sendMsgActivatedTracks(int tracks);
  /**
   * @brief Send the actual beat count
   * @param beat Beat count
   */
  void sendMsgBeatCount(int beat);
  /**
   * @brief Notify the client of the song's playing start
   * @param tempo Actual song's tempo
   */
  void sendMsgPlay(int tempo);
  /**
   * @brief Send the actual song's title
   * @param title Song's title
   */
  void sendMsgSongTitle(const char* title);
  /**
   * @brief Send the available songs' list
   * @param list Songs' titles list
   */
  void sendMsgSongsList(const char* list);
  /**
   * @brief Send the song's number of tracks
   * @param num Count of tracks
   */
  void sendMsgTracksCount(int num);
  /**
   * @brief Send the song's track list
   * @param list Tracks list
   */
  void sendMsgTracksList(const char* list);
  /**
   * @brief Notify the client of the loading state
   * @param isReady Server's loading state
   */
  void sendMsgReady(bool isReady);

  /*******************
   * EVENTS HANDLING *
   *******************/

  /**
   * @brief update_threshold event handling
   * @param args New threshold value
   *
   * Change the threshold value
   */
  void handle__box_updateThreshold(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief reset_threshold event handling
   * @param args Nothing
   *
   * Reset the threshold value
   */
  void handle__box_resetThreshold(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief enable event handling
   * @param args Track number
   *
   * Switch a track state (enable/disable)
   */
  void handle__box_enable(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief volume event handling
   * @param args Track number and new volume (integer)
   *
   * Change the volume of a track
   */
  void handle__box_volume(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief pan event handling
   * @param args Track number and new pan (integer)
   *
   * Change the pan (left-right volume) of a track
   */
  void handle__box_pan(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief mute event handling
   * @param args Track number and new state (boolean)
   *
   * Mute or unmute a track
   */
  void handle__box_mute(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief solo event handling
   * @param args Track number and new state (boolean)
   *
   * "Solo" or "unsolo" a track
   */
  void handle__box_solo(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief play event handling
   * @param args Nothing
   *
   * Play the song
   */
  void handle__box_play(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief stop event handling
   * @param args Nothing
   *
   * Stop the song
   */
  void handle__box_stop(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief master event handling
   * @param args New volume (integer)
   *
   * Change the master volume
   */
  void handle__box_master(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief reset event handling
   * @param args Nothing
   *
   * Stop the song and reset its options to their default values
   */
  void handle__box_reset(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief refresh_song event handling
   * @param args Nothing
   *
   * Refresh the song data
   */
  void handle__box_refreshSong(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief select_song event handling
   * @param args Song's filename (char*)
   *
   * Select a new song
   */
  void handle__box_selectSong(osc::ReceivedMessageArgumentStream args);
  /**
   * @brief sync event handling
   * @param args Nothing
   *
   * Send the informations of the actual song and the current state of the
   * player
   */
  void handle__box_sync(osc::ReceivedMessageArgumentStream args);

 public:
  /**
   * @brief Constructor of the Server class
   * @param opt Configuration data
   */
  explicit Server(QSettings* opt);
  ~Server();

  /**
   * @brief Tempo getter
   * @return Tempo value
   */
  int getTempo() const;

  /**
   * @brief Calculate the threshold value
   * @return Threshold value to transmit
   */
  unsigned int getThreshold() const;

  /**
   * @brief Load or initialize the configuration options
   * @param c Pointer to the options data
   * @return If the config options have been loaded (true) or generated from the
   * default values (false)
   */
  bool initConf(QSettings* c);

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
 signals:
  /**
   * @brief Notify the need to reload the actual song
   */
  void actionLoad();
  /**
   * @brief Notify of a new threshold value
   * @param threshold New threshold value
   */
  void updateThreshold(int m_threshold);
  /**
   * @brief Notify the threshold reset
   */
  void resetThreshold();

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
  int load();

  /**
   * @brief Save the modifications
   */
  void save();

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
   * @param on Next track number
   * @param max Max track number
   */
  void onSongLoaded(unsigned int on, unsigned int max);

  /**
   * @brief Update the client's track list
   * @param list Track list
   */
  void updateTrackList(const char* list);

  /**
   * @brief Tempo setter
   * @param arg New tempo value
   */
  void setTempo(unsigned int arg);

  /***************************
   * TRANSMISSIONS TO CLIENT *
   ***************************/

  // Perform eventual calculations and then call the private sending functions

  /**
   * @brief Send the actual beat count
   * @param beat Beat count
   */
  void sendBeatCount(unsigned int beat);
  /**
   * @brief Send the actual song's title
   */
  void sendSongTitle();
  /**
   * @brief Send the available songs' list
   */
  void sendSongsList();
  /**
   * @brief Send the song's number of tracks
   */
  void sendTracksCount();
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
   * @brief Notify the client of the song's playing start
   */
  void sendPlay();
};

#endif  // SERVER_H
