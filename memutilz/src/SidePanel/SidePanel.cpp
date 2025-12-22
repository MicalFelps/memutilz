#include "SidePanel/SidePanel.h"

#include "SidePanel/math.h"
#include "SidePanel/SidePanelHelpers.h"

#include "QDebug"

void SidePanel::setState(const SidePanelState new_state) {
	if (_state == new_state) {
		return;
	}

	_state = new_state;

	emit stateChanged(_state);
}

void SidePanel::updateHandlerRect(const qreal progress, const QRect& goem) {
	if (!_handler) return;
	const QRect handleGeom = alignedHandlerRect(goem, _handler->size(), progress);
	_handler->setGeometry(handleGeom);
}

// ---------------------------------------------------------

#include <QEvent>

bool SidePanel::eventFilter(QObject* watched, QEvent* event) {
	// Don't return true, because other widgets that might be
	// listening won't get notified of the event

	if (watched == parentWidget() &&
		(event->type() == QEvent::Resize || event->type() == QEvent::Move)) {

		const QRect parentRect = parentWidget()->rect();
		QRect geom;

		if (_state == SidePanelState::Opened || _state == SidePanelState::Opening) {
			geom = getOpenedRect(parentRect);
		} else {
			geom = getClosedRect(parentRect);
		}

		if (geometry() != geom) {
			setGeometry(geom);
			updateHandlerRect(_animProgress, geom);
		}
	}

	if (_triggerMode == TriggerMode::HoverOnly && watched == viewport()) {
		if (event->type() == QEvent::Enter) {
			if (_state == SidePanelState::Closing
				&& _timer->isActive()) {

				_timer->stop();
				setState(SidePanelState::Opening);
				_timeStart = std::chrono::system_clock::now();
				_timer->start();

			} else if (_state == SidePanelState::Closed
				&& !_timer->isActive()) {
				setState(SidePanelState::Opening);
				_timeStart = std::chrono::system_clock::now();
				_timer->start();
			}
		} else if (event->type() == QEvent::Leave) {
			if (_state == SidePanelState::Opening
				&& _timer->isActive()) {

				_timer->stop();
				setState(SidePanelState::Closing);
				_timeStart = std::chrono::system_clock::now();
				_timer->start();

			} else if (_state == SidePanelState::Opened
				&& !_timer->isActive()) {
				setState(SidePanelState::Closing);
				_timeStart = std::chrono::system_clock::now();
				_timer->start();
			}
		}
	}
	return base_t::eventFilter(watched, event);
}

// ---------------------------------------------------------

SidePanel::SidePanel(QWidget* parent, TriggerMode mode)
	: base_t(parent)
	, _triggerMode{mode}
{
	/*
	No member std::function are called here, they are called in init()
	*/

	setMouseTracking(true);
	viewport()->setMouseTracking(true);
	viewport()->setAttribute(Qt::WA_Hover);  // Helps with enter/leave
	viewport()->installEventFilter(this);

	_timer = new QTimer(this);
	_timer->setInterval(10);

	connect(_timer, &QTimer::timeout, this, [this] {
		const auto timeNow = std::chrono::system_clock::now();

		// If animation finished
		if ((timeNow - _timeStart) >= _duration) {
			_timer->stop();

			const QRect targetGeom = (_state == SidePanelState::Opening)
				? getOpenedRect(parentWidget()->rect())
				: getClosedRect(parentWidget()->rect());

			setGeometry(targetGeom);
			_animProgress = (_state == SidePanelState::Opening) ? 1.0 : 0.0;
			updateHandlerRect(_animProgress, targetGeom);

			if (_state == SidePanelState::Opening) {
				show();
				setState(SidePanelState::Opened);
			}
			else {
				if (_panelSizeMin <= 0) hide();
				setState(SidePanelState::Closed);
			}
			return;
		}
		
		// Raw progress since last direction change
		qreal rawProgress = q_sp::scale(
			timeNow.time_since_epoch().count(),
			_timeStart.time_since_epoch().count(),
			(_timeStart + _duration).time_since_epoch().count(),
			0.0,
			1.0
		);

		// Apply easing curve
		if (_state == SidePanelState::Opening) {
			rawProgress = _curveOnOpen.valueForProgress(rawProgress); }
		else if (_state == SidePanelState::Closing) {
			rawProgress = _curveOnClose.valueForProgress(rawProgress); }

		// Define two endpoints
		const QRectF closedGeom = getClosedRect(parentWidget()->rect());
		const QRectF openGeom = getOpenedRect(parentWidget()->rect());

		// Current position (what you currently see)
		const QRectF currentGeom = q_sp::lerp(_animProgress, closedGeom, openGeom);

		// Target geometry based on current state
		const QRectF targetGeom = (_state == SidePanelState::Opening) ? openGeom : closedGeom;

		// Smoothly interpolate from current → target using the eased rawProgress (step forward)
		const QRectF newGeom = q_sp::lerp(rawProgress, currentGeom, targetGeom);

		// update stored progress (based on width)
		qreal widthProgress = (newGeom.width() - closedGeom.width()) /
							  (openGeom.width() - closedGeom.width());

		_animProgress = qBound(0.0, widthProgress, 1.0);

		setGeometry(newGeom.toRect());
		updateHandlerRect(_animProgress, newGeom.toRect());
	});

	if (_triggerMode == TriggerMode::ClickOnly) {
		_handler = new HandlerWidgetT(parent);
		_handler->setObjectName("SidePanelHandler");
		_panelSizeMin = 0;
	} else {
		_handler = nullptr;
		setPanelSizeMin(20); // so it's not invisible
	}

	// =========================================================
	// Default behaviour same as PanelLeftSide

	this->getOpenedRect = [this](const QRect& parentRect) -> QRect
	{
		return q_sp::rect_opened_left(getPanelSize(), parentRect);
	};

	this->getClosedRect = [this](const QRect& parentRect) -> QRect
	{
		QRect r = q_sp::rect_closed_left(getPanelSize(), parentRect);
		r.setWidth(r.width() + _panelSizeMin);
		return r;
	};

	// -------------------------------------------------------------------------

	this->alignedHandlerRect = [](const QRect& panelGeom, const QSize& handlerSize, qreal) -> QRect
	{
		return q_sp::rect_aligned_right_center(panelGeom, handlerSize);
	};

	// -------------------------------------------------------------------------

	this->initialHandlerSize = [this]() -> QSize
	{
		return _handler ? _handler->size() : QSize();
	};

	// -------------------------------------------------------------------------

	this->updateHandler = [](const SidePanelState state, HandlerWidgetT* handler)
	{
		Q_UNUSED(state);
		Q_UNUSED(handler);
	};
}

