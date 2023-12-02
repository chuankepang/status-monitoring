#ifndef MOUSESERVER_H
#define MOUSESERVER_H

#include <QObject>
#include <QPointer>
#include <QVector>
#include <QMetaType>
#include "devices.h"


class MouseThread : public QObject
{
    Q_OBJECT
public:
    explicit MouseThread(QThread* trd,QObject *parent = nullptr);
    ~MouseThread();
    QString getName(){return deviceName;};
    DeviceState getState(){return state;};
    void setName(QString n){deviceName = n;};
    void setState(DeviceState s);
    void initial();
    QThread* trd;

signals:
    void error(const QString &s);
    void ask();
    void msg(int x,int y);

private:
    QString deviceName;
    DeviceState state;
    QTimer* auto_timer;
    QCursor* mouse;
    void readData();
    void writeData();
};

class MouseServer: public Devices
{
    Q_OBJECT
public:
    MouseServer(QObject *parent = 0);
    ~MouseServer();
    void msgHandler(int x,int y); //message handler for msg parse
    QPointer<MouseThread> threadHandler;
    void incomingConnection();
    DeviceState getDeviceState(){return threadHandler->getState();};
protected:


private:
};

#endif // MOUSESERVER_H
