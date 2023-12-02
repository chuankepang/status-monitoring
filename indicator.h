#ifndef INDICATOR_H
#define INDICATOR_H

#include <QLabel>

class IndicatorLabel : public QLabel {
    Q_OBJECT
public:
    enum class Status {
        Ok,
        Busy,
        Warning,
        Error,
        Offline
    };

    explicit IndicatorLabel(QWidget *parent = nullptr);
    void setStatus(Status status);

private:
    QString m_okColor;
    QString m_busyColor;
    QString m_warningColor;
    QString m_errorColor;
    QString m_offlineColor;
};

#endif // INDICATOR_H
