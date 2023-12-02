#ifndef DEVICECONSOLE_H
#define DEVICECONSOLE_H


#include <QWidget>

namespace Ui {
class DeviceConsole;
}

class DeviceConsole : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceConsole(QWidget *parent = nullptr);
    ~DeviceConsole();

private:
    Ui::DeviceConsole *ui;
};

#endif // DEVICECONSOLE_H
