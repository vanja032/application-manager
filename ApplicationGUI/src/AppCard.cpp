#include "AppCard.h"

#include "CircularProgress.h"
#include "UiIcons.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOption>
#include <QtWidgets/QVBoxLayout>

namespace
{

QLabel *makeLabel(const QString &text, const QString &objectName, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName(objectName);
    return label;
}

QString versionText(const QString &prefix, const QString &version)
{
    if (version.isEmpty())
    {
        return prefix + QStringLiteral("unknown");
    }

    return prefix + version;
}

bool isBusy(AppAvailability availability)
{
    return availability == AppAvailability::Queued || availability == AppAvailability::Downloading ||
           availability == AppAvailability::Installing || availability == AppAvailability::Updating ||
           availability == AppAvailability::UninstallQueued || availability == AppAvailability::Uninstalling ||
           availability == AppAvailability::UpdateQueued || availability == AppAvailability::UpdateDownloading;
}

bool canPause(AppAvailability availability)
{
    return availability == AppAvailability::Queued || availability == AppAvailability::Downloading ||
           availability == AppAvailability::UpdateQueued || availability == AppAvailability::UpdateDownloading;
}

QString busyTitle(AppAvailability availability)
{
    switch (availability)
    {
    case AppAvailability::Queued:
        return QStringLiteral("Queued");
    case AppAvailability::Downloading:
        return QStringLiteral("Downloading...");
    case AppAvailability::Updating:
        return QStringLiteral("Updating...");
    case AppAvailability::UninstallQueued:
        return QStringLiteral("Queued");
    case AppAvailability::Uninstalling:
        return QStringLiteral("Uninstalling...");
    case AppAvailability::UpdateQueued:
        return QStringLiteral("Queued");
    case AppAvailability::UpdateDownloading:
        return QStringLiteral("Downloading...");
    default:
        return QStringLiteral("Installing...");
    }
}

} // namespace

AppCard::AppCard(const AppCardModel &model, QWidget *parent) : QWidget(parent), m_model(model)
{
    setAttribute(Qt::WA_Hover, true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);

    auto *icon = new QLabel(this);
    icon->setObjectName(QStringLiteral("packageIcon"));
    icon->setFixedSize(48, 48);
    icon->setAlignment(Qt::AlignCenter);
    icon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    icon->setPixmap(UiIcons::pixmap(UiIcons::Glyph::Package, QColor(QStringLiteral("#8B919A")), 28));

    auto *name = makeLabel(m_model.name, QStringLiteral("appName"), this);
    auto *packageId = makeLabel(m_model.packageId, QStringLiteral("packageId"), this);
    name->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    packageId->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    auto *identity = new QVBoxLayout();
    identity->setContentsMargins(0, 0, 0, 0);
    identity->setSpacing(2);
    identity->addWidget(name);
    identity->addWidget(packageId);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(16, 14, 16, 14);
    m_layout->setSpacing(16);
    m_layout->addWidget(icon, 0, Qt::AlignVCenter);
    m_layout->addLayout(identity, 1);
    rebuildStatusAndActions();
}

QString AppCard::name() const
{
    return m_model.name;
}

QString AppCard::packageId() const
{
    return m_model.packageId;
}

QString AppCard::source() const
{
    return m_model.source;
}

AppAvailability AppCard::availability() const
{
    return m_model.availability;
}

bool AppCard::hasAvailability() const
{
    return m_model.hasAvailability;
}

int AppCard::catalogIndex() const
{
    return m_model.catalogIndex;
}

bool AppCard::retriesUninstall() const
{
    return m_model.retryUninstall;
}

bool AppCard::retriesUpdate() const
{
    return m_model.retryUpdate;
}

const AppCardModel &AppCard::model() const
{
    return m_model;
}

