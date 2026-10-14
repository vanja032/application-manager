#include "UiIcons.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QPainter>
#include <QtGui/QScreen>

#include <functional>

namespace
{

QPixmap createPixmap(int size, const std::function<void(QPainter &)> &draw)
{
    qreal devicePixelRatio = 1.0;
    if (const QScreen *screen = QGuiApplication::primaryScreen())
    {
        devicePixelRatio = screen->devicePixelRatio();
    }

    QPixmap image(qRound(size * devicePixelRatio), qRound(size * devicePixelRatio));
    image.setDevicePixelRatio(devicePixelRatio);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    draw(painter);
    return image;
}

void drawMdl2(QPainter &painter, const QColor &color, const QRectF &rect, ushort codePoint)
{
    painter.save();
    QFont font(QStringLiteral("Segoe MDL2 Assets"));
    font.setPixelSize(qMax(8, qRound(rect.height() * 0.92)));
    painter.setFont(font);
    painter.setPen(color);
    painter.drawText(rect, Qt::AlignCenter, QString(QChar(codePoint)));
    painter.restore();
}

void drawSearch(QPainter &painter, const QColor &color, const QRectF &rect)
{
    drawMdl2(painter, color, rect, 0xE721);
}

void drawGrid(QPainter &painter, const QColor &color, const QRectF &rect)
{
    drawMdl2(painter, color, rect, 0xE71D);
}

void drawRefresh(QPainter &painter, const QColor &color, const QRectF &rect)
{
    drawMdl2(painter, color, rect, 0xE895);
}

void drawSettings(QPainter &painter, const QColor &color, const QRectF &rect)
{
    drawMdl2(painter, color, rect, 0xE713);
}

void drawMoon(QPainter &painter, const QColor &color, const QRectF &rect)
{
    drawMdl2(painter, color, rect, 0xE708);
}

void drawInfo(QPainter &painter, const QColor &color, const QRectF &rect)
{
    drawMdl2(painter, color, rect, 0xE946);
}

void drawCheck(QPainter &painter, const QColor &color, const QRectF &rect)
{
    const qreal scale = rect.width() / 24.0;
    painter.save();
    painter.translate(rect.topLeft());
    painter.scale(scale, scale);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(QRectF(2.0, 2.0, 20.0, 20.0));
    painter.setPen(QPen(Qt::white, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(QPointF(7.0, 12.2), QPointF(10.6, 15.8));
    painter.drawLine(QPointF(10.6, 15.8), QPointF(17.2, 8.4));
    painter.restore();
}

void drawDot(QPainter &painter, const QColor &color, const QRectF &rect)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    const qreal diameter = rect.width() * 0.42;
    painter.drawEllipse(QRectF(rect.center().x() - diameter / 2.0, rect.center().y() - diameter / 2.0, diameter, diameter));
}

void drawClose(QPainter &painter, const QColor &color, const QRectF &rect)
{
    painter.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap));
    const QRectF mark = rect.adjusted(rect.width() * 0.22, rect.height() * 0.22, -rect.width() * 0.22, -rect.height() * 0.22);
    painter.drawLine(mark.topLeft(), mark.bottomRight());
    painter.drawLine(mark.topRight(), mark.bottomLeft());
}

void drawPackage(QPainter &painter, const QColor &color, const QRectF &rect)
{
    const QPointF center = rect.center();
    const qreal width = rect.width() * 0.30;
    const qreal height = rect.height() * 0.16;
    const QPointF top(center.x(), center.y() - height * 2.15);
    const QPointF right(center.x() + width, center.y() - height);
    const QPointF bottom(center.x(), center.y());
    const QPointF left(center.x() - width, center.y() - height);
    const QPointF bottomLeft(center.x() - width, center.y() + height);
    const QPointF bottomRight(center.x() + width, center.y() + height);
    const QPointF bottomCenter(center.x(), center.y() + height * 2.15);

    painter.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawPolygon(QPolygonF() << top << right << bottom << left);
    painter.drawPolygon(QPolygonF() << left << bottom << bottomCenter << bottomLeft);
    painter.drawPolygon(QPolygonF() << right << bottom << bottomCenter << bottomRight);
}

} // namespace

QPixmap UiIcons::pixmap(Glyph glyph, const QColor &color, int size)
{
    return createPixmap(size, [&](QPainter &painter) {
        const QRectF rect(0.0, 0.0, size, size);
        switch (glyph)
        {
        case Glyph::Search:
            drawSearch(painter, color, rect);
            break;
        case Glyph::Grid:
            drawGrid(painter, color, rect);
            break;
        case Glyph::Refresh:
            drawRefresh(painter, color, rect);
            break;
        case Glyph::Settings:
            drawSettings(painter, color, rect);
            break;
        case Glyph::Moon:
            drawMoon(painter, color, rect);
            break;
        case Glyph::Info:
            drawInfo(painter, color, rect);
            break;
        case Glyph::Check:
            drawCheck(painter, color, rect);
            break;
        case Glyph::Dot:
            drawDot(painter, color, rect);
            break;
        case Glyph::Package:
            drawPackage(painter, color, rect);
            break;
        case Glyph::Close:
            drawClose(painter, color, rect);
            break;
        }
    });
}

QIcon UiIcons::icon(Glyph glyph, const QColor &color, int size)
{
    return QIcon(pixmap(glyph, color, size));
}

QPixmap UiIcons::windowsLogo(int size)
{
    return createPixmap(size, [&](QPainter &painter) {
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setPen(Qt::NoPen);
        const qreal gap = size * 0.08;
        const qreal tile = (size - gap) / 2.0;
        painter.setBrush(QColor(QStringLiteral("#F25022")));
        painter.drawRect(QRectF(0.0, 0.0, tile, tile));
        painter.setBrush(QColor(QStringLiteral("#7FBA00")));
        painter.drawRect(QRectF(tile + gap, 0.0, tile, tile));
        painter.setBrush(QColor(QStringLiteral("#00A4EF")));
        painter.drawRect(QRectF(0.0, tile + gap, tile, tile));
        painter.setBrush(QColor(QStringLiteral("#FFB900")));
        painter.drawRect(QRectF(tile + gap, tile + gap, tile, tile));
    });
}
