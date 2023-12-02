#ifndef WISHBONESERVER_H
#define WISHBONESERVER_H

#include <QObject>
#include <QPointer>
#include <QVector>
#include <QMetaType>
#include "devices.h"
#include "GL_include/myopenglwidget.h"

class WishboneThread : public QObject  //设备通信线程类
{
    Q_OBJECT
public:
    explicit WishboneThread(QThread* trd,QObject *parent = nullptr);
    ~WishboneThread();
    QString getName(){return deviceName;};
    DeviceState getState(){return state;};  //获得设备状态
    void setName(QString n){deviceName = n;};
    void setState(DeviceState s); //设置状态，用于启动读取
    void initial();
    QThread* trd;  //保存当前指向这个thread的指针

signals:
    void error(const QString &s);
    void ask();  //写设备数据的信号，参数根据具体设备改动
    void msg(int x,int y,int z,int t); //主要的消息信号，参数根据具体设备改动，readData函数中发出，由Server的msgHandler进行处理

private:
    QString deviceName;//设备名称
    DeviceState state; //设备状态，需在函数中根据读取的状态赋值
    QTimer* auto_timer;//读取间隔定时器
    void readData();   //读取设备数据
    void writeData();  //写入设备数据
};

class WishboneServer: public Devices   //设备类
{
    Q_OBJECT
public:
    WishboneServer(MyOpenGLWidget* pOpenGLWidget,QObject *parent = 0);
    ~WishboneServer();
    void msgHandler(int x,int y,int z,int t); //数据处理函数，参数需和msg消息的参数一致
//    QPointer<WishboneThread> threadHandler;   //线程管理
    void incomingConnection();                //设备连接
    DeviceState getDeviceState(){return DeviceState::Connected_S;}; //获取设备状态
    int setDeviceJointAngle(QVector<float> joint) override;
protected:


private:
    MyOpenGLWidget* pOpenGLWidget;
};


#endif // WISHBONESERVER_H
