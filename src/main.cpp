#include "PasswordWindow.h"
#include "cybershow/common/CyberAppMode.h"
#include "cybershow/common/CyberOperationalLog.h"
#include "cybershow/common/CyberOrchestratorProtocol.h"
#include "cybershow/ui/CyberTheme.h"

#include <QApplication>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>

namespace {

QScreen* targetScreen(const cybershow::AppLaunchOptions& options)
{
    const auto screens = QGuiApplication::screens();
    if (options.screenIndex >= 0 && options.screenIndex < screens.size()) {
        return screens.at(options.screenIndex);
    }
    return QGuiApplication::primaryScreen();
}

double uiScaleForOptions(const cybershow::AppLaunchOptions& options)
{
    const QScreen* screen = targetScreen(options);
    if (!screen) return 1.0;
    return qBound(0.85, screen->geometry().height() / 900.0, 1.15);
}

void showMainWindow(PasswordWindow& window, const cybershow::AppLaunchOptions& options)
{
    QScreen* screen = targetScreen(options);

    if (options.windowed) {
        if (screen) {
            window.setGeometry(screen->availableGeometry());
        }
        window.show();
        return;
    }

    if (screen) {
        window.move(screen->geometry().topLeft());
    }
    window.showFullScreen();
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("under_attack_password_qt"));

    cybershow::OperationalLog::configure(QStringLiteral("startup"), QStringLiteral("unknown"));
    cybershow::OperationalLog::write(QStringLiteral("INFO"), QStringLiteral("startup"), QStringLiteral("Application process started"));

    const cybershow::ParseResult launchParse =
        cybershow::parseAppLaunchOptions(QCoreApplication::arguments());
    if (!launchParse.ok) {
        cybershow::OrchestratorProtocol::status(QStringLiteral("ERROR"), QStringLiteral("INVALID_ARGUMENTS"));
        cybershow::OperationalLog::write(QStringLiteral("ERROR"), QStringLiteral("startup"), QStringLiteral("Invalid launch arguments"));
        QMessageBox::critical(
            nullptr,
            QStringLiteral("Password - Startup Error"),
            QStringLiteral("The application cannot start because the launch arguments are invalid.\n\n")
                + launchParse.error);
        return 2;
    }

    cybershow::OperationalLog::configure(
        cybershow::launchModeToString(launchParse.options.launchMode),
        cybershow::launchModeToString(launchParse.options.launchMode));
    cybershow::OrchestratorProtocol::status(QStringLiteral("READY"));
    cybershow::OperationalLog::write(QStringLiteral("INFO"), QStringLiteral("startup"), QStringLiteral("Application ready"));

    app.setStyle(QStringLiteral("Fusion"));
    app.setStyleSheet(CyberTheme::globalStyleSheet(uiScaleForOptions(launchParse.options)));

    PasswordWindow window(launchParse.options);
    showMainWindow(window, launchParse.options);

    cybershow::OrchestratorProtocol::status(QStringLiteral("RUNNING"));
    cybershow::OperationalLog::write(QStringLiteral("INFO"), QStringLiteral("runtime"), QStringLiteral("Runtime window shown"));

    const int result = app.exec();
    cybershow::OperationalLog::write(QStringLiteral("INFO"), QStringLiteral("runtime"), QString("Application exited with code %1").arg(result));
    return result;
}

