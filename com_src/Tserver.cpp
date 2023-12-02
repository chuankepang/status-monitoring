#include "com_include/Tserver.h"
#include <QDebug>
#include <QTimer>
#include <QThread>

TemplateThread::TemplateThread(QThread* trd,QObject *parent)
    : QObject(parent),deviceName("Template"),trd(trd),state(Disconnected_S)
{
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&TemplateThread::readData);
}

TemplateThread::~TemplateThread()
{
    auto_timer->stop();
    delete auto_timer;
}

void TemplateThread::initial()
{
    return;
    //完成设备初始化和连接
}

void TemplateThread::readData()
{
    return;
    //完成设备的定时读取
}

void TemplateThread::writeData()
{
    return;
    //完成设备的数据写入，通过qt信号传入参数
}

void TemplateThread::setState(DeviceState s)
{
    if(s == Connected_S){
        state = s;
        auto_timer->stop();
    }
    else if(s == Busy_S){
        auto_timer->start(5);
        //设置每次读取数据的间隔时间
    }
}


//---------------------------------------------------------------------------

TemplateServer::TemplateServer(QObject *parent): Devices(parent)
{
    this->change_DeviceType(0);
    this->change_device_name("模板");   //设备命名
    this->imgpath = tr(":/new/prefix1/image/Template.png");   //图片文件位置
    this->dof = 3;  //自由度设置

    //需要读取的变量名称设置
    this->getDeviceSignals().clear();
    this->getDeviceSignals().insert("x",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("y",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("z",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("t",QVector<QPair<double,double>>());

    //变量初始化赋值
    this->getDeviceSignals()["x"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["y"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["z"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["t"].append(QPair<double,double>(0,0));

    pose << 0 << 0 << 0 << 0 << 0 << 0;
}

TemplateServer::~TemplateServer(){
    QThread* &trd = threadHandler->trd;
    trd->quit();
}

void TemplateServer::msgHandler(int x,int y,int z,int t)
{
    //数据处理函数，将TemplateThread传出来的信号数据，进行处理，并赋值到变量数组里面，包含时间戳
    double key(QTime::currentTime().msecsSinceStartOfDay()/1000.0);
    this->getDeviceSignals()["x"].append(QPair<double,double>(key,x));
    this->getDeviceSignals()["y"].append(QPair<double,double>(key,y));
    this->getDeviceSignals()["z"].append(QPair<double,double>(key,z));
    this->getDeviceSignals()["t"].append(QPair<double,double>(key,t));
    qDebug() << x << y << z << t;

}

void TemplateServer::incomingConnection()
{
    //连接函数，逻辑无需改动
    QThread* trd = new QThread();
    threadHandler = new TemplateThread(trd);
    threadHandler->moveToThread(trd);
    connect(trd,&QThread::started,threadHandler,&TemplateThread::initial);
    connect(trd, SIGNAL(finished()), threadHandler, SLOT(deleteLater()));
    connect(threadHandler, &TemplateThread::msg,this,&TemplateServer::msgHandler);
    qRegisterMetaType<DeviceState>("DeviceState");
    connect(this,&TemplateServer::cmd,threadHandler,&TemplateThread::setState);
    trd->start();
}
