#include "devicesscan.h"
#include "ui_devicesscan.h"
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include "indicator.h"

DevicesScan::DevicesScan(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DevicesScan)
{
    ui->setupUi(this);
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&DevicesScan::updateStatus);
    ui->exit->setDown(true);
    connect(ui->save,&QPushButton::clicked,[=](){
        QMessageBox::information(this,QString("设备扫描"),tr("已保存"),QMessageBox::Ok);
        ui->exit->setEnabled(true);
    });
    ui->light1->setStatus(IndicatorLabel::Status::Offline);
    ui->light2->setStatus(IndicatorLabel::Status::Error);
    ui->light3->setStatus(IndicatorLabel::Status::Ok);
    ui->light4->setStatus(IndicatorLabel::Status::Warning);
//    this->showNormal();
}

DevicesScan::~DevicesScan()
{
    delete ui;
    delete auto_timer;
}
/*
在UI设计界面，选择按钮“转到槽”，自动生成以下槽函数实现模板
void 类名::on_按钮名_信号()
{
}
*/
void DevicesScan::on_exit_clicked()
{
    this->auto_timer->stop();
    this->back();//产生一个自定义的信号，从当前页面返回
}

Ui::DevicesScan* DevicesScan::get_ui()
{
    return this->ui;
}

void DevicesScan::refresh_input_list(QVector<Devices*> const &connectInputDevices)//刷新输入设备列表
{
    //删除之前的控件排布
    qDebug()<<"刷新输入设备";
    QLayoutItem* item;
    for(auto i:input_rb)
        delete i;
    input_rb.clear();
    for(auto i:input_pb)
        delete i;
    input_pb.clear();
    while ((item = this->ui->gridLayout_input->takeAt(0)))
    {
        delete item->widget();
        delete item;
    }

    //生成新的控件排布
    for(int i=0;i<connectInputDevices.length();++i)
    {
        QLabel* label = new QLabel(connectInputDevices[i]->getDeviceName());
        QRadioButton* radioButton = new QRadioButton("使能");
        input_rb.append(radioButton);
        connectInputDevices[i]->incomingConnection();
        connect(radioButton, &QRadioButton::toggled, [=]() {
            if (radioButton->isChecked())
            {
                radioButton->setText(tr("已使能 "));
                qDebug()<<tr("设备 %1 被使能 ").arg(connectInputDevices[i]->getDeviceName());
                current_input_device=i;
            }
            else
            {
                radioButton->setText(tr("使能 "));
                qDebug()<<tr("设备 %1 被禁用 ").arg(connectInputDevices[i]->getDeviceName());
            }
        });
        IndicatorLabel* pushButton = new IndicatorLabel();
//        pushButton->setStatus(IndicatorLabel::Status::Ok);
        input_pb.append(pushButton);//将所有指示灯装入一个列表

        // 在 QGridLayout 中添加新的一行

        int row = this->ui->gridLayout_input->rowCount();
        this->ui->gridLayout_input->addWidget(label, row, 0);
        this->ui->gridLayout_input->addWidget(radioButton, row, 1);
        this->ui->gridLayout_input->addWidget(pushButton, row, 2);
    }
    QThread::sleep(3);
    for(int i=0;i<input_pb.length();++i){
        switch (connectInputDevices[i]->getDeviceState()) {
           case Disconnected_S: input_pb[i]->setStatus(IndicatorLabel::Status::Offline);break;
           case Connected_S:input_pb[i]->setStatus(IndicatorLabel::Status::Ok);break;
           case DeviceErr_S:input_pb[i]->setStatus(IndicatorLabel::Status::Error);break;
           case Busy_S:input_pb[i]->setStatus(IndicatorLabel::Status::Busy);break;
           default:break;
        }
    }
    inputDevice = connectInputDevices;
}
void DevicesScan::refresh_output_list(QVector<Devices*> const &connectOutputDevices)//刷新输出设备列表
{
    qDebug()<<"刷新输出设备";
    QLayoutItem* item;
    for(auto i:output_rb)
        delete i;
    output_rb.clear();
    for(auto i:output_pb)
        delete i;
    output_pb.clear();
    while ((item = this->ui->gridLayout_output->takeAt(0)))
    {
        delete item->widget();
        delete item;
    }
    for(int i=0;i<connectOutputDevices.length();++i)
    {
        QLabel* label = new QLabel(connectOutputDevices[i]->getDeviceName());
        QRadioButton* radioButton = new QRadioButton("使能");
        output_rb.append(radioButton);
        connectOutputDevices[i]->incomingConnection();
        connect(radioButton, &QRadioButton::toggled, [=](bool checked) {
            if (checked)
            {
                radioButton->setText("已使能 ");
                qDebug()<<tr("设备 %1 被使能 ").arg(connectOutputDevices[i]->getDeviceName());
                current_output_device=i;
            }
            else
            {
                radioButton->setText(tr("使能"));
                qDebug()<<tr("设备 %1 被禁用 ").arg(connectOutputDevices[i]->getDeviceName());
            }
        });
        IndicatorLabel* pushButton = new IndicatorLabel();
//        pushButton->setStatus(IndicatorLabel::Status::Ok);
        output_pb.append(pushButton);
        // 在 QGridLayout 中添加新的一行

        int row = this->ui->gridLayout_output->rowCount();
        this->ui->gridLayout_output->addWidget(label, row, 0);
        this->ui->gridLayout_output->addWidget(radioButton, row, 1);
        this->ui->gridLayout_output->addWidget(pushButton, row, 2);
    }
    QThread::sleep(1);
    for(int i=0;i<output_pb.length();++i){
        switch (connectOutputDevices[i]->getDeviceState()) {
           case Disconnected_S: output_pb[i]->setStatus(IndicatorLabel::Status::Offline);break;
           case Connected_S:output_pb[i]->setStatus(IndicatorLabel::Status::Ok);break;
           case DeviceErr_S:output_pb[i]->setStatus(IndicatorLabel::Status::Error);break;
           case Busy_S:output_pb[i]->setStatus(IndicatorLabel::Status::Busy);break;
           default:break;
        }
    }
    outputDevice = connectOutputDevices;
}

