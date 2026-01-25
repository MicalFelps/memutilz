#pragma once

#include <QWidget>

class CentralDockingArea : public QWidget {
	Q_OBJECT

public:
	explicit CentralDockingArea(QWidget* parent = nullptr);
	virtual ~CentralDockingArea() = default;
private:

};