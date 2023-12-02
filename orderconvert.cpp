#include "orderconvert.h"
#include "ui_orderconvert.h"
#include "qcustomplot.h"
OrderConvert::OrderConvert(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderConvert)
{
    ui->setupUi(this);
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    //设定X轴为时间
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->original_signal->xAxis->setTicker(timeTicker);
    ui->output_signal->xAxis->setTicker(timeTicker);
   //所有曲线可见
    ui->original_signal->axisRect()->setupFullAxesBox();
    ui->output_signal->axisRect()->setupFullAxesBox();




    //配置上和右坐标轴来显示刻度,但是不显示数字
    //xAxis, yAxis, xAxis2, yAxis2，分别对应下、左、上、右。
    ui->original_signal->xAxis2->setVisible(true);
    ui->original_signal->xAxis2->setTickLabels(false);
    ui->original_signal->yAxis2->setVisible(true);
    ui->original_signal->yAxis2->setTickLabels(false);

    ui->output_signal->xAxis2->setVisible(true);
    ui->output_signal->xAxis2->setTickLabels(false);
    ui->output_signal->yAxis2->setVisible(true);
    ui->output_signal->yAxis2->setTickLabels(false);

    //修改左和底坐标轴,使之与右和上坐标轴始终匹配
    connect(ui->original_signal->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->original_signal->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->original_signal->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->original_signal->yAxis2, SLOT(setRange(QCPRange)));

    connect(ui->output_signal->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->output_signal->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->output_signal->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->output_signal->yAxis2, SLOT(setRange(QCPRange)));

    ui->original_signal->rescaleAxes();
    ui->output_signal->rescaleAxes();
    //使自适应范围


    //允许用户用鼠标缩放
    ui->original_signal->setInteractions(QCP::iRangeDrag |QCP::iRangeZoom);
    ui->original_signal->selectionRect()->setPen(QPen(Qt::black,1,Qt::DashLine));//设置选框的样式：虚线
    ui->original_signal->selectionRect()->setBrush(QBrush(QColor(0,0,100,50)));//设置选框的样式：半透明浅蓝
    ui->original_signal->setSelectionRectMode(QCP::SelectionRectMode::srmZoom);

    ui->output_signal->setInteractions(QCP::iRangeDrag |QCP::iRangeZoom);
    ui->output_signal->selectionRect()->setPen(QPen(Qt::black,1,Qt::DashLine));//设置选框的样式：虚线
    ui->output_signal->selectionRect()->setBrush(QBrush(QColor(0,0,100,50)));//设置选框的样式：半透明浅蓝
    ui->output_signal->setSelectionRectMode(QCP::SelectionRectMode::srmZoom);

    position_alpha=1;//位置系数为1

}

OrderConvert::~OrderConvert()
{
    delete ui;
}
/*
在UI设计界面，选择按钮“转到槽”，自动生成以下槽函数实现模板
void 类名::on_按钮名_信号()
{
}
*/
void OrderConvert::on_exit_clicked()
{
    this->back();//产生一个自定义的信号，从当前页面返回
}

void OrderConvert::init_window()
{
    QLayoutItem *child;

    while ((child = ui->gridLayout_signal->takeAt(0)) != nullptr)//删除gridLayout_input的所有元素
    {
        delete child->widget();
        delete child;
    }
    int graphCount = ui->original_signal->graphCount();
    for (int i = 0; i < graphCount; ++i) {

        ui->original_signal->removeGraph(0); // 从 QCustomPlot 中移除曲线

    }
    graphCount=ui->output_signal->graphCount();
    for (int i = 0; i < graphCount; ++i) {
        ui->output_signal->removeGraph(0); // 从 QCustomPlot 中移除曲线
    }
    signal_checkbox.clear();
}

void OrderConvert::draw_window(const QMap<QString, QVector<QPair<double,double>>> &signal_list)
{
    int input_signal_num=signal_list.size();
    for(int i=0;i<input_signal_num;++i)//处理输入设备信号的显示效果
    {
       ui->original_signal->addGraph();//添加原始信号线条
        // 随机分配线条颜色
       QColor lineColor = QColor::fromRgb(QRandomGenerator::global()->bounded(256),
                                           QRandomGenerator::global()->bounded(256),
                                           QRandomGenerator::global()->bounded(256));

       ui->original_signal->graph(i)->setPen(QPen(lineColor));
       ui->original_signal->graph(i)->setVisible(false);//默认均不可见

       ui->output_signal->addGraph();//添加原始信号线条
       ui->output_signal->graph(i)->setPen(QPen(lineColor));
       ui->output_signal->graph(i)->setVisible(false);//默认均不可见
    }

    for (int i = 0; i < input_signal_num; ++i)
    {
       QCheckBox *checkBox = new QCheckBox(QString(signal_list.keys().at(i)));

       signal_checkbox.append(checkBox);

       connect(checkBox, &QCheckBox::stateChanged, [=](int state) {
           ui->original_signal->graph(i)->setVisible(state == Qt::Checked);
           ui->original_signal->yAxis->rescale(true);//调整Y轴的范围，使之能显示出所有的曲线的Y值
           ui->original_signal->replot();

           ui->output_signal->graph(i)->setVisible(state == Qt::Checked);
           ui->output_signal->yAxis->rescale(true);//调整Y轴的范围，使之能显示出所有的曲线的Y值
           ui->output_signal->replot();
       });//将复选框和线条连接起来


//       QLabel *label = new QLabel();
//               input_label.append(label);
       ui->gridLayout_signal->addWidget(checkBox, i, 0);
//       ui->gridLayout_signal->addWidget(label, i, 1);
    }

}

Ui::OrderConvert *OrderConvert::getui()
{
    return ui;
}
void OrderConvert::draw_graph(int graph_ID,double key,double noise_value, double value)
{
    ui->original_signal->graph(graph_ID)->addData(key,noise_value);//在变量监视器页面绘图区也添加新的坐标
    ui->original_signal->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->original_signal->yAxis->rescale(true);//调整Y轴的范围，使之能显示出所有的曲线的Y
    ui->original_signal->replot();

    ui->output_signal->graph(graph_ID)->addData(key,value);//在变量监视器页面绘图区也添加新的坐标
    ui->output_signal->xAxis->setRange(key, 8, Qt::AlignRight);

    ui->output_signal->yAxis->rescale(true);//调整Y轴的范围，使之能显示出所有的曲线的Y
    ui->output_signal->replot();
}

void OrderConvert::change_checkbox_text(const int ID, QString new_text)
{
    signal_checkbox[ID]->setText(new_text);
}


const double OrderConvert::get_position_alpha()
{
    return position_alpha;
}


void OrderConvert::on_save_clicked()
{
    position_alpha=ui->position_alpha_lineEdit->text().toDouble();
    QMessageBox::information(this,QString("设备分析"),tr("保存成功"),QMessageBox::Ok);
}

void OrderConvert::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
