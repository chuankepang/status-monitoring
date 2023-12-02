#ifndef CANSERVER_H
#define CANSERVER_H

#include <QThread>
#include <QPointer>
#include "ControlCAN.h"
#include <QDebug>

class CANThread:public QThread
{
    Q_OBJECT
public:
    CANThread(QObject *parent = 0);

    void stop();

    bool openCAN();

    void closeCAN();

    void sendData(int ID,unsigned char *ch);

    int deviceType;
    int debicIndex;
    int baundRate;
    int debicCom;

    bool stopped;

signals:
    void error(const QString &s);
    void msg(VCI_CAN_OBJ *vci,DWORD dwRel);
    void ask(QByteArray &qba);

private:
    void run();
    void sleep(unsigned int msec);

};

class CANServer: public QObject
{
    Q_OBJECT
public:
    CANServer(QObject *parent = 0);
    ~CANServer();
    void open();
    void close();
    void msgHandler(VCI_CAN_OBJ *vci,DWORD dwRel);

private:
    QPointer<CANThread> threadHandler;
};

#endif // CANSERVER_H
