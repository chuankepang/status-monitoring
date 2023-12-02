#include "com_include/otherserver.h"
#include <QDebug>
#include <QTimer>
#include <QThread>

std::map<QString,QString> WORDMAP = {
    {"距离1","NC_Obj1 (Module1).ADS.laser_distance[0]"},
    {"距离2","NC_Obj1 (Module1).ADS.laser_distance[1]"},
    {"右前距离","NC_Obj1 (Module1).ADS.UltraSonic_Data[0]"},
    {"左前距离","NC_Obj1 (Module1).ADS.UltraSonic_Data[1]"},
    {"右后距离","NC_Obj1 (Module1).ADS.UltraSonic_Data[2]"},
    {"左后距离","NC_Obj1 (Module1).ADS.UltraSonic_Data[3]"},
    {"右侧前距离","NC_Obj1 (Module1).ADS.UltraSonic_Data[4]"},
    {"右侧后距离","NC_Obj1 (Module1).ADS.UltraSonic_Data[5]"},
    {"左侧前距离","NC_Obj1 (Module1).ADS.UltraSonic_Data[6]"},
    {"左侧后距离","NC_Obj1 (Module1).ADS.UltraSonic_Data[7]"},
    {"俯仰角","NC_Obj1 (Module1).ADS.Gradienter[0]"},
    {"横滚角","NC_Obj1 (Module1).ADS.Gradienter[1]"},
    {"偏航角","NC_Obj1 (Module1).ADS.Gradienter[2]"},
    {"磁强计偏航角","NC_Obj1 (Module1).ADS.Magnetometer[0]"},
    {"X轴磁强","NC_Obj1 (Module1).ADS.Magnetometer[1]"},
    {"Y轴磁强","NC_Obj1 (Module1).ADS.Magnetometer[2]"},
    {"Z轴磁强","NC_Obj1 (Module1).ADS.Magnetometer[3]"},
    {"纬度","NC_Obj1 (Module1).ADS.GPS_data[0]"},
    {"经度","NC_Obj1 (Module1).ADS.GPS_data[1]"},
    {"对地速度","NC_Obj1 (Module1).ADS.GPS_data[2]"},
    {"对地航向","NC_Obj1 (Module1).ADS.GPS_data[3]"},
    {"纬度方向","NC_Obj1 (Module1).ADS.GPS_Dir[0]"},
    {"经度方向","NC_Obj1 (Module1).ADS.GPS_Dir[1]"},

    {"IMU_x轴角速度","NC_Obj1 (Module1).ADS.IMU_Data_Processed[0]"},
    {"IMU_y轴角速度","NC_Obj1 (Module1).ADS.IMU_Data_Processed[1]"},
    {"IMU_z轴角速度","NC_Obj1 (Module1).ADS.IMU_Data_Processed[2]"},
    {"IMU_x轴加速度","NC_Obj1 (Module1).ADS.IMU_Data_Processed[3]"},
    {"IMU_y轴加速度","NC_Obj1 (Module1).ADS.IMU_Data_Processed[4]"},
    {"IMU_z轴加速度","NC_Obj1 (Module1).ADS.IMU_Data_Processed[5]"},
    {"IMU_x轴磁力计","NC_Obj1 (Module1).ADS.IMU_Data_Processed[6]"},
    {"IMU_y轴磁力计","NC_Obj1 (Module1).ADS.IMU_Data_Processed[7]"},
    {"IMU_z轴磁力计","NC_Obj1 (Module1).ADS.IMU_Data_Processed[8]"},
    {"AHRS_横滚角速度","NC_Obj1 (Module1).ADS.IMU_Data_Processed[9]"},
    {"AHRS_俯仰角速度","NC_Obj1 (Module1).ADS.IMU_Data_Processed[10]"},
    {"AHRS_偏航角速度","NC_Obj1 (Module1).ADS.IMU_Data_Processed[11]"},
    {"AHRS_横滚角","NC_Obj1 (Module1).ADS.IMU_Data_Processed[12]"},
    {"AHRS_俯仰角","NC_Obj1 (Module1).ADS.IMU_Data_Processed[13]"},
    {"AHRS_偏航角","NC_Obj1 (Module1).ADS.IMU_Data_Processed[14]"},
    {"AHRS_Qw","NC_Obj1 (Module1).ADS.IMU_Data_Processed[15]"},
    {"AHRS_Qx","NC_Obj1 (Module1).ADS.IMU_Data_Processed[16]"},
    {"AHRS_Qy","NC_Obj1 (Module1).ADS.IMU_Data_Processed[17]"},
    {"AHRS_Qz","NC_Obj1 (Module1).ADS.IMU_Data_Processed[18]"},

};

