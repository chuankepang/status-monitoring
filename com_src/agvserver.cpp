#include "com_include/agvserver.h"
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QTime>

int CONNECT_MODE = 1; //0：无线通讯，1：有线ads通讯

AGVThread::AGVThread(QThread* trd,QObject *parent)
    : QObject(parent),deviceName("AGV"),trd(trd),state(Disconnected_S)
{
    auto_timer = new QTimer(this);
    if(CONNECT_MODE == 0){
        connect(this,&AGVThread::addr,this,&AGVThread::link);
        connect(auto_timer,&QTimer::timeout,this,&AGVThread::checkState);
    }
    else{
        connect(auto_timer,&QTimer::timeout,this,&AGVThread::readData);
    }
}

AGVThread::~AGVThread()
{
    auto_timer->stop();
    delete auto_timer;
    tcpSocket->close();
    delete  tcpSocket;
}

void AGVThread::initial()
{
    state = Disconnected_S;
    if(CONNECT_MODE == 0){
        tcpSocket = new QTcpSocket();
        emit this->addr(ip,port);
        connect(tcpSocket,&QTcpSocket::readyRead,this,&AGVThread::readData);
        connect(this,&AGVThread::ask,this,&AGVThread::writeData);
        if (!tcpSocket->waitForConnected(500)) {
            qDebug() << "connect " << ip << " : " << port << " error.";
            state = DeviceErr_S;
            return;
        }
        qDebug() << "connect " << ip << " : " << port << " succeed.";
        state = Connected_S;
        auto_timer->start(100);
    }
    else{
        if(AdsPortOpen() > 10)
            state = Connected_S;
        m_port = 349;
        QStringList qst = QString("5.69.185.74.1.1").split(".");
        for(int i = 0; i < qst.size();i++){
            m_netId.b[i] = static_cast<uchar>(qst.at(i).toInt());
        }
    //    *m_adsAddr = {{0,0,0,0,0,0},0};
        memcpy(&m_adsAddr.netId,&m_netId,sizeof(m_netId));
        memcpy(&m_adsAddr.port,&m_port,sizeof(m_port));
    //    qDebug() << m_adsAddr.netId.b << m_adsAddr.port;
        connect(this,&AGVThread::ask,this,&AGVThread::writeData);
        auto_timer->start(100);
    }
}

void AGVThread::link(QString IP, quint16 PORT)
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

void AGVThread::checkState()
{
    return;
}

void AGVThread::readData()
{
    if(CONNECT_MODE == 0){
        QByteArray Qba = tcpSocket->readAll();
        static QList<uint8_t> buffer;
        for(const auto &byte : Qba){
            if(buffer.empty() && byte == char(0x55)){
                buffer.append(byte);
            }
            else if (buffer.size() <= 2 && byte == char(0x00)) {
                buffer.append(byte);
            }
            else if(buffer.size() < 18 && buffer.size()>2){
                buffer.append(byte);
                if(buffer.size() == 18){
    //               process(buffer);
                   float v_x = (buffer[3]-128.0)/100.0;
                   float v_y = (buffer[3]-128.0)/100.0;
                   float v_z_r = (buffer[3]-128.0)/100.0;
                   qDebug() << "速度："<<v_x<< v_y<<v_z_r;
                   buffer.clear();
                }
            }
            else
                buffer.clear();
        }
    }
    else{
        char x_vel_var[] = "NC_Obj1 (Module1).CAR_CONTROL_Input.SpeedX_Fdb";
        char y_vel_var[] = "NC_Obj1 (Module1).CAR_CONTROL_Input.SpeedY_Fdb";
        char z_rotate_var[] = "NC_Obj1 (Module1).CAR_CONTROL_Input.SpeedW_Fdb";
        short x_vel = 0,y_vel = 0,z_rotate = 0;
        readADSData(x_vel,x_vel_var,sizeof (x_vel_var),m_adsAddr);
        readADSData(y_vel,y_vel_var,sizeof (y_vel_var),m_adsAddr);
        readADSData(z_rotate,z_rotate_var,sizeof (z_rotate_var),m_adsAddr);
//        qDebug() << int(z_rotate);
        QVector<float> vel_list;
        float vScale = 1.1938,rScale = 1.5454;
        vel_list << x_vel/128.f*vScale << y_vel/128.f*vScale << z_rotate/128.f*rScale;
        state = Busy_S;
        emit msg(vel_list);
    }
}

