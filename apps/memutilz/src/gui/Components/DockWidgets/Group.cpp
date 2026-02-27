#include <QPainter>
#include <kddockwidgets/core/Group.h>
#include "Group.h"

Group::Group(KDDockWidgets::Core::Group* controller, QWidget* parent)
    : KDDockWidgets::QtWidgets::Group(controller, parent)
{}

void Group::paintEvent(QPaintEvent*) {
    if (!m_group->isFloating()) {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const qreal penWidth = 1;
        const qreal halfPenWidth = penWidth / 2;
        const QRectF rectf = QWidget::rect();

        const bool isOverlayed = m_group->isOverlayed();
        const QColor penColor = isOverlayed ? QColor(0x666666) : QColor(0x121212);
        const QColor brushColor = penColor;
        QPen pen(penColor);
        QBrush brush(brushColor);
        pen.setWidthF(penWidth);
        p.setPen(pen);
        p.setBrush(brush);

        if (isOverlayed) {
            pen.setJoinStyle(Qt::MiterJoin);
            p.drawRect(rectf.adjusted(halfPenWidth, penWidth, -halfPenWidth, -halfPenWidth));
        }
        else {
            p.drawRoundedRect(
                rectf.adjusted(halfPenWidth, halfPenWidth, -halfPenWidth, -halfPenWidth), 2, 2);
        }
    }
}