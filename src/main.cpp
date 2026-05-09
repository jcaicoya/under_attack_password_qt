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
#include <QtGlobal>

namespace {

QRect availableGeometryForScreenIndex(int screenIndex)
{
    const auto screens = QGuiApplication::screens();
    if (screenIndex >= 0 && screenIndex < screens.size()) {
        return screens.at(screenIndex)->availableGeometry();
    }

    if (const QScreen* screen = QGuiApplication::primaryScreen()) {
        return screen->availableGeometry();
    }

    return {};
}

double uiScaleForOptions(const cybershow::AppLaunchOptions& options)
{
    const QRect available = availableGeometryForScreenIndex(options.screenIndex);
    if (available.isEmpty()) {
        return 1.0;
    }

    return qBound(0.85, available.height() / 900.0, 1.15);
}

void showMainWindow(PasswordWindow& window, const cybershow::AppLaunchOptions& options)
{
    const QRect available = availableGeometryForScreenIndex(options.screenIndex);
    if (options.fullscreen) {
        if (!available.isEmpty()) {
            window.setGeometry(available);
        }
        window.showFullScreen();
        return;
    }

    if (options.windowed) {
        if (!available.isEmpty()) {
            const QSize targetSize(
                qBound(1024, int(available.width() * 0.90), 1680),
                qBound(700, int(available.height() * 0.90), 1080));
            const QRect targetRect(
                QPoint(
                    available.x() + (available.width() - targetSize.width()) / 2,
                    available.y() + (available.height() - targetSize.height()) / 2),
                targetSize);
            window.setGeometry(targetRect.intersected(available));
        } else {
            window.resize(1024, 700);
        }
        window.show();
        return;
    }

    if (!available.isEmpty()) {
        window.setGeometry(available);
    }
    window.showMaximized();
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("password"));

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

