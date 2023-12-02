#ifndef DEVICESSCAN_H
#define DEVICESSCAN_H


#include <QWidget>
#include <QGridLayout>
#include "devices.h"
#include "QVector"
#include "QRadioButton"
#include "QPushButton"
#include "indicator.h"
#include <QStyleOption>
#include <QPainter>
#include <QTimer>
#include <QKeyEvent>
#include <QCheckBox>
namespace Ui {
class DevicesScan;
}

class DevicesScan : public QWidget
{
    Q_OBJECT

public:
    explicit DevicesScan(QWidget *parent = nullptr);
    ~DevicesScan();
    Ui::DevicesScan *get_ui();
    void refresh_input_list(QVector<Devices*> const &connectInputDevices);//刷新输入设备列表
    void refresh_output_list(QVector<Devices*> const &connectOutputDevices);//刷新输入设备列表
    void refresh_other_list(QMap<QString, int> *connectOtherDevices);
    const int get_current_input_device();//当前选定的输入设备
    const int get_current_output_device();//当前选定的输出设备
    void paintEvent(QPaintEvent *event) override;
    void startTimer();

private slots:
    void on_exit_clicked();//自动生成的槽函数声明

signals:
    void back();//自定义信号的声明
    void stop();

private:
    Ui::DevicesScan *ui;
    QVector<QRadioButton*> input_rb;
    QVector<IndicatorLabel*> input_pb;
    QVector<QRadioButton*> output_rb;
    QVector<IndicatorLabel*> output_pb;
    QVector<Devices*> inputDevice;
    QVector<Devices*> outputDevice;

    QVector<QCheckBox*> other_rb;
    QVector<IndicatorLabel*> other_pb;
    QMap<QString,int>* otherDevice;

    QTimer* auto_timer;
    int current_input_device=-1;
    int current_output_device=-1;
    void updateStatus();
    void keyPressEvent(QKeyEvent *event) override {
        if(event->key() == Qt::Key_Return)
        {
            emit stop();
        }
    } ;

};

#endif // DEVICESSCAN_H