void AppCard::applyStatus(AppAvailability availability, int percent, int downloadedMb, int totalMb, const QString &errorText,
                           bool retryUninstall, bool retryUpdate, const QString &installedVersion)
{
    const bool wasBusy = isBusy(m_model.availability);
    const bool nowBusy = isBusy(availability);
    const bool pauseChanged = canPause(m_model.availability) != canPause(availability);
    if (availability == AppAvailability::NotInstalled)
    {
        m_model.installedVersion.clear();
    }
    else if (!installedVersion.isEmpty())
    {
        m_model.installedVersion = installedVersion;
    }
    m_model.availability = availability;
    m_model.progressPercent = percent;
    m_model.downloadedMb = downloadedMb;
    m_model.totalMb = totalMb;
    m_model.errorText = errorText;
    m_model.retryUninstall = retryUninstall;
    m_model.retryUpdate = retryUpdate;
    m_model.hasAvailability = true;

    if (wasBusy && nowBusy && !pauseChanged && m_progress && m_phaseLabel && m_bytesLabel)
    {
        m_progress->setValue(percent);
        m_phaseLabel->setText(busyTitle(availability));
        if (totalMb > 0)
        {
            m_bytesLabel->setText(QStringLiteral("%1 MB of %2 MB").arg(downloadedMb).arg(totalMb));
            m_bytesLabel->setVisible(true);
        }
        else
        {
            m_bytesLabel->setVisible(false);
        }
        return;
    }

    rebuildStatusAndActions();
}

void AppCard::setDarkMode(bool dark)
{
    m_dark = dark;
    if (m_progress)
    {
        m_progress->setDarkMode(dark);
    }
}

void AppCard::rebuildStatusAndActions()
{
    if (m_statusPanel)
    {
        m_layout->removeWidget(m_statusPanel);
        m_statusPanel->deleteLater();
        m_statusPanel = nullptr;
        m_progress = nullptr;
        m_phaseLabel = nullptr;
        m_bytesLabel = nullptr;
    }

    if (m_actionPanel)
    {
        m_layout->removeWidget(m_actionPanel);
        m_actionPanel->deleteLater();
        m_actionPanel = nullptr;
    }

    if (!m_model.hasAvailability)
    {
        return;
    }

    m_statusPanel = createStatusPanel();
    m_actionPanel = createActionPanel();
    m_layout->addWidget(m_statusPanel, 0, Qt::AlignVCenter);
    m_layout->addWidget(m_actionPanel, 0, Qt::AlignVCenter);
    m_statusPanel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    if (m_progress)
    {
        m_progress->setDarkMode(m_dark);
    }
}

void AppCard::paintEvent(QPaintEvent *)
{
    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
}

QLabel *makeStatusIcon(const QPixmap &pixmap, QWidget *parent)
{
    auto *icon = new QLabel(parent);
    icon->setObjectName(QStringLiteral("statusIcon"));
    icon->setFixedSize(16, 16);
    icon->setAlignment(Qt::AlignCenter);
    icon->setPixmap(pixmap);
    return icon;
}

void addDetailLine(QVBoxLayout *column, const QString &text, QWidget *parent)
{
    constexpr int kIconColumn = 16 + 8;
    auto *row = new QHBoxLayout();
    row->setContentsMargins(kIconColumn, 0, 0, 0);
    row->setSpacing(0);
    auto *detail = makeLabel(text, QStringLiteral("statusDetail"), parent);
    detail->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    row->addWidget(detail);
    row->addStretch(1);
    column->addLayout(row);
}

