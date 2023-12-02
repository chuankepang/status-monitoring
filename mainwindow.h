#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include "devicesscan.h"
#include "devicesanalyse.h"
#include "orderconvert.h"
#include "setting.h"
#include "devices.h"
#include "QCheckBox"
#include "QLabel"
#include "QTimer"
#include <QFile>
#include <QTextStream>
#include "monitors.h"
#include "mycheckbox.h"
#include "fliter.h"
#include "videomonitors.h"
#include "com_include/gloveserver.h"
#include "com_include/armserver.h"
#include "com_include/agvserver.h"
#include "com_include/mouseserver.h"
#include "com_include/ur5server.h"
#include "com_include/stickserver.h"
#include "com_include/videoserver.h"
#include "com_include/adsserver.h"
#include "com_include/wishboneserver.h"
#include "com_include/humanoidserver.h"
#include "com_include/otherserver.h"
#include "opencv2/opencv.hpp"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void init_mainwindow();//初始化主窗口
    ~MainWindow();

signals:
    void refresh_input_list(QVector<Devices*> connectInputDevices);

private slots:
    /*
    在UI设计界面，选择按钮“转到槽”，自动生成以下槽函数声明
    void 类名::on_按钮名_信号()
    */

    void on_devicesScan_triggered();

    void on_devicesAnalyse_triggered();

    void on_orderConvert_triggered();

    void on_settingSerial_triggered();
    //MainWindow类的自定义槽函数的实现
    void when_devicesscan_back();
    void when_devicesanalyse_back();
    void when_orderconvert_back();
    void when_setting_back();

    void on_CloseProgramButton_clicked();
    void create_devices_list();//自定义的槽函数，相应设备扫描页面发出的设备扫描信号，创建当前的输入输出设备列表
    void refresh_mainwindow();//根据选中的输出输出设备，刷新主页面

    void updateFrame(cv::Mat frame);  //更新图像
    void startCamera();  //开启相机
    void stopCamera();   //关闭相机

    void on_input_device_clicked();

    void on_output_device_clicked();

    void realtimeDataSlot();

    void stateCheck();
    void sysStop();

    void on_pause_clicked();
    void on_save_clicked();

    void on_stop_clicked();

    void customContextMenuRequested(const QPoint& pos);//实现变量名的修改

    void on_getURL_clicked();

    void on_test_clicked();

    virtual void mousePressEvent(QMouseEvent * event) override;
    virtual void mouseMoveEvent(QMouseEvent * event) override;
    virtual void mouseReleaseEvent(QMouseEvent * event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    Ui::MainWindow *ui;
    //指向子页面的指针声明，以指针+new的方式创建在堆区
    DevicesScan *ds;
    DevicesAnalyse *da;
    OrderConvert *oc;
    Setting *s;
    QVector<Monitors*> monitors;//变量显示窗口（单独的）的列表，输入输出信号都包含，先是输入，再是输出
    VideoServer *vs;
    VideoMonitors *vm;
    OtherServer *os;

    QVector<Devices*> connectInputDevices;//连接到当前系统的输入设备
    QVector<Devices*> connectOutputDevices;//连接到当前系统的输出设备
    QVector<MyCheckBox*> input_checkbox;//控制输入信号图像是否显示的复选框
    QVector<MyCheckBox*> output_checkbox;//控制输出信号图像是否显示的复选框
    QVector<MyCheckBox*> other_checkbox;//其他设备信号图像是否显示的复选框
    QVector<QLabel*> input_label;//显示输入信号值的label的集合
    QVector<QLabel*> output_label;//显示输出信号值的label的集合
    QVector<QLabel*> other_label;//显示其他信号值的label的集合
    QMap<QString,int> other_label_id_map;
    void draw();//生成线条
    QTimer dataTimer;
    QTimer stateTimer;
    cv::VideoCapture cap;
    int input_signal_num;
    int output_signal_num;
    int other_signal_num;
    QVector<MovingAverageFilter*> fliters;//对变量进行滤波的滤波器们
    bool isDragging;
    QPoint mouse_startPoint;
    QPoint window_top_left_point;

};
#endif // MAINWINDOW_H
