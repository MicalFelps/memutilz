#ifndef WIDGETS_ICONBUTTON_H
#define WIDGETS_ICONBUTTON_H

#include <QToolButton>
#include <QIcon>
#include <QColor>

#include <QTextLayout>

class IconButton : public QToolButton {
	Q_OBJECT

public:
	enum class TextTruncateMode { Clip, NoClip, Ellipsis };
	enum class TextWrapMode { NoWrap, WrapToFit };
	enum class MenuIndicatorLayout { Compact, Separate };

	Q_ENUM(TextTruncateMode)
	Q_ENUM(TextWrapMode)
	Q_ENUM(MenuIndicatorLayout)

	Q_PROPERTY(TextTruncateMode		textTruncateMode		READ textTruncateMode		WRITE setTextTruncateMode)
	Q_PROPERTY(TextWrapMode			textWrapMode			READ textWrapMode			WRITE setTextWrapMode)
	Q_PROPERTY(MenuIndicatorLayout	menuIndicatorLayout		READ menuIndicatorLayout	WRITE setMenuIndicatorLayout)
	Q_PROPERTY(int					horizontalPadding		READ horizontalPadding		WRITE setHorizontalPadding)
	Q_PROPERTY(int					verticalPadding			READ verticalPadding		WRITE setVerticalPadding)
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
			updateGeometry();
			updateRectLayout();
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

	int horizontalPadding() const { return _horizontalPadding; }
	void setHorizontalPadding(int padding) {
		if (_horizontalPadding != padding) {
			_horizontalPadding = qMax(0, padding);
			updateRectLayout();
			update();
		}
	}

	int verticalPadding() const { return _verticalPadding; }
	void setVerticalPadding(int padding) {
		if (_verticalPadding != padding) {
			_verticalPadding = qMax(0, padding);
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

	void setMenu(QMenu* menu) {
		QToolButton::setMenu(menu);

		updateGeometry();
		updateRectLayout();
		update();
	}

	virtual QSize minimumSizeHint() const override;
	virtual QSize sizeHint() const override;
protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
private:
	void updateRectLayout();
	void drawMenuIndicator(QPainter& p, const QRect& r, qreal thickness);

	TextTruncateMode _truncateMode{ TextTruncateMode::Clip };
	TextWrapMode _wrapMode{ TextWrapMode::WrapToFit };
	int _horizontalPadding		{ 0 };
	int _verticalPadding		{ 0 };
	int _iconTextSpacing		{ 10 };
	int _menuIndicatorSpacing	{ 2 };
	int _iconScalePercent		{ 50 };
	bool _reserveIconSpace		{ true };
	QColor _checkedBarColor		{QColor("#ffffff")};

	MenuIndicatorLayout _menuIndicatorLayout{ MenuIndicatorLayout::Separate };

	QRect _iconHitRect{ QRect() };
	QRect _textHitRect{ QRect() };
	QRect _menuHitRect{ QRect() };

	QRect _iconPaintRect{ QRect() };
	QRect _textPaintRect{ QRect() };
	QRect _menuPaintRect{ QRect() };
};

#endif