QWidget *AppCard::createStatusPanel()
{
    auto *panel = new QWidget(this);
    panel->setFixedWidth(280);

    if (isBusy(m_model.availability))
    {
        auto *row = new QHBoxLayout(panel);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(10);

        m_progress = new CircularProgress(panel);
        m_progress->setValue(m_model.progressPercent);
        row->addWidget(m_progress, 0, Qt::AlignVCenter);

        auto *lines = new QVBoxLayout();
        lines->setContentsMargins(0, 0, 0, 0);
        lines->setSpacing(1);
        m_phaseLabel = makeLabel(busyTitle(m_model.availability), QStringLiteral("statusBusy"), panel);
        m_phaseLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        lines->addWidget(m_phaseLabel);
        m_bytesLabel = makeLabel(QStringLiteral("%1 MB of %2 MB").arg(m_model.downloadedMb).arg(m_model.totalMb),
                                  QStringLiteral("statusDetail"), panel);
        m_bytesLabel->setVisible(m_model.totalMb > 0);
        lines->addWidget(m_bytesLabel);
        row->addLayout(lines);
        row->addStretch(1);
        return panel;
    }

    auto *column = new QVBoxLayout(panel);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(1);

    auto *titleRow = new QHBoxLayout();
    titleRow->setContentsMargins(0, 0, 0, 0);
    titleRow->setSpacing(8);

    QLabel *icon = nullptr;
    QLabel *title = nullptr;
    if (m_model.availability == AppAvailability::Installed)
    {
        icon = makeStatusIcon(UiIcons::pixmap(UiIcons::Glyph::Check, QColor(QStringLiteral("#107C10")), 16), panel);
        title = makeLabel(QStringLiteral("Installed"), QStringLiteral("statusInstalled"), panel);
    }
    else if (m_model.availability == AppAvailability::UpdateAvailable)
    {
        icon = makeStatusIcon(UiIcons::pixmap(UiIcons::Glyph::Refresh, QColor(QStringLiteral("#0078D4")), 16), panel);
        title = makeLabel(QStringLiteral("Update available"), QStringLiteral("statusUpdate"), panel);
    }
    else if (m_model.availability == AppAvailability::Paused || m_model.availability == AppAvailability::UpdatePaused)
    {
        icon = makeStatusIcon(UiIcons::pixmap(UiIcons::Glyph::Dot, QColor(QStringLiteral("#8B919A")), 16), panel);
        title = makeLabel(QStringLiteral("Paused"), QStringLiteral("statusMuted"), panel);
    }
    else if (m_model.availability == AppAvailability::Error)
    {
        icon = makeStatusIcon(UiIcons::pixmap(UiIcons::Glyph::Dot, QColor(QStringLiteral("#D13438")), 16), panel);
        title = makeLabel(QStringLiteral("Error"), QStringLiteral("statusError"), panel);
    }
    else
    {
        icon = makeStatusIcon(UiIcons::pixmap(UiIcons::Glyph::Dot, QColor(QStringLiteral("#8B919A")), 16), panel);
        title = makeLabel(QStringLiteral("Not installed"), QStringLiteral("statusMuted"), panel);
    }

    title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleRow->addWidget(icon, 0, Qt::AlignVCenter);
    titleRow->addWidget(title, 0, Qt::AlignVCenter);
    titleRow->addStretch(1);
    column->addLayout(titleRow);

    if (m_model.availability == AppAvailability::Paused || m_model.availability == AppAvailability::UpdatePaused)
    {
        QString progress = QStringLiteral("%1%").arg(m_model.progressPercent);
        if (m_model.totalMb > 0)
        {
            progress = QStringLiteral("%1% · %2 MB of %3 MB")
                           .arg(m_model.progressPercent)
                           .arg(m_model.downloadedMb)
                           .arg(m_model.totalMb);
        }
        addDetailLine(column, progress, panel);
    }
    else if (m_model.availability == AppAvailability::Installed)
    {
        const QString version = m_model.installedVersion.isEmpty()
                                    ? QStringLiteral("Version unknown")
                                    : QStringLiteral("Version %1").arg(m_model.installedVersion);
        addDetailLine(column, version, panel);
    }
    else if (m_model.availability == AppAvailability::UpdateAvailable)
    {
        addDetailLine(column, versionText(QStringLiteral("Installed version: "), m_model.installedVersion), panel);
        addDetailLine(column, versionText(QStringLiteral("Available version: "), m_model.availableVersion), panel);
    }
    else if (m_model.availability == AppAvailability::Error && !m_model.errorText.isEmpty())
    {
        auto *detail = makeLabel(m_model.errorText, QStringLiteral("statusDetail"), panel);
        detail->setWordWrap(true);
        column->addWidget(detail);
    }

    return panel;
}

