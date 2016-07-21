#ifndef SERVER_H
#define SERVER_H

/**
 * @file Server.h
 * @brief Server interface
 */

#include "PlayThread.h"
#include "SaveManager.h"
#include "SerialManager.h"

#include "osc/oscreceiver.h"
#include "osc/oscsender.h"
#include "osc/oscmessagegenerator.h"

class SaveManager;

#define EXPORT_FOLDER "/home/ubuntu/songs/" /*< Files save/load folder*/
#define FILES_EXTENSION "*.song" /*< Song files extension */

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
    PlayThread player; /*< Audio play manager */
    SaveManager saveManager; /*< File handling manager */
    SerialManager serialManager {this}; /*< Interface with serial port */

    SongData song; /*< Actual song's data */
    QString selSong; /*< Selected song's name */
    int threshold;
    int nbChannels;
    QString currentFile {};

    bool m_loaded {false}; /*< Indicate if a song has been loaded */
    bool m_playing {false}; /*< Indicate if a song is playing */

    int m_tempo {};
    double m_beatCount {};
    // Optimization : Comparison with the previous beat
    int m_previousBeat {-1};


    OscReceiver receiver {9988}; /*< Receiving interface with OSC protocol */
    OscSender sender {"192.170.0.17", 9989}; /*< Sending interface with OSC protocol */

    /***************************
     * TRANSMISSIONS TO CLIENT *
     ***************************/

    // Only send the corresponding data using OSC protocol

    /**
     * @brief Send the actual threshold value to the client
     * @param (Calculated) Threshold value (obtained with getThreshold() )
     */
    void sendMsgThreshold(int boxSensor);
    /**
     * @brief Notify the client of a box activation
     * @param Track number
     */
    void sendMsgBoxActivation(unsigned int chan);
    /**
     * @brief Send the activated tracks' numbers to the client
     * @param val Activated tracks
     *
     * Called each 8 beats, to keep the client synchronized
     */
    void sendMsgActivatedTracks(int val);
    /**
     * @brief Send the actual beat count
     * @param beat Beat count
     */
    void sendMsgBeatCount(unsigned int beat);
    /**
     * @brief Notify the client of the song's playing start
     * @param tempo Song's tempo
     */
    void sendMsgPlay(unsigned int tempo);
    /**
     * @brief Send the actual song's title
     * @param title Song's title
     */
    void sendMsgSongTitle(const char *title);
    /**
     * @brief Send the available songs' list
     * @param list Songs' titles list
     */
    void sendMsgSongsList(const char *list);
    /**
     * @brief Send the song's number of tracks
     * @param num Count of tracks
     */
    void sendMsgTracksCount(unsigned int num);
    /**
     * @brief Send the song's track list
     * @param list Tracks list
     */
    void sendMsgTracksList(const char *list);
    /**
     * @brief Notify the client of the loading state
     * @param isReady Server's loading state
     */
    void sendMsgReady(bool sendMsgReady);


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

public:
    explicit Server();
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

signals:
    /**
     * @brief Notify the need to reload the actual song
     */
    void actionLoad();
    /**
     * @brief Notify of a new threshold value
     * @param threshold New threshold value
     */
    void updateThreshold(int threshold);
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
    void updateTrackList(const char *list);

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
     * @brief Notify the client of the activation of a box
     * @param i Track (box) number
     * @param val Sensor value
     */
    void sendBoxActivation(unsigned int i, int val);
    /**
     * @brief Send the actual beat count
     * @param beat Beat count
     */
    void sendBeatCount(unsigned int beat);
    /**
     * @brief Send the actual song's title
     * @param title Song's title
     */
    void sendSongTitle(const char* title);
    /**
     * @brief Send the available songs' list
     * @param list Songs' titles list
     */
    void sendSongsList(const char* list);
    /**
     * @brief Send the song's number of tracks
     * @param num Count of tracks
     */
    void sendTracksCount(unsigned int num);
    /**
     * @brief Notify the client of the loading state
     * @param isReady Server's loading state
     *
     * Send true if all the songs are loaded, false else
     */
    void sendReady(bool isReady);
};

#endif // SERVER_H
