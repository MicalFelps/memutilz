#include "Widgets/Metrics.h"
#include "Widgets/SideBar.h"

#include <chrono>
#include <QStyle>

#include <QPainter>
#include <QStyleOption>

SideBar::SideBar(ExpandMode mode, QWidget* parent)
	: _expandMode{ mode }
	, QWidget( parent )
{
	setMouseTracking(true);
	setAutoFillBackground(true);

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	setMinimumWidth(Ui::SideBar::collapsedWidth);
	setMaximumWidth(Ui::SideBar::expandedWidth);

	// setAttribute(Qt::WA_StyledBackground, true);

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

	_animation = new QPropertyAnimation(this, "minimumWidth", this);
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

		_handler->setIconScalePercent(50);
		_handler->setIconTextSpacing(20);
		_handler->setPadding({ (Ui::SideBar::buttonHeight - _handler->iconPaintRect().width()) / 2, 0, 0, 0 });
		_handler->setProperty("group", "sidebar");
	}

	_separatorLine = new QFrame(this);
	_separatorLine->setObjectName("sidebarSeparator");
	_separatorLine->setFrameShape(QFrame::HLine);
	_separatorLine->setFrameShadow(QFrame::Sunken);
	_separatorLine->setVisible(false);
}

// ---------------------------------------------------------

void SideBar::addTopButton(IconButton* button, PageId id) {
	if (!button || _topButtons.contains(button)) return;

	QRect r = this->rect();
	r.setSize(QSize(width(), Ui::SideBar::buttonHeight));
	_topButtons.isEmpty() 
		? r.moveTop(0)
		: r.moveTop(_topButtons.last()->geometry().bottom() + 1);

	button->setGeometry(r);
	_topButtons.append(button);

	// add to hashmap
	_buttonMap[button] = id;
	if (button != _handler && id != PageId::None) {
		connect(button, &IconButton::clicked, this, [this, button]() {
			onNewSelection(button);
		});
	}

	// First button becomes selected
	if (_topButtons.size() > 2 || _topButtons.isEmpty()) return;

	const int index = (_handler != nullptr) ? 1 : 0;

	if (_topButtons.size() == index + 1 &&
		_topButtons[index] == button)
	{
		onNewSelection(button);
	}

	if(_separatorLine) _separatorLine->raise();
}
void SideBar::addBottomButton(IconButton* button, PageId id) {
	QRect r = this->rect();
	r.setSize(QSize(width(), Ui::SideBar::buttonHeight));
	_bottomButtons.isEmpty()
		? r.moveBottom(rect().bottom())
		: r.moveBottom(_bottomButtons.last()->geometry().top() - 1);

	button->setGeometry(r);
	_bottomButtons.append(button);

	// add to hashmap
	_buttonMap[button] = id;
	connect(button, &IconButton::clicked, this, [this, button]() {
		onNewSelection(button);
	});

	_separatorLine->raise();
}
void SideBar::removeButton(IconButton* button) {
	if (_buttonMap.contains(button) && button != _handler) {
		_buttonMap.remove(button);

		if (_bottomButtons.contains(button)) {
			_bottomButtons.removeOne(button);

			int lastTopY{ rect().bottom() };

			for (auto& b : _bottomButtons) {
				b->move(b->x(), lastTopY - b->height());
				lastTopY = b->y();
			}
		}
		else { // in top buttons
			_topButtons.removeOne(button);

			int lastBottomY{ rect().top() };

			for (auto& b : _topButtons) {
				b->move(b->x(), lastBottomY);
				lastBottomY += b->height();
			}
		}
	}
}

// ------------------- PROTECTED -------------------

void SideBar::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event);

	QPainter p(this);

	QStyleOption opt;
	opt.initFrom(this);

	// background and border
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	if (_topButtons.isEmpty() || _bottomButtons.isEmpty()) return;

	const int topY = _bottomButtons.last()->y();
	const int bottomY = _topButtons.last()->y() + _topButtons.last()->height();

	const bool overlap = (topY <= bottomY);

	if (overlap) {
		_separatorLine->setGeometry(0, topY, width(), 1);
	}

	_separatorLine->setVisible(overlap);
}
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

	int lastTopY{rect().bottom()};

	for (auto& b : _bottomButtons) {
		QRect r{};
		r.setSize(QSize(width(), b->height()));
		r.moveBottom(lastTopY);

		b->setGeometry(r);
		lastTopY = b->geometry().top() - 1;
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

	PageId id = _buttonMap.value(_currSelection);
	if (id != PageId::None) {
		emit selectedPageChanged(_buttonMap.value(_currSelection));
	}
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
		setMinimumWidth(Ui::SideBar::expandedWidth);
		setState(State::Opened);
		return;
	}

	int currMinW = minimumWidth();
	int target{ Ui::SideBar::expandedWidth };

	_animation->setStartValue(currMinW);
	_animation->setEndValue(target);
	_animation->start();

	setState(State::Opening);
}
void SideBar::collapse(bool animate) {
	if (_state == State::Closed || _state == State::Closing) return;

	_animation->stop();

	if (!animate) {
		setMinimumWidth(Ui::SideBar::collapsedWidth);
		setState(State::Closed);
		return;
	}

	int currMinW = minimumWidth();
	int target{ Ui::SideBar::collapsedWidth };

	_animation->setStartValue(currMinW);
	_animation->setEndValue(target);
	_animation->start();

	setState(State::Closing);
}