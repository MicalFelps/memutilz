#include <QStyle>
#include <QRegularExpression>
#include <QStyleOptionToolButton>
#include <QPainter>
#include <QMouseEvent>

#include <QPainterPath>

#include <QTextLayout>

#include "Widgets/IconButton.h"
#include "Widgets/Metrics.h"

IconButton::IconButton(const QIcon& icon, const QString& text, QWidget* parent)
	: QToolButton(parent)
{
	setIcon(icon);
	setText(text);
	setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	setPopupMode(QToolButton::InstantPopup);

	setFocusPolicy(Qt::TabFocus);

	connect(this, &IconButton::toggled, this, [this](bool checked) {
		if (!checked) clearFocus(); });
}

QSize IconButton::minimumSizeHint() const {
	const int frame = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, this);
	const int margin = style()->pixelMetric(QStyle::PM_ButtonMargin, nullptr, this);

	const QSize chrome{ (margin + frame) * 2,
						(margin + frame) * 2 };

	QSize minimum = QSize(Ui::IconButton::minSize, Ui::IconButton::minSize);

	bool hasIcon = !icon().isNull();
	bool hasText = !text().isEmpty();
	bool hasMenu = menu() != nullptr;

	bool isHorizontal = (toolButtonStyle() != Qt::ToolButtonTextUnderIcon);

	// none case
	if ((!hasIcon && !_reserveIconSpace) && !hasText && !hasMenu) {
		return minimum + chrome;
	}

	// icon metrics
	const int minScaledIconSize = qMax(
		Ui::IconButton::minScaledIconSize,
		Ui::IconButton::baseIconSize * _iconScalePercent / 100);
	const int minIconW = (hasIcon || _reserveIconSpace) ? minScaledIconSize : 0;
	const int minIconH = minIconW;

	// text metrics
	QFontMetrics fm{ font() };
	int minTextW = 0;
	int minTextH = 0;

	// menu metrics
	const int indicatorW = qRound(fm.height() * Ui::IconButton::indicatorWidthFactor);
	const int indicatorH = qRound(fm.height() * Ui::IconButton::indicatorHeightFactor);
	const int spacing = fm.horizontalAdvance(' ');
	const int extraIndicatorW = indicatorW + spacing;

	// constraints
	/*
	horizontal
	- no wrap
	- if menu, no clipping
	vertical
	- forces noclip
	*/
	TextWrapMode wrapMode = _wrapMode;
	TextTruncateMode truncateMode = _truncateMode;
	MenuIndicatorLayout menuIndicatorLayout = _menuIndicatorLayout;

	if (isHorizontal && wrapMode == TextWrapMode::WrapToFit) {
		if (wrapMode == TextWrapMode::WrapToFit) wrapMode = TextWrapMode::NoWrap; // prefer no wrap in horizontal
		if (hasMenu && _menuIndicatorLayout == MenuIndicatorLayout::Compact) {
			truncateMode = TextTruncateMode::NoClip;
			menuIndicatorLayout = MenuIndicatorLayout::Separate;
		}
	}
	else if (!isHorizontal && truncateMode != TextTruncateMode::NoClip) {
		truncateMode = TextTruncateMode::NoClip; // prefer full wrap in vertical
	}

	// --- Text Calculations ---
	if (hasText) {

		// only happens for horizontal
		if (truncateMode != TextTruncateMode::NoClip) {
			minTextW = 0;
			minTextH = fm.height();
		}
		else {
			// Layout
			QTextLayout layout{ text(), font() };

			QTextOption opt;
			opt.setAlignment(Qt::AlignLeft);
			opt.setWrapMode(
				wrapMode == TextWrapMode::WrapToFit
				? QTextOption::WordWrap
				: QTextOption::NoWrap);

			layout.setTextOption(opt);

			layout.beginLayout();

			qreal longestLineW = 0;
			qreal lastLineW = 0;
			qreal textH = 0;

			// width constraints
			const qreal lineWidth =
				(wrapMode == TextWrapMode::WrapToFit)
				? Ui::IconButton::minWrapWidth
				: std::numeric_limits<qreal>::max();

			while (true) {
				QTextLine line = layout.createLine();
				if (!line.isValid()) {
					break;
				}

				line.setLineWidth(lineWidth);
				lastLineW = line.naturalTextWidth();
				longestLineW = qMax(longestLineW, lastLineW);
				textH += line.height();

				// force single line
				if (wrapMode == TextWrapMode::NoWrap) break;
			}

			layout.endLayout();

			minTextW = qCeil(longestLineW);
			minTextH = qCeil(textH);

			if (wrapMode == TextWrapMode::WrapToFit) {
				minTextW = qMax(Ui::IconButton::minWrapWidth, minTextW);
			}
			else {
				minTextW = fm.horizontalAdvance(text());
			}

			if (hasMenu && (menuIndicatorLayout == MenuIndicatorLayout::Compact)) {
				minTextW = qMax(minTextW, qCeil(lastLineW) + extraIndicatorW);
			}
		}
	}

	// adding up
	int totalMinW = 0;
	int totalMinH = 0;

	int iconTextSpacing = (minIconW > 0 && minTextW > 0) ? _iconTextSpacing : 0;
	if (isHorizontal) {
		totalMinW = minIconW + iconTextSpacing + minTextW;
		totalMinH = qMax(minIconH, minTextH);
	}
	else { // vertical
		totalMinW = qMax(minIconW, minTextW);
		totalMinH = minIconH + iconTextSpacing + minTextH;
	}

	// we've already accounted for this in text if compact
	if (hasMenu
		&& (menuIndicatorLayout == MenuIndicatorLayout::Separate)) {
		if (isHorizontal) {
			totalMinW += indicatorW + _menuIndicatorSpacing;
		}
		else {
			totalMinH += indicatorH + _menuIndicatorSpacing;
		}
	}
	return minimum.expandedTo(QSize(totalMinW, totalMinH)) + chrome;
}
QSize IconButton::sizeHint() const {
	QSize minimum = minimumSizeHint();
	int preferredW = minimum.width();
	int preferredH = minimum.height();

	if (!text().isEmpty()) {
		QFontMetrics fm{ font() };
		int fullTextW = fm.horizontalAdvance(text());
		
		if (toolButtonStyle() == Qt::ToolButtonTextBesideIcon) {
			if (_truncateMode != TextTruncateMode::NoClip) {
				preferredW += fullTextW;
			}
		}
	}

	return QSize(preferredW, preferredH);
}

