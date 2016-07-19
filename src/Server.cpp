#include "Server.h"

#include <QDebug>

Server::Server(QWidget *parent) :
    QWidget(parent)
    /*,ui(new Ui::MainWidget)*/ {
    /*ui->setupUi(this);
    connect(ui->channelList, SIGNAL(volumeChanged(int, int)),
            &playThread,	 SLOT(setVolume(int,int)));
    connect(ui->channelList, SIGNAL(panChanged(int, int)),
            &playThread,	 SLOT(setPan(int,int)));
    connect(ui->channelList, SIGNAL(muteChanged(int,bool)),
            &playThread,	 SLOT(setMute(int,bool)));

    connect(ui->masterVolume, SIGNAL(valueChanged(int)),
            &playThread,	  SLOT(setMasterVolume(int)));*/

    qDebug() << "MainWidget";

    connect(&playThread, &PlayThread::muteChanged,
            &playThread, &PlayThread::setMute);
    connect(this, &Server::updateThreshold,
            &playThread, &PlayThread::setThreshold);
    connect(this, &Server::resetThreshold,
            &playThread, &PlayThread::resetThreshold);

    connect(&playThread,	&PlayThread::spentTime,
            this,		&Server::updateBeat);
    connect(&playThread,	&PlayThread::setTotalTime,
            this,		&Server::updateBeatCount);
    connect(&playThread,	&PlayThread::charged,
            this,		&Server::onIsLoaded);
    connect(&savemanager,	&SaveManager::send_liste_name,
            this,		&Server::updateTrackList);

    connect(&serialmanager,	&SerialManager::boxActivated,
            this,			&Server::switchBox);

    connect(&serialmanager,	&SerialManager::boxActivated,
            this,			&Server::send_Boite);

    /*connect(this,		 &MainWidget::openConfDialog,
            &confdialog, &ConfigurationDialog::exec);*/

    //ui->masterVolume->setDefaultValue(50);
    //ui->masterVolume->setEnabledStylesheet();
    serialmanager.start();

    //retour de tablette
    oscReceiver.addHandler("/box/update_threshold",
                           std::bind(&Server::handle__box_updateThreshold,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/reset_threshold",
                           std::bind(&Server::handle__box_resetThreshold,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/enable",
                           std::bind(&Server::handle__box_enable,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/volume",
                           std::bind(&Server::handle__box_volume,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/pan",
                           std::bind(&Server::handle__box_pan,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/mute",
                           std::bind(&Server::handle__box_mute,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/solo",
                           std::bind(&Server::handle__box_solo,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/play",
                           std::bind(&Server::handle__box_play,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/stop",
                           std::bind(&Server::handle__box_stop,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/master",
                           std::bind(&Server::handle__box_master,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/reset",
                           std::bind(&Server::handle__box_reset,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/refresh_song",
                           std::bind(&Server::handle__box_refreshSong,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/select_song",
                           std::bind(&Server::handle__box_selectSong,
                                     this, std::placeholders::_1));

    oscReceiver.run();
}

Server::~Server() {
    //delete ui;
    stop();
    playThread.wait();

    serialmanager.stop();
    serialmanager.wait();
}

int Server::getTempo() const {
    return m_tempo;
}

//test envoie sur tablette
int Server::getThreshold() {
    //int sensor = 99-(confdialog.threshold-100)/4;
    return 99 - (playThread.getThreshold()-100)/4;
}

void Server::send_go(bool i) {
    ready(i);
}

void Server::send_numb_track(int i) {
    sendTracksCount(i);
}

void Server::send_Boite(int i, int val) {
    //if(val >= confdialog.threshold)
    if(val >= playThread.getThreshold()){
        qDebug() << "SEND " << i;
        sendBoxActivation(i);
    }
}

void Server::send_Beat(int i) {
    sendBeatCount(i);
}

void Server::send_Liste(const char *i) {
    sendSongsList(i);
}

void Server::send_Titre(const char *i) {
    sendSongTitle(i);
}

//reception tablette
void Server::handle__box_updateThreshold(osc::ReceivedMessageArgumentStream args) {
    osc::int32 sense_updated;
    args >> sense_updated;
    qDebug() << "MAINWIDGET::UPDATE_THRESHOLD" << sense_updated;
    //connect(this, SIGNAL(update_threshold(int)), &confdialog, SLOT(newThreshold(int)) );//&confdialog
    emit updateThreshold(sense_updated);
}

void Server::handle__box_resetThreshold(osc::ReceivedMessageArgumentStream args) {
    osc::int32 sense_updated;
    args >> sense_updated;
    //connect(this, SIGNAL(update_threshold(int)), &confdialog, SLOT(newThreshold(int)) );//&confdialog
    emit resetThreshold();
}

void Server::handle__box_selectSong(osc::ReceivedMessageArgumentStream args) {
    const char *receptSong;
    args >> receptSong;
    QString so = QString(QLatin1String(receptSong));
    if(!so.isEmpty())
        selSong = so;
    connect(this, SIGNAL(actionLoad()), this, SLOT(load()) );
    emit actionLoad();
}

void Server::handle__box_refreshSong(osc::ReceivedMessageArgumentStream args) {
    bool etat;
    args >> etat;
    load();
}
void Server::handle__box_enable(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    args >> box;
    switchBox(box, playThread.getThreshold());
}

void Server::handle__box_volume(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    osc::int32 vol;
    args >> box >> vol;
    /*if(box < ui->channelList->channels.size())
        ui->channelList->channels[box]->setVolume(vol);*/
    if(box < playThread.tracksCount())
        playThread.setVolume(box, vol);
}

void Server::handle__box_pan(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    osc::int32 vol;
    args >> box >> vol;
    /*if(box < ui->channelList->channels.size())
        ui->channelList->channels[box]->setPan(vol);*/
    if(box < playThread.tracksCount())
        playThread.setPan(box, vol);
}

void Server::handle__box_mute(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    bool etat;
    args >> box >> etat;
    /*if(box < ui->channelList->channels.size())
        ui->channelList->channels[box]->mute(etat);*/
    qDebug() << "MAINWIDGET::HANDLE_MUTE" << box << (etat ? "TRUE" : "FALSE");
    if(box < playThread.tracksCount())
        playThread.setMute(box, etat);
}

void Server::handle__box_solo(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    bool etat;
    args >> box >> etat;
    /*if(box < ui->channelList->channels.size())
        ui->channelList->channels[box]->solo(etat);*/
    playThread.solo(box, etat);
}

void Server::handle__box_play(osc::ReceivedMessageArgumentStream args) {
    bool etat;
    args >> etat;
    play();
}

void Server::handle__box_stop(osc::ReceivedMessageArgumentStream args) {
    bool etat;
    args >> etat;
    stop();
}

void Server::handle__box_master(osc::ReceivedMessageArgumentStream args) {
    osc::int32 vol;
    args >> vol;
    //ui->masterVolume->setValue(vol);
    playThread.setMasterVolume(vol);
}

void Server::handle__box_reset(osc::ReceivedMessageArgumentStream args) {
    bool etat;
    args >> etat;
    if(etat)
        stop();
    //ui->masterVolume->setValue(ui->masterVolume->getDefaultValue());
    //ui->channelList->reset();
    playThread.reset();
}

void Server::reset() {
    stop();
    //ui->masterVolume->setValue(ui->masterVolume->getDefaultValue());
    //ui->channelList->reset();
    playThread.reset();
}

void Server::switchBox(int i, int val) {
    /*if(val >= confdialog.threshold)
        ui->channelList->switchBox(i);*/

    if(i < playThread.tracksCount() && val >= playThread.getThreshold()) {
        qDebug() << "SWITCH " << i;
        playThread.switchBox(i);
    } else
        qDebug() << "NOSWITCH " << i;
}

void Server::play() {
    if(!m_loaded) {
        if(load()) return;
    }

    sendPlay(getTempo());
    playThread.start();
    m_playing = true;
}

void Server::stop() {
    if(!m_loaded || !m_playing) return;

    playThread.stop();
    m_previousBeat = 0;
    m_playing = false;
    updateBeat(0);
}

void Server::updateBeat(double t) { // en secondes

    int time= (int)(t * getTempo() / 60.0f) + 1;

    if(time != m_previousBeat && time <= m_beatCount && !playThread.isStopped()) {
        //ui->temps->setText(QString("%1 / %2").arg(time)
        //                   .arg(int(m_beatCount)));
        m_previousBeat = time;

        //ui->timeCount->setBeats(time);
        send_Beat(time);
        if(time == 8 || time == 16 || time == 24){ //sync
            sendActivatedTracks(playThread.getActivatedTracks());
            send_Beat(time);
        }
    } else if(playThread.isStopped()) {
        //ui->temps->setText(QString("%1 / %2").arg(1)
        //                   .arg(int(m_beatCount)));
        m_previousBeat = 0;

        //ui->timeCount->setBeats(0);
        send_Beat(0);
    }
}

void Server::updateBeatCount(double t) { // en secondes
    // Ici on calcule le nombre de temps dans la boucle.
    // Formule : secondes * tempo/60 = nb. temps ds boucle.

    m_beatCount = t * getTempo() / 60.0;
    m_previousBeat = -1;

    //ui->timeCount->setMaxBeats(m_beatCount);
}

void Server::onIsLoaded(int on,int max) { // qd tout est chargé
    if (on == max)
        send_go(true);
}

void Server::updateTrackList(const char *list) { // qd tout est chargé
    sendTrackList(list);
}

void Server::setTempo(int arg) {
    m_tempo = arg;
}

void Server::sendThreshold(int boxSensor) {
    sender.send(osc::MessageGenerator()("/box/sensor", boxSensor));
}

void Server::sendBoxActivation(int chan) {
    sender.send(osc::MessageGenerator()("/box/enable_out", chan));
}

void Server::sendActivatedTracks(int val) {
    sender.send(osc::MessageGenerator()("/box/enable_sync", val));
}

void Server::sendBeatCount(int beat) {
    sender.send(osc::MessageGenerator()("/box/beat", beat));
}

void Server::sendPlay(int tempo) {
    sender.send(osc::MessageGenerator()("/box/play", tempo));
}

void Server::sendSongTitle(const char *title) {
    sender.send(osc::MessageGenerator()("/box/titre", title));
}

void Server::sendSongsList(const char *list) {
    sender.send(osc::MessageGenerator()("/box/liste", list));
}

void Server::sendTracksCount(int num) {
    sender.send(osc::MessageGenerator()("/box/NumbTrack", num));
}

void Server::sendTrackList(const char *list) {
    sender.send(osc::MessageGenerator()("/box/listeTrack", list));
}

void Server::ready(bool ready) {
    sender.send(osc::MessageGenerator()("/box/ready_to_go", ready));
}

int Server::load() {
    QDir export_folder("/home/ubuntu/songs");
    //QDir export_folder("/home/carlou/songs");
    export_folder.setNameFilters(QStringList()<<"*.song");
    QStringList fileList = export_folder.entryList();
    QString str = fileList.join("|");
    QByteArray liste = str.toLatin1();
    const char *c_liste = liste.data();
    send_Liste(c_liste);
    sendThreshold(getThreshold());
    //selSong="08-01 Metisolea .song";

    if (!selSong.isEmpty()) {
        try {
            stop();

            QString file = "/home/ubuntu/songs/"+selSong;
            //QString file = "/home/carlou/songs/"+selSong;

            if(!file.isEmpty()) {
                currentFile = file;
                song = savemanager.load(file);
                //ui->timeCount->setNumerator(song.sigNumerateur);
                //ui->timeCount->setDenominator(song.sigDenominateur);

                setTempo(song.tempo);
                playThread.load(song);

                /*ui->channelList->clear();
                ui->channelList->load(song);
                ui->masterVolume->setValue(ui->masterVolume->getDefaultValue());
                ui->morceau->setText(QString::fromStdString(song.name));*/
                m_loaded = true;
                QString titre_brut = QString::fromStdString(song.name);
                QByteArray titre = titre_brut.toLatin1();
                const char *c_titre = titre.data();
                send_Titre(c_titre);

                //int j = ui->channelList->channels.size();
                //send_numb_track(j);
                send_numb_track(playThread.tracksCount());

                return 0;
            }
            return 1;
        } catch(std::exception& e) {
            QMessageBox::warning(this, tr("Erreur au chargement"), e.what());
            return 1;
        }
    }//end verif selsong
    return 1;
}

void Server::save() {
    try {
        if(m_loaded)
            savemanager.save(currentFile, this);
    } catch(std::exception& e) {
        QMessageBox::warning(this, tr("Erreur à la sauvegarde"), e.what());
    }
}
