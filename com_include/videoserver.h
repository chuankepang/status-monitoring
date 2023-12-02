#ifndef VIDEOSERVER_H
#define VIDEOSERVER_H

#include <QObject>
#include <QPointer>
#include <QVector>
#include <QMetaType>
#include "devices.h"
#include <opencv2/opencv.hpp>

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QEventLoop>


class VideoThread : public QObject
{
    Q_OBJECT
public:
    explicit VideoThread(QThread* trd,QObject *parent = nullptr);
    ~VideoThread();
    void initial();
    void setUrl(QString Url);
    void startCamera();
    void stopCamera();
    void changeCameraState(bool state);
    QThread* trd;
    bool quit;

signals:
    void error(const QString &s);
    void state(bool s);
    void url(QString u);
    void img(cv::Mat image);
    void scale(int maxSize);

private:
    QTimer* auto_timer;
    int maxSize;
    cv::VideoCapture* cap;
    QString video_url;
    void updateFrame();
};

class VideoServer: public Devices
{
    Q_OBJECT
public:
    VideoServer(QObject *parent = 0);
    ~VideoServer();
    QPointer<VideoThread> threadHandler;
    void incomingConnection();
    DeviceState getDeviceState() {return Connected_S;};
protected:


private:
};

QString getPlayUrl(QWidget* parent);

#endif // VIDEOSERVER_H
