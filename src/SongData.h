#ifndef SONGDATA_H
#define SONGDATA_H

/**
 * @file SongData.h
 * @brief Song informations representation
 */

#include <QImage>
#include <QString>
#include <vector>

/**
 * @brief Informations of a track
 */
struct TrackData {
  TrackData() = default;
  TrackData(TrackData&&) = default;
  TrackData(const TrackData&) = delete;

  TrackData(const QString n,
            const QString f,
            const QImage image,
            const double v,
            const double p)
      : name(n), file(f), image(image), volume(v), pan(p) {}

  TrackData& operator=(TrackData&&) = default;
  TrackData& operator=(const TrackData&) = delete;
  const QString name{};
  const QString file{}; /*< Track's file name */
  const QImage image{};
  const double volume{};
  const double pan{};
};

/**
 * @brief Informations of the whole song
 */
struct SongData {
  SongData() = default;
  SongData(SongData&&) = default;
  SongData& operator=(SongData&&) = default;

  std::vector<TrackData> tracks{}; /*< Song's list of tracks */
  int tempo{};
  QString name{};
  int sigNumerator{};
  int sigDenominator{};
};

#endif  // SONGDATA_H
