#include "MainWindow.h"

#include "AppCard.h"
#include "AppDetailPage.h"
#include "UiIcons.h"

#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore/QMetaObject>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QShowEvent>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QToolButton>

#include <algorithm>
#include <cstdint>
#include <functional>

class ToggleSwitch : public QWidget
{
  public:
    explicit ToggleSwitch(QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedSize(42, 22);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::TabFocus);
    }

    bool isChecked() const
    {
        return m_checked;
    }

    void setChecked(bool checked)
    {
        if (m_checked == checked)
        {
            return;
        }

        m_checked = checked;
        update();
        if (m_onToggled)
        {
            m_onToggled(m_checked);
        }
    }

    void setDarkMode(bool dark)
    {
        m_dark = dark;
        update();
    }

    void setOnToggled(std::function<void(bool)> callback)
    {
        m_onToggled = std::move(callback);
    }

  protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);

        const QColor track = m_checked ? QColor(QStringLiteral("#0078D4"))
                                        : (m_dark ? QColor(QStringLiteral("#4A5160")) : QColor(QStringLiteral("#D5D9E0")));
        painter.setBrush(track);
        painter.drawRoundedRect(rect(), height() / 2.0, height() / 2.0);

        const int knob = height() - 6;
        const int x = m_checked ? width() - knob - 3 : 3;
        painter.setBrush(Qt::white);
        painter.drawEllipse(x, 3, knob, knob);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && rect().contains(event->position().toPoint()))
        {
            setChecked(!m_checked);
        }
        QWidget::mouseReleaseEvent(event);
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {
            setChecked(!m_checked);
            return;
        }
        QWidget::keyPressEvent(event);
    }

  private:
    bool m_checked = false;
    bool m_dark = false;
    std::function<void(bool)> m_onToggled;
};

namespace
{

QString loadStyleSheet(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning("Failed to load stylesheet: %s", qPrintable(path));
        return {};
    }

    return QString::fromUtf8(file.readAll());
}

} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);

    ui.appScrollArea->viewport()->setAutoFillBackground(false);
    ui.appListContainer->setAutoFillBackground(false);

    m_darkModeSwitch = new ToggleSwitch(ui.sidebar);
    delete ui.darkModeLayout->replaceWidget(ui.darkModeSwitchHost, m_darkModeSwitch);
    ui.darkModeSwitchHost->hide();
    ui.searchRowLayout->setStretch(ui.searchRowLayout->indexOf(ui.searchEdit), 1);
    ui.appListLayout->setContentsMargins(0, 0, 2, 0);

    const auto configureDropdown = [](QComboBox *combo) {
        auto *popup = new QListView(combo);
        popup->setObjectName(QStringLiteral("comboPopup"));
        popup->setFrameShape(QFrame::NoFrame);
        popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        popup->setSpacing(0);
        combo->setView(popup);
        combo->setCursor(Qt::PointingHandCursor);
    };
    configureDropdown(ui.sourceCombo);
    configureDropdown(ui.sortCombo);

    m_updatesBadge = new QLabel(ui.updatesButton);
    m_updatesBadge->setObjectName(QStringLiteral("updatesBadge"));
    m_updatesBadge->setAlignment(Qt::AlignCenter);
    m_updatesBadge->setFixedSize(22, 22);
    m_updatesBadge->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_updatesBadge->setVisible(false);
    ui.updatesButton->installEventFilter(this);

    configureNavigation();
    applyTheme(false);
    showSection(Section::Discover);

    m_darkModeSwitch->setOnToggled([this](bool checked) { applyTheme(checked); });

    connect(ui.searchButton, &QPushButton::clicked, this, [this]() { startSearch(); });
    connect(ui.searchEdit, &QLineEdit::returnPressed, this, [this]() { startSearch(); });
    connect(ui.sourceCombo, &QComboBox::currentTextChanged, this, [this](const QString &) { refreshAppList(); });
    connect(ui.sortCombo, &QComboBox::currentTextChanged, this, [this](const QString &) { refreshAppList(); });

    m_detailPage = new AppDetailPage(ui.pages);
    ui.pages->addWidget(m_detailPage);
    connect(m_detailPage, &AppDetailPage::backRequested, this, &MainWindow::closeDetail);
    connect(m_detailPage, &AppDetailPage::installRequested, this, &MainWindow::startInstall, Qt::QueuedConnection);
    connect(m_detailPage, &AppDetailPage::uninstallRequested, this, &MainWindow::startUninstall, Qt::QueuedConnection);
    connect(m_detailPage, &AppDetailPage::updateRequested, this, &MainWindow::startUpdate, Qt::QueuedConnection);
    connect(m_detailPage, &AppDetailPage::launchRequested, this, &MainWindow::startLaunch, Qt::QueuedConnection);
    connect(m_detailPage, &AppDetailPage::pauseRequested, this, [this](const QString &packageId) {
        stopPackage(packageId, true);
    });
    connect(m_detailPage, &AppDetailPage::cancelRequested, this, [this](const QString &packageId) {
        stopPackage(packageId, false);
    });
}