SidePanel::~SidePanel()
{
	if (_timer != nullptr)
	{
		_timer->stop();

		delete _timer;
		_timer = nullptr;
	}

	if (_handler != nullptr) {
		delete _handler;
		_handler = nullptr;
	}

	if (parentWidget()) {
		removeEventFilter(this);
	}
}

void SidePanel::init()
{
	// Close on init
	QTimer::singleShot(0, [this] {
		const auto geom = getClosedRect(this->parentWidget()->rect());
		this->setGeometry(geom);
		_animProgress = 0.0;
		updateHandlerRect(_animProgress, geom);
		if (_panelSizeMin > 0) { show(); } 
		else { hide(); }
	});

	if (_triggerMode == TriggerMode::ClickOnly) {
		QTimer::singleShot(0, [this] {
			_handler->resize(initialHandlerSize());
			updateHandler(_state, _handler);
			});

		connect(_handler, &QAbstractButton::clicked, this, [this] {
			if (_timer->isActive()) {
				qDebug() << "Clicked During Animation";

				if (_state == SidePanelState::Opening) {
					setState(SidePanelState::Closing);
				} else if (_state == SidePanelState::Closing) {
					setState(SidePanelState::Opening);
				}
			} else {
				if (_state == SidePanelState::Closed) { show();
					setState(SidePanelState::Opening);
				} else if (_state == SidePanelState::Opened) {
					setState(SidePanelState::Closing);
				}

				_timeStart = std::chrono::system_clock::now();
				_timer->start();
			}
		});

		connect(this, &SidePanel::stateChanged, _handler, [this](SidePanelState state) {
			updateHandler(state, _handler);
		});
	}
}

// ---------------------------------------------------------

void SidePanel::openPanel()
{
	_timer->stop(); // Stop animation, if it's running

	this->show();

	const QRect newGeom = getOpenedRect(this->parentWidget()->rect());
	setGeometry(newGeom);

	_animProgress = 1.0;
	updateHandlerRect(_animProgress, newGeom);

	setState(SidePanelState::Opened);
}

void SidePanel::closePanel()
{
	_timer->stop(); // Stop animation, if it's running

	if (_triggerMode == TriggerMode::ClickOnly) { this->hide(); }

	const QRect newGeom = getClosedRect(this->parentWidget()->rect());
	setGeometry(newGeom);

	_animProgress = 0.0;
	updateHandlerRect(_animProgress, newGeom);

	setState(SidePanelState::Closed);
}

// ---------------------------------------------------------

void SidePanel::setDuration(const std::chrono::milliseconds& duration)
{
	_duration = duration;

	// TODO: is it safe during animation ?
}

std::chrono::milliseconds SidePanel::getDuration() const
{
	return _duration;
}

// ---------------------------------------------------------

void SidePanel::setPanelSize(int size)
{
	_panelSize = size;

	// TODO: handle somehow opened/closed state. and state during animation
}

int SidePanel::getPanelSize() const
{
	return _panelSize;
}

// ---------------------------------------------------------

void SidePanel::setPanelSizeMin(int size) {
	_panelSizeMin = size;
}

int SidePanel::getPanelSizeMin() const
{
	return _panelSizeMin;
}

// ---------------------------------------------------------

void SidePanel::setOpenEasingCurve(const QEasingCurve& curve)
{
	_curveOnOpen = curve;
}

void SidePanel::setCloseEasingCurve(const QEasingCurve& curve)
{
	_curveOnClose = curve;
}

// ---------------------------------------------------------

QSize SidePanel::getHandlerSize() const
{
	return _handler->size();
}

// ---------------------------------------------------------

#include <QResizeEvent>

void SidePanel::resizeEvent(QResizeEvent* event)
{
	base_t::resizeEvent(event);

	updateHandlerRect(_animProgress, geometry());
}