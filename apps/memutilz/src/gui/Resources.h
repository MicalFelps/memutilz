#pragma once

#include <QStringList>
#include <QPalette>
#include <optional>

#include "../Globals.h"

void loadFonts();
void loadThemes();

namespace Themes {

MEMUTILZ_DEFINE_THEME(dark, p.setColor(QPalette::Window, QColor("#0b0b0b"));
                      p.setColor(QPalette::WindowText, QColor("#bbbbbb"));
                      p.setColor(QPalette::Base, QColor("#121212"));
                      p.setColor(QPalette::AlternateBase, QColor("#181818"));
                      p.setColor(QPalette::ToolTipBase, QColor("#202020"));
                      p.setColor(QPalette::ToolTipText, QColor("#e6e6e6"));
                      p.setColor(QPalette::Text, QColor("#d8d8d8"));
                      p.setColor(QPalette::Button, QColor("#1c1c1c"));
                      p.setColor(QPalette::ButtonText, QColor("#d6d6d6"));
                      p.setColor(QPalette::BrightText, QColor("#ffffff"));
                      p.setColor(QPalette::Light, QColor("#2a2a2a"));
                      p.setColor(QPalette::Midlight, QColor("#202020"));
                      p.setColor(QPalette::Mid, QColor("#151515"));
                      p.setColor(QPalette::Dark, QColor("#101010"));
                      p.setColor(QPalette::Shadow, QColor("#050505"));
                      p.setColor(QPalette::PlaceholderText, QColor("#7a7a7a"));)
}  // namespace Themes

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
