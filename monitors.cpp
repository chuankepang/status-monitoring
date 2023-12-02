#include "monitors.h"
#include "ui_monitors.h"

Monitors::Monitors(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Monitors)
{

    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog);//将监视器窗口设置为单独页面

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    //设定X轴为时间
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->display->xAxis->setTicker(timeTicker);
   //所有曲线可见
    ui->display->axisRect()->setupFullAxesBox();
    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
//    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));//在这个窗口不需要


    //配置上和右坐标轴来显示刻度,但是不显示数字
    //xAxis, yAxis, xAxis2, yAxis2，分别对应下、左、上、右。
    ui->display->xAxis2->setVisible(true);
    ui->display->xAxis2->setTickLabels(false);
    ui->display->yAxis2->setVisible(true);
    ui->display->yAxis2->setTickLabels(false);

    //修改左和底坐标轴,使之与右和上坐标轴始终匹配
    connect(ui->display->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->display->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->display->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->display->yAxis2, SLOT(setRange(QCPRange)));

    ui->display->rescaleAxes();
    //使自适应范围


    //允许用户用鼠标缩放
    ui->display->setInteractions(QCP::iRangeDrag |QCP::iRangeZoom);
    ui->display->selectionRect()->setPen(QPen(Qt::black,1,Qt::DashLine));//设置选框的样式：虚线
    ui->display->selectionRect()->setBrush(QBrush(QColor(0,0,100,50)));//设置选框的样式：半透明浅蓝
    ui->display->setSelectionRectMode(QCP::SelectionRectMode::srmZoom);


    ui->display->addGraph();//添加线条，数量为所有输入输出信号之和

    ui->display->graph(0)->setVisible(true);//默认可见

}

Monitors::~Monitors()
{
    delete ui;
}

Ui::Monitors* Monitors::get_ui()
{
    return ui;
}

void Monitors::draw(double key, double value)//绘制图像
{
    ui->display->graph(0)->addData(key,value);//在变量监视器页面绘图区也添加新的坐标
    ui->display->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->display->replot();
//    ui->display->yAxis->rescale(true);//调整Y轴的范围，使之能显示出所有的曲线的Y
}
void Monitors::set_graph_color(QColor lineColor)
{
    ui->display->graph(0)->setPen(QPen(lineColor));
}
