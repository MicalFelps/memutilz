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

// ###############################################

DummyScrollArea::DummyScrollArea(QWidget* parent)
    : QAbstractScrollArea(parent) {
    // Force scrollbars to always be visible
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Some huge virtual content size (you can make this dynamic later)
    const int virtualWidth = 3400;
    const int virtualHeight = 5200;

    // Tell Qt how big our "document" is
    horizontalScrollBar()->setRange(0, virtualWidth - viewport()->width());
    verticalScrollBar()->setRange(0, virtualHeight - viewport()->height());

    // Optional: make steps nicer
    horizontalScrollBar()->setSingleStep(60);
    verticalScrollBar()->setSingleStep(80);
    horizontalScrollBar()->setPageStep(viewport()->width() * 3 / 4);
    verticalScrollBar()->setPageStep(viewport()->height() * 3 / 4);

    // Prepare some long text lines for painting
    prepareDummyContent();
}

void DummyScrollArea::resizeEvent(QResizeEvent* event) {
    QAbstractScrollArea::resizeEvent(event);
    updateScrollBarRanges();
}

void DummyScrollArea::scrollContentsBy(int dx, int dy) {
    QAbstractScrollArea::scrollContentsBy(dx, dy);
    viewport()->update();  // most important line — repaint visible area
}

void DummyScrollArea::paintEvent(QPaintEvent* event) {
    QPainter painter(viewport());
    painter.fillRect(event->rect(), Qt::white);

    // Where is the top-left corner of the visible area in content
    // coordinates?
    int x = horizontalScrollBar()->value();
    int y = verticalScrollBar()->value();

    painter.translate(-x, -y);  // ← shift coordinate system

    // Draw background grid just to see scrolling clearly
    painter.setPen(QPen(Qt::lightGray, 1, Qt::DotLine));
    for (int ix = 0; ix < 4000; ix += 200) painter.drawLine(ix, 0, ix, 6000);
    for (int iy = 0; iy < 6000; iy += 200) painter.drawLine(0, iy, 4000, iy);

    // Draw long dummy text
    painter.setPen(Qt::darkBlue);
    QFont font = painter.font();
    font.setPointSize(14);
    painter.setFont(font);

    int lineHeight = QFontMetrics(font).height() + 4;

    for (int i = 0; i < m_lines.size(); ++i) {
        int yy = 60 + i * lineHeight;
        painter.drawText(40, yy, m_lines[i]);
    }

    // Some extra visual marker at bottom-right
    painter.setPen(Qt::red);
    painter.drawText(3000, 5000, "END — scrollbars should still be visible ↑←");
}

void DummyScrollArea::updateScrollBarRanges() {
    int virtualWidth = 3400;
    int virtualHeight = 5200;

    horizontalScrollBar()->setRange(
        0, qMax(0, virtualWidth - viewport()->width()));
    verticalScrollBar()->setRange(
        0, qMax(0, virtualHeight - viewport()->height()));
}

void DummyScrollArea::prepareDummyContent() {
    QString longChunk =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do "
        "eiusmod tempor incididunt "
        "ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis "
        "nostrud exercitation ullamco "
        "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure "
        "dolor in reprehenderit in "
        "voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
        "Excepteur sint occaecat cupidatat "
        "non proident, sunt in culpa qui officia deserunt mollit anim id "
        "est laborum.   →→→ 1234567890 "
        "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    m_lines.reserve(180);
    for (int i = 0; i < 140; ++i) {
        QString line = QString("%1   ").arg(i + 1, 4) + longChunk;
        if (i % 7 == 0) line += "     ★ PARAGRAPH BREAK ★";
        m_lines << line;
    }
}
