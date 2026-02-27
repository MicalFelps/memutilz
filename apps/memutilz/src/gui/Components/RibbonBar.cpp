#include <SARibbonCategory.h>
#include <SARibbonApplicationButton.h>
#include <SARibbonMenu.h>

#include "RibbonBar.h"
#include "../Constants/Ui.h"

using namespace Memutilz;

struct RibbonBar::Impl {
    Impl(RibbonBar* _public) : _this{_public} {}
    void setupUi();

   private:
    MEMUTILZ_DECLARE_PUBLIC(RibbonBar)

    void createApplicationButton();
    void createCategoryMain();
    void createCategoryView();
};

void RibbonBar::Impl::setupUi() {
    createApplicationButton();
    createCategoryMain();
    createCategoryView();
}

void RibbonBar::Impl::createApplicationButton() {
    QAbstractButton* btn = _this->applicationButton();
    if (!btn) {
        btn = new SARibbonApplicationButton(_this);
        _this->setApplicationButton(btn);
    }
    btn->setText(QString("&File"));
}
void RibbonBar::Impl::createCategoryMain() {
    SARibbonCategory* page = _this->addCategoryPage("Main");
    page->setObjectName("categoryMain");
    SARibbonPanel* panelFlowControl = page->addPanel("Flow Control");

    QAction* actionDebugPause =
        new QAction(QIcon(":/icons/debug-pause"), "Break", _this);
    actionDebugPause->setToolTip("");
    panelFlowControl->addLargeAction(actionDebugPause);

    SARibbonMenu* menuDebugContinue{new SARibbonMenu(_this)};
    {
        QAction* action = nullptr;
        QIcon icon = QIcon(":/icons/debug-continue-bugless");

        action = menuDebugContinue->addAction(icon, {"Go Unhandled Exception"});
        action->setObjectName({"Go Unhandled Exception"});

        action = menuDebugContinue->addAction(icon, {"Go Handled Exception"});
        action->setObjectName({"Go Handled Exception"});
    }

    QAction* actionDebugContinue =
        new QAction(QIcon(":/icons/debug-continue"), "Go", _this);
    actionDebugContinue->setToolTip("");
    actionDebugContinue->setMenu(menuDebugContinue);
    panelFlowControl->addLargeAction(actionDebugContinue,
                                     QToolButton::MenuButtonPopup);

    QAction* actionDebugStepOut =
        new QAction(QIcon(":/icons/debug-step-out"), "Step Out", _this);
    QAction* actionDebugStepInto =
        new QAction(QIcon(":/icons/debug-step-into"), "Step Into", _this);
    QAction* actionDebugStepOver =
        new QAction(QIcon(":/icons/debug-step-over"), "Step Over", _this);

    panelFlowControl->addSmallAction(actionDebugStepOut);
    panelFlowControl->addSmallAction(actionDebugStepInto);
    panelFlowControl->addSmallAction(actionDebugStepOver);

    // End Panel

    SARibbonPanel* panelEnd = page->addPanel({"End"});

    QAction* actionDebugRestart =
        new QAction(QIcon(":/icons/debug-restart"), "Restart", _this);
    QAction* actionDebugStop =
        new QAction(QIcon(":/icons/debug-stop"), "Stop", _this);
    QAction* actionDebugDetach =
        new QAction(QIcon(":/icons/debug-detach"), "Detach", _this);

    panelEnd->addSmallAction(actionDebugRestart);
    panelEnd->addSmallAction(actionDebugStop);
    panelEnd->addSmallAction(actionDebugDetach);
}
void RibbonBar::Impl::createCategoryView() {
    SARibbonCategory* page = _this->addCategoryPage("View");
    page->setCategoryName("View");
    SARibbonPanel* panelWindows = page->addPanel("Windows");

    QAction* actionCommandWindow =
        new QAction(QIcon(":/icons/console"), "Command", _this);
    panelWindows->addLargeAction(actionCommandWindow);

    QAction* actionDisassemblyWindow =
        new QAction(QIcon(":/icons/file-asm"), "Disassembly", _this);
    panelWindows->addLargeAction(actionDisassemblyWindow);

    SARibbonMenu* menuMemoryWindow{new SARibbonMenu(_this)};
    {
        QIcon icon = QIcon(":/icons/file-binary");

        for (int i = 0; i < Constants::Ui::MaxMemoryWindowWidgets; ++i) {
            menuMemoryWindow->addAction(icon, QString("Memory %1").arg(i));
        }
    }
    QAction* actionMemoryWindow =
        new QAction(QIcon(":/icons/file-binary"), "Memory", _this);
    actionMemoryWindow->setMenu(menuMemoryWindow);
    panelWindows->addLargeAction(actionMemoryWindow);
}

// ###########################################
// RibbonBar
// ###########################################

RibbonBar::RibbonBar(QWidget* parent)
    : SARibbonBar(parent), d{std::make_unique<RibbonBar::Impl>(this)} {
    d->setupUi();

    connect(applicationButton(), &QAbstractButton::clicked, this, [](bool c) {
        Q_UNUSED(c);
        qDebug() << "ApplicationButton clicked!";
    });
}
RibbonBar::~RibbonBar() {}
