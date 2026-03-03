#include <QPalette>
#include "../../Globals.h"

namespace Memutilz {
namespace Themes {
MEMUTILZ_DEFINE_THEME(dark, p.setColor(QPalette::Window, QColor("#0b0b0b"));
                      p.setColor(QPalette::WindowText, QColor("#bbbbbb"));
                      p.setColor(QPalette::Base, QColor("#121212"));
                      p.setColor(QPalette::AlternateBase, QColor("#181818"));
                      p.setColor(QPalette::ToolTipBase, QColor("#202020"));
                      p.setColor(QPalette::ToolTipText, QColor("#e6e6e6"));
                      p.setColor(QPalette::Text, QColor("#ffffff"));
                      p.setColor(QPalette::Button, QColor("#1c1c1c"));
                      p.setColor(QPalette::ButtonText, QColor("#d6d6d6"));
                      p.setColor(QPalette::BrightText, QColor("#ffffff"));
                      p.setColor(QPalette::Light, QColor("#2a2a2a"));
                      p.setColor(QPalette::Midlight, QColor("#202020"));
                      p.setColor(QPalette::Mid, QColor("#151515"));
                      p.setColor(QPalette::Dark, QColor("#101010"));
                      p.setColor(QPalette::Shadow, QColor("#050505"));
                      p.setColor(QPalette::PlaceholderText, QColor("#7a7a7a"));)
MEMUTILZ_DEFINE_THEME(debug, p.setColor(QPalette::Window, QColor("#FF00FF"));       // Neon Magenta background
                      p.setColor(QPalette::WindowText, QColor("#00FFFF"));          // Bright Cyan text
                      p.setColor(QPalette::Base, QColor("#FFFF00"));                // Neon Yellow for text input background
                      p.setColor(QPalette::AlternateBase, QColor("#FF8000"));       // Bright Orange alternate backgrounds
                      p.setColor(QPalette::ToolTipBase, QColor("#00FF00"));         // Neon Green tooltip background
                      p.setColor(QPalette::ToolTipText, QColor("#0000FF"));         // Bright Blue tooltip text
                      p.setColor(QPalette::Text, QColor("#FF0000"));                // Red normal text
                      p.setColor(QPalette::Button, QColor("#00FFCC"));              // Turquoise buttons
                      p.setColor(QPalette::ButtonText, QColor("#FF00CC"));          // Hot pink button text
                      p.setColor(QPalette::BrightText, QColor("#FFFFFF"));          // Pure white bright text
                      p.setColor(QPalette::Light, QColor("#FFFF66"));               // Light neon yellow
                      p.setColor(QPalette::Midlight, QColor("#FF6600"));            // Neon orange midlight
                      p.setColor(QPalette::Mid, QColor("#66FF66"));                 // Bright green mid
                      p.setColor(QPalette::Dark, QColor("#6600FF"));                // Deep neon purple
                      p.setColor(QPalette::Shadow, QColor("#FF0066"));              // Strong pink shadow
                      p.setColor(QPalette::PlaceholderText, QColor("#00FF99"));)    // Neon green placeholder
}
}  // namespace Memutilz