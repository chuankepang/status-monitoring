#ifndef AGVSERVER_H
#define AGVSERVER_H

#include <QObject>
#include <QPointer>
#include <QVector>
#include <QMetaType>
#include "devices.h"
#include <QTcpSocket>
#include "com_include/adsserver.h"


class AGVThread : public QObject
{
    Q_OBJECT
public:
    explicit AGVThread(QThread* trd,QObject *parent = nullptr);
    ~AGVThread();
    QString getName(){return deviceName;};
    DeviceState getState(){return state;};
    void setName(QString n){deviceName = n;};
    void setState(DeviceState s);
    void initial();
    QThread* trd;

signals:
    void error(const QString &s);
    void ask(QByteArray data);
    void msg(QVector<float> vel);
    void addr(QString ip, quint16 port);

private:
    QString deviceName;
    DeviceState state;
    QTimer* auto_timer;
    QTcpSocket* tcpSocket = nullptr;
    QString ip;
    quint16 port;
    QByteArray tcpAllRecvData;
    AmsNetId m_netId;
    quint16 m_port;
    AmsAddr m_adsAddr;
    void readData();
    void writeData(QByteArray data);
    void checkState();
    void link(QString IP, quint16 PORT);
};

class AGVServer: public Devices
{
    Q_OBJECT
public:
    AGVServer(QObject *parent = 0,QString ip = tr("127.0.0.1"), quint16 port = 1234);
    ~AGVServer();
    void msgHandler(QVector<float> vel); //message handler for msg parse
    QPointer<AGVThread> threadHandler;
    void incomingConnection() override;
    DeviceState getDeviceState() override {return threadHandler->getState();};
    int setDeviceVel(QVector<float> vel) override;
protected:


private:
    QString ip;
    quint16 port;
    AmsNetId m_netId;
    quint16 m_port;
    AmsAddr m_adsAddr;
};

#endif // AGVSERVER_H
