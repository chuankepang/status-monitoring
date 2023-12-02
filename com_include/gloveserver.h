#ifndef GLOVESERVER_H
#define GLOVESERVER_H

#include <QObject>
#include <QPointer>
#include <QVector>
#include <QMetaType>
#include "mainwindow.h"
#include "devices.h"


class GloveThread : public QObject
{
    Q_OBJECT
public:
    explicit GloveThread(QThread* trd,QObject *parent = nullptr);
    ~GloveThread();
    QString getName(){return deviceName;};
    DeviceState getState(){return state;};
    void setName(QString n){deviceName = n;};
    void setState(DeviceState s);
    void initial();
    QThread* trd;
    bool quit;

signals:
    void error(const QString &s);
    void ask(bool LR, bool type, QVector<int> percent, int ms);
    void flexion(bool LR,QVector<float> flexions);
    void pose(bool LR);

private:
    QString deviceName;
    bool gotLeft,gotRight;
    DeviceState state;
    QTimer* auto_timer;
    void readData();
    void writeData(bool LR, bool type, QVector<int> percent, int ms);
};

class GloveServer: public Devices
{
    Q_OBJECT
public:
    GloveServer(QObject *parent = 0);
    ~GloveServer();
    void flexionHandler(bool LR, QVector<float> flexions); //message handler for msg parse
    QPointer<GloveThread> threadHandler;
    void incomingConnection();
    DeviceState getDeviceState(){return threadHandler->getState();};
protected:


private:
};

#endif // GLOVESERVER_H
