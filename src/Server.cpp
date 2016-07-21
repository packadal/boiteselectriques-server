#include "Server.h"

/**
 * @file Server.cpp
 * @brief Server implementation
 */

#include <QDebug>
#include <exception>

Server::Server() {

    connect(&player, &PlayThread::muteChanged,
            &player, &PlayThread::setMute);

    connect(this, &Server::updateThreshold,
            &player, &PlayThread::setThreshold);
    connect(this, &Server::resetThreshold,
            &player, &PlayThread::resetThreshold);

    connect(&player,	&PlayThread::actualBeatChanged,
            this,		&Server::updateBeat);
    connect(&player,	&PlayThread::beatCountChanged,
            this,		&Server::updateBeatCount);

    connect(&player,	&PlayThread::songLoaded,
            this,		&Server::onSongLoaded);
    connect(&saveManager,	&SaveManager::updatedTracksList,
            this,		&Server::updateTrackList);

    connect(&serialManager,	&SerialManager::boxActivated,
            this,			&Server::switchBox);
    connect(&serialManager,	&SerialManager::boxActivated,
            this,			&Server::sendBoxActivation);

    serialManager.start();

    //Client events
    receiver.addHandler("/box/update_threshold",
                           std::bind(&Server::handle__box_updateThreshold,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/reset_threshold",
                           std::bind(&Server::handle__box_resetThreshold,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/enable",
                           std::bind(&Server::handle__box_enable,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/volume",
                           std::bind(&Server::handle__box_volume,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/pan",
                           std::bind(&Server::handle__box_pan,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/mute",
                           std::bind(&Server::handle__box_mute,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/solo",
                           std::bind(&Server::handle__box_solo,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/master",
                           std::bind(&Server::handle__box_master,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/play",
                           std::bind(&Server::handle__box_play,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/stop",
                           std::bind(&Server::handle__box_stop,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/reset",
                           std::bind(&Server::handle__box_reset,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/refresh_song",
                           std::bind(&Server::handle__box_refreshSong,
                                     this, std::placeholders::_1));
    receiver.addHandler("/box/select_song",
                           std::bind(&Server::handle__box_selectSong,
                                     this, std::placeholders::_1));

    receiver.run();
}

Server::~Server() {
    stop();
    player.wait();

    serialManager.stop();
    serialManager.wait();
}

int Server::getTempo() const {
    return m_tempo;
}

unsigned int Server::getThreshold() const {
    return 99 - (player.getThreshold()-100)/4;
}

void Server::sendReady(bool isReady) {
    sendMsgReady(isReady);
}

void Server::sendTracksCount(unsigned int i) {
    sendMsgTracksCount(i);
}

void Server::sendBoxActivation(unsigned int i, int val) {
    if(val >= player.getThreshold()){
        sendMsgBoxActivation(i);
    }
}

void Server::sendBeatCount(int i) {
    sendMsgBeatCount(i);
}

void Server::sendSongsList(const char *i) {
    sendMsgSongsList(i);
}

void Server::sendSongTitle(const char *i) {
    sendMsgSongTitle(i);
}

void Server::handle__box_updateThreshold(osc::ReceivedMessageArgumentStream args) {
    osc::int32 senseUpdated;
    args >> senseUpdated;
    qDebug() << "received /box/update_threshold" << senseUpdated;

    emit updateThreshold(senseUpdated);
}

void Server::handle__box_resetThreshold(osc::ReceivedMessageArgumentStream args) {
    osc::int32 senseUpdated;
    args >> senseUpdated;
    qDebug() << "received /box/reset_threshold" << senseUpdated;

    emit resetThreshold();
}

void Server::handle__box_enable(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    args >> box;
    qDebug() << "received /box/enable" << box;

    switchBox(box, player.getThreshold());
}

void Server::handle__box_volume(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    osc::int32 vol;
    args >> box >> vol;
    qDebug() << "received /box/volume" << vol;

    player.setVolume(box, vol);
}

void Server::handle__box_pan(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    osc::int32 vol;
    args >> box >> vol;
    qDebug() << "received /box/pan" << box << vol;

    player.setPan(box, vol);
}

void Server::handle__box_mute(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    bool state;
    args >> box >> state;
    qDebug() << "received /box/mute" << box << state;

    player.setMute(box, state);
}

void Server::handle__box_solo(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    bool state;
    args >> box >> state;
    qDebug() << "received /box/solo" << box << state;

    player.solo(box, state);
}

void Server::handle__box_master(osc::ReceivedMessageArgumentStream args) {
    osc::int32 vol;
    args >> vol;
    qDebug() << "received /box/master" << vol;

    player.setMasterVolume(vol);
}

void Server::handle__box_play(osc::ReceivedMessageArgumentStream args) {
    bool state;
    args >> state;
    qDebug() << "received /box/play" << state;

    play();
}

void Server::handle__box_stop(osc::ReceivedMessageArgumentStream args) {
    bool state;
    args >> state;
    qDebug() << "received /box/stop" << state;

    stop();
}

void Server::handle__box_reset(osc::ReceivedMessageArgumentStream args) {
    bool state;
    args >> state;
    qDebug() << "received /box/reset" << state;

    if(state)
        stop();

    player.reset();
}

void Server::handle__box_refreshSong(osc::ReceivedMessageArgumentStream args) {
    bool state;
    args >> state;
    qDebug() << "received /box/refresh_song" << state;

    load();
}

void Server::handle__box_selectSong(osc::ReceivedMessageArgumentStream args) {
    const char *receptSong;
    args >> receptSong;
    qDebug() << "received /box/select_song" << receptSong;

    QString so = QString(QLatin1String(receptSong));
    if(!so.isEmpty())
        selSong = so;

    connect(this, SIGNAL(actionLoad()),
            this, SLOT(load()) );
    emit actionLoad();
}

void Server::reset() {
    stop();

    player.reset();
}

void Server::switchBox(int i, int val) {
    if(val >= player.getThreshold()) {
        player.switchBox(i);
    }
}

void Server::play() {
    if(!m_loaded) {
        if(load()) return;
    }

    sendMsgPlay(getTempo());
    player.start();
    m_playing = true;
}

void Server::stop() {
    if(!m_loaded || !m_playing) return;

    player.stop();
    m_previousBeat = 0;
    m_playing = false;

    updateBeat(0);
}

void Server::updateBeat(double t) { // in seconds

    int time = (int)(t * getTempo() / 60.0f) + 1;

    if(time != m_previousBeat && time <= m_beatCount && !player.isStopped()) {
        m_previousBeat = time;

        sendBeatCount(time);
        if(time == 8 || time == 16 || time == 24){ //sync
            sendMsgActivatedTracks(player.getActivatedTracks());
            sendBeatCount(time);
        }
    } else if(player.isStopped()) {
        m_previousBeat = 0;
        sendBeatCount(0);
    }
}

void Server::updateBeatCount(double t) { // in seconds
    // Here we calculate the number of beats in the loop
    // Formula : seconds * tempo/60 = nb. beats in loop.

    m_beatCount = t * getTempo() / 60.0;
    m_previousBeat = -1;
}

void Server::onSongLoaded(unsigned int on, unsigned int max) {
    if (on == max)
        sendReady(true);
}

void Server::updateTrackList(const char *list) {
    sendMsgTracksList(list);
}

void Server::setTempo(unsigned int arg) {
    m_tempo = arg;
}

void Server::sendMsgThreshold(int boxSensor) {
    sender.send(osc::MessageGenerator()("/box/sensor", boxSensor));
}

void Server::sendMsgBoxActivation(unsigned int chan) {
    sender.send(osc::MessageGenerator()("/box/enable_out", chan));
}

void Server::sendMsgActivatedTracks(int val) {
    sender.send(osc::MessageGenerator()("/box/enable_sync", val));
}

void Server::sendMsgBeatCount(unsigned int beat) {
    sender.send(osc::MessageGenerator()("/box/beat", beat));
}

void Server::sendMsgPlay(unsigned int tempo) {
    sender.send(osc::MessageGenerator()("/box/play", tempo));
}

void Server::sendMsgSongTitle(const char *title) {
    sender.send(osc::MessageGenerator()("/box/title", title));
}

void Server::sendMsgSongsList(const char *list) {
    sender.send(osc::MessageGenerator()("/box/songs_list", list));
}

void Server::sendMsgTracksCount(unsigned int num) {
    sender.send(osc::MessageGenerator()("/box/tracks_count", num));
}

void Server::sendMsgTracksList(const char *list) {
    sender.send(osc::MessageGenerator()("/box/tracks_list", list));
}

void Server::sendMsgReady(bool ready) {
    sender.send(osc::MessageGenerator()("/box/ready", ready));
}

int Server::load() {
    QDir exportFolder(EXPORT_FOLDER);

    exportFolder.setNameFilters(QStringList()<<FILES_EXTENSION);
    QStringList fileList = exportFolder.entryList();
    QString str = fileList.join("|");
    QByteArray list = str.toLatin1();
    const char *c_list = list.data();
    sendSongsList(c_list);
    sendMsgThreshold(getThreshold());

    if (!selSong.isEmpty()) {
        try {
            stop();

            QString file = EXPORT_FOLDER + selSong;

            if(!file.isEmpty()) {
                currentFile = file;
                song = saveManager.load(file);

                setTempo(song.tempo);
                player.load(song);

                m_loaded = true;
                QString raw_title = QString::fromStdString(song.name);
                QByteArray title = raw_title.toLatin1();
                const char *c_title = title.data();
                sendSongTitle(c_title);

                sendTracksCount(player.getTracksCount());

                return 0;
            }
            return 1;
        } catch(std::exception& e) {
            qCritical() << tr("LOADING ERROR :") << e.what();
            return 1;
        }
    }//end check selsong
    return 1;
}

void Server::save() {
    try {
        if(m_loaded)
            saveManager.save(currentFile, this);
    } catch(std::exception& e) {
        qCritical() << tr("SAVING ERROR :") << e.what();
    }
}
