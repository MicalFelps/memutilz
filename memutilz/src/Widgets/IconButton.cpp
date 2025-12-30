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
}

QSize IconButton::minimumSizeHint() const {
	int styleMargin = style()->pixelMetric(QStyle::PM_ButtonMargin, nullptr, this);
	int horizontalMargin = _horizontalMargin + styleMargin;
	int verticalMargin = _verticalMargin + styleMargin;

	int minIconScaled{ qMax(
		Ui::IconButton::minScaledIconSize,
		Ui::IconButton::baseIconSize * _iconScalePercent / 100) };
	bool hasIcon = !icon().isNull();
	int minIconW = (hasIcon || _reserveIconSpace) ? minIconScaled : 0;
	int minIconH = (hasIcon || _reserveIconSpace) ? minIconScaled : 0;

	int minW = Ui::IconButton::minSize + 2 * horizontalMargin;
	int minH = Ui::IconButton::minSize + 2 * verticalMargin;

	// none case
	if (text().isEmpty() && !hasIcon && !_reserveIconSpace) {
		return QSize(minW, minH);
	}

	// handle text calculations
	QFontMetrics fm{ font() };
	QString t = text();
	int minTextW = 0;
	int minTextH = 0;

	if (!t.isEmpty()) {
		int fullTextW = fm.horizontalAdvance(t);
		int longestPartW = 0;
		int ellipsisW = 0;

		QStringList parts = t.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
		for (const QString& part : parts) {
			longestPartW = qMax(longestPartW, fm.horizontalAdvance(part));
		}

		if (_truncateMode == TextTruncateMode::Ellipsis) {
			ellipsisW = fm.horizontalAdvance("...");
		}

		Qt::ToolButtonStyle orientation = toolButtonStyle();
		bool isHorizontal = (orientation != Qt::ToolButtonTextUnderIcon);

		// mutually exclusive constraints on TextWrapMode and TextTruncateMode
		// based on orientation / text position
		TextWrapMode wrapMode = _wrapMode;
		TextTruncateMode truncateMode = _truncateMode;
		if (isHorizontal && wrapMode == TextWrapMode::WrapToFit) {
			wrapMode = TextWrapMode::NoWrap; // prefer no wrap in horizontal
		} else if (!isHorizontal && truncateMode != TextTruncateMode::NoClip) {
			truncateMode = TextTruncateMode::NoClip; // prefer full wrap in vertical
		}

		if (wrapMode == TextWrapMode::WrapToFit) { // vertical text
			minTextW = qMax(Ui::IconButton::minWrapWidth, longestPartW + ellipsisW);
			QRect bound = fm.boundingRect(QRect(0, 0, minTextW, INT_MAX), 
				Qt::TextWordWrap | Qt::AlignLeft, t);
			minTextH = bound.height();
		} else { // NoWrap
			if (truncateMode == TextTruncateMode::NoClip) {
				minTextW = fullTextW;
			} else {
				minTextW = ellipsisW; // 0 if not ellipsis, which is what we want
			}
			minTextH = fm.height();
		}
	}

	// figure out total sizes
	Qt::ToolButtonStyle orientation = toolButtonStyle();
	int totalMinW = 0;
	int totalMinH = 0;
	int iconTextSpacing = (minTextW > 0 && minIconW > 0) ? _iconTextSpacing : 0;
	if (orientation == Qt::ToolButtonTextUnderIcon) { // vertical
		totalMinW = qMax(minIconW, minTextW) + 2 * horizontalMargin;
		totalMinH = minIconH + iconTextSpacing + minTextH + 2 * verticalMargin;
	} else { // horizontal
		totalMinW = minIconW + iconTextSpacing + minTextW + 2 * horizontalMargin;
		totalMinH = qMax(minIconH, minTextH) + 2 * verticalMargin;
	}

	if (menu() != nullptr) {
		QStyleOptionToolButton opt;
		opt.initFrom(this);
		int indicatorSize = style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);
		if (orientation == Qt::ToolButtonTextUnderIcon) {
			totalMinH += indicatorSize + _menuIndicatorSpacing;
		}
		else {
			totalMinW += indicatorSize + _menuIndicatorSpacing;
		}
	}

	// force a minimum
	return QSize(qMax(totalMinW, minW), qMax(totalMinH, minH));
}
QSize IconButton::sizeHint() const {
	QSize minimum = minimumSizeHint();
	int preferredW = minimum.width();
	int preferredH = minimum.height();

	if (!text().isEmpty()) {
		QFontMetrics fm{ font() };
		int fullTextW = fm.horizontalAdvance(text());
		
		if (_truncateMode == TextTruncateMode::Ellipsis) {
			fullTextW -= fm.horizontalAdvance("...");
		}

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
	opt.iconSize = QSize(minIconScaled, minIconScaled);

	if (menu() != nullptr) {
		opt.features |= QStyleOptionToolButton::MenuButtonPopup;
	}
	
	QSize contentSize(preferredW, preferredH);
	QSize polished = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, contentSize, this);
	return polished.expandedTo(minimumSizeHint());
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
	if (menu() != nullptr) {
		opt.features |= QStyleOptionToolButton::MenuButtonPopup;
	}

	// current state
	bool hovered = opt.state & QStyle::State_MouseOver;
	bool pressed = opt.state & QStyle::State_Sunken;
	bool checked = isChecked();

	// Draw native tool button panel (background, border, etc)
	style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);

	// Get content area excluding margins added by style
	QRect contentRect = style()->subControlRect(QStyle::CC_ToolButton, &opt,
												QStyle::SC_ToolButton, this);

	// Adjust content rect ignoring our added margins
	int styleMargin = style()->pixelMetric(QStyle::PM_ButtonMargin, nullptr, this);
	int horizontalMargin = _horizontalMargin + styleMargin;
	int verticalMargin = _verticalMargin + styleMargin;
	contentRect.adjust(horizontalMargin, verticalMargin, -horizontalMargin, -verticalMargin);

	// Icon calculations
	int iconSize = qMax(Ui::IconButton::minScaledIconSize, Ui::IconButton::baseIconSize * _iconScalePercent / 100);
	bool hasIcon = !icon().isNull();
	bool showIcon = hasIcon || _reserveIconSpace;
	bool isHorizontal = (toolButtonStyle() == Qt::ToolButtonTextBesideIcon);

	QRect iconRect{ QRect() };
	if (showIcon) {
		iconRect = QRect(contentRect.left(), contentRect.top(), iconSize, iconSize);
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
			textRect = QRect(textX, contentRect.top(), contentRect.right() - textX, contentRect.height());

			if (truncateMode != TextTruncateMode::NoClip) {
				int availableW = textRect.width();
				int fullTextW = fm.horizontalAdvance(t);

				if (fullTextW > availableW) {
					if (truncateMode == TextTruncateMode::Ellipsis) {
						displayText = fm.elidedText(t, Qt::ElideRight, availableW);
					}
					else if (truncateMode == TextTruncateMode::HideIfClip) {
						displayText = "";
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
			textRect = QRect(contentRect.left(), textY, contentRect.width(), contentRect.height() - textY);

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
	if (hasIcon) {
		QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
		if (hovered) mode = QIcon::Active;
		if (pressed || checked) mode = QIcon::Selected;

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
		if (isHorizontal) {
			barRect = QRectF(0, contentRect.height() - (iconSize / 2), barWidth, iconSize);
		}
		else {
			barRect = QRectF(contentRect.width() - (iconSize / 2), 0, iconSize, barWidth);
		}
		p.setPen(Qt::NoPen);
		p.setBrush(_checkedBarColor);
		p.drawRoundedRect(barRect, barRadius, barRadius);
	}

	// Draw menu arrow
	if (menu() != nullptr) {
		style()->drawPrimitive(QStyle::PE_IndicatorButtonDropDown, &opt, &p, this);
	}
}