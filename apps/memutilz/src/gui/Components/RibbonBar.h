#pragma once

#include <SARibbonBar.h>
#include "../../Globals.h"

class RibbonBar : public SARibbonBar {
    Q_OBJECT
   public:
    explicit RibbonBar(QWidget* parent = nullptr);
    virtual ~RibbonBar() override;

   private:
    MEMUTILZ_DECLARE_PRIVATE()
};
