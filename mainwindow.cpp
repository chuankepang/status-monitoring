#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "devicesscan.h"
#include "ui_devicesscan.h"
#include "ui_monitors.h"
#include "devicesanalyse.h"
#include "orderconvert.h"
#include "setting.h"
#include <QDebug>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //指向子页面的指针实现，以指针+new的方式创建在堆区
    ds=new DevicesScan();
    da=new DevicesAnalyse();
    oc=new OrderConvert();
    s=new Setting();

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);//无边框

    //自定义槽函数与自定义信号的连接
    connect(this->ds,&DevicesScan::back,this,&MainWindow::when_devicesscan_back);
    connect(this->da,&DevicesAnalyse::back,this,&MainWindow::when_devicesanalyse_back);
    connect(this->oc,&OrderConvert::back,this,&MainWindow::when_orderconvert_back);
    connect(this->s,&Setting::back,this,&MainWindow::when_setting_back);
    connect(this->ds->get_ui()->scanDevices,&QPushButton::clicked,this,&MainWindow::create_devices_list);
    connect(this->ds->get_ui()->save,&QPushButton::clicked,this,&MainWindow::refresh_mainwindow);


    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    //设定X轴为时间
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->customPlot->xAxis->setTicker(timeTicker);
   //所有曲线可见
    ui->customPlot->axisRect()->setupFullAxesBox();

    //子界面急停连接
    connect(this->ds,&DevicesScan::stop,this,&MainWindow::sysStop);
    connect(this->oc,&OrderConvert::stop,this,&MainWindow::sysStop);
    connect(this->da,&DevicesAnalyse::stop,this,&MainWindow::sysStop);
    connect(this->s,&Setting::stop,this,&MainWindow::sysStop);


    //打开状态更新和界面主程序的寄存器
    dataTimer.stop();
    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    connect(this->ds->get_ui()->save,&QPushButton::clicked,[=]{
        stateTimer.start(100);
    });
    connect(&stateTimer,SIGNAL(timeout()), this, SLOT(stateCheck()));

    //远端视频相关函数
    vs = new VideoServer(this);
    vs->incomingConnection();
    connect(vs->threadHandler,&VideoThread::img,this,&MainWindow::updateFrame);
    connect(ui->videoLink, &QPushButton::clicked, this, &MainWindow::startCamera);
    connect(ui->videoUnlink, &QPushButton::clicked, this, &MainWindow::stopCamera);
    int maxSize = std::max(ui->videoLabel->width(), ui->videoLabel->height());
    emit vs->threadHandler->scale(maxSize);

    //配置上和右坐标轴来显示刻度,但是不显示数字
    //xAxis, yAxis, xAxis2, yAxis2，分别对应下、左、上、右。
    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTickLabels(false);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->yAxis2->setTickLabels(false);

    //修改左和底坐标轴,使之与右和上坐标轴始终匹配
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    ui->customPlot->rescaleAxes();
    //使自适应范围


    //允许用户用鼠标缩放
    ui->customPlot->setInteractions(QCP::iRangeDrag |QCP::iRangeZoom);
    ui->customPlot->selectionRect()->setPen(QPen(Qt::black,1,Qt::DashLine));//设置选框的样式：虚线
    ui->customPlot->selectionRect()->setBrush(QBrush(QColor(0,0,100,50)));//设置选框的样式：半透明浅蓝
    ui->customPlot->setSelectionRectMode(QCP::SelectionRectMode::srmZoom);
}
MainWindow::~MainWindow()
{
    delete ui;
}
/*
在UI设计界面，选择按钮“转到槽”，自动生成以下槽函数实现模板
void 类名::on_按钮名_信号()
{
}
*/

void MainWindow::on_devicesScan_triggered()
{
//    this->hide();
    this->ds->show();
    this->ds->startTimer();
}

