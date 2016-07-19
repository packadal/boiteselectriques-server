#include "PlayThread.h"
#include "Track.h"

//#define CHANNEL_INDEX channels.indexOf(qobject_cast<ChannelWidget*>(QObject::sender()))

#include <io/SfxInputProxy.h>
#include <stream_io/RtAudioOutput.h>
#include <RtAudio.h>
#include <QDebug>

PlayThread::PlayThread(QObject *parent) :
    QThread(parent) {
    qDebug() << "PlayThread";
}

void PlayThread::run() {
    manager->input()->reset();
    bufferCount = 0;
    isPlaying = true;
    manager->execute();
}

void PlayThread::stop() {
    manager->stop();
    manager->input()->reset();
    bufferCount = 0;
    isPlaying = false;
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
        connect(t, &Track::on_enable, this, &PlayThread::on_enablementChanged);
        tracks[i] = t;


        auto file = new FFMPEGFileInput<double>(/*s.tracks[i].file*/tracks[i]->getFile(), conf);
        maxBufferCount = file->v(0).size() / conf.bufferSize;
        emit setTotalTime(file->v(0).size() / double(conf.samplingRate));
        chains[i] = Input_p(new SfxInputProxy<double>(new StereoAdapter<double>(new LoopInputProxy<double>(file)),
                            new Sequence<double>(conf, tracks[i]->getVolumePtr(), tracks[i]->getPanPtr(), tracks[i]->isMute() /*volumes[i], pans[i], mutes[i]*/)));
        emit charged(i+1,track_count);
    }

    // Piste master
    auto input = Input_p(new SfxInputProxy<double>(new SummationProxy<double>(new InputMultiplexer<double>(conf, chains)), masterVolume));

    // Manager
    manager = std::make_shared<StreamingManager<double>>(std::move(input),
              std::move(std::make_shared<RtAudioOutput<double>>(conf)),
              std::bind(&PlayThread::timeHandle, this),
              conf);
}

void PlayThread::on_enablementChanged(bool enabled, int box) {
    qDebug() << "enablementChanged";
    qDebug() << "emit MUTE_CHANGED";
             /*<< ( enabled ? "TRUE" : "FALSE" )
             << box;*/
    emit muteChanged(box,
                     !enabled);
}
