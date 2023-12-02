#ifndef UR5SERVER_H
#define UR5SERVER_H

#include <QObject>
#include <QPointer>
#include <QVector>
#include <QMetaType>
#include "devices.h"
#include <QTcpSocket>


class UR5Thread : public QObject
{
    Q_OBJECT
public:
    explicit UR5Thread(QThread* trd,QObject *parent = nullptr);
    ~UR5Thread();
    QString getName(){return deviceName;};
    DeviceState getState(){return state;};
    void setName(QString n){deviceName = n;};
    void setState(DeviceState s);
    void initial();
    QThread* trd;

signals:
    void error(const QString &s);
    void ask(QByteArray data);
    void msg(QVector<float> joint,QVector<float> force);
    void addr(QString ip, quint16 port);

private:
    QString deviceName;
    DeviceState state;
    QTimer* auto_timer;
    QTcpSocket* tcpSocket = nullptr;
    QString ip;
    quint16 port;
    QByteArray tcpAllRecvData;
    void readData();
    void writeData(QByteArray data);
    void checkState();
    void link(QString IP, quint16 PORT);
};

class UR5Server: public Devices
{
    Q_OBJECT
public:
    UR5Server(QObject *parent = 0,QString ip = tr("127.0.0.1"), quint16 port = 1234);
    ~UR5Server();
    void msgHandler(QVector<float> joint,QVector<float> force); //message handler for msg parse
    QPointer<UR5Thread> threadHandler;
    void incomingConnection();
    DeviceState getDeviceState(){return threadHandler->getState();};
    int setDevicePose(QVector<float> rpyt);
protected:


private:
    QString ip;
    quint16 port;
};

#endif // UR5SERVER_H