void MainWindow::on_devicesAnalyse_triggered()
{
    int current_input_device = this->ds->get_current_input_device();
    int current_output_device = this->ds->get_current_output_device();
    if(current_input_device==-1){
        QMessageBox::warning(this,tr("设备分析"),tr("未选择输入设备"),QMessageBox::Ok);
    }
    else if(current_output_device==-1){
        QMessageBox::warning(this,tr("设备分析"),tr("未选择输出设备"),QMessageBox::Ok);
    }
    else{
        this->da->setDevice(connectInputDevices[current_input_device],connectOutputDevices[current_output_device]);
        this->hide();
        this->da->show();
    }
}

void MainWindow::on_orderConvert_triggered()
{
    this->hide();
    this->oc->show();
}

void MainWindow::on_settingSerial_triggered()
{
    this->hide();
    this->s->show();
}

//MainWindow类的自定义槽函数的实现
void MainWindow::when_devicesscan_back()
{
    this->ds->close();
    if(this->ds->get_current_input_device()!=-1){
        DeviceState inputState = connectInputDevices[this->ds->get_current_input_device()]->getDeviceState();
        switch (inputState) {
           case Disconnected_S: this->ui->input_device_status->setStatus(IndicatorLabel::Status::Offline);break;
           case Connected_S:this->ui->input_device_status->setStatus(IndicatorLabel::Status::Ok);break;
           case DeviceErr_S:this->ui->input_device_status->setStatus(IndicatorLabel::Status::Error);break;
           case Busy_S:this->ui->input_device_status->setStatus(IndicatorLabel::Status::Busy);break;
           default:break;
        }
    }
    if(this->ds->get_current_output_device()!=-1){
        DeviceState outputState = connectOutputDevices[this->ds->get_current_output_device()]->getDeviceState();
        switch (outputState) {
           case Disconnected_S: this->ui->output_device_status->setStatus(IndicatorLabel::Status::Offline);break;
           case Connected_S:this->ui->output_device_status->setStatus(IndicatorLabel::Status::Ok);break;
           case DeviceErr_S:this->ui->output_device_status->setStatus(IndicatorLabel::Status::Error);break;
           case Busy_S:this->ui->output_device_status->setStatus(IndicatorLabel::Status::Busy);break;
           default:break;
        }
    }
    QString out = connectOutputDevices[this->ds->get_current_output_device()]->getDeviceName();
    if(out == tr("双叉臂") || out == tr("人形机器人"))
    {
        if(out==tr("双叉臂")){
            this->ui->openGLWidget->setIsPaintMaster(true);
            this->ui->openGLWidget->setIsPaintSlave(false);
        }
        else if(out == tr("人形机器人")){
            this->ui->openGLWidget->setIsPaintMaster(false);
            this->ui->openGLWidget->setIsPaintSlave(true);
        }
        this->ui->stackedWidget->setCurrentIndex(1);
    }
    else{
        this->ui->stackedWidget->setCurrentIndex(0);
    }
    this->show();
}

void MainWindow::when_devicesanalyse_back()
{
    this->show();
    this->da->close();
}

void MainWindow::when_orderconvert_back()
{
    this->show();
    this->oc->close();
}

void MainWindow::when_setting_back()
{
    this->show();
    this->s->close();
}

void MainWindow::on_CloseProgramButton_clicked()
{
    this->close();
}

void MainWindow::create_devices_list()
{
    for(auto &device:connectInputDevices){
        delete device;
    }
    for(auto &device:connectOutputDevices){
        delete device;
    }
    connectInputDevices.clear();
    connectOutputDevices.clear();
    QString ip = this->s->getIP();
    quint16 port = this->s->getPort();
    //生成设备类列表
    connectInputDevices << new ArmServer("左",this) << new ArmServer("右",this) << new GloveServer(this) << new MouseServer(this) << new StickServer(this);


    //生成设备类列表
    connectOutputDevices << new AGVServer(this,ip,port) << new UR5Server(this,ip,port)<< new WishboneServer(ui->openGLWidget,this)<< new HumanoidServer(ui->openGLWidget,this);



    os = new OtherServer();
    os->incomingConnection();

    this->ds->refresh_input_list(connectInputDevices);
    this->ds->refresh_output_list(connectOutputDevices);
    this->ds->refresh_other_list(os->getOtherDevice());


    qDebug()<<"已经正常生成了输入/输出设备列表";
}

