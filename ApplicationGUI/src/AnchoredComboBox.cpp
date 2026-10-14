#include "AnchoredComboBox.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtWidgets/QAbstractItemView>

AnchoredComboBox::AnchoredComboBox(QWidget *parent) : QComboBox(parent)
{
}

void AnchoredComboBox::showPopup()
{
    QComboBox::showPopup();

    QAbstractItemView *list = view();
    QWidget *popup = list ? list->parentWidget() : nullptr;
    if (!popup || !popup->isVisible())
    {
        return;
    }

    list->scrollToTop();

    const int popupWidth = qMax(width(), popup->width());
    popup->resize(popupWidth, popup->height());

    QPoint position = mapToGlobal(QPoint(0, height()));
    const QScreen *display = screen();
    if (!display)
    {
        display = QGuiApplication::primaryScreen();
    }

    if (display)
    {
        const QRect available = display->availableGeometry();
        if (position.x() + popup->width() > available.right())
        {
            position.setX(qMax(available.left(), available.right() - popup->width() + 1));
        }

        if (position.y() + popup->height() > available.bottom())
        {
            const int above = mapToGlobal(QPoint(0, 0)).y() - popup->height();
            if (above >= available.top())
            {
                position.setY(above);
            }
        }
    }

    popup->move(position);
}
