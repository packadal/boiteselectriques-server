#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

/**
 * @file SerialManager.h
 * @brief Serial port interface
 */

#include <QThread>
#include <QString>
#include <QStringList>
#include <QtSerialPort/QtSerialPort>
#include <memory>
#include <wiringPiSPI.h>
#include <wiringPi.h>
#include <mcp3004.h>

class QSerialPort;


#define SERIAL_PORT "ttymxc3" /*< Serial port to open */

/**
 * @brief The SerialManager class
 *
 * Serial port interface, to read the messages from the Arduino.
 */
class SerialManager : public QThread {
    Q_OBJECT
public:
    SerialManager(QObject* parent = 0);

signals:
    /**
     * @brief Notify of a box activation
     * @param box Related box number
     * @param val Sensor value
     */
    void boxActivated(int box, int val);

public slots:
    /**
     * @brief Stop the messages reading
     */
    void stop();

protected:
    /**
     * @brief Start the messages reading
     */
    virtual void run();

private:
    bool finished {false}; /*< Status of the reading */
    std::shared_ptr<QSerialPort> port; /*< Port to open */
};

#endif // SERIALMANAGER_H
