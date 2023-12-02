#ifndef REMOTECAMERA_H
#define REMOTECAMERA_H

#include <QObject>
#include <QLabel>
#include <QPushButton>
#include "opencv2/opencv.hpp"
#include <QTimer>

class RemoteCamera:public QObject
{
    Q_OBJECT
public:
    RemoteCamera(QObject *parent = 0);
    ~RemoteCamera();

signals:
    void url(QString video_addr);
    void img(QImage image);

private:
    QLabel *videoLabel;
    cv::VideoCapture cap;
    QTimer *timer;
    QPushButton *startButton;
    QPushButton *stopButton;

private slots:
    void startCamera();
    void stopCamera();
    void updateFrame();
};

#endif // REMOTECAMERA_H