// ---------------------------------------------------------

void IconButton::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event);

	QPainter p{ this };
	p.setRenderHint(QPainter::TextAntialiasing, true);

	QStyleOptionToolButton opt;
	opt.initFrom(this);

	// add states manually
	if (isDown()) opt.state |= QStyle::State_Sunken;
	if (isChecked()) opt.state |= QStyle::State_On;

	// add features manually
	bool hasMenu = menu() != nullptr;
	if (hasMenu) {
		opt.features |= QStyleOptionToolButton::HasMenu;
	}

	bool hasIcon = !icon().isNull();
	bool showIcon = hasIcon || _reserveIconSpace;
	bool isHorizontal = (toolButtonStyle() == Qt::ToolButtonTextBesideIcon);

	QRect buttonRect = rect();
	QRect contentRect = buttonRect.adjusted(_horizontalPadding, _verticalPadding, -_horizontalPadding, -_verticalPadding);

	int iconSize = qMin(contentRect.width(), contentRect.height());
	int scaledIconSize = iconSize * _iconScalePercent / 100;

	// Draw base panel (background, hover, border, etc)
	style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);

	// Text
	TextWrapMode wrapMode = _wrapMode;
	TextTruncateMode truncateMode = _truncateMode;
	MenuIndicatorLayout menuIndicatorLayout = _menuIndicatorLayout;

	if (isHorizontal && wrapMode == TextWrapMode::WrapToFit) {
		if (wrapMode == TextWrapMode::WrapToFit) wrapMode = TextWrapMode::NoWrap; // prefer no wrap in horizontal
		if (hasMenu && _menuIndicatorLayout == MenuIndicatorLayout::Compact) {
			truncateMode = TextTruncateMode::NoClip;
			menuIndicatorLayout = MenuIndicatorLayout::Separate;
		}
	}
	else if (!isHorizontal && truncateMode != TextTruncateMode::NoClip) {
		truncateMode = TextTruncateMode::NoClip; // prefer full wrap in vertical
	}

	QFontMetrics fm{ font() };
	QString t{ text() };
	QString displayText{ t };

	if (!displayText.isEmpty() && isHorizontal) {
		if (truncateMode != TextTruncateMode::NoClip) {
			int availableW = _textPaintRect.width();
			int textW = fm.horizontalAdvance(displayText);

			if (truncateMode == TextTruncateMode::Ellipsis) {
				displayText = fm.elidedText(t, Qt::ElideRight, availableW);
			} else {
				int low = 0;
				int high = t.length();
				while (low < high) {
					int mid = (low + high + 1) / 2;
					if (fm.horizontalAdvance(t.left(mid)) <= availableW) {
						low = mid;
					}
					else {
						high = mid - 1;
					}
				}
				displayText = t.left(low);
			}
		}
	}

	QTextLayout layout(displayText, font());

	QTextOption textOpt;
	if (isHorizontal) {
		textOpt.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	} else {
		textOpt.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	}

	textOpt.setWrapMode(
		wrapMode == TextWrapMode::WrapToFit
		? QTextOption::WordWrap
		: QTextOption::NoWrap);

	layout.setTextOption(textOpt);



	layout.beginLayout();

	qreal longestLineW = 0;
	qreal lastLineW = 0;
	qreal textH = 0;

	// width constraints
	const qreal lineWidth =
		(wrapMode == TextWrapMode::WrapToFit)
		? Ui::IconButton::minWrapWidth
		: std::numeric_limits<qreal>::max();

	while (true) {
		QTextLine line = layout.createLine();
		if (!line.isValid()) {
			break;
		}

		line.setLineWidth(lineWidth);
		longestLineW = qMax(longestLineW, line.naturalTextWidth());
		textH += line.height();

		// force single line
		if (wrapMode == TextWrapMode::NoWrap) break;
	}

	layout.endLayout();

	// ---------------------------------------------------------

	if (!isEnabled()) {
		p.setOpacity(0.35);
	}

	// Draw Icon
	if (hasIcon
		&& _iconPaintRect.size().width() > Ui::IconButton::minScaledIconSize
		&& _iconPaintRect.size().height() > Ui::IconButton::minScaledIconSize) {

		icon().paint(&p, _iconPaintRect, Qt::AlignCenter);
	}

	// Draw text
	if (!displayText.isEmpty()) {
		p.setPen(palette().color(QPalette::ButtonText));
		layout.draw(&p, QPointF(_textPaintRect.left(), _textPaintRect.top()));
	}

	// Draw menu arrow
	if (hasMenu) {
		QRect target = _textPaintRect;
		target.setX(42);
		target.setY(target.y() + 7);
		target.setWidth(_menuPaintRect.width());
		target.setHeight(_menuPaintRect.height());

		drawMenuIndicator(p, target, 2);
		qDebug() << "Menu Rect:" << target;
	}

	// Draw checked bar
	if (isChecked() && _checkedBarColor.isValid()) {
		const int barWidth = 4;
		qreal barRadius = barWidth / 2.0;
		QRectF barRect{ QRectF() };
		int barLength = scaledIconSize;
		if (isHorizontal) {
			barRect = QRectF(0, buttonRect.top() + (buttonRect.height() - barLength) / 2, barWidth, barLength);
		}
		else {
			barRect = QRectF(buttonRect.left() + (buttonRect.width() - barLength) / 2, 0, barLength, barWidth);
		}
		p.setPen(Qt::NoPen);
		p.setBrush(_checkedBarColor);
		p.drawRoundedRect(barRect, barRadius, barRadius);
	}
}

