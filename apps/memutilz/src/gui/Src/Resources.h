#pragma once

#include <QStringList>
#include <QFontDatabase>
#include <QDebug>

#include <QPalette>
#include <QHash>
#include <optional>

void loadFonts();
void loadThemes();

namespace Themes {

QPalette dark;

// --- DARK ---
dark.setColor(QPalette::Window, QColor("#0b0b0b"));
dark.setColor(QPalette::WindowText, QColor("#bbbbbb"));

dark.setColor(QPalette::Base, QColor("#121212"));
dark.setColor(QPalette::AlternateBase,
              QColor("#181818"));  // checked / alternate rows

dark.setColor(QPalette::ToolTipBase, QColor("#202020"));
dark.setColor(QPalette::ToolTipText, QColor("#e6e6e6"));

dark.setColor(QPalette::Text, QColor("#d8d8d8"));
dark.setColor(QPalette::Button, QColor("#1c1c1c"));
dark.setColor(QPalette::ButtonText, QColor("#d6d6d6"));
dark.setColor(QPalette::BrightText,
              QColor("#ffffff"));  // warnings / emphasis only

dark.setColor(QPalette::Light,
              QColor("#2a2a2a"));  // raised edges / highlight
dark.setColor(QPalette::Midlight, QColor("#202020"));  // hover
dark.setColor(QPalette::Mid, QColor("#151515"));       // pressed
dark.setColor(QPalette::Dark, QColor("#0b0b0b"));      // borders
dark.setColor(QPalette::Shadow, QColor("#050505"));    // depth

dark.setColor(QPalette::PlaceholderText, QColor("#7a7a7a"));
// ------------
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
