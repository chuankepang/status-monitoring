#ifndef DEVICES_H
#define DEVICES_H


#include <QObject>
#include <QString>
#include <QMap>
#include <QTime>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "deviceconsole.h"
#define InputDevice false
#define OutputDevice true

enum DeviceState{
    Disconnected_S, //未连接，不清楚是否正常
    Connected_S,    //已连接过，设备和通讯正常
    DeviceErr_S,    //已连接，设备异常
    Busy_S        //设备通讯中
};

class Devices : public QObject
{
    Q_OBJECT
public:
    Devices(QObject *parent = nullptr);//构造函数
//    ~Devices();//析构函数
    QMap<QString, QVector<QPair<double,double>>> &getDeviceSignals();//获取当前设备的变量列表
    const QString &getDeviceName();//获取当前串口设备的名称
    void change_device_name(QString new_name);//改变设备名称
    void change_DeviceType(bool new_type);//改变设备类型
    bool get_DeviceType() ;//获取设备类型
    virtual DeviceState getDeviceState()=0;
    virtual void updateState(){return;};      //更新状态
    virtual void incomingConnection() {return;};  //连接设备
    virtual int setDevicePose(QVector<float> pose) {return -1;};  //设置末端位置
    virtual int setDeviceVel(QVector<float> vel) {return -1;};
    virtual int setDeviceJointAngle(QVector<float> joint) {return -1;}; //设置关节角
    virtual int setDeviceJointVel(QVector<float> jointVel) {return -1;};//设置关节速度
    virtual int setDeviceTorque(QVector<float> torque) {return -1;}; //设置扭矩
    virtual int setDeviceForce(QVector<float> force) {return -1;}; //设置力
    virtual int setDeviceCurrent(QVector<float> current) {return -1;};//设置电流
    QString imgpath = "";
    int dof = -1;
    QVector<float> pose;
    QVector<float> joint;
    QVector<float> force;
    QVector<float> torque;

signals:
    void error(const QString& errorMessage);//异常时发出信号
    void cmd(DeviceState s);
    void enable();
    void disable();

private:

    QString device_name;//设备名
    DeviceState device_state;
    bool if_enable=false;
    QMap<QString, QVector<QPair<double,double>>> device_signals;//变量列表

    bool DeviceType;//表示是输入设备还是输出设备

};


template <class T>
int setDeviceJointAngle(T* device, QVector<float> joint)
{
    return device->setDeviceJointAngle(joint);
}
template <class T>
int setDevicePose(T* device, QVector<float> pose)
{
    return device->setDevicePose(pose);
}
template <class T>
int setDeviceForce(T* device, QVector<float> force)
{
    return device->setDeviceForce(force);
}
template <class T>
int setDeviceTorque(T* device, QVector<float> torque)
{
    return device->setDeviceTorque(torque);
}

template <class T>
QString getDeviceName(T* device)
{
    return device->device_name;
}

template <class T>
int getDeviceDOF(T* device)
{
    return device->dof;
}
template <class T>
QVector<float> getDevicePose(T* device)
{
    return device->pose;
}

template <class T>
DeviceState getDeviceState(T* device)
{
    return device->device_state;
}

template <class T>
QVector<float> getDeviceJointAngle(T* device)
{
    return device->joint;
}


#endif // DEVICES_H
