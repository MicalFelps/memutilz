#include "SideBar/Config.h"
#include "SideBar/IconButton.h"
#include "Colors.h"

#include <QFontMetrics>
#include <QPainter>

IconButton::IconButton(const QIcon& icon, const QString& text, QWidget* parent)
	: QPushButton(parent)
	, _icon{ icon }
	, _text{ text }
{
	// I don't want the buttons to look 3D
	setFlat(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setMouseTracking(true);

	setAttribute(Qt::WA_StyledBackground, false);
}

QSize IconButton::minimumSizeHint() const
{
	// Adjust through trial and error
	return QSize(config::iconSize, config::iconSize);
}
QSize IconButton::sizeHint() const
{
	if (_text.isEmpty()) return minimumSizeHint();

	// base calculations off of what you'd normally expect
	const int typicalHeight = config::iconSize;

	int side = typicalHeight;
	int iconSize = (side * _iconSizePercentage) / 100;
	int padding = (side - iconSize) / 2;

	QFontMetrics fm(font());
	int textWidth = fm.horizontalAdvance(_text);

	int neededWidth = side + _iconTextGap + textWidth + padding;

	return QSize(neededWidth, typicalHeight);
}

// ---------------------------------------------------------

void IconButton::enterEvent(QEnterEvent* event) {
	_hovered = true;
	update();
	QPushButton::enterEvent(event);
}
void IconButton::leaveEvent(QEvent* event) {
	_hovered = false;
	update();
	QPushButton::leaveEvent(event);
}
void IconButton::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event);
	// This line isn't necessary anymore since we do everything ourselves
	// QPushButton::paintEvent(event);

	// for smooooooth edges
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.setRenderHint(QPainter::SmoothPixmapTransform);

	const QRect r = rect();

	// --- Background ---

	QColor bgColor = _hovered ? _hoverBgColor : _normalBgColor;
	p.fillRect(r, bgColor);

	// --- Sizes and Positions ---

	int side = qMin(r.width(), r.height());
	int iconSize = (side * _iconSizePercentage) / 100;

	if (iconSize < 10) {
		return; // Too small to draw
	}

	int padding = (side - iconSize) / 2;

	int iconX = r.left() + padding;
	int iconY = r.top() + (r.height() - iconSize) / 2; // no guarantee width is larger than height

	// --- Selection ---
	if (_selected && _selectable) {
		const int barWidth = 3;
		qreal barRadius = barWidth / 2.0;
		QRectF bar(0, iconY, barWidth, iconSize);

		p.setPen(Qt::NoPen);
		p.setBrush(_selectedBarColor);
		p.drawRoundedRect(bar, barRadius, barRadius);
	}

	// --- Draw Icon ---
	if (!_icon.isNull()) {
		QPixmap pixmap = _icon.pixmap(iconSize, iconSize);
		pixmap.setDevicePixelRatio(devicePixelRatioF());
		p.drawPixmap(iconX, iconY, pixmap);
	}

	// --- Draw Text ---

	if (_text.isEmpty()) return;

	int textX = r.left() + side + _iconTextGap;
	int leftoverWidth = r.width() - textX - padding;

	if (leftoverWidth > 0) {
		int targetFontSize = qRound(iconSize * 0.70);
		targetFontSize = qMax(10, targetFontSize);

		QFont textFont = font();
		textFont.setPixelSize(targetFontSize);

		p.setFont(textFont);

		QFontMetrics fm(textFont);
		int fullTextWidth = fm.horizontalAdvance(_text);

		QString visibleText;

		if (fullTextWidth <= leftoverWidth) {
			// draw full text
			visibleText = _text;
		} else {
			// Figure out the longest possible prefix that fits
			// Do binary search for efficiency
			int low = 0;
			int high = _text.length();
			while (low < high) {
				int mid = (low + high + 1) / 2;
				if (fm.horizontalAdvance(_text.left(mid)) <= leftoverWidth) {
					low = mid;
				} else {
					high = mid - 1;
				}
			}
			visibleText = _text.left(low);
		}

		if (!visibleText.isEmpty()) {
			QRect textRect(textX, r.top(), leftoverWidth, r.height());
			p.setPen(palette().color(QPalette::ButtonText));
			p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, visibleText);
		}
	}
}