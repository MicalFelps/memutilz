#pragma once

#include <QStringList>
#include <QPalette>
#include <optional>

void loadFonts();
void loadThemes();

enum class ThemeType {
    dark,
    light,
};
struct ThemeInfo {
    ThemeType type{};
    QPalette palette{};

    ThemeInfo() = default;
    ThemeInfo(ThemeType t, const QPalette& p) : type(t), palette(p) {}
};

// TODO: Better theme handling (allow overwriting and deleting themes)
class ThemeRegistry {
   public:
    ThemeRegistry() = default;
    static std::optional<ThemeInfo> getOrRegister(
        QString name, std::optional<ThemeInfo> = std::nullopt);

   private:
    static QHash<QString, ThemeInfo> _themes;
};
