#pragma once

#include <cstdint>
#include <vector>

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtWidgets/QMainWindow>

#include "AppCard.h"
#include "WingetService.h"

#include "ui_MainWindow.h"

class QButtonGroup;
class QEvent;
class QFrame;
class QLabel;
class QShowEvent;
class QToolButton;
class ToggleSwitch;

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

  protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

  private:
    enum class Section
    {
        Discover,
        Installed,
        Updates,
        Settings,
        About
    };

    enum class StatusFilter
    {
        All = 0,
        Installed = 1,
        NotInstalled = 2
    };

    void configureNavigation();
    void configureSearchField();
    void configureIcons();
    void applyTheme(bool dark);
    void showSection(Section section);
    void startSearch();
    void startInstalledList();
    void startUpdatesList();
    void startInstall(const QString &packageId);
    void startUninstall(const QString &packageId);
    void startUpdate(const QString &packageId);
    void showPackageStatus(const QString &packageId, AppAvailability availability, int percent, int downloadedMb,
                           int totalMb, const QString &errorText, bool retryUninstall = false,
                           bool retryUpdate = false, const QString &installedVersion = QString());
    AppCard *findCard(const QString &packageId) const;
    void showStatus(const QString &text);
    void clearCards();
    void applySearchCompletion(const SearchCompletion &completion);
    void refreshAppList();
    void positionUpdatesBadge();
    bool cardMatches(const AppCard *card) const;
    QString emptyListMessage() const;

    Ui::MainWindowClass ui;
    QButtonGroup *m_navGroup = nullptr;
    QButtonGroup *m_filterGroup = nullptr;
    ToggleSwitch *m_darkModeSwitch = nullptr;
    QFrame *m_searchFrame = nullptr;
    QLabel *m_searchIcon = nullptr;
    QToolButton *m_searchClear = nullptr;
    QLabel *m_updatesBadge = nullptr;
    std::vector<AppCard *> m_cards;
    Section m_section = Section::Discover;
    Section m_loadedSection = Section::Discover;
    bool m_listLoaded = false;
    bool m_darkMode = false;
    struct InstallUiState
    {
        AppAvailability availability = AppAvailability::NotInstalled;
        int progressPercent = 0;
        int downloadedMb = 0;
        int totalMb = 0;
        QString error;
        QString installedVersion;
        bool retryUninstall = false;
        bool retryUpdate = false;
    };

    std::uint64_t m_searchGeneration = 0;
    QString m_statusText;
    QHash<QString, InstallUiState> m_installUi;
    WingetService m_service;
};
