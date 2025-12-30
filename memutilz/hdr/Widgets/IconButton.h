#ifndef WIDGETS_ICONBUTTON_H
#define WIDGETS_ICONBUTTON_H

#include <QToolButton>
#include <QIcon>
#include <QColor>

class IconButton : public QToolButton {
	Q_OBJECT

public:
	enum class TextTruncateMode { Clip, NoClip, HideIfClip, Ellipsis };
	enum class TextWrapMode { NoWrap, WrapToFit };

	Q_ENUM(TextTruncateMode)
	Q_ENUM(TextWrapMode)

	Q_PROPERTY(TextTruncateMode textTruncateMode READ textTruncateMode WRITE setTextTruncateMode)
	Q_PROPERTY(TextWrapMode     textWrapMode     READ textWrapMode     WRITE setTextWrapMode)
	Q_PROPERTY(int              horizontalMargin READ horizontalMargin WRITE setHorizontalMargin)
	Q_PROPERTY(int              verticalMargin   READ verticalMargin   WRITE setVerticalMargin)
	Q_PROPERTY(int              iconTextSpacing  READ iconTextSpacing  WRITE setIconTextSpacing)
	Q_PROPERTY(int				menuIndicatorSpacing READ menuIndicatorSpacing WRITE setMenuIndicatorSpacing)
	Q_PROPERTY(int              iconScalePercent READ iconScalePercent WRITE setIconScalePercent)
	Q_PROPERTY(bool				reserveIconSpace READ reserveIconSpace WRITE setReserveIconSpace)
	Q_PROPERTY(QColor           checkedBarColor  READ checkedBarColor  WRITE setCheckedBarColor)

	explicit IconButton(const QIcon& icon = QIcon(), const QString& text = QString(), QWidget* parent = nullptr);
	virtual ~IconButton() = default;

	TextTruncateMode textTruncateMode() const { return _truncateMode; }
	void setTextTruncateMode(TextTruncateMode mode) {
		if (_truncateMode != mode) {
			_truncateMode = mode;
			updateGeometry(); // signal size hints might have changed
			update();
		}
	}

	TextWrapMode textWrapMode() const { return _wrapMode; }
	void setTextWrapMode(TextWrapMode mode) {
		if (_wrapMode != mode) {
			_wrapMode = mode;
			updateGeometry(); // signal size hints might have changed
			update();
		}
	}

	int horizontalMargin() const { return _horizontalMargin; }
	void setHorizontalMargin(int margin) {
		if (_horizontalMargin != margin) {
			_horizontalMargin = qMax(0, margin);
			updateGeometry();
			update();
		}
	}

	int verticalMargin() const { return _verticalMargin; }
	void setVerticalMargin(int margin) {
		if (_verticalMargin != margin) {
			_verticalMargin = qMax(0, margin);
			updateGeometry();
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
private:
	TextTruncateMode _truncateMode{ TextTruncateMode::NoClip };
	TextWrapMode _wrapMode{ TextWrapMode::WrapToFit };
	int _horizontalMargin		{ 10 };
	int _verticalMargin			{ 10 };
	int _iconTextSpacing		{ 5 };
	int _menuIndicatorSpacing	{ 5 };
	int _iconScalePercent		{ 90 };
	bool _reserveIconSpace		{ true };
	QColor _checkedBarColor		{QColor()};
};

#endif