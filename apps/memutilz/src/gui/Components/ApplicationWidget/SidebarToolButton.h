#pragma once

#include <QToolButton>

class SidebarToolButton : public QToolButton {
    Q_OBJECT
public:
    explicit SidebarToolButton(QString id, const QIcon& icon, const QString& text, QWidget* parent = nullptr);

    const QString& id() { return m_id; }

    bool operator==(const QString& id) const { return id == m_id; }
    bool operator!=(const QString& id) const { return id != m_id; }
    bool operator==(const SidebarToolButton& other) const { return m_id == other.m_id; }
    bool operator!=(const SidebarToolButton& other) const { return m_id != other.m_id; }

    friend size_t qHash(const SidebarToolButton& btn, size_t seed = 0) noexcept {
        return QT_PREPEND_NAMESPACE(qHash)(btn.m_id, seed);
    }
private:
    explicit SidebarToolButton(QString id, QWidget* parent = nullptr);

    QString m_id;
};