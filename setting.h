#ifndef SETTING_H
#define SETTING_H


#include <QWidget>
#include <QStyleOption>
#include <QPainter>
#include <QKeyEvent>

namespace Ui {
class Setting;
}

class Setting : public QWidget
{
    Q_OBJECT

public:
    explicit Setting(QWidget *parent = nullptr);
    ~Setting();
    QString getIP(){return ip;};
    quint16 getPort() {return port;};
    void paintEvent(QPaintEvent *event) override;

signals:
    void back();//自定义信号的声明
    void stop();

private slots:
    void on_exit_clicked();//自动生成的槽函数声明


    void on_save_clicked();

private:
    Ui::Setting *ui;
    QString ip;
    quint16 port;
    void keyPressEvent(QKeyEvent *event) override {
        if(event->key() == Qt::Key_Return)
        {
            emit stop();
        }
    } ;
};

#endif // SETTING_H
