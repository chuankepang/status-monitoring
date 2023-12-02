#ifndef VIDEOMONITORS_H
#define VIDEOMONITORS_H

#include <QWidget>
#include <opencv2/opencv.hpp>

namespace Ui {
class VideoMonitors;
}

class VideoMonitors : public QWidget
{
    Q_OBJECT

public:
    explicit VideoMonitors(QWidget *parent = nullptr);
    ~VideoMonitors();
    void updateFrame(cv::Mat frame);
    void updateFrame(QImage image);
    double getScaleFactor();


private:
    Ui::VideoMonitors *ui;
};

#endif // VIDEOMONITORS_H
