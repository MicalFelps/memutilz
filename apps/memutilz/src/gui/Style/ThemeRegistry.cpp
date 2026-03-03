#include <QFontDatabase>
#include <QHash>
#include <QDebug>

#include "ThemeRegistry.h"
#include "../Constants/Themes.h"

QHash<QString, ThemeInfo> ThemeRegistry::_themes;

void loadFonts() {
    QStringList fontFiles = {
        // Inter (UI)
        ":/fonts/Inter_18pt-Regular.ttf", ":/fonts/Inter_18pt-Bold.ttf",
        ":/fonts/Inter_18pt-Italic.ttf", ":/fonts/Inter_18pt-BoldItalic.ttf",
        ":/fonts/Inter_18pt-SemiBold", ":/fonts/Inter_18pt-SemiBoldItalic.ttf",

        // Monospace
        ":/fonts/SourceCodePro-Regular.ttf", ":/fonts/SourceCodePro-Bold.ttf",
        ":/fonts/SourceCodePro-Italic.ttf",
        ":/fonts/SourceCodePro-BoldItalic.ttf"};

    for (const auto& file : fontFiles) {
        int fontId = QFontDatabase::addApplicationFont(file);
        if (fontId == -1) qWarning() << "Failed to load font:" << file;
    }
}
void loadThemes() {
    ThemeRegistry::getOrRegister(
        "dark", ThemeInfo{ThemeType::dark, Memutilz::Themes::dark()});
    ThemeRegistry::getOrRegister(
        "debug", ThemeInfo{ ThemeType::dark, Memutilz::Themes::debug() });
}

std::optional<ThemeInfo> ThemeRegistry::getOrRegister(
    QString name, std::optional<ThemeInfo> themeInfo) {
    if (_themes.contains(name)) {
        return _themes[name];
    } else {
        if (!themeInfo.has_value()) return std::nullopt;
        _themes[name] = themeInfo.value();
        return themeInfo.value();
    }
}
