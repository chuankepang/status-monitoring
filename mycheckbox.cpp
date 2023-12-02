#include "mycheckbox.h"

MyCheckBox::MyCheckBox(QWidget *parent) : QCheckBox(parent)
{
    // 初始化ID为0
    ID = 0;
}
MyCheckBox::MyCheckBox(const QString& text, QWidget *parent)
    : QCheckBox(text, parent)
{
    // 初始化ID为0
    ID = 0;
}

void MyCheckBox::setID(int id)
{
    ID = id;
}

void MyCheckBox::set_signal_type(bool type)
{
    signal_type=type;
}

int MyCheckBox::getID() const
{
    return ID;
}

bool MyCheckBox::get_signal_type() const
{
    return signal_type;
}