std::map<QString,QString> TYPEMAP = {
    {"距离1","short"},
    {"距离2","short"},
    {"右前距离","short"},
    {"左前距离","short"},
    {"右后距离","short"},
    {"左后距离","short"},
    {"右侧前距离","short"},
    {"右侧后距离","short"},
    {"左侧前距离","short"},
    {"左侧后距离","short"},
    {"俯仰角","float"},
    {"横滚角","float"},
    {"偏航角","float"},
    {"X轴磁强","float"},
    {"Y轴磁强","float"},
    {"Z轴磁强","float"},
    {"磁强计偏航角","float"},
    {"纬度","float"},
    {"经度","float"},
    {"对地速度","float"},
    {"对地航向","float"},
    {"纬度方向","short"},
    {"经度方向","short"},

    {"IMU_x轴角速度","float"},
    {"IMU_y轴角速度","float"},
    {"IMU_z轴角速度","float"},
    {"IMU_x轴加速度","float"},
    {"IMU_y轴加速度","float"},
    {"IMU_z轴加速度","float"},
    {"IMU_x轴磁力计","float"},
    {"IMU_y轴磁力计","float"},
    {"IMU_z轴磁力计","float"},
    {"AHRS_横滚角速度","float"},
    {"AHRS_俯仰角速度","float"},
    {"AHRS_偏航角速度","float"},
    {"AHRS_横滚角","float"},
    {"AHRS_俯仰角","float"},
    {"AHRS_偏航角","float"},
    {"AHRS_Qw","float"},
    {"AHRS_Qx","float"},
    {"AHRS_Qy","float"},
    {"AHRS_Qz","float"},
};

OtherThread::OtherThread(QList<QString> var_name,QThread* trd,QObject *parent)
    : QObject(parent),deviceName("Other"),trd(trd),state(Disconnected_S)
{
    m_var_name = var_name;
    m_port = 349;
    QStringList qst = QString("5.69.185.74.1.1").split(".");
    for(int i = 0; i < qst.size();i++){
        m_netId.b[i] = static_cast<uchar>(qst.at(i).toInt());
    }
//    *m_adsAddr = {{0,0,0,0,0,0},0};
    memcpy(&m_adsAddr.netId,&m_netId,sizeof(m_netId));
    memcpy(&m_adsAddr.port,&m_port,sizeof(m_port));
    qDebug() << m_adsAddr.netId.b << m_adsAddr.port;
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&OtherThread::readOtherDeviceData);
}

OtherThread::~OtherThread()
{
    auto_timer->stop();
    delete auto_timer;
}

void OtherThread::initial()
{
    AdsPortOpen();
    return;
    //完成设备初始化和连接
}

void OtherThread::setState(DeviceState s)
{
    if(s == Connected_S){
        state = s;
        auto_timer->stop();
    }
    else if(s == Busy_S){
        auto_timer->start(100);
        //设置每次读取数据的间隔时间
    }
}

void OtherThread::readOtherDeviceData()
{
    QVector<double> data_list;
    data_list.clear();
    for(int i=0;i < m_var_name.size();i++){
        auto item = m_var_name[i];
        QString item_type = TYPEMAP[item];
        if(item_type ==  "short"){
            short data2read;
            QString item_word = WORDMAP[item];
            QByteArray item_b = item_word.toUtf8();
            readData(data2read,item_b.data(),item_b.size());
            data_list << static_cast<double>(data2read);
//            qDebug() << m_var_name[i] << ":" << data2read;
        }
        else if(item_type ==  "float"){
            float data2read;
            QString item_word = WORDMAP[item];
            QByteArray item_b = item_word.toUtf8();
            readData(data2read,item_b.data(),item_b.size());
            data_list << static_cast<double>(data2read);
//            qDebug() << m_var_name[i] << ":" << data2read;
        }

    }
    emit msg(data_list);
}


