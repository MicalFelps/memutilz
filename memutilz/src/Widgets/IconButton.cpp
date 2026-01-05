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
	const int spacing = indicatorW;
	const int extraIndicatorW = indicatorW + spacing;

	// constraints
	TextWrapMode wrapMode = _wrapMode;
	TextTruncateMode truncateMode = _truncateMode;
	MenuIndicatorLayout menuIndicatorLayout = _menuIndicatorLayout;

	if (isHorizontal) {
		if (wrapMode == TextWrapMode::WrapToFit) wrapMode = TextWrapMode::NoWrap; // prefer no wrap in horizontal
		if (hasMenu && _menuIndicatorLayout == MenuIndicatorLayout::Compact) {
			truncateMode = TextTruncateMode::NoClip;
		}
	}
	else {
		if (truncateMode != TextTruncateMode::NoClip) {
			truncateMode = TextTruncateMode::NoClip; // prefer full wrap in vertical
		}
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

			opt.setAlignment(isHorizontal ? Qt::AlignLeft : Qt::AlignHCenter);
			opt.setWrapMode(
				wrapMode == TextWrapMode::WrapToFit
				? QTextOption::WordWrap
				: QTextOption::NoWrap);

			layout.setTextOption(opt);


			layout.beginLayout();

			qreal requiredW = 0;
			qreal requiredH = 0;
			qreal lastLineW = 0;
			const qreal fixedLineW =
				(wrapMode == TextWrapMode::WrapToFit)
				? Ui::IconButton::minWrapWidth
				: std::numeric_limits<qreal>::max();

			while (true) {
				QTextLine line = layout.createLine();
				if (!line.isValid()) {
					break;
				}

				line.setLineWidth(fixedLineW);
				lastLineW = line.naturalTextWidth();
				requiredW = qMax(requiredW, lastLineW);
				requiredH += line.height();
			}

			layout.endLayout();

			minTextW = qCeil(requiredW);
			minTextH = qCeil(requiredH);

			if (wrapMode == TextWrapMode::WrapToFit) {
				minTextW = qMax(Ui::IconButton::minWrapWidth, minTextW);
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
	if (hasMenu && (menuIndicatorLayout == MenuIndicatorLayout::Separate)) {
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
	return minimumSizeHint();
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
	bool hasText = !text().isEmpty();

	bool isHorizontal = (toolButtonStyle() == Qt::ToolButtonTextBesideIcon);

	QFontMetrics fm{ font() };
	const int spacing = qRound(fm.height() * Ui::IconButton::indicatorWidthFactor);
	const int extraIndicatorW = _menuPaintRect.width() + spacing;

	// Draw base panel (background, hover, border, etc)
	style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);

	// ---------------------------------------------------------

	TextWrapMode wrapMode = _wrapMode;
	TextTruncateMode truncateMode = _truncateMode;
	MenuIndicatorLayout menuIndicatorLayout = _menuIndicatorLayout;

	if (isHorizontal) {
		if (wrapMode == TextWrapMode::WrapToFit) wrapMode = TextWrapMode::NoWrap; // prefer no wrap in horizontal
		if (hasMenu && _menuIndicatorLayout == MenuIndicatorLayout::Compact) {
			truncateMode = TextTruncateMode::NoClip;
		}
	}
	else {
		if (truncateMode != TextTruncateMode::NoClip) {
			truncateMode = TextTruncateMode::NoClip; // prefer full wrap in vertical
		}
	}

	QTextLayout layout{ text(), font()};
	QTextOption textOpt;
	textOpt.setAlignment(Qt::AlignLeft);
	textOpt.setWrapMode(wrapMode == TextWrapMode::WrapToFit ? QTextOption::WordWrap : QTextOption::NoWrap);
	layout.setTextOption(textOpt);

	layout.beginLayout();
	qreal y = 0;

	while (true) {
		QTextLine line = layout.createLine();
		if (!line.isValid()) {
			break;
		}

		line.setLineWidth(_textPaintRect.width());

		qreal lineBottom = y + line.height();
		if (lineBottom > _textPaintRect.height()) break;

		line.setPosition(QPointF(0, y));
		y = lineBottom;
	}

	layout.endLayout();

	if (!isHorizontal) {
		for (int i = 0; i < layout.lineCount(); ++i) {
			QTextLine line = layout.lineAt(i);

			qreal lineW = line.naturalTextWidth();
			QPointF pos = line.position();

			if (hasMenu
				&& menuIndicatorLayout == MenuIndicatorLayout::Compact
				&& i == (layout.lineCount() - 1)) {

				int lastLineW = line.naturalTextWidth();
				lastLineW += extraIndicatorW;

				if (lastLineW <= _textPaintRect.width()) {
					pos.rx() = (_textPaintRect.width() - lastLineW) / 2;
				}

				line.setPosition(pos);
			}
			else {
				pos.setX((_textPaintRect.width() - lineW) / 2);
			}
		}
	}

	// ---------------------------------------------------------

	if (!isEnabled()) {
		p.setOpacity(0.35);
	}

	if (hasIcon
		&& _iconPaintRect.size().width() > Ui::IconButton::minScaledIconSize
		&& _iconPaintRect.size().height() > Ui::IconButton::minScaledIconSize) {

		icon().paint(&p, _iconPaintRect, Qt::AlignCenter);
	}

	if (hasText
		&& _textPaintRect.size().width() > 0
		&& _textPaintRect.size().height() > 0) {

		p.setPen(palette().color(QPalette::ButtonText));
		layout.draw(&p, QPointF(_textPaintRect.left(), _textPaintRect.top()));
	}

	if (hasMenu
		&& _menuPaintRect.size().width() > 0
		&& _menuPaintRect.size().height() > 0) {

		drawMenuIndicator(p, _menuPaintRect, 2);
	}

	// Draw checked bar
	if (isChecked() && _checkedBarColor.isValid()) {
		const int barWidth = 4;
		qreal barRadius = barWidth / 2.0;
		QRectF barRect{ QRectF() };
		int barLength = _iconPaintRect.size().width();
		if (isHorizontal) {
			barRect = QRectF(0, rect().top() + (rect().height() - barLength) / 2, barWidth, barLength);
		}
		else {
			barRect = QRectF(rect().left() + (rect().width() - barLength) / 2, 0, barLength, barWidth);
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
	QFontMetricsF fm{ font() };
	QTextLayout layout{ text(), font() };

	// Menu
	const int indicatorW = qRound(fm.height() * Ui::IconButton::indicatorWidthFactor);
	const int indicatorH = qRound(fm.height() * Ui::IconButton::indicatorHeightFactor);
	const int spacing = indicatorW;
	const int extraIndicatorW = indicatorW + spacing;

	// constraints
	TextWrapMode wrapMode = _wrapMode;
	TextTruncateMode truncateMode = _truncateMode;
	MenuIndicatorLayout menuIndicatorLayout = _menuIndicatorLayout;

	if (isHorizontal) {
		if (wrapMode == TextWrapMode::WrapToFit) wrapMode = TextWrapMode::NoWrap; // prefer no wrap in horizontal
		if (hasMenu && _menuIndicatorLayout == MenuIndicatorLayout::Compact) {
			truncateMode = TextTruncateMode::NoClip;
		}
	}
	else {
		if (truncateMode != TextTruncateMode::NoClip) {
			truncateMode = TextTruncateMode::NoClip; // prefer full wrap in vertical
		}
	}

	// ----- Paint Rectangles -----

	_iconPaintRect = QRect();
	_textPaintRect = QRect();
	_menuPaintRect = QRect();

	_iconHitRect = QRect();
	_textHitRect = QRect();
	_menuHitRect = QRect();

	if (showIcon) {

		// --- IconPaintRect ---

		if (scaledIconSize >= Ui::IconButton::minScaledIconSize
			&& contentRect.width() >= scaledIconSize
			&& contentRect.height() >= scaledIconSize) {

			if (isHorizontal) {
				if (contentRect.height() >= scaledIconSize) {
					_iconPaintRect = QRect(
						contentRect.left() + (contentRect.height() - scaledIconSize) / 2,
						contentRect.top() + (contentRect.height() - scaledIconSize) / 2,
						scaledIconSize,
						scaledIconSize);
				}
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

		// --- IconHitRect ---

		if (scaledIconSize > 0) {
			_iconHitRect = buttonRect;
			if (isHorizontal) {
				_iconHitRect.setWidth(_horizontalPadding + qMin(scaledIconSize, contentRect.width()));
				int textX = scaledIconSize + (hasText ? _iconTextSpacing : 0);
				buttonRect.adjust(textX, 0, 0, 0);
				if (!buttonRect.isValid()) return;
			}
			else {
				_iconHitRect.setHeight(_verticalPadding + qMin(scaledIconSize, contentRect.height()));
				int textY = scaledIconSize + (hasText ? _iconTextSpacing : 0);
				buttonRect.adjust(0, textY, 0, 0);
				if (!buttonRect.isValid()) return;
			}
		}
	}

	if (hasMenu && menuIndicatorLayout == MenuIndicatorLayout::Separate) {
		_menuHitRect = buttonRect;

		if (isHorizontal) {
			if (contentRect.width() < _menuIndicatorSpacing + indicatorW
				&& contentRect.height() < indicatorH) {
				_menuHitRect = buttonRect;
				return;
			}

			_menuPaintRect = QRect(
				contentRect.right() - indicatorW,
				(contentRect.height() - indicatorH) / 2,
				indicatorW,
				indicatorH
			);

			_menuHitRect.adjust(_menuHitRect.width() - (_horizontalPadding + indicatorW + _menuIndicatorSpacing), 0, 0, 0);
			int menuX = buttonRect.right() - _menuHitRect.x();
			buttonRect.adjust(0, 0, -menuX, 0);
			if (!buttonRect.isValid()) return;
		}
		else { // vertical
			if (contentRect.width() < indicatorW
				&& contentRect.height() < _menuIndicatorSpacing + indicatorH) {
				_menuHitRect = buttonRect;
				return;
			}

			_menuPaintRect = QRect(
				(contentRect.right() - indicatorW) / 2,
				contentRect.bottom() - indicatorH,
				indicatorW,
				indicatorH
			);

			_menuHitRect.adjust(0, _menuHitRect.height() - (_verticalPadding + indicatorH + _menuIndicatorSpacing), 0, 0);
			int menuY = buttonRect.bottom() - _menuHitRect.y();
			buttonRect.adjust(0, 0, 0, -menuY);
			if (!buttonRect.isValid()) return;
		}
	}

	if (hasText) {
		_textHitRect = buttonRect;

		QTextLayout layout{ text(), font() };
		QTextOption opt;
		opt.setAlignment(Qt::AlignLeft);
		opt.setWrapMode(wrapMode == TextWrapMode::WrapToFit ? QTextOption::WordWrap : QTextOption::NoWrap);
		layout.setTextOption(opt);

		qreal maxWidth = contentRect.width();
		qreal maxHeight = contentRect.height();

		layout.beginLayout();
		qreal y = 0;

		while (true) {
			QTextLine line = layout.createLine();
			if (!line.isValid()) {
				break;
			}

			line.setLineWidth(maxWidth);

			qreal lineBottom = y + line.height();
			if (lineBottom > maxHeight) break;

			line.setPosition(QPointF(0, y));
			y = lineBottom;
		}

		layout.endLayout();

		QRectF bounds = layout.boundingRect();
		int textW = qCeil(bounds.width());
		int textH = qCeil(bounds.height());

		_textPaintRect = QRect(0, 0, textW, textH);
		_textPaintRect.moveCenter(contentRect.center());

		if (isHorizontal) {
			_textPaintRect.moveLeft(_iconPaintRect.right() + _iconTextSpacing);
		}
		else {
			_textPaintRect.moveTop(_iconPaintRect.bottom() + _iconTextSpacing);
		}

		if (!isHorizontal) {
			for (int i = 0; i < layout.lineCount(); ++i) {
				QTextLine line = layout.lineAt(i);

				qreal lineW = line.naturalTextWidth();
				QPointF pos = line.position();

				if (hasMenu
					&& menuIndicatorLayout == MenuIndicatorLayout::Compact
					&& i == (layout.lineCount() - 1)) {

					int lastLineW = line.naturalTextWidth();
					lastLineW += extraIndicatorW;

					if (lastLineW <= _textPaintRect.width()) {
						pos.rx() = (_textPaintRect.width() - lastLineW) / 2;
					}

					line.setPosition(pos);

					_menuPaintRect = QRect(
						_textPaintRect.left() + line.x() + lineW + spacing,
						_textPaintRect.top() + y - (line.height() / 2),
						indicatorW,
						indicatorH);
				}
				else {
					pos.setX((_textPaintRect.width() - lineW) / 2);
				}
			}
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