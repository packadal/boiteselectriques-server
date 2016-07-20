#include "PlayThread.h"
#include "Track.h"

//#define CHANNEL_INDEX channels.indexOf(qobject_cast<ChannelWidget*>(QObject::sender()))

#include <io/SfxInputProxy.h>
#include <stream_io/RtAudioOutput.h>
#include <RtAudio.h>
#include <QDebug>

PlayThread::PlayThread() :
    QThread(0) {
    qDebug() << "PlayThread";
}

unsigned int PlayThread::getTracksCount() const {
    return tracks.size();
}

Track* PlayThread::getTrack(const unsigned int track){
    return tracks[track];
}

int PlayThread::getActivatedTracks() const{
    int res= 0;
    for(int i= 0; i<tracks.size(); i++)
        if(tracks[i]->isActivated())
            res+= pow(2,i);
    return res;
}

int PlayThread::getThreshold() const {
    return m_threshold;
}

void PlayThread::run() {
    manager->input()->reset();
    bufferCount = 0;
    isPlaying = true;
    manager->execute();
}

void PlayThread::setMasterVolume(const int vol) {
    masterVolume->setGain(vol / 10.0);
}

void PlayThread::setVolume(const int track, const int vol) {
    //volumes[track]->setGain(vol / 100.0);
    tracks[track]->setVolume(vol);
}

void PlayThread::setPan(const int track, const int pan) {
    //pans[track]->setPan(pan / 100.0);
    tracks[track]->setPan(pan);
}

void PlayThread::setMute(const int track, const bool doMute) {
    //doMute? mutes[track]->mute() : mutes[track]->unmute();
    qDebug() << "THREAD::SET_MUTE" << track << (doMute ? "TRUE" : "FALSE");
    tracks[track]->setMute(doMute);
}

void PlayThread::timeHandle() {
    if(++bufferCount > maxBufferCount) bufferCount = 0;
    emit actualBeatChanged(bufferCount * conf.bufferSize / double(conf.samplingRate));
}

void PlayThread::stop() {
    manager->stop();
    manager->input()->reset();
    bufferCount = 0;
    isPlaying = false;
}

bool PlayThread::isStopped() {
    return !isPlaying;
}

void PlayThread::reset() {
    setMasterVolume(DEFAULT_MASTER_VOLUME);
    for(int i=0; i<tracks.size(); i++)
        tracks[i]->reset();
}

void PlayThread::solo(const int track, const bool state) {
    qDebug() << "PLAYTHREAD::SOLO" << track << (state ? "TRUE" : "FALSE");
    if(track < tracks.size())
        tracks[track]->setSolo(state);

    if(state) {
        tracks[track]->setMute(false);

        qDebug() << "TO_MUTE:";
        for(int i=0; i<tracks.size(); i++)
            if(!tracks[i]->isSolo()){
                qDebug() << "MUTE->" << i;
                tracks[i]->setMute(true);
            }
    } else {
        qDebug() << "UNMUTE ?";

        bool noMoreSolo = true;
        for(int i=0; i<tracks.size() && noMoreSolo; i++)
            if(tracks[i]->isSolo())
                noMoreSolo = false;

        if(noMoreSolo)
        {
            qDebug() << "UNMUTE :";
            for(int i=0; i<tracks.size(); i++)
            {
                qDebug() << "UNMUTE->" << i;
                tracks[i]->setMute( !tracks[i]->isActivated() );
            }
        } else
            tracks[track]->setMute( true );
    }
}

void PlayThread::switchBox(const unsigned int track) {
    qDebug() << (tracks[track]->isActivated() ? "DES" : "") << "ACTIVATE";
    tracks[track]->setActivated(!tracks[track]->isActivated());
    qDebug() << "SWITCHED";
}

void PlayThread::setThreshold(const int threshold){
    qDebug() << "THREAD::SET_THRESHOLD" << threshold;
    m_threshold = (99 -  threshold) * 4 + 100;
    qDebug() << "NEW THRESHOLD" << m_threshold;
}

void PlayThread::resetThreshold() {
    setThreshold(DEFAULT_THRESHOLD);
}

void PlayThread::load(const SongData& s) {
    //// Remise à 0
    bufferCount = 0;
    int track_count = s.tracks.size();
    /*volumes.clear();
    pans.clear();
    mutes.clear();*/
    for(int i=0; i<tracks.size(); i++)
        delete tracks[i];
    tracks.clear();

    manager.reset();

    /*volumes.resize(track_count);
    pans.resize(track_count);
    mutes.resize(track_count);*/
    tracks.resize(track_count);

    //// Chargement
    std::vector<Input_p> chains(track_count);

#pragma omp parallel for
    for(int i = 0; i < track_count; i++) {
        /*volumes[i] = std::make_shared<Amplify<double>>(conf);
        pans[i] = std::make_shared<Pan<double>>(conf);
        mutes[i] = std::make_shared<Mute<double>>(conf);*/

        Track* t = new Track(s.tracks[i], conf, i);
        //connect(channels.last(), SIGNAL(on_enable(bool)), this, SLOT(on_enablementChanged(bool)));
        connect(t, &Track::on_enable, this, &PlayThread::onEnablementChanged);
        tracks[i] = t;


        auto file = new FFMPEGFileInput<double>(/*s.tracks[i].file*/tracks[i]->getFile(), conf);
        maxBufferCount = file->v(0).size() / conf.bufferSize;
        emit beatCountChanged(file->v(0).size() / double(conf.samplingRate));
        chains[i] = Input_p(new SfxInputProxy<double>(new StereoAdapter<double>(new LoopInputProxy<double>(file)),
                            new Sequence<double>(conf, tracks[i]->getVolumePtr(), tracks[i]->getPanPtr(), tracks[i]->isMute() /*volumes[i], pans[i], mutes[i]*/)));
        emit songLoaded(i+1,track_count);
    }

    // Piste master
    auto input = Input_p(new SfxInputProxy<double>(new SummationProxy<double>(new InputMultiplexer<double>(conf, chains)), masterVolume));

    // Manager
    manager = std::make_shared<StreamingManager<double>>(std::move(input),
              std::move(std::make_shared<RtAudioOutput<double>>(conf)),
              std::bind(&PlayThread::timeHandle, this),
              conf);
}

void PlayThread::onEnablementChanged(bool enabled, int track) {
    qDebug() << "enablementChanged";
    qDebug() << "emit MUTE_CHANGED";
             /*<< ( enabled ? "TRUE" : "FALSE" )
             << box;*/
    emit muteChanged(track,
                     !enabled);
}
