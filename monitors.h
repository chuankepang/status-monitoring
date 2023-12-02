#ifndef MONITORS_H
#define MONITORS_H


#include <QWidget>

namespace Ui {
class Monitors;
}

class Monitors : public QWidget
{
    Q_OBJECT

public:
    explicit Monitors(QWidget *parent = nullptr);
    ~Monitors();
    Ui::Monitors* get_ui();
    void draw(double key, double value);
    void set_graph_color(QColor lineColor);

private:
    Ui::Monitors *ui;
};

#endif // MONITORS_H
