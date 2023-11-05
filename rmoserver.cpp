#include "rmoserver.h"

#include <qdatastream.h>
#include <QSettings>


RmoServer::RmoServer()
{
//    QSettings settings("/rmoconfig/settings.ini", QSettings::IniFormat, this);
    QSettings settings("./settings.ini", QSettings::IniFormat, this);
    settings.beginGroup("ServerConnection");
    quint16 serverPort = settings.value("port", 2323).toUInt();
    settings.endGroup();

    if(this->listen(QHostAddress::Any, serverPort))
    {
        qDebug() << "start server";
        beamLineAngle = 0.0f;
        rotationStatus = 0;
        radiationStatus = 0;
        maxDistance = 0;
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(timerSlot()));
        timer->start(50);
    }
    else
    {
        qDebug() << "Error! server don't started.";
    }

}

RmoServer::~RmoServer(){
    this->close();
}



//Updade beam line angle timer
void RmoServer::timerSlot()
{
    if(rotationStatus != 0 ){
        if (rotationStatus==1)
            beamLineAngle+=0.9f;
        else
            beamLineAngle+=1.8f;
        if (beamLineAngle >= 360)
            beamLineAngle-= 360;
        sendToClients(RadarParameters::AntennaPosition, beamLineAngle);
        qInfo()<<RadarParameters::AntennaPosition << beamLineAngle;
    }
 }

//Conections
//Firct connection to client and sending all parameters
void RmoServer::incomingConnection(qintptr handle){
    nextBlockSize = 0;
    socket = new QTcpSocket;
    socket->setSocketDescriptor(handle);
    connect(socket, &QTcpSocket::readyRead, this, &RmoServer::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &RmoServer::slotClientDisconnected);

    socketList.push_back(socket);

    sendToClients(RadarParameters::RotationSpeed, rotationStatus, socket);
    sendToClients(RadarParameters::RadiationPower, radiationStatus, socket);
    sendToClients(RadarParameters::MaxDistance, maxDistance, socket);
    sendToClients(RadarParameters::AntennaPosition, beamLineAngle, socket);
    qDebug() << "new rmo connected" << handle;
    connectionCounter();
}

// Delete disconected client socket and this socket from socketList
void RmoServer::slotClientDisconnected()
{
    socket = (QTcpSocket*)sender();
    socketList.erase(std::remove(socketList.begin(),socketList.end(), socket), socketList.end());
    socket->deleteLater();
    qDebug() << "rmo disconnected";
    connectionCounter();
}

//Receiving and processing messages from the clients
void RmoServer::slotReadyRead()
{
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_12);
    if(in.status()==QDataStream::Ok)
    {
        while (true)
        {
            if(nextBlockSize == 0)
            {
                if(socket->bytesAvailable() < 2)
                    break;
                in >> nextBlockSize;
            }
            if(socket->bytesAvailable()<nextBlockSize)
                break;
            nextBlockSize = 0;
            RadarParameters parameter;
            int value;
            in >> (quint32&)parameter >> value;
            qDebug() << parameter << value;


            switch (parameter) {
                case RadarParameters::RotationSpeed:
                    rotationStatus = value;
                    break;
                case RadarParameters::RadiationPower:
                    radiationStatus = value;
                    break;
                case RadarParameters::AntennaPosition:
                    beamLineAngle = float(value);
                    break;
                case RadarParameters::MaxDistance:
                    maxDistance = value;
                    break;
                default:
                    qDebug() << "Unknown Radar Parameter";
                    break;
            }
            sendToClients(parameter, value);

        }
    }
    else
    {
        qDebug() << "connection error!";
    }
}

// Sending message to one client if it is specified in the parameters, else to all clients
void RmoServer::sendToClients(RadarParameters parameter, float value, QTcpSocket* socket1)
{
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << quint16(0) << parameter << value;
    out.device()->seek(0);
    out << quint16(data.size()-sizeof(quint16));
    if (socket1!=nullptr)
    {
        if(socket1->state() == QAbstractSocket::ConnectedState || socket1->state() == QAbstractSocket::ConnectingState)
            socket1->write(data);
    }
    else
    {
        for(auto& s: socketList){
            if(s->state() == QAbstractSocket::ConnectedState || s->state() == QAbstractSocket::ConnectingState)
                s->write(data);
        }
    }
}

// counts the number of connections
void RmoServer::connectionCounter()
{
    qInfo() << "now" << socketList.size() << "connections";
}
