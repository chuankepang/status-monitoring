#include "com_include/mouseserver.h"
#include <QDebug>
#include <QTimer>
#include <QThread>

MouseThread::MouseThread(QThread* trd,QObject *parent)
    : QObject(parent),deviceName("Mouse"),trd(trd),state(Disconnected_S)
{
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&MouseThread::readData);
}

MouseThread::~MouseThread()
{
    auto_timer->stop();
    delete auto_timer;
}

void MouseThread::initial()
{
    mouse = new QCursor();
    state = Connected_S;
    return;
}

void MouseThread::readData()
{
    QPoint cursorPos = QCursor::pos();
    state = Busy_S;
    emit msg(cursorPos.x(),cursorPos.y());
    return;
}

void MouseThread::writeData()
{
    return;
}

void MouseThread::setState(DeviceState s)
{
    if(s == Connected_S){
        state = s;
        auto_timer->stop();
    }
    else if(s == Busy_S){
        auto_timer->start(50);
    }
}

//---------------------------------------------------------------------------

MouseServer::MouseServer(QObject *parent): Devices(parent)
{
    this->change_DeviceType(0);
    this->change_device_name("鼠标");
    this->imgpath = tr(":/new/prefix1/image/mouse.png");
    this->dof = 2;
    this->getDeviceSignals().clear();
    this->getDeviceSignals().insert("X",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("Y",QVector<QPair<double,double>>());

    this->getDeviceSignals()["X"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["Y"].append(QPair<double,double>(0,0));

    pose << 0 << 0 << 0 << 0 << 0 << 0;
}

MouseServer::~MouseServer(){
    QThread* &trd = threadHandler->trd;
    trd->quit();
}

void MouseServer::msgHandler(int x,int y)
{
    double key(QTime::currentTime().msecsSinceStartOfDay()/1000.0);
    auto tmp = this->getDeviceSignals().begin();
    tmp->append(QPair<double,double>(key,x));
    (tmp+1)->append(QPair<double,double>(key,y));
//    qDebug() << x << y;
}

void MouseServer::incomingConnection()
{
    QThread* trd = new QThread();
    threadHandler = new MouseThread(trd);
    threadHandler->moveToThread(trd);
    connect(trd,&QThread::started,threadHandler,&MouseThread::initial);
    connect(trd, SIGNAL(finished()), threadHandler, SLOT(deleteLater()));
    connect(threadHandler, &MouseThread::msg,this,&MouseServer::msgHandler);
    qRegisterMetaType<DeviceState>("DeviceState");
    connect(this,&MouseServer::cmd,threadHandler,&MouseThread::setState);
    trd->start();
}
