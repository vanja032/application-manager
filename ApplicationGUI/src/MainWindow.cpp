#include "MainWindow.h"

#include "AppCard.h"
#include "UiIcons.h"

#include <QtCore/QEvent>
#include <QtCore/QFile>
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

AppCardModel makeApp(const QString &name, const QString &packageId, const QString &source, AppAvailability availability)
{
    AppCardModel model;
    model.name = name;
    model.packageId = packageId;
    model.source = source;
    model.availability = availability;
    return model;
}

std::vector<AppCardModel> mockCatalog()
{
    const QString store = QStringLiteral("Microsoft Store");
    const QString winget = QStringLiteral("winget");

    AppCardModel whatsApp = makeApp(QStringLiteral("WhatsApp"), QStringLiteral("9NKSQGP7F2NH"), store, AppAvailability::Installed);

    AppCardModel business =
        makeApp(QStringLiteral("WhatsApp Business"), QStringLiteral("9PG2B0DLY5HT"), store, AppAvailability::UpdateAvailable);
    business.availableVersion = QStringLiteral("2.2639.100.0");

    AppCardModel desktop =
        makeApp(QStringLiteral("WhatsApp Desktop"), QStringLiteral("5319275A.WhatsAppDesktop"), store, AppAvailability::Installing);
    desktop.progressPercent = 68;
    desktop.downloadedMb = 325;
    desktop.totalMb = 520;

    AppCardModel web = makeApp(QStringLiteral("WhatsApp Web"), QStringLiteral("9NBLGGH4M8R0"), store, AppAvailability::NotInstalled);
    AppCardModel beta = makeApp(QStringLiteral("WhatsApp Beta"), QStringLiteral("9NKSQGP7F2NH"), store, AppAvailability::NotInstalled);
    AppCardModel messenger =
        makeApp(QStringLiteral("WhatsApp Messenger"), QStringLiteral("9NKSQGP7F2NH"), store, AppAvailability::NotInstalled);

    AppCardModel code = makeApp(QStringLiteral("Visual Studio Code"), QStringLiteral("Microsoft.VisualStudioCode"), winget,
                                 AppAvailability::UpdateAvailable);
    code.installedVersion = QStringLiteral("1.95.3");
    code.availableVersion = QStringLiteral("1.96.2");

    AppCardModel spotify = makeApp(QStringLiteral("Spotify"), QStringLiteral("9NCBCSZSJRSB"), store, AppAvailability::UpdateAvailable);
    spotify.availableVersion = QStringLiteral("1.2.70.0");

    AppCardModel firefox =
        makeApp(QStringLiteral("Mozilla Firefox"), QStringLiteral("Mozilla.Firefox"), winget, AppAvailability::Installed);
    firefox.installedVersion = QStringLiteral("133.0.3");

    AppCardModel powerToys =
        makeApp(QStringLiteral("PowerToys"), QStringLiteral("Microsoft.PowerToys"), winget, AppAvailability::Updating);
    powerToys.progressPercent = 42;
    powerToys.downloadedMb = 80;
    powerToys.totalMb = 190;

    AppCardModel vlc = makeApp(QStringLiteral("VLC media player"), QStringLiteral("VideoLAN.VLC"), winget, AppAvailability::NotInstalled);

    return {whatsApp, business, desktop, web, beta, messenger, code, spotify, firefox, powerToys, vlc};
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
    ui.updatesButton->installEventFilter(this);

    configureNavigation();
    loadMockCatalog();
    applyTheme(false);
    showSection(Section::Discover);

    m_darkModeSwitch->setOnToggled([this](bool checked) { applyTheme(checked); });

    connect(ui.searchButton, &QPushButton::clicked, this, [this]() { refreshAppList(); });
    connect(ui.searchEdit, &QLineEdit::returnPressed, this, [this]() { refreshAppList(); });
    connect(ui.searchEdit, &QLineEdit::textChanged, this, [this](const QString &) { refreshAppList(); });
    connect(ui.sourceCombo, &QComboBox::currentTextChanged, this, [this](const QString &) { refreshAppList(); });
    connect(ui.sortCombo, &QComboBox::currentTextChanged, this, [this](const QString &) { refreshAppList(); });
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

void MainWindow::loadMockCatalog()
{
    const std::vector<AppCardModel> catalog = mockCatalog();
    m_cards.reserve(catalog.size());

    int updateCount = 0;
    for (int index = 0; index < static_cast<int>(catalog.size()); ++index)
    {
        AppCardModel model = catalog[index];
        model.catalogIndex = index;
        if (model.availability == AppAvailability::UpdateAvailable && model.source == QStringLiteral("Microsoft Store"))
        {
            ++updateCount;
        }

        auto *card = new AppCard(model, ui.appListContainer);
        card->hide();
        m_cards.push_back(card);
    }

    m_updatesBadge->setText(QString::number(updateCount));
    m_updatesBadge->setVisible(updateCount > 0);
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
        palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#9AA3B2")));
    }

    qApp->setPalette(palette);
    qApp->setStyleSheet(loadStyleSheet(dark ? QStringLiteral(":/styles-dark.qss") : QStringLiteral(":/styles.qss")));

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
    m_section = section;
    const bool showList = section == Section::Discover || section == Section::Installed || section == Section::Updates;
    ui.searchRow->setVisible(showList);
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
    refreshAppList();
}

void MainWindow::refreshAppList()
{
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

    ui.emptyStateLabel->setVisible(!anyVisible);
    ui.appScrollArea->setVisible(anyVisible);
    ui.appScrollArea->verticalScrollBar()->setValue(scrollValue);
}

bool MainWindow::cardMatches(const AppCard *card) const
{
    const AppAvailability availability = card->availability();
    if (m_section == Section::Installed && availability == AppAvailability::NotInstalled)
    {
        return false;
    }
    if (m_section == Section::Updates && availability != AppAvailability::UpdateAvailable)
    {
        return false;
    }

    const int filterId = m_filterGroup->checkedId();
    if (filterId == static_cast<int>(StatusFilter::Installed) && availability == AppAvailability::NotInstalled)
    {
        return false;
    }
    if (filterId == static_cast<int>(StatusFilter::NotInstalled) && availability != AppAvailability::NotInstalled)
    {
        return false;
    }

    const QString source = ui.sourceCombo->currentText();
    if (source != QStringLiteral("All sources") && card->source() != source)
    {
        return false;
    }

    const QString query = ui.searchEdit->text().trimmed();
    if (query.isEmpty())
    {
        return true;
    }

    return card->name().contains(query, Qt::CaseInsensitive) || card->packageId().contains(query, Qt::CaseInsensitive);
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