void AGVThread::writeData(QByteArray data)
{
    if(CONNECT_MODE == 0){
        if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
            tcpSocket->write(data);
            state = Busy_S;
        } else {
            qDebug() << deviceName <<" connect first.";
            state = DeviceErr_S;
        }
    }
    else{
        char x_vel_var[] = "NC_Obj1 (Module1).CAR_CONTROL_Output.SpeedX";
        char y_vel_var[] = "NC_Obj1 (Module1).CAR_CONTROL_Output.SpeedY";
        char z_rotate_var[] = "NC_Obj1 (Module1).CAR_CONTROL_Output.SpeedW";
        char EN_var[] = "NC_Obj1 (Module1).CAR_CONTROL_Output.EN_Car";
        char MODE_var[] = "NC_Obj1 (Module1).CAR_CONTROL_Output.Mode_Car";
        short x_vel = (static_cast<short>(data[0])&0xFF)-128,y_vel = (static_cast<short>(data[1])&0xFF)-128,z_rotate = (static_cast<short>(data[2])&0xFF)-128;
        char EN = data[3],MODE=data[4];

        if(x_vel > 15)
            x_vel = 15;
        else if(x_vel < -15)
            x_vel = -15;
        if(y_vel > 15)
            y_vel = 15;
        else if(y_vel < -15)
            y_vel = -15;
        if(z_rotate > 10)
            z_rotate = 10;
        else if(z_rotate < -10)
            z_rotate = -10;

//        qDebug() << x_vel << y_vel << z_rotate << int(data[0]) << int(data[1]) << int(data[2]);
        static ulong x_handle = getADSHandle(x_vel_var,sizeof(x_vel_var),m_adsAddr);
        static ulong y_handle = getADSHandle(y_vel_var,sizeof(y_vel_var),m_adsAddr);
        static ulong z_handle = getADSHandle(z_rotate_var,sizeof(z_rotate_var),m_adsAddr);
        static ulong EN_handle = getADSHandle(EN_var,sizeof(EN_var),m_adsAddr);
        static ulong MODE_handle = getADSHandle(MODE_var,sizeof(MODE_var),m_adsAddr);
        writeADSData(x_vel,x_vel_var,sizeof (x_vel_var),m_adsAddr,x_handle);
        writeADSData(y_vel,y_vel_var,sizeof (y_vel_var),m_adsAddr,y_handle);
        writeADSData(z_rotate,z_rotate_var,sizeof (z_rotate_var),m_adsAddr,z_handle);
        writeADSData(EN,EN_var,sizeof (EN_var),m_adsAddr,EN_handle);
        writeADSData(MODE,MODE_var,sizeof (MODE_var),m_adsAddr,MODE_handle);
//        qDebug() << QTime::currentTime().msecsSinceStartOfDay();
    }
}


//---------------------------------------------------------------------------

AGVServer::AGVServer(QObject *parent,QString ip, quint16 port): Devices(parent)
{
    this->ip = ip;
    this->port = port;
    this->change_DeviceType(0);
    this->change_device_name("移动小车");
    this->imgpath = tr(":/new/prefix1/image/agv.png");
    this->dof = 2;
    this->getDeviceSignals().clear();
    this->getDeviceSignals().insert("x_vel",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("y_vel",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("z_rotate",QVector<QPair<double,double>>());

    this->getDeviceSignals()["x_vel"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["y_vel"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["z_rotate"].append(QPair<double,double>(0,0));

    pose << 0 << 0 << 0 << 0 << 0 << 0;
}

AGVServer::~AGVServer(){
    QThread* &trd = threadHandler->trd;
    trd->quit();
}

void AGVServer::msgHandler(QVector<float> vel)
{
    double key(QTime::currentTime().msecsSinceStartOfDay()/1000.0);
    this->getDeviceSignals()["x_vel"].append(QPair<double,double>(key,vel[0]));
    this->getDeviceSignals()["y_vel"].append(QPair<double,double>(key,vel[1]));
    this->getDeviceSignals()["z_rotate"].append(QPair<double,double>(key,vel[2]));
}

void AGVServer::incomingConnection()
{
    QThread* trd = new QThread();
    threadHandler = new AGVThread(trd);
    threadHandler->moveToThread(trd);
    connect(trd,&QThread::started,threadHandler,&AGVThread::initial);
    connect(trd, SIGNAL(finished()), threadHandler, SLOT(deleteLater()));
    qRegisterMetaType<DeviceState>("DeviceState");
    connect(threadHandler, &AGVThread::msg,this,&AGVServer::msgHandler);
    qRegisterMetaType<QVector<float>>("QVector<float>");
    trd->start();
    if(CONNECT_MODE == 0)
        emit threadHandler->addr(ip,port);
}

int AGVServer::setDeviceVel(QVector<float> vel)
{
    if(CONNECT_MODE == 0){
        char x = vel[0]+1;
        char y = vel[1]+1;
        char z_rotate = vel[5]+1;
        QByteArray packet;

        // 添加帧头
        packet.append('\x55');
        packet.append('\xaa');
        packet.append('\x01');
        packet.append('\x01');
        packet.append('\x00');

        // 添加速度数据
        packet.append(x);
        packet.append(y);
        packet.append(z_rotate);

        // 添加0x00
        for (int i = 0; i < 9; ++i) {
            packet.append('\x00');
        }

        // 添加帧尾
        packet.append('\xAB');
        packet.append('\x56');
    //    qDebug() << packet;
        emit threadHandler->ask(packet);
        return 1;
    }
    else{

        char x = vel[0]+1;
        char y = vel[1]+1;
        char z_rotate = vel[5]+1;
        char EN = 1;
        char MODE = 1;
        QByteArray packet;
        packet.append(x);
        packet.append(y);
        packet.append(z_rotate);
        packet.append(EN);
        packet.append(MODE);
        emit threadHandler->ask(packet);
        return 1;
    }
}