MainWindow::~MainWindow() = default;

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    positionUpdatesBadge();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui.updatesButton
        && (event->type() == QEvent::Resize || event->type() == QEvent::Show || event->type() == QEvent::StyleChange))
    {
        positionUpdatesBadge();
    }

    if (m_searchFrame && watched == m_searchFrame && event->type() == QEvent::MouseButtonPress)
    {
        ui.searchEdit->setFocus();
    }

    if (m_searchFrame && watched == ui.searchEdit && (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut))
    {
        m_searchFrame->setProperty("focused", event->type() == QEvent::FocusIn);
        m_searchFrame->style()->unpolish(m_searchFrame);
        m_searchFrame->style()->polish(m_searchFrame);
        m_searchFrame->update();
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::configureNavigation()
{
    m_navGroup = new QButtonGroup(this);
    m_navGroup->setExclusive(true);
    m_navGroup->addButton(ui.discoverButton, static_cast<int>(Section::Discover));
    m_navGroup->addButton(ui.installedButton, static_cast<int>(Section::Installed));
    m_navGroup->addButton(ui.updatesButton, static_cast<int>(Section::Updates));
    m_navGroup->addButton(ui.settingsButton, static_cast<int>(Section::Settings));
    m_navGroup->addButton(ui.aboutButton, static_cast<int>(Section::About));

    m_filterGroup = new QButtonGroup(this);
    m_filterGroup->setExclusive(true);
    m_filterGroup->addButton(ui.filterAllButton, static_cast<int>(StatusFilter::All));
    m_filterGroup->addButton(ui.filterInstalledButton, static_cast<int>(StatusFilter::Installed));
    m_filterGroup->addButton(ui.filterNotInstalledButton, static_cast<int>(StatusFilter::NotInstalled));

    connect(m_navGroup, &QButtonGroup::idClicked, this, [this](int id) { showSection(static_cast<Section>(id)); });
    connect(m_filterGroup, &QButtonGroup::idClicked, this, [this](int) {
        if (m_section == Section::Discover || m_section == Section::Installed || m_section == Section::Updates)
        {
            refreshAppList();
        }
    });
}

void MainWindow::configureSearchField()
{
    const QColor color = m_darkMode ? QColor(QStringLiteral("#AEB6C2")) : QColor(QStringLiteral("#8B919A"));
    if (!m_searchFrame)
    {
        m_searchFrame = new QFrame(ui.searchRow);
        m_searchFrame->setObjectName(QStringLiteral("searchFrame"));
        m_searchFrame->setFixedHeight(40);
        m_searchFrame->installEventFilter(this);

        auto *row = new QHBoxLayout(m_searchFrame);
        row->setContentsMargins(16, 0, 14, 0);
        row->setSpacing(10);

        m_searchIcon = new QLabel(m_searchFrame);
        m_searchIcon->setObjectName(QStringLiteral("searchIcon"));
        m_searchIcon->setFixedSize(16, 16);
        m_searchIcon->setAlignment(Qt::AlignCenter);

        m_searchClear = new QToolButton(m_searchFrame);
        m_searchClear->setObjectName(QStringLiteral("searchClearButton"));
        m_searchClear->setCursor(Qt::PointingHandCursor);
        m_searchClear->setAutoRaise(true);
        m_searchClear->setFocusPolicy(Qt::NoFocus);
        m_searchClear->setFixedSize(16, 16);
        m_searchClear->setIconSize(QSize(14, 14));

        ui.searchEdit->setClearButtonEnabled(false);
        ui.searchEdit->setFrame(false);
        ui.searchEdit->setTextMargins(0, 0, 0, 0);
        ui.searchEdit->installEventFilter(this);

        delete ui.searchRowLayout->replaceWidget(ui.searchEdit, m_searchFrame);
        ui.searchRowLayout->setStretch(ui.searchRowLayout->indexOf(m_searchFrame), 1);
        row->addWidget(m_searchIcon, 0, Qt::AlignVCenter);
        row->addWidget(ui.searchEdit, 1);
        row->addWidget(m_searchClear, 0, Qt::AlignVCenter);

        connect(m_searchClear, &QToolButton::clicked, ui.searchEdit, &QLineEdit::clear);
        connect(ui.searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
            m_searchClear->setVisible(!text.isEmpty());
        });
        m_searchClear->setVisible(!ui.searchEdit->text().isEmpty());
    }

    m_searchIcon->setPixmap(UiIcons::pixmap(UiIcons::Glyph::Search, color, 16));
    m_searchClear->setIcon(UiIcons::icon(UiIcons::Glyph::Close, color, 14));
}

