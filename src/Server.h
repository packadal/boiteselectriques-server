#ifndef SERVER_H
#define SERVER_H

/**
 * @file Server
 * @brief Server interface
 */

#include "PlayThread.h"
#include "SaveManager.h"
#include "SerialManager.h"

#include "osc/oscreceiver.h"
#include "osc/oscsender.h"
#include "osc/oscmessagegenerator.h"

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <exception>

class SaveManager;

/**
 * @brief Main class
 *
 * Handles the events and dispatch the corresponding actions
 */
class Server : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int tempo READ getTempo WRITE setTempo)

    friend class SaveManager;
public:
    explicit Server(QWidget *parent = 0);
    ~Server();

    /**
     * @brief Tempo getter
     * @return Tempo value
     */
    int getTempo() const;

    /*******************
     * EVENTS HANDLING *
     *******************/

    /**
     * @brief update_threshold event handling
     * @param args Event New threshold value
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

signals:
    //void select_Song(const char *);
    void actionLoad();
    void updateThreshold(int);
    void resetThreshold();

public slots:
    /**
     * @brief Calculate the threshold value
     * @return Threshold value to transmit
     */
    int getThreshold();

    /**
     * @brief Reset the values to default
     */
    void reset();

    /**
     * @brief Change a track status
     * @param i Track number
     * @param val Sensibility value (to validate the action)
     */
    void switchBox(int i, int val);

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
     * @brief Update the beat value
     * @param t Actual time (in seconds)
     */
    void updateBeat(double t);

    /**
     * @brief Update the beat count
     * @param t Actual time (in seconds)
     */
    void updateBeatCount(double t);

    /**
     * @brief Actions to perform at the end of the song's loading
     * @param on Next track number
     * @param max Max track number
     */
    void onIsLoaded(int on, int max);

    /**
     * @brief Update the client's track list
     * @param list Track list
     */
    void updateTrackList(const char *list);

    /**
     * @brief Tempo setter
     * @param arg New tempo value
     */
    void setTempo(int arg);


    /***************************
     * TRANSMISSIONS TO TABLET *
     ***************************/

    /**
     * @brief Send the actual threshold value to the client
     * @param (Calculated) Threshold value (obtained with getThreshold() )
     */
    void sendThreshold(int boxSensor);
    /**
     * @brief Notify the client of a box activation
     * @param Track number
     */
    void sendBoxActivation(int chan);
    /**
     * @brief Send the activated tracks' numbers to the client
     * @param val Activated tracks
     *
     * Called each 8 beats, to keep the client synchronized
     */
    void sendActivatedTracks(int val);
    /**
     * @brief Send the actual beat count
     * @param beat Beat count
     */
    void sendBeatCount(int beat);
    /**
     * @brief Notify the client of the song's playing start
     * @param tempo Song's tempo
     */
    void sendPlay(int tempo);
    /**
     * @brief Send the actual song's title
     * @param title Song's title
     */
    void sendSongTitle(const char *title);
    /**
     * @brief Send the available songs' list
     * @param list Songs' titles list
     */
    void sendSongsList(const char *list);
    /**
     * @brief Send the song's number of tracks
     * @param num Count of tracks
     */
    void sendTracksCount(int num);
    /**
     * @brief Send the song's track list
     * @param list Tracks list
     */
    void sendTrackList(const char *list);
    /**
     * @brief Notify the client of the loading state
     * @param isReady Server's loading state
     *
     * Send true if all the songs are loaded, false else
     */
    void ready(bool ready) {
        sender.send(osc::MessageGenerator()("/box/ready_to_go", ready));
    }

    //envoi sur tablette
    void send_Boite(int,int);
    void send_Beat(int);
    void send_Titre(const char *);
    void send_Liste(const char *);
    void send_numb_track(int);
    void send_go(bool);


private:
    //Ui::MainWidget *ui;
    PlayThread playThread;
    SaveManager savemanager;
    SerialManager serialmanager {this};
    //ConfigurationDialog confdialog {this};
    OscReceiver oscReceiver {9988};

    SongData song;

    QString selSong;

    int threshold;
    int nbChannels;

    QString currentFile {};

    int m_tempo {};
    double m_beatCount {};

    // Optimisation : on compare avec le temps précédent.
    int m_previousBeat {-1};

    // Indique si un morceau a été chargé.
    bool m_loaded {false};

    // Indique si un morceau est en cours de lecture
    bool m_playing {false};

    OscSender sender {"192.170.0.17", 9989};

};

#endif // SERVER_H
