#include "AppCard.h"

#include "CircularProgress.h"
#include "UiIcons.h"

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

} // namespace

AppCard::AppCard(const AppCardModel &model, QWidget *parent) : QWidget(parent), m_model(model)
{
    setAttribute(Qt::WA_Hover, true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setCursor(Qt::ArrowCursor);

    auto *icon = new QLabel(this);
    icon->setObjectName(QStringLiteral("packageIcon"));
    icon->setFixedSize(48, 48);
    icon->setAlignment(Qt::AlignCenter);
    icon->setPixmap(UiIcons::pixmap(UiIcons::Glyph::Package, QColor(QStringLiteral("#8B919A")), 28));

    auto *name = makeLabel(m_model.name, QStringLiteral("appName"), this);
    auto *packageId = makeLabel(m_model.packageId, QStringLiteral("packageId"), this);

    auto *identity = new QVBoxLayout();
    identity->setContentsMargins(0, 0, 0, 0);
    identity->setSpacing(2);
    identity->addWidget(name);
    identity->addWidget(packageId);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(16);
    layout->addWidget(icon, 0, Qt::AlignVCenter);
    layout->addLayout(identity, 3);
    layout->addWidget(createStatusPanel(), 0, Qt::AlignVCenter);
    layout->addStretch(1);
    layout->addWidget(createActionPanel(), 0, Qt::AlignVCenter);
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

int AppCard::catalogIndex() const
{
    return m_model.catalogIndex;
}

void AppCard::setDarkMode(bool dark)
{
    if (m_progress)
    {
        m_progress->setDarkMode(dark);
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

    if (m_model.availability == AppAvailability::Installing || m_model.availability == AppAvailability::Updating)
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
        const QString operation = m_model.availability == AppAvailability::Updating ? QStringLiteral("Updating...")
                                                                                    : QStringLiteral("Installing...");
        auto *title = makeLabel(operation, QStringLiteral("statusBusy"), panel);
        title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        lines->addWidget(title);
        if (m_model.totalMb > 0)
        {
            lines->addWidget(makeLabel(QStringLiteral("%1 MB of %2 MB").arg(m_model.downloadedMb).arg(m_model.totalMb),
                                        QStringLiteral("statusDetail"), panel));
        }
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

    if (m_model.availability == AppAvailability::Installed)
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

    return panel;
}

QWidget *AppCard::createActionPanel()
{
    auto *panel = new QWidget(this);
    panel->setFixedWidth(112);

    auto *column = new QVBoxLayout(panel);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(8);

    if (m_model.availability == AppAvailability::UpdateAvailable)
    {
        column->addWidget(createButton(QStringLiteral("Update"), true));
        column->addWidget(createButton(QStringLiteral("Uninstall"), false));
    }
    else if (m_model.availability == AppAvailability::Installed)
    {
        column->addWidget(createButton(QStringLiteral("Uninstall"), false));
    }
    else if (m_model.availability == AppAvailability::NotInstalled)
    {
        column->addWidget(createButton(QStringLiteral("Install"), true));
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
