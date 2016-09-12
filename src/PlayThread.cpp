#include "PlayThread.h"

/**
 * @file PlayThread.h
 * @brief Audio player implementation
 */

#include "Track.h"

#include <io/proxies/SfxInputProxy.h>
#include <stream_io/RtAudioOutput.h>
#include <RtAudio.h>
#include <QDebug>

PlayThread::PlayThread(QSettings* c) :
    QThread(0), m_options(c) {

    m_options->beginGroup("default");
    setThreshold(m_options->value("threshold").toInt());
    m_options->endGroup();
}

unsigned int PlayThread::getTracksCount() const {
    return m_tracks.size();
}

Track* PlayThread::getTrack(const unsigned int track){
    return (isValidTrack(track) ? m_tracks[track] : NULL);
}

int PlayThread::getActivatedTracks() const{
    int res = 0;
    for(int i= 0; i<m_tracks.size(); i++)
        res += (m_tracks[i]->isActivated() ? pow(2,i) : 0);

    return res;
}

int PlayThread::getThreshold() const {
    return m_threshold;
}

void PlayThread::run() {
    m_manager->input()->reset();
    m_bufferCount = 0;
    m_isPlaying = true;
    m_manager->execute();
}

void PlayThread::setMasterVolume(const unsigned int vol) {
    m_masterVolume->setGain(vol / 10.0);
}

void PlayThread::setVolume(const unsigned int track, const unsigned int vol) {
    if(isValidTrack(track))
        m_tracks[track]->setVolume(vol);
}

void PlayThread::setPan(const unsigned int track, const int pan) {
    if(isValidTrack(track))
        m_tracks[track]->setPan(pan);
}

void PlayThread::setMute(const unsigned int track, const bool doMute) {
    if(isValidTrack(track))
        m_tracks[track]->setMute(doMute);
}

void PlayThread::timeHandle() {
    if(++m_bufferCount > m_maxBufferCount)
        m_bufferCount = 0;

    emit actualBeatChanged(m_bufferCount
                           * m_conf.bufferSize
                           / double(m_conf.samplingRate));
}

void PlayThread::stop() {
    if(this->isRunning()){
        m_manager->stop();
        m_manager->input()->reset();
        m_bufferCount = 0;
        m_isPlaying = false;
    }
}

bool PlayThread::isStopped() const {
    return !m_isPlaying;
}

void PlayThread::reset() {
    m_options->beginGroup("default");
    setMasterVolume(m_options->value("master").toInt());
    m_options->endGroup();

    for(int i=0; i<m_tracks.size(); i++)
        m_tracks[i]->reset();
}

void PlayThread::solo(const unsigned int track, const bool state) {
    if(isValidTrack(track)) {
        m_tracks[track]->setSolo(state);

        if(state) {
            m_tracks[track]->setMute(false);

            for(int i=0; i<m_tracks.size(); i++)
                if(!m_tracks[i]->isSolo())
                    m_tracks[i]->setMute(true);
        } else {
            bool noMoreSolo = true;
            for(int i=0; i<m_tracks.size() && noMoreSolo; i++)
                if(m_tracks[i]->isSolo())
                    noMoreSolo = false;

            if(noMoreSolo)
                for(int i=0; i<m_tracks.size(); i++)
                    m_tracks[i]->setMute( !m_tracks[i]->isActivated() );
            else
                m_tracks[track]->setMute( true );
        }
    }
}

void PlayThread::switchBox(const unsigned int track) {
    if(isValidTrack(track))
        m_tracks[track]->setActivated(!m_tracks[track]->isActivated());
}

void PlayThread::setThreshold(const unsigned int threshold){
    m_threshold = (99 -  threshold) * 4 + 100;
}

void PlayThread::resetThreshold() {
    m_options->beginGroup("default");
    setThreshold(m_options->value("threshold").toInt());
    m_options->endGroup();
}

void PlayThread::load(const SongData& s) {
    // Reset to 0
    m_bufferCount = 0;
    int track_count = s.tracks.size();
    for(int i=0; i<m_tracks.size(); i++)
        delete m_tracks[i];
    m_tracks.clear();
    m_manager.reset();

    // Loading
    m_tracks.resize(track_count);
    std::vector<Input_p> chains(track_count);

#pragma omp parallel for
    for(int i = 0; i < track_count; i++) {
        Track* t = new Track(s.tracks[i], m_conf, m_options, i);
        connect(t, &Track::onActivationSwitch,
                this, &PlayThread::onEnablementChanged);
        m_tracks[i] = t;

        auto file = new FFMPEGFileInput<double>(m_tracks[i]->getFile(), m_conf);
        m_maxBufferCount = file->v(0).size() / m_conf.bufferSize;
        emit beatCountChanged(file->v(0).size() / double(m_conf.samplingRate));

        chains[i] = Input_p(new SfxInputProxy<double>(
                                new StereoAdapter<double>(
                                    new LoopInputProxy<double>(file)),
                            new Sequence<double>(m_conf,
                                                 m_tracks[i]->getVolumePtr(),
                                                 m_tracks[i]->getPanPtr(),
                                                 m_tracks[i]->getMutePtr() )));

        emit songLoaded(i+1,track_count);
    }

    // Master
    auto input = Input_p(new SfxInputProxy<double>(
                             new SummationProxy<double>(
                                 new InputMultiplexer<double>(m_conf, chains)),
                             m_masterVolume));

    // Manager
    m_manager = std::make_shared<StreamingManager<double>>(std::move(input),
                                                           std::move(
                                                               std::make_shared<RtAudioOutput<double>>(
                                                                   m_conf)),
                                                           std::bind(
                                                               &PlayThread::timeHandle,
                                                               this),
                                                           m_conf);
}

bool PlayThread::isValidTrack(unsigned int track)
{
    bool isValid = track < getTracksCount();
    if(!isValid)
        qCritical() << tr("ERROR : Inexistent track") << track;

    return isValid;
}

void PlayThread::onEnablementChanged(bool enabled, int track) {
    emit muteChanged(track,
                     !enabled);
}
