#ifndef WIDGETS_SIDEBAR_H
#define WIDGETS_SIDEBAR_H

#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QFrame>
#include <QHash>

#include "Widgets/IconButton.h"
#include "Widgets/Pages.h"

class SideBar : public QWidget {
	Q_OBJECT

public:
	using HandlerWidget = IconButton;

	enum class ExpandMode {
		Click,
		Hover
	};

	enum class State {
		Opening = 0,
		Opened,
		Closing,
		Closed
	};

	explicit SideBar(
		ExpandMode mode = ExpandMode::Hover,
		QWidget* parent = nullptr);
	virtual ~SideBar() = default;

	void collapseNow() { collapse(false); }
	void expandNow() { expand(false); }

	void addTopButton(IconButton* button, PageId id = PageId::None);
	void addBottomButton(IconButton* button, PageId id = PageId::None);
	// void removeButton(IconButton* button);

	IconButton* selectedButton() const { return _currSelection; }

signals:
	void stateChanged(State state);
	void selectedPageChanged(PageId page);

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void enterEvent(QEnterEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override; // to update handler

private slots:
	void onNewSelectedPage(IconButton* selectedButton);
	void onHandlerClicked();
private:
	ExpandMode _expandMode{ ExpandMode::Hover };
	State _state{ State::Closed };
	void setState(const State state);

	QList<IconButton*> _topButtons{ QList<IconButton*>() };
	QList<IconButton*> _bottomButtons{ QList<IconButton*>() };
	QHash<IconButton*, PageId> _buttonMap{ QHash<IconButton*, PageId>() };
	QWidget* _contentWidget{ nullptr };

	QTimer* _hoverTimer{ nullptr };
	bool _hovering{ false };
	QPropertyAnimation* _animation{ nullptr };

	HandlerWidget* _handler{ nullptr };
	IconButton* _currSelection{ nullptr };

	QFrame* _separatorLine{ nullptr };

	void expand(bool animate = true);
	void collapse(bool animate = true);
};

#endif
