#include "SaveManager.h"

/**
 * @file SaveManager.cpp
 * @brief File saving implementation
 */

#include <stdexcept>

#include "Server.h"

SaveManager::SaveManager(QObject *parent) :
    QObject(parent) {
}

SongData SaveManager::load(const QString loadpath) {
    //// Temp. dir creation ////
    tempdir.reset(new QTemporaryDir);

    //// Archive extraction to tempdir ////
    KZip archive(loadpath);

    if (!archive.open(QIODevice::ReadOnly))
        throw std::runtime_error("Invalid file");

    const KArchiveDirectory *root = archive.directory();

    root->copyTo(tempdir->path(), true);

    archive.close();

    //// Data reading
    // Looking for .ini :
    QStringList nameFilter("*.ini");
    QDir directory(tempdir->path());
    QString iniFile = tempdir->path() + "/" + directory.entryList(nameFilter).first();

    //// Loading
    QSettings settings(iniFile, QSettings::IniFormat);
    SongData sd;

    int count = settings.value("General/trackCount").toInt();
    sd.tempo = settings.value("General/tempo").toInt();
    sd.name = settings.value("General/songName").toString().toStdString();
    sd.sigNumerator = settings.value("General/sigNumerator").toInt();
    sd.sigDenominator = settings.value("General/sigDenominator").toInt();

    QString temp;
    int t = 0;
    bool end = false;

    for(int i = 0; i < count; ++ i) {
        sd.tracks.emplace_back(settings.value(QString("Track%1/name").arg(i)).toString().toStdString(),
                               (tempdir->path() + "/" + settings.value(QString("Track%1/filename").arg(i)).toString()).toStdString(),
                               settings.value(QString("Track%1/volume").arg(i)).toInt(),
                               settings.value(QString("Track%1/pan").arg(i)).toInt());

        QString track_name = settings.value(QString("Track%1/name").arg(i)).toString();
        temp = temp + track_name;
        t++;

        if (t != count)
            temp = temp + "|";

        if (i == count-1)
            end=true;
    }

    if(end) {
        QByteArray TrackName = temp.toUtf8();
        emit updatedTracksList(TrackName.data());
    }

    return sd;
}

void SaveManager::save(const QString savepath, Server* manager) {
    //// Opening of .ini file in tempdir
    QStringList nameFilter("*.ini");
    QDir directory(tempdir->path());
    QString iniFile = tempdir->path() + "/" + directory.entryList(nameFilter).first();

    //// Modification
    QSettings settings(iniFile, QSettings::IniFormat);
    int count = settings.value("General/trackCount").toInt();

    for(int i = 0; i < count; ++ i) {
        settings.setValue(QString("Track%1/volume").arg(i), manager->player.getTrack(i)->getVolume());
        settings.setValue(QString("Track%1/pan").arg(i), manager->player.getTrack(i)->getPan());
    }

    settings.sync();

    KZip archive(savepath);

    if (!archive.open(QIODevice::ReadWrite)) {
        throw std::runtime_error("Invalid file");
    }

    archive.addLocalFile(iniFile, directory.entryList(nameFilter).first());
}
