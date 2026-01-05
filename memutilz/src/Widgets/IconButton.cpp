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
	setAttribute(Qt::WA_Hover, true);

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
	const int menuIndicatorW = qRound(fm.height() * Ui::IconButton::indicatorWidthFactor);
	const int menuIndicatorH = qRound(fm.height() * Ui::IconButton::indicatorHeightFactor);
	const int menuIndicatorSpacing = menuIndicatorW;

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
		QTextLayout layout{ text(), font() };
		QTextOption opt;

		opt.setAlignment(Qt::AlignLeft);
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

		if (hasMenu && (menuIndicatorLayout == MenuIndicatorLayout::Compact)) {
			minTextW = qMax(minTextW, qCeil(lastLineW) + menuIndicatorSpacing + menuIndicatorW);
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
			totalMinW += _menuIndicatorSpacing + menuIndicatorW;
		}
		else {
			totalMinH += _menuIndicatorSpacing + menuIndicatorH;
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
	bool hasIcon = !icon().isNull();
	bool hasMenu = menu() != nullptr;
	if (hasMenu) {
		opt.features |= QStyleOptionToolButton::HasMenu;
	}
	bool hasText = !text().isEmpty();

	bool isHorizontal = (toolButtonStyle() == Qt::ToolButtonTextBesideIcon);

	QFontMetrics fm{ font() };
	const int menuIndicatorW = qRound(fm.height() * Ui::IconButton::indicatorWidthFactor);
	const int menuIndicatorH = qRound(fm.height() * Ui::IconButton::indicatorHeightFactor);
	const int menuIndicatorSpacing = menuIndicatorW;

	// Draw base panel (background, hover, border, etc)
	style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);

	// ---------------------------------------------------------

	p.fillRect(_iconPaintRect, QColor("#ff0000"));
	p.fillRect(_textPaintRect, QColor("#00ff00"));
	p.fillRect(_menuPaintRect, QColor("#0000ff"));

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

	/*

		while (true) {


			if (lineBottom > maxH) break;
			y = lineBottom;
		}

		layout.endLayout();

		QRectF bounds = layout.boundingRect();
		int textW = qCeil(bounds.width());
		int textH = qCeil(bounds.height());

		if (!isHorizontal && hasMenu
			&& menuIndicatorLayout == MenuIndicatorLayout::Compact) {
			QTextLine lastLine = layout.lineAt(layout.lineCount() - 1);
			qreal lastLineW = lastLine.naturalTextWidth() + menuIndicatorSpacing + menuIndicatorW;

			if (lastLineW <= maxW) {
				_menuPaintRect = QRect(
					_textPaintRect.left() + lastLineW - menuIndicatorW,
					_textPaintRect.top() + y - (lastLine.height() / 2),
					menuIndicatorW,
					menuIndicatorH
				);
			}

			lastLineW = qMax(lastLineW, maxW);
			textW = qMax(qCeil(lastLineW), textW);
		}

		_textPaintRect = QRect(0, 0, textW, textH);
		_textPaintRect.moveCenter(contentRect.center());

		if (isHorizontal) {
			_textPaintRect.moveLeft(_iconPaintRect.right() + (hasIcon ? _iconTextSpacing : 0));
		}
		else {
			_textPaintRect.moveTop(_iconPaintRect.bottom() + (hasIcon ? _iconTextSpacing : 0));
		}
	*/

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
		y = lineBottom;

		line.setPosition(QPointF(0, y));
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

				qreal lastLineW = line.naturalTextWidth();
				lastLineW += menuIndicatorW + menuIndicatorSpacing;

				if (lastLineW <= _textPaintRect.width()) {
					pos.rx() = (_textPaintRect.width() - lastLineW) / 2;
				}

				line.setPosition(pos);
			}
			else {
				pos.setX(_textPaintRect.x() + (_textPaintRect.width() - lineW) / 2);
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
bool IconButton::event(QEvent* event) {
	return QToolButton::event(event);
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
	QRect contentRect = buttonRect.adjusted(_padding.left, _padding.top, -_padding.right, -_padding.bottom);

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
	const int menuIndicatorW = qRound(fm.height() * Ui::IconButton::indicatorWidthFactor);
	const int menuIndicatorH = qRound(fm.height() * Ui::IconButton::indicatorHeightFactor);
	const int menuIndicatorSpacing = menuIndicatorW;

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
				_iconPaintRect = QRect(
					contentRect.left(),
					contentRect.top() + (contentRect.height() - scaledIconSize) / 2,
					scaledIconSize,
					scaledIconSize);
			}
			else {
				_iconPaintRect = QRect(
					contentRect.left() + (contentRect.width() - scaledIconSize) / 2,
					contentRect.top(),
					scaledIconSize,
					scaledIconSize
				);
			}
		}

		// --- IconHitRect ---

		if (scaledIconSize > 0) {
			_iconHitRect = buttonRect;

			if (isHorizontal) {
				_iconHitRect.adjust(0, 0, _padding.left + qMin(scaledIconSize, contentRect.width()), 0);

				int remainderX = scaledIconSize + (hasText ? _iconTextSpacing : 0);
				contentRect.adjust(remainderX, 0, 0, 0);
				buttonRect.adjust(_padding.left + scaledIconSize, 0, 0, 0);
				if (!buttonRect.isValid() || !contentRect.isValid()) return;
			}
			else {
				_iconHitRect.adjust(0, 0, 0, _padding.top + qMin(scaledIconSize, contentRect.height()));
				int remainderY = scaledIconSize + (hasText ? _iconTextSpacing : 0);
				contentRect.adjust(0, remainderY, 0, 0);
				buttonRect.adjust(0, _padding.top + scaledIconSize, 0, 0);
				if (!buttonRect.isValid() || !contentRect.isValid()) return;
			}
		}
	}

	if (hasMenu && menuIndicatorLayout == MenuIndicatorLayout::Separate) {

		if (contentRect.width() >= menuIndicatorW
			&& contentRect.height() >= menuIndicatorH) {

			if (isHorizontal) {
				_menuPaintRect = QRect(
					contentRect.right() - menuIndicatorW,
					contentRect.top() + (contentRect.height() - menuIndicatorH) / 2,
					menuIndicatorW,
					menuIndicatorH
				);
			}
			else {
				_menuPaintRect = QRect(
					contentRect.left() + (contentRect.width() - menuIndicatorW) / 2,
					contentRect.bottom() - menuIndicatorH,
					menuIndicatorW,
					menuIndicatorH
				);
			}
		}

		if ((menuIndicatorW > 0 || menuIndicatorH > 0)
			&& !contentRect.isNull()) {
			_menuHitRect = buttonRect;

			if (isHorizontal) {
				_menuHitRect.adjust(
					buttonRect.width() - (_padding.right + qMin(menuIndicatorW + _menuIndicatorSpacing, contentRect.width())), 0, 0, 0
				);
				contentRect.adjust(0, 0, -qMin(_menuIndicatorSpacing + menuIndicatorW, contentRect.width()), 0);
				buttonRect.adjust(0, 0, -qMin(_menuIndicatorSpacing + menuIndicatorW + _padding.right, buttonRect.width()), 0);
				if (!buttonRect.isValid() || !contentRect.isValid() || buttonRect.isNull() || contentRect.isNull()) return;
			}
			else {
				_menuHitRect.adjust(
					0, buttonRect.height() - (_padding.bottom + qMin(menuIndicatorH + _menuIndicatorSpacing, contentRect.height())), 0, 0
				);
				contentRect.adjust(0, 0, 0, -qMin(_menuIndicatorSpacing + menuIndicatorH, contentRect.height()));
				buttonRect.adjust(0, 0, 0, -qMin(_menuIndicatorSpacing + menuIndicatorH + _padding.bottom, buttonRect.height()));
				if (!buttonRect.isValid() || !contentRect.isValid()) return;
			}
		}
	}

	if (hasText) {
		if(!buttonRect.isNull()) _textHitRect = buttonRect; // we give the rest to text

		QTextLayout layout{ text(), font() };
		QTextOption opt;
		opt.setAlignment(Qt::AlignLeft); // we do our own alignment
		opt.setWrapMode(wrapMode == TextWrapMode::WrapToFit ? QTextOption::WordWrap : QTextOption::NoWrap);
		layout.setTextOption(opt);

		qreal maxW = contentRect.width();
		qreal maxH = contentRect.height();

		layout.beginLayout();
		qreal y = 0;

		while (true) {
			QTextLine line = layout.createLine();
			if (!line.isValid()) {
				break;
			}

			line.setLineWidth(maxW);

			qreal lineBottom = y + line.height();
			if (lineBottom > maxH) break;
			y = lineBottom;
		}

		layout.endLayout();

		QRectF bounds = layout.boundingRect();
		int textW = qCeil(bounds.width());
		int textH = qCeil(bounds.height());

		if (!isHorizontal && hasMenu
			&& menuIndicatorLayout == MenuIndicatorLayout::Compact) {
			QTextLine lastLine = layout.lineAt(layout.lineCount() - 1);
			qreal lastLineW = lastLine.naturalTextWidth() + menuIndicatorSpacing + menuIndicatorW;

			if (lastLineW <= maxW) {
				_menuPaintRect = QRect(
					_textPaintRect.left() + lastLineW - menuIndicatorW,
					_textPaintRect.top() + y - (lastLine.height() / 2),
					menuIndicatorW,
					menuIndicatorH
				);
			}

			lastLineW = qMax(lastLineW, maxW);
			textW = qMax(qCeil(lastLineW), textW);
		}

		_textPaintRect = QRect(0, 0, textW, textH);
		_textPaintRect.moveCenter(contentRect.center());

		if (isHorizontal) {
			_textPaintRect.moveLeft(_iconPaintRect.right() + (hasIcon ? _iconTextSpacing : 0));
		}
		else {
			_textPaintRect.moveTop(_iconPaintRect.bottom() + (hasIcon ? _iconTextSpacing : 0));
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