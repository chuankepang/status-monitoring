#ifndef SERIALSERVER_H
#define SERIALSERVER_H

#include <QString>
#include <QThread>
#include <QtSerialPort>
#include <vector>
#include <QPointer>
#include <QDebug>

//thread，单个串口通讯线程，可以通过ask信号发送，返回msg信号
class SerialThread : public QObject
{
    Q_OBJECT

public:
    SerialThread(QString portName,QThread* trd,QObject *parent = 0);
    QString getName(){return deviceName;};
    int getState(){return state;};
    QString getPortName(){return portName;};
    void setName(QString n){deviceName = n;};
    void setState(int s){state = s;};
    void initial();
    QThread* trd;
    bool quit;

signals:
    void error(const QString &s);
    void msg(const QByteArray &Qba, QString deviceName ,SerialThread* pointer,QString portName);
    void ask(const QByteArray &Qba);

protected:

private:
    QString portName;
    QString deviceName;
    QSerialPort* serialport;
    int state;
    void sendData(const QByteArray &qba);
    void readData();
};


//server，自动连接串口，处理各个线程的开启和销毁
class SerialServer: public QObject
{
    Q_OBJECT
public:
    SerialServer(QObject *parent = 0);
    ~SerialServer();
    QTimer* auto_timer;
    void msgHandler(QByteArray msg, QString deviceName ,SerialThread* pointer, QString portName); //message handler for msg parse
    std::vector<QPointer<SerialThread>> threadHandler;

protected:
    void incomingConnection();

private:
    void update();
};

#endif // SERIALSERVER_H
