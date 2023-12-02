#ifndef DEVICESANALYSE_H
#define DEVICESANALYSE_H
#include <QObject>
#include "devices.h"
#include <QWidget>
#include <QStyleOption>
#include <QPainter>
#include <QKeyEvent>
enum ConvertMode{
    PosePosMode,
    PoseVelMode,
    JointMode
};

namespace Ui {
class DevicesAnalyse;
}

class DevicesAnalyse : public QWidget
{
    Q_OBJECT

public:
    explicit DevicesAnalyse(QWidget *parent = nullptr);
    ~DevicesAnalyse();
    void setDevice(Devices* inputDevice,Devices* outputDevice);
    ConvertMode getMode() {return mode;};
    void paintEvent(QPaintEvent *event) override;

private slots:
    void on_exit_clicked();//自动生成的槽函数声明
    void on_save_clicked();

signals:
    void back();//自定义信号的声明
    void deviceType();
    void stop();

private:
    Ui::DevicesAnalyse *ui;
    ConvertMode mode;
    void keyPressEvent(QKeyEvent *event) override {
        if(event->key() == Qt::Key_Return)
        {
            emit stop();
        }
    } ;
};

void convertData(Devices *inputDevice, Devices *outputDevice, double kScale, ConvertMode mode);
void convertDataPosePosMode(Devices *inputDevice, Devices *outputDevice, double kScale);
void convertDataPoseVelMode(Devices *inputDevice, Devices *outputDevice, double kScale);
void convertDataJointMode(Devices *inputDevice, Devices *outputDevice, double kScale);

#endif // DEVICESANALYSE_H
