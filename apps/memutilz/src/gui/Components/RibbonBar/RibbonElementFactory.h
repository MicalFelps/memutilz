#pragma once

#include <SARibbonElementFactory.h>
#include "SystemButtonBar.h"

/**
 * @brief Factory class for any ribbon element widget
 */
class RibbonElementFactory : public SARibbonElementFactory {
   public:
    RibbonElementFactory();
    virtual ~RibbonElementFactory();
    virtual SystemButtonBar* createSystemButtonBar(QWidget* parent);
};