void MainWindow::configureIcons()
{
    const QColor glyph = m_darkMode ? QColor(QStringLiteral("#D0D6E0")) : QColor(QStringLiteral("#5C6570"));
    constexpr int kNavIconSize = 16;

    ui.brandIcon->setPixmap(UiIcons::windowsLogo(32));
    ui.aboutLogo->setPixmap(UiIcons::windowsLogo(48));
    setWindowIcon(QIcon(UiIcons::windowsLogo(32)));

    const auto applyNavIcon = [&](QPushButton *button, UiIcons::Glyph type) {
        auto *iconLabel = button->findChild<QLabel *>(QStringLiteral("navIcon"), Qt::FindDirectChildrenOnly);
        auto *textLabel = button->findChild<QLabel *>(QStringLiteral("navText"), Qt::FindDirectChildrenOnly);
        if (!iconLabel || !textLabel)
        {
            const QString label = button->text();
            button->setText(QString());
            button->setIcon(QIcon());

            auto *row = new QHBoxLayout(button);
            const int rightMargin = button == ui.updatesButton ? 34 : 12;
            row->setContentsMargins(12, 0, rightMargin, 0);
            row->setSpacing(10);

            iconLabel = new QLabel(button);
            iconLabel->setObjectName(QStringLiteral("navIcon"));
            iconLabel->setFixedSize(kNavIconSize, kNavIconSize);
            iconLabel->setAlignment(Qt::AlignCenter);
            iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

            textLabel = new QLabel(label, button);
            textLabel->setObjectName(QStringLiteral("navText"));
            textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            textLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

            row->addWidget(iconLabel, 0, Qt::AlignVCenter);
            row->addWidget(textLabel, 0, Qt::AlignVCenter);
            row->addStretch(1);
        }

        iconLabel->setPixmap(UiIcons::pixmap(type, glyph, kNavIconSize));
    };

    applyNavIcon(ui.discoverButton, UiIcons::Glyph::Search);
    applyNavIcon(ui.installedButton, UiIcons::Glyph::Grid);
    applyNavIcon(ui.updatesButton, UiIcons::Glyph::Refresh);
    applyNavIcon(ui.settingsButton, UiIcons::Glyph::Settings);
    applyNavIcon(ui.aboutButton, UiIcons::Glyph::Info);
    ui.darkModeIcon->setPixmap(UiIcons::pixmap(UiIcons::Glyph::Moon, glyph, kNavIconSize));
    configureSearchField();
}

void MainWindow::applyTheme(bool dark)
{
    m_darkMode = dark;

    QPalette palette;
    if (!dark)
    {
        palette.setColor(QPalette::Window, QColor(QStringLiteral("#F3F5F9")));
        palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#1A1A1A")));
        palette.setColor(QPalette::Base, QColor(QStringLiteral("#FFFFFF")));
        palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#F3F5F9")));
        palette.setColor(QPalette::Text, QColor(QStringLiteral("#1A1A1A")));
        palette.setColor(QPalette::Button, QColor(QStringLiteral("#FFFFFF")));
        palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#1A1A1A")));
        palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#0078D4")));
        palette.setColor(QPalette::HighlightedText, Qt::white);
        palette.setColor(QPalette::Link, QColor(QStringLiteral("#0078D4")));
        palette.setColor(QPalette::LinkVisited, QColor(QStringLiteral("#0078D4")));
        palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#8D939C")));
    }
    else
    {
        palette.setColor(QPalette::Window, QColor(QStringLiteral("#171B22")));
        palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#F3F5F8")));
        palette.setColor(QPalette::Base, QColor(QStringLiteral("#262C36")));
        palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#171B22")));
        palette.setColor(QPalette::Text, QColor(QStringLiteral("#F3F5F8")));
        palette.setColor(QPalette::Button, QColor(QStringLiteral("#2A3140")));
        palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#F3F5F8")));
        palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#4DA3FF")));
        palette.setColor(QPalette::HighlightedText, Qt::white);
        palette.setColor(QPalette::Link, QColor(QStringLiteral("#4DA3FF")));
        palette.setColor(QPalette::LinkVisited, QColor(QStringLiteral("#4DA3FF")));
        palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#9AA3B2")));
    }

    qApp->setPalette(palette);
    qApp->setStyleSheet(loadStyleSheet(dark ? QStringLiteral(":/styles-dark.qss") : QStringLiteral(":/styles.qss")));

    const QString linkColor = dark ? QStringLiteral("#4DA3FF") : QStringLiteral("#0078D4");
    ui.aboutLink->setText(
        QStringLiteral("<a href=\"https://vanjasretenovic.com\" style=\"color:%1;\">vanjasretenovic.com</a>")
            .arg(linkColor));

    if (m_darkModeSwitch)
    {
        m_darkModeSwitch->setDarkMode(dark);
    }

    configureIcons();
    for (AppCard *card : m_cards)
    {
        card->setDarkMode(dark);
    }
    positionUpdatesBadge();
}

