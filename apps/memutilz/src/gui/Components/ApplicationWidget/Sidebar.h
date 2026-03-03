#pragma once

#include <QFrame>
#include <QButtonGroup>
#include <QVBoxLayout>

#include "SidebarToolButton.h"

class Sidebar : public QFrame {
    Q_OBJECT
public:
    explicit Sidebar(QWidget* parent);
    virtual ~Sidebar() override;

    SidebarToolButton* button(QString id) const;
    void addButton(SidebarToolButton* btn, std::optional<int> pos = std::nullopt);

public slots:
    void select(SidebarToolButton* btn);

signals:
    void selectionChanged(SidebarToolButton* selected);

private:
    QButtonGroup* _group{ nullptr };
    QVBoxLayout* _layout{ nullptr };
    SidebarToolButton* _selected{ nullptr };
};