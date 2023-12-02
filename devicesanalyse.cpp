#include "devicesanalyse.h"
#include "ui_devicesanalyse.h"
#include <QMessageBox>
#include <QDebug>


DevicesAnalyse::DevicesAnalyse(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DevicesAnalyse),mode(PosePosMode)
{
    ui->setupUi(this);
}

DevicesAnalyse::~DevicesAnalyse()
{
    delete ui;
}

void DevicesAnalyse::setDevice(Devices* inputDevice,Devices* outputDevice)
{
    QString pathIn = inputDevice->imgpath;
    QString pathOut = outputDevice->imgpath;

    int dofIn = inputDevice->dof;
    int dofOut = outputDevice->dof;
    ui->freedom_input->setText(tr("自由度：%1").arg(dofIn));
    ui->freedom_output->setText(tr("自由度：%1").arg(dofOut));

    QImage imgIn(pathIn);
    QImage imgOut(pathOut);
    QPixmap pixmapIn = QPixmap::fromImage(imgIn.scaled(ui->image_input->size(), Qt::KeepAspectRatio));
    QPixmap pixmapOut = QPixmap::fromImage(imgOut.scaled(ui->image_output->size(), Qt::KeepAspectRatio));
    ui->image_input->setPixmap(pixmapIn);
    ui->image_output->setPixmap(pixmapOut);

    ui->comboBox->clear();
    if(outputDevice->getDeviceName() == QString("移动小车")){
        ui->comboBox->addItem(tr("位姿映射-速度模式"));
    }
    else if (outputDevice->getDeviceName() == QString("UR机械臂")){
        ui->comboBox->addItem(tr("位姿映射-位置模式"));
        ui->comboBox->addItem(tr("位姿映射-速度模式"));
        ui->comboBox->addItem(tr("关节映射"));
    }
    else if (outputDevice->getDeviceName() == QString("双叉臂")){
        ui->comboBox->addItem(tr("关节映射"));
    }
    else if (outputDevice->getDeviceName() == QString("人形机器人")){
        ui->comboBox->addItem(tr("关节映射"));
    }
    QString currentMode = ui->comboBox->currentText();
    if(currentMode == tr("位姿映射-位置模式"))
        mode = PosePosMode;
    else if(currentMode == tr("位姿映射-速度模式"))
        mode = PoseVelMode;
    else if(currentMode == tr("关节映射"))
        mode = JointMode;
    else{
        QMessageBox::warning(this,QString("设备分析"),tr("未实现，请重新选择"),QMessageBox::Ok);
        return;
    }
}

void DevicesAnalyse::on_exit_clicked()
{
    QString currentMode = ui->comboBox->currentText();
    if(currentMode == tr("位姿映射-位置模式"))
        mode = PosePosMode;
    else if(currentMode == tr("位姿映射-速度模式"))
        mode = PoseVelMode;
    else if(currentMode == tr("关节映射"))
        mode = JointMode;
    else{
        QMessageBox::warning(this,QString("设备分析"),tr("未实现，请重新选择"),QMessageBox::Ok);
        return;
    }
    this->back();//产生一个自定义的信号，从当前页面返回
}

void convertData(Devices *inputDevice, Devices *outputDevice, double kScale, ConvertMode mode)
{
    switch (mode) {
    case PosePosMode: convertDataPosePosMode(inputDevice,outputDevice,kScale); break;
    case PoseVelMode: convertDataPoseVelMode(inputDevice,outputDevice,kScale); break;
    case JointMode: convertDataJointMode(inputDevice,outputDevice,kScale); break;
    default: break;
    }
}

void convertDataPosePosMode(Devices *inputDevice, Devices *outputDevice,double kScale)
{
    QVector<float> input_pose = getDevicePose(inputDevice);
    for(auto &item:input_pose){
        item = kScale * item;
    }
    setDevicePose(outputDevice,input_pose);
}
void convertDataPoseVelMode(Devices *inputDevice, Devices *outputDevice,double kScale)
{
    if(inputDevice->getDeviceName() == QString("摇杆") && outputDevice->getDeviceName() == QString("移动小车"))
    {
        QVector<float> input_pose = getDevicePose(inputDevice);
        for(auto &item:input_pose){
            item *= kScale;
            item += 255;
            item /= 2;
        }
//        qDebug() << input_pose;
        outputDevice->setDeviceVel(input_pose);
    }
//    QVector<float> input_pose = getDevicePose(inputDevice);
//    QVector<float> output_pose = getDevicePose(outputDevice);
//    for(int i = 0; i < output_pose.size();i++){
//        output_pose[i] += input_pose[i] * kScale;
//    }
//    setDevicePose(outputDevice,output_pose);
}
//TODO: 获取输入设备的位置或者关节，对输出设备进行设置，位置模式直接设置输出设备的位置或增量位置，关节映射要具体设备具体写函数
void convertDataJointMode(Devices *inputDevice, Devices *outputDevice,double kScale)
{
    QVector<float> input_joint = getDeviceJointAngle(inputDevice);
//    if(inputDevice->getDeviceName() == QString("摇杆") && outputDevice->getDeviceName() == QString("双叉臂")){
//        for(auto &item:input_joint){
//            item /= 255;
//            item *= 180;
//            item *= kScale;
//        }
//        qDebug() << input_joint;
//        outputDevice->setDeviceJointAngle(input_joint);
//    }
    if(inputDevice->getDeviceName() == QString("摇杆"))
    {
        for(auto &item:input_joint){
            item /= 255;
        }
    }
    if(outputDevice->getDeviceName() == QString("双叉臂") || outputDevice->getDeviceName() == QString("人形机器人")){
        for(auto &item:input_joint){
            item *= 180;
            item *= kScale;
        }
        outputDevice->setDeviceJointAngle(input_joint);
    }
    return;
}


void DevicesAnalyse::on_save_clicked()
{
    QString currentMode = ui->comboBox->currentText();
    if(currentMode == tr("位姿映射-位置模式"))
        mode = PosePosMode;
    else if(currentMode == tr("位姿映射-速度模式"))
        mode = PoseVelMode;
    else if(currentMode == tr("关节映射"))
        mode = JointMode;
    else{
        QMessageBox::warning(this,QString("设备分析"),tr("未实现，请重新选择"),QMessageBox::Ok);
        return;
    }
    QMessageBox::information(this,QString("设备分析"),tr("保存成功"),QMessageBox::Ok);
}

void DevicesAnalyse::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
