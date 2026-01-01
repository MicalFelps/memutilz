#include "Widgets/Metrics.h"
#include "Widgets/SideBar.h"

#include <Chrono>
#include <QStyle>

SideBar::SideBar(QWidget* contentWidget, ExpandMode mode, QWidget* parent)
	: _contentWidget{ contentWidget }
	, _expandMode{ mode }
	, QWidget( parent )
{
	setMouseTracking(true);
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	setMinimumWidth(Ui::SideBar::collapsedWidth);
	setMaximumWidth(Ui::SideBar::expandedWidth);
	resize(QSize(minimumWidth(), parent->height()));

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
	_animation->setDuration(Ui::SideBar::animDuration);
	_animation->setEasingCurve(QEasingCurve::Type::OutCubic);

	connect(_animation, &QPropertyAnimation::finished, this, [this]() {
		if (_state == State::Opening) {
			setState(State::Opened);
		}
		else if (_state == State::Closing) {
			setState(State::Closed);
		}
		});

	if (_expandMode == ExpandMode::Click) {
		_handler = new IconButton(QIcon(":/icons/3bars.svg"), QString(), this);
		_handler->setCheckable(false);
		addTopButton(_handler);

		connect(_handler, &IconButton::clicked, this, [this]() {
			onHandlerClicked();
			});
	}

	// Change the geometry of the content widget, we start collapsed
	if (_contentWidget) {
		QRect target { parent->geometry() };
		target.setWidth(target.width() - minimumWidth());
		target.moveTopLeft(this->rect().topRight());
		_contentWidget->setGeometry(target);
	}
}

// ---------------------------------------------------------

void SideBar::addTopButton(IconButton* button) {
	if (!button || _topButtons.contains(button)) return;

	QRect r = this->rect();
	r.setSize(QSize(width(), Ui::SideBar::buttonHeight));
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
	r.setSize(QSize(width(), Ui::SideBar::buttonHeight));
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
		_hoverTimer->start(Ui::SideBar::hoverDelay);
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

	if (_contentWidget) {
		QRect target{ _contentWidget->rect() };
		target.setSize({ parentWidget()->width() - width(), parentWidget()->height() });
		target.moveTopLeft(rect().topRight());
		_contentWidget->setGeometry(target);
	}
}

// --------------------- SLOTS ---------------------

void SideBar::onNewSelection(IconButton* selectedButton) {
	if (!selectedButton->isCheckable()) return; 

	if (_currSelection && _currSelection != selectedButton) {
		_currSelection->setChecked(false);
	}
	_currSelection = selectedButton;
	_currSelection->setChecked(true);
	emit selectionChanged(_currSelection);
}
void SideBar::onHandlerClicked() {
	if (_state == State::Closed
		|| _state == State::Closing)
	{
		expand(true);
	}
	else if (_state == State::Opened
		|| _state == State::Opening)
	{
		collapse(true);
	}
}

// -------------------- PRIVATE --------------------

void SideBar::setState(const State state) {
	if (_state == state) return;
	_state = state;
	emit stateChanged(_state);
}

void SideBar::expand(bool animate) {
	if (_state == State::Opened || _state == State::Opening) return;

	_animation->stop();

	if (!animate) {
		QRect r = geometry();
		r.setWidth(Ui::SideBar::expandedWidth);
		setGeometry(r);
		setState(State::Opened);
		return;
	}

	QRect geom{ geometry() };
	QRect target{ geom };
	target.setWidth(maximumWidth());

	_animation->setStartValue(geom);
	_animation->setEndValue(target);
	_animation->start();

	setState(State::Opening);
}
void SideBar::collapse(bool animate) {
	if (_state == State::Closed || _state == State::Closing) return;

	_animation->stop();

	if (!animate) {
		QRect r = geometry();
		r.setWidth(Ui::SideBar::collapsedWidth);
		setGeometry(r);
		setState(State::Closed);
		return;
	}

	QRect geom{ geometry() };
	QRect target{ geom };

	target.setWidth(minimumWidth());

	_animation->setStartValue(geom);
	_animation->setEndValue(target);
	_animation->start();

	setState(State::Closing);
}