#pragma once

#include <QFrame>

class QShowEvent;
class QKeyEvent;
class MainWindow;

class ApplicationWidgetInternal : public QFrame {
    Q_OBJECT

public:
    explicit ApplicationWidgetInternal(MainWindow* parent = nullptr);
    virtual ~ApplicationWidgetInternal() override;

protected:
    virtual void resizeToParent(const QSize& parentSize);

protected:
    virtual bool eventFilter(QObject* obj, QEvent* e) override;
    virtual void showEvent(QShowEvent* e) override;
    virtual void keyPressEvent(QKeyEvent* e) override;
};
