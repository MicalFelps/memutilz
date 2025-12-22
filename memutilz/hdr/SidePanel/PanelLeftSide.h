#ifndef PANELLEFTSIDE_H
#define PANELLEFTSIDE_H

#include "SidePanel/SidePanel.h"

class PanelLeftSide : public SidePanel {
    Q_OBJECT
public:
    explicit PanelLeftSide(QWidget *parent = nullptr);
    virtual ~PanelLeftSide() override;
};

#endif // PANELLEFTSIDE_H
