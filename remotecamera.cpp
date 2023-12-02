#include "remotecamera.h"
#include <QDebug>

RemoteCamera::RemoteCamera(QObject *parent)
{

}

RemoteCamera::~RemoteCamera()
{

}

void RemoteCamera::startCamera()
{
    // 打开网络摄像头（需要根据实际情况修改地址）
    cap.open("http://admin:admin@10.134.5.73:8081/video");
    if (!cap.isOpened()) {
        qDebug() << "Failed to open camera.";
    }

    // 启动定时器
    timer->start(30);
}

void RemoteCamera::stopCamera()
{
    // 停止定时器
    timer->stop();

    // 关闭摄像头
    cap.release();
}

void RemoteCamera::updateFrame()
{
    // 获取视频帧
    cv::Mat frame;
    cap.read(frame);
    if (frame.empty()) {
        return;
    }

    // 转换为Qt图像格式并设置为标签控件的图像
    QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_BGR888);
    videoLabel->setPixmap(QPixmap::fromImage(image.rgbSwapped()));
}