void MainWindow::showSection(Section section)
{
    m_detailPackageId.clear();
    m_section = section;
    const bool discover = section == Section::Discover;
    const bool showList = discover || section == Section::Installed || section == Section::Updates;
    ui.searchRow->setVisible(discover);
    ui.filterAllButton->setVisible(discover);
    ui.filterInstalledButton->setVisible(discover);
    ui.filterNotInstalledButton->setVisible(discover);
    ui.filterRow->setVisible(showList);

    if (section == Section::Settings)
    {
        ui.pages->setCurrentWidget(ui.pageSettings);
        return;
    }
    if (section == Section::About)
    {
        ui.pages->setCurrentWidget(ui.pageAbout);
        return;
    }

    ui.pages->setCurrentWidget(ui.pageList);
    if (m_listLoaded && m_loadedSection == section)
    {
        refreshAppList();
        return;
    }

    if (section == Section::Installed)
    {
        startInstalledList();
    }
    else if (section == Section::Updates)
    {
        startUpdatesList();
    }
    else if (!ui.searchEdit->text().trimmed().isEmpty())
    {
        startSearch();
    }
    else
    {
        startFeaturedList();
    }
}

void MainWindow::startSearch()
{
    const QString query = ui.searchEdit->text().trimmed();
    if (query.isEmpty())
    {
        startFeaturedList();
        return;
    }

    const std::uint64_t generation = ++m_searchGeneration;
    m_listLoaded = false;
    showStatus(QStringLiteral("Searching..."));

    m_service.SearchAppsAsync(query.toStdWString(), [this, generation](SearchCompletion completion) {
        QMetaObject::invokeMethod(
            this,
            [this, generation, completion]() {
                if (generation != m_searchGeneration)
                {
                    return;
                }

                applySearchCompletion(completion);
            },
            Qt::QueuedConnection);
    });
}

void MainWindow::startFeaturedList()
{
    const std::uint64_t generation = ++m_searchGeneration;
    m_listLoaded = false;
    showStatus(QStringLiteral("Loading featured applications..."));

    m_service.ListFeaturedAppsAsync([this, generation](SearchCompletion completion) {
        QMetaObject::invokeMethod(
            this,
            [this, generation, completion]() {
                if (generation != m_searchGeneration)
                {
                    return;
                }

                applySearchCompletion(completion);
            },
            Qt::QueuedConnection);
    });
}

void MainWindow::startInstalledList()
{
    const std::uint64_t generation = ++m_searchGeneration;
    m_listLoaded = false;
    showStatus(QStringLiteral("Loading installed applications..."));

    m_service.ListInstalledAppsAsync([this, generation](SearchCompletion completion) {
        QMetaObject::invokeMethod(
            this,
            [this, generation, completion]() {
                if (generation != m_searchGeneration)
                {
                    return;
                }

                applySearchCompletion(completion);
            },
            Qt::QueuedConnection);
    });
}

void MainWindow::startUpdatesList()
{
    const std::uint64_t generation = ++m_searchGeneration;
    m_listLoaded = false;
    showStatus(QStringLiteral("Checking for updates..."));

    m_service.ListUpdatesAsync([this, generation](SearchCompletion completion) {
        QMetaObject::invokeMethod(
            this,
            [this, generation, completion]() {
                if (generation != m_searchGeneration)
                {
                    return;
                }

                applySearchCompletion(completion);
            },
            Qt::QueuedConnection);
    });
}

namespace
{

bool isInstalledCard(const AppCard *card)
{
    switch (card->availability())
    {
    case AppAvailability::Installed:
    case AppAvailability::UpdateAvailable:
    case AppAvailability::UninstallQueued:
    case AppAvailability::Uninstalling:
    case AppAvailability::UpdateQueued:
    case AppAvailability::UpdateDownloading:
    case AppAvailability::Updating:
    case AppAvailability::UpdatePaused:
        return true;
    case AppAvailability::Error:
        return card->retriesUninstall() || card->retriesUpdate();
    default:
        return false;
    }
}

bool isUpdateCard(const AppCard *card)
{
    switch (card->availability())
    {
    case AppAvailability::UpdateAvailable:
    case AppAvailability::UpdateQueued:
    case AppAvailability::UpdateDownloading:
    case AppAvailability::Updating:
    case AppAvailability::UpdatePaused:
        return true;
    case AppAvailability::Error:
        return card->retriesUpdate();
    default:
        return false;
    }
}

bool isPackageBusy(AppAvailability availability)
{
    switch (availability)
    {
    case AppAvailability::Queued:
    case AppAvailability::Downloading:
    case AppAvailability::Installing:
    case AppAvailability::Updating:
    case AppAvailability::UninstallQueued:
    case AppAvailability::Uninstalling:
    case AppAvailability::UpdateQueued:
    case AppAvailability::UpdateDownloading:
        return true;
    default:
        return false;
    }
}

} // namespace

