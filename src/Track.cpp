#include "Track.h"

/**
 * @file Track.cpp
 * @brief Tracks manipulation implementation
 */

Track::Track()
    : m_id(0),
      m_file(""),
      m_name(""),
      m_soloState(false),
      m_activatedState(false),
      m_options(nullptr) {}

Track::Track(const TrackData& data,
             Parameters<double> conf,
             QSettings* opt,
             int id)
    : m_id(id),
      m_file(data.file),
      m_name(data.name),
      m_soloState(false),
      m_activatedState(false),
      m_options(opt) {
  m_volumePtr = std::make_shared<Amplify<double>>(conf);
  m_panPtr = std::make_shared<Pan<double>>(conf);
  m_muteState = std::make_shared<Mute<double>>(conf);

  setVolume(data.volume);
  setPan(data.pan);
  setMute(true);
}

std::string Track::getName() const {
  return m_name;
}

std::string Track::getFile() const {
  return m_file;
}

double Track::getVolume() const {
  return m_volume;
}

double Track::getPan() const {
  return m_pan;
}

std::shared_ptr<Amplify<double>> Track::getVolumePtr() const {
  return m_volumePtr;
}

std::shared_ptr<Pan<double>> Track::getPanPtr() const {
  return m_panPtr;
}

std::shared_ptr<Mute<double>> Track::getMutePtr() const {
  return m_muteState;
}

bool Track::isActivated() const {
  return m_activatedState;
}

bool Track::isSolo() const {
  return m_soloState;
}

void Track::setVolume(const double vol) {
  m_volume = vol;
  m_volumePtr->setGain(m_volume / 100.0);
}

void Track::setPan(const double pan) {
  m_pan = pan;
  m_panPtr->setPan(m_pan / 100.0);
}

void Track::setMute(const bool state) {
  state ? m_muteState->mute() : m_muteState->unmute();
}

void Track::setActivated(const bool state) {
  m_activatedState = state;
  notifyEnabled(state);
}

void Track::setSolo(const bool state) {
  m_soloState = state;
}

void Track::notifyEnabled(bool enabled) {
  if (!m_soloState)
    emit onActivationSwitch(enabled, m_id);
}

void Track::reset() {
  m_options->beginGroup("default");

  setVolume(m_options->value("volume").toInt());
  setPan(m_options->value("pan").toInt());
  setActivated(m_options->value("activation").toBool());

  m_options->endGroup();
}
