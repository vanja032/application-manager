#pragma once

#include <QtWidgets/QWidget>

#include "AppCard.h"

class QPushButton;
class QVBoxLayout;

class AppDetailPage : public QWidget
{
    Q_OBJECT

  public:
    explicit AppDetailPage(QWidget *parent = nullptr);

    void setModel(const AppCardModel &model);
    void setLaunchError(const QString &text);

  signals:
    void backRequested();
    void installRequested(const QString &packageId);
    void uninstallRequested(const QString &packageId);
    void updateRequested(const QString &packageId);
    void launchRequested(const QString &packageId);

  private:
    void rebuild();
    QPushButton *createButton(const QString &text, bool primary) const;

    AppCardModel m_model;
    QString m_launchError;
    QVBoxLayout *m_root = nullptr;
    QWidget *m_body = nullptr;
};
