#ifndef ARMSERVER_H
#define ARMSERVER_H

#include <QObject>
#include <QPointer>
#include <QVector>
#include <QMetaType>
#include "devices.h"


class ArmThread : public QObject
{
    Q_OBJECT
public:
    explicit ArmThread(QThread* trd,QObject *parent = nullptr);
    ~ArmThread();
    QString getName(){return deviceName;};
    DeviceState getState(){return state;};
    void setName(QString n){deviceName = n;};
    void setState(DeviceState s){state = s;};
    void initial();
    QThread* trd;
    bool quit;

signals:
    void error(const QString &s);
    void ask(bool LR, bool type, QVector<int> percent, int ms);
    void msg();

private:
    QString deviceName;
    bool gotLeft,gotRight;
    DeviceState state;
    QTimer* auto_timer;
    void readData();
    void writeData(bool LR, bool type, QVector<int> percent, int ms);
};

class ArmServer: public Devices
{
    Q_OBJECT
public:
    ArmServer(QString type,QObject *parent = 0);
    ~ArmServer();
    void msgHandler(); //message handler for msg parse
    QPointer<ArmThread> threadHandler;
    void incomingConnection();
    DeviceState getDeviceState(){return threadHandler->getState();};
protected:


private:
};

#endif // ARMSERVER_H
