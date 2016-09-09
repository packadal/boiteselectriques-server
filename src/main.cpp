/**
 * @file main.cpp
 * @brief Run the program
 */

#include "Server.h"
#include <QCoreApplication>
#include <QDebug>
#include <csignal>

#define COMPANY_NAME "Rock & Chanson"
#define APP_NAME "Boites Electriques"

struct QuitStruct{
    QuitStruct(){
        std::signal(SIGINT, &QuitStruct::exitApp );
        std::signal(SIGTERM, &QuitStruct::exitApp );
    }

    static void exitApp(int sig){
        qDebug() << "Exit with signal " << sig;
        QCoreApplication::exit(0);
    }
};

int main(int argc, char *argv[]) {
    QuitStruct qs;
    QCoreApplication app(argc, argv);

    QCommandLineParser p;
    p.setApplicationDescription("Boites Electriques - Server");
    p.addHelpOption();
    QCommandLineOption configFile(QStringList()<<"c"<<"config",
                                  QCoreApplication::translate("config", "Set config file path to <directory>"),
                                  QCoreApplication::translate("config", "directory"));
    p.addOption(configFile);
    p.process(app);

    QSettings* opt;
    if(p.isSet(configFile)){
        qDebug() << "Cust : " << p.value(configFile);
        opt = new QSettings(p.value(configFile), QSettings::IniFormat);
    } else {
        opt = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                            COMPANY_NAME, APP_NAME);
        }

    Server s(opt);

    return app.exec();
}