void MainWindow::startInstall(const QString &packageId)
{
    const AppCard *card = findCard(packageId);
    if (!card)
    {
        return;
    }

    if (isPackageBusy(card->availability()))
    {
        return;
    }

    m_stopIntent.remove(packageId);
    showPackageStatus(packageId, AppAvailability::Queued, 0, 0, 0, QString());

    m_service.InstallAppAsync(
        packageId.toStdWString(),
        [this, packageId](const InstallProgressInfo &info) {
            const int percent = static_cast<int>(info.percent);
            const auto megabytes = [](std::uint64_t bytes) { return static_cast<int>(bytes / (1024ull * 1024ull)); };
            QMetaObject::invokeMethod(
                this,
                [this, packageId, info, percent, megabytes]() {
                    if (!acceptsInstallProgress(packageId))
                    {
                        return;
                    }

                    switch (info.state)
                    {
                    case InstallState::Queued:
                        showPackageStatus(packageId, AppAvailability::Queued, 0, 0, 0, QString());
                        break;
                    case InstallState::Downloading:
                        showPackageStatus(packageId, AppAvailability::Downloading, percent,
                                          megabytes(info.downloadedBytes), megabytes(info.totalBytes), QString());
                        break;
                    case InstallState::Installing:
                        showPackageStatus(packageId, AppAvailability::Installing, percent, 0, 0, QString());
                        break;
                    case InstallState::Completed:
                        break;
                    }
                },
                Qt::QueuedConnection);
        },
        [this, packageId](InstallCompletion completion) {
            QMetaObject::invokeMethod(
                this,
                [this, packageId, completion]() {
                    if (completion.cancelled)
                    {
                        const bool pause = m_stopIntent.value(packageId, StopIntent::Cancel) == StopIntent::Pause;
                        applyStop(packageId, pause, false);
                        return;
                    }

                    m_stopIntent.remove(packageId);
                    if (!completion.error.empty())
                    {
                        showPackageStatus(packageId, AppAvailability::Error, 0, 0, 0,
                                          QString::fromStdString(completion.error));
                        return;
                    }

                    showPackageStatus(packageId, AppAvailability::Installed, 100, 0, 0, QString(), false, false,
                                      QString::fromStdWString(completion.installedVersion));
                },
                Qt::QueuedConnection);
        });
}

void MainWindow::startUninstall(const QString &packageId)
{
    const AppCard *card = findCard(packageId);
    if (!card)
    {
        return;
    }

    if (isPackageBusy(card->availability()))
    {
        return;
    }

    showPackageStatus(packageId, AppAvailability::UninstallQueued, 0, 0, 0, QString());

    m_service.UninstallAppAsync(
        packageId.toStdWString(),
        [this, packageId](const UninstallProgressInfo &info) {
            const int percent = static_cast<int>(info.percent);
            QMetaObject::invokeMethod(
                this,
                [this, packageId, info, percent]() {
                    switch (info.state)
                    {
                    case UninstallState::Queued:
                        showPackageStatus(packageId, AppAvailability::UninstallQueued, 0, 0, 0, QString());
                        break;
                    case UninstallState::Uninstalling:
                        showPackageStatus(packageId, AppAvailability::Uninstalling, percent, 0, 0, QString());
                        break;
                    case UninstallState::Completed:
                        showPackageStatus(packageId, AppAvailability::NotInstalled, 0, 0, 0, QString());
                        break;
                    }
                },
                Qt::QueuedConnection);
        },
        [this, packageId](UninstallCompletion completion) {
            QMetaObject::invokeMethod(
                this,
                [this, packageId, completion]() {
                    if (!completion.error.empty())
                    {
                        showPackageStatus(packageId, AppAvailability::Error, 0, 0, 0,
                                          QString::fromStdString(completion.error), true);
                        return;
                    }

                    showPackageStatus(packageId, AppAvailability::NotInstalled, 0, 0, 0, QString());
                },
                Qt::QueuedConnection);
        });
}

void MainWindow::startUpdate(const QString &packageId)
{
    const AppCard *card = findCard(packageId);
    if (!card)
    {
        return;
    }

    if (isPackageBusy(card->availability()))
    {
        return;
    }

    m_stopIntent.remove(packageId);
    showPackageStatus(packageId, AppAvailability::UpdateQueued, 0, 0, 0, QString());

    m_service.UpdateAppAsync(
        packageId.toStdWString(),
        [this, packageId](const UpdateProgressInfo &info) {
            const int percent = static_cast<int>(info.percent);
            const auto megabytes = [](std::uint64_t bytes) { return static_cast<int>(bytes / (1024ull * 1024ull)); };
            QMetaObject::invokeMethod(
                this,
                [this, packageId, info, percent, megabytes]() {
                    if (!acceptsUpdateProgress(packageId))
                    {
                        return;
                    }

                    switch (info.state)
                    {
                    case UpdateState::Queued:
                        showPackageStatus(packageId, AppAvailability::UpdateQueued, 0, 0, 0, QString());
                        break;
                    case UpdateState::Downloading:
                        showPackageStatus(packageId, AppAvailability::UpdateDownloading, percent,
                                          megabytes(info.downloadedBytes), megabytes(info.totalBytes), QString());
                        break;
                    case UpdateState::Updating:
                        showPackageStatus(packageId, AppAvailability::Updating, percent, 0, 0, QString());
                        break;
                    case UpdateState::Completed:
                        break;
                    }
                },
                Qt::QueuedConnection);
        },
        [this, packageId](UpdateCompletion completion) {
            QMetaObject::invokeMethod(
                this,
                [this, packageId, completion]() {
                    if (completion.cancelled)
                    {
                        const bool pause = m_stopIntent.value(packageId, StopIntent::Cancel) == StopIntent::Pause;
                        applyStop(packageId, pause, true);
                        return;
                    }

                    m_stopIntent.remove(packageId);
                    if (!completion.error.empty())
                    {
                        showPackageStatus(packageId, AppAvailability::Error, 0, 0, 0,
                                          QString::fromStdString(completion.error), false, true);
                        return;
                    }

                    showPackageStatus(packageId, AppAvailability::Installed, 100, 0, 0, QString(), false, false,
                                      QString::fromStdWString(completion.installedVersion));
                },
                Qt::QueuedConnection);
        });
}

