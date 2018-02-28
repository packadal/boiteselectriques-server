#include "Server.h"

/**
 * @file Server.cpp
 * @brief Server implementation
 */

#include <QDebug>
#include <exception>

#include <unistd.h>
#include <wiringPi.h>

#define DEFAULT_EXTENSION "*.song"       /*< Song files extension */
#define DEFAULT_FOLDER "/home/pi/songs/" /*< Files save/load folder*/

#define DEFAULT_IP "192.170.0.17"
#define DEFAULT_SENDER 9989
#define DEFAULT_RECEIVER 9988

#define DEFAULT_THRESHOLD 50 /*< Default threshold value */

#define DEFAULT_VOLUME 50
#define DEFAULT_PAN 0
#define DEFAULT_ACTIVATION false

#define DEFAULT_LED 6         /*< wiringPi green LED's GPIO identifier */
#define DEFAULT_WARNING_LED 5 /*< wiringPi red LED's GPIO identifier */

void Server::ledSetup() {
  wiringPiSetupSys();
  // pinMode(LED_VALUE, OUTPUT);

  QString c = QStringLiteral("gpio mode %1 out")
                  .arg(m_options->value("gpio/led").toInt());
  std::system(c.toStdString().c_str());
  c = QStringLiteral("gpio mode %1 out")
          .arg(m_options->value("gpio/warning").toInt());
  std::system(c.toStdString().c_str());
}

void Server::ledOn(int n) {
  // digitalWrite(LED_VALUE, HIGH);

  QString c = QStringLiteral("gpio write %1 1").arg(n);
  std::system(c.toStdString().c_str());
}

void Server::ledOn() {
  ledOn(m_options->value("gpio/led").toInt());
}

void Server::ledOff(int n) {
  // digitalWrite(n, LOW);

  QString c = QStringLiteral("gpio write %1 0").arg(n);
  std::system(c.toStdString().c_str());
}

void Server::ledOff() {
  ledOff(m_options->value("gpio/led").toInt());
}

void Server::ledBlink() {
#ifdef __arm__
  for (int i = 0; i < 2; i++) {
    ledOff();
    usleep(300);
    ledOn();
  }
#endif
}

Server::Server(QSettings* opt) : m_options(opt) {
  initConf(m_options);
  qDebug() << m_options->fileName();

#ifdef __arm__
  ledSetup();
#endif
  ledOn();

  m_player = new PlayThread(m_options);
  m_player->moveToThread(&m_playThread);
  connect(m_player, &PlayThread::actualBeatChanged, this, &Server::updateBeat,
          Qt::DirectConnection);
  connect(m_player, &PlayThread::beatCountChanged, this,
          &Server::updateBeatCount);
  connect(m_player, &PlayThread::songLoaded, this, &Server::onSongLoaded);
  connect(m_player, &PlayThread::playChanged, this, &Server::sendPlay);

  connect(this, &Server::updateThreshold, m_player, &PlayThread::setThreshold);
  connect(this, &Server::resetThreshold, m_player, &PlayThread::resetThreshold);
  connect(this, &Server::playSong, m_player, &PlayThread::playSong);

  connect(&m_serialManager, &SerialManager::boxActivated, this,
          &Server::switchBox);

  m_playThread.start();

  m_receiver = new OscReceiver(m_options->value("osc/receiver").toUInt());
  m_sender = new OscSender(m_options->value("osc/ip").toString().toStdString(),
                           m_options->value("osc/sender").toInt());

// this allows to build and run on a PC for easier development
#ifdef __arm__
  m_serialManager.start();
#endif
  // Client events
  m_receiver->addHandler("/box/update_threshold",
                         std::bind(&Server::handle__box_updateThreshold, this,
                                   std::placeholders::_1));
  m_receiver->addHandler("/box/reset_threshold",
                         std::bind(&Server::handle__box_resetThreshold, this,
                                   std::placeholders::_1));
  m_receiver->addHandler("/box/enable", std::bind(&Server::handle__box_enable,
                                                  this, std::placeholders::_1));
  m_receiver->addHandler("/box/volume", std::bind(&Server::handle__box_volume,
                                                  this, std::placeholders::_1));
  m_receiver->addHandler("/box/pan", std::bind(&Server::handle__box_pan, this,
                                               std::placeholders::_1));
  m_receiver->addHandler("/box/mute", std::bind(&Server::handle__box_mute, this,
                                                std::placeholders::_1));
  m_receiver->addHandler("/box/solo", std::bind(&Server::handle__box_solo, this,
                                                std::placeholders::_1));
  m_receiver->addHandler("/box/master", std::bind(&Server::handle__box_master,
                                                  this, std::placeholders::_1));
  m_receiver->addHandler("/box/play", std::bind(&Server::handle__box_play, this,
                                                std::placeholders::_1));
  m_receiver->addHandler("/box/stop", std::bind(&Server::handle__box_stop, this,
                                                std::placeholders::_1));
  m_receiver->addHandler("/box/reset", std::bind(&Server::handle__box_reset,
                                                 this, std::placeholders::_1));
  m_receiver->addHandler(
      "/box/refresh_song",
      std::bind(&Server::handle__box_refreshSong, this, std::placeholders::_1));
  m_receiver->addHandler(
      "/box/select_song",
      std::bind(&Server::handle__box_selectSong, this, std::placeholders::_1));
  m_receiver->addHandler("/box/sync", std::bind(&Server::handle__box_sync, this,
                                                std::placeholders::_1));

  m_receiver->addHandler(
      "/box/delete_song",
      std::bind(&Server::handle__box_deleteSong, this, std::placeholders::_1));

  m_receiver->run();
}

