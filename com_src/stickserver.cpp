#include "com_include/stickserver.h"
#include <QDebug>
#include <QTimer>
#include <QThread>
#define MAX_STR 255

StickThread::StickThread(QThread* trd,QObject *parent)
    : QObject(parent),deviceName("Stick"),trd(trd),state(Disconnected_S)
{
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&StickThread::readData);
}

StickThread::~StickThread()
{
    auto_timer->stop();
    delete auto_timer;
    if(!handle)
        hid_close(handle);
    hid_exit();
}

void StickThread::initial()
{
    int res;
    res = hid_init();
    if(res){
        qDebug() << "hid init error";
    }

    handle = hid_open(0x044f, 0xb10a, NULL);
    if (!handle){
        qDebug() << "hid open failed";
        state = Disconnected_S;
        return;
    }
    state = Connected_S;
}

void StickThread::readData()
{
    if(!handle){
        state = Disconnected_S;
        handle = hid_open(0x044f, 0xb10a, NULL);
        return;
    }
    else if(state == DeviceErr_S){
        handle = hid_open(0x044f, 0xb10a, NULL);
        return;
    }
    unsigned char buf[9] = {0};
    int res = hid_read(handle,buf,9);
    if(res < 0){
        state = DeviceErr_S;
        return;
    }
    state = Busy_S;
    int y = buf[4]*256+buf[3] - 8192;
    int x = -(buf[6]*256+buf[5] - 8192);
    int z = buf[7] - 128;
    int t = 255-buf[8];
//    qDebug() << x << y << z << t;
    emit msg(x,y,z,t);
}

void StickThread::writeData()
{
    return;
}

void StickThread::setState(DeviceState s)
{
    if(s == Connected_S){
        if(handle)
            state = s;
        auto_timer->stop();
    }
    else if(s == Busy_S){
        auto_timer->start(50);
    }
}

DeviceState StickThread::getState()
{
    handle = hid_open(0x044f, 0xb10a, NULL);
    if (!handle){
        state = Disconnected_S;
        return state;
    }
    else{
        if(state == Disconnected_S){
            state = Connected_S;
        }
        return state;
    }
}


//---------------------------------------------------------------------------

StickServer::StickServer(QObject *parent): Devices(parent)
{
    this->change_DeviceType(0);
    this->change_device_name("摇杆");
    this->imgpath = tr(":/new/prefix1/image/stick.png");
    this->dof = 3;
    this->getDeviceSignals().clear();
    this->getDeviceSignals().insert("x",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("y",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("z",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("t",QVector<QPair<double,double>>());

    this->getDeviceSignals()["x"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["y"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["z"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["t"].append(QPair<double,double>(0,0));

    pose << 0 << 0 << 0 << 0 << 0 << 0;
    joint << 0 << 0 << 0 << 0 << 0 << 0;
}

StickServer::~StickServer(){
    QThread* &trd = threadHandler->trd;
    trd->quit();
}

void StickServer::msgHandler(int x,int y,int z,int t)
{
    double key(QTime::currentTime().msecsSinceStartOfDay()/1000.0);
    this->getDeviceSignals()["x"].append(QPair<double,double>(key,x));
    this->getDeviceSignals()["y"].append(QPair<double,double>(key,y));
    this->getDeviceSignals()["z"].append(QPair<double,double>(key,z));
    this->getDeviceSignals()["t"].append(QPair<double,double>(key,t));
    pose.clear();
    pose << x*t/8200.0 << y*t/8200.0 << 0 << 0 << 0 << z*t/128.0;
    joint.clear();
    joint << x*t/8200.0 << y*t/8200.0 << 0 << 0 << 0 << z*t/128.0;
//    qDebug() <<pose;
//    qDebug() << x << y << z << t;

}

void StickServer::incomingConnection()
{
    QThread* trd = new QThread();
    threadHandler = new StickThread(trd);
    threadHandler->moveToThread(trd);
    connect(trd,&QThread::started,threadHandler,&StickThread::initial);
    connect(trd, SIGNAL(finished()), threadHandler, SLOT(deleteLater()));
    connect(threadHandler, &StickThread::msg,this,&StickServer::msgHandler);
    qRegisterMetaType<DeviceState>("DeviceState");
    connect(this,&StickServer::cmd,threadHandler,&StickThread::setState);
    trd->start();
}
