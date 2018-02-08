#include "SaveManager.h"

/**
 * @file SaveManager.cpp
 * @brief File saving implementation
 */

#include <stdexcept>

#include "Server.h"

std::unique_ptr<QTemporaryDir> SaveManager::tempdir = nullptr;

SongData SaveManager::load(const QString& loadpath) {
  //// Temp. dir creation ////
  tempdir.reset(new QTemporaryDir);

  //// Archive extraction to tempdir ////
  KZip archive(loadpath);

  if (!archive.open(QIODevice::ReadOnly))
    throw std::runtime_error("Invalid file");

  const KArchiveDirectory* root = archive.directory();

  root->copyTo(tempdir->path(), true);

  archive.close();

  //// Data reading
  // Looking for .ini :
  QStringList nameFilter("*.ini");
  QDir directory(tempdir->path());
  QString iniFile =
      tempdir->path() + "/" + directory.entryList(nameFilter).first();

  //// Loading
  QSettings settings(iniFile, QSettings::IniFormat);
  SongData sd;

  int count = settings.value("General/trackCount").toInt();
  sd.tempo = settings.value("General/tempo").toInt();
  sd.name = settings.value("General/songName").toString();
  sd.sigNumerator = settings.value("General/sigNumerator").toInt();
  sd.sigDenominator = settings.value("General/sigDenominator").toInt();

  for (int i = 0; i < count; ++i) {
    sd.tracks.emplace_back(
        settings.value(QString("Track%1/name").arg(i)).toString(),
        (tempdir->path() + "/" +
         settings.value(QString("Track%1/filename").arg(i)).toString()),
        settings.value(QString("Track%1/volume").arg(i)).toInt(),
        settings.value(QString("Track%1/pan").arg(i)).toInt());
  }

  return sd;
}
