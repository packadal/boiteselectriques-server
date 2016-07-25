#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

/**
 * @file PlayThread.h
 * @brief Audio player interface
 */

#include "SongData.h"
#include "Track.h"

#include <QThread>
#include <manager/StreamingManager.h>
#include <unistd.h>
#include <io/inputs/FFMPEGFileInput.h>
#include <io/proxies/LoopInputProxy.h>
#include <io/proxies/InputMultiplexer2.h>
#include <io/proxies/StereoAdapter.h>
#include <io/proxies/SummationProxy.h>
#include <benchmark/Amplify.h>
#include <benchmark/Pan.h>
#include <benchmark/Mute.h>
#include <benchmark/Sequence.h>

template<typename T>
class StreamingManager;


#define DEFAULT_THRESHOLD 200 /*< Default raw threshold value */
#define DEFAULT_MASTER_VOLUME 50 /*< Default master volume value */

/**
 * @brief The PlayThread class
 *
 * Liaison with the audio engine
 */
class PlayThread : public QThread {
    Q_OBJECT
public:
    explicit PlayThread();

    /**
     * @brief Give the number of the song's tracks
     * @return Tracks count of the current song
     */
    unsigned int getTracksCount() const;
    /**
     * @brief Access a track from its number
     * @param track Track number
     * @return Pointer to the asked Track
     */
    Track* getTrack(const unsigned int track);
    /**
     * @brief Give the number tracks currently active
     * @return Count of activated tracks
     */
    int getActivatedTracks() const;
    /**
     * @brief Threshold getter
     * @return Raw threshold value
     */
    int getThreshold() const;

    /**
     * @brief Give the playing status
     * @return true if the playing is stopped, false else
     */
    bool isStopped() const;

signals:
    /**
     * @brief Notify of the modification of the actual beat value
     * @param time Actual time (seconds)
     */
    void actualBeatChanged(double time);
    /**
     * @brief Notify of the total beat count modification
     * @param time Actual time (seconds)
     *
     * For example, called when a new song is loaded
     */
    void beatCountChanged(double time);
    /**
     * @brief Notify of the mute value modification
     * @param box Related track number
     * @param state New state
     */
    void muteChanged(int box, bool state);
    /**
     * @brief Notify of the song loading end
     * @param on Next track number
     * @param max Max track number
     */
    void songLoaded(int,int);

public slots:
    /**
     * @brief Play the song
     */
    void run();

    /**
     * @brief Stop the playing
     */
    void stop();

    /**
     * @brief Master volume setter
     * @param vol New volume (between 0 and 100)
     */
    void setMasterVolume(const unsigned int vol);
    /**
     * @brief Track volume setter
     * @param track Related track number
     * @param vol New volume (between 0 and 100)
     */
    void setVolume(const unsigned int track, const unsigned int vol);
    /**
     * @brief Track pan setter
     * @param track Related track number
     * @param pan New pan value (between -100 and 100)
     */
    void setPan(const unsigned int track, const int pan);
    /**
     * @brief (Un)Mute a track
     * @param track Related track number
     * @param doMute New mute state
     *
     * Mute a given track with doMute = true, unmute it else
     */
    void setMute(const unsigned int track, const bool doMute);

    /**
     * @brief Change the solo state of a track
     * @param track Related track number
     * @param state New solo state
     */
    void solo(const unsigned int track, const bool state);

    /**
     * @brief Reset the song to its default values
     */
    void reset();

    /**
     * @brief Called when the activated status of a track is changed
     * @param enabled New activation status
     * @param track Related track number
     */
    void onEnablementChanged(bool enabled, int track);

    /**
     * @brief Switch a box activation status
     * @param track Related track number
     */
    void switchBox(const unsigned int track);

    /**
     * @brief Threshold setter
     * @param "New" (not raw) threshold value (0-100)
     */
    void setThreshold(const unsigned int threshold);

    /**
     * @brief Reset threshold to its default value
     */
    void resetThreshold();

    /**
     * @brief Regularly called to update the beat value
     */
    void timeHandle();

    /**
     * @brief Load a song in the engine
     * @param s Song's informations
     */
    void load(const SongData& s);

private:
    Parameters<double> conf; /*< Configuration data */
    std::shared_ptr<Amplify<double>> masterVolume {new Amplify<double>(conf)};
    std::shared_ptr<StreamingManager<double>> manager;

    QVector<Track*> tracks; /*< List of the current song's tracks */

    int bufferCount {}; /*< Buffer in which we are s*/
    int maxBufferCount {}; /*< Total buffer count in a loop */

    bool isPlaying {false};
    int m_threshold {DEFAULT_THRESHOLD};

    /**
     * @brief Check if a given track exists
     * @param track Track number
     * @return true if the track exists, false else
     */
    bool isValidTrack(unsigned int track);
};

#endif // PLAYTHREAD_H
