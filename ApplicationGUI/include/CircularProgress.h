#pragma once

#include <QtGui/QColor>
#include <QtWidgets/QWidget>

class CircularProgress : public QWidget
{
    Q_OBJECT

  public:
    explicit CircularProgress(QWidget *parent = nullptr);

    int value() const;
    void setValue(int value);
    void setDarkMode(bool dark);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    int m_value = 0;
    QColor m_trackColor = QColor(QStringLiteral("#E6E8EE"));
    QColor m_valueColor = QColor(QStringLiteral("#0078D4"));
    QColor m_textColor = QColor(QStringLiteral("#202124"));
};
