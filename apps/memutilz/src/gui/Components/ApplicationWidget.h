#pragma once

#include <QFrame>
#include "../../Globals.h"

class QShowEvent;
class QKeyEvent;
class MainWindow;

class ApplicationWidget : public QFrame {
    Q_OBJECT

   public:
    explicit ApplicationWidget(MainWindow* parent);
    virtual ~ApplicationWidget() override;

   protected:
    virtual void resizeToParent(const QSize& parentSize);

   protected:
    virtual bool eventFilter(QObject* obj, QEvent* e) override;
    virtual void showEvent(QShowEvent* e) override;
    virtual void keyPressEvent(QKeyEvent* e) override;

   private:
    MEMUTILZ_DECLARE_PRIVATE();
};
