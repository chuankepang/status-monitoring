#include "deviceconsole.h"
#include "ui_deviceconsole.h"

DeviceConsole::DeviceConsole(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceConsole)
{
    ui->setupUi(this);
}

DeviceConsole::~DeviceConsole()
{
    delete ui;
}
