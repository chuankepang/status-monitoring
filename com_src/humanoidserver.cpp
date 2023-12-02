#include "com_include/humanoidserver.h"
#include <QDebug>
#include <QTimer>
#include <QThread>


#define kSlaveJointNum 14
const std::vector<float> kSlaveJointOffsets = {0., 0., 0., 0., 0., 0., 0.,0., 0., 0., 0., 0., 0., 0.};
const std::vector<float> kSlaveJointDir = {1, -1, 1, -1, 1, 1, 1,1, -1, 1, -1, 1, 1, 1};
struct QSlaveJointAngle {
    float jointAngle[kSlaveJointNum];
};

const QVector3D cameraNormalPos = QVector3D{2.0f, 2.0f, 4.0f};
const QVector3D cameraNormalTargetPos = QVector3D{-0.0f, 0.0f, 0.2f};
const float cameraNormalFov = 19.5;
const float cameraNormalAspectRatio = 1.7;

const QVector3D cameraTopViewPos = QVector3D{-0.12f, 0.185f, 5.0f};
const QVector3D cameraTopViewTargetPos = QVector3D{-0.12f, 0.185f, -0.015f};
const float cameraTopViewFov = 19.5;
const float cameraTopViewAspectRatio = 1.7;

HumanoidThread::HumanoidThread(QThread* trd,QObject *parent)
    : QObject(parent),deviceName("Humanoid"),trd(trd),state(Disconnected_S)
{
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&HumanoidThread::readData);
}

HumanoidThread::~HumanoidThread()
{
    auto_timer->stop();
    delete auto_timer;
}

void HumanoidThread::initial()
{
    return;
    //完成设备初始化和连接
}

void HumanoidThread::readData()
{
    return;
    //完成设备的定时读取
}

void HumanoidThread::writeData()
{
    return;
    //完成设备的数据写入，通过qt信号传入参数
}

void HumanoidThread::setState(DeviceState s)
{
    if(s == Connected_S){
        state = s;
        auto_timer->stop();
    }
    else if(s == Busy_S){
        auto_timer->start(5);
        //设置每次读取数据的间隔时间
    }
}


//---------------------------------------------------------------------------

HumanoidServer::HumanoidServer(MyOpenGLWidget* pOpenGLWidget,QObject *parent): Devices(parent)
{
    this->change_DeviceType(0);
    this->change_device_name("人形机器人");   //设备命名
    this->imgpath = tr(":/new/prefix1/image/Humanoid.png");   //图片文件位置
    this->dof = 3;  //自由度设置
    this->pOpenGLWidget = pOpenGLWidget;

    //需要读取的变量名称设置
    this->getDeviceSignals().clear();
    this->getDeviceSignals().insert("x",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("y",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("z",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("t",QVector<QPair<double,double>>());

    //变量初始化赋值
    this->getDeviceSignals()["x"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["y"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["z"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["t"].append(QPair<double,double>(0,0));

    pose << 0 << 0 << 0 << 0 << 0 << 0;
}

HumanoidServer::~HumanoidServer(){
}

void HumanoidServer::msgHandler(int x,int y,int z,int t)
{
    //数据处理函数，将HumanoidThread传出来的信号数据，进行处理，并赋值到变量数组里面，包含时间戳
    double key(QTime::currentTime().msecsSinceStartOfDay()/1000.0);
    this->getDeviceSignals()["x"].append(QPair<double,double>(key,x));
    this->getDeviceSignals()["y"].append(QPair<double,double>(key,y));
    this->getDeviceSignals()["z"].append(QPair<double,double>(key,z));
    this->getDeviceSignals()["t"].append(QPair<double,double>(key,t));
    qDebug() << x << y << z << t;

}

void HumanoidServer::incomingConnection()
{
//    //连接函数，逻辑无需改动
//    QThread* trd = new QThread();
//    threadHandler = new HumanoidThread(trd);
//    threadHandler->moveToThread(trd);
//    connect(trd,&QThread::started,threadHandler,&HumanoidThread::initial);
//    connect(trd, SIGNAL(finished()), threadHandler, SLOT(deleteLater()));
//    connect(threadHandler, &HumanoidThread::msg,this,&HumanoidServer::msgHandler);
//    qRegisterMetaType<DeviceState>("DeviceState");
//    connect(this,&HumanoidServer::cmd,threadHandler,&HumanoidThread::setState);
//    trd->start();
    pOpenGLWidget->getCamera().resetCamera(cameraNormalPos, cameraNormalTargetPos, cameraNormalFov, cameraNormalAspectRatio);
}

int HumanoidServer::setDeviceJointAngle(QVector<float> joint){
    pose.clear();
    pose = joint;
    for(int i = pose.size();i<kSlaveJointNum;i++){
        pose << 0.0;
    }
    if (pOpenGLWidget) {
        std::vector<float> jointAngleVector;
        // const float jointOffsets[7] = {-90., -90., -135., 0., 0., 0.};
        for (int i = 0; i < kSlaveJointNum; ++i) {
            jointAngleVector.push_back(kSlaveJointDir[i] * pose[i] + kSlaveJointOffsets[i]);
        }
//        qDebug()<<jointAngleVector;
        pOpenGLWidget->setSlaveJointAngles(jointAngleVector);
    }
    pOpenGLWidget->update();
    return 1;
}