void MainWindow::refresh_mainwindow()
{
    // 遍历容器，逐个删除元素
    init_mainwindow();
    int current_input_device=this->ds->get_current_input_device();//当前的
    if(current_input_device!=-1)
    {
        qDebug()<<"当前输入设备是 "<<current_input_device;
        ui->input_device->setText(connectInputDevices[current_input_device]->getDeviceName());
        if(connectInputDevices[current_input_device]->getDeviceSignals().size()>0)
        {
            for (int i = 0; i < connectInputDevices[current_input_device]->getDeviceSignals().size(); ++i)
            {
    //         QMap<QString, QVector<QPair<QTime,double>>> device_signals;//变量列表
//               qDebug()<<"到这里了175，生成控制变量显示的MyCheckBox";
               MyCheckBox *checkBox = new MyCheckBox(QString(connectInputDevices[current_input_device]->getDeviceSignals().keys().at(i)));
               checkBox->setID(i);//当前的checkbox控制ID号为i的线条、label
               checkBox->set_signal_type(INPUT_SIGNAL);

               checkBox->setContextMenuPolicy(Qt::CustomContextMenu);
               connect(checkBox, &QCheckBox::customContextMenuRequested, this, &MainWindow::customContextMenuRequested);//连接上下文处理函数

               input_checkbox.append(checkBox);

               connect(checkBox, &QCheckBox::stateChanged, [=](int state) {
                   ui->customPlot->graph(i)->setVisible(state == Qt::Checked);
                   ui->customPlot->yAxis->rescale(true);//调整Y轴的范围，使之能显示出所有的曲线的Y值
                   ui->customPlot->replot();
                   monitors[i]->setVisible(state == Qt::Checked);//暂时将这个复选框和监视窗口连接起来
                   monitors[i]->setWindowTitle(connectInputDevices[current_input_device]->getDeviceName()+"变量"+input_checkbox[i]->text());

               });//将复选框和线条连接起来


               QLabel *label = new QLabel();
               input_label.append(label);
               ui->gridLayout_input->addWidget(checkBox, i, 0);
               ui->gridLayout_input->addWidget(label, i, 1);
            }
        }
    }
    input_signal_num=input_checkbox.size();
    int current_output_device=this->ds->get_current_output_device();
    qDebug()<<"当前输出设备是 "<<current_output_device;
    if(current_output_device!=-1)
    {
        ui->output_device->setText(connectOutputDevices[current_output_device]->getDeviceName());
        if(connectOutputDevices[current_output_device]->getDeviceSignals().size()>0)
        {
            for (int i = 0; i < connectOutputDevices[current_output_device]->getDeviceSignals().size(); ++i)
            {
    //         QMap<QString, QVector<QPair<QTime,double>>> device_signals;//变量列表
               MyCheckBox *checkBox = new MyCheckBox(QString(connectOutputDevices[current_output_device]->getDeviceSignals().keys().at(i)));
               checkBox->setID(i);//当前的checkbox控制ID号为i的线条、label
               checkBox->set_signal_type(OUTPUT_SIGNAL);

               checkBox->setContextMenuPolicy(Qt::CustomContextMenu);
               connect(checkBox, &QCheckBox::customContextMenuRequested, this, &MainWindow::customContextMenuRequested);//连接上下文处理函数

               connect(checkBox, &QCheckBox::stateChanged, [=](int state) {
                   ui->customPlot->graph(i+input_signal_num)->setVisible(state == Qt::Checked);

                   ui->customPlot->yAxis->rescale(true);//调整Y轴的范围，使之能显示出所有的曲线的Y值
                   ui->customPlot->replot();
                   monitors[i+input_signal_num]->setVisible(state == Qt::Checked);//暂时将这个复选框和监视窗口连接起来
                   monitors[i+input_signal_num]->setWindowTitle(connectOutputDevices[current_output_device]->getDeviceName()+"变量"+output_checkbox[i]->text());//名字以checkbox的名字为准
               });//将复选框和线条连接起来

               output_checkbox.append(checkBox);
               QLabel *label = new QLabel();
               output_label.append(label);
               ui->gridLayout_output->addWidget(checkBox, i, 0);
               ui->gridLayout_output->addWidget(label, i, 1);
            }
        }

    }

    output_signal_num=output_checkbox.size();
//    input_dataset.resize(input_signal_num);
//    output_dataset.resize(output_signal_num);

    QMap<QString,int>* otherDevice = os->getOtherDevice();
    auto otherDeviceVar = os->getOtherDeviceVar();
    auto otherDeviceVarVal = os->getOtherDeviceVarVal();
    int currentID = 0;
    for(int i=0;i<otherDevice->size();i++){
       if(otherDevice->value(otherDevice->keys()[i]) != 0){
           for(int j=0;j<otherDeviceVar->value(otherDevice->keys()[i]).size();j++){
               MyCheckBox *checkBox = new MyCheckBox(otherDeviceVar->value(otherDevice->keys()[i])[j]);
               checkBox->setID(currentID);//当前的checkbox控制ID号为i的线条、label

               checkBox->set_signal_type(OUTPUT_SIGNAL);

               connect(checkBox, &QCheckBox::stateChanged, [=](int state) {
                   ui->customPlot->graph(i+input_signal_num+output_signal_num)->setVisible(state == Qt::Checked);

                   ui->customPlot->yAxis->rescale(true);//调整Y轴的范围，使之能显示出所有的曲线的Y值
                   ui->customPlot->replot();
                   monitors[currentID+input_signal_num+output_signal_num]->setVisible(state == Qt::Checked);//暂时将这个复选框和监视窗口连接起来
                   monitors[currentID+input_signal_num+output_signal_num]->setWindowTitle(otherDevice->keys()[i]+"变量"+other_checkbox[currentID]->text());//名字以checkbox的名字为准
               });//将复选框和线条连接起来

               other_checkbox.append(checkBox);
               QLabel *label = new QLabel();
               other_label.append(label);
               ui->gridLayout_output_2->addWidget(checkBox, currentID, 0);
               ui->gridLayout_output_2->addWidget(label, currentID, 1);
               other_label_id_map.insert(otherDeviceVar->value(otherDevice->keys()[i])[j],currentID);
               currentID++;
           }
       }
    }
    other_signal_num = currentID;
    qDebug()<<"假装已经刷新了主页面";
    draw();
    oc->init_window();
    QMap<QString, QVector<QPair<double,double>>> tmp=connectInputDevices[current_input_device]->getDeviceSignals();
    oc->draw_window(tmp);
}


