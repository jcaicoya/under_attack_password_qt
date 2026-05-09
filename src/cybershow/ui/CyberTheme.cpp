#include "CyberTheme.h"

#include <QtGlobal>

namespace CyberTheme {
namespace {

int scaled(int value, double scale)
{
    return qMax(1, qRound(value * scale));
}

} // namespace

QColor color(const char* hex, int alpha)
{
    QColor c(QString::fromUtf8(hex));
    c.setAlpha(alpha);
    return c;
}

QString globalStyleSheet(double scale)
{
    QString css = QStringLiteral(R"QSS(
        QWidget {
            background: transparent;
            color: #F2F5F8;
            font-family: "Segoe UI", "Inter", "Arial";
            font-size: 13px;
        }

        QLabel#ScreenTitle {
            color: #F2F5F8;
            font-size: 24px;
            font-weight: 800;
            letter-spacing: 0.8px;
        }

        QLabel#ScreenSubtitle {
            color: #8D96A3;
            font-size: 13px;
            font-weight: 600;
            letter-spacing: 0.5px;
        }

        QLabel#KickerLabel {
            color: #00D1FF;
            font-size: 11px;
            font-weight: 800;
            letter-spacing: 2px;
        }

        QLabel#PanelTitle {
            color: #F2F5F8;
            font-size: 16px;
            font-weight: 800;
        }

        QLabel#MutedLabel {
            color: #8D96A3;
            font-size: 13px;
        }

        QLabel#StatusInfo {
            color: #00D1FF;
            font-family: "Consolas", "JetBrains Mono", monospace;
            font-size: 12px;
            font-weight: 700;
        }

        QFrame#CyberPanel,
        QWidget#CyberPanel {
            background-color: #101318;
            border: 1px solid #293241;
            border-radius: 12px;
        }

        QFrame#CyberPanelRaised,
        QWidget#CyberPanelRaised {
            background-color: #151922;
            border: 1px solid #334052;
            border-radius: 14px;
        }

        QPushButton {
            background-color: #202633;
            color: #F2F5F8;
            border: 1px solid #3B4654;
            border-radius: 8px;
            padding: 8px 14px;
            font-weight: 700;
        }

        QPushButton:hover {
            background-color: #263044;
            border-color: #64748B;
        }

        QPushButton#NavButton {
            background-color: #141820;
            color: #8D96A3;
            border: 1px solid #293241;
            border-radius: 7px;
            padding: 8px 12px;
            font-size: 12px;
            font-weight: 800;
        }

        QPushButton#NavButton:checked {
            background-color: #202633;
            color: #F2F5F8;
            border: 1px solid #2EA8FF;
        }
    )QSS");

    const struct Replacement {
        const char* from;
        int value;
    } sizes[] = {
        {"font-size: 11px", 11},
        {"font-size: 12px", 12},
        {"font-size: 13px", 13},
        {"font-size: 16px", 16},
        {"font-size: 24px", 24},
    };

    for (const auto& item : sizes) {
        css.replace(item.from, QString("font-size: %1px").arg(scaled(item.value, scale)));
    }

    css.replace("letter-spacing: 0.5px", QString("letter-spacing: %1px").arg(scaled(1, scale)));
    css.replace("letter-spacing: 0.8px", QString("letter-spacing: %1px").arg(scaled(1, scale)));
    css.replace("letter-spacing: 2px", QString("letter-spacing: %1px").arg(scaled(2, scale)));
    css.replace("padding: 8px 14px", QString("padding: %1px %2px").arg(scaled(8, scale)).arg(scaled(14, scale)));
    css.replace("padding: 8px 12px", QString("padding: %1px %2px").arg(scaled(8, scale)).arg(scaled(12, scale)));

    return css;
}

} // namespace CyberTheme
