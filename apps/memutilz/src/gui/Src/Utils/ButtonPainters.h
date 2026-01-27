#pragma once

#include <QPainter>
#include <QStyleOptionToolButton>

namespace Utils {
namespace Painters {
	void drawRoundedBarLeft(
		QPainter* p,
		const QStyleOptionToolButton& opt
	);
} // namespace Painters
} // namespace Utils