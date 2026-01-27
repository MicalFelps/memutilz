#include <QRect>
#include "ButtonPainters.h"

namespace Utils {
namespace Painters {
	void drawRoundedBarLeft(QPainter* p, const QStyleOptionToolButton& opt) {
		if (!p) return;

		const QRect r = opt.rect;
		const int height = r.height();

		const int barWidth = 4;
		const int offsetX = 0;
		const int verticalPadding = 6; // top/bottom padding
		const qreal cornerRadius = 2.0;
		const QColor barColor("#6D68CC");

		// -- Drawing --

		p->save();

		p->setRenderHint(QPainter::Antialiasing, true);
		p->setPen(Qt::NoPen);
		p->setBrush(barColor);

		QRectF barRect{
			r.left() + offsetX,
			r.top() + verticalPadding,
			barWidth,
			height - verticalPadding * 2
		};

		p->drawRoundedRect(
			barRect,
			cornerRadius,
			cornerRadius,
			Qt::AbsoluteSize
		);

		p->restore();
	}
}
}