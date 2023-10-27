#include "serverwindow.h"
#include "./ui_serverwindow.h"

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &ServerWindow::slotNewConnection);
    if(server->listen(QHostAddress::Any, 2323))
    {
        qDebug() << "start server";
        ui->buttonRotationStop->setPalette(QPalette(Qt::darkGreen));
        rotationStatus = ui->buttonRotationStop->text();
        ui->buttonRadiationOff->setPalette(QPalette(Qt::darkGreen));
        radiationStatus = ui->buttonRadiationOff->text();
        beamLineAngle = 0.0f;
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(timerSlot()));
        timer->start(50);
    }
    else
    {
        qDebug() << "Error! server don't started.";
    }

}

ServerWindow::~ServerWindow()
{
    delete ui;
}

// User Interafce

// Setting indication of the active antenna rotation control button
void ServerWindow::setCheckedRotationButton(QString buttonText)
{
    QPushButton* buttons[3] = {ui->buttonRotationStop, ui->buttonRotation3rpm, ui->buttonRotation6rpm};
    rotationStatus = buttonText;
    for (auto btn : buttons)
    {
        btn->setPalette(QPalette(btn->text() == buttonText?Qt::darkGreen:QColor(239,239,239)));
    }
    sendToClients("rotation", buttonText);

}

// Processing of clicked the antenna rotation control buttons
void ServerWindow::on_buttonRotationStop_clicked()
{
    setCheckedRotationButton(ui->buttonRotationStop->text());

}


void ServerWindow::on_buttonRotation3rpm_clicked()
{
    setCheckedRotationButton(ui->buttonRotation3rpm->text());
}

void ServerWindow::on_buttonRotation6rpm_clicked()
{
    setCheckedRotationButton(ui->buttonRotation6rpm->text());
}

// Setting indication of the active radiation control button
void ServerWindow::setCheckedRadiationButton(QString buttonText)
{
    QPushButton* buttons[3] = {ui->buttonRadiationOff, ui->buttonRadiation50, ui->buttonRadiation100};
    radiationStatus = buttonText;
    for (auto btn : buttons)
    {
        btn->setPalette(QPalette(btn->text() == buttonText?Qt::darkGreen:QColor(239,239,239)));
    }
    sendToClients("radiation", buttonText);
}

// Processing of clicking the radiation control buttons
void ServerWindow::on_buttonRadiationOff_clicked()
{
    setCheckedRadiationButton(ui->buttonRadiationOff->text());

}

void ServerWindow::on_buttonRadiation50_clicked()
{
    setCheckedRadiationButton(ui->buttonRadiation50->text());
}


void ServerWindow::on_buttonRadiation100_clicked()
{
    setCheckedRadiationButton(ui->buttonRadiation100->text());
}

//Updade beam line angle timer
void ServerWindow::timerSlot()
{
    if(rotationStatus!=ui->buttonRotationStop->text()){
        if (rotationStatus==ui->buttonRotation3rpm->text())
            beamLineAngle+=0.9f;
        else
            beamLineAngle+=1.8f;
        if (beamLineAngle >= 360)
            beamLineAngle-= 360;
        sendToClients("angle", QString::number(beamLineAngle));
    }
 }

//Conections
//Firct connection to client and sending all parameters
void ServerWindow::slotNewConnection(){
    nextBlockSize = 0;
    socket = server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &ServerWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ServerWindow::slotClientDisconnected);

    socketList.push_back(socket);
    connectionCounter();

    sendToClients("rotation", rotationStatus, socket);
    sendToClients("radiation", radiationStatus, socket);
    sendToClients("angle", QString::number(beamLineAngle), socket);
    qDebug() << "new rmo connected";
}

// Delete disconected client socket and this socket from socketList
void ServerWindow::slotClientDisconnected()
{
    socket = (QTcpSocket*)sender();
    socketList.erase(std::remove(socketList.begin(),socketList.end(), socket), socketList.end());
    socket->deleteLater();
    connectionCounter();
    qDebug() << "rmo disconnected";
}

//Receiving and processing messages from the clients
void ServerWindow::slotReadyRead()
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
            QString parameter;
            QString value;
            in >> parameter >> value;
            qDebug() << parameter << value;

            if (parameter == "rotation")
                setCheckedRotationButton(value);
            else if (parameter == "radiation")
                setCheckedRadiationButton(value);
        }
    }
    else
    {
        qDebug() << "connection error!";
    }
}

// Sending message to one client if it is specified in the parameters, else to all clients
void ServerWindow::sendToClients(QString action, QString message, QTcpSocket* socket1)
{
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << quint16(0) << action << message;
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

// counts the number of connections and displays it on a button
void ServerWindow::connectionCounter()
{
    ui->connectionCounterButton->setText(QString::number(socketList.size()));
    ui->connectionCounterButton->setPalette(QPalette(socketList.size()==0?Qt::darkRed:Qt::darkGreen));

}


