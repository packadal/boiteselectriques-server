#include "Track.h"

/**
 * @file Track.cpp
 * @brief Tracks manipulation implementation
 */

void Track::updateAudible() {
  const bool audible = !m_mute && m_activatedState;
  if (audible)
    m_muteState->unmute();
  else
    m_muteState->mute();
}

Track::Track() : m_file(""), m_name(""), m_options(nullptr) {}

Track::Track(const TrackData& data, Parameters<double> conf, QSettings* opt)
    : m_file(data.file), m_name(data.name), m_options(opt) {
  m_volumePtr = std::make_shared<Amplify<double>>(conf);
  m_panPtr = std::make_shared<Pan<double>>(conf);
  m_muteState = std::make_shared<Mute<double>>(conf);
  m_muteState->mute();

  setVolume(data.volume);
  setPan(data.pan);
}

QString Track::getName() const {
  return m_name;
}

QString Track::getFile() const {
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

void Track::setMute(bool mute) {
  m_mute = mute;
  updateAudible();
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

void Track::setActivated(const bool state) {
  m_activatedState = state;
  updateAudible();
}

void Track::setSolo(const bool state) {
  m_soloState = state;
}

void Track::reset() {
  m_options->beginGroup("default");

  setVolume(50);
  setPan(0);
  setActivated(false);
  setMute(false);
  setSolo(false);

  m_options->endGroup();
}
