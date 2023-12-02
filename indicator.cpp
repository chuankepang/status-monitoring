#include "indicator.h"
#include <QSizePolicy>

IndicatorLabel::IndicatorLabel(QWidget *parent)
    : QLabel(parent)
{
    // 设置默认样式和提示文本
    setFixedSize(16, 16);
//    setAlignment(Qt::AlignCenter);
    setToolTip(tr("Status"));
    setText(tr(""));

    m_okColor = "#00b200";
    m_busyColor = "#c8a800";
    m_warningColor = "#ff8000";
    m_errorColor = "#ff0000";
    m_offlineColor = "#808080";

    setStatus(Status::Offline);
}

void IndicatorLabel::setStatus(Status status)
{
    // 根据状态设置不同颜色的背景
    switch (status) {
    case Status::Ok:
        setStyleSheet(QString("background-color: %1;border-radius: 8px").arg(m_okColor));
        setToolTip(tr("Online"));
        break;
    case Status::Busy:
        setStyleSheet(QString("background-color: %1;border-radius: 8px").arg(m_busyColor));
        setToolTip(tr("Busy"));
        break;
    case Status::Warning:
        setStyleSheet(QString("background-color: %1;border-radius: 8px").arg(m_warningColor));
        setToolTip(tr("Warning"));
        break;
    case Status::Error:
        setStyleSheet(QString("background-color: %1;border-radius: 8px").arg(m_errorColor));
        setToolTip(tr("Error"));
        break;
    case Status::Offline:
        setStyleSheet(QString("background-color: %1;border-radius: 8px").arg(m_offlineColor));
        setToolTip(tr("Offline"));
        break;
    }
}