void MainWindow::stopPackage(const QString &packageId, bool pause)
{
    const AppCard *card = findCard(packageId);
    if (!card)
    {
        return;
    }

    const AppAvailability availability = card->availability();
    if (availability == AppAvailability::Paused || availability == AppAvailability::UpdatePaused)
    {
        if (!pause)
        {
            applyStop(packageId, false, availability == AppAvailability::UpdatePaused);
        }
        return;
    }

    const bool update = availability == AppAvailability::UpdateQueued ||
                        availability == AppAvailability::UpdateDownloading || availability == AppAvailability::Updating;
    const bool install = availability == AppAvailability::Queued || availability == AppAvailability::Downloading ||
                         availability == AppAvailability::Installing;
    if (!install && !update)
    {
        return;
    }

    if (availability == AppAvailability::Installing || availability == AppAvailability::Updating)
    {
        return;
    }

    m_stopIntent.insert(packageId, pause ? StopIntent::Pause : StopIntent::Cancel);
    const OperationControl control = update ? m_service.CancelUpdate(packageId.toStdWString())
                                            : m_service.CancelInstall(packageId.toStdWString());
    if (control == OperationControl::RemovedFromQueue)
    {
        applyStop(packageId, pause, update);
    }
    else if (control == OperationControl::None)
    {
        m_stopIntent.remove(packageId);
    }
}

void MainWindow::applyStop(const QString &packageId, bool pause, bool update)
{
    m_stopIntent.remove(packageId);
    const AppCard *card = findCard(packageId);
    const int percent = card ? card->model().progressPercent : 0;
    const int downloadedMb = card ? card->model().downloadedMb : 0;
    const int totalMb = card ? card->model().totalMb : 0;
    const QString installedVersion = card ? card->model().installedVersion : QString();

    if (pause)
    {
        showPackageStatus(packageId, update ? AppAvailability::UpdatePaused : AppAvailability::Paused, percent,
                          downloadedMb, totalMb, QString(), false, false, installedVersion);
        return;
    }

    if (update)
    {
        showPackageStatus(packageId, AppAvailability::UpdateAvailable, 0, 0, 0, QString(), false, false,
                          installedVersion);
        return;
    }

    showPackageStatus(packageId, AppAvailability::NotInstalled, 0, 0, 0, QString());
}

bool MainWindow::acceptsInstallProgress(const QString &packageId) const
{
    if (m_stopIntent.contains(packageId))
    {
        return false;
    }

    const AppCard *card = findCard(packageId);
    if (!card)
    {
        return true;
    }

    switch (card->availability())
    {
    case AppAvailability::Queued:
    case AppAvailability::Downloading:
    case AppAvailability::Installing:
        return true;
    default:
        return false;
    }
}

bool MainWindow::acceptsUpdateProgress(const QString &packageId) const
{
    if (m_stopIntent.contains(packageId))
    {
        return false;
    }

    const AppCard *card = findCard(packageId);
    if (!card)
    {
        return true;
    }

    switch (card->availability())
    {
    case AppAvailability::UpdateQueued:
    case AppAvailability::UpdateDownloading:
    case AppAvailability::Updating:
        return true;
    default:
        return false;
    }
}

void MainWindow::startLaunch(const QString &packageId)
{
    if (m_detailPackageId == packageId)
    {
        m_detailPage->setLaunchError(QString());
    }

    m_service.LaunchAppAsync(packageId.toStdWString(), [this, packageId](LaunchCompletion completion) {
        QMetaObject::invokeMethod(
            this,
            [this, packageId, completion]() {
                std::string error = completion.error;
                if (error.empty())
                {
                    error = m_service.ActivateAppUserModelIds(completion.appUserModelIds);
                }
                if (error.empty())
                {
                    return;
                }

                if (m_detailPackageId != packageId)
                {
                    openDetail(packageId);
                }
                if (m_detailPackageId == packageId)
                {
                    m_detailPage->setLaunchError(QString::fromStdString(error));
                }
            },
            Qt::QueuedConnection);
    });
}

