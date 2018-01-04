#include "Server.h"

/**
 * @file Server.cpp
 * @brief Server implementation
 */

#include <QDebug>
#include <exception>

#include <wiringPi.h>
#include <unistd.h>

#define DEFAULT_EXTENSION "*.song" /*< Song files extension */
#define DEFAULT_FOLDER "/home/pi/songs/" /*< Files save/load folder*/

#define DEFAULT_IP "192.170.0.17"
#define DEFAULT_SENDER 9989
#define DEFAULT_RECEIVER 9988

#define DEFAULT_THRESHOLD 50 /*< Default threshold value */
#define DEFAULT_MASTER_VOLUME 50 /*< Default master volume value */

#define DEFAULT_VOLUME 50
#define DEFAULT_PAN 0
#define DEFAULT_ACTIVATION false

#define DEFAULT_LED 6 /*< wiringPi green LED's GPIO identifier */
#define DEFAULT_WARNING_LED 5 /*< wiringPi red LED's GPIO identifier */

void Server::ledSetup(){
    wiringPiSetupSys();
    //pinMode(LED_VALUE, OUTPUT);

    QString c = QStringLiteral("gpio mode %1 out").arg(m_options->value("gpio/led").toInt());
    std::system(c.toStdString().c_str());
    c = QStringLiteral("gpio mode %1 out").arg(m_options->value("gpio/warning").toInt());
    std::system(c.toStdString().c_str());
}

void Server::ledOn(int n){
    //digitalWrite(LED_VALUE, HIGH);

    QString c = QStringLiteral("gpio write %1 1").arg(n);
    std::system(c.toStdString().c_str());
}

void Server::ledOn(){
    ledOn(m_options->value("gpio/led").toInt());
}

void Server::ledOff(int n){
    //digitalWrite(n, LOW);

    QString c = QStringLiteral("gpio write %1 0").arg(n);
    std::system(c.toStdString().c_str());
}

void Server::ledOff(){
    ledOff(m_options->value("gpio/led").toInt());
}

void Server::ledBlink(){
    for(int i=0; i<2; i++) {
        ledOff();
        usleep(300);
        ledOn();
    }
}

