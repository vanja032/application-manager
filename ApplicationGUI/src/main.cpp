#include "MainWindow.h"

#include <QtGui/QFont>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (QStyle *style = QStyleFactory::create(QStringLiteral("Fusion")))
    {
        app.setStyle(style);
    }

    QFont font(QStringLiteral("Segoe UI"));
    font.setPointSize(10);
    app.setFont(font);

    MainWindow window;
    window.show();
    return app.exec();
}
