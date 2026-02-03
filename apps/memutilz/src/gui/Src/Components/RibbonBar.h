#pragma once

#include <SARibbonBar.h>
#include <SARibbonCategory.h>
#include <SARibbonActionsManager.h>

class RibbonBar : public QWidget {
    Q_OBJECT
public:
    explicit RibbonBar(SARibbonBar* ribbon, QWidget* parent = nullptr);
    virtual ~RibbonBar() = default;

signals:
    void applicationButtonClicked();
    void createTextWidget();
private:
    void createRibbonApplicationButton();

    void createCategoryMain(SARibbonCategory* page);
    void createCategoryView(SARibbonCategory* page);

    //void createQuickAccessBar();
    //void createRightButtonGroup();
    //void createWindowButtonGroupBar();

    // void createActionManager();

    QAction* createAction(const QString& text, const QString& iconurl, const QString& objName);
    QAction* createAction(const QString& text, const QString& iconurl);

    SARibbonBar* _ribbon{ nullptr };
    SARibbonActionsManager* actionsManager{ nullptr };
};