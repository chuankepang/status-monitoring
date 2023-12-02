#include "setting.h"
#include "ui_setting.h"
#include <QDebug>
#include <QMessageBox>

Setting::Setting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Setting)
{
    ui->setupUi(this);
    ip = ui->IP->toPlainText();
    port = ui->Port->toPlainText().toUInt();
    qDebug() << ip << port;
}

Setting::~Setting()
{
    delete ui;
}
/*
在UI设计界面，选择按钮“转到槽”，自动生成以下槽函数实现模板
void 类名::on_按钮名_信号()
{
}
*/
void Setting::on_exit_clicked()
{
    this->back();//产生一个自定义的信号，从当前页面返回
}

void Setting::on_save_clicked()
{
    ip = ui->IP->toPlainText();
    port = ui->Port->toPlainText().toUInt();
    QMessageBox::information(this,QString("网络设置"),tr("保存成功"),QMessageBox::Ok);
}

void Setting::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

