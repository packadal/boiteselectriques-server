#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include "SongData.h"

/**
 * @file SaveManager.h
 * @brief File saving interface
 */

#include <QFileInfo>
#include <QObject>
#include <QSettings>
#include <QTemporaryDir>
#include <QVector>
#include <memory>
#include <KF5/KArchive/KZip>

class Server;

/**
 * @brief The SaveManager class
 *
 * File saving handling
 *
 */
class SaveManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief SaveManager constructor
     * @param parent Parent object (facultative)
     */
    explicit SaveManager(QObject *parent = 0);

    std::shared_ptr<QTemporaryDir> tempdir {}; /*< Temp. directory where .song files' data are extracted */

    /**
     * @brief Load a file
     * @param name Filename
     * @return Corresponding song data
     */
    SongData load(const QString name);

    /**
     * @brief Save the parameters (volume, pan, ...)
     * @param name Filename
     * @param manager Audio server manager (pointer)
     */
    void save(const QString name, Server* manager);

signals:
    /**
     * @brief Notify of a new tracks list
     * @param list Tracks list
     */
    void updatedTracksList(const char *list);

};

#endif // SAVEMANAGER_H
