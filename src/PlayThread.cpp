#include "PlayThread.h"

/**
 * @file PlayThread.h
 * @brief Audio player implementation
 */

#include "Track.h"

#include <io/proxies/SfxInputProxy.h>
#include <rtaudio/RtAudio.h>
#include <stream_io/RtAudioOutput.h>
#include <QDebug>

PlayThread::PlayThread(QSettings* c) : QThread(nullptr), m_options(c) {
  setThreshold(m_options->value("default/threshold").toUInt());

  // make sure the master volume is properly initialized
  setMasterVolume(m_options->value("default/master").toUInt());
}

size_t PlayThread::getTracksCount() const {
  return m_tracks.size();
}

Track* PlayThread::getTrack(const size_t track) {
  return (isValidTrack(track) ? m_tracks[track] : nullptr);
}

int PlayThread::getActivatedTracks() const {
  int res = 0;
  for (size_t i = 0; i < m_tracks.size(); i++)
    res += (m_tracks[i]->isActivated() ? pow(2, i) : 0);

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

void PlayThread::setVolume(const size_t track, const unsigned int vol) {
  if (isValidTrack(track))
    m_tracks[track]->setVolume(vol);
}

int PlayThread::volume(unsigned int track) const {
  return isValidTrack(track) ? m_tracks[track]->getVolume() : 50;
}

void PlayThread::setPan(const size_t track, const int pan) {
  if (isValidTrack(track))
    m_tracks[track]->setPan(pan);
}

int PlayThread::pan(const size_t track) const {
  return isValidTrack(track) ? m_tracks[track]->getPan() : 0;
}

void PlayThread::setMute(const size_t track, const bool doMute) {
  if (isValidTrack(track))
    m_tracks[track]->setMute(doMute);
}

Track* PlayThread::track(const size_t track) const {
  return isValidTrack(track) ? m_tracks[track] : nullptr;
}

void PlayThread::timeHandle() {
  if (++m_bufferCount > m_maxBufferCount)
    m_bufferCount = 0;

  emit actualBeatChanged(m_bufferCount * m_conf.bufferSize /
                         double(m_conf.samplingRate));
}

void PlayThread::stop() {
  if (this->isRunning()) {
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
  setMasterVolume(m_options->value("default/master").toInt());

  for (auto& track : m_tracks)
    track->reset();
}

void PlayThread::solo(const size_t track, const bool state) {
  if (isValidTrack(track))
    m_tracks[track]->setActivated(true);

  for (unsigned char i = 0; i < m_tracks.size(); ++i) {
    if (!isValidTrack(i))
      continue;

    Track* currentTrack = m_tracks[i];
    // this will only be true for the track passed as parameter if 'state' is
    // true, in all other cases this will be false.
    const bool currentTrackIsTarget = state && (i != track);
    currentTrack->setSolo(currentTrackIsTarget);
    currentTrack->setMute(currentTrackIsTarget);
  }
}

void PlayThread::switchBox(const size_t track) {
  if (isValidTrack(track))
    m_tracks[track]->setActivated(!m_tracks[track]->isActivated());
}

void PlayThread::setTrackActivated(unsigned int track, bool activated) {
  if (isValidTrack(track)) {
    m_tracks[track]->setActivated(activated);
  }
}

void PlayThread::setThreshold(const unsigned int threshold) {
  m_threshold = (99 - threshold) * 4 + 100;
  m_options->setValue("default/threshold", m_threshold);
}

void PlayThread::resetThreshold() {
  setThreshold(m_options->value("default/threshold").toUInt());
}

void PlayThread::load(const SongData& s) {
  // Reset to 0
  m_bufferCount = 0;
  const size_t track_count = s.tracks.size();
  for (auto& track : m_tracks)
    delete track;
  m_tracks.clear();
  m_manager.reset();

  // Loading
  m_tracks.resize(track_count);
  std::vector<Input_p> chains(track_count);

#pragma omp parallel for
  for (size_t i = 0; i < track_count; i++) {
    auto* t = new Track(s.tracks[i], m_conf, m_options);
    m_tracks[i] = t;

    auto file = new FFMPEGFileInput<double>(m_tracks[i]->getFile(), m_conf);
    m_maxBufferCount = file->v(0).size() / m_conf.bufferSize;
    emit beatCountChanged(file->v(0).size() / double(m_conf.samplingRate));

    chains[i] = Input_p(new SfxInputProxy<double>(
        new StereoAdapter<double>(new LoopInputProxy<double>(file)),
        new Sequence<double>(m_conf, m_tracks[i]->getVolumePtr(),
                             m_tracks[i]->getPanPtr(),
                             m_tracks[i]->getMutePtr())));

    emit songLoaded(i + 1, track_count);
  }

  // Master
  auto input = Input_p(new SfxInputProxy<double>(
      new SummationProxy<double>(new InputMultiplexer<double>(m_conf, chains)),
      m_masterVolume));

  // Manager
  m_manager = std::make_shared<StreamingManager<double>>(
      std::move(input), std::make_shared<RtAudioOutput<double>>(m_conf),
      std::bind(&PlayThread::timeHandle, this), m_conf);
}

bool PlayThread::isValidTrack(size_t track) const {
  const bool isValid = track < getTracksCount();
  if (!isValid)
    qCritical() << tr("ERROR : Inexistent track") << track;

  return isValid;
}
