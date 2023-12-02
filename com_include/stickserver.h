#ifndef STICKSERVER_H
#define STICKSERVER_H

#include <QObject>
#include <QPointer>
#include <QVector>
#include <QMetaType>
#include "devices.h"
#include "hidapi.h"


class StickThread : public QObject
{
    Q_OBJECT
public:
    explicit StickThread(QThread* trd,QObject *parent = nullptr);
    ~StickThread();
    QString getName(){return deviceName;};
    DeviceState getState();
    void setName(QString n){deviceName = n;};
    void setState(DeviceState s);
    void initial();
    QThread* trd;

signals:
    void error(const QString &s);
    void ask();
    void msg(int x,int y,int z,int t);

private:
    QString deviceName;
    DeviceState state;
    hid_device *handle;
    QTimer* auto_timer;
    void readData();
    void writeData();
};

class StickServer: public Devices
{
    Q_OBJECT
public:
    StickServer(QObject *parent = 0);
    ~StickServer();
    void msgHandler(int x,int y,int z,int t); //message handler for msg parse
    QPointer<StickThread> threadHandler;
    void incomingConnection();
    DeviceState getDeviceState(){return threadHandler->getState();};
protected:


private:
};

#endif // STICKSERVER_H