Server::Server(QSettings* opt):
    m_options(opt){

    initConf(m_options);
    qDebug() << m_options->fileName();

    ledSetup();
    ledOn();

    m_player = new PlayThread(m_options);

    m_options->beginGroup("osc");
    m_receiver = new OscReceiver(m_options->value("receiver").toInt());
    m_sender = new OscSender(m_options->value("ip").toString().toStdString(),
                     m_options->value("sender").toInt());

    m_options->endGroup();

    connect(m_player, &PlayThread::muteChanged,
            m_player, &PlayThread::setMute);

    connect(this, &Server::updateThreshold,
            m_player, &PlayThread::setThreshold);
    connect(this, &Server::resetThreshold,
            m_player, &PlayThread::resetThreshold);

    connect(m_player,	&PlayThread::actualBeatChanged,
            this,		&Server::updateBeat);
    connect(m_player,	&PlayThread::beatCountChanged,
            this,		&Server::updateBeatCount);

    connect(m_player,	&PlayThread::songLoaded,
            this,		&Server::onSongLoaded);
    connect(&m_saveManager,	&SaveManager::updatedTracksList,
            this,		&Server::updateTrackList);

    connect(&m_serialManager,	&SerialManager::boxActivated,
            this,			&Server::switchBox);

    m_serialManager.start();

    //Client events
    m_receiver->addHandler("/box/update_threshold",
                        std::bind(&Server::handle__box_updateThreshold,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/reset_threshold",
                        std::bind(&Server::handle__box_resetThreshold,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/enable",
                        std::bind(&Server::handle__box_enable,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/volume",
                        std::bind(&Server::handle__box_volume,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/pan",
                        std::bind(&Server::handle__box_pan,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/mute",
                        std::bind(&Server::handle__box_mute,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/solo",
                        std::bind(&Server::handle__box_solo,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/master",
                        std::bind(&Server::handle__box_master,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/play",
                        std::bind(&Server::handle__box_play,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/stop",
                        std::bind(&Server::handle__box_stop,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/reset",
                        std::bind(&Server::handle__box_reset,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/refresh_song",
                        std::bind(&Server::handle__box_refreshSong,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/select_song",
                        std::bind(&Server::handle__box_selectSong,
                                  this, std::placeholders::_1));
    m_receiver->addHandler("/box/sync",
                        std::bind(&Server::handle__box_sync,
                                  this, std::placeholders::_1));

    m_receiver->run();
}

Server::~Server() {
    int led = m_options->value("gpio/led").toInt();
    qDebug() << "Stopping server...";
    stop();

    m_player->stop();
    qDebug() << "PlayThread stopping...";
    if(!m_player->wait(5000)) {
        qWarning("PlayThread : Potential deadlock detected !! Terminating...");
        m_player->terminate();
        m_player->wait(5000);
        m_player->exit(1);
    }

    qDebug() << "SerialManager stopping...";
    m_serialManager.stop();
    if(!m_serialManager.wait(5000)) {
        qWarning("SerialManager : Potential deadlock detected !! Terminating...");
        m_serialManager.terminate();
        m_serialManager.wait(5000);
        m_serialManager.exit(1);
    }

    qDebug() << "Deleting pointers...";
    delete m_player;
    delete m_options;

    qDebug() << "Stopping CoreApp...";

    QCoreApplication::exit(0);
    ledOff(led);
}

int Server::getTempo() const {
    return m_tempo;
}

unsigned int Server::getThreshold() const {
    return 99 - (m_player->getThreshold()-100)/4;
}

bool Server::initConf(QSettings *c) {

    QList<struct Settings> default_settings;

    default_settings.append(Settings("default/threshold", DEFAULT_THRESHOLD));
    default_settings.append(Settings("default/master", DEFAULT_MASTER_VOLUME));
    default_settings.append(Settings("default/volume", DEFAULT_VOLUME));
    default_settings.append(Settings("default/pan", DEFAULT_PAN));
    default_settings.append(Settings("default/activation", DEFAULT_ACTIVATION));

    default_settings.append(Settings("files/folder", DEFAULT_FOLDER));
    default_settings.append(Settings("files/extension", DEFAULT_EXTENSION));

    default_settings.append(Settings("gpio/led", DEFAULT_LED));
    default_settings.append(Settings("gpio/warning", DEFAULT_WARNING_LED));

    default_settings.append(Settings("osc/ip", DEFAULT_IP));
    default_settings.append(Settings("osc/sender", DEFAULT_SENDER));
    default_settings.append(Settings("osc/receiver", DEFAULT_RECEIVER));

    QList<struct Settings>::const_iterator it;
    for(it = default_settings.constBegin(); it < default_settings.constEnd(); it++){
        if(!c->contains(it->key))
            c->setValue(it->key, it->value.toString());
    }

    // Debugging
    QStringList lst = c->allKeys();
    for(QStringList::const_iterator k = lst.constBegin(); k<lst.constEnd(); k++)
        qDebug() << *k << " : " << c->value(*k).toString();

    return false;
}

void Server::sendReady(bool isReady) {
    sendMsgReady(isReady);
}

void Server::sendThreshold() {
    sendMsgThreshold(getThreshold());
}

void Server::sendActivatedTracks() {
    sendMsgActivatedTracks(m_player->getActivatedTracks());
}

void Server::sendPlay()
{
    sendMsgPlay(getTempo());
}

void Server::sendTracksCount() {
    sendMsgTracksCount(m_player->getTracksCount());
}

void Server::sendBeatCount(unsigned int i) {
    sendMsgBeatCount(i);
}

void Server::sendSongsList() {
    m_options->beginGroup("files");
    QDir exportFolder(m_options->value("folder").toString());

    exportFolder.setNameFilters(QStringList()<<m_options->value("extension").toString());
    QStringList fileList = exportFolder.entryList();
    QString str = fileList.join("|");
    QByteArray list = str.toLatin1();
    const char *c_list = list.data();

    m_options->endGroup();

    sendMsgSongsList(c_list);
}

void Server::sendSongTitle() {
    QString raw_title = QString::fromStdString(m_song.name);
    QByteArray title = raw_title.toLatin1();
    const char *c_title = title.data();

    sendMsgSongTitle(c_title);
}

void Server::handle__box_updateThreshold(osc::ReceivedMessageArgumentStream args) {
    osc::int32 senseUpdated;
    args >> senseUpdated;
    qDebug() << "received /box/update_threshold" << senseUpdated;

    ledBlink();
    emit updateThreshold(senseUpdated);
}

void Server::handle__box_resetThreshold(osc::ReceivedMessageArgumentStream args) {
    osc::int32 senseUpdated;
    args >> senseUpdated;
    qDebug() << "received /box/reset_threshold" << senseUpdated;

    ledBlink();
    emit resetThreshold();
}

void Server::handle__box_enable(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    args >> box;
    qDebug() << "received /box/enable" << box;

    ledBlink();
    switchBox(box, m_player->getThreshold());
}

void Server::handle__box_volume(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    osc::int32 vol;
    args >> box >> vol;
    qDebug() << "received /box/volume" << vol;

    ledBlink();
    m_player->setVolume(box, vol);
}

void Server::handle__box_pan(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    osc::int32 vol;
    args >> box >> vol;
    qDebug() << "received /box/pan" << box << vol;

    ledBlink();
    m_player->setPan(box, vol);
}

void Server::handle__box_mute(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    bool state;
    args >> box >> state;
    qDebug() << "received /box/mute" << box << state;

    ledBlink();
    m_player->setMute(box, state);
}

void Server::handle__box_solo(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    bool state;
    args >> box >> state;
    qDebug() << "received /box/solo" << box << state;

    ledBlink();
    m_player->solo(box, state);
}

void Server::handle__box_master(osc::ReceivedMessageArgumentStream args) {
    osc::int32 vol;
    args >> vol;
    qDebug() << "received /box/master" << vol;

    ledBlink();
    m_player->setMasterVolume(vol);
}

void Server::handle__box_play(osc::ReceivedMessageArgumentStream args) {
    bool state;
    args >> state;
    qDebug() << "received /box/play" << state;

    ledBlink();
    play();
}

void Server::handle__box_stop(osc::ReceivedMessageArgumentStream args) {
    bool state;
    args >> state;
    qDebug() << "received /box/stop" << state;

    ledBlink();
    stop();
}

void Server::handle__box_reset(osc::ReceivedMessageArgumentStream args) {
    bool state;
    args >> state;
    qDebug() << "received /box/reset" << state;

    ledBlink();

    if(state)
        stop();

    m_player->reset();
}

void Server::handle__box_refreshSong(osc::ReceivedMessageArgumentStream args) {
    bool state;
    args >> state;
    qDebug() << "received /box/refresh_song" << state;

    ledBlink();
    load();
}

void Server::handle__box_selectSong(osc::ReceivedMessageArgumentStream args) {
    const char *receptSong;
    args >> receptSong;
    qDebug() << "received /box/select_song" << receptSong;

    ledBlink();

    QString so = QString(QLatin1String(receptSong));
    if(!so.isEmpty())
        m_selSong = so;

    connect(this, SIGNAL(actionLoad()),
            this, SLOT(load()) );
    emit actionLoad();
}

void Server::handle__box_sync(osc::ReceivedMessageArgumentStream args)
{
    bool state;
    args >> state;
    qDebug() << "received /box/sync" << state;

    ledBlink();

    sendSongsList();
    sendThreshold();

    sendSongTitle();
    sendTracksCount();
    sendActivatedTracks();
}

void Server::reset() {
    stop();

    m_player->reset();
}

void Server::switchBox(unsigned int i, int val) {
    if(val >= m_player->getThreshold()) {
        m_player->switchBox(i);
    }
    sendActivatedTracks();
}

void Server::play() {
    if(!m_loaded)
        if(load())
            return;

    sendPlay();
    m_player->start();
    m_playing = true;
}

void Server::stop() {
    if(m_loaded && m_playing){
        m_player->stop();
        m_previousBeat = 0;
        m_playing = false;

        updateBeat(0);
    }
}

void Server::updateBeat(double t) { // in seconds

    int time = (int)(t * getTempo() / 60.0f) + 1;

    if(time != m_previousBeat && time <= m_beatCount && !m_player->isStopped()) {
        m_previousBeat = time;

        sendBeatCount(time);
        if(time%8 == 0){ //sync
            sendActivatedTracks();
            sendBeatCount(time);
        }
    } else if(m_player->isStopped()) {
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

void Server::sendMsgThreshold(int t) {
    m_sender->send(osc::MessageGenerator()("/box/sensor", t));
    qDebug() << "sent /box/sensor" << t;
}

void Server::sendMsgActivatedTracks(int tracks) {
    m_sender->send(osc::MessageGenerator()("/box/enable_sync", tracks));
    qDebug() << "sent /box/enable_sync" << tracks;
}

void Server::sendMsgBeatCount(int beat) {
    m_sender->send(osc::MessageGenerator()("/box/beat", beat));
    qDebug() << "sent /box/beat" << beat;
}

void Server::sendMsgPlay(int tempo) {
    m_sender->send(osc::MessageGenerator()("/box/play", tempo));
    qDebug() << "sent /box/play" << tempo;
}

void Server::sendMsgSongTitle(const char *title) {
    m_sender->send(osc::MessageGenerator()("/box/title", title));
    qDebug() << "sent /box/title" << title;
}

void Server::sendMsgSongsList(const char *list) {
    m_sender->send(osc::MessageGenerator()("/box/songs_list", list));
    qDebug() << "sent /box/songs_list" << list;
}

void Server::sendMsgTracksCount(int num) {
    m_sender->send(osc::MessageGenerator()("/box/tracks_count", num));
    qDebug() << "sent /box/tracks_count" << num;
}

void Server::sendMsgTracksList(const char *list) {
    m_sender->send(osc::MessageGenerator()("/box/tracks_list", list));
    qDebug() << "sent /box/tracks_list" << list;
}

void Server::sendMsgReady(bool isReady) {
    m_sender->send(osc::MessageGenerator()("/box/ready", isReady));
    qDebug() << "sent /box/ready" << isReady;
}

int Server::load() {
    ledOn(m_options->value("gpio/warning").toInt());
    sendSongsList();
    sendThreshold();

    if (!m_selSong.isEmpty()) {
        try {
            stop();

            QString file = m_options->value("files/folder").toString() + m_selSong;

            if(!file.isEmpty()) {
                m_currentFile = file;
                m_song = m_saveManager.load(file);

                setTempo(m_song.tempo);
                m_player->load(m_song);

                m_loaded = true;
                sendSongTitle();

                sendTracksCount();

                ledOff(m_options->value("gpio/warning").toInt());
                return 0;
            }
            ledOff(m_options->value("gpio/warning").toInt());
            return 1;
        } catch(std::exception& e) {
            qCritical() << tr("LOADING ERROR :") << e.what();
            ledOff(m_options->value("gpio/warning").toInt());
            return 1;
        }
    }//end check selsong

    ledOff(m_options->value("gpio/warning").toInt());
    return 1;
}

void Server::save() {
    try {
        if(m_loaded)
            m_saveManager.save(m_currentFile, this);
    } catch(std::exception& e) {
        qCritical() << tr("SAVING ERROR :") << e.what();
    }
}
