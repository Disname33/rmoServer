#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QPushButton>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWindow; }
QT_END_NAMESPACE

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private:
    Ui::ServerWindow *ui;
    QTcpServer *server;
    QTcpSocket *socket;
    QVector <QTcpSocket*> sockets;
    QByteArray data;
    QTimer *timer;
    float beamLineAngle;
    quint16 nextBlockSize;
    QString rotationStatus;
    QString radiationStatus;
    void sendToClients(QString, QString, QTcpSocket* socket1 = nullptr);
    void connectionCounter();
    void setCheckedRotationButton(QString);
    void setCheckedRadiationButton(QString);

private slots:
    void slotClientDisconnected();
    void slotNewConnection();
    void slotReadyRead();

    void on_buttonRotationStop_clicked();
    void on_buttonRotation3rpm_clicked();
    void on_buttonRotation6rpm_clicked();
    void on_buttonRadiationOff_clicked();
    void on_buttonRadiation50_clicked();
    void on_buttonRadiation100_clicked();

    void timerSlot();


};
#endif // SERVERWINDOW_H
