#ifndef ORDERCONVERT_H
#define ORDERCONVERT_H


#include <QWidget>
#include <devices.h>
#include "QCheckBox"
#include <QStyleOption>
#include <QPainter>
#include <QKeyEvent>
namespace Ui {
class OrderConvert;
}

class OrderConvert : public QWidget
{
    Q_OBJECT

public:
    explicit OrderConvert(QWidget *parent = nullptr);
    void init_window();
    void draw_window(const QMap<QString, QVector<QPair<double,double>>> &signal);
    ~OrderConvert();
    Ui::OrderConvert *getui();
    void draw_graph(int graph_ID,double key,double noise_value, double value);
    void change_checkbox_text(const int ID,QString new_text);
    const double get_position_alpha();
    void paintEvent(QPaintEvent *event) override;

signals:
    void back();//自定义信号的声明
    void stop();

private slots:
    void on_exit_clicked();//自动生成的槽函数声明

    void on_save_clicked();
    void keyPressEvent(QKeyEvent *event) override {
        if(event->key() == Qt::Key_Return)
        {
            emit stop();
        }
    } ;

private:
    Ui::OrderConvert *ui;
    QVector<QCheckBox*> signal_checkbox;
    double position_alpha;
};

#endif // ORDERCONVERT_H
