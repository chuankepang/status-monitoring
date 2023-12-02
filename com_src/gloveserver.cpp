#include "com_include/gloveserver.h"
#include <QDebug>
#include <QTimer>
#include <QThread>
#include "com_include/SG/Library.h" //Contains version information on SGCore / SGConnect Libraries
#include "com_include/SG/SenseCom.h" //Functions to check scanning process - and to start it if need be.

#include "com_include/SG/HapticGlove.h" //Haptic Glove Interfacing
#include "com_include/SG/Tracking.h" //To calculate wrist location based on fixed hardware offsets.
#include "com_include/SG/SG_FFBCmd.h" //force-feedback command(s)
#include "com_include/SG/SG_BuzzCmd.h" //vibration command(s)
#include "com_include/SG/SGConnect.h"

GloveThread::GloveThread(QThread* trd,QObject *parent)
    : QObject(parent),deviceName("Glove"),trd(trd),state(Disconnected_S)
{
    auto_timer = new QTimer(this);
    connect(auto_timer,&QTimer::timeout,this,&GloveThread::readData);
}

GloveThread::~GloveThread(){
    bool scanActive = SGConnect::ScanningActive();
    auto_timer->stop();
    delete auto_timer;

    if(scanActive){
        int disposeCode = SGConnect::Dispose();
        if (disposeCode > 0)
        {
            qDebug() << QString::fromStdString("Succesfully cleaned up SGConnect resources: (DisposeCode = " + std::to_string(disposeCode) + ")");
        }
        else
        {
             qDebug() << QString::fromStdString( "Unable to properly dispose of SGConnect resources: (DisposeCode = " + std::to_string(disposeCode) + ")." );
        }
    }
}

void GloveThread::initial()
{
    qDebug() << "Testing " << QString::fromStdString(SGConnect::GetLibraryVersion());

    SGConnect::SetDebugLevel(0);
    bool scanActive = SGConnect::ScanningActive();

    if (!scanActive)
    {
        int initCode = SGConnect::Init();
        if (initCode > 0)
        {
            qDebug() << QString::fromStdString("Succesfully initialized background process: (InitCode = " + std::to_string(initCode) + ")");
            qDebug() << "Showing the list at the end:";
            qDebug() << QString::fromStdString(SGConnect::GetConnectionStates());
        }
        else{
            emit error(tr("server error"));
            state = DeviceErr_S;
        }
    }

    bool connectionsActive = SGCore::SenseCom::ScanningActive();
    if (!connectionsActive)
    {
        bool startedSenseCom = SGCore::SenseCom::StartupSenseCom();
        if (startedSenseCom)
        {
            qDebug() << "Successfully started SenseCom. It will take a few seconds to connect...";
            connectionsActive = SGCore::SenseCom::ScanningActive();
        }
        else
        {
            emit error("server error");
            state = DeviceErr_S;
        }
    }

    std::shared_ptr<SGCore::HapticGlove> testGlove; //SGCore.HapticGlove - The class from which all of our gloves derive. For this reason, we use a (smart) pointer, rather than a class itself.
    bool gotHapticGlove = SGCore::HapticGlove::GetGlove(testGlove); //GetGlove returns the first glove connected to your system - doesn't care if it's a left or a right hand.
    while (!gotHapticGlove) //For this exercise, I'll keep trying to connect to a glove.
    {
        emit error("disconnected");
        gotHapticGlove = SGCore::HapticGlove::GetGlove(testGlove);
    }

    state = Connected_S;

    std::shared_ptr<SGCore::HapticGlove> leftGlove, rightGlove;
    gotLeft  = SGCore::HapticGlove::GetGlove(false, leftGlove); // passing false returns the first connected left hand
    gotRight = SGCore::HapticGlove::GetGlove(true,  rightGlove); // passing true returns the first connected right hand

    // Alternatively, you can get a list of all connected Haptic Gloves on your system
    std::vector<std::shared_ptr<SGCore::HapticGlove>> allGloves = SGCore::HapticGlove::GetHapticGloves();

    qDebug() << QString::fromStdString("There are " + std::to_string(allGloves.size()) + " Haptic Glove(s) connected to your system.");

    if (gotLeft)
    { qDebug() << QString::fromStdString("Your Left hand is " + leftGlove->ToString()); }
    else
    { qDebug() << QString::fromStdString("You have no left hands connected to your system."); }

    if (gotRight)
    { qDebug() << QString::fromStdString("Your Right hand is " + rightGlove->ToString()); }
    else
    { qDebug() << QString::fromStdString("You have no left hands connected to your system."); }

    connect(this,&GloveThread::ask,this,&GloveThread::writeData);
}

