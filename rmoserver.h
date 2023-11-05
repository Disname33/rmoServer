#ifndef RMOSERVER_H
#define RMOSERVER_H

#include <QtCore/qglobal.h>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include "radarparameters.h"


class RmoServer : public QTcpServer
{
    Q_OBJECT

public:
    RmoServer();
    ~RmoServer();

private:
    QTcpSocket *socket;
    QVector <QTcpSocket*> socketList;
    QByteArray data;
    QTimer *timer;
    float beamLineAngle;
    quint16 nextBlockSize;
    quint8 rotationStatus;
    quint8 radiationStatus;
    quint8 maxDistance;
    void sendToClients(RadarParameters parameter, float value, QTcpSocket* socket1 = nullptr);
    void connectionCounter();

private slots:
    void incomingConnection(qintptr handle);
    void slotClientDisconnected();
    void slotReadyRead();

    void timerSlot();
};

#endif // RMOSERVER_H
