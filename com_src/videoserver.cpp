#include "com_include/videoserver.h"
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QMessageBox>
#include "ui_monitors.h"

VideoThread::VideoThread(QThread* trd,QObject *parent)
    : QObject(parent),trd(trd),video_url(""),maxSize(500)
{
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&VideoThread::updateFrame);
    connect(this,&VideoThread::scale,[=](int maxSize){
        this->maxSize = maxSize;
    });
}

VideoThread::~VideoThread()
{
    auto_timer->stop();
    delete auto_timer;
}


void VideoThread::initial()
{
    connect(this,&VideoThread::url,this,&VideoThread::setUrl);
    connect(this,&VideoThread::state,this,&VideoThread::changeCameraState);
    cap = new cv::VideoCapture();
}

void VideoThread::setUrl(QString Url)
{
    video_url = Url;
    if(cap->isOpened()){
        cap->release();
    }
}

void VideoThread::startCamera()
{
    cap->open(video_url.toStdString());
    if(!cap->isOpened()){
        qDebug() << "open camera failed";
        return;
    }
    auto_timer->start(30);
}

void VideoThread::stopCamera()
{
    auto_timer->stop();
    cap->release();
}

void VideoThread::changeCameraState(bool state)
{
    if(state == false){
        stopCamera();
    }
    else{
        startCamera();
    }
}

void VideoThread::updateFrame()
{
    cv::Mat frame;
    cap->read(frame);

    if (frame.empty()) {
        return;
    }
//    cv::Mat resizedImage;
//    double scaleFactor = static_cast<double>(maxSize/std::max(frame.cols,frame.rows));
//    cv::resize(frame,resizedImage,cv::Size(),scaleFactor,scaleFactor);
    // 记录上一次更新图片的时间
    static qint64 lastUpdateTime = 0;

    // 获取当前时间
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    // 计算与上一次更新图片的时间差
    qint64 elapsedTime = currentTime - lastUpdateTime;

    // 如果时间间隔大于等于35ms，则发送当前图片并记录上一次更新图片的时间
    if (elapsedTime >= 35) {
        emit img(frame);
        lastUpdateTime = currentTime;
    }
}

//---------------------------------------------------------------------------

VideoServer::VideoServer(QObject *parent): Devices(parent)
{
}

VideoServer::~VideoServer(){
    QThread* &trd = threadHandler->trd;
    trd->quit();
}


void VideoServer::incomingConnection()
{
    QThread* trd = new QThread();
    threadHandler = new VideoThread(trd);
    threadHandler->moveToThread(trd);
    connect(trd,&QThread::started,threadHandler,&VideoThread::initial);
    connect(trd, SIGNAL(finished()), threadHandler, SLOT(deleteLater()));
    qRegisterMetaType<cv::Mat>("cv::Mat");
    trd->start();
}

QString getPlayUrl(QWidget* parent) {
    QString appKey = "3819e72f710445ed8c32c996ef1c098c";
    QString secret = "2046cabedb2d4c51deb8618e1cad0fad";
    QString deviceSerial = "246487845";
    int channelNo = 1;

    QUrl tokenUrl("https://open.ys7.com/api/lapp/token/get");
    QUrl playUrl("https://open.ys7.com/api/lapp/v2/live/address/get");

    // 参数检测
    if (appKey.isEmpty() || secret.isEmpty()) {
        qDebug() << "Please set appkey and secret first.";
        return QString();
    }

    // 请求 App Token
    QNetworkAccessManager manager;
    QUrlQuery tokenParam;
    tokenParam.addQueryItem("appKey", appKey);
    tokenParam.addQueryItem("appSecret", secret);

    QNetworkRequest tokenRequest(tokenUrl);
    tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *tokenReply = manager.post(tokenRequest, tokenParam.toString(QUrl::FullyEncoded).toUtf8());
    QEventLoop loop1;
    QObject::connect(tokenReply, &QNetworkReply::finished, &loop1, &QEventLoop::quit);
    loop1.exec();

    QString accessToken;

    if (tokenReply->error() == QNetworkReply::NoError) {
        QByteArray rawData = tokenReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(rawData);
        accessToken = doc["data"].toObject()["accessToken"].toString();
        qDebug() << "get token succeed";
        qDebug() << doc;
    } else {
        qDebug() << "get token error: " << tokenReply->errorString();
    }

    tokenReply->deleteLater();

    // 请求通道播放地址
    QUrlQuery playParam;
    playParam.addQueryItem("accessToken", accessToken);
    playParam.addQueryItem("deviceSerial", deviceSerial);
    playParam.addQueryItem("channelNo", QString::number(channelNo));
    playParam.addQueryItem("protocol", "3");
    playParam.addQueryItem("quality", "1");

    QNetworkRequest playRequest(playUrl);
    playRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *playReply = manager.post(playRequest, playParam.toString(QUrl::FullyEncoded).toUtf8());
    QEventLoop loop2;
    QObject::connect(playReply, &QNetworkReply::finished, &loop2, &QEventLoop::quit);
    loop2.exec();

    QString Url;

    if (playReply->error() == QNetworkReply::NoError) {
        QByteArray rawData = playReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(rawData);
        qDebug() << doc;
        Url = doc["data"].toObject()["url"].toString();
        if(Url == QString(""))
        {
            QMessageBox::warning(parent,QString("视频流"),doc["msg"].toString(),QMessageBox::Ok);
        }
        qDebug() << "get url succeed";
    } else {
        qDebug() << "get url error: " << playReply->errorString();
    }

    playReply->deleteLater();

    return Url;
}
