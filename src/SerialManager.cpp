#include "SerialManager.h"

/**
 * @file SerialManager.cpp
 * @brief Serial port interface implementation
 */

#define STR_SIZE 128
#define BAUD_RATE 115200
#define WAIT_TIME 100 /*< Time to wait between readings */

#define BASE 100
#define SPI_CHAN 0
#define SPIspeed 1000000

SerialManager::SerialManager(QObject *parent):
    QThread(parent) {

    qDebug() << "SerialManager";
}

void SerialManager::stop() {
    this->m_isRunning = false;
}

void SerialManager::run() {

#ifdef UDOO_BUILD // On UDOO
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

                if(numRead > 0)
                    emit boxActivated(msg[0].toInt(),
                        msg[1].toInt());
            }
        }
    }
#else // On Raspberry Pi
    int PadCutOff[8] = {300,300,300,300,300,300,300,300};
    int MaxPlayTime[8] = {90,90,90,90,90,90,90,90};
    bool ActivePad[8] = {0,0,0,0,0,0,0,0};
    int PinPlayTime[8] = {0,0,0,0,0,0,0,0};
    int pin = 0;
    int hitavg = 0;

    this->m_isRunning = true;

    //wiringPiSetupSys ();

    mcp3004Setup(BASE,SPI_CHAN);

    while (this->m_isRunning){

       for(pin=0; pin<8; pin++){

           hitavg = analogRead(BASE + pin);

           if(hitavg>PadCutOff[pin]){
               if(ActivePad[pin]==false){
                   emit boxActivated(pin, 0);
                   PinPlayTime[pin]=0;
                   ActivePad[pin]=true;
               } else {
                   PinPlayTime[pin]=PinPlayTime[pin]+1;
               }
           }
           else if(ActivePad[pin]==true){
               PinPlayTime[pin]=PinPlayTime[pin]+1;
               if(PinPlayTime[pin]>MaxPlayTime[pin]){
                   ActivePad[pin] = false;
                   emit boxActivated(pin, hitavg+300);
               }
           }

       }

    }
#endif

}
