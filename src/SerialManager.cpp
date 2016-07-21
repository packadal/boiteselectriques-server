#include "SerialManager.h"

/**
 * @file SerialManager.cpp
 * @brief Serial port interface implementation
 */

#define STR_SIZE 128
#define BAUD_RATE 115200
#define WAIT_TIME 100 /*< Time to wait between readings */

SerialManager::SerialManager(QObject *parent):
    QThread(parent) {

    qDebug() << "SerialManager";
}

void SerialManager::stop() {
    finished = true;
}

void SerialManager::run() {
    // Port opening
    port = std::make_shared<QSerialPort>(SERIAL_PORT);

    // Configuration
    port->open(QIODevice::ReadOnly);
    port->setBaudRate(BAUD_RATE);
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::HardwareControl);

    if(port->isOpen()) {
        char txt[STR_SIZE];
        int numRead;

        while(!finished) {
            port->waitForReadyRead(WAIT_TIME);
            if(port->canReadLine()) {
                numRead = port->readLine(txt, STR_SIZE);
                auto msg = QString(txt).simplified().split(' ');
                if(numRead > 0) emit boxActivated(msg[0].toInt(), msg[1].toInt());
            }
        }
    }
}
