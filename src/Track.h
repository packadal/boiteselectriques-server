#ifndef TRACK_H
#define TRACK_H

/**
 * @file Track.h
 * @brief Tracks manipulation interface
 */

#include "SongData.h"

#include <QDebug>

#include <QString>
#include <QSettings>
#include <benchmark/Amplify.h>
#include <benchmark/Pan.h>
#include <benchmark/Mute.h>

/**
 * @brief The Track class
 *
 * Class to handle the tracks' informations
 */

class Track : public QObject
{
    Q_OBJECT
private:
    unsigned int m_id; /*< Track number (default : its position in the PlayThread Tracks vector) */
    std::string m_file; /*< Filepath */

    std::string m_name;
    bool m_soloState;
    bool m_activatedState;

    QSettings* m_options;

    std::shared_ptr<Amplify<double>> m_volumePtr; /*< Volume in the audio engine */
    std::shared_ptr<Pan<double>> m_panPtr; /*< Pan in the audio engine */
    std::shared_ptr<Mute<double>> m_muteState; /*< Mute state in the audio engine */

    double m_volume; /*< "Raw" volume */
    double m_pan; /*< "Raw" pan */

public:
    Track();
    /**
     * @brief Track constructor
     * @param data Track informations
     * @param conf Default configuration data (volume, pan, mute)
     * @param id Track number (should be its position in the PlayThread's tracks vector)
     */
    Track(const TrackData& data, Parameters<double> conf,
          QSettings* opt, int id);

    std::string getName() const;
    std::string getFile() const;
    double getVolume() const;
    double getPan() const;
    bool isActivated() const;
    bool isSolo() const;

    std::shared_ptr<Amplify<double>> getVolumePtr() const;
    std::shared_ptr<Pan<double>> getPanPtr() const;
    std::shared_ptr<Mute<double>> getMutePtr() const;

    void setMute(const bool state);

    /**
     * @brief Volume setter
     * @param vol New volume value
     *
     * Update the value and the pointer
     */
    void setVolume(const double vol);
    /**
     * @brief Pan setter
     * @param pan New pan value
     *
     * Update the value and the pointer
     */
    void setPan(const double pan);

    /**
     * @brief Activate/Deactivate a track
     * @param state New activation state (true = activated, false else)
     */
    void setActivated(const bool state);

    /**
     * @brief Change the track's solo mode status
     * @param state New solo mode status (true = solo, false else)
     */
    void setSolo(const bool state);

    /**
     * @brief Signal the new track state (activated or not)
     *
     * If possible (i.e. the track is not in solo mode),
     * signals the new track's activation status.
     */
    void notifyEnabled(bool);

    /**
     * @brief Reset the track's settings to their default values
     */
    void reset();

signals:
    /**
     * @brief Notify of the change of the track's activation status
     * @param status New activation status
     * @param id Related track number
     */
    void onActivationSwitch(bool status, int id);
};

#endif // TRACK_H