void IconButton::resizeEvent(QResizeEvent* event) {
	QToolButton::resizeEvent(event);

	updateRectLayout();
}
void IconButton::mousePressEvent(QMouseEvent* event) {
	if (menu()) {
		if (!_iconHitRect.contains(event->pos())) {
			showMenu();
			return;
		}

		QMenu* oldMenu = menu();
		setMenu(nullptr);
		QToolButton::mousePressEvent(event);
		setMenu(oldMenu);

		return;
	}

	QToolButton::mousePressEvent(event);
}

// ---------------------------------------------------------

void IconButton::updateRectLayout() {
	QRect buttonRect = rect();
	QRect contentRect = buttonRect.adjusted(_horizontalPadding, _verticalPadding, -_horizontalPadding, -_verticalPadding);

	bool hasIcon = !icon().isNull();
	bool showIcon = hasIcon || _reserveIconSpace;
	bool hasText = !text().isEmpty();
	bool hasMenu = menu() != nullptr;

	bool isHorizontal = (toolButtonStyle() == Qt::ToolButtonTextBesideIcon);

	// Icon
	int iconSize = qMin(contentRect.width(), contentRect.height());
	int scaledIconSize = iconSize * _iconScalePercent / 100;

	// Text
	QFontMetrics fm{ font() };
	QTextLayout layout{ text(), font() };

	// Menu
	const int indicatorW = qRound(fm.height() * Ui::IconButton::indicatorWidthFactor);
	const int indicatorH = qRound(fm.height() * Ui::IconButton::indicatorHeightFactor);
	const int spacing = fm.horizontalAdvance(' ');
	const int extraIndicatorW = indicatorW + spacing;

	// constraints
	TextWrapMode wrapMode = _wrapMode;
	TextTruncateMode truncateMode = _truncateMode;
	MenuIndicatorLayout menuIndicatorLayout = _menuIndicatorLayout;

	if (isHorizontal && wrapMode == TextWrapMode::WrapToFit) {
		if (wrapMode == TextWrapMode::WrapToFit) wrapMode = TextWrapMode::NoWrap; // prefer no wrap in horizontal
		if (hasMenu && _menuIndicatorLayout == MenuIndicatorLayout::Compact) {
			truncateMode = TextTruncateMode::NoClip;
			menuIndicatorLayout = MenuIndicatorLayout::Separate;
		}
	}
	else if (!isHorizontal && truncateMode != TextTruncateMode::NoClip) {
		truncateMode = TextTruncateMode::NoClip; // prefer full wrap in vertical
	}

	// ----- Paint Rectangles -----

	_iconPaintRect = QRect();
	_textPaintRect = QRect();
	_menuPaintRect = QRect();

	if (showIcon) {
		if (isHorizontal) {
			_iconPaintRect = QRect(
				contentRect.left() + (contentRect.height() - scaledIconSize) / 2,
				contentRect.top() + (contentRect.height() - scaledIconSize) / 2,
				scaledIconSize,
				scaledIconSize);
		}
		else {
			_iconPaintRect = QRect(
				contentRect.left() + (contentRect.width() - scaledIconSize) / 2,
				contentRect.top() + (contentRect.width() - scaledIconSize) / 2,
				scaledIconSize,
				scaledIconSize
			);
		}
	}

	if (hasText) {
		int width = 0;
		int height = 0;

		QTextOption opt;
		opt.setAlignment(Qt::AlignLeft);
		opt.setWrapMode(
			wrapMode == TextWrapMode::WrapToFit
			? QTextOption::WordWrap
			: QTextOption::NoWrap);

		layout.setTextOption(opt);

		layout.beginLayout();

		QList <QTextLine> lines;
		qreal longestLineW = 0;
		qreal lastLineW = 0;
		qreal textH = 0;

		// width constraints
		const qreal lineWidth =
			(wrapMode == TextWrapMode::WrapToFit)
			? Ui::IconButton::minWrapWidth
			: std::numeric_limits<qreal>::max();

		while (true) {
			QTextLine line = layout.createLine();
			if (!line.isValid()) {
				break;
			}

			line.setLineWidth(lineWidth);
			lastLineW = line.naturalTextWidth();
			longestLineW = qMax(longestLineW, lastLineW);
			textH += line.height();
			lines.append(line);

			// force single line
			if (wrapMode == TextWrapMode::NoWrap) break;
		}

		layout.endLayout();

		width = qCeil(longestLineW);
		height = qCeil(textH);

		if (wrapMode == TextWrapMode::WrapToFit) {
			width = qMax(Ui::IconButton::minWrapWidth, width);
		}
		else {
			width = fm.horizontalAdvance(text());
		}

		if (hasMenu && (menuIndicatorLayout == MenuIndicatorLayout::Compact)) {
			width = qMax(width, qCeil(lastLineW) + extraIndicatorW);
		}

		if (isHorizontal) {
			height = qMin(contentRect.height(), height);
			int textX = showIcon ? _iconPaintRect.right() + _iconTextSpacing : contentRect.left();
			int textY = (contentRect.height() - height) / 2;

			_textPaintRect = QRect(textX, textY, width, height);
		}
		else {
			width = qMin(contentRect.width(), width);
			int textX = (contentRect.width() - width) / 2;
			int textY = showIcon ? _iconPaintRect.bottom() + _iconTextSpacing : contentRect.top();

			_textPaintRect = QRect(textX, textY, width, height);
		}

		/*
		Figure out menuPaintRect while here
		*/

		const QTextLine& lastLine = lines.last();
		int lastCharIndex = text().length() - 1;

		qreal x = lastLine.naturalTextWidth() + spacing;
		qreal y = lastLine.y();

		_menuPaintRect = QRect(x, y, indicatorH, indicatorW);
	}

	if (hasMenu) {
		if (isHorizontal) {
			if (!hasText
				|| !(_truncateMode == TextTruncateMode::NoClip)) {
				_menuIndicatorLayout = MenuIndicatorLayout::Separate;
			}

			if (_menuIndicatorLayout == MenuIndicatorLayout::Separate) {
				_menuPaintRect = QRect(
					contentRect.right() - indicatorW,
					contentRect.center().y() - (indicatorH / 2),
					indicatorW,
					indicatorH);
			}
		}
		else {
			if (!hasText
				|| !(_truncateMode == TextTruncateMode::NoClip)) {
				_menuIndicatorLayout = MenuIndicatorLayout::Separate;
			}

			if (_menuIndicatorLayout == MenuIndicatorLayout::Separate) {
				_menuPaintRect = QRect(
					contentRect.bottom() - indicatorH,
					contentRect.center().x() - (indicatorW / 2),
					indicatorW,
					indicatorH);
			}
		}
	}

	// ----- Hit Rectangles -----

	_iconHitRect = QRect();
	_textHitRect = QRect();
	_menuHitRect = QRect();

	if (isHorizontal) {
		if (_iconPaintRect.x() > 0 && _iconPaintRect.y() > 0) {
			_iconHitRect = buttonRect;
			_iconHitRect.setWidth(_horizontalPadding + _iconPaintRect.width());
		}

		if (_textPaintRect.x() > 0 && _textPaintRect.y() > 0) {
			_textHitRect = buttonRect;

			int textX = showIcon ? _iconHitRect.right() : buttonRect.left();
			_textHitRect.moveLeft(textX);

			int spacing = showIcon ? _iconTextSpacing : 0;
			_textHitRect.setWidth(_textPaintRect.width() + spacing);
		}

		if (_menuPaintRect.x() > 0 && _menuPaintRect.y() > 0
			&& _menuIndicatorLayout == MenuIndicatorLayout::Separate) {

			_menuHitRect = buttonRect;
			_menuHitRect.setWidth(indicatorW + _menuIndicatorSpacing);
			_menuHitRect.moveRight(buttonRect.right());
		}
	} else {
		if (_iconPaintRect.x() > 0 && _iconPaintRect.y() > 0) {
			_iconHitRect = buttonRect;
			_iconHitRect.setHeight(_verticalPadding + _iconPaintRect.height());
		}

		if (_textPaintRect.x() > 0 && _textPaintRect.y() > 0) {
			_textHitRect = buttonRect;

			int textY = showIcon ? _iconHitRect.bottom() : buttonRect.top();
			_textHitRect.moveTop(textY);

			int spacing = showIcon ? _iconTextSpacing : 0;
			_textHitRect.setHeight(_textPaintRect.height() + spacing);
		}

		if (_menuPaintRect.x() > 0 && _menuPaintRect.y() > 0
			&& _menuIndicatorLayout == MenuIndicatorLayout::Separate) {

			_menuHitRect = buttonRect;
			_menuHitRect.setHeight(indicatorH + _menuIndicatorSpacing);
			_menuHitRect.moveBottom(buttonRect.bottom());
		}
	}
}

void IconButton::drawMenuIndicator(QPainter& p, const QRect& r, qreal thickness) {
	QPainterPath path;

	const qreal cx = r.center().x();
	const qreal cy = r.center().y();
	const qreal hw = (r.width() * 0.8 / 2);
	const qreal hh = (r.height() * 0.8 / 2);

	path.moveTo(cx - hw, cy - hh);
	path.lineTo(cx, cy + hh);
	path.lineTo(cx + hw, cy - hh);

	QPen pen(palette().color(QPalette::Text), thickness);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);

	p.save();
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(pen);
	p.setBrush(Qt::NoBrush);
	p.drawPath(path);
	p.restore();
}