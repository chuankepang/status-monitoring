#include "com_include/adsserver.h"

ADSThread::ADSThread(QString netId, quint16 port,QThread* trd,QObject *parent)
    :trd(trd),m_port(port)
{
    QStringList qst = netId.split(".");
    for(int i = 0; i < qst.size();i++){
        m_netId.b[i] = static_cast<uchar>(qst.at(i).toInt());
    }
//    *m_adsAddr = {{0,0,0,0,0,0},0};
    memcpy(&m_adsAddr.netId,&m_netId,sizeof(m_netId));
    memcpy(&m_adsAddr.port,&m_port,sizeof(m_port));
    qDebug() << m_adsAddr.netId.b << m_adsAddr.port;
}


ADSThread::~ADSThread(){
    disconnectFromPlc();
}

void ADSThread::connectToPlc(){
    long nErr = AdsPortOpen();
    if (nErr){
        qDebug() << "Error: AdsPortOpen: " << nErr ;
        emit error(nErr);
    }
    m_isConnected = true;
}

void ADSThread::disconnectFromPlc(){
    long nErr = AdsPortClose();
    if (nErr){
        qDebug() << "Error: AdsPortClose: " << nErr ;
        emit error(nErr);
        return;
    }
    m_isConnected = false;
}

ulong getADSHandle(char* varName, int varSize,AmsAddr m_adsAddr){
    long nErr;
    ulong varHander;
    nErr = AdsSyncReadWriteReq(&m_adsAddr,ADSIGRP_SYM_HNDBYNAME,0x0,sizeof(varHander),&varHander,varSize,varName);
    if (nErr){
        qDebug() << "Error: Handler Error:" << nErr;
    }
    return varHander;
}

