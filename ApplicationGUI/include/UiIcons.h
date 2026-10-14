#pragma once

#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>

namespace UiIcons
{

enum class Glyph
{
    Search,
    Grid,
    Refresh,
    Settings,
    Moon,
    Info,
    Check,
    Dot,
    Package,
    Close
};

QPixmap pixmap(Glyph glyph, const QColor &color, int size);
QIcon icon(Glyph glyph, const QColor &color, int size);
QPixmap windowsLogo(int size);

} // namespace UiIcons
