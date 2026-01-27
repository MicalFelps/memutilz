#pragma once

#include <functional>
#include <optional>
#include <QToolButton>

namespace Base {
class CustomPaintToolButton : public QToolButton {
	Q_OBJECT
public:
	using PainterFn = std::function<void(QPainter*, const QStyleOptionToolButton&)>;

	enum class ButtonState {
		Hover,
		Checked
		// Optionally add others like Pressed or CheckedHover
	};

	explicit CustomPaintToolButton(QWidget* parent)
		: QToolButton(parent) {}
	virtual ~CustomPaintToolButton() override = default;

	void setPainter(ButtonState state, PainterFn fn) {
		switch (state) {
		case ButtonState::Hover:	_hoverPainter = fn; break;
		case ButtonState::Checked:	_checkedPainter = fn; break;
		}
		update();
	}
protected:
	void paintEvent(QPaintEvent* event) override;
private:
	std::optional<PainterFn> _hoverPainter;
	std::optional<PainterFn> _checkedPainter;
};
}; // namespace Base