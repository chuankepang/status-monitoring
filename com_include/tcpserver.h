#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <vector>
#include <QPointer>
#include <QThread>


//thread，单个tcp的通讯线程，可以通过ask信号发送，返回msg信号
class TcpThread : public QObject
{
    Q_OBJECT

public:
    TcpThread(int socketDescriptor, QThread* trd, QObject *parent = 0);
    QString getName(){return deviceName;};
    int getState(){return state;};
    void setName(QString n){deviceName = n;};
    void setState(int s){state = s;};
    QString getIp(){return ip;};
    qint32 getPort(){return port;};
    void initial();
    QThread* trd;
    bool quit;

signals:
    void error(QTcpSocket::SocketError socketError);
    void msg(QByteArray Qba, QString deviceName,TcpThread* pointer,QString ip, qint32 port);
    void ask(QByteArray Qba);

protected:

private:
    int socketDescriptor;
    QString deviceName;
    int state;
    QString ip;
    qint32 port;
    QTcpSocket* tcpSocket;
    void readData();
    void sendData(const QByteArray &qba);
};

//server，处理各个线程的开启和销毁
class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    TcpServer(QObject *parent = 0);
    ~TcpServer();
    void msgHandler(QByteArray msg, QString deviceName,TcpThread* pointer, QString ip, qint32 port); //message handler for msg parse
    std::vector<QPointer<TcpThread>> threadHandler;

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    void update();
};

#endif // TCPSERVER_H
