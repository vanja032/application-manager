#include "CircularProgress.h"

#include <QtGui/QFont>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>

#include <algorithm>
#include <cmath>

CircularProgress::CircularProgress(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFixedSize(sizeHint());
}

int CircularProgress::value() const
{
    return m_value;
}

void CircularProgress::setValue(int value)
{
    const int clamped = std::clamp(value, 0, 100);
    if (m_value == clamped)
    {
        return;
    }

    m_value = clamped;
    update();
}

void CircularProgress::setDarkMode(bool dark)
{
    m_trackColor = dark ? QColor(QStringLiteral("#3A4250")) : QColor(QStringLiteral("#E6E8EE"));
    m_valueColor = dark ? QColor(QStringLiteral("#4DA3FF")) : QColor(QStringLiteral("#0078D4"));
    m_textColor = dark ? QColor(QStringLiteral("#F3F5F8")) : QColor(QStringLiteral("#202124"));
    update();
}

QSize CircularProgress::sizeHint() const
{
    return QSize(58, 58);
}

QSize CircularProgress::minimumSizeHint() const
{
    return sizeHint();
}

void CircularProgress::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const qreal penWidth = 5.0;
    const QRectF arcRect = QRectF(rect()).adjusted(penWidth, penWidth, -penWidth, -penWidth);

    painter.setPen(QPen(m_trackColor, penWidth, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(arcRect);

    painter.setPen(QPen(m_valueColor, penWidth, Qt::SolidLine, Qt::RoundCap));
    if (m_value >= 100)
    {
        painter.drawEllipse(arcRect);
    }
    else if (m_value > 0)
    {
        const int span = -static_cast<int>(std::lround((m_value / 100.0) * 360.0 * 16.0));
        painter.drawArc(arcRect, 90 * 16, span);
    }

    QFont font(QStringLiteral("Segoe UI"));
    font.setPixelSize(12);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(m_textColor);
    painter.drawText(rect(), Qt::AlignCenter, QString::number(m_value) + QLatin1Char('%'));
}
