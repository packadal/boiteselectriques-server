#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

/**
 * @file PlayThread.h
 * @brief Audio player interface
 */

#include "SongData.h"
#include "Track.h"

#include <benchmark/Amplify.h>
#include <benchmark/Mute.h>
#include <benchmark/Pan.h>
#include <benchmark/Sequence.h>
#include <io/inputs/FFMPEGFileInput.h>
#include <io/proxies/InputMultiplexer2.h>
#include <io/proxies/LoopInputProxy.h>
#include <io/proxies/StereoAdapter.h>
#include <io/proxies/SummationProxy.h>
#include <manager/StreamingManager.h>
#include <unistd.h>
#include <QSettings>
#include <QThread>

template <typename T>
class StreamingManager;

/**
 * @brief The PlayThread class
 *
 * Liaison with the audio engine
 */
class PlayThread : public QObject {
  Q_OBJECT
 public:
  explicit PlayThread(QSettings*);

  /**
   * @brief Give the number of the song's tracks
   * @return Tracks count of the current song
   */
  size_t getTracksCount() const;

  /**
   * @brief Give the number tracks currently active
   * @return Count of activated tracks
   */
  int getActivatedTracks() const;
  /**
   * @brief Threshold getter
   * @return Raw threshold value
   */
  int getThreshold() const;

  /**
   * @brief Give the playing status
   * @return true if playing, false otherwise
   */
  bool isPlaying() const;

 signals:

  void playChanged();

  /**
   * @brief Notify of the modification of the actual beat value
   * @param time Actual time (seconds)
   */
  void actualBeatChanged(double time);
  /**
   * @brief Notify of the total beat count modification
   * @param time Actual time (seconds)
   *
   * For example, called when a new song is loaded
   */
  void beatCountChanged(double time);
  /**
   * @brief Notify of the song loading end
   * @param on Next track number
   * @param max Max track number
   */
  void songLoaded(int, int);

 public slots:

  /**
   * @brief Stop the playing
   */
  void stop();

  /**
   * @brief Master volume setter
   * @param vol New volume (between 0 and 100)
   */
  void setMasterVolume(const unsigned int vol);
  /**
   * @brief masterVolume getter
   * @return the master volume
   */
  unsigned int masterVolume() const;
  /**
   * @brief Track volume setter
   * @param track Related track number
   * @param vol New volume (between 0 and 100)
   */
  void setVolume(const size_t track, const unsigned int vol);
  /**
   * @brief volume getter.
   * @param track for which to return the volume.
   * @return the volume if the track is valid, or a default value of 50
   * otherwise.
   */
  int volume(unsigned int track) const;
  /**
   * @brief Track pan setter
   * @param track Related track number
   * @param pan New pan value (between -100 and 100)
   */
  void setPan(const size_t track, const int pan);
  /**
   * @brief pan setter
   * @param track for which to return the pan.
   * @return the pan (between -100 and 100) if the track is valid, or a default
   * value of 0 otherwise.
   */
  int pan(const size_t track) const;
  /**
   * @brief (Un)Mute a track
   * @param track Related track number
   * @param doMute New mute state
   *
   * Mute a given track with doMute = true, unmute it else
   */
  void setMute(const size_t track, const bool doMute);

  /**
   * @brief Access a track from its number
   * @param track Track number
   * @return Pointer to the asked Track
   */
  Track* track(const size_t track) const;

  /**
   * @brief Change the solo state of a track
   * @param track Related track number
   * @param state New solo state
   *
   * If state is true, this sets track 'track' to be the only playing one by
   * * activating it
   * * muting all other tracks
   * * disabling solo on all other tracks
   *
   * If state is false, this
   * * removes the solo on all tracks
   * * removes the muting on all tracks
   */
  void solo(const size_t track, const bool state);

  /**
   * @brief Reset the song to its default values
   */
  void reset();

  /**
   * @brief Switch a box activation status
   * @param track Related track number
   */
  void switchBox(const size_t track);

  /**
   * @brief setTrackActivated enables or disables a track
   * @param track the track to enabled/disable
   * @param activated wether to enable or disable the track
   */
  void setTrackActivated(unsigned int track, bool activated);

  /**
   * @brief Threshold setter
   * @param "New" (not raw) threshold value (0-100)
   */
  void setThreshold(int threshold);

  /**
   * @brief Reset threshold to its default value
   */
  void resetThreshold();

  /**
   * @brief Regularly called to update the beat value
   */
  void timeHandle();

  /**
   * @brief Play the song
   */
  void playSong();

  /**
   * @brief Load a song in the engine
   * @param s Song's informations
   */
  void load(const SongData& s);

  /**
   * @brief Check if a given track exists
   * @param track Track number
   * @return true if the track exists, false else
   */
  bool isValidTrack(size_t track) const;

 private:
  QSettings* m_options;
  Parameters<double> m_conf; /*< Configuration data */
  std::shared_ptr<Amplify<double>> m_masterVolume{new Amplify<double>(m_conf)};
  std::shared_ptr<StreamingManager<double>> m_manager;

  std::vector<Track*> m_tracks; /*< List of the current song's tracks */

  int m_bufferCount{};    /*< Buffer in which we are s*/
  int m_maxBufferCount{}; /*< Total buffer count in a loop */

  bool m_isPlaying{false};
  int m_threshold;
  unsigned int m_masterVolumeValue = DEFAULT_MASTER_VOLUME;

  static const unsigned int DEFAULT_MASTER_VOLUME = 50;
};

#endif  // PLAYTHREAD_H
