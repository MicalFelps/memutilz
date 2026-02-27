#include "ProxyStyle.h"

void ProxyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
    if (element == QStyle::PE_PanelButtonTool) return;
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}