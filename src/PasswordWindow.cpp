#include "PasswordWindow.h"

#include "ScreenPage.h"
#include "cybershow/common/CyberOperationalLog.h"
#include "cybershow/common/CyberOrchestratorProtocol.h"
#include "cybershow/ui/BottomNavBar.h"
#include "cybershow/ui/CyberBackgroundWidget.h"

#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QEasingCurve>
#include <QEvent>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QtGlobal>

#include <utility>

namespace {

QRect availableGeometryForOptions(const cybershow::AppLaunchOptions& options)
{
    const auto screens = QApplication::screens();
    if (options.screenIndex >= 0 && options.screenIndex < screens.size()) {
        return screens.at(options.screenIndex)->availableGeometry();
    }

    if (QScreen* screen = QApplication::primaryScreen()) {
        return screen->availableGeometry();
    }

    return {};
}

double displayScaleForOptions(const cybershow::AppLaunchOptions& options)
{
    const QRect available = availableGeometryForOptions(options);
    if (available.isEmpty()) {
        return 1.0;
    }

    const double widthScale = available.width() / 1440.0;
    const double heightScale = available.height() / 900.0;
    return qBound(0.82, qMin(widthScale, heightScale), 1.10);
}

} // namespace

PasswordWindow::PasswordWindow(const cybershow::AppLaunchOptions& options, QWidget* parent)
    : QMainWindow(parent)
    , m_options(options)
{
    m_screens = {
        {1, QStringLiteral("principal"), QStringLiteral("Principal"), nullptr},
        {2, QStringLiteral("segunda"), QStringLiteral("Segunda"), nullptr},
    };

    buildUi();
    wireNavigation();
    setupModeBadge();
    goTo(0);

    qApp->installEventFilter(this);
}

void PasswordWindow::buildUi()
{
    setWindowTitle(QStringLiteral("Password"));
    setMinimumSize(1024, 700);

    auto* central = new CyberBackgroundWidget(this);
    central->setGlowIntensity(0.85);

    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_stack = new QStackedWidget(central);
    m_bottomNav = new BottomNavBar(central);

    m_screens[0].page = createPlaceholderPage(
        QStringLiteral("Pantalla base"),
        QStringLiteral("Contenido placeholder para la primera pantalla del nuevo modulo Cybershow."));
    m_screens[1].page = createPlaceholderPage(
        QStringLiteral("Segunda pantalla"),
        QStringLiteral("Contenido placeholder para la segunda pantalla. Sustituir por la experiencia real de la app."));

    QStringList navLabels;
    for (const Screen& screen : std::as_const(m_screens)) {
        m_stack->addWidget(screen.page);
        navLabels << screen.navLabel;
    }

    m_bottomNav->setItems(navLabels);

    rootLayout->addWidget(m_stack, 1);
    rootLayout->addWidget(m_bottomNav, 0);
    setCentralWidget(central);

    setBottomNavVisible(false);
}

ScreenPage* PasswordWindow::createPlaceholderPage(const QString& title, const QString& body)
{
    auto* page = new ScreenPage(QString(), title, this);

    auto* panel = new QFrame(page);
    panel->setObjectName(QStringLiteral("CyberPanelRaised"));
    panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(12);

    auto* kicker = new QLabel(QStringLiteral("CYBERSHOW PASSWORD"), panel);
    kicker->setObjectName(QStringLiteral("KickerLabel"));
    kicker->setAlignment(Qt::AlignCenter);

    auto* message = new QLabel(body, panel);
    message->setObjectName(QStringLiteral("MutedLabel"));
    message->setAlignment(Qt::AlignCenter);
    message->setWordWrap(true);

    auto* command = new QLabel(QStringLiteral("Reemplazar esta pantalla por la logica visual del nuevo modulo."), panel);
    command->setObjectName(QStringLiteral("StatusInfo"));
    command->setAlignment(Qt::AlignCenter);
    command->setWordWrap(true);

    layout->addStretch(1);
    layout->addWidget(kicker);
    layout->addWidget(message);
    layout->addWidget(command);
    layout->addStretch(1);

    page->contentLayout()->addWidget(panel, 1);
    return page;
}

void PasswordWindow::wireNavigation()
{
    connect(m_bottomNav, &BottomNavBar::currentIndexChanged, this, [this](int index) {
        goTo(index);
    });
}

