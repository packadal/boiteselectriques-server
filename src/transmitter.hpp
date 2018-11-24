#ifndef TRANSMITTER_HPP
#define TRANSMITTER_HPP

#include <QCoreApplication>
#include <QDataStream>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>

#include <functional>

class Transmitter : public QObject {
  Q_OBJECT

public:
  Transmitter(const QString &hostName, quint16 receivePort = 9988,
              quint16 sendPort = 9989)
      : m_hostname(hostName), m_receivePort(receivePort), m_sendPort(sendPort) {
    m_socket.setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_socket.setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    connect(
        &m_socket,
        QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
        [this]() { qWarning() << m_socket.errorString(); });
    //    connect(&m_socket, &QAbstractSocket::connected, []() {
    //      qWarning() << "client connected";
    //    });

    m_inputData.setDevice(&m_socket);
    m_inputData.setVersion(QDataStream::Qt_5_0);

    connect(&m_socket, &QIODevice::readyRead, [this]() {
      m_inputData.startTransaction();

      quint16 size;

      do {
        m_inputData >> size;

        if (m_socket.bytesAvailable() < size) {
          qWarning() << m_socket.bytesAvailable() << " < " << size;
          m_inputData.rollbackTransaction();
          return;
        }

        QString eventName;
        m_inputData >> eventName;

        if (!m_handlers.contains(eventName)) {
          qWarning() << "unhandled event: " << eventName;
          exit(-1);
        } else {
          m_handlers[eventName](m_inputData);
        }
      } while (!m_inputData.atEnd());

      if (!m_inputData.commitTransaction()) {
        qWarning() << "could not commit transaction";
        return;
      }
    });

    if (!m_server.listen(QHostAddress::Any, m_receivePort)) {
      qWarning() << "could not start server";
    }

    connect(&m_connectionTimer, &QTimer::timeout, [this]() {
      if (m_socket.state() == QAbstractSocket::ConnectedState) {
        m_connectionTimer.stop();
        return;
      }
      m_socket.connectToHost(m_hostname, m_sendPort);
    });
    m_connectionTimer.start(100);
  }

  template <typename... Args> void send(QString eventName, Args... args) {
    QByteArray dataBuffer;
    QDataStream stream(&dataBuffer, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream << eventName;
    packData(stream, args...);
    internalSend(dataBuffer);
  }

  void registerEventHandler(const QString &eventName,
                            std::function<void(QDataStream &)> handler) {
    m_handlers[eventName] = handler;
  }

private:
  template <typename T> void packData(QDataStream &data, T first) {
    data << first;
  }
  template <typename T, typename... Args>
  void packData(QDataStream &data, T first, Args... args) {
    data << first;
    packData(data, args...);
  }
  void internalSend(const QByteArray &data) {
    while (m_serverConnection == nullptr) {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
      m_serverConnection = m_server.nextPendingConnection();
    }
    m_serverConnection->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_serverConnection->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    connect(
        m_serverConnection,
        QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
        [this]() {
          qWarning() << "server connection error: "
                     << m_serverConnection->errorString();
        });

    QByteArray sizeBuffer;
    QDataStream sizeStream(&sizeBuffer, QIODevice::WriteOnly);
    sizeStream.setVersion(QDataStream::Qt_5_0);
    sizeStream << quint16(data.size());

    m_serverConnection->write(sizeBuffer);
    if (m_serverConnection->write(data) < 0) {
      qWarning() << "error sending data: " << data;
    }
  }

private:
  QString m_hostname;
  quint16 m_receivePort;
  quint16 m_sendPort;

  QTcpServer m_server;
  QTcpSocket *m_serverConnection = nullptr;
  QTcpSocket m_socket;
  QDataStream m_inputData;
  QMap<QString, std::function<void(QDataStream &)>> m_handlers;

  QTimer m_connectionTimer;
};

#endif // TRANSMITTER_HPP
