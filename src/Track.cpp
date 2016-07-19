#include "Track.h"

Track::Track():
    m_soloState(false), m_activatedState(false), m_id(0)
{}

Track::Track(const TrackData& data, Parameters<double> conf, int id):
    m_file(data.file), m_name(data.name), m_id(id),
    m_soloState(false), m_activatedState(false)
{
    m_volumePtr = std::make_shared<Amplify<double>>(conf);
    m_panPtr = std::make_shared<Pan<double>>(conf);
    m_muteState = std::make_shared<Mute<double>>(conf);

    setVolume(data.volume);
    setPan(data.pan);
    setMute(true);

    //connect(this, &Track::on_enable, parent, &PlayThread::on_enablementChanged);
}

void Track::slot_enable(bool enabled) {
    if(!m_soloState) {
        qDebug() << "emit TRACK::ON_ENABLE" << (enabled ? "TRUE" : "FALSE") << m_id;
        emit on_enable(enabled, m_id);
    }
}
