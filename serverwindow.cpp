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
        timer->start(60);
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

void ServerWindow::setCheckedRotationButton(QString buttonText){
    QPushButton* buttons[3] = {ui->buttonRotationStop, ui->buttonRotation3rpm, ui->buttonRotation6rpm};
    rotationStatus = buttonText;
    for (auto btn : buttons)
    {
        btn->setPalette(QPalette(btn->text() == buttonText?Qt::darkGreen:QColor(239,239,239)));
    }
    sendToClients("rotation", buttonText);

}

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

void ServerWindow::setCheckedRadiationButton(QString buttonText){
    QPushButton* buttons[3] = {ui->buttonRadiationOff, ui->buttonRadiation50, ui->buttonRadiation100};
    radiationStatus = buttonText;
    for (auto btn : buttons)
    {
        btn->setPalette(QPalette(btn->text() == buttonText?Qt::darkGreen:QColor(239,239,239)));
    }
    sendToClients("radiation", buttonText);
}

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

//Timer
void ServerWindow::timerSlot()
{
    if(rotationStatus!=ui->buttonRotationStop->text()){
        if (rotationStatus==ui->buttonRotation3rpm->text())
            beamLineAngle+=0.5f;
        else
            beamLineAngle+=1.0f;
        if (beamLineAngle >= 360)
            beamLineAngle-= 360;
        sendToClients("angle", QString::number(beamLineAngle));
    }
 }

//Conections

void ServerWindow::slotNewConnection(){
    nextBlockSize = 0;
    socket = server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &ServerWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ServerWindow::slotClientDisconnected);

    sockets.push_back(socket);
    connectionCounter();

    sendToClients("rotation", rotationStatus, socket);
//    sendToClients("radiation", radiationStatus, socket);
    qDebug() << "new rmo connected";
}

void ServerWindow::slotClientDisconnected()
{
    socket = (QTcpSocket*)sender();
    sockets.erase(std::remove(sockets.begin(),sockets.end(), socket), sockets.end());
    socket->deleteLater();
    connectionCounter();
    qDebug() << "rmo disconnected";
}

void ServerWindow::slotReadyRead()
{
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_12);
    if(in.status()==QDataStream::Ok)
    {
        for (; ; )
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
            QString str;
            QString action;
            in >> action >> str;
            qDebug() << action << str;

            if (action == "rotation")
                setCheckedRotationButton(str);
            else if (action == "radiation")
                setCheckedRadiationButton(str);
            else if (action == "Connect")
                sendToClients("radiation", radiationStatus, socket);
            break;
        }

    }
    else
    {
        qDebug() << "connection error!";
    }
}


void ServerWindow::sendToClients(QString action, QString str, QTcpSocket* socket1)
{
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << quint16(0) << action << str;
    out.device()->seek(0);
    out << quint16(data.size()-sizeof(quint16));
    if (socket1!=nullptr)
    {
        if(socket1->state() == QAbstractSocket::ConnectedState || socket1->state() == QAbstractSocket::ConnectingState)
            socket1->write(data);
    }
    else
    {
        for(auto& s: sockets){
            if(s->state() == QAbstractSocket::ConnectedState || s->state() == QAbstractSocket::ConnectingState)
                s->write(data);
        }
    }
}

void ServerWindow::connectionCounter()
{
    ui->connectionCounterButton->setText(QString::number(sockets.size()));
    ui->connectionCounterButton->setPalette(QPalette(sockets.size()==0?Qt::darkRed:Qt::darkGreen));

}


