#include "RibbonElementFactory.h"

RibbonElementFactory::RibbonElementFactory() {}
RibbonElementFactory::~RibbonElementFactory() {}

SystemButtonBar* RibbonElementFactory::createSystemButtonBar(QWidget* parent) {
    return new SystemButtonBar(parent);
}
