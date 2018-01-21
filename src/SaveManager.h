#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include "SongData.h"

/**
 * @file SaveManager.h
 * @brief File saving interface
 */

#include <QFileInfo>
#include <QIODevice>
#include <QObject>
#include <QSettings>
#include <QTemporaryDir>
#include <QVector>
#include <memory>

#include <kzip.h>

class Server;

/**
 * @brief The SaveManager class
 *
 * File saving handling
 *
 */
class SaveManager : public QObject {
 public:
  /**
   * @brief Load a file
   * @param name Filename
   * @return Corresponding song data
   */
  static SongData load(const QString& name);

 private:
  static std::unique_ptr<QTemporaryDir> tempdir;
};

#endif  // SAVEMANAGER_H
