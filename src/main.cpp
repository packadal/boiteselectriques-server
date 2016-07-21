/**
 * @file main.cpp
 * @brief Run the program
 */

#include "Server.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Server w;
    return a.exec();
}
