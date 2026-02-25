#include "DummyWidget.h"
#include <QPainter>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QDebug>

DummyWidget::DummyWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(140, 90);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setAttribute(Qt::WA_Hover, true);
}

DummyWidget::DummyWidget(const QString& label, const QColor& bg,
                         QWidget* parent)
    : DummyWidget(parent) {
    m_label = label;
    m_bgColor = bg;
}

void DummyWidget::setLabel(const QString& text) {
    if (m_label == text) return;
    m_label = text;
    update();
    emit labelChanged(text);
}

void DummyWidget::setBackgroundColor(const QColor& color) {
    if (m_bgColor == color) return;
    m_bgColor = color;
    update();
    emit backgroundColorChanged(color);
}

void DummyWidget::setShowBorder(bool show) {
    if (m_showBorder == show) return;
    m_showBorder = show;
    update();
    emit showBorderChanged(show);
}

void DummyWidget::setBorderWidth(int width) {
    width = qBound(0, width, 12);
    if (m_borderWidth == width) return;
    m_borderWidth = width;
    update();
    emit borderWidthChanged(width);
}

// ────────────────────────────────────────────────
//            Static convenience creators
// ────────────────────────────────────────────────

DummyWidget* DummyWidget::createRed(const QString& text, QWidget* parent) {
    auto w = new DummyWidget(text, QColor(230, 50, 60), parent);
    w->setMinimumSize(160, 100);
    return w;
}

DummyWidget* DummyWidget::createGreen(const QString& text, QWidget* parent) {
    auto w = new DummyWidget(text, QColor(60, 180, 90), parent);
    w->setMinimumSize(140, 80);
    return w;
}

DummyWidget* DummyWidget::createBlue(const QString& text, QWidget* parent) {
    auto w = new DummyWidget(text, QColor(50, 120, 220), parent);
    w->setMinimumSize(180, 110);
    return w;
}

// ────────────────────────────────────────────────
//                     Painting
// ────────────────────────────────────────────────

void DummyWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Background
    p.fillRect(rect(), m_bgColor);

    // Border
    if (m_showBorder && m_borderWidth > 0) {
        QPen pen(Qt::black, m_borderWidth);
        pen.setJoinStyle(Qt::MiterJoin);
        p.setPen(pen);
        p.drawRect(rect().adjusted(m_borderWidth / 2, m_borderWidth / 2,
                                   -m_borderWidth / 2, -m_borderWidth / 2));
    }

    // Text
    p.setPen(Qt::white);
    if (m_bgColor.lightness() > 180) p.setPen(Qt::black);

    QFont font = p.font();
    font.setBold(true);
    font.setPointSizeF(font.pointSizeF() * 1.3);
    p.setFont(font);

    QFontMetrics fm(font);
    QString displayText = m_label;

    // Optional click counter
    if (m_clickCount > 0) displayText += QString(" ×%1").arg(m_clickCount);

    p.drawText(rect(), Qt::AlignCenter | Qt::TextWordWrap, displayText);

    // Tiny debug info in corner
    font.setPointSizeF(font.pointSizeF() * 0.55);
    font.setBold(false);
    p.setFont(font);
    p.setOpacity(0.65);
    p.drawText(rect().adjusted(6, 6, -6, -6), Qt::AlignBottom | Qt::AlignRight,
               QString("%1 × %2").arg(width()).arg(height()));
}

// ────────────────────────────────────────────────
//                     Interaction
// ────────────────────────────────────────────────

void DummyWidget::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_clickCount++;
        update();

        qDebug() << "DummyWidget clicked @" << e->pos() << "size:" << size()
                 << "click #" << m_clickCount;

        emit clicked();  // you could add this signal if desired
    }
    QWidget::mousePressEvent(e);
}

void DummyWidget::mouseDoubleClickEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        qDebug() << "Double-click → sizeHint() =" << sizeHint();
        // Example: toggle border
        setShowBorder(!m_showBorder);
    }
    QWidget::mouseDoubleClickEvent(e);
}

void DummyWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    update();  // make sure size text updates
}
