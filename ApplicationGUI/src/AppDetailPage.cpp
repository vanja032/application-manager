#include "AppDetailPage.h"

#include "UiIcons.h"

#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

namespace
{

bool isBusy(AppAvailability availability)
{
    return availability == AppAvailability::Queued || availability == AppAvailability::Downloading ||
           availability == AppAvailability::Installing || availability == AppAvailability::Updating ||
           availability == AppAvailability::UninstallQueued || availability == AppAvailability::Uninstalling ||
           availability == AppAvailability::UpdateQueued || availability == AppAvailability::UpdateDownloading;
}

QString statusTitle(const AppCardModel &model)
{
    switch (model.availability)
    {
    case AppAvailability::Installed:
        return QStringLiteral("Installed");
    case AppAvailability::UpdateAvailable:
        return QStringLiteral("Update available");
    case AppAvailability::Queued:
    case AppAvailability::UpdateQueued:
    case AppAvailability::UninstallQueued:
        return QStringLiteral("Queued");
    case AppAvailability::Downloading:
    case AppAvailability::UpdateDownloading:
        return QStringLiteral("Downloading");
    case AppAvailability::Installing:
        return QStringLiteral("Installing");
    case AppAvailability::Updating:
        return QStringLiteral("Updating");
    case AppAvailability::Uninstalling:
        return QStringLiteral("Uninstalling");
    case AppAvailability::Error:
        return QStringLiteral("Error");
    default:
        return QStringLiteral("Not installed");
    }
}

QString versionOrUnknown(const QString &version)
{
    return version.isEmpty() ? QStringLiteral("Unknown") : version;
}

void addInfoRow(QVBoxLayout *column, const QString &label, const QString &value)
{
    auto *name = new QLabel(label);
    name->setObjectName(QStringLiteral("detailLabel"));
    auto *text = new QLabel(value);
    text->setObjectName(QStringLiteral("detailValue"));
    text->setWordWrap(true);
    column->addWidget(name);
    column->addWidget(text);
    column->addSpacing(12);
}

} // namespace

AppDetailPage::AppDetailPage(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("detailPage"));

    m_root = new QVBoxLayout(this);
    m_root->setContentsMargins(0, 4, 0, 0);
    m_root->setSpacing(20);

    auto *back = new QPushButton(QStringLiteral("Back"), this);
    back->setObjectName(QStringLiteral("secondaryButton"));
    back->setCursor(Qt::PointingHandCursor);
    back->setFixedSize(112, 36);
    back->setFocusPolicy(Qt::NoFocus);
    connect(back, &QPushButton::clicked, this, &AppDetailPage::backRequested);

    auto *backRow = new QHBoxLayout();
    backRow->setContentsMargins(0, 0, 0, 0);
    backRow->addWidget(back, 0, Qt::AlignLeft);
    backRow->addStretch(1);
    m_root->addLayout(backRow);
    m_root->addStretch(1);
}

void AppDetailPage::setModel(const AppCardModel &model)
{
    m_model = model;
    m_launchError.clear();
    rebuild();
}

void AppDetailPage::setLaunchError(const QString &text)
{
    m_launchError = text;
    rebuild();
}