void MainWindow::on_input_device_clicked()
{
    this->hide();
    this->ds->show();
}

void MainWindow::on_output_device_clicked()
{
    this->hide();
    this->ds->show();
}

void MainWindow::draw()
{
   //添加图并填充内容
//    int current_input_device=this->ds->get_current_input_device();//当前的输入设备
//    int current_output_device=this->ds->get_current_output_device();
    monitors.clear();
    for(int i=0;i<input_signal_num;++i)//处理输入设备信号的显示效果
    {
       ui->customPlot->addGraph();//添加线条，数量为所有输入输出信号之和
        // 随机分配线条颜色
       QColor lineColor = QColor::fromRgb(QRandomGenerator::global()->bounded(256),
                                           QRandomGenerator::global()->bounded(256),
                                           QRandomGenerator::global()->bounded(256));

       ui->customPlot->graph(i)->setPen(QPen(lineColor));
       ui->customPlot->graph(i)->setVisible(false);//默认均不可见

       //新建一个线条显示窗口，在构造函数中已完成曲线添加、坐标轴参数修改
       Monitors *tmp=new Monitors(this);
//       tmp->setWindowTitle(connectInputDevices[current_input_device]->getDeviceName()+"变量"+input_checkbox[i]->text());
       //设置线条显示窗口图像
       tmp->set_graph_color(lineColor);
       monitors.append(tmp);
       MovingAverageFilter* fliter=new MovingAverageFilter(10);//滑动窗口为10的移动平均滤波器
       fliters.append(fliter);
    }

    for(int i=input_signal_num;i<input_signal_num+output_signal_num;++i)//注意，输出图像的ID排在输入图像之后
    {
       ui->customPlot->addGraph();//添加线条，数量为所有输入输出信号之和
        // 随机分配线条颜色
       QColor lineColor = QColor::fromRgb(QRandomGenerator::global()->bounded(256),
                                           QRandomGenerator::global()->bounded(256),
                                           QRandomGenerator::global()->bounded(256));

       ui->customPlot->graph(i)->setPen(QPen(lineColor));
       ui->customPlot->graph(i)->setVisible(false);//默认均可见

       //新建一个线条显示窗口
       Monitors *tmp=new Monitors(this);
       tmp->set_graph_color(lineColor);
       //设置线条显示窗口图像
       monitors.append(tmp);
    }

    for(int i=input_signal_num+output_signal_num;i<input_signal_num+output_signal_num+other_signal_num;++i)//注意，输出图像的ID排在输入图像之后
    {
       ui->customPlot->addGraph();//添加线条，数量为所有输入输出信号之和
        // 随机分配线条颜色
       QColor lineColor = QColor::fromRgb(QRandomGenerator::global()->bounded(256),
                                           QRandomGenerator::global()->bounded(256),
                                           QRandomGenerator::global()->bounded(256));

       ui->customPlot->graph(i)->setPen(QPen(lineColor));
       ui->customPlot->graph(i)->setVisible(false);//默认均可见

       //新建一个线条显示窗口
       Monitors *tmp=new Monitors(this);
       tmp->set_graph_color(lineColor);
       //设置线条显示窗口图像
       monitors.append(tmp);
    }

}

