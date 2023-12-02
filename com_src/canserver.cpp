#include "com_include/canserver.h"
#include "com_include/ControlCAN.h"
#include <QTime>
#include <QCoreApplication>
#include <QMetaType>
#include <string.h>
VCI_BOARD_INFO vbi;

CANThread::CANThread(QObject *parent):QThread(parent),deviceType(4),debicIndex(0),debicCom(0),baundRate(500)
{
    stopped = false;
    qRegisterMetaType<VCI_CAN_OBJ>("VCI_CAN_OBJ");
    qRegisterMetaType<DWORD>("DWORD");
}

void CANThread::stop()
{
    stopped = true;
}

bool CANThread::openCAN()
{
    int nDeviceType = deviceType; /* USBCAN-2A或USBCAN-2C或CANalyst-II */
    int nDeviceInd = debicIndex; /* 第1个设备 */
    int nCANInd = debicCom; /* 第1个通道 */
    DWORD dwRel;
    dwRel = VCI_OpenDevice(nDeviceType, nDeviceInd, nCANInd);
    if(dwRel != 1)
    {
//        qDebug()<<"open fail:"<< MB_OK<<MB_ICONQUESTION;
        return false;
    }
    else
//        qDebug()<<"open success";

    dwRel = VCI_ClearBuffer(nDeviceType, nDeviceInd, nCANInd);
    dwRel = VCI_ClearBuffer(nDeviceType, nDeviceInd, nCANInd+1);
    VCI_INIT_CONFIG vic;
    vic.AccCode=0x80000008;
    vic.AccMask=0xFFFFFFFF;
    vic.Filter=1;
    vic.Mode=0;
    switch (baundRate) {
    case 10:
        vic.Timing0=0x31;
        vic.Timing1=0x1c;
        break;
    case 20:
        vic.Timing0=0x18;
        vic.Timing1=0x1c;
        break;
    case 40:
        vic.Timing0=0x87;
        vic.Timing1=0xff;
        break;
    case 50:
        vic.Timing0=0x09;
        vic.Timing1=0x1c;
        break;
    case 80:
        vic.Timing0=0x83;
        vic.Timing1=0xff;
        break;
    case 100:
        vic.Timing0=0x04;
        vic.Timing1=0x1c;
        break;
    case 125:
        vic.Timing0=0x03;
        vic.Timing1=0x1c;
        break;
    case 200:
        vic.Timing0=0x81;
        vic.Timing1=0xfa;
        break;
    case 250:
        vic.Timing0=0x01;
        vic.Timing1=0x1c;
        break;
    case 400:
        vic.Timing0=0x80;
        vic.Timing1=0xfa;
        break;
    case 500:
        vic.Timing0=0x00;
        vic.Timing1=0x1c;
        break;
    case 666:
        vic.Timing0=0x80;
        vic.Timing1=0xb6;
        break;
    case 800:
        vic.Timing0=0x00;
        vic.Timing1=0x16;
        break;
    case 1000:
        vic.Timing0=0x00;
        vic.Timing1=0x14;
        break;
    case 33:
        vic.Timing0=0x09;
        vic.Timing1=0x6f;
        break;
    case 66:
        vic.Timing0=0x04;
        vic.Timing1=0x6f;
        break;
    case 83:
        vic.Timing0=0x03;
        vic.Timing1=0x6f;
        break;
    default:
        break;
    }
    dwRel = VCI_InitCAN(nDeviceType, nDeviceInd, nCANInd, &vic);
    dwRel = VCI_InitCAN(nDeviceType, nDeviceInd, nCANInd+1, &vic);
    if(dwRel !=1)
    {
//        qDebug()<<"init fail:"<<MB_OK<<MB_ICONQUESTION;
        return false;
    }
    else
//        qDebug()<<"init success";


    dwRel = VCI_ReadBoardInfo(nDeviceType, nDeviceInd, &vbi);
    if(dwRel != 1)
    {
//        qDebug()<<"get dev message fail:"<<MB_OK<<MB_ICONQUESTION;
        return false;
    }
    else
    {
        qDebug() <<"CAN channelNum: "<< vbi.can_Num;
        qDebug() <<"hardware version:"<< vbi.hw_Version;
        qDebug() << "interface version:" <<vbi.in_Version;
        qDebug() << "interupt request num:" << vbi.irq_Num;
    }

    if(VCI_StartCAN(nDeviceType, nDeviceInd, nCANInd) !=1)
    {
//        qDebug()<<"start"<<nCANInd<<"fail:"<<MB_OK<<MB_ICONQUESTION;
        return false;
    }
    else
        qDebug()<<"start"<<nCANInd<<"success:";

    int comid = nCANInd+1;
    if(VCI_StartCAN(nDeviceType, nDeviceInd, comid) !=1)
    {
//        qDebug()<<"start"<<comid<<"fail:"<<MB_OK<<MB_ICONQUESTION;
        return false;
    }
    else
        qDebug()<<"start"<<comid<<"success:";

    return true;
}

