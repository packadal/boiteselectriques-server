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

SerialManager::SerialManager(QObject* parent) : QThread(parent) {
  qDebug() << "SerialManager";
}

void SerialManager::stop() {
  this->m_isRunning = false;
}

void SerialManager::run() {
#ifdef UDOO_BUILD  // On UDOO
  // Port opening
  port = std::make_shared<QSerialPort>(SERIAL_PORT);

  // Configuration
  port->open(QIODevice::ReadOnly);
  port->setBaudRate(BAUD_RATE);
  port->setDataBits(QSerialPort::Data8);
  port->setParity(QSerialPort::NoParity);
  port->setStopBits(QSerialPort::OneStop);
  port->setFlowControl(QSerialPort::HardwareControl);

  if (port->isOpen()) {
    char txt[STR_SIZE];
    int numRead;

    while (!finished) {
      port->waitForReadyRead(WAIT_TIME);
      if (port->canReadLine()) {
        numRead = port->readLine(txt, STR_SIZE);
        auto msg = QString(txt).simplified().split(' ');

        if (numRead > 0)
          emit boxActivated(msg[0].toInt(), msg[1].toInt());
      }
    }
  }
#else  // On Raspberry Pi
  const int PadCutOff = 50;
  int TriggerHitValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  this->m_isRunning = true;

  // wiringPiSetupSys ();

  mcp3004Setup(BASE, SPI_CHAN);

  while (this->m_isRunning) {
    for (unsigned char pin = 0; pin < 8; pin++) {
      const int hitValue = analogRead(BASE + pin);
      const int actualValue = hitValue - PadCutOff;

      // if the hit is strong enough to not be background noise
      if (actualValue >= 0) {
        // decrement the value by PadCutOff so that 0 is the
        // lowest hit value that can possibly trigger the event
        TriggerHitValue[pin] = std::max(TriggerHitValue[pin], actualValue);
      }
      // if the value is below the threshold, we have fallen back to
      // background noise. If there was a triggering hit, send an event
      if (actualValue < 0 && TriggerHitValue[pin] > 0){
        emit boxActivated(pin, TriggerHitValue[pin]);
        TriggerHitValue[pin] = 0;
      }
    }
  }
#endif
}
