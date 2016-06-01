#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "ui_MainWidget.h"
#include "ChannelListWidget.h"
#include "ChannelWidget.h"
#include "PlayThread.h"
#include "SaveManager.h"
#include "SerialManager.h"
#include "ConfigurationDialog.h"
#include "osc/oscreceiver.h"
#include "osc/oscsender.h"
#include "osc/oscmessagegenerator.h"

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <exception>

namespace Ui {
class MainWidget;
}

class SaveManager;
/**
 * @brief The MainWidget class
 *
 * L'écran principal
 */
class MainWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int tempo READ getTempo WRITE setTempo)

    friend class SaveManager;
public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

    int getTempo() const {
        return m_tempo;
    }

    void handle__box_receive_new_treshold(osc::ReceivedMessageArgumentStream args);
    void handle__box_enable(osc::ReceivedMessageArgumentStream args);
    void handle__box_volume(osc::ReceivedMessageArgumentStream args);
    void handle__box_pan(osc::ReceivedMessageArgumentStream args);
    void handle__box_mute(osc::ReceivedMessageArgumentStream args);
    void handle__box_solo(osc::ReceivedMessageArgumentStream args);
    void handle__box_play(osc::ReceivedMessageArgumentStream args);
    void handle__box_stop(osc::ReceivedMessageArgumentStream args);
    void handle__box_master(osc::ReceivedMessageArgumentStream args);
    void handle__box_reset(osc::ReceivedMessageArgumentStream args);
    void handle__box_refreshsong(osc::ReceivedMessageArgumentStream args);
    void handle__box_selectsong(osc::ReceivedMessageArgumentStream args);

signals:
    void openConfDialog();
    void select_Song(const char *);
    void actionCharger();
    void update_treshold(int);

public slots:
    //config sensibilité des boites
    void sensibility_boites_to_send();

    // Remet les paramètres à zéro
    void reset();

    // Active / désactive une boite
    void switchBox(int, int);

    // Lance la lecture
    void play();

    // Charge un morceau
    int load();

    // Sauve les modifications
    void save();

    // S'arrête
    void stop();

    // Met à jour le temps actuel affiché (ex. 13 / 32)
    void updateBeat(double);

    // Met à jour le temps total (ex. 32)
    void updateBeatCount(double t);

    //quand le titre est chargé
    void ischarged(int on, int max);

    //nom de chaque pistes
    void liste_track(const char *liste);

    // Définit le tempo
    void setTempo(int arg) {
        m_tempo = arg;
    }
    /* fonction renvois vers tablette*/
    void send_sensibility(int box_sensor) {
        sender.send(osc::MessageGenerator()("/box/sensor", box_sensor));
    }
    void buton_send(int chan) {
        sender.send(osc::MessageGenerator()("/box/enable_out", chan));
    }
    void buton_sync(int val) {
        sender.send(osc::MessageGenerator()("/box/enable_sync", val));
    }
    void send_BeatCount(int beat) {
        sender.send(osc::MessageGenerator()("/box/beat", beat));
    }
    void send_Play(int tempo) {
        sender.send(osc::MessageGenerator()("/box/play", tempo));
    }
    void send_titreSong(const char *song_titre) {
        sender.send(osc::MessageGenerator()("/box/titre", song_titre));
    }
    void send_listeSong(const char *song_liste) {
        sender.send(osc::MessageGenerator()("/box/liste", song_liste));
    }
    void numb_track(int numb_t) {
        sender.send(osc::MessageGenerator()("/box/NumbTrack", numb_t));
    }
    void send_listeTrack(const char *song_track) {
        sender.send(osc::MessageGenerator()("/box/listeTrack", song_track));
    }
    void ready(bool ready_to_go) {
        sender.send(osc::MessageGenerator()("/box/ready_to_go", ready_to_go));
    }

    //envoi sur tablette
    void send_Boite(int,int);
    void send_Beat(int);
    void send_Titre(const char *);
    void send_Liste(const char *);
    void send_numb_track(int);
    void send_go(bool);


private:
    Ui::MainWidget *ui;
    PlayThread playThread;
    SaveManager savemanager;
    SerialManager serialmanager {this};
    ConfigurationDialog confdialog {this};
    OscReceiver oscReceiver {9988};

    SongData song;

    QString selSong;

    QString currentFile {};

    int m_tempo {};
    double m_beatCount {};

    // Optimisation : on compare avec le temps précédent.
    int m_previousBeat {-1};

    // Indique si un morceau a été chargé.
    bool m_loaded {false};

    // Indique si un morceau est en cours de lecture
    bool m_playing {false};

    OscSender sender {"192.170.0.17", 9989};

};

#endif // MAINWIDGET_H
