#pragma once

#include <QFrame>
#include <QVBoxLayout>
#include <QButtonGroup>

#include <Utils/Id.h>

#include "SidebarToolButton.h"

class Sidebar : public QFrame {
    Q_OBJECT
public:
    explicit Sidebar(QWidget* parent);
    virtual ~Sidebar() override;

    void addButton(SidebarToolButton* btn);

public slots:
    void select(Utils::Id id);

signals:
    void selectionChanged(Utils::Id id);

private:
    QButtonGroup* _group{ nullptr };
    QVBoxLayout* _layout{ nullptr };
    SidebarToolButton* _selected{ nullptr };
};