#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include "SongData.h"
//#include "ChannelWidget.h"

#include <QFileInfo>
#include <QObject>
#include <QSettings>
#include <QTemporaryDir>
#include <QVector>
#include <memory>
#include <KF5/KArchive/KZip>

/**
 * @brief The SaveManager class
 *
 * Ouverture des fichiers de sauvegarde
 *
 */
class Server;
class SaveManager : public QObject {
    Q_OBJECT
public:
    explicit SaveManager(QObject *parent = 0);

    // Dossier temporaire ou sont extraits les données contenues dans les fichiers .song
    std::shared_ptr<QTemporaryDir> tempdir {};

    // Charge un fichier
    SongData load(const QString name);

    // Utilisé pour sauvegarder les paramètres de vol / pan..
    void save(const QString name, Server* manager);

signals:
    void send_liste_name(const char *list);

};

#endif // SAVEMANAGER_H
