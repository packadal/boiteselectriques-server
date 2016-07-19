#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#define DEFAULT_THRESHOLD 200

#include "SongData.h"
#include "Track.h"

#include <QThread>
#include <manager/StreamingManager.h>
#include <unistd.h>
#include <io/FFMPEGFileInput.h>
#include <io/LoopInputProxy.h>
#include <io/InputMultiplexer2.h>
#include <io/StereoAdapter.h>
#include <io/SummationProxy.h>
#include <benchmark/Amplify.h>
#include <benchmark/Pan.h>
#include <benchmark/Mute.h>
#include <benchmark/Sequence.h>

template<typename T>
class StreamingManager;

/**
 * @brief The PlayThread class
 *
 * Classe qui fait la liaison avec le moteur audio
 */
class PlayThread : public QThread {
    Q_OBJECT
public:
    explicit PlayThread(QObject *parent = 0);

    unsigned int tracksCount() const {
        return tracks.size();
    }
    Track* getTrack(const unsigned int track){
        return tracks[track];
    }

    int getActivatedTracks() const{
        int res= 0;
        for(int i= 0; i<tracks.size(); i++)
            if(tracks[i]->isActivated())
                res+= pow(2,i);
        return res;
    }

    int getThreshold() const {
        return m_threshold;
    }

signals:
    void spentTime(double);
    void setTotalTime(double);
    void muteChanged(int, bool);
    // quand le morceau es chargé
    void charged(int,int);

public slots:
    // A appeler quand on veut faire lecture
    void run();

    // Volume principal
    // Entre 0 et 100
    void setMasterVolume(const int vol) {
        masterVolume->setGain(vol / 10.0);
    }

    // Volume par canal
    // Entre 0 et 100
    void setVolume(const int track, const int vol) {
        //volumes[track]->setGain(vol / 100.0);
        tracks[track]->setVolume(vol);
    }

    // Pan
    // Entre -100 et 100
    void setPan(const int track, const int pan) {
        //pans[track]->setPan(pan / 100.0);
        tracks[track]->setPan(pan);
    }

    // Sourdine d'un canal
    void setMute(const int track, const bool doMute) {
        //doMute? mutes[track]->mute() : mutes[track]->unmute();
        qDebug() << "THREAD::SET_MUTE" << track << (doMute ? "TRUE" : "FALSE");
        tracks[track]->setMute(doMute);
    }

    // Appelé régulièrement pour mettre à jour le beat ou on se trouve
    // dans l'interface
    void timeHandle() {
        if(++bufferCount > maxBufferCount) bufferCount = 0;
        emit spentTime(bufferCount * conf.bufferSize / double(conf.samplingRate));
    }

    // Charge un morceau dans le moteur
    void load(const SongData& s);

    // Arrête la lecture
    void stop();

    bool isStopped() {
        return !isPlaying;
    }

    void reset() {
        setMasterVolume(50);
        for(int i=0; i<tracks.size(); i++)
            tracks[i]->reset();
    }

    void solo(const int box, const bool state) {
        qDebug() << "PLAYTHREAD::SOLO" << box << (state ? "TRUE" : "FALSE");
        if(box < tracks.size())
            tracks[box]->setSolo(state);

        if(state) {
            tracks[box]->setMute(false);

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
                tracks[box]->setMute( true );
        }
    }

    void switchBox(const unsigned int box) {
        qDebug() << (tracks[box]->isActivated() ? "DES" : "") << "ACTIVATE";
        tracks[box]->setActivated(!tracks[box]->isActivated());
        qDebug() << "SWITCHED";
    }

    void setThreshold(const int threshold){
        qDebug() << "THREAD::SET_THRESHOLD" << threshold;
        m_threshold = (99 -  threshold) * 4 + 100;
        qDebug() << "NEW THRESHOLD" << m_threshold;
    }

    void resetThreshold() {
        setThreshold(DEFAULT_THRESHOLD);
    }

    void on_enablementChanged(bool, int); // Bouton contrôlé par les boîtes

private:
    Parameters<double> conf;
    std::shared_ptr<Amplify<double>> masterVolume {new Amplify<double>(conf)};
    /*std::vector<std::shared_ptr<Amplify<double>>> volumes;
    std::vector<std::shared_ptr<Pan<double>>> pans;
    std::vector<std::shared_ptr<Mute<double>>> mutes;*/
    std::shared_ptr<StreamingManager<double>> manager;

    QVector<Track*> tracks;

    // Buffer auquel on se trouve
    int bufferCount {};

    // Nombre de buffers total dans une boucle
    int maxBufferCount {};

    bool isPlaying {false};
    int m_threshold {DEFAULT_THRESHOLD};
};

#endif // PLAYTHREAD_H
