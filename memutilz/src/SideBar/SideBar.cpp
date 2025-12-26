#include "Colors.h"
#include "SideBar/SideBar.h"
#include "SideBar/SideBarState.h"

#include <Chrono>
#include <QStyle>

SideBar::SideBar(QWidget* parent, ExpandMode mode, QWidget* contentWidget)
	: QWidget(parent)
	, _expandMode{mode}
	, _contentWidget{contentWidget}
{
	setMouseTracking(true);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	setAutoFillBackground(true);

	setMinimumWidth(config::collapsedWidth);
	setMaximumWidth(config::expandedWidth);
	resize(QSize(minimumWidth(), parent->height()));

	QPalette p = palette();
	p.setColor(QPalette::Window, Colors::Surface); // background
	setPalette(p);

	if (_expandMode == ExpandMode::Hover) {
		_hoverTimer = new QTimer(this);
		_hoverTimer->setSingleShot(true);

		connect(_hoverTimer, &QTimer::timeout, this, [this]() {
			if (_hovering) {
				expand(true);
			}
			else {
				collapse(true);
			}
		});
	}

	_animation = new QPropertyAnimation(this, "geometry", this);
	_animation->setDuration(config::animDuration);
	_animation->setEasingCurve(QEasingCurve::Type::OutCubic);

	this->_updateHandler = [](const SideBarState state, HandlerWidgetT* handler)
		{
			Q_UNUSED(state);
			Q_UNUSED(handler);
		};

	if (_expandMode == ExpandMode::Click) {
		_handler = new IconButton(QIcon(":/MainWindow/icons/options.svg"), QString(), this);
		_handler->setSelectable(false);
		addTopButton(_handler);
	}
	
	// Change the geometry of the content widget, since we start collapsed
	// It doesn't really matter what _overlapWithContent is
	if (_contentWidget) {
		QRect target { parent->geometry() };
		target.setWidth(target.width() - minimumWidth());
		target.moveTopLeft(this->rect().topRight());
		_contentWidget->setGeometry(target);
	}
}

void SideBar::initConnections() {
	if (_expandMode == ExpandMode::Click) {
		connect(_handler, &IconButton::clicked, this, [this]() {
			onHandlerClicked();
			});

		connect(this, &SideBar::stateChanged, _handler, [this](SideBarState state) {
			_updateHandler(state, _handler);
			});
	}

	connect(_animation, &QPropertyAnimation::finished, this, [this]() {
		if (_state == SideBarState::Opening) {
			setState(SideBarState::Opened);
		} else if (_state == SideBarState::Closing) {
			setState(SideBarState::Closed);
		}
	});
}

// ---------------------------------------------------------

void SideBar::setOverlapWithContent(bool overlap) {
	_overlapWithContent = overlap;

	// update geometry once, then let resize recycle the logic
	if (!_overlapWithContent) {
		QRect target{ _contentWidget->rect() };
		target.setSize({ parentWidget()->width() - width(), parentWidget()->height() });
		target.moveTopLeft(rect().topRight());
		_contentWidget->setGeometry(target);
	}
}

// ---------------------------------------------------------

void SideBar::addTopButton(IconButton* button) {
	if (!button || _topButtons.contains(button)) return;

	QRect r = this->rect();
	r.setSize(QSize(width(), config::iconSize));
	_topButtons.isEmpty() ?
		r.moveTopLeft(QPoint(0, 0))
		: r.moveTopLeft(_topButtons.last()->geometry().bottomLeft());
	
	button->setGeometry(r);
	_topButtons.append(button);

	if (button != _handler) {
		connect(button, &IconButton::clicked, this, [this, button]() {
			onNewSelection(button);
		});
	}
	
	// First button becomes selected
	if (_topButtons.size() > 2) return;

	const int index = (_handler != nullptr) ? 1 : 0;

	if (_topButtons.size() == index + 1 &&
		_topButtons[index] == button)
	{
		onNewSelection(button);
	}
}
void SideBar::addBottomButton(IconButton* button) {
	QRect r = this->rect();
	r.setSize(QSize(width(), config::iconSize));
	_bottomButtons.isEmpty() ?
		r.moveBottomLeft(rect().bottomLeft())
		: r.moveBottomLeft(_bottomButtons.last()->geometry().topLeft());

	button->setGeometry(r);
	_bottomButtons.append(button);

	connect(button, &IconButton::clicked, this, [this, button]() {
		onNewSelection(button);
	});
}

