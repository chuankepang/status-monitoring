#ifndef ADSSERVER_H
#define ADSSERVER_H

#include <QObject>
#include <QThread>
#include <QPointer>
#include <Windows.h>
#include <com_include/TcAdsDef.h>
#include <com_include/TcAdsAPI.h>

#include <QDebug>
#include <QTime>

//inline void qdebugVarName(char* varName,int varSize){
//    QString output;
//    for(size_t i; i< varSize; i++){
//        output.append(varName[i]);
//    }
//    qDebug()<<"var name:"<<output;
//}

class ADSThread : public QObject
{
    Q_OBJECT

public:
    explicit ADSThread(QString netId, quint16 port,QThread* trd=nullptr,QObject *parent = nullptr);
    ~ADSThread();
    bool m_isConnected;
    QThread* trd;
    template<typename T>
    void readData(T &data2read, char* varName, int varSize){
        long nErr;
        ulong varHander;
        nErr = AdsSyncReadWriteReq(&m_adsAddr,ADSIGRP_SYM_HNDBYNAME,0x0,sizeof(varHander),&varHander,varSize,varName);
        if (nErr){
            qDebug() << "Error: Handler Error:" << nErr;
            emit error(nErr);
        }
//        void* v_p = (void *)&data2read;
       nErr = AdsSyncReadReq(&m_adsAddr,ADSIGRP_SYM_VALBYHND,varHander,sizeof(data2read),&data2read);
       if (nErr){
           qDebug() << "Error: Read Error:" << nErr;
           emit error(nErr);
       }
    }


    template<typename T>
    void writeData(T &data2write, char* varName, int varSize){
        long nErr;
        ulong varHander;
        nErr = AdsSyncReadWriteReq(&m_adsAddr,ADSIGRP_SYM_HNDBYNAME,0x0,sizeof(varHander),&varHander,varSize,varName);
        if (nErr){
            qDebug() << "Error: Handler Error:" << nErr;
            emit error(nErr);
        }
        void* v_p = (void *)&data2write;
       nErr = AdsSyncWriteReq(&m_adsAddr,ADSIGRP_SYM_VALBYHND,varHander,sizeof(data2write),v_p);
       if (nErr){
           qDebug() << "Error: Write Error :" << nErr;
           emit error(nErr);
       }
    }


signals:
    void error(long errCode);

public slots:
    void connectToPlc();
    void disconnectFromPlc();

private:
    AmsNetId m_netId;
    quint16 m_port;
    AmsAddr m_adsAddr;
    QMap<QString, int> otherDevice;
    QMap<QString, QVector<QString>> otherDeviceVar;
    QMap<QString, QVector<QPair<double,double>>> otherDeviceVarVal;
};

extern ulong getADSHandle(char* varName, int varSize,AmsAddr m_adsAddr);


template<typename T>
void readADSData(T &data2read, char* varName, int varSize,AmsAddr m_adsAddr){
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
void writeADSData(T &data2write, char* varName, int varSize, AmsAddr m_adsAddr, ulong varHander){
    long nErr;
   nErr = AdsSyncWriteReq(&m_adsAddr,ADSIGRP_SYM_VALBYHND,varHander,sizeof(data2write),&data2write);
   if (nErr){
       qDebug() << "Error: Write Error :" << nErr;
   }
}


//class ADSServer : public QObject
//{
//    Q_OBJECT
//public:
//    ADSServer(QObject *parent = 0);
//    ~ADSServer();
//    void open();
//    void close();

//private:
//    QPointer<ADSThread> threadHandler;
//};

#endif // ADSSERVER_H
