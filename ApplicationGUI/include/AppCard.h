#pragma once

#include <QtCore/QString>
#include <QtWidgets/QWidget>

enum class AppAvailability
{
    NotInstalled,
    Installed,
    UpdateAvailable,
    Installing,
    Updating
};

struct AppCardModel
{
    QString name;
    QString packageId;
    QString source;
    AppAvailability availability = AppAvailability::NotInstalled;
    QString installedVersion;
    QString availableVersion;
    int progressPercent = 0;
    int downloadedMb = 0;
    int totalMb = 0;
    int catalogIndex = 0;
    bool hasAvailability = false;
};

class CircularProgress;
class QPushButton;

class AppCard : public QWidget
{
    Q_OBJECT

  public:
    explicit AppCard(const AppCardModel &model, QWidget *parent = nullptr);

    QString name() const;
    QString packageId() const;
    QString source() const;
    AppAvailability availability() const;
    bool hasAvailability() const;
    int catalogIndex() const;

    void setDarkMode(bool dark);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QWidget *createStatusPanel();
    QWidget *createActionPanel();
    QPushButton *createButton(const QString &text, bool primary) const;

    AppCardModel m_model;
    CircularProgress *m_progress = nullptr;
};
