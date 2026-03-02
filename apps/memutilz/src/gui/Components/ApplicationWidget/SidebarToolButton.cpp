#include <QPainter>
#include "SidebarToolButton.h"

SidebarToolButton::SidebarToolButton(Utils::Id id, QWidget* parent)
    : QToolButton(parent)
    , m_id{ id }
{
    setAttribute(Qt::WA_StyledBackground, true);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setCheckable(true);
    
    setIconSize(QSize(40, 40));

    setFixedHeight(50);
    setMinimumWidth(250);
}
SidebarToolButton::SidebarToolButton(Utils::Id id, const QIcon& icon, const QString& text, QWidget* parent)
    : SidebarToolButton(id, parent) {
    setText(text);
    if (!icon.isNull())
        setIcon(icon);
}