void MainWindow::stateCheck()
{
    DeviceState inputState = connectInputDevices[this->ds->get_current_input_device()]->getDeviceState();
    DeviceState outputState = connectOutputDevices[this->ds->get_current_output_device()]->getDeviceState();
    switch (inputState) {
       case Disconnected_S: this->ui->input_device_status->setStatus(IndicatorLabel::Status::Offline);break;
       case Connected_S:this->ui->input_device_status->setStatus(IndicatorLabel::Status::Ok);break;
       case DeviceErr_S:this->ui->input_device_status->setStatus(IndicatorLabel::Status::Error);break;
       case Busy_S:this->ui->input_device_status->setStatus(IndicatorLabel::Status::Busy);break;
       default:break;
    }
    switch (outputState) {
       case Disconnected_S: this->ui->output_device_status->setStatus(IndicatorLabel::Status::Offline);break;
       case Connected_S:this->ui->output_device_status->setStatus(IndicatorLabel::Status::Ok);break;
       case DeviceErr_S:this->ui->output_device_status->setStatus(IndicatorLabel::Status::Error);break;
       case Busy_S:this->ui->output_device_status->setStatus(IndicatorLabel::Status::Busy);break;
       default:break;
    }
    for (int i = 0; i < connectInputDevices.size(); i++) {
        DeviceState state = connectInputDevices[i]->getDeviceState();
        if(state == DeviceErr_S){
//            qDebug() << "error";
        }
    }
    for (int i = 0; i < connectOutputDevices.size(); i++) {
        DeviceState state = connectOutputDevices[i]->getDeviceState();
        if(state == DeviceErr_S){
//            qDebug() << "error";
        }
    }
}

