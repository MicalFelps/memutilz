#include "RibbonElementFactory.h"
#include "../RibbonBar.h"

RibbonElementFactory::RibbonElementFactory() {}
RibbonElementFactory::~RibbonElementFactory() {}

SystemButtonBar* RibbonElementFactory::createSystemButtonBar(QWidget* parent) {
    return new SystemButtonBar(parent);
}