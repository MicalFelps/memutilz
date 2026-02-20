#pragma once

#include <QFrame>
#include <QToolButton>

#include "../../../Globals.h"

// TODO: Handle i3 on X11 and other tiling WMs

/**
 * @brief The MainWindow's system buttons (minimize, maximize, and close)
 *
 * It uses an event filter to capture events from the MainWindow.
 * It handles MainWindow events through the event filter to prevent layout
 * issues caused by incorrectly overriding resizeEvent, which leads to an
 * incorrect positioning of the system button bar.
 *
 * The MainWindow class installs the event filter like so:
 *
 * @code
 * MainWindow::MainWindow() {
 *      InstallEventFilter(_systemButtonBar)
 * }
 * @endcode
 *
 */
class SystemButtonBar : public QFrame {
    Q_OBJECT
    MEMUTILZ_DECLARE_PRIVATE()

   public:
    explicit SystemButtonBar(QWidget* parent = nullptr);
    explicit SystemButtonBar(QWidget* parent, Qt::WindowFlags flags);
    virtual ~SystemButtonBar() override;
    void setupMinimizeButton(bool on);
    void setupMaximizeButton(bool on);
    void setupCloseButton(bool on);
    QAbstractButton* minimizeButton() const;
    QAbstractButton* maximizeButton() const;
    QAbstractButton* closeButton() const;

    void updateWindowFlags();
    Qt::WindowFlags windowFlags() const;
    void setupSystemButtons(Qt::WindowFlags flags);
    void updateSystemButtonStates(Qt::WindowStates states);

    int windowTitleHeight() const;
    void setWindowTitleHeight(int h);

    int windowButtonWidth() const;
    void setWindowButtonWidth(int w);

    void setButtonWidthStretch(int close = 2, int max = 2, int min = 2);

    virtual QSize sizeHint() const override;

    QSize iconSize() const;
    void setIconSize(const QSize& size);

    void addAction(QAction* a);
    void addMenuAction(
        QAction* menuAction,
        QToolButton::ToolButtonPopupMode popupMode = QToolButton::InstantPopup);
    QAction* addMenuAction(
        QMenu* menu,
        QToolButton::ToolButtonPopupMode popupMode = QToolButton::InstantPopup);
    QAction* addSeparator();
    QAction* addWidget(QWidget* w);

   protected:
    virtual bool eventFilter(QObject* obj, QEvent* event) override;
    virtual void resizeEvent(QResizeEvent* e) override;

   private slots:
    void closeWindow();
    void minimizeWindow();
    void maximizeWindow();
};

/**
 * @brief Class used by SystemButtonBar
 */
class SystemToolButton : public QToolButton {
    Q_OBJECT
   public:
    SystemToolButton(QWidget* parent = nullptr);
};
