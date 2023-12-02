#include "com_include/ur5server.h"
#include <QDebug>
#include <QTimer>
#include <QThread>

UR5Thread::UR5Thread(QThread* trd,QObject *parent)
    : QObject(parent),deviceName("UR5"),trd(trd),state(Disconnected_S),ip("127.0.0.1"),port(1234)
{
    connect(this,&UR5Thread::addr,this,&UR5Thread::link);
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&UR5Thread::checkState);
}

UR5Thread::~UR5Thread()
{
    auto_timer->stop();
    delete auto_timer;
    tcpSocket->close();
    delete  tcpSocket;
}

void UR5Thread::initial()
{
    state = Disconnected_S;
    tcpSocket = new QTcpSocket();
    emit this->addr(ip,port);
    connect(tcpSocket,&QTcpSocket::readyRead,this,&UR5Thread::readData);
    connect(this,&UR5Thread::ask,this,&UR5Thread::writeData);
    if (!tcpSocket->waitForConnected(500)) {
        qDebug() << "connect " << ip << " : " << port << " error.";
        state = DeviceErr_S;
        return;
    }
    qDebug() << "connect " << ip << " : " << port << " succeed.";
    state = Connected_S;
    auto_timer->start(100);
}

void UR5Thread::link(QString IP, quint16 PORT)
{
    if(tcpSocket){
        ip = IP;
        port = PORT;
        tcpSocket->connectToHost(IP,PORT);
        if (!tcpSocket->waitForConnected(500)) {
            state = DeviceErr_S;
            return;
        }
        state = Connected_S;
    }
    else{
        ip = IP;
        port = PORT;
    }
}

void UR5Thread::checkState()
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        return;
    }
    else if(tcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        state = Disconnected_S;
    }
    else {
        state = DeviceErr_S;
    }
}

void UR5Thread::writeData(QByteArray data)
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->write(data);
        state = Busy_S;
    } else {
//        qDebug() << "Please connect first.";
        state = DeviceErr_S;
    }
}

void UR5Thread::readData()
{
    QByteArray data = tcpSocket->readAll();

    tcpAllRecvData += QByteArray(data.data(), data.length());

    int allRecvDataLen = tcpAllRecvData.length();
    if (allRecvDataLen < 7) {
        return;
    }

    int endIndex = 0;
    for (int i = 0; i < allRecvDataLen - 7; ++i) {
        if (tcpAllRecvData[i + 0] == (char)0xFE && tcpAllRecvData[i + 1] == (char)0xFE) {
            uint16_t dataLen = tcpAllRecvData[i + 3] + tcpAllRecvData[i + 4] * 16;
            if ((i + dataLen + 8) <= allRecvDataLen
                && tcpAllRecvData[i + dataLen + 6] == (char)0xFF && tcpAllRecvData[i + dataLen + 7] == (char)0xFF) {

                QVector<float> jointAngle(6);
                QVector<float> forceAndTorque(6);

                char check = 0x00;
                const char* dataPtr = tcpAllRecvData.data();
                for (int m = 0; m < 6; ++m) {
                    jointAngle[m] = *(float*)&dataPtr[m*sizeof(float) + 5];
                    check += dataPtr[m*sizeof(float) + 5] + dataPtr[m*sizeof(float) + 6] + dataPtr[m*sizeof(float) + 7] + dataPtr[m*sizeof(float) + 8] + dataPtr[m*sizeof(float) + 9] + dataPtr[m*sizeof(float) + 10] + dataPtr[m*sizeof(float) + 11] + dataPtr[m*sizeof(float) + 12];
                }
                for (int m = 0; m < 6; ++m) {
                    forceAndTorque[m] = *(float*)&dataPtr[(6+m)*sizeof(float) + 5];
                    check += dataPtr[(6+m)*sizeof(float) + 5] + dataPtr[(6+m)*sizeof(float) + 6] + dataPtr[(6+m)*sizeof(float) + 7] + dataPtr[(6+m)*sizeof(float) + 8] + dataPtr[(6+m)*sizeof(float) + 9] + dataPtr[(6+m)*sizeof(float) + 10] + dataPtr[(6+m)*sizeof(float) + 11] + dataPtr[(6+m)*sizeof(float) + 12];
                }

                if (check == tcpAllRecvData[i + dataLen + 5]) {
                    if ((tcpAllRecvData[i + 2] == (char)0x02 || tcpAllRecvData[i + 2] == (char)0x01)) {
                        emit msg(jointAngle,forceAndTorque);
                    }
                    endIndex = i + dataLen + 8;
                }
            }
        }
    }

    if (tcpAllRecvData.length() >= endIndex) {
        tcpAllRecvData = tcpAllRecvData.mid(endIndex);
    }
}

