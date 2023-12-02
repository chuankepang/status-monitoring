#include "com_include/tcpserver.h"
#include <stdlib.h>
#include <QDebug>

//thread
TcpThread::TcpThread(int socketDescriptor, QThread* trd, QObject *parent)
    : QObject(parent),deviceName("Unknown"), state(0),socketDescriptor(socketDescriptor),trd(trd)
{
}

void TcpThread::initial()
{
    tcpSocket = new QTcpSocket();
    quit = false;
    if (!tcpSocket->setSocketDescriptor(socketDescriptor)) {
        emit error(tcpSocket->error());
        this->quit = true;
        return;
    }

    ip = tcpSocket->peerAddress().toString();
    port = tcpSocket->peerPort();

    //TODO 判断是什么设备，并写到deviceName里

    connect(tcpSocket,&QTcpSocket::readyRead,this,&TcpThread::readData);

    connect(this,&TcpThread::ask,this,&TcpThread::sendData);

    //todo : get device type, write to name
    connect(tcpSocket,&QTcpSocket::disconnected,[=](){
        this->quit = true;
    });
}

void TcpThread::sendData(const QByteArray &qba)
{
    if(qba == QByteArray("q"))
        quit = true;
    else if(!quit)
        tcpSocket->write(qba);
}

void TcpThread::readData()
{
    if(!quit){
        QByteArray qba = tcpSocket->readAll();
        emit msg(qba,deviceName,this,ip,port);
    }
}

//---------------------------------------------------------

//server
TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
{
}

TcpServer::~TcpServer()
{
    update();
    for(auto &thread : threadHandler){
        QThread* &trd = thread->trd;
        trd->quit();
        delete thread;
    }
}

void TcpServer::msgHandler(QByteArray msg,QString deviceName,TcpThread* pointer,QString ip,qint32 port)
{
    qDebug() << "ip:" << ip << " port:" << port << " deviceName:" << deviceName << " msg:" << msg;
}

void TcpServer::update()
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

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    update();
    QThread* trd = new QThread();
    QPointer<TcpThread> thread = new TcpThread(socketDescriptor, trd);
    thread->moveToThread(trd);
    connect(trd,&QThread::started,thread,&TcpThread::initial);
    connect(trd, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, &TcpThread::msg,this,&TcpServer::msgHandler);
    trd->start();
    threadHandler.push_back(thread);
}
