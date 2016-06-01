#include "MainWidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget) {
    ui->setupUi(this);
    connect(ui->channelList, SIGNAL(volumeChanged(int, int)),
            &playThread,	 SLOT(setVolume(int,int)));
    connect(ui->channelList, SIGNAL(panChanged(int, int)),
            &playThread,	 SLOT(setPan(int,int)));
    connect(ui->channelList, SIGNAL(muteChanged(int,bool)),
            &playThread,	 SLOT(setMute(int,bool)));

    connect(ui->masterVolume, SIGNAL(valueChanged(int)),
            &playThread,	  SLOT(setMasterVolume(int)));

    connect(&playThread,	&PlayThread::spentTime,
            this,		&MainWidget::updateBeat);
    connect(&playThread,	&PlayThread::setTotalTime,
            this,		&MainWidget::updateBeatCount);
    connect(&playThread,	&PlayThread::charged,
            this,		&MainWidget::ischarged);
    connect(&savemanager,	&SaveManager::send_liste_name,
            this,		&MainWidget::liste_track);

    connect(&serialmanager,	&SerialManager::boxActivated,
            this,			&MainWidget::switchBox);

    connect(&serialmanager,	&SerialManager::boxActivated,
            this,			&MainWidget::send_Boite);

    connect(this,		 &MainWidget::openConfDialog,
            &confdialog, &ConfigurationDialog::exec);

    ui->masterVolume->setDefaultValue(50);
    ui->masterVolume->setEnabledStylesheet();
    serialmanager.start();

    //retour de tablette
    oscReceiver.addHandler("/box/update_treshold",
                           std::bind(&MainWidget::handle__box_receive_new_treshold,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/enable",
                           std::bind(&MainWidget::handle__box_enable,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/volume",
                           std::bind(&MainWidget::handle__box_volume,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/pan",
                           std::bind(&MainWidget::handle__box_pan,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/mute",
                           std::bind(&MainWidget::handle__box_mute,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/solo",
                           std::bind(&MainWidget::handle__box_solo,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/play",
                           std::bind(&MainWidget::handle__box_play,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/stop",
                           std::bind(&MainWidget::handle__box_stop,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/master",
                           std::bind(&MainWidget::handle__box_master,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/reset",
                           std::bind(&MainWidget::handle__box_reset,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/refreshsong",
                           std::bind(&MainWidget::handle__box_refreshsong,
                                     this, std::placeholders::_1));
    oscReceiver.addHandler("/box/selectsong",
                           std::bind(&MainWidget::handle__box_selectsong,
                                     this, std::placeholders::_1));

    oscReceiver.run();
}

MainWidget::~MainWidget() {
    delete ui;
    stop();
    playThread.wait();

    serialmanager.stop();
    serialmanager.wait();
}

//test envoie sur tablette
void MainWidget::sensibility_boites_to_send() {
    int sensor = 99-(confdialog.threshold-100)/4;
    send_sensibility(sensor);
}

void MainWidget::send_go(bool i) {
    ready(i);
}

void MainWidget::send_numb_track(int i) {
    numb_track(i);
}

void MainWidget::send_Boite(int i, int val) {
    if(val >= confdialog.threshold)
        buton_send(i);
}

void MainWidget::send_Beat(int i) {
    send_BeatCount(i);
}

void MainWidget::send_Liste(const char *i) {
    send_listeSong(i);
}

void MainWidget::send_Titre(const char *i) {
    send_titreSong(i);
}

//reception tablette
void MainWidget::handle__box_receive_new_treshold(osc::ReceivedMessageArgumentStream args) {
    osc::int32 sense_updated;
    args >> sense_updated;
    connect(this, SIGNAL(update_treshold(int)), &confdialog, SLOT(newTreshold(int)) );//&confdialog
    emit update_treshold(sense_updated);
}

void MainWidget::handle__box_selectsong(osc::ReceivedMessageArgumentStream args) {
    const char *receptSong;
    args >> receptSong;
    QString so = QString(QLatin1String(receptSong));
    if(!so.isEmpty())
        selSong = so;
    connect(this, SIGNAL(actionCharger()), this, SLOT(load()) );
    emit actionCharger();
}

void MainWidget::handle__box_refreshsong(osc::ReceivedMessageArgumentStream args) {
    bool etat;
    args >> etat;
    load();
}
void MainWidget::handle__box_enable(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    args >> box;
    switchBox(box, confdialog.threshold);
}

void MainWidget::handle__box_volume(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    osc::int32 vol;
    args >> box >> vol;
    if(box < ui->channelList->channels.size())
        ui->channelList->channels[box]->setVolume(vol);
}

void MainWidget::handle__box_pan(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    osc::int32 vol;
    args >> box >> vol;
    if(box < ui->channelList->channels.size())
        ui->channelList->channels[box]->setPan(vol);
}

void MainWidget::handle__box_mute(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    bool etat;
    args >> box >> etat;
    if(box < ui->channelList->channels.size())
        ui->channelList->channels[box]->mute(etat);
}

void MainWidget::handle__box_solo(osc::ReceivedMessageArgumentStream args) {
    osc::int32 box;
    bool etat;
    args >> box >> etat;
    if(box < ui->channelList->channels.size())
        ui->channelList->channels[box]->solo(etat);
}

void MainWidget::handle__box_play(osc::ReceivedMessageArgumentStream args) {
    bool etat;
    args >> etat;
    play();
}

void MainWidget::handle__box_stop(osc::ReceivedMessageArgumentStream args) {
    bool etat;
    args >> etat;
    stop();
}

void MainWidget::handle__box_master(osc::ReceivedMessageArgumentStream args) {
    osc::int32 vol;
    args >> vol;
    ui->masterVolume->setValue(vol);
}

void MainWidget::handle__box_reset(osc::ReceivedMessageArgumentStream args) {
    bool etat;
    args >> etat;
    if(etat)
        stop();
    ui->masterVolume->setValue(ui->masterVolume->getDefaultValue());
    ui->channelList->reset();
}

void MainWidget::reset() {
    stop();
    ui->masterVolume->setValue(ui->masterVolume->getDefaultValue());
    ui->channelList->reset();
}

void MainWidget::switchBox(int i, int val) {
    if(val >= confdialog.threshold)
        ui->channelList->switchBox(i);
}

void MainWidget::play() {
    if(!m_loaded) {
        if(load()) return;
    }

    send_Play(getTempo());
    playThread.start();
    m_playing = true;
}

void MainWidget::stop() {
    if(!m_loaded || !m_playing) return;

    playThread.stop();
    m_previousBeat = 0;
    m_playing = false;
    updateBeat(0);
}

void MainWidget::updateBeat(double t) { // en secondes

    int time= (int)(t * getTempo() / 60.0f) + 1;

    if(time != m_previousBeat && time <= m_beatCount && !playThread.isStopped()) {
        ui->temps->setText(QString("%1 / %2").arg(time)
                           .arg(int(m_beatCount)));
        m_previousBeat = time;

        ui->timeCount->setBeats(time);
        //send_Beat(time);
        if(time == 8 || time == 16 || time == 24){ //sync
            buton_sync(ui->channelList->getAllEnable());
            send_Beat(time);
        }
    } else if(playThread.isStopped()) {
        ui->temps->setText(QString("%1 / %2").arg(1)
                           .arg(int(m_beatCount)));
        m_previousBeat = 0;

        ui->timeCount->setBeats(0);
        //send_Beat(0);
    }
}

void MainWidget::updateBeatCount(double t) { // en secondes
    // Ici on calcule le nombre de temps dans la boucle.
    // Formule : secondes * tempo/60 = nb. temps ds boucle.

    m_beatCount = t * getTempo() / 60.0;
    m_previousBeat = -1;

    ui->timeCount->setMaxBeats(m_beatCount);
}

void MainWidget::ischarged(int on,int max) { // qd tout est chargé
    if (on == max)
        send_go(true);
}

void MainWidget::liste_track(const char *liste) { // qd tout est chargé
    send_listeTrack(liste);
}

int MainWidget::load() {
    QDir export_folder("/home/ubuntu/songs");
    //QDir export_folder("/home/carlou/songs");
    export_folder.setNameFilters(QStringList()<<"*.song");
    QStringList fileList = export_folder.entryList();
    QString str = fileList.join("|");
    QByteArray liste = str.toLatin1();
    const char *c_liste = liste.data();
    send_Liste(c_liste);
    sensibility_boites_to_send();
    selSong="08-01 Metisolea_44k.song";

    if (!selSong.isEmpty()) {
        try {
            stop();

            //QString file = "/home/ubuntu/songs/"+selSong;
            QString file = "/home/carlou/songs/"+selSong;

            if(!file.isEmpty()) {
                currentFile = file;
                song = savemanager.load(file);
                ui->timeCount->setNumerator(song.sigNumerateur);
                ui->timeCount->setDenominator(song.sigDenominateur);

                setTempo(song.tempo);
                playThread.load(song);

                ui->channelList->clear();
                ui->channelList->load(song);
                ui->masterVolume->setValue(ui->masterVolume->getDefaultValue());
                ui->morceau->setText(QString::fromStdString(song.name));
                m_loaded = true;
                QString titre_brut = QString::fromStdString(song.name);
                QByteArray titre = titre_brut.toLatin1();
                const char *c_titre = titre.data();
                send_Titre(c_titre);

                int j = ui->channelList->channels.size();
                send_numb_track(j);

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

void MainWidget::save() {
    try {
        if(m_loaded)
            savemanager.save(currentFile, this);
    } catch(std::exception& e) {
        QMessageBox::warning(this, tr("Erreur à la sauvegarde"), e.what());
    }
}
