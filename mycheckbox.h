#ifndef MYCHECKBOX_H
#define MYCHECKBOX_H
#define INPUT_SIGNAL true
#define OUTPUT_SIGNAL false


#include <QObject>
#include <QCheckBox>

class MyCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit MyCheckBox(QWidget *parent = nullptr);
    explicit MyCheckBox(const QString& text, QWidget *parent = nullptr);
    // 设置ID
    void setID(int id);
    void set_signal_type(bool type);

    // 获取ID
    int getID() const;
    bool get_signal_type() const;

signals:

private:
    int ID; // 私有成员变量ID
    bool signal_type;


};

#endif // MYCHECKBOX_H
