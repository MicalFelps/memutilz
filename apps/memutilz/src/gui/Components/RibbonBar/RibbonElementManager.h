#pragma once

#include "RibbonElementFactory.h"

/**
 * @brief Singleton wrapper around RibbonElementFactory
 */
class RibbonElementManager {
   protected:
    RibbonElementManager();

   public:
    virtual ~RibbonElementManager();
    static RibbonElementManager* instance();
    RibbonElementFactory* factory();
    void initFactory(RibbonElementFactory* factory);

   private:
    std::unique_ptr<RibbonElementFactory> _factory;
};

#ifndef RibbonSubElementManager
#define RibbonSubElementManager RibbonElementManager::instance()
#endif
#ifndef RibbonSubElementFactory
#define RibbonSubElementFactory RibbonElementManager::instance()->factory()
#endif