void MainWindow::openDetail(const QString &packageId)
{
    const AppCard *card = findCard(packageId);
    if (!card)
    {
        return;
    }

    m_detailPackageId = packageId;
    m_detailPage->setModel(card->model());
    ui.searchRow->setVisible(false);
    ui.filterRow->setVisible(false);
    ui.pages->setCurrentWidget(m_detailPage);
}

void MainWindow::closeDetail()
{
    m_detailPackageId.clear();
    const bool discover = m_section == Section::Discover;
    const bool showList = discover || m_section == Section::Installed || m_section == Section::Updates;
    ui.searchRow->setVisible(discover);
    ui.filterRow->setVisible(showList);
    ui.pages->setCurrentWidget(ui.pageList);
}

void MainWindow::showPackageStatus(const QString &packageId, AppAvailability availability, int percent, int downloadedMb,
                                   int totalMb, const QString &errorText, bool retryUninstall, bool retryUpdate,
                                   const QString &installedVersion)
{
    InstallUiState &state = m_installUi[packageId];
    const AppAvailability previous = state.availability;
    state.availability = availability;
    state.progressPercent = percent;
    state.downloadedMb = downloadedMb;
    state.totalMb = totalMb;
    state.error = errorText;
    state.retryUninstall = retryUninstall;
    state.retryUpdate = retryUpdate;
    if (availability == AppAvailability::NotInstalled)
    {
        state.installedVersion.clear();
    }
    else if (!installedVersion.isEmpty())
    {
        state.installedVersion = installedVersion;
    }

    AppCard *card = findCard(packageId);
    if (!card)
    {
        return;
    }

    card->applyStatus(availability, percent, downloadedMb, totalMb, errorText, retryUninstall, retryUpdate,
                      installedVersion);
    if (m_detailPackageId == packageId)
    {
        m_detailPage->setModel(card->model());
    }
    if (previous != availability)
    {
        refreshAppList();
    }
}

AppCard *MainWindow::findCard(const QString &packageId) const
{
    for (AppCard *card : m_cards)
    {
        if (card->packageId() == packageId)
        {
            return card;
        }
    }

    return nullptr;
}

void MainWindow::showStatus(const QString &text)
{
    clearCards();
    m_statusText = text;
    ui.emptyStateLabel->setText(text);
    ui.emptyStateLabel->setVisible(true);
    ui.appScrollArea->setVisible(false);
}

void MainWindow::clearCards()
{
    while (QLayoutItem *item = ui.appListLayout->takeAt(0))
    {
        delete item->widget();
        delete item;
    }

    m_cards.clear();
}

void MainWindow::applySearchCompletion(const SearchCompletion &completion)
{
    m_loadedSection = m_section;
    m_listLoaded = completion.error.empty();

    if (!completion.error.empty())
    {
        showStatus(QString::fromStdString(completion.error));
        return;
    }

    clearCards();
    m_statusText.clear();

    int index = 0;
    for (const AppInfo &app : completion.apps)
    {
        AppCardModel model;
        model.name = QString::fromStdWString(app.name);
        model.packageId = QString::fromStdWString(app.id);
        model.source = QStringLiteral("Microsoft Store");
        model.catalogIndex = index++;
        model.hasAvailability = true;
        model.installedVersion = QString::fromStdWString(app.installedVersion);
        model.availableVersion = QString::fromStdWString(app.availableVersion);
        if (app.updateAvailable)
        {
            model.availability = AppAvailability::UpdateAvailable;
        }
        else if (app.installed)
        {
            model.availability = AppAvailability::Installed;
        }
        else
        {
            model.availability = AppAvailability::NotInstalled;
        }

        const auto saved = m_installUi.constFind(model.packageId);
        if (saved != m_installUi.constEnd())
        {
            model.availability = saved->availability;
            model.progressPercent = saved->progressPercent;
            model.downloadedMb = saved->downloadedMb;
            model.totalMb = saved->totalMb;
            model.errorText = saved->error;
            model.retryUninstall = saved->retryUninstall;
            model.retryUpdate = saved->retryUpdate;
            if (model.installedVersion.isEmpty())
            {
                model.installedVersion = saved->installedVersion;
            }
        }

        auto *card = new AppCard(model, ui.appListContainer);
        card->setDarkMode(m_darkMode);
        card->hide();
        connect(card, &AppCard::installRequested, this, &MainWindow::startInstall, Qt::QueuedConnection);
        connect(card, &AppCard::uninstallRequested, this, &MainWindow::startUninstall, Qt::QueuedConnection);
        connect(card, &AppCard::updateRequested, this, &MainWindow::startUpdate, Qt::QueuedConnection);
        connect(card, &AppCard::launchRequested, this, &MainWindow::startLaunch, Qt::QueuedConnection);
        connect(card, &AppCard::pauseRequested, this, [this](const QString &packageId) { stopPackage(packageId, true); });
        connect(card, &AppCard::cancelRequested, this,
                [this](const QString &packageId) { stopPackage(packageId, false); });
        connect(card, &AppCard::detailsRequested, this, &MainWindow::openDetail, Qt::QueuedConnection);
        m_cards.push_back(card);
    }

    if (m_cards.empty())
    {
        showStatus(emptyListMessage());
        return;
    }

    refreshAppList();
}

