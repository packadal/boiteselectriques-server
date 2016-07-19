#include "Server.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    qDebug() << "main";

    QApplication a(argc, argv);

    qDebug() << "window:";

    Server w;

    qDebug() << "exec:";
    return a.exec();
}
