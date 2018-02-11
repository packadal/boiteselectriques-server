#include "PlayThread.h"

/**
 * @file PlayThread.h
 * @brief Audio player implementation
 */

#include "Track.h"

#include <io/proxies/SfxInputProxy.h>
#ifdef __arm__
#include <RtAudio.h>
#else
#include <rtaudio/RtAudio.h>
#endif
#include <stream_io/RtAudioOutput.h>
#include <QDebug>

PlayThread::PlayThread(QSettings* c) : m_options(c) {
  resetThreshold();

  // make sure the master volume is properly initialized
  setMasterVolume(DEFAULT_MASTER_VOLUME);
}

size_t PlayThread::getTracksCount() const {
  return m_tracks.size();
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

void PlayThread::setMasterVolume(const unsigned int vol) {
  m_masterVolumeValue = vol;
  m_masterVolume->setGain(m_masterVolumeValue / 10.0);
}

unsigned int PlayThread::masterVolume() const {
  return m_masterVolumeValue;
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
  m_manager->stop();
  m_bufferCount = 0;
  m_isPlaying = false;

  emit playChanged();
}

void PlayThread::playSong() {
  m_manager->input()->reset();
  m_bufferCount = 0;
  m_isPlaying = true;

  // emit before starting the execution as it prevents anything else from
  // running on this thread
  emit playChanged();

  m_manager->execute();
}

bool PlayThread::isPlaying() const {
  return m_isPlaying;
}

void PlayThread::reset() {
  setMasterVolume(DEFAULT_MASTER_VOLUME);

  for (auto& track : m_tracks)
    track->reset();

  m_manager->input()->reset();
  m_bufferCount = 0;
  m_isPlaying = false;
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

void PlayThread::setThreshold(int threshold) {
  m_threshold = threshold;
  m_options->setValue("default/threshold", m_threshold);
}

void PlayThread::resetThreshold() {
  setThreshold(m_options->value("default/threshold").toInt());
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

    auto file = new FFMPEGFileInput<double>(
        m_tracks[i]->getFile().toStdString(), m_conf);
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
