#include "SidePanel/PanelLeftSide.h"

#include "SidePanel/SidePanelHelpers.h"

PanelLeftSide::PanelLeftSide(QWidget* parent)
    : SidePanel(parent)
{
    getOpenedRect = [this](const QRect& parentRect) -> QRect {
        return q_sp::rect_opened_left(getPanelSize(), parentRect);
    };

    getClosedRect = [this](const QRect& parentRect) -> QRect {
        QRect r = q_sp::rect_closed_left(getPanelSize(), parentRect);
        r.setWidth(r.width() + getPanelSizeMin());
        return r;
    };

    alignedHandlerRect = [](const QRect& panelGeom, const QSize& handlerSize, qreal) -> QRect {
        return q_sp::rect_aligned_right_center(panelGeom, handlerSize);
    };

    initialHandlerSize = []() -> QSize { return { 25, 50 }; };
    
    updateHandler = [](const SidePanelState state, HandlerWidgetT* handler) {
        switch (state) {
        case SidePanelState::Opening: { handler->setText("<"); } break;
        case SidePanelState::Opened: { handler->setText("<"); } break;
        case SidePanelState::Closing: { handler->setText(">"); } break;
        case SidePanelState::Closed: { handler->setText(">"); } break;
        default: break;
        }
    };
}

PanelLeftSide::~PanelLeftSide()
{
}