#include "mainwindow.h"
#include <QApplication>
#include "devicesscan.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
