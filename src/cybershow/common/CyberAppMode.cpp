#include "CyberAppMode.h"

namespace cybershow {

QString launchModeToString(LaunchMode mode)
{
    switch (mode) {
    case LaunchMode::Demo: return QStringLiteral("demo");
    case LaunchMode::Live: return QStringLiteral("live");
    }
    return QStringLiteral("live");
}

ParseResult parseAppLaunchOptions(const QStringList& arguments)
{
    ParseResult result;
    bool modeSeen = false;

    // No arguments means live mode. Arguments normally include argv[0].
    result.options.launchMode = LaunchMode::Live;
    result.options.originalModeArgument = QStringLiteral("--live");

    for (int i = 1; i < arguments.size(); ++i) {
        const QString arg = arguments.at(i);

        if (arg == QStringLiteral("--demo") || arg == QStringLiteral("--live")) {
            if (modeSeen) {
                result.ok = false;
                result.error = QStringLiteral("Multiple launch modes provided. Use only one of --demo or --live.");
                return result;
            }
            modeSeen = true;
            result.options.originalModeArgument = arg;
            result.options.launchMode = (arg == QStringLiteral("--demo")) ? LaunchMode::Demo : LaunchMode::Live;
        } else if (arg == QStringLiteral("--fullscreen")) {
            result.options.fullscreen = true;
        } else if (arg == QStringLiteral("--windowed")) {
            result.options.windowed = true;
        } else if (arg == QStringLiteral("--screen")) {
            if (i + 1 >= arguments.size()) {
                result.ok = false;
                result.error = QStringLiteral("Missing value after --screen.");
                return result;
            }

            bool ok = false;
            const int value = arguments.at(++i).toInt(&ok);
            if (!ok || value < 0) {
                result.ok = false;
                result.error = QStringLiteral("Invalid --screen value. Expected a non-negative integer.");
                return result;
            }
            result.options.screenIndex = value;
        } else {
            result.ok = false;
            result.error = QStringLiteral("Unknown argument: %1").arg(arg);
            return result;
        }
    }

    if (result.options.fullscreen && result.options.windowed) {
        result.ok = false;
        result.error = QStringLiteral("Use only one of --fullscreen or --windowed.");
        return result;
    }

    return result;
}

} // namespace cybershow