Server::~Server() {
  int led = m_options->value("gpio/led").toInt();
  qDebug() << "Stopping server...";
  stop();

  m_player->stop();
  qDebug() << "PlayThread stopping...";
  if (!m_playThread.wait(5000)) {
    qWarning("PlayThread : Potential deadlock detected !! Terminating...");
    m_playThread.terminate();
    m_playThread.wait(5000);
    m_playThread.exit(1);
  }

  qDebug() << "SerialManager stopping...";
  m_serialManager.stop();
  if (!m_serialManager.wait(5000)) {
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

int Server::getThreshold() const {
  return m_player->getThreshold();
}

bool Server::initConf(QSettings* c) {
  QList<struct Settings> default_settings;

  default_settings.append(Settings("default/threshold", DEFAULT_THRESHOLD));
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
  for (it = default_settings.constBegin(); it < default_settings.constEnd();
       it++) {
    if (!c->contains(it->key))
      c->setValue(it->key, it->value.toString());
  }

  // Debugging
  QStringList lst = c->allKeys();
  for (QStringList::const_iterator k = lst.constBegin(); k < lst.constEnd();
       k++)
    qDebug() << *k << " : " << c->value(*k).toString();

  return false;
}

void Server::sendReady(bool isReady) {
  m_sender->send(osc::MessageGenerator()("/box/ready", isReady));
  qDebug() << "sent /box/ready" << isReady;
}

void Server::sendThreshold() {
  m_sender->send(osc::MessageGenerator()("/box/sensor", getThreshold()));
  qDebug() << "sent /box/sensor" << getThreshold();
}

void Server::sendActivatedTracks() {
  m_sender->send(osc::MessageGenerator()("/box/enable_sync",
                                         m_player->getActivatedTracks()));
  qDebug() << "sent /box/enable_sync" << m_player->getActivatedTracks();
}

void Server::sendMasterVolume() {
  m_sender->send(osc::MessageGenerator()(
      "/box/master", static_cast<int>(m_player->masterVolume())));
  qDebug() << "sent /box/master" << m_player->masterVolume();
}

void Server::sendPlay() {
  m_playSignalSent = false;
  m_sender->send(osc::MessageGenerator()("/box/play", m_player->isPlaying()));
  qDebug() << "sent /box/play" << m_player->isPlaying();
}

void Server::sendMute() {
  int muteStatus = 0;
  for (unsigned char i = 0; i < m_player->getTracksCount(); ++i) {
    muteStatus += (m_player->isValidTrack(i) && m_player->track(i)->isMuted())
                      ? (1 << i)
                      : 0;
  }
  m_sender->send(osc::MessageGenerator()("/box/mute", muteStatus));
}

void Server::sendTracksList() {
  QStringList tracks;
  for (unsigned char i = 0; i < m_player->getTracksCount(); ++i) {
    if (m_player->isValidTrack(i)) {
      tracks << m_player->track(i)->getName();
    }
  }
  QByteArray trackList = tracks.join('|').toUtf8();
  const char* trackListChar = trackList.data();
  m_sender->send(osc::MessageGenerator()("/box/tracks_list", trackListChar));
  qDebug() << "sent /box/tracks_list" << trackListChar;
}

void Server::sendBeatCount() {
  m_sender->send(osc::MessageGenerator()("/box/beat_count", m_beatCount));
}

void Server::sendBeat(int beat) {
  m_sender->send(osc::MessageGenerator()("/box/beat", beat));
  //  qDebug() << "sent /box/beat" << beat;
}

void Server::sendSongsList() {
  QDir exportFolder(m_options->value("files/folder").toString());

  exportFolder.setNameFilters(
      QStringList() << m_options->value("files/extension").toString());
  QStringList fileList = exportFolder.entryList();
  QByteArray list = fileList.join("|").toUtf8();
  const char* c_list = list.data();

  m_sender->send(osc::MessageGenerator()("/box/songs_list", c_list));
  qDebug() << "sent /box/songs_list" << c_list;
}

void Server::sendSongTitle() {
  QByteArray title = m_song.name.toUtf8();
  const char* c_title = title.data();

  m_sender->send(osc::MessageGenerator()("/box/title", c_title));
  qDebug() << "sent /box/title" << c_title;
}

void Server::handle__box_updateThreshold(
    osc::ReceivedMessageArgumentStream args) {
  osc::int32 senseUpdated;
  args >> senseUpdated;
  qDebug() << "received /box/update_threshold" << senseUpdated;

  ledBlink();
  emit updateThreshold(senseUpdated);
  sendThreshold();
}

void Server::handle__box_resetThreshold(
    osc::ReceivedMessageArgumentStream args) {
  osc::int32 senseUpdated;
  args >> senseUpdated;
  qDebug() << "received /box/reset_threshold" << senseUpdated;

  ledBlink();
  emit resetThreshold();
}

void Server::handle__box_enable(osc::ReceivedMessageArgumentStream args) {
  osc::int32 box;
  args >> box;
  bool activated;
  args >> activated;
  qDebug() << "received /box/enable" << box;

  ledBlink();
  m_player->setTrackActivated(box, activated);
  sendActivatedTracks();
}

void Server::handle__box_volume(osc::ReceivedMessageArgumentStream args) {
  osc::int32 box;
  osc::int32 vol;
  args >> box >> vol;
  qDebug() << "received /box/volume" << vol;

  ledBlink();
  m_player->setVolume(box, vol);

  sendTrackVolume(box, m_player->volume(box));
}

void Server::handle__box_pan(osc::ReceivedMessageArgumentStream args) {
  osc::int32 box;
  osc::int32 vol;
  args >> box >> vol;
  qDebug() << "received /box/pan" << box << vol;

  ledBlink();
  m_player->setPan(box, vol);

  sendTrackPan(box, m_player->pan(box));
}

void Server::handle__box_mute(osc::ReceivedMessageArgumentStream args) {
  osc::int32 box;
  bool state;
  args >> box >> state;
  qDebug() << "received /box/mute" << box << state;

  ledBlink();
  m_player->setMute(box, state);

  sendMute();
}

void Server::sendSolo() {
  int soloStatus = 0;
  for (unsigned char i = 0; i < m_player->getTracksCount(); ++i) {
    soloStatus += (m_player->isValidTrack(i) && m_player->track(i)->isSolo())
                      ? (1 << i)
                      : 0;
  }
  m_sender->send(osc::MessageGenerator()("/box/solo", soloStatus));
}

void Server::handle__box_solo(osc::ReceivedMessageArgumentStream args) {
  osc::int32 box;
  bool state;
  args >> box >> state;
  qDebug() << "received /box/solo" << box << state;

  ledBlink();
  m_player->solo(box, state);
  sendSolo();

  sendMute();
  sendActivatedTracks();
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

  if (state)
    stop();

  m_player->reset();

  updateTrackStatus();
  sendActivatedTracks();
  sendMute();
  sendSolo();
}

void Server::handle__box_refreshSong(osc::ReceivedMessageArgumentStream args) {
  bool state;
  args >> state;
  qDebug() << "received /box/refresh_song" << state;

  ledBlink();
  load();
}

void Server::handle__box_selectSong(osc::ReceivedMessageArgumentStream args) {
  const char* receptSong;
  args >> receptSong;
  qDebug() << "received /box/select_song" << receptSong;

  ledBlink();

  QString so = QString(receptSong);
  if (!so.isEmpty())
    m_selSong = so;

  load();
}

void Server::handle__box_sync(osc::ReceivedMessageArgumentStream args) {
  bool state;
  args >> state;
  qDebug() << "received /box/sync" << state;

  ledBlink();

  sendSongsList();
  sendThreshold();
  sendBeatCount();
  sendBeat(m_previousBeat);
  sendSongTitle();
  sendTracksList();
  sendMasterVolume();
  updateTrackStatus();

  sendPlay();

  sendReady(m_player->getTracksCount() > 0);
}

void Server::updateTrackStatus() {
  sendMute();
  sendSolo();
  sendActivatedTracks();
  for (unsigned char i = 0; i < m_player->getTracksCount(); ++i) {
    sendTrackVolume(i, m_player->volume(i));
    sendTrackPan(i, m_player->pan(i));
  }
}

void Server::reset() {
  stop();

  m_player->reset();

  updateTrackStatus();
}

void Server::switchBox(unsigned int i, int val) {
  if (val >= m_player->getThreshold()) {
    m_player->switchBox(i);
  }
  sendActivatedTracks();
}

void Server::play() {
  if (!m_loaded) {
    load();
    return;
  }

  if (!m_playSignalSent) {
    m_playSignalSent = true;
    emit playSong();
  }
}

void Server::stop() {
  if (m_loaded && m_player->isPlaying()) {
    m_player->stop();
    m_previousBeat = 0;

    updateBeat(0);
  }
}

void Server::updateBeat(double t) {  // in seconds

  int time = (int)(t * m_tempo / 60.0f) + 1;
  time = time % 33;

  if (time != m_previousBeat && time <= m_beatCount && m_player->isPlaying()) {
    m_previousBeat = time;

    sendBeat(time);
  } else if (!m_player->isPlaying()) {
    m_previousBeat = 0;
    sendBeat(0);
  }
}

void Server::updateBeatCount(double t) {  // in seconds
  // Here we calculate the number of beats in the loop
  // Formula : seconds * tempo/60 = nb. beats in loop.

  m_beatCount = t * m_tempo / 60;
  m_previousBeat = -1;

  sendBeatCount();
}

void Server::onSongLoaded() {
  sendTracksList();
  updateTrackStatus();
  sendReady(true);
}

void Server::sendTrackVolume(int track, int volume) {
  m_sender->send(osc::MessageGenerator()("/box/volume", track, volume));
}

void Server::sendTrackPan(int track, int pan) {
  m_sender->send(osc::MessageGenerator()("/box/pan", track, pan));
}

void Server::handle__box_deleteSong(osc::ReceivedMessageArgumentStream args) {
  const char* receptSong;
  args >> receptSong;
  qDebug() << "received /box/delete_song" << receptSong;

  QFile::remove(m_options->value("files/folder").toString() + receptSong);
  sendSongsList();
}

bool Server::load() {
  ledOn(m_options->value("gpio/warning").toInt());

  if (!m_selSong.isEmpty()) {
    try {
      stop();

      QString file = m_options->value("files/folder").toString() + m_selSong;

      if (!file.isEmpty()) {
        m_currentFile = file;
        m_song = m_saveManager.load(file);

        m_tempo = m_song.tempo;

        m_player->load(m_song);

        m_loaded = true;
        sendSongTitle();

        ledOff(m_options->value("gpio/warning").toInt());
        return false;
      }
      ledOff(m_options->value("gpio/warning").toInt());
      return true;
    } catch (std::exception& e) {
      qCritical() << tr("LOADING ERROR :") << e.what();
      ledOff(m_options->value("gpio/warning").toInt());
      return true;
    }
  }  // end check selsong

  ledOff(m_options->value("gpio/warning").toInt());
  return true;
}
