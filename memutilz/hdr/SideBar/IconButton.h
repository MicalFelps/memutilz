#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QPushButton>
#include <QIcon>
#include <QString>

#include "Colors.h"

class IconButton : public QPushButton {
	Q_OBJECT
public:
	explicit IconButton(
		const QIcon& icon = QIcon()
		, const QString& text = QString()
		, QWidget* parent = nullptr);
	virtual ~IconButton() = default;

	// --- Padding ---
	int getIconTextGap() const { return _iconTextGap; }
	void setIconTextGap(int iconTextGap) { 
		if (_iconTextGap != iconTextGap) {
			_iconTextGap = qMax(0, iconTextGap);
			update();
		}
	}

	int getIconSizePercentage() const { return _iconSizePercentage; }
	void setIconSizePercentage(int percent) {
		if (_iconSizePercentage != percent) {
			_iconSizePercentage = qBound(0, percent, 100);
			update();
		}
	}

	// --- Colors ---

	QColor normalBackgroundColor() const { return _normalBgColor; }
	void setNormalBackgroundColor(const QColor& color) {
		if (_normalBgColor != color) {
			_normalBgColor = color;
			update();  // Trigger repaint
		}
	}
	QColor hoverBackgroundColor() const { return _hoverBgColor; }
	void setHoverBackgroundColor(const QColor& color) {
		if (_hoverBgColor != color) {
			_hoverBgColor = color;
			update();
		}
	}
	QColor selectedBarColor() const { return _selectedBarColor; }
	void setSelectedBarColor(const QColor& color) {
		if (_selectedBarColor != color) {
			_selectedBarColor = color;
			update();
		}
	}

	// --- Selection State ---

	bool isSelected() const { return _selected; }
	void setSelected(bool isSelected) { _selected = isSelected; update(); }

	bool isSelectable() const { return _selectable; }
	void setSelectable(bool selectable) { _selectable = selectable; }

	// --- Size Hints ---

	virtual QSize minimumSizeHint() const override;
	virtual QSize sizeHint() const override;
protected:
	virtual void enterEvent(QEnterEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void paintEvent(QPaintEvent* event) override;
private:
	QIcon _icon				{ QIcon() };
	QString _text			{ QString() };
	int _iconTextGap		{ 5 };				// px -> arbitrary
	int _iconSizePercentage	{ 40 };				// 40%

	bool _hovered			{ false };
	bool _selected			{ false };			// Off by default
	bool _selectable		{ true };

	QColor _normalBgColor	{ Colors::Base };
	QColor _hoverBgColor	{ Colors::Surface };
	QColor _selectedBarColor{ Colors::White };
};

#endif