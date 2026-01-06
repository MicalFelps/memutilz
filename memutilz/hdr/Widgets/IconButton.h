#ifndef WIDGETS_ICONBUTTON_H
#define WIDGETS_ICONBUTTON_H

#include <QToolButton>
#include <QIcon>
#include <QColor>
#include <QHoverEvent>

class IconButton : public QToolButton {
	Q_OBJECT

public:
	enum class TextTruncateMode { NoClip, Clip, Ellipsis }; // just changes visually
	enum class TextWrapMode { NoWrap, WrapToFit };
	enum class MenuIndicatorLayout { Compact, Separate };
	enum class HoverRegion {
		Icon,
		Text,
		Menu,
		None
	};

	struct Padding {
		int left;
		int top;
		int right;
		int bottom;

		bool operator==(const Padding & other) const noexcept {
			return left == other.left
				&& top == other.top
				&& right == other.right
				&& bottom == other.bottom;
		}

		bool operator!=(const Padding& other) const noexcept {
			return !(*this == other);
		}
	};

	Q_ENUM(TextTruncateMode)
	Q_ENUM(TextWrapMode)
	Q_ENUM(MenuIndicatorLayout)
	Q_ENUM(HoverRegion)

	Q_PROPERTY(TextTruncateMode		textTruncateMode		READ textTruncateMode		WRITE setTextTruncateMode)
	Q_PROPERTY(TextWrapMode			textWrapMode			READ textWrapMode			WRITE setTextWrapMode)
	Q_PROPERTY(MenuIndicatorLayout	menuIndicatorLayout		READ menuIndicatorLayout	WRITE setMenuIndicatorLayout)
	Q_PROPERTY(Padding				padding					READ padding				WRITE setPadding)
	Q_PROPERTY(int					iconTextSpacing			READ iconTextSpacing		WRITE setIconTextSpacing)
	Q_PROPERTY(int					menuIndicatorSpacing	READ menuIndicatorSpacing	WRITE setMenuIndicatorSpacing)
	Q_PROPERTY(int					iconScalePercent		READ iconScalePercent		WRITE setIconScalePercent)
	Q_PROPERTY(bool					reserveIconSpace		READ reserveIconSpace		WRITE setReserveIconSpace)
	Q_PROPERTY(QColor				checkedBarColor			READ checkedBarColor		WRITE setCheckedBarColor)

	explicit IconButton(const QIcon& icon = QIcon(), const QString& text = QString(), QWidget* parent = nullptr);
	virtual ~IconButton() = default;

	TextTruncateMode textTruncateMode() const { return _truncateMode; }
	void setTextTruncateMode(TextTruncateMode mode) {
		if (_truncateMode != mode) {
			_truncateMode = mode;
			update();
		}
	}

	TextWrapMode textWrapMode() const { return _wrapMode; }
	void setTextWrapMode(TextWrapMode mode) {
		if (_wrapMode != mode) {
			_wrapMode = mode;
			updateGeometry();
			updateRectLayout();
			update();
		}
	}

	MenuIndicatorLayout menuIndicatorLayout() const { return _menuIndicatorLayout; }
	void setMenuIndicatorLayout(MenuIndicatorLayout layout) {
		if (_menuIndicatorLayout != layout) {
			_menuIndicatorLayout = layout;
			updateGeometry();
			updateRectLayout();
			update();
		}
	}

	Padding padding() const { return _padding; }
	void setPadding(Padding padding) {
		if (_padding != padding) {
			_padding = padding;
			updateRectLayout();
			update();
		}
	}

	int iconTextSpacing() const { return _iconTextSpacing; }
	void setIconTextSpacing(int iconTextSpacing) {
		if (_iconTextSpacing != iconTextSpacing) {
			_iconTextSpacing = qMax(0, iconTextSpacing);
			updateGeometry();
			updateRectLayout();
			update();
		}
	}

	int menuIndicatorSpacing() const { return _menuIndicatorSpacing; }
	void setMenuIndicatorSpacing(int spacing) {
		if (_menuIndicatorSpacing != spacing) {
			_menuIndicatorSpacing = spacing;
			updateGeometry();
			updateRectLayout();
			update();
		}
	}

	int iconScalePercent() const { return _iconScalePercent; }
	void setIconScalePercent(int percent) {
		if (_iconScalePercent != percent) {
			_iconScalePercent = qBound(0, percent, 100);
			updateGeometry();
			updateRectLayout();
			update();
		}
	}

	bool reserveIconSpace() const { return _reserveIconSpace; }
	void setReserveIconSpace(bool reserveIconSpace) {
		if (_reserveIconSpace != reserveIconSpace) {
			_reserveIconSpace = reserveIconSpace;
			updateGeometry();
			updateRectLayout();
			update();
		}
	}

	QColor checkedBarColor() const { return _checkedBarColor; }
	void setCheckedBarColor(const QColor& color) {
		if (_checkedBarColor != color) {
			_checkedBarColor = color;
			update();  // Trigger repaint
		}
	}

	QRect iconPaintRect() const { return _iconPaintRect; }
	QRect textPaintRect() const { return _textPaintRect; }
	QRect menuPaintRect() const { return _menuPaintRect; }

	// ------------------------------------------------------


	void setMenu(QMenu* menu) {
		QToolButton::setMenu(menu);

		updateGeometry();
		updateRectLayout();
		update();
	}

	virtual QSize minimumSizeHint() const override;
	virtual QSize sizeHint() const override;
signals:
	void hoverRegionChanged(HoverRegion region);
protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual bool event(QEvent* event) override;
	virtual bool hitButton(const QPoint& pos) const override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
private:
	void updateRectLayout();
	void drawMenuIndicator(QPainter& p, const QRect& r, qreal thickness);

	TextTruncateMode _truncateMode	{ TextTruncateMode::Clip };
	TextWrapMode _wrapMode			{ TextWrapMode::WrapToFit };
	Padding _padding				{ 0, 0, 0, 0 };
	int _iconTextSpacing			{ 0 };
	int _menuIndicatorSpacing		{ 0 };
	int _iconScalePercent			{ 50 };
	bool _reserveIconSpace			{ true };
	QColor _checkedBarColor			{QColor("#ffffff")};

	MenuIndicatorLayout _menuIndicatorLayout{ MenuIndicatorLayout::Compact };

	QRect _iconPaintRect{ QRect() };
	QRect _textPaintRect{ QRect() };
	QRect _menuPaintRect{ QRect() };

	QRect _iconHitRect{ QRect() };
	QRect _textHitRect{ QRect() };
	QRect _menuHitRect{ QRect() };

	HoverRegion _hoverRegion{ HoverRegion::None };
	
	void setHoverRegion(HoverRegion region) {
		if (_hoverRegion != region) {
			_hoverRegion = region;
			emit hoverRegionChanged(_hoverRegion);
		}
	}
};

Q_DECLARE_METATYPE(IconButton::Padding)

#endif