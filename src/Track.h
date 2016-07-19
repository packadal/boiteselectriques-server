#ifndef TRACK_H
#define TRACK_H

#include "SongData.h"

#include <QDebug>

#include <QString>
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
    std::string m_file;
    std::string m_name;
    unsigned int m_id;

    double m_volume, m_pan;
    std::shared_ptr<Amplify<double>> m_volumePtr;
    std::shared_ptr<Pan<double>> m_panPtr;

    std::shared_ptr<Mute<double>> m_muteState;
    bool m_soloState;
    bool m_activatedState;

public:
    Track();
    Track(const TrackData&, Parameters<double>, int id);

    std::string getName() const {
        return m_name;
    }
    std::string getFile() const {
        return m_file;
    }
    double getVolume() const {
        return m_volume;
    }
    double getPan() const {
        return m_pan;
    }
    std::shared_ptr<Amplify<double>> getVolumePtr() const{
        return m_volumePtr;
    }
    std::shared_ptr<Pan<double>> getPanPtr() const{
        return m_panPtr;
    }
    std::shared_ptr<Mute<double>> isMute() const {
        return m_muteState;
    }
    bool isActivated() const {
        return m_activatedState;
    }
    bool isSolo() const {
        return m_soloState;
    }

    void setVolume(const double vol) {
        m_volume = vol;
        m_volumePtr->setGain(m_volume / 100.0);
    }
    void setPan(const double pan) {
        m_pan = pan;
        m_panPtr->setPan(m_pan / 100.0);
    }
    void setMute(const bool state) {
        qDebug() << "TRACK::SET_MUTE" << (state ? "TRUE" : "FALSE");
        state ? m_muteState->mute() : m_muteState->unmute();
    }
    void setActivated(const bool state) {
        m_activatedState = state;
        qDebug() << "track " << (isActivated() ? "activated" : "desactivated");
        slot_enable(state);
    }
    // Appelé lorsqu'on tape sur la boite
    void slot_enable(bool);

    void setSolo(const bool state) {
        m_soloState = state;
    }

    void reset() {
        setVolume(50);
        setPan(0);
        setActivated(false);
    }

signals:
    void on_enable(bool, int);
};

#endif // TRACK_H
