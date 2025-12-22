#ifndef COLOURS_H
#define COLOURS_H

#include <QColor>

namespace Colours {
	// Accent colors
	const QColor Rosewater = QColor("#f5e0dc"); // Soft rose
	const QColor Flamingo = QColor("#f2cdcd"); // Light pink
	const QColor Pink = QColor("#f5c2e7"); // Pastel pink
	const QColor Mauve = QColor("#cba6f7"); // Purple mauve
	const QColor Red = QColor("#f38ba8"); // Soft red
	const QColor Maroon = QColor("#eba0ac"); // Muted maroon
	const QColor Peach = QColor("#fab387"); // Warm peach
	const QColor Yellow = QColor("#f9e2af"); // Soft yellow
	const QColor Green = QColor("#a6e3a1"); // Pastel green
	const QColor Teal = QColor("#94e2d5"); // Light teal
	const QColor Sky = QColor("#89dceb"); // Sky blue
	const QColor Sapphire = QColor("#74c7ec"); // Bright sapphire
	const QColor Blue = QColor("#89b4fa"); // Soft blue
	const QColor Lavender = QColor("#b4befe"); // Light lavender

	// Base colors (backgrounds and surfaces)
	const QColor Base = QColor("#1e1e2e"); // Main background
	const QColor Mantle = QColor("#181825"); // Slightly darker surface
	const QColor Crust = QColor("#11111b"); // Darkest surface

	// Overlay and text colors
	const QColor Overlay0 = QColor("#6c7086"); // Subtle overlay
	const QColor Overlay1 = QColor("#7f849c"); // Overlay
	const QColor Overlay2 = QColor("#9399b2"); // Highlight overlay

	const QColor Surface0 = QColor("#313244"); // Raised surface
	const QColor Surface1 = QColor("#45475a"); // Higher surface
	const QColor Surface2 = QColor("#585b70"); // Highest surface

	const QColor Text = QColor("#cdd6f4"); // Primary text
	const QColor Subtext0 = QColor("#a6adc8"); // Secondary text
	const QColor Subtext1 = QColor("#bac2de"); // Subtle text

} // namespace Colours

#endif // COLOURS_H