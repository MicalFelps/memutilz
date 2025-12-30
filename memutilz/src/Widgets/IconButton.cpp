#include <QStyle>
#include <QRegularExpression>
#include <QStyleOptionToolButton>
#include <QPainter>

#include "Widgets/IconButton.h"
#include "Widgets/Metrics.h"

IconButton::IconButton(const QIcon& icon, const QString& text, QWidget* parent)
	: QToolButton(parent)
{
	setIcon(icon);
	setText(text);
	setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	setPopupMode(QToolButton::InstantPopup);
}

QSize IconButton::minimumSizeHint() const {
	QStyleOptionToolButton opt;
	initStyleOption(&opt);

	QSize styleBase = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(0, 0), this);

	bool hasIcon = !icon().isNull();
	int minScaledIconSize = qMax(
		Ui::IconButton::minScaledIconSize,
		Ui::IconButton::baseIconSize * _iconScalePercent / 100);
	int minIconW = (hasIcon || _reserveIconSpace) ? minScaledIconSize : 0;
	int minIconH = minIconW;

	styleBase = styleBase.expandedTo(QSize(Ui::IconButton::minSize, Ui::IconButton::minSize));

	// none case
	if (text().isEmpty() && !hasIcon && !_reserveIconSpace) { return styleBase; }

	// handle text calculations
	QFontMetrics fm{ font() };
	QString t = text();
	int minTextW = 0;
	int minTextH = 0;

	if (!t.isEmpty()) {
		bool isHorizontal = (toolButtonStyle() != Qt::ToolButtonTextUnderIcon);

		// mutually exclusive constraints on TextWrapMode and TextTruncateMode
		// based on orientation (text position)
		TextWrapMode wrapMode = _wrapMode;
		TextTruncateMode truncateMode = _truncateMode;
		if (isHorizontal && wrapMode == TextWrapMode::WrapToFit) {
			wrapMode = TextWrapMode::NoWrap; // prefer no wrap in horizontal
		} else if (!isHorizontal && truncateMode != TextTruncateMode::NoClip) {
			truncateMode = TextTruncateMode::NoClip; // prefer full wrap in vertical
		}

		// Cases
		if (wrapMode == TextWrapMode::WrapToFit) { // vertical text
			int longestPartW = 0;

			QStringList parts = t.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
			for (const QString& part : parts) {
				longestPartW = qMax(longestPartW, fm.horizontalAdvance(part));
			}

			minTextW = qMax(Ui::IconButton::minWrapWidth, longestPartW);
			QRect bound = fm.boundingRect(QRect(0, 0, minTextW, INT_MAX), 
				Qt::TextWordWrap | Qt::AlignLeft, t);
			minTextH = bound.height();
		} else { // horizontal text (NoWrap)
			minTextW = truncateMode == TextTruncateMode::NoClip ? fm.horizontalAdvance(t) : 0;
			minTextH = fm.height();
		}
	}

	// figure out total sizes
	Qt::ToolButtonStyle orientation = toolButtonStyle();
	int totalMinW = 0;
	int totalMinH = 0;
	int iconTextSpacing = (minIconW > 0 && minTextW > 0) ? _iconTextSpacing : 0;
	if (orientation == Qt::ToolButtonTextUnderIcon) { // vertical
		totalMinW = qMax(minIconW, minTextW);
		totalMinH = minIconH + iconTextSpacing + minTextH;
	} else { // horizontal
		totalMinW = minIconW + iconTextSpacing + minTextW;
		totalMinH = qMax(minIconH, minTextH);
	}

	if (menu() != nullptr) {
		int indicatorSize = style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);
		if (orientation == Qt::ToolButtonTextUnderIcon) {
			totalMinH += indicatorSize + _menuIndicatorSpacing;
		}
		else {
			totalMinW += indicatorSize + _menuIndicatorSpacing;
		}
	}

	return styleBase.expandedTo(QSize(totalMinW, totalMinH));
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

	int minIconScaled{ qMax(
		Ui::IconButton::minScaledIconSize,
		Ui::IconButton::baseIconSize * _iconScalePercent / 100) };

	QStyleOptionToolButton opt;
	opt.initFrom(this);
	opt.text = text();
	opt.icon = icon();
	opt.iconSize = QSize(minIconScaled, minIconScaled);

	if (menu() != nullptr) {
		opt.features |= QStyleOptionToolButton::MenuButtonPopup;
	}
	
	QSize contentSize(preferredW, preferredH);
	return style()->sizeFromContents(QStyle::CT_ToolButton, &opt, contentSize, this);
}

// ---------------------------------------------------------

