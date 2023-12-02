#ifndef OTHERSERVER_H
#define OTHERSERVER_H

#include <QObject>
#include <QPointer>
#include <QVector>
#include <QMetaType>
#include "devices.h"

#include <Windows.h>
#include <com_include/TcAdsDef.h>
#include <com_include/TcAdsAPI.h>
#include <QDebug>

class OtherThread : public QObject  //设备通信线程类
{
    Q_OBJECT
public:
    explicit OtherThread(QList<QString> var_name,QThread* trd,QObject *parent = nullptr);
    ~OtherThread();
    QString getName(){return deviceName;};
    DeviceState getState(){return state;};  //获得设备状态
    void setName(QString n){deviceName = n;};
    void setState(DeviceState s); //设置状态，用于启动读取
    void initial();
    QThread* trd;  //保存当前指向这个thread的指针

signals:
    void error(const QString &s);
    void ask(void *data2write, int dataSize, char* varName, int varSize);  //写设备数据的信号，参数根据具体设备改动
    void msg(QVector<double>); //主要的消息信号，参数根据具体设备改动，readData函数中发出，由Server的msgHandler进行处理

private:
    QString deviceName;//设备名称
    DeviceState state; //设备状态，需在函数中根据读取的状态赋值
    QTimer* auto_timer;//读取间隔定时器
    void readOtherDeviceData();

    template<typename T>
    void readData(T &data2read, char* varName, int varSize){
        long nErr;
        ulong varHander;
        nErr = AdsSyncReadWriteReq(&m_adsAddr,ADSIGRP_SYM_HNDBYNAME,0x0,sizeof(varHander),&varHander,varSize,varName);
        if (nErr){
            qDebug() << "Error: Handler Error:" << nErr;
        }
//        void* v_p = (void *)&data2read;
       nErr = AdsSyncReadReq(&m_adsAddr,ADSIGRP_SYM_VALBYHND,varHander,sizeof(data2read),&data2read);
       if (nErr){
           qDebug() << "Error: Read Error:" << nErr;
       }
    }


    template<typename T>
    void writeData(T &data2write, char* varName, int varSize){
        long nErr;
        ulong varHander;
        nErr = AdsSyncReadWriteReq(&m_adsAddr,ADSIGRP_SYM_HNDBYNAME,0x0,sizeof(varHander),&varHander,varSize,varName);
        if (nErr){
            qDebug() << "Error: Handler Error:" << nErr;
        }
        void* v_p = (void *)&data2write;
       nErr = AdsSyncWriteReq(&m_adsAddr,ADSIGRP_SYM_VALBYHND,varHander,sizeof(data2write),v_p);
       if (nErr){
           qDebug() << "Error: Write Error :" << nErr;
       }
    }

    AmsNetId m_netId;
    quint16 m_port;
    AmsAddr m_adsAddr;
    QList<QString> m_var_name;
};

class OtherServer: public Devices
{
    Q_OBJECT
public:
    OtherServer(QObject *parent = 0);
    ~OtherServer();
    void msgHandler(QVector<double> data_list); //数据处理函数，参数需和msg消息的参数一致
    QPointer<OtherThread> threadHandler;   //线程管理
    void incomingConnection();                //设备连接
    DeviceState getDeviceState(){return threadHandler->getState();}; //获取设备状态

    void setOtherDevice(QVector<QString> deviceName, QVector<QVector<QString>> deviceVar);

    QMap<QString, int> *getOtherDevice(){return &otherDevice;};
    QMap<QString, QVector<QString>> *getOtherDeviceVar(){return &otherDeviceVar;};
    QMap<QString, QVector<QPair<double,double>>> *getOtherDeviceVarVal(){return &otherDeviceVarVal;};
    QMap<QString, QVector<QPair<double,double>>> &getDeviceSignals() {return otherDeviceVarVal;};
protected:


private:
    QMap<QString, int> otherDevice;
    QMap<QString, QVector<QString>> otherDeviceVar;
    QMap<QString, QVector<QPair<double,double>>> otherDeviceVarVal;
};

#endif // OTHERSERVER_H