// ------------------- PROTECTED -------------------

void SideBar::enterEvent(QEnterEvent* event) {
	QWidget::enterEvent(event);
	_hovering = true;
	if (_expandMode == ExpandMode::Hover) {
		_hoverTimer->start(config::hoverDelay);
	}
}

void SideBar::leaveEvent(QEvent* event) {
	QWidget::leaveEvent(event); 
	_hovering = false;
	if (_expandMode == ExpandMode::Hover) {
		_hoverTimer->start(0);
	}
}

void SideBar::resizeEvent(QResizeEvent* event) {
	QWidget::resizeEvent(event);

	for (auto& b : _topButtons) {
		b->setGeometry({QPoint(b->pos()), QSize(width(), b->height())});
	}

	QPoint lastBottomRect{rect().bottomLeft()};

	for (auto& b : _bottomButtons) {
		QRect r{};
		r.setSize(QSize(width(), b->height()));
		r.moveBottomLeft(lastBottomRect);

		b->setGeometry(r);
		lastBottomRect = b->geometry().topLeft();
	}

	if (!_overlapWithContent) {
		QRect target{ _contentWidget->rect() };
		target.setSize({ parentWidget()->width() - width(), parentWidget()->height() });
		target.moveTopLeft(rect().topRight());
		_contentWidget->setGeometry(target);
	}
}

// --------------------- SLOTS ---------------------

void SideBar::onNewSelection(IconButton* selectedButton) {
	if (!selectedButton->isSelectable()) return; 

	if (_currSelection && _currSelection != selectedButton) {
		_currSelection->setSelected(false);
	}
	_currSelection = selectedButton;
	_currSelection->setSelected(true);
	emit selectionChanged(_currSelection);
}

void SideBar::onHandlerClicked() {
	qDebug() << to_str(_state);

	if (_state == SideBarState::Closed
		|| _state == SideBarState::Closing)
	{
		expand(true);
	}
	else if (_state == SideBarState::Opened
		|| _state == SideBarState::Opening)
	{
		collapse(true);
	}
}

// -------------------- PRIVATE --------------------

void SideBar::setState(const SideBarState state) {
	if (_state == state) return;
	_state = state;
	emit stateChanged(_state);
}

void SideBar::expand(bool animate) {
	if (_state == SideBarState::Opened || _state == SideBarState::Opening) return;

	_animation->stop();

	if (!animate) {
		QRect r = geometry();
		r.setWidth(config::expandedWidth);
		setGeometry(r);
		setState(SideBarState::Opened);
		return;
	}

	QRect geom{ geometry() };
	QRect target{ geom };
	target.setWidth(maximumWidth());

	_animation->setStartValue(geom);
	_animation->setEndValue(target);
	_animation->start();

	setState(SideBarState::Opening);
}
void SideBar::collapse(bool animate) {
	if (_state == SideBarState::Closed || _state == SideBarState::Closing) return;

	_animation->stop();

	if (!animate) {
		QRect r = geometry();
		r.setWidth(config::collapsedWidth);
		setGeometry(r);
		setState(SideBarState::Closed);
		return;
	}

	QRect geom{ geometry() };
	QRect target{ geom };

	qDebug() << minimumWidth();

	target.setWidth(minimumWidth());

	_animation->setStartValue(geom);
	_animation->setEndValue(target);
	_animation->start();

	setState(SideBarState::Closing);
}