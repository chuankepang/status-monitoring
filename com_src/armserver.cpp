#include "com_include/armserver.h"
#include <QDebug>
#include <QTimer>
#include <QThread>

ArmThread::ArmThread(QThread* trd,QObject *parent)
    : QObject(parent),deviceName("AGV"),trd(trd),state(Disconnected_S)
{
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&ArmThread::readData);
}

ArmThread::~ArmThread()
{
    auto_timer->stop();
    delete auto_timer;
}

void ArmThread::initial()
{
    return;
}

void ArmThread::readData()
{
    return;
}

void ArmThread::writeData(bool LR, bool type, QVector<int> percent, int ms)
{
    return;
}


//---------------------------------------------------------------------------

ArmServer::ArmServer(QString type,QObject *parent): Devices(parent)
{
    this->change_DeviceType(0);
    this->change_device_name(tr("外骨骼%1臂").arg(type));
    this->imgpath = tr(":/new/prefix1/image/right_arm.png");
    this->dof = 7;
    this->getDeviceSignals().clear();
    this->getDeviceSignals().insert(tr("%1臂关节1").arg(type),QVector<QPair<double,double>>());
    this->getDeviceSignals().insert(tr("%1臂关节2").arg(type),QVector<QPair<double,double>>());
    this->getDeviceSignals().insert(tr("%1臂关节3").arg(type),QVector<QPair<double,double>>());
    this->getDeviceSignals().insert(tr("%1臂关节4").arg(type),QVector<QPair<double,double>>());
    this->getDeviceSignals().insert(tr("%1臂关节5").arg(type),QVector<QPair<double,double>>());

    pose << 0 << 0 << 0 << 0 << 0 << 0;
}

ArmServer::~ArmServer(){
    QThread* &trd = threadHandler->trd;
    trd->quit();
}

void ArmServer::msgHandler()
{
    return;
}

void ArmServer::incomingConnection()
{
    QThread* trd = new QThread();
    threadHandler = new ArmThread(trd);
    threadHandler->moveToThread(trd);
    connect(trd,&QThread::started,threadHandler,&ArmThread::initial);
    connect(trd, SIGNAL(finished()), threadHandler, SLOT(deleteLater()));
    connect(threadHandler, &ArmThread::msg,this,&ArmServer::msgHandler);
    trd->start();
}
