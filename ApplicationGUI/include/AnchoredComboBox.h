#pragma once

#include <QtWidgets/QComboBox>

class AnchoredComboBox : public QComboBox
{
  public:
    explicit AnchoredComboBox(QWidget *parent = nullptr);

  protected:
    void showPopup() override;
};