void IconButton::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event);

	QPainter p{ this };
	p.setRenderHint(QPainter::Antialiasing);
	p.setRenderHint(QPainter::SmoothPixmapTransform);

	QStyleOptionToolButton opt;
	opt.initFrom(this);

	// add menu features
	bool hasMenu = menu() != nullptr;
	if (hasMenu) {
		opt.features |= QStyleOptionToolButton::HasMenu;
	}

	// current state
	bool hovered = opt.state & QStyle::State_MouseOver;
	bool pressed = opt.state & QStyle::State_Sunken;
	bool checked = isChecked();

	// Draw native tool button panel (background, border, etc)
	style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);

	// Get content area excluding margins added by style
	QRect buttonRect = style()->subControlRect(QStyle::CC_ToolButton, &opt,
												QStyle::SC_ToolButton, this);
	// Adjust content rect ignoring our added margins
	QRect contentRect = buttonRect.adjusted(_horizontalPadding, _verticalPadding, -_horizontalPadding, -_verticalPadding);

	// Icon calculations
	int iconSize = qMin(contentRect.width(), contentRect.height());
	int scaledIconSize = iconSize * _iconScalePercent / 100;
	bool hasIcon = !icon().isNull();
	bool showIcon = hasIcon || _reserveIconSpace;

	bool isHorizontal = (toolButtonStyle() == Qt::ToolButtonTextBesideIcon);

	QRect iconRect{ QRect() };
	if (showIcon) {
		if (isHorizontal) {
			iconRect = QRect(
				contentRect.left(),
				contentRect.top() + (contentRect.height() - scaledIconSize) / 2,
				scaledIconSize,
				scaledIconSize);
		}
		else {
			iconRect = QRect(
				contentRect.left() + (contentRect.width() - scaledIconSize) / 2,
				contentRect.top(),
				scaledIconSize,
				scaledIconSize
			);
		}
	}

	// Menu arrow calculations
	QRect arrowRect{ QRect() };
	if (hasMenu) {
		if (isHorizontal) {
			arrowRect = QRect(
				contentRect.left() + contentRect.width() - Ui::IconButton::arrowRectSize,
				contentRect.top(),
				Ui::IconButton::arrowRectSize,
				contentRect.height()
			);
		}
		else {
			arrowRect = QRect(
				contentRect.left(),
				contentRect.top() + contentRect.height() - Ui::IconButton::arrowRectSize,
				contentRect.width(),
				Ui::IconButton::arrowRectSize
			);
		}
	}

	// Text calculations
	QString t = text();
	QRect textRect{ QRect() };
	QString displayText = t;

	QFontMetrics fm{ font() };
	p.setFont(font());

	TextWrapMode wrapMode = _wrapMode;
	TextTruncateMode truncateMode = _truncateMode;
	if (isHorizontal && wrapMode == TextWrapMode::WrapToFit) {
		wrapMode = TextWrapMode::NoWrap; // prefer no wrap in horizontal
	}
	else if (!isHorizontal && truncateMode != TextTruncateMode::NoClip) {
		truncateMode = TextTruncateMode::NoClip; // prefer full wrap in vertical
	}

	if (!t.isEmpty()) {
		if (isHorizontal) {
			int textX = showIcon ? iconRect.right() + _iconTextSpacing : contentRect.left();
			int arrowRectW = hasMenu ? arrowRect.width() + _menuIndicatorSpacing : 0;
			textRect = QRect(textX, contentRect.top(), contentRect.right() - textX - arrowRectW, contentRect.height());

			if (truncateMode != TextTruncateMode::NoClip) {
				int availableW = textRect.width();
				int fullTextW = fm.horizontalAdvance(t);

				if (fullTextW > availableW) {
					if (truncateMode == TextTruncateMode::Ellipsis) {
						displayText = fm.elidedText(t, Qt::ElideRight, availableW);
					}
					else { // longest prefix that fits
						// Do binary search for efficiency
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
		}
		else { // vertical
			int textY = showIcon ? iconRect.bottom() + _iconTextSpacing : contentRect.top();
			int arrowRectH = hasMenu ? arrowRect.height() + _menuIndicatorSpacing : 0;
			textRect = QRect(contentRect.left(), textY, contentRect.width(), contentRect.height() - textY - arrowRectH);

			if (wrapMode == TextWrapMode::WrapToFit) {
				p.setPen(palette().color(QPalette::ButtonText));
				p.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap, t);
				displayText = ""; // Skip further drawing
			}
			else {
				if (fm.horizontalAdvance(t) > textRect.width()) {
					displayText = fm.elidedText(t, Qt::ElideRight, textRect.width());
				}
			}
		}
	}

	// Draw Icon
	if (hasIcon && scaledIconSize > Ui::IconButton::minScaledIconSize) {
		QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
		if (pressed || checked) mode = QIcon::Selected;
		if (hovered) mode = QIcon::Active;

		icon().paint(&p, iconRect, Qt::AlignCenter, mode);
	}

	// Draw text
	if (!displayText.isEmpty()) {
		p.setPen(palette().color(QPalette::ButtonText));
		p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, displayText);
	}

	// Draw checked bar
	if (checked && _checkedBarColor.isValid()) {
		const int barWidth = 4;
		qreal barRadius = barWidth / 2.0;
		QRectF barRect{ QRectF() };
		int barLength = scaledIconSize / 2;
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
	
	// Draw menu arrow
	if (hasMenu) {
		opt.rect = arrowRect;
		style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, &p,this);
	}
}