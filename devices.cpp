#include "devices.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

Devices::Devices(QObject *parent) : QObject(parent),device_state(Disconnected_S),device_name("Unknown")
{
    disable();
    this->device_signals.insert("x", QVector<QPair<double, double>>());
    this->device_signals.insert("y", QVector<QPair<double, double>>());
    this->device_signals.insert("z", QVector<QPair<double, double>>());
    this->device_signals.insert("k", QVector<QPair<double, double>>());
    this->device_signals.insert("m", QVector<QPair<double, double>>());
    this->device_signals.insert("n", QVector<QPair<double, double>>());
}

/*
QString Devices::getDeviceName()//获取当前串口设备的名称
{
    ;//访问设备，获取当前串口设备的名称
}
*/

QMap<QString, QVector<QPair<double,double>>> &Devices::getDeviceSignals()
{
//  访问设备，获取设备的变量列表，在这里暂时写死
    return device_signals;
//    QVector<QTime> a;
}

const QString &Devices::getDeviceName()
{
    return device_name;//设备名
}

void Devices::change_device_name(QString new_name)//改变设备名称
{
    this->device_name=new_name;
}

bool Devices::get_DeviceType()
{
    return DeviceType;
}

void Devices::change_DeviceType(bool new_type)//改变设备类型
{
    this->DeviceType=new_type;
}