bool PasswordWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (event && event->type() == QEvent::KeyPress) {
        if (handleRuntimeKeyPress(static_cast<QKeyEvent*>(event))) {
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void PasswordWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateModeBadgeGeometry();
}

bool PasswordWindow::focusIsEditable(QWidget* focusWidget) const
{
    if (!focusWidget) return false;
    if (qobject_cast<QLineEdit*>(focusWidget)) return true;
    if (qobject_cast<QTextEdit*>(focusWidget)) return true;
    if (qobject_cast<QPlainTextEdit*>(focusWidget)) return true;
    if (qobject_cast<QAbstractSpinBox*>(focusWidget)) return true;
    if (auto* combo = qobject_cast<QComboBox*>(focusWidget)) return combo->isEditable();
    return false;
}

bool PasswordWindow::handleRuntimeKeyPress(QKeyEvent* event)
{
    if (!event || focusIsEditable(QApplication::focusWidget())) {
        return false;
    }

    switch (event->key()) {
    case Qt::Key_Left:
        goToAdjacent(-1);
        return true;
    case Qt::Key_Right:
        goToAdjacent(1);
        return true;
    case Qt::Key_F9:
        setBottomNavVisible(!m_bottomNavVisible);
        return true;
    case Qt::Key_F10:
        setModeBadgeVisible(!m_modeBadgeVisible);
        return true;
    default:
        break;
    }

    if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9) {
        const int index = event->key() - Qt::Key_1;
        if (index >= 0 && index < m_screens.size()) {
            goTo(index);
            return true;
        }
    }

    return false;
}

void PasswordWindow::goTo(int index)
{
    if (!m_stack || index < 0 || index >= m_screens.size()) {
        return;
    }

    m_stack->setCurrentIndex(index);
    m_bottomNav->setCurrentIndex(index);

    const Screen& screen = m_screens.at(index);
    cybershow::OrchestratorProtocol::screen(screen.number, screen.id);
    cybershow::OperationalLog::write(QStringLiteral("INFO"), QStringLiteral("navigation"), QString("Screen %1 %2").arg(screen.number).arg(screen.id));
}

void PasswordWindow::goToAdjacent(int direction)
{
    if (!m_stack || m_screens.isEmpty()) {
        return;
    }

    const int current = m_stack->currentIndex();
    const int next = (current + direction + m_screens.size()) % m_screens.size();
    goTo(next);
}

void PasswordWindow::setupModeBadge()
{
    m_modeBadge = new QLabel(this);
    const double scale = displayScaleForOptions(m_options);
    m_modeBadge->setFocusPolicy(Qt::NoFocus);
    m_modeBadge->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_modeBadge->setTextInteractionFlags(Qt::NoTextInteraction);
    m_modeBadge->setAlignment(Qt::AlignCenter);

    const QString style = QString(
        "color: white; background: transparent; font-family: Consolas, monospace; "
        "font-size: %1px; font-weight: 900; letter-spacing: %2px;")
        .arg(qBound(46, int(56 * scale), 72))
        .arg(qBound(4, int(6 * scale), 8));
    m_modeBadge->setStyleSheet(style);

    m_modeBadgeOpacity = new QGraphicsOpacityEffect(m_modeBadge);
    m_modeBadgeOpacity->setOpacity(0.18);
    m_modeBadge->setGraphicsEffect(m_modeBadgeOpacity);

    m_modeBadgeAnim = new QPropertyAnimation(m_modeBadgeOpacity, "opacity", this);
    m_modeBadgeAnim->setStartValue(0.18);
    m_modeBadgeAnim->setKeyValueAt(0.5, 0.92);
    m_modeBadgeAnim->setEndValue(0.18);
    m_modeBadgeAnim->setDuration(2500);
    m_modeBadgeAnim->setEasingCurve(QEasingCurve::InOutSine);
    m_modeBadgeAnim->setLoopCount(-1);
    m_modeBadgeAnim->start();

    updateModeBadgeText();
    updateModeBadgeGeometry();
    setModeBadgeVisible(false);
}

void PasswordWindow::updateModeBadgeText()
{
    if (!m_modeBadge) {
        return;
    }

    m_modeBadge->setText(m_options.launchMode == cybershow::LaunchMode::Demo
        ? QStringLiteral("DEMO")
        : QStringLiteral("LIVE"));
}

void PasswordWindow::updateModeBadgeGeometry()
{
    if (!m_modeBadge) {
        return;
    }

    const QSize badgeSize = m_modeBadge->sizeHint().expandedTo(QSize(200, 84));
    const int marginRight = qMax(32, width() / 18);
    const int x = qMax(0, width() - badgeSize.width() - marginRight);
    const int y = qMax(0, (height() - badgeSize.height()) / 2);
    m_modeBadge->setGeometry(x, y, badgeSize.width(), badgeSize.height());
    m_modeBadge->raise();
}

void PasswordWindow::setModeBadgeVisible(bool visible)
{
    m_modeBadgeVisible = visible;
    if (!m_modeBadge) {
        return;
    }

    updateModeBadgeText();
    updateModeBadgeGeometry();
    m_modeBadge->setVisible(visible);
    if (visible) {
        m_modeBadge->raise();
    }
}

void PasswordWindow::setBottomNavVisible(bool visible)
{
    m_bottomNavVisible = visible;
    if (m_bottomNav) {
        m_bottomNav->setVisible(visible);
    }
}