void CANThread::closeCAN()
{
    VCI_CloseDevice(deviceType, debicIndex);
}

void CANThread::sendData(int ID,unsigned char *ch)
{
    int count = strlen((char*)ch);
    int temp1 = count /8;
    int temp2 = count %8;
    if(temp2>0)
        count = temp1+1;
    else
        count = temp1;
    DWORD dwRel;
    VCI_CAN_OBJ vco[3];
    int i=0;
    for(;i<temp1;i++)
    {
        vco[i].ID =ID ;
        vco[i].RemoteFlag = 0;
        vco[i].ExternFlag = 0;
        vco[i].DataLen = 8;
        for(int j = 0;j<8;j++)
            vco[i].Data[j] = ch[j+(i*8)];
    }
    if(temp2>0)
    {
        vco[i].ID = i;
        vco[i].RemoteFlag = 0;
        vco[i].ExternFlag = 0;
        vco[i].DataLen = temp2;
        for(int j = 0;j<temp2;j++)
            vco[i].Data[j] = ch[j+(i*8)];
    }
    dwRel = VCI_Transmit(deviceType, debicIndex, debicCom, vco,count);
    if(dwRel>0)
        qDebug()<<"发送帧数："+dwRel;
    else
        qDebug()<<"发送错误："+dwRel;

}

void CANThread::run()
{
    while(!stopped)
    {
        DWORD dwRel;
        VCI_CAN_OBJ vco[2500];
        dwRel = VCI_Receive(deviceType, debicIndex, debicCom+1, vco,2500,0);
        if(dwRel > 0)
        {
//            for(int i = 0;i<dwRel;i++)
//            {
//                qDebug()<<"第"<<i<<"帧";
//                for(int j = 0;j<8;j++)
//                    qDebug()<<vco[i].Data[j];
//            }
//            qDebug()<<"from thread slot::" <<QThread::currentThreadId();
            emit msg(vco,dwRel);
        }
        else if(dwRel == -1)
        {
        /* USB-CAN设备不存在或USB掉线，可以调用VCI_CloseDevice并重新
        VCI_OpenDevice。如此可以达到USB-CAN设备热插拔的效果。 */
            qDebug()<<"设备不存在或USB掉线";
//            VCI_CloseDevice(deviceType, debicIndex);
//            VCI_OpenDevice(deviceType, debicIndex, debicCom);
        }
//        else
//            qDebug()<<"接收其它故障:"<<dwRel;
        sleep(1000);
    }
    stopped = false;
}

void CANThread::sleep(unsigned int msec)
{
   QTime dieTime = QTime::currentTime().addMSecs(msec);
   while( QTime::currentTime() < dieTime )
       QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

//-------------------------------------------------------------------

CANServer::CANServer(QObject* parent):QObject(parent),threadHandler(new CANThread(this))
{
}

CANServer::~CANServer(){
    threadHandler->stop();
    threadHandler->closeCAN();
    threadHandler->deleteLater();
    threadHandler->wait();
    delete threadHandler;
}

void CANServer::msgHandler(VCI_CAN_OBJ *vci,DWORD dwRel)
{
    QByteArray data;
    for(int i = 0;i<dwRel;i++)
    {
        for(int j = 0;j<8;j++)
            data.append(vci[i].Data[j]);
    }
    qDebug() << QString(data);
}

void CANServer::open(){
    threadHandler->openCAN();
    connect(threadHandler,&CANThread::msg,this,&CANServer::msgHandler);
    threadHandler->start();
}

void CANServer::close()
{
    threadHandler->stop();
    threadHandler->closeCAN();
}
