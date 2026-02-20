#pragma once

#include <QPointer>

#include "../Utils/Id.h"

/**
 * @brief CDockWidget Identifier
 *
 * DockKeys with std::nullopt index don't have an index
 * pool associated with the type, so you can only have a
 * single instance of the type.
 */
struct DockKey {
    DockKey() = default;
    DockKey(Utils::Id dockType, std::optional<int> dockIndex = std::nullopt)
        : type{dockType}, index{dockIndex} {}

    Utils::Id type;
    std::optional<int> index;

    bool operator==(const DockKey& other) const {
        return type == other.type && index == other.index;
    }
};
inline size_t qHash(const DockKey& k, size_t seed = 0) {
    seed = qHash(k.type, seed);

    if (k.index.has_value()) {
        seed = qHash(*k.index, seed);
    } else {
        seed = qHash(0x9e3779b9, seed);
    }

    return seed;
}

/**
 * @brief Index pool for dock widgets that allow multiple instances
 *
 * The limit needs to be at least 2, or else there's not really
 * any point in tracking which indexes are available and which aren't
 */
class IndexPool;

class DockManager {};

/**
 * @brief Owns and manages all visible dock widgets
 *
 * CentralDockingArea is a wrapper around dockManager that
 * manages the ownership of all registered dock widgets. Closing
 * a dock widget either hides it or deletes it entirely depending
 * on whether DockWidgetDeleteOnClose is set for that CDockWidget.
 * This lets the caller decide which widgets are considered cheap, and
 * which are considered expensive (and thus not worth deleting).
 * If an expensive dock widget remains hidden for a long time
 * it could become a memory leak, but this is very very unlikely.
 */
class CentralDockingArea : public QObject {
    Q_OBJECT

   public:
    CentralDockingArea() = delete;
    explicit CentralDockingArea(QObject* parent = nullptr,
                                ads::CDockManager* dockManager = nullptr);
    virtual ~CentralDockingArea() override;

    QPointer<ads::CDockManager> dockManager() const { return _dockManager; }

    /**
     * @return You need to specify the index for types that have an index pool
     * or you get nullptr
     */
    QPointer<ads::CDockWidget> dockWidget(DockKey key) const;
    QPointer<ads::CDockWidget> dockWidget(Utils::Id type) const {
        return dockWidget(DockKey{type});
    }

    bool contains(DockKey key) const { return _docks.contains(key); }
    bool contains(Utils::Id type) const { return contains(DockKey{type}); }
    std::optional<DockKey> keyFromDock(ads::CDockWidget* dockWidget) const;

    /**
     * You need to call setLimit before creating any widgets if you want
     * to allow multiple instances for that type
     */
    void setLimit(Utils::Id type, int limit);

    ads::CDockAreaWidget* addDockWidgetOrShow(
        ads::DockWidgetArea area, Utils::Id type, ads::CDockWidget* dockWidget,
        ads::CDockAreaWidget* dockAreaWidget = nullptr);

    ads::CDockAreaWidget* addDockWidgetFactoryOrShow(
        ads::DockWidgetArea area, Utils::Id type,
        ads::CDockAreaWidget* dockAreaWidget = nullptr);

    ads::CDockAreaWidget* showDock(DockKey key, bool bringToFront = true);
    void hideDock(DockKey key);
    void toggleDock(DockKey key);
    void activateDock(DockKey key);  // show + raise + setFocus

    ads::CDockAreaWidget* showDock(Utils::Id type, bool bringToFront = true) {
        return showDock(DockKey{type, std::nullopt}, bringToFront);
    }
    void hideDock(Utils::Id type) { hideDock(DockKey{type, std::nullopt}); }
    void toggleDock(Utils::Id type) { toggleDock(DockKey{type, std::nullopt}); }
    void activateDock(Utils::Id type) {
        activateDock(DockKey{type, std::nullopt});
    }

    bool remove(DockKey key);
    void removeAll();

    void toggleLockLayout();
    bool isLayoutLocked() const { return _layoutLocked; }

    void registerFactory(Utils::Id type,
                         std::function<ads::CDockWidget*()> factory);
   private slots:
    void onDockDestroyed(QObject* obj);

   private:
    QPointer<ads::CDockManager> _dockManager;
    QHash<DockKey, QPointer<ads::CDockWidget>> _docks;
    QHash<ads::CDockWidget*, DockKey> _keys;

    QHash<Utils::Id, std::function<ads::CDockWidget*()>> _factories;
    QHash<Utils::Id, IndexPool> _indexPools;

    bool _layoutLocked{false};
};