//---------------------------------------------------------------------------

OtherServer::OtherServer(QObject *parent): Devices(parent)
{
    this->change_DeviceType(0);
    this->change_device_name("其他设备");   //设备命名
    this->imgpath = tr(":/new/prefix1/image/Other.png");   //图片文件位置
    this->dof = 0;  //自由度设置

    pose << 0 << 0 << 0 << 0 << 0 << 0;

    QVector<QString> otherDeivces;
    //otherDeivces << "红外传感器";
    otherDeivces << "红外传感器" <<"超声传感器"<<"水平仪"<<"磁强计"<<"GPS"<<"IMU";
    QVector<QVector<QString>> otherDeivcesVal;
    otherDeivcesVal << QVector<QString>{"距离1","距离2"}
                    << QVector<QString>{"右前距离","左前距离","右后距离","左后距离","右侧前距离","右侧后距离","左侧前距离","左侧后距离"}
                    << QVector<QString>{"俯仰角","偏航角","横滚角"}
                    << QVector<QString>{"X轴磁强","Y轴磁强","Z轴磁强","磁强计偏航角"}
                    << QVector<QString>{"纬度","经度","对地速度","对地航向","纬度方向","经度方向"}
                    << QVector<QString>{"IMU_x轴角速度","IMU_y轴角速度","IMU_z轴角速度","IMU_x轴加速度","IMU_y轴加速度","IMU_z轴加速度","IMU_x轴磁力计","IMU_y轴磁力计","IMU_z轴磁力计",
                                        "AHRS_横滚角速度","AHRS_俯仰角速度","AHRS_偏航角速度","AHRS_横滚角","AHRS_俯仰角","AHRS_偏航角","AHRS_Qw","AHRS_Qx","AHRS_Qy","AHRS_Qz"};
    setOtherDevice(otherDeivces,otherDeivcesVal);
}

OtherServer::~OtherServer(){
    QThread* &trd = threadHandler->trd;
    trd->quit();
}

void OtherServer::msgHandler(QVector<double> data_list)
{
    //数据处理函数，将OtherThread传出来的信号数据，进行处理，并赋值到变量数组里面，包含时间戳
    double key(QTime::currentTime().msecsSinceStartOfDay()/1000.0);
    for(int i=0;i < otherDeviceVarVal.keys().size();i++){
        auto item = otherDeviceVarVal.keys()[i];
        this->getDeviceSignals()[item].append(QPair<double,double>(key,data_list[i]));
    }

}

void OtherServer::incomingConnection()
{
    //连接函数，逻辑无需改动
    QThread* trd = new QThread();
    threadHandler = new OtherThread(otherDeviceVarVal.keys(),trd);
    threadHandler->moveToThread(trd);
    connect(trd,&QThread::started,threadHandler,&OtherThread::initial);
    connect(trd, SIGNAL(finished()), threadHandler, SLOT(deleteLater()));
    qRegisterMetaType<DeviceState>("DeviceState");
    qRegisterMetaType<QVector<double>>("QVector<double>");
    connect(this,&OtherServer::cmd,threadHandler,&OtherThread::setState);
    connect(threadHandler,&OtherThread::msg,this,&OtherServer::msgHandler);
    trd->start();
}

void OtherServer::setOtherDevice(QVector<QString> deviceName, QVector<QVector<QString>> deviceVar)
{
    otherDeviceVarVal = this->getDeviceSignals();
    for(int i = 0; i< deviceName.size();i++){
        otherDevice.insert(deviceName[i],0);
        otherDeviceVar.insert(deviceName[i],deviceVar[i]);
        for(int j=0;j<deviceVar[i].size();j++){
            otherDeviceVarVal.insert(deviceVar[i][j],QVector<QPair<double,double>>());
        }
    }
    for(int i=0;i < otherDeviceVarVal.keys().size();i++){
        auto item = otherDeviceVarVal.keys()[i];
        this->getDeviceSignals()[item].append(QPair<double,double>(0,0));
    }
}
