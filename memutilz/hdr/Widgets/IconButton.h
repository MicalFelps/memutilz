#ifndef WIDGETS_ICONBUTTON_H
#define WIDGETS_ICONBUTTON_H

#include <QToolButton>
#include <QIcon>
#include <QColor>

class IconButton : public QToolButton {
	Q_OBJECT

public:
	enum class TextTruncateMode { Clip, NoClip, Ellipsis };
	enum class TextWrapMode { NoWrap, WrapToFit };

	Q_ENUM(TextTruncateMode)
	Q_ENUM(TextWrapMode)

	Q_PROPERTY(TextTruncateMode textTruncateMode		READ textTruncateMode		WRITE setTextTruncateMode)
	Q_PROPERTY(TextWrapMode     textWrapMode			READ textWrapMode			WRITE setTextWrapMode)
	Q_PROPERTY(int				horizontalPadding		READ horizontalPadding		WRITE setHorizontalPadding)
	Q_PROPERTY(int				verticalPadding			READ verticalPadding		WRITE setVerticalPadding)
	Q_PROPERTY(int              iconTextSpacing			READ iconTextSpacing		WRITE setIconTextSpacing)
	Q_PROPERTY(int				menuIndicatorSpacing	READ menuIndicatorSpacing	WRITE setMenuIndicatorSpacing)
	Q_PROPERTY(int              iconScalePercent		READ iconScalePercent		WRITE setIconScalePercent)
	Q_PROPERTY(bool				reserveIconSpace		READ reserveIconSpace		WRITE setReserveIconSpace)
	Q_PROPERTY(QColor           checkedBarColor			READ checkedBarColor		WRITE setCheckedBarColor)

	explicit IconButton(const QIcon& icon = QIcon(), const QString& text = QString(), QWidget* parent = nullptr);
	virtual ~IconButton() = default;

	TextTruncateMode textTruncateMode() const { return _truncateMode; }
	void setTextTruncateMode(TextTruncateMode mode) {
		if (_truncateMode != mode) {
			_truncateMode = mode;
			updateGeometry();
			update();
		}
	}

	TextWrapMode textWrapMode() const { return _wrapMode; }
	void setTextWrapMode(TextWrapMode mode) {
		if (_wrapMode != mode) {
			_wrapMode = mode;
			updateGeometry();
			update();
		}
	}

	int horizontalPadding() const { return _horizontalPadding; }
	void setHorizontalPadding(int padding) {
		if (_horizontalPadding != padding) {
			_horizontalPadding = qMax(0, padding);
			update();
		}
	}

	int verticalPadding() const { return _verticalPadding; }
	void setVerticalPadding(int padding) {
		if (_verticalPadding != padding) {
			_verticalPadding = qMax(0, padding);
			update();
		}
	}

	int iconTextSpacing() const { return _iconTextSpacing; }
	void setIconTextSpacing(int iconTextSpacing) {
		if (_iconTextSpacing != iconTextSpacing) {
			_iconTextSpacing = qMax(0, iconTextSpacing);
			updateGeometry();
			update();
		}
	}

	int menuIndicatorSpacing() const { return _menuIndicatorSpacing; }
	void setMenuIndicatorSpacing(int spacing) {
		if (_menuIndicatorSpacing != spacing) {
			_menuIndicatorSpacing = spacing;
			updateGeometry();
			update();
		}
	}

	int iconScalePercent() const { return _iconScalePercent; }
	void setIconScalePercent(int percent) {
		if (_iconScalePercent != percent) {
			_iconScalePercent = qBound(0, percent, 100);
			updateGeometry();
			update();
		}
	}

	bool reserveIconSpace() const { return _reserveIconSpace; }
	void setReserveIconSpace(bool reserveIconSpace) {
		if (_reserveIconSpace != reserveIconSpace) {
			_reserveIconSpace = reserveIconSpace;
			updateGeometry();
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

	virtual QSize minimumSizeHint() const override;
	virtual QSize sizeHint() const override;
protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
private:
	QRect calculateArrowRect(QRect contentRect = QRect());

	TextTruncateMode _truncateMode{ TextTruncateMode::Clip };
	TextWrapMode _wrapMode{ TextWrapMode::WrapToFit };
	int _horizontalPadding		{ 0 };
	int _verticalPadding		{ 0 };
	int _iconTextSpacing		{ 10 };
	int _menuIndicatorSpacing	{ 2 };
	int _iconScalePercent		{ 80 };
	bool _reserveIconSpace		{ true };
	QColor _checkedBarColor		{QColor("#ffffff")};
};

#endif