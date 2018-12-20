#ifndef TRACK_H
#define TRACK_H

/**
 * @file Track.h
 * @brief Tracks manipulation interface
 */

#include "SongData.h"

#include <QDebug>

#include <benchmark/Amplify.h>
#include <benchmark/Mute.h>
#include <benchmark/Pan.h>
#include <QSettings>
#include <QString>

/**
 * @brief The Track class
 *
 * Class to handle the tracks' informations
 */

class Track : public QObject {
  Q_OBJECT
 private:
  QString m_file; /*< Filepath */

  QString m_name;
  bool m_soloState = false;
  bool m_activatedState = false;
  bool m_previousActivatedState = false;

  QSettings* m_options;

  std::shared_ptr<Amplify<double>>
      m_volumePtr;                       /*< Volume in the audio engine */
  std::shared_ptr<Pan<double>> m_panPtr; /*< Pan in the audio engine */
  std::shared_ptr<Mute<double>>
      m_muteState; /*< Mute state in the audio engine */

  double m_volume; /*< "Raw" volume */
  double m_pan;    /*< "Raw" pan */
  bool m_mute = false;

  /**
   * @brief updateAudible this controls if a track can be heard, depending on
   * wether it's muted and enabled
   */
  void updateAudible();

 public:
  Track();
  /**
   * @brief Track constructor
   * @param data Track informations
   * @param conf Default configuration data (volume, pan, mute)
   */
  Track(const TrackData& data, Parameters<double> conf, QSettings* opt);

  QString getName() const;
  QString getFile() const;
  double getVolume() const;
  double getPan() const;
  bool isActivated() const;
  bool wasPreviouslyActivated() const;
  bool isSolo() const;
  bool isMuted() const { return m_mute; }

  std::shared_ptr<Amplify<double>> getVolumePtr() const;
  std::shared_ptr<Pan<double>> getPanPtr() const;
  std::shared_ptr<Mute<double>> getMutePtr() const;

  void setMute(bool mute);

  /**
   * @brief Volume setter
   * @param vol New volume value
   *
   * Update the value and the pointer
   */
  void setVolume(const double vol);
  /**
   * @brief Pan setter
   * @param pan New pan value
   *
   * Update the value and the pointer
   */
  void setPan(const double pan);

  /**
   * @brief Activate/Deactivate a track
   * @param state New activation state (true = activated, false else)
   */
  void setActivated(const bool state);

  /**
   * @brief Change the track's solo mode status
   * @param state New solo mode status (true = solo, false else)
   */
  void setSolo(const bool state);

  /**
   * @brief Reset the track's settings to their default values
   */
  void reset();
};

#endif  // TRACK_H