void AppDetailPage::rebuild()
{
    if (m_body)
    {
        m_root->removeWidget(m_body);
        m_body->deleteLater();
        m_body = nullptr;
    }

    m_body = new QWidget(this);
    auto *page = new QVBoxLayout(m_body);
    page->setContentsMargins(0, 0, 0, 0);
    page->setSpacing(20);

    auto *icon = new QLabel(m_body);
    icon->setFixedSize(64, 64);
    icon->setAlignment(Qt::AlignCenter);
    icon->setPixmap(UiIcons::pixmap(UiIcons::Glyph::Package, QColor(QStringLiteral("#8B919A")), 36));

    auto *name = new QLabel(m_model.name, m_body);
    name->setObjectName(QStringLiteral("detailTitle"));
    name->setWordWrap(true);
    auto *packageId = new QLabel(m_model.packageId, m_body);
    packageId->setObjectName(QStringLiteral("detailSubtitle"));
    packageId->setWordWrap(true);
    auto *source = new QLabel(m_model.source, m_body);
    source->setObjectName(QStringLiteral("detailSubtitle"));

    auto *titles = new QVBoxLayout();
    titles->setContentsMargins(0, 0, 0, 0);
    titles->setSpacing(4);
    titles->addWidget(name);
    titles->addWidget(packageId);
    titles->addWidget(source);

    auto *header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(16);
    header->addWidget(icon, 0, Qt::AlignTop);
    header->addLayout(titles, 1);
    page->addLayout(header);

    auto *card = new QFrame(m_body);
    card->setObjectName(QStringLiteral("detailCard"));
    card->setMinimumWidth(520);
    card->setMaximumWidth(720);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 22, 24, 22);
    cardLayout->setSpacing(0);

    addInfoRow(cardLayout, QStringLiteral("Status"), statusTitle(m_model));
    if (m_model.availability != AppAvailability::NotInstalled)
    {
        addInfoRow(cardLayout, QStringLiteral("Installed version"), versionOrUnknown(m_model.installedVersion));
    }
    if (!m_model.availableVersion.isEmpty())
    {
        addInfoRow(cardLayout, QStringLiteral("Available version"), m_model.availableVersion);
    }
    if (m_model.availability == AppAvailability::Error && !m_model.errorText.isEmpty())
    {
        addInfoRow(cardLayout, QStringLiteral("Details"), m_model.errorText);
    }
    else if (isBusy(m_model.availability))
    {
        QString progress = QStringLiteral("%1%").arg(m_model.progressPercent);
        if (m_model.totalMb > 0)
        {
            progress = QStringLiteral("%1% · %2 MB of %3 MB")
                           .arg(m_model.progressPercent)
                           .arg(m_model.downloadedMb)
                           .arg(m_model.totalMb);
        }
        addInfoRow(cardLayout, QStringLiteral("Progress"), progress);
    }

    if (!m_launchError.isEmpty())
    {
        auto *error = new QLabel(m_launchError, card);
        error->setObjectName(QStringLiteral("detailError"));
        error->setWordWrap(true);
        cardLayout->addWidget(error);
        cardLayout->addSpacing(12);
    }

    auto *actions = new QVBoxLayout();
    actions->setContentsMargins(0, 8, 0, 0);
    actions->setSpacing(8);
    actions->setAlignment(Qt::AlignLeft);

    const bool update = m_model.availability == AppAvailability::UpdateAvailable ||
                        (m_model.availability == AppAvailability::Error && m_model.retryUpdate);
    const bool installed = m_model.availability == AppAvailability::Installed ||
                           m_model.availability == AppAvailability::UpdateAvailable ||
                           (m_model.availability == AppAvailability::Error &&
                            (m_model.retryUninstall || m_model.retryUpdate));
    const bool install = m_model.availability == AppAvailability::NotInstalled ||
                         (m_model.availability == AppAvailability::Error && !m_model.retryUninstall && !m_model.retryUpdate);

    if (!isBusy(m_model.availability))
    {
        if (update)
        {
            auto *button = createButton(QStringLiteral("Update"), true);
            connect(button, &QPushButton::clicked, this, [this]() { emit updateRequested(m_model.packageId); });
            actions->addWidget(button, 0, Qt::AlignLeft);
        }
        if (installed)
        {
            auto *button = createButton(QStringLiteral("Launch"), true);
            connect(button, &QPushButton::clicked, this, [this]() { emit launchRequested(m_model.packageId); });
            actions->addWidget(button, 0, Qt::AlignLeft);

            auto *remove = createButton(QStringLiteral("Uninstall"), false);
            connect(remove, &QPushButton::clicked, this, [this]() { emit uninstallRequested(m_model.packageId); });
            actions->addWidget(remove, 0, Qt::AlignLeft);
        }
        else if (install)
        {
            auto *button = createButton(QStringLiteral("Install"), true);
            connect(button, &QPushButton::clicked, this, [this]() { emit installRequested(m_model.packageId); });
            actions->addWidget(button, 0, Qt::AlignLeft);
        }
    }

    cardLayout->addLayout(actions);

    auto *cardRow = new QHBoxLayout();
    cardRow->setContentsMargins(0, 0, 0, 0);
    cardRow->setSpacing(0);
    cardRow->addWidget(card, 0, Qt::AlignLeft);
    cardRow->addStretch(1);
    page->addLayout(cardRow);
    page->addStretch(1);

    m_root->insertWidget(1, m_body);
}

QPushButton *AppDetailPage::createButton(const QString &text, bool primary) const
{
    auto *button = new QPushButton(text);
    button->setObjectName(primary ? QStringLiteral("primaryButton") : QStringLiteral("secondaryButton"));
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedSize(112, 36);
    button->setFocusPolicy(Qt::NoFocus);
    return button;
}
