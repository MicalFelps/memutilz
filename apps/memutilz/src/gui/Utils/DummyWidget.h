#pragma once

#include <QWidget>
#include <QColor>
#include <QString>

class DummyWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    Q_PROPERTY(QColor bgColor READ backgroundColor WRITE setBackgroundColor
                   NOTIFY backgroundColorChanged)
    Q_PROPERTY(bool showBorder READ showBorder WRITE setShowBorder NOTIFY
                   showBorderChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY
                   borderWidthChanged)

   public:
    explicit DummyWidget(QWidget* parent = nullptr);
    explicit DummyWidget(const QString& label, const QColor& bg = Qt::red,
                         QWidget* parent = nullptr);

    QString label() const { return m_label; }
    QColor backgroundColor() const { return m_bgColor; }
    bool showBorder() const { return m_showBorder; }
    int borderWidth() const { return m_borderWidth; }

   public slots:
    void setLabel(const QString& text);
    void setBackgroundColor(const QColor& color);
    void setShowBorder(bool show);
    void setBorderWidth(int width);

    // Quick factory-style helpers
    static DummyWidget* createRed(const QString& text = "Dummy",
                                  QWidget* parent = nullptr);
    static DummyWidget* createGreen(const QString& text = "OK",
                                    QWidget* parent = nullptr);
    static DummyWidget* createBlue(const QString& text = "Drop here",
                                   QWidget* parent = nullptr);

   signals:
    void labelChanged(const QString&);
    void backgroundColorChanged(const QColor&);
    void showBorderChanged(bool);
    void borderWidthChanged(int);
    void clicked();

   protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

   private:
    QString m_label = "Dummy";
    QColor m_bgColor = QColor(220, 40, 40);  // nice saturated red
    bool m_showBorder = true;
    int m_borderWidth = 2;
    int m_clickCount = 0;
};
