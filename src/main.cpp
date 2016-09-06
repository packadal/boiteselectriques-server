/**
 * @file main.cpp
 * @brief Run the program
 */

#include "Server.h"
#include <QCoreApplication>
#include <QDebug>
#include <csignal>

struct QuitStruct{
    QuitStruct(){
        std::signal(SIGINT, &QuitStruct::exitApp );
        std::signal(SIGTERM, &QuitStruct::exitApp );
    }

    static void exitApp(int sig){
        QCoreApplication::exit(0);
    }
};

int main(int argc, char *argv[]) {
    QuitStruct qs;
    Server s(argc, argv);
    return s.exec();
}
