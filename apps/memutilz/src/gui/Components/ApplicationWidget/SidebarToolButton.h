#pragma once

#include <QToolButton>
#include <Utils/Id.h>

class SidebarToolButton : public QToolButton {
    Q_OBJECT
public:
    explicit SidebarToolButton(Utils::Id id, QWidget* parent = nullptr);
    explicit SidebarToolButton(Utils::Id id, const QIcon& icon, const QString& text, QWidget* parent = nullptr);

    QColor accentColor() const { return _accentColor; }
    void setAccentColor(const QColor& color) { _accentColor = color; update(); }

    Utils::Id id() const { return m_id; }

private:
    Utils::Id m_id;
    QColor _accentColor{ "#FFFFFF" };
};