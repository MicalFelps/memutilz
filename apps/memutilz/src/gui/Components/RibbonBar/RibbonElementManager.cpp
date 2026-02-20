#include "RibbonElementManager.h"

RibbonElementManager::RibbonElementManager() {
    _factory.reset(new RibbonElementFactory);
}
RibbonElementManager::~RibbonElementManager() {}
RibbonElementManager* RibbonElementManager::instance() {
    static RibbonElementManager _instance;
    return &_instance;
}
RibbonElementFactory* RibbonElementManager::factory() { return _factory.get(); }
void RibbonElementManager::initFactory(RibbonElementFactory* factory) {
    _factory.reset(factory);
}