QWidget *AppCard::createActionPanel()
{
    auto *panel = new QWidget(this);
    panel->setFixedWidth(112);

    auto *column = new QVBoxLayout(panel);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(8);

    if (m_model.availability == AppAvailability::Paused || m_model.availability == AppAvailability::UpdatePaused)
    {
        auto *resume = createButton(QStringLiteral("Resume"), true);
        connect(resume, &QPushButton::clicked, this, [this]() {
            if (m_model.availability == AppAvailability::UpdatePaused)
            {
                emit updateRequested(m_model.packageId);
            }
            else
            {
                emit installRequested(m_model.packageId);
            }
        });
        column->addWidget(resume);

        auto *cancel = createButton(QStringLiteral("Cancel"), false);
        connect(cancel, &QPushButton::clicked, this, [this]() { emit cancelRequested(m_model.packageId); });
        column->addWidget(cancel);
    }
    else if (canPause(m_model.availability))
    {
        auto *pause = createButton(QStringLiteral("Pause"), false);
        connect(pause, &QPushButton::clicked, this, [this]() { emit pauseRequested(m_model.packageId); });
        column->addWidget(pause);

        auto *cancel = createButton(QStringLiteral("Cancel"), false);
        connect(cancel, &QPushButton::clicked, this, [this]() { emit cancelRequested(m_model.packageId); });
        column->addWidget(cancel);
    }
    else if (m_model.availability == AppAvailability::UpdateAvailable ||
             (m_model.availability == AppAvailability::Error && m_model.retryUpdate))
    {
        auto *update = createButton(QStringLiteral("Update"), true);
        connect(update, &QPushButton::clicked, this, [this]() { emit updateRequested(m_model.packageId); });
        column->addWidget(update);

        auto *launch = createButton(QStringLiteral("Launch"), true);
        connect(launch, &QPushButton::clicked, this, [this]() { emit launchRequested(m_model.packageId); });
        column->addWidget(launch);

        auto *uninstall = createButton(QStringLiteral("Uninstall"), false);
        connect(uninstall, &QPushButton::clicked, this, [this]() { emit uninstallRequested(m_model.packageId); });
        column->addWidget(uninstall);
    }
    else if (m_model.availability == AppAvailability::Installed ||
             (m_model.availability == AppAvailability::Error && m_model.retryUninstall))
    {
        auto *launch = createButton(QStringLiteral("Launch"), true);
        connect(launch, &QPushButton::clicked, this, [this]() { emit launchRequested(m_model.packageId); });
        column->addWidget(launch);

        auto *uninstall = createButton(QStringLiteral("Uninstall"), false);
        connect(uninstall, &QPushButton::clicked, this, [this]() { emit uninstallRequested(m_model.packageId); });
        column->addWidget(uninstall);
    }
    else if (m_model.availability == AppAvailability::NotInstalled || m_model.availability == AppAvailability::Error)
    {
        auto *install = createButton(QStringLiteral("Install"), true);
        connect(install, &QPushButton::clicked, this, [this]() { emit installRequested(m_model.packageId); });
        column->addWidget(install);
    }

    return panel;
}

QPushButton *AppCard::createButton(const QString &text, bool primary) const
{
    auto *button = new QPushButton(text);
    button->setObjectName(primary ? QStringLiteral("primaryButton") : QStringLiteral("secondaryButton"));
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedSize(112, 36);
    button->setFocusPolicy(Qt::NoFocus);
    return button;
}

void AppCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void AppCard::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit detailsRequested(m_model.packageId);
    }

    QWidget::mouseReleaseEvent(event);
}
