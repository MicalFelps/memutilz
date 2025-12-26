#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <functional>
#include <QTimer>
#include <QPropertyAnimation>

#include "SideBar/Config.h"
#include "SideBar/SideBarState.h"
#include "IconButton.h"

class SideBar : public QWidget {
	Q_OBJECT
public:
	using HandlerWidgetT = IconButton;

	enum class ExpandMode {
		Click,
		Hover
	};

	using UpdateHandlerFn = std::function<void(const SideBarState, HandlerWidgetT*)>;
	UpdateHandlerFn _updateHandler;

	explicit SideBar(
		QWidget* parent = nullptr,
		ExpandMode mode = ExpandMode::Hover,
		QWidget* contentWidget = nullptr);
	virtual ~SideBar() = default;

	void initConnections();

	void collapseNow() { collapse(false); }
	void expandNow() { expand(false); }

	bool getOverlapWithContent() const { return _overlapWithContent; }
	void setOverlapWithContent(bool overlap);

	void addTopButton(IconButton* button);
	void addBottomButton(IconButton* button);
	// void removeButton(IconButton* button);

	IconButton* getSelectedButton() const { return _currSelection; }

signals:
	void stateChanged(SideBarState state);
	void selectionChanged(IconButton* button);

protected:
	virtual void enterEvent(QEnterEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override; // to update handler

private slots:
	void onNewSelection(IconButton* selectedButton);
	void onHandlerClicked();
private:
	ExpandMode _expandMode{ ExpandMode::Hover };
	SideBarState _state{ SideBarState::Closed };
	void setState(const SideBarState state);

	QList<IconButton*> _topButtons{ QList<IconButton*>() };
	QList<IconButton*> _bottomButtons{ QList<IconButton*>() };
	QWidget* _contentWidget{ nullptr };

	QTimer* _hoverTimer{ nullptr };
	bool _hovering{ false };
	QPropertyAnimation* _animation{ nullptr };

	HandlerWidgetT* _handler{ nullptr };
	IconButton* _currSelection{ nullptr };

	bool _overlapWithContent{ true };

	void expand(bool animate = true);
	void collapse(bool animate = true);
};

#endif