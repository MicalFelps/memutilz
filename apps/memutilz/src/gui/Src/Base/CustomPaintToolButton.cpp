#include <QStyleOptionToolButton>
#include <QPainter>

#include "CustomPaintToolButton.h"

namespace Base {
	void CustomPaintToolButton::paintEvent(QPaintEvent* event) {
		QStyleOptionToolButton opt;
		initStyleOption(&opt);

		bool isHovered = opt.state & QStyle::State_MouseOver;
		bool isChecked = opt.state & QStyle::State_On;

		bool drawCustom = (
			(isHovered && _hoverPainter.has_value())
			|| (isChecked && _checkedPainter.has_value()));

		if (!drawCustom) {
			QToolButton::paintEvent(event);
			return;
		}

		// -- Apply custom drawings --

		QPainter p{ this };
		p.setRenderHint(QPainter::Antialiasing, true);

		// Draw all base elements, this is the safest option
		// Calling QToolButton::paintEvent(event) instead might be better
		style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);

		if (isHovered && _hoverPainter.has_value()) {
			auto hoverPainter = *_hoverPainter;
			hoverPainter(&p, opt);
		}
		if (isChecked && _checkedPainter.has_value()) {
			auto checkedPainter = *_checkedPainter;
			checkedPainter(&p, opt);
		}
	}
}
