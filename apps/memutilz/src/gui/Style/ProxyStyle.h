#pragma once

#include <QProxyStyle>
#include <QStyleFactory>

// TODO: 2 - IDA-like scrollbar
// TODO: 2 - DockWidget TabBar
class ProxyStyle : public QProxyStyle {
   public:
    ProxyStyle() : QProxyStyle(QStyleFactory::create("Fusion")) {}

    virtual void drawPrimitive(PrimitiveElement element,
        const QStyleOption* option,
        QPainter* painter,
        const QWidget* widget = nullptr) const override;

    // void drawControl(ControlElement element, const QStyleOption* option,
    //                  QPainter* painter,
    //                  const QWidget* widget = nullptr) const override {
    //     if (element == CE_PushButton) {
    //         painter->save();
    //         painter->setBrush(Qt::red);
    //         painter->drawRect(option->rect);
    //         painter->restore();
    //         return;
    //     }
    //
    //     // fallback to normal Fusion behavior
    //     QProxyStyle::drawControl(element, option, painter, widget);
    // }
};