void GloveThread::readData()
{
    std::shared_ptr<SGCore::HapticGlove> glove;
    gotLeft  = SGCore::HapticGlove::GetGlove(false, glove); // passing false returns the first connected left hand
    gotRight = SGCore::HapticGlove::GetGlove(true,  glove); // passing true returns the first connected right hand
    if(gotLeft){
        SGCore::HapticGlove::GetGlove(false, glove);
        SGCore::HandPose handPose;
        if(glove->GetHandPose(handPose)){
            qDebug() << QString::fromStdString(handPose.Serialize());
            std::vector<float> flexions = handPose.GetNormalizedFlexion();

            SGCore::PosTrackingHardware tracker = SGCore::PosTrackingHardware::ViveTracker;
            SGCore::Kinematics::Vect3D trackerPosition = SGCore::Kinematics::Vect3D::zero; //set at 0, 0, 0
            SGCore::Kinematics::Quat trackerRotation = SGCore::Kinematics::Quat::identity; //no rotation either.

            SGCore::Kinematics::Vect3D wristPosition;
            SGCore::Kinematics::Quat wristRotation;
            glove->GetWristLocation(trackerPosition, trackerRotation, tracker, wristPosition, wristRotation);

            qDebug() << QString::fromStdString("If your tracked device (" + SGCore::Tracking::ToString(tracker) + ") is at position " + trackerPosition.ToString() + ", and rotation " + trackerRotation.ToEuler().ToString()
                + ", your wrist position is " + wristPosition.ToString() + ", with rotation at " + wristRotation.ToEuler().ToString());
        }
    }
    if(gotRight){
        SGCore::HapticGlove::GetGlove(true, glove);
        SGCore::HandPose handPose;
        if(glove->GetHandPose(handPose)){
//            qDebug() << QString::fromStdString(handPose.Serialize());
            std::vector<float> flexions = handPose.GetNormalizedFlexion();
            emit flexion(true,QVector<float>(flexions.begin(),flexions.end()));
        }
    }
}

void GloveThread::writeData(bool LR, bool type, QVector<int> percent, int ms)
{
    std::shared_ptr<SGCore::HapticGlove> glove;
    bool test = SGCore::HapticGlove::GetGlove(LR, glove);
    if(type == false){
        SGCore::Haptics::SG_FFBCmd ffb(percent[0], percent[1], percent[2], percent[3], percent[4]); //sets each of the fingers to a desired level [thumb ... pinky]
        glove->SendHaptics(ffb); //and send it to the SendHaptics Command
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        glove->StopHaptics(); //turns off all Vibration and Force-Feedback.
    }
    else if(type == true) {
        SGCore::Haptics::SG_BuzzCmd buzz(percent[0], percent[1], percent[2], percent[3], percent[4]); //sets each of the fingers to a desired level [thumb ... pinky]
        glove->SendHaptics(buzz);
        QThread::msleep(ms);
        glove->StopVibrations(); //Stops all vibrations on the glove.
    }
}


void GloveThread::setState(DeviceState s)
{
    if(s == Connected_S){
        state = s;
        auto_timer->stop();
    }
    else if(s == Busy_S){
        state = s;
        auto_timer->start(50);
    }
}

GloveServer::GloveServer(QObject *parent): Devices(parent)
{
    this->change_DeviceType(0);
    this->change_device_name("数据手套");
    this->imgpath = tr(":/new/prefix1/image/glove.png");
    this->dof = 0;
    this->getDeviceSignals().clear();
    this->getDeviceSignals().insert("大拇指弯曲",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("食指弯曲",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("中指弯曲",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("无名指弯曲",QVector<QPair<double,double>>());
    this->getDeviceSignals().insert("小拇指弯曲",QVector<QPair<double,double>>());

    this->getDeviceSignals()["大拇指弯曲"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["食指弯曲"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["中指弯曲"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["无名指弯曲"].append(QPair<double,double>(0,0));
    this->getDeviceSignals()["小拇指弯曲"].append(QPair<double,double>(0,0));
    pose << 0 << 0 << 0 << 0 << 0 << 0;
}

GloveServer::~GloveServer(){
    QThread* &trd = threadHandler->trd;
    trd->quit();
}

void GloveServer::flexionHandler(bool LR,QVector<float> flexions)
{
//    qDebug() << LR;
//    std::string flexes = "";
//    for (int i=0; i<flexions.size(); i++)
//    {
//        flexes += std::to_string( (int) std::round( flexions[i] ) );
//        if (i < flexions.size() - 1) { flexes += ", "; }
//    }
//    qDebug() << QString::fromStdString("Normalized finger flexions [" + flexes + "]");

    double key(QTime::currentTime().msecsSinceStartOfDay()/1000.0);
    this->getDeviceSignals()["大拇指弯曲"].append(QPair<double,double>(key,flexions[0]));
    this->getDeviceSignals()["食指弯曲"].append(QPair<double,double>(key,flexions[1]));
    this->getDeviceSignals()["中指弯曲"].append(QPair<double,double>(key,flexions[2]));
    this->getDeviceSignals()["无名指弯曲"].append(QPair<double,double>(key,flexions[3]));
    this->getDeviceSignals()["小拇指弯曲"].append(QPair<double,double>(key,flexions[4]));

}

void GloveServer::incomingConnection()
{
    QThread* trd = new QThread();
    threadHandler = new GloveThread(trd);
    threadHandler->moveToThread(trd);
    connect(trd,&QThread::started,threadHandler,&GloveThread::initial);
    connect(trd, SIGNAL(finished()), threadHandler, SLOT(deleteLater()));
    qRegisterMetaType<QVector<float>>("QVector<float>");
    qRegisterMetaType<QVector<int>>("QVector<int>");
    connect(threadHandler, &GloveThread::flexion,this,&GloveServer::flexionHandler);
    qRegisterMetaType<DeviceState>("DeviceState");
    connect(this,&GloveServer::cmd,threadHandler,&GloveThread::setState);
    trd->start();
}