void MainWindow::refreshAppList()
{
    if (m_cards.empty())
    {
        ui.emptyStateLabel->setText(emptyListMessage());
        ui.emptyStateLabel->setVisible(true);
        ui.appScrollArea->setVisible(false);
        return;
    }

    std::vector<AppCard *> ordered = m_cards;
    if (ui.sortCombo->currentText() == QStringLiteral("Name"))
    {
        std::sort(ordered.begin(), ordered.end(), [](const AppCard *left, const AppCard *right) {
            const int byName = QString::localeAwareCompare(left->name(), right->name());
            if (byName != 0)
            {
                return byName < 0;
            }
            return left->catalogIndex() < right->catalogIndex();
        });
    }
    else
    {
        std::sort(ordered.begin(), ordered.end(), [](const AppCard *left, const AppCard *right) {
            return left->catalogIndex() < right->catalogIndex();
        });
    }

    const int scrollValue = ui.appScrollArea->verticalScrollBar()->value();

    if (ui.appListLayout->count() > 0)
    {
        QLayoutItem *last = ui.appListLayout->itemAt(ui.appListLayout->count() - 1);
        if (last && last->spacerItem())
        {
            delete ui.appListLayout->takeAt(ui.appListLayout->count() - 1);
        }
    }

    for (AppCard *card : ordered)
    {
        ui.appListLayout->removeWidget(card);
    }

    bool anyVisible = false;
    for (AppCard *card : ordered)
    {
        const bool visible = cardMatches(card);
        card->setVisible(visible);
        ui.appListLayout->addWidget(card);
        anyVisible = anyVisible || visible;
    }
    ui.appListLayout->addStretch(1);

    if (!anyVisible)
    {
        ui.emptyStateLabel->setText(emptyListMessage());
    }
    ui.emptyStateLabel->setVisible(!anyVisible);
    ui.appScrollArea->setVisible(anyVisible);
    ui.appScrollArea->verticalScrollBar()->setValue(scrollValue);
}

bool MainWindow::cardMatches(const AppCard *card) const
{
    if (m_section == Section::Discover)
    {
        const int filter = m_filterGroup->checkedId();
        if (filter == static_cast<int>(StatusFilter::Installed) && !isInstalledCard(card))
        {
            return false;
        }
        if (filter == static_cast<int>(StatusFilter::NotInstalled) && isInstalledCard(card))
        {
            return false;
        }
    }
    else if (m_section == Section::Installed)
    {
        if (!isInstalledCard(card))
        {
            return false;
        }
    }
    else if (m_section == Section::Updates)
    {
        if (!isUpdateCard(card))
        {
            return false;
        }
    }

    const QString source = ui.sourceCombo->currentText();
    if (source != QStringLiteral("All sources") && card->source() != source)
    {
        return false;
    }

    if (m_section != Section::Discover)
    {
        return true;
    }

    const QString query = ui.searchEdit->text().trimmed();
    if (query.isEmpty())
    {
        return true;
    }

    return card->name().contains(query, Qt::CaseInsensitive) || card->packageId().contains(query, Qt::CaseInsensitive);
}

QString MainWindow::emptyListMessage() const
{
    if (!m_statusText.isEmpty() && m_cards.empty())
    {
        return m_statusText;
    }

    if (m_section == Section::Installed)
    {
        return QStringLiteral("No installed applications.");
    }
    if (m_section == Section::Updates)
    {
        return QStringLiteral("No updates available.");
    }
    if (m_filterGroup->checkedId() == static_cast<int>(StatusFilter::Installed))
    {
        return QStringLiteral("No installed applications match your search.");
    }
    if (m_filterGroup->checkedId() == static_cast<int>(StatusFilter::NotInstalled))
    {
        return QStringLiteral("No applications to install match your search.");
    }

    if (m_section == Section::Discover && ui.searchEdit->text().trimmed().isEmpty() &&
        m_filterGroup->checkedId() != static_cast<int>(StatusFilter::Installed) &&
        m_filterGroup->checkedId() != static_cast<int>(StatusFilter::NotInstalled))
    {
        return QStringLiteral("No featured applications are available.");
    }

    return QStringLiteral("No applications match your search.");
}

void MainWindow::positionUpdatesBadge()
{
    if (!m_updatesBadge)
    {
        return;
    }

    const int x = ui.updatesButton->width() - m_updatesBadge->width() - 10;
    const int y = (ui.updatesButton->height() - m_updatesBadge->height()) / 2;
    m_updatesBadge->move(std::max(0, x), std::max(0, y));
    m_updatesBadge->raise();
}
