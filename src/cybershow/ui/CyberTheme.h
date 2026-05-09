#pragma once

#include <QColor>
#include <QString>

namespace CyberTheme {

inline constexpr const char* BackgroundBase = "#050608";
inline constexpr const char* BackgroundSoft = "#080B10";
inline constexpr const char* BackgroundBlueDepth = "#0A1018";
inline constexpr const char* AccentPrimary = "#1688E8";
inline constexpr const char* AccentCyan = "#00D1FF";
inline constexpr const char* AccentGreen = "#00FF55";

QColor color(const char* hex, int alpha = 255);
QString globalStyleSheet(double scale = 1.0);

} // namespace CyberTheme
