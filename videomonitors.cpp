#include "videomonitors.h"
#include "ui_videomonitors.h"

VideoMonitors::VideoMonitors(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoMonitors)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog);//将监视器窗口设置为单独页面

}

VideoMonitors::~VideoMonitors()
{
    delete ui;
}

double VideoMonitors::getScaleFactor()
{
    return 0;
}

void VideoMonitors::updateFrame(cv::Mat frame)
{
    double scaleFactor = static_cast<double>(std::max(ui->label->width(), ui->label->height())) / std::max(frame.cols, frame.rows);

    // 调整图像大小
    cv::Mat resizedImage;
    cv::resize(frame, resizedImage, cv::Size(), scaleFactor, scaleFactor);

    // 将OpenCV图像转换为Qt图像
    QImage qImage(resizedImage.data, resizedImage.cols, resizedImage.rows, QImage::Format_BGR888);
    ui->label->setPixmap(QPixmap::fromImage(qImage));
}


void VideoMonitors::updateFrame(QImage image)
{
    image.scaled(ui->label->width(), ui->label->height());
    ui->label->setPixmap(QPixmap::fromImage(image));
}
