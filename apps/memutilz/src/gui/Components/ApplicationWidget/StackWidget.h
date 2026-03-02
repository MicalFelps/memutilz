#pragma once

#include <QStackedWidget>

class StackedWidget : public QStackedWidget {
	 Q_OBJECT
public:
	explicit StackedWidget(QWidget* parent) : QStackedWidget(parent){}
	virtual ~StackedWidget() = default;
};