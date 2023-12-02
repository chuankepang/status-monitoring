#include "com_include/serialserver.h"

SerialThread::SerialThread(QString portName,QThread* trd,QObject *parent)
    :QObject(parent),portName(portName),deviceName("Unknown"),trd(trd)
{
}

//thread
void SerialThread::initial()
{
    serialport = new QSerialPort();
    quit = false;
    serialport->setPortName(portName);
    serialport->setBaudRate(115200);//串口波特率
    if (!serialport->open(QIODevice::ReadWrite)) {
        emit error(tr("Can't open %1, error code %2")
                   .arg(portName).arg(serialport->error()));
        quit = true;
        return;}

    //TODO 判断是什么设备，并写到deviceName里

    //数据发送
    connect(this,&SerialThread::ask,this,&SerialThread::sendData);

    //数据接收
    connect(serialport,&QSerialPort::readyRead,this,&SerialThread::readData);

    //断开链接判断
    connect(serialport,&QSerialPort::errorOccurred,[=](QSerialPort::SerialPortError err){
        qDebug()<< err;
        if(err == QSerialPort::PermissionError || err == QSerialPort::ResourceError){
            serialport->close();
            this->quit = true;}
    });

//    qDebug()<<"finish";
}

void SerialThread::sendData(const QByteArray &qba)
{
    if (qba == QByteArray("q")){
        serialport->close();
        quit = true;
    }
    else{
        if(!quit){
            serialport->write(qba);
            if(!serialport->waitForBytesWritten(50))
                emit error(tr("out of date"));
        }
    }
}

void SerialThread::readData()
{
    QByteArray qba = serialport->readAll();
    emit msg(qba,deviceName,this,portName);
}

//---------------------------------------------------------
//server

SerialServer::SerialServer(QObject *parent)
    :QObject(parent)
{
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&SerialServer::incomingConnection);
}

SerialServer::~SerialServer(){
    for(auto &thread : threadHandler){
        QThread* &trd = thread->trd;
        trd->quit();
        delete thread;
    }
}

void SerialServer::msgHandler(QByteArray msg, QString deviceName,SerialThread* pointer,QString portName)
{
    qDebug() << "port:" << portName << " deviceName:" << deviceName << " msg:" << msg;
}


void SerialServer::update()
{
    if(!threadHandler.empty())
    {
        for(int i = 0; i < threadHandler.size();i++)
        {
            if(threadHandler[i]->quit)
            {
                QThread* trd = threadHandler[i]->trd;
                threadHandler.erase(threadHandler.begin()+i);
                trd->quit();
                trd->deleteLater();
                trd->wait();
                delete trd;
                qDebug()<<"thread destroyed";
                i--;
            }
        }
    }
}

void SerialServer::incomingConnection()
{
    update();
    const auto infos = QSerialPortInfo::availablePorts();
    for(const auto &info : infos){
        bool new_flag = true;
        for(auto &thread : threadHandler)
        {
            if(thread->getPortName() == info.portName())
            {
                new_flag = false;
                break;
            }
        }
        if(new_flag == true){
            QThread* trd = new QThread();
            QPointer<SerialThread> thread = new SerialThread(info.portName(),trd);
            thread->moveToThread(trd);
            connect(trd,&QThread::started,thread,&SerialThread::initial);
            connect(trd,&QThread::finished,thread,&SerialThread::deleteLater);
            connect(thread,&SerialThread::msg,this,&SerialServer::msgHandler);
            trd->start();
            threadHandler.push_back(thread);
        }
    }
}