void UR5Thread::setState(DeviceState s)
{
    if(s == Connected_S){
        state = s;
    }
    else if(s == Busy_S){
        state = s;
    }
}


//---------------------------------------------------------------------------

UR5Server::UR5Server(QObject *parent,QString ip, quint16 port): Devices(parent)
{
    this->ip = ip;
    this->port = port;
    this->change_DeviceType(0);
    this->change_device_name("UR机械臂");
    this->imgpath = tr(":/new/prefix1/image/ur5.png");
    this->dof = 6;
    this->getDeviceSignals().clear();
    this->getDeviceSignals().insert("关节1角度",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("关节2角度",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("关节3角度",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("关节4角度",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("关节5角度",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("关节6角度",QVector<QPair<double,double>>());

    pose << 0 << 0 << 0 << 0 << 0 << 0;
}

UR5Server::~UR5Server(){
    QThread* &trd = threadHandler->trd;
    trd->quit();
}

void UR5Server::msgHandler(QVector<float> joint,QVector<float> force)
{
    this->joint = joint;
    this->force = force;
    double key(QTime::currentTime().msecsSinceStartOfDay()/1000.0);
    auto tmp = this->getDeviceSignals().begin();
    for(int i = 0; i < this->getDeviceSignals().size();i++)
    {
        (tmp+i)->append(QPair<double,double>(key,joint[i]));//找到当前连接的输入设备列表中的使能设备，按ID号索引储存变量值的QVector，插入一个QPair，未来可改为按键值索引
    }
}

void UR5Server::incomingConnection()
{
    QThread* trd = new QThread();
    threadHandler = new UR5Thread(trd);
    threadHandler->moveToThread(trd);
    connect(trd,&QThread::started,threadHandler,&UR5Thread::initial);
    connect(trd, SIGNAL(finished()), threadHandler, SLOT(deleteLater()));
    connect(threadHandler, &UR5Thread::msg,this,&UR5Server::msgHandler);
    qRegisterMetaType<DeviceState>("DeviceState");
    connect(this,&UR5Server::cmd,threadHandler,&UR5Thread::setState);
    trd->start();
    emit threadHandler->addr(ip,port);
}

int UR5Server::setDevicePose(QVector<float> rpyt) {
    int kJointNum = 6;
    bool isEnableMasterControl = false;
    bool isGrasp = false;
    QVector<float> jointAngle = {0,0,0,0,0,0};
    float sendtRpy[6] = {rpyt[3], rpyt[4], rpyt[5], rpyt[0], rpyt[1], rpyt[2]};
    QByteArray data;
    uint16_t dataLen = (1 + kJointNum + 6) * 4;
    data.append((char)0xFE);
    data.append((char)0xFE);
    data.append((char)0x01);
    data.append((char)(dataLen & 0xff));
    data.append((char)(dataLen >> 8 & 0xff));
    char check = 0x00;
    uint16_t grasp = isGrasp;
    uint16_t mode = isEnableMasterControl;
    char *pGrasp = (char *)(&grasp);
    char *pMode = (char *)(&mode);
    for (int i = 0; i < sizeof(uint16_t); ++i) {
        data.append((char)pMode[i]);
        check += pMode[i];
    }
    for (int i = 0; i < sizeof(uint16_t); ++i) {
        data.append((char)pGrasp[i]);
        check += pMode[i];
    }
    for (int i = 0; i < kJointNum; ++i) {
        char *pAngle = (char *)(&jointAngle[i]);
        for (int j = 0; j < 4; ++j) {
            data.append((char)pAngle[j]);
            check += pAngle[j];
        }
    }
    for (int i = 0; i < 6; ++i) {
        char *pRpyt = (char *)(&sendtRpy[i]);
        for (int j = 0; j < 4; ++j) {
            data.append((char)pRpyt[j]);
            check += pRpyt[j];
        }
    }
    data.append(check);
    data.append((char)0xFF);
    data.append((char)0xFF);
    emit threadHandler->ask(data);
    return 1;
}
