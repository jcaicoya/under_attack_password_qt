#include "PasswordWindow.h"

#include "ScreenPage.h"
#include "AttackScreen.h"
#include "AppConfig.h"
#include "PasswordWsServer.h"
#include "UdpBeacon.h"
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
#include <QWidget>
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
        {1, QStringLiteral("ataque"), QStringLiteral("Ataque"), nullptr},
    };

    buildUi();
    wireNavigation();
    setupBadgeOverlay();
    goTo(0);

    // Start WebSocket server and UDP beacon
    m_wsServer = new PasswordWsServer(this);
    connect(m_wsServer, &PasswordWsServer::passwordReceived,
            m_attackScreen, &AttackScreen::onPasswordReceived);
    connect(m_wsServer, &PasswordWsServer::clientConnected,
            m_attackScreen, &AttackScreen::onClientConnected);
    connect(m_wsServer, &PasswordWsServer::clientConnected, this, [this](bool connected) {
        m_mobileConnected = connected;
        updateConnectionBadge();
    });
    connect(m_attackScreen, &AttackScreen::verdictReady,
            m_wsServer, &PasswordWsServer::sendVerdict);

    m_udpBeacon = new UdpBeacon(static_cast<quint16>(AppConfig::instance().wsPort), this);
    m_udpBeacon->start();

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

    m_attackScreen = new AttackScreen(this);
    auto* attackPage = new ScreenPage(QStringLiteral("ataque"), QStringLiteral("ANÁLISIS DE CONTRASEÑA"), this);
    attackPage->contentLayout()->setContentsMargins(0, 0, 0, 0);
    attackPage->contentLayout()->addWidget(m_attackScreen, 1);
    m_screens[0].page = attackPage;

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
    repositionBadgeOverlay();
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
    case Qt::Key_F8:
        setBottomNavVisible(!m_bottomNavVisible);
        return true;
    case Qt::Key_F9:
        setConnectionBadgeVisible(!m_connectionBadgeVisible);
        return true;
    case Qt::Key_F10:
        setModeBadgeVisible(!m_modeBadgeVisible);
        return true;
    case Qt::Key_F11:
        isFullScreen() ? showNormal() : showFullScreen();
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

void PasswordWindow::setupBadgeOverlay()
{
    const double scale = displayScaleForOptions(m_options);

    m_badgeOverlay = new QWidget(this);
    m_badgeOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_badgeOverlay->setFocusPolicy(Qt::NoFocus);
    m_badgeOverlay->setStyleSheet("background: transparent;");

    auto* layout = new QVBoxLayout(m_badgeOverlay);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_modeBadge = new QLabel(m_badgeOverlay);
    m_modeBadge->setFocusPolicy(Qt::NoFocus);
    m_modeBadge->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_modeBadge->setTextInteractionFlags(Qt::NoTextInteraction);
    m_modeBadge->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_modeBadge->setStyleSheet(QString(
        "color: white; background: transparent; font-family: Consolas, monospace; "
        "font-size: %1px; font-weight: 900; letter-spacing: %2px;")
        .arg(qBound(46, int(56 * scale), 72))
        .arg(qBound(4, int(6 * scale), 8)));
    updateModeBadgeText();
    m_modeBadge->hide();

    m_connectionBadge = new QLabel(m_badgeOverlay);
    m_connectionBadge->setFocusPolicy(Qt::NoFocus);
    m_connectionBadge->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_connectionBadge->setTextInteractionFlags(Qt::NoTextInteraction);
    m_connectionBadge->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    updateConnectionBadge();
    m_connectionBadge->hide();

    layout->addWidget(m_modeBadge);
    layout->addWidget(m_connectionBadge);

    m_badgeOpacity = new QGraphicsOpacityEffect(m_badgeOverlay);
    m_badgeOpacity->setOpacity(0.18);
    m_badgeOverlay->setGraphicsEffect(m_badgeOpacity);

    m_badgePulse = new QPropertyAnimation(m_badgeOpacity, "opacity", this);
    m_badgePulse->setStartValue(0.18);
    m_badgePulse->setKeyValueAt(0.5, 0.92);
    m_badgePulse->setEndValue(0.18);
    m_badgePulse->setDuration(2500);
    m_badgePulse->setEasingCurve(QEasingCurve::InOutSine);
    m_badgePulse->setLoopCount(-1);
    m_badgePulse->start();

    m_badgeOverlay->hide();
    repositionBadgeOverlay();
}

void PasswordWindow::repositionBadgeOverlay()
{
    if (!m_badgeOverlay) return;
    m_badgeOverlay->adjustSize();
    const int x = width() - m_badgeOverlay->width() - 48;
    const int y = (height() - m_badgeOverlay->height()) / 2;
    m_badgeOverlay->move(qMax(0, x), qMax(0, y));
    m_badgeOverlay->raise();
}

void PasswordWindow::updateModeBadgeText()
{
    if (!m_modeBadge) return;
    m_modeBadge->setText(m_options.launchMode == cybershow::LaunchMode::Demo
        ? QStringLiteral("DEMO")
        : QStringLiteral("LIVE"));
}

void PasswordWindow::updateConnectionBadge()
{
    if (!m_connectionBadge) return;
    const double scale = displayScaleForOptions(m_options);
    const QString color = m_mobileConnected ? QStringLiteral("#00FF55") : QStringLiteral("#FF3347");
    const QString text  = m_mobileConnected ? QStringLiteral("ENLACE ACTIVO") : QStringLiteral("SIN ENLACE");
    m_connectionBadge->setText(text);
    m_connectionBadge->setStyleSheet(QString(
        "color: %1; background: transparent; font-family: Consolas, monospace; "
        "font-size: %2px; font-weight: 900;")
        .arg(color)
        .arg(qBound(20, int(28 * scale), 36)));
    repositionBadgeOverlay();
}

void PasswordWindow::updateBadgeOverlayVisibility()
{
    if (!m_badgeOverlay) return;
    if (m_modeBadge)       m_modeBadge->setVisible(m_modeBadgeVisible);
    if (m_connectionBadge) m_connectionBadge->setVisible(m_connectionBadgeVisible);
    m_badgeOverlay->setVisible(m_modeBadgeVisible || m_connectionBadgeVisible);
    repositionBadgeOverlay();
}

void PasswordWindow::setModeBadgeVisible(bool visible)
{
    m_modeBadgeVisible = visible;
    updateModeBadgeText();
    updateBadgeOverlayVisibility();
}

void PasswordWindow::setConnectionBadgeVisible(bool visible)
{
    m_connectionBadgeVisible = visible;
    updateBadgeOverlayVisibility();
}

void PasswordWindow::setBottomNavVisible(bool visible)
{
    m_bottomNavVisible = visible;
    if (m_bottomNav) {
        m_bottomNav->setVisible(visible);
    }
}

