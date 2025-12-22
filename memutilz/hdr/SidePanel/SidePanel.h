#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include <QScrollArea>

#include <QTimer>
#include <chrono>

#include <QEasingCurve>

#include <QPushButton>

#include <functional>

#include "SidePanel/SidePanelState.h"

class SidePanel : public QScrollArea {
	Q_OBJECT
public:
	using HandlerWidgetT = QPushButton;

	enum class TriggerMode {
		ClickOnly, // has a handler
		HoverOnly, // doesn't have a handler
	};

	Q_ENUM(TriggerMode);

private:
	TriggerMode _triggerMode;
	SidePanelState _state = SidePanelState::Closed;
	void setState(const SidePanelState new_state);

	std::chrono::milliseconds _duration{ 500 };
	std::chrono::system_clock::time_point _timeStart;

	QEasingCurve _curveOnOpen{ QEasingCurve::Type::Linear };
	QEasingCurve _curveOnClose{ QEasingCurve::Type::Linear };

	QTimer* _timer{ nullptr };
	HandlerWidgetT* _handler{ nullptr };

	qreal _animProgress{ 0.0 };
	
	void updateHandlerRect(const qreal progress, const QRect& goem);

	int _panelSize{ 100 }; // px
	int _panelSizeMin{ 0 }; // minimum

protected:
	// To intercept resize and move events before they get passed to parent
	bool eventFilter(QObject* watched, QEvent* event) override;

public:
	// =========================================================
	// Callbacks for custom behaviour

	using rect_func_t = std::function<QRect(const QRect& /*parent rect*/)>;

	rect_func_t getOpenedRect;
	rect_func_t getClosedRect;

	// ---------------------------------------------------------

	using handler_align_func_t = std::function<QRect(const QRect& /*panel_geom*/, const QSize& /*handler_size*/, qreal /*progress*/)>;

	handler_align_func_t alignedHandlerRect;

	// ---------------------------------------------------------

	using handler_size_func_t = std::function<QSize()>;

	handler_size_func_t initialHandlerSize;
	
	// ---------------------------------------------------------

	using handler_update_func_t = std::function<void(const SidePanelState, HandlerWidgetT*)>;

	handler_update_func_t updateHandler;

	// =========================================================

	using base_t = QScrollArea;
	explicit SidePanel(QWidget* parent = nullptr, TriggerMode mode = TriggerMode::HoverOnly);
	virtual ~SidePanel() override;

	void init();

	// Immediately open/close panel without animation
	void openPanel();
	void closePanel();

	void setDuration(const std::chrono::milliseconds& duration);
	std::chrono::milliseconds getDuration() const;

	void setPanelSize(int size);
	int getPanelSize() const;

	void setPanelSizeMin(int size);
	int getPanelSizeMin() const;

	void setOpenEasingCurve(const QEasingCurve& curve);
	void setCloseEasingCurve(const QEasingCurve& curve);

	QSize getHandlerSize() const;

signals:
	void stateChanged(SidePanelState state);

protected:
	void resizeEvent(QResizeEvent* event) override;
};

#endif // SIDEPANEL_H