void MainWindow::realtimeDataSlot()//实时处理数据
{
   static QTime time(QTime::currentTime());
   double key = time.elapsed()/1000.0; // 开始到现在的时间，单位秒
   static double lastPointKey = 0;

   if (key-lastPointKey > 0.01) // 大约10ms转换一次数据
   {
     // 添加数据到graph
     auto tmp=connectInputDevices[this->ds->get_current_input_device()]->getDeviceSignals().begin();
//     const QMap<QString, QVector<QPair<double, double>>>::iterator tmp = connectInputDevices[this->ds->get_current_input_device()]->getDeviceSignals().begin();
     for(int i=0;i<input_signal_num;++i)
     {
//         double noise_value=qSin(key)+i+qrand()/(double)RAND_MAX*1*qSin(key/0.3843);
         double input_value = (tmp+i)->last().second;
         double value=oc->get_position_alpha()*fliters[i]->filter(input_value);
         ui->customPlot->graph(i)->addData(key,input_value);
         monitors[i]->draw(key,input_value);//在变量监视器页面绘图区也添加新的坐标
         oc->draw_graph(i,key,input_value,value);
         input_label[i]->setText(QString::number(value));//输出当前数值
     }

     tmp=connectOutputDevices[this->ds->get_current_output_device()]->getDeviceSignals().begin();
     for(int i=0;i<output_signal_num;++i)
     {
         double output_value = (tmp+i)->last().second;
         (tmp+i)->append(QPair<double,double>(key,output_value));//找到当前连接的输出设备列表中的使能设备，按ID号索引储存变量值的QVector，插入一个QPair
         ui->customPlot->graph(i+input_signal_num)->addData(key,output_value);
         monitors[i+input_signal_num]->draw(key,output_value);//在变量监视器页面绘图区也添加新的坐标
         output_label[i]->setText(QString::number(output_value));//输出当前数值
     }

     tmp=this->os->getDeviceSignals().begin();
     for(int i=0;i<other_signal_num;++i)
     {
         double otherVal = (tmp+i)->last().second;
         int currentID = other_label_id_map[(tmp+i).key()];
         ui->customPlot->graph(currentID+input_signal_num+output_signal_num)->addData(key,otherVal);
         monitors[currentID+input_signal_num+output_signal_num]->draw(key,otherVal);
         other_label[currentID]->setText(QString::number(otherVal));
     }
     //记录当前时刻
     lastPointKey = key;
   }
   // 曲线能动起来的关键在这里，设定x轴范围为最近8个时刻
    ui->customPlot->xAxis->setRange(key, 8, Qt::AlignRight);
//    ui->customPlot->yAxis->rescale(true);//调整Y轴的范围，使之能显示出所有的曲线的Y
    //还要把每个子页面绘图区调整好


   //绘图
    ui->customPlot->replot();

    //指令转化
    convertData(connectInputDevices[this->ds->get_current_input_device()],connectOutputDevices[this->ds->get_current_output_device()],1.0,da->getMode());
}

void MainWindow::on_pause_clicked()
{
    if(dataTimer.isActive())
    {
        dataTimer.stop();
        ui->pause->setText("开始");
        int current_input_device=this->ds->get_current_input_device();
        int current_output_device=this->ds->get_current_output_device();
        this->connectInputDevices[current_input_device]->cmd(Connected_S);
        this->connectOutputDevices[current_output_device]->cmd(Connected_S);
        this->os->cmd(Connected_S);
        ui->customPlot->yAxis->rescale(true);//调整Y轴的范围，使之能显示出所有的曲线的Y值
        //绘图
        ui->customPlot->replot();
    }
    else
    {
        dataTimer.start(0); // Interval 0 means to refresh as fast as possible
        ui->pause->setText("停止");
        int current_input_device=this->ds->get_current_input_device();
        int current_output_device=this->ds->get_current_output_device();
        this->connectInputDevices[current_input_device]->cmd(Busy_S);
        this->connectOutputDevices[current_output_device]->cmd(Busy_S);
        this->os->cmd(Busy_S);
    }
}


void MainWindow::on_save_clicked()
{
    qDebug() << "保存文件";
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString input_device_name=ui->input_device->text();
    QString output_device_name=ui->output_device->text();


    // 将输入设备的信号数据写入文件
    auto tmp_connectInputDevices=connectInputDevices[this->ds->get_current_input_device()]->getDeviceSignals().begin();
    auto tmp_connectOutputDevices=connectOutputDevices[this->ds->get_current_output_device()]->getDeviceSignals().begin();

    for (int i = 0; i < input_checkbox.size(); i++)
    {
        if(input_checkbox[i]->isChecked())//检查当前输入信号变量是否已被勾选
        {
            QString fileName = input_device_name+"_"+input_checkbox[i]->text()+"_"+currentDateTime.toString("yyyyMMdd_hhmmss") + ".txt";//变量名以checkbox的显示为准
            qDebug()<<fileName;
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&file);
                out << "Time" << "\t" << "Value" << "\n";
                for (int j=1 ;j< (tmp_connectInputDevices+i)->size();++j)
                {
                    out << (tmp_connectInputDevices+i).value()[j].first-tmp_connectInputDevices.value()[1].first << " " << (tmp_connectInputDevices+i).value()[j].second << "\n";
                }
                file.close();
            }
            else
            {
                qDebug() << "Failed to open file for writing.";
            }
        }

    }

    for (int i = 0; i < output_checkbox.size(); i++)
    {
        if(output_checkbox[i]->isChecked())//检查当前输入信号变量是否已被勾选
        {
            QString fileName = output_device_name+"_"+output_checkbox[i]->text() +"_"+currentDateTime.toString("yyyyMMdd_hhmmss") + ".txt";//变量名以checkbox的显示为准
            qDebug()<<fileName;
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&file);
                out << "Time" << " " << "Value" << "\n";
                for (int j=1 ;j< (tmp_connectOutputDevices+i)->size();++j)
                {
                    out << (tmp_connectOutputDevices+i).value()[j].first - tmp_connectOutputDevices.value()[1].first<< " " << (tmp_connectOutputDevices+i).value()[j].second << "\n";
                }
                file.close();
            }
            else
            {
                qDebug() << "Failed to open file for writing.";
            }

        }

    }
    QMessageBox::information(this,QString("保存"),tr("保存成功"),QMessageBox::Ok);
}

void MainWindow::sysStop()
{
    qDebug() << "要急停";
    if(dataTimer.isActive())
    {
        dataTimer.stop();
        ui->pause->setText("开始");
        int current_input_device=this->ds->get_current_input_device();
        int current_output_device=this->ds->get_current_output_device();
        this->connectInputDevices[current_input_device]->cmd(Connected_S);
        this->connectOutputDevices[current_output_device]->cmd(Connected_S);
        ui->customPlot->yAxis->rescale(true);//调整Y轴的范围，使之能显示出所有的曲线的Y值
        //绘图
        ui->customPlot->replot();
    }
}

void MainWindow::on_stop_clicked()
{
    sysStop();
}

// 在事件处理函数中处理复选框的上下文菜单事件
void MainWindow::customContextMenuRequested(const QPoint& pos)
{
    int current_input_device=this->ds->get_current_input_device();//当前的输入设备
    int current_output_device=this->ds->get_current_output_device();
    MyCheckBox* checkbox = qobject_cast<MyCheckBox*>(sender()); // 获取发送信号的对象，即复选框
    if (checkbox) {
        QMenu contextMenu; // 创建上下文菜单
        QAction* editAction = new QAction("编辑文本", &contextMenu); // 创建编辑动作
        connect(editAction, &QAction::triggered, this, [=]() {
            // 在触发编辑动作时，弹出输入对话框
            bool ok;
            QString newText = QInputDialog::getText(this, "编辑文本 ", "请输入新的文本 ", QLineEdit::Normal, checkbox->text(), &ok);
            if (ok && !newText.isEmpty()) {
                checkbox->setText(newText); // 更新复选框的文本

                const int ID=checkbox->getID();
                const bool signal_type=checkbox->get_signal_type();

                if(signal_type==INPUT_SIGNAL)
                {
                    oc->change_checkbox_text(ID,newText);
                    monitors[ID]->setWindowTitle(connectInputDevices[current_input_device]->getDeviceName()+"变量"+newText);
                }

                else if(signal_type==OUTPUT_SIGNAL)
                    monitors[input_checkbox.size()+ID]->setWindowTitle(connectOutputDevices[current_output_device]->getDeviceName()+"变量"+newText);
                //monitors[i]->setWindowTitle(connectInputDevices[current_input_device]->getDeviceName()+"变量"+input_checkbox[i]->text());
            }
        });
        contextMenu.addAction(editAction); // 将编辑动作添加到上下文菜单中
        contextMenu.exec(checkbox->mapToGlobal(pos)); // 在鼠标位置显示上下文菜单
    }
}
void MainWindow::init_mainwindow()
{
    QLayoutItem *child;

    while ((child = ui->gridLayout_input->takeAt(0)) != nullptr)//删除gridLayout_input的所有元素
    {
        delete child->widget();
        delete child;
    }

    while ((child = ui->gridLayout_output->takeAt(0)) != nullptr)//删除gridLayout_output的所有元素
    {
        delete child->widget();
        delete child;
    }

    for(auto i:monitors)
    {
        delete i;
        i=nullptr;
    }
    int graphCount = ui->customPlot->graphCount();
    for (int i = 0; i < graphCount; ++i) {

        ui->customPlot->removeGraph(0); // 从 QCustomPlot 中移除曲线

    }
    monitors.clear();
    input_checkbox.clear();
    output_checkbox.clear();
    input_label.clear();
    output_label.clear();
}