void DevicesScan::refresh_other_list(QMap<QString, int> *connectOtherDevices)
{
    qDebug()<<"刷新其他设备";
    QLayoutItem* item;
    for(auto i:other_rb)
        delete i;
    output_rb.clear();
    for(auto i:other_pb)
        delete i;
    output_pb.clear();
    while ((item = this->ui->gridLayout_output_3->takeAt(0)))
    {
        delete item->widget();
        delete item;
    }
    for(int i=0;i<connectOtherDevices->keys().size();++i)
    {
        QLabel* label = new QLabel(connectOtherDevices->keys()[i]);
        QCheckBox* radioButton = new QCheckBox("使能");
        other_rb.append(radioButton);
        connect(radioButton, &QCheckBox::stateChanged, [=](int state) {
            if (state)
            {
                radioButton->setText("已使能 ");
                qDebug()<<tr("其他设备 %1 被使能 ").arg(connectOtherDevices->keys()[i]);
                connectOtherDevices->insert(connectOtherDevices->keys()[i],state);
            }
            else
            {
                radioButton->setText(tr("使能"));
                qDebug()<<tr("设备 %1 被禁用 ").arg(connectOtherDevices->keys()[i]);
                connectOtherDevices->insert(connectOtherDevices->keys()[i],state);
            }
        });
        IndicatorLabel* pushButton = new IndicatorLabel();
        other_pb.append(pushButton);
        // 在 QGridLayout 中添加新的一行

        int row = this->ui->gridLayout_output_3->rowCount();
        this->ui->gridLayout_output_3->addWidget(label, row, 0);
        this->ui->gridLayout_output_3->addWidget(radioButton, row, 1);
        this->ui->gridLayout_output_3->addWidget(pushButton, row, 2);
    }

//    for(int i=0;i<output_pb.length();++i){
//        switch (connectOutputDevices[i]->getDeviceState()) {
//           case Disconnected_S: output_pb[i]->setStatus(IndicatorLabel::Status::Offline);break;
//           case Connected_S:output_pb[i]->setStatus(IndicatorLabel::Status::Ok);break;
//           case DeviceErr_S:output_pb[i]->setStatus(IndicatorLabel::Status::Error);break;
//           case Busy_S:output_pb[i]->setStatus(IndicatorLabel::Status::Busy);break;
//           default:break;
//        }
//    }

}

const int DevicesScan::get_current_input_device()
{
    return current_input_device;
}
const int DevicesScan::get_current_output_device()
{
    return current_output_device;
}

void DevicesScan::startTimer()
{
    auto_timer->start(1000);
}

void DevicesScan::updateStatus()
{
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowStaysOnTopHint);
    this->show();
    if(!input_pb.empty()){
        for(int i=0;i<input_pb.length();++i){
            switch (inputDevice[i]->getDeviceState()) {
               case Disconnected_S: input_pb[i]->setStatus(IndicatorLabel::Status::Offline);break;
               case Connected_S:input_pb[i]->setStatus(IndicatorLabel::Status::Ok);break;
               case DeviceErr_S:input_pb[i]->setStatus(IndicatorLabel::Status::Error);break;
               case Busy_S:input_pb[i]->setStatus(IndicatorLabel::Status::Busy);break;
               default:break;
            }
        }
    }
    if(!output_pb.empty()){
        for(int i=0;i<output_pb.length();++i){
            switch (outputDevice[i]->getDeviceState()) {
               case Disconnected_S: output_pb[i]->setStatus(IndicatorLabel::Status::Offline);break;
               case Connected_S:output_pb[i]->setStatus(IndicatorLabel::Status::Ok);break;
               case DeviceErr_S:output_pb[i]->setStatus(IndicatorLabel::Status::Error);break;
               case Busy_S:output_pb[i]->setStatus(IndicatorLabel::Status::Busy);break;
               default:break;
            }
        }
    }
}

void DevicesScan::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
