#pragma once

#include <QtCore/QString>
#include <QtWidgets/QWidget>

enum class AppAvailability
{
    NotInstalled,
    Installed,
    UpdateAvailable,
    Queued,
    Downloading,
    Installing,
    Updating,
    UninstallQueued,
    Uninstalling,
    Error
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
    QString errorText;
    bool retryUninstall = false;
};

class CircularProgress;
class QHBoxLayout;
class QLabel;
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

    bool retriesUninstall() const;

    void applyStatus(AppAvailability availability, int percent, int downloadedMb, int totalMb, const QString &errorText,
                     bool retryUninstall = false);
    void setDarkMode(bool dark);

  signals:
    void installRequested(const QString &packageId);
    void uninstallRequested(const QString &packageId);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    void rebuildStatusAndActions();
    QWidget *createStatusPanel();
    QWidget *createActionPanel();
    QPushButton *createButton(const QString &text, bool primary) const;

    AppCardModel m_model;
    QHBoxLayout *m_layout = nullptr;
    QWidget *m_statusPanel = nullptr;
    QWidget *m_actionPanel = nullptr;
    QLabel *m_phaseLabel = nullptr;
    QLabel *m_bytesLabel = nullptr;
    CircularProgress *m_progress = nullptr;
    bool m_dark = false;
};