void MainWindow::startCamera()
{
    // 打开网络摄像头（需要根据实际情况修改地址）
    QString url = ui->videoStream->text();
//    cap.open(url.toStdString());
//    if (!cap.isOpened()) {
//        qDebug() << "Failed to open camera.";
//        QMessageBox::warning(this,tr("视频流"),tr("打开失败，请检查视频流地址"),QMessageBox::Ok);
//    }

//    // 启动定时器
//    picTimer->start(20);
    emit vs->threadHandler->url(url);
    emit vs->threadHandler->state(true);
//    vm = new VideoMonitors(this);
//    vm->setVisible(true);
//    vm->setWindowTitle(tr("大屏视频图像"));

}

void MainWindow::stopCamera()
{
//    // 停止定时器
//    picTimer->stop();

//    // 关闭摄像头
//    cap.release();
    emit vs->threadHandler->state(false);
}

void MainWindow::updateFrame(cv::Mat frame)
{

    double scaleFactor = static_cast<double>(std::max(ui->videoLabel->width(), ui->videoLabel->height())) / std::max(frame.cols, frame.rows);

//    // 调整图像大小
//    cv::Mat resizedImage;
//    cv::resize(frame, resizedImage, cv::Size(), scaleFactor, scaleFactor);


    // 将OpenCV图像转换为Qt图像
    QImage* qImage = new QImage((const uchar*) frame.data, frame.cols, frame.rows,  frame.cols*3 ,QImage::Format_BGR888);


    // 将Qt图像保存为pixmap，并设置到QLabel中
    ui->videoLabel->setPixmap(QPixmap::fromImage(qImage->scaled(frame.cols*scaleFactor,frame.rows*scaleFactor)));
    delete qImage;
}

void MainWindow::on_getURL_clicked()
{
   QString url = getPlayUrl(this);
   if(url!=tr(""))
       ui->videoStream->setText(url);
   qDebug()<<url;
}

void MainWindow::on_test_clicked()
{
//    ADSThread* ads = new ADSThread("5.69.185.74.1.1",350);
//    ads->connectToPlc();
//    char test_var[] = "NC_Obj1 (Module1).Outputs.Control";
//    float data2read = 0;
////    ads->writeData(data2read,test_var,sizeof(test_var));
//    ads->readData(data2read,test_var,sizeof(test_var));
//    qDebug() << data2read;

}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QMainWindow::mousePressEvent(event);
    if(nullptr == event)
        return ;
    if(event->button() == Qt::LeftButton)
    {
        isDragging = true;
        mouse_startPoint = event->globalPos() ;
        window_top_left_point = this->frameGeometry().topLeft();
    }
}



void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QMainWindow::mouseMoveEvent(event);
    if(nullptr == event)
    {
        return ;
    }
    if(isDragging)
    {
        QPoint distance = event->globalPos() - mouse_startPoint;
        this->move(window_top_left_point + distance);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QMainWindow::mouseReleaseEvent(event);
    if(nullptr == event)
        return;
    if(event->button() == Qt::LeftButton)
        isDragging = false ;
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
    }
    else if(event->button() == Qt::RightButton){
        ADSThread* ads = new ADSThread("5.69.185.74.1.1",349);
        ads->connectToPlc();
        char test_var[] = "NC_Obj1 (Module1).ADS.laser_distance[0]";
        short data2read = 0;
    //    ads->writeData(data2read,test_var,sizeof(test_var));
        ads->readData(data2read,test_var,sizeof(test_var));
        qDebug() << data2read;
    }
}

