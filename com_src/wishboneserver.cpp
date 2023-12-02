#include "com_include/wishboneserver.h"
#include <QDebug>
#include <QTimer>
#include <QThread>

#define kSlaveJointNum 15
const std::vector<float> kSlaveJointOffsets = {0., 0., 0., 0., 0., 0., 0.,0., 0., 0., 0., 0., 0., 0.,0.};
const std::vector<float> kSlaveJointDir = {1, -1, 1, -1, 1, 1, 1,1, -1, 1, -1, 1, 1, 1,1};

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

WishboneThread::WishboneThread(QThread* trd,QObject *parent)
    : QObject(parent),deviceName("Wishbone"),trd(trd),state(Disconnected_S)
{
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&WishboneThread::readData);
}

WishboneThread::~WishboneThread()
{
    auto_timer->stop();
    delete auto_timer;
}

void WishboneThread::initial()
{
    return;
    //完成设备初始化和连接
}

void WishboneThread::readData()
{
    return;
    //完成设备的定时读取
}

void WishboneThread::writeData()
{
    return;
    //完成设备的数据写入，通过qt信号传入参数
}

void WishboneThread::setState(DeviceState s)
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

WishboneServer::WishboneServer(MyOpenGLWidget* pOpenGLWidget,QObject *parent): Devices(parent)
{
    this->pOpenGLWidget = pOpenGLWidget;
    this->change_DeviceType(0);
    this->change_device_name("双叉臂");   //设备命名
    this->imgpath = tr(":/new/prefix1/image/Wishbone.png");   //图片文件位置
    this->dof = 6;  //自由度设置

    this->getDeviceSignals().clear();
    this->getDeviceSignals().insert("关节1角度",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("关节2角度",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("关节3角度",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("关节4角度",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("关节5角度",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("关节6角度",QVector<QPair<double,double>>());


    //变量初始化赋值
    this->getDeviceSignals()["关节1角度"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["关节2角度"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["关节3角度"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["关节4角度"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["关节5角度"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["关节6角度"].append(QPair<double,double>(0,0));

    pose << 0 << 0 << 0 << 0 << 0 << 0;
}

WishboneServer::~WishboneServer(){
//    QThread* &trd = threadHandler->trd;
//    trd->quit();
}

void WishboneServer::msgHandler(int x,int y,int z,int t)
{
    //数据处理函数，将WishboneThread传出来的信号数据，进行处理，并赋值到变量数组里面，包含时间戳
    double key(QTime::currentTime().msecsSinceStartOfDay()/1000.0);
    this->getDeviceSignals()["x"].append(QPair<double,double>(key,x));
    this->getDeviceSignals()["y"].append(QPair<double,double>(key,y));
    this->getDeviceSignals()["z"].append(QPair<double,double>(key,z));
    this->getDeviceSignals()["t"].append(QPair<double,double>(key,t));
    qDebug() << x << y << z << t;

}

void WishboneServer::incomingConnection()
{
//    //连接函数，逻辑无需改动
//    QThread* trd = new QThread();
//    threadHandler = new WishboneThread(trd);
//    threadHandler->moveToThread(trd);
//    connect(trd,&QThread::started,threadHandler,&WishboneThread::initial);
//    connect(trd, SIGNAL(finished()), threadHandler, SLOT(deleteLater()));
//    connect(threadHandler, &WishboneThread::msg,this,&WishboneServer::msgHandler);
//    qRegisterMetaType<DeviceState>("DeviceState");
//    connect(this,&WishboneServer::cmd,threadHandler,&WishboneThread::setState);
//    trd->start();
    pOpenGLWidget->getCamera().resetCamera(cameraNormalPos, cameraNormalTargetPos, cameraNormalFov, cameraNormalAspectRatio);
}

int WishboneServer::setDeviceJointAngle(QVector<float> joint){
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
        pOpenGLWidget->setMasterJointAngles(jointAngleVector);
    }
    pOpenGLWidget->update();
    return 1;
}

