#ifndef CENTRALDOCKINGAREA_H
#define CENTRALDOCKINGAREA_H

#include <QWidget>

class CentralDockingArea : public QWidget {
	Q_OBJECT

public:
	explicit CentralDockingArea(QWidget* parent = nullptr);
	virtual ~CentralDockingArea() = default;
private:

};

#endif // CENTRALDOCKINGAREA_H