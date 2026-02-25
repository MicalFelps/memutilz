#pragma once

#include <QObject>
#include <QPointer>

#include <Utils/Id.h>

#include <kddockwidgets/MainWindow.h>
#include <kddockwidgets/DockWidget.h>

/**
 * @brief DockWidget Identifier
 *
 * DockKeys with std::nullopt index don't have an index
 * pool associated with the dock widget type, so you can
 * only have a single dock widget instance of this type.
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
 * @brief Index pool for dock widgets that allow multiple
 * instances of a single widget type.
 */
class IndexPool;

// TODO: Make factory class for DockManager (with slots and signals for widget
// creation)

/**
 * @brief Interface for adding dock widgets to the central widget.
 *
 * DockManager is just a proxy for kddw's mainWindow. Instead of calling
 * addDockWidget directly, use this class' methods.
 */
class DockManager : public QObject {
    Q_OBJECT
    using DockWidget = KDDockWidgets::QtWidgets::DockWidget;

   public:
    explicit DockManager(
        KDDockWidgets::QtWidgets::MainWindow* parent = nullptr);
    virtual ~DockManager() override;

    QPointer<DockWidget> dockWidget(DockKey key) const;
    bool contains(DockKey key) const { return _docks.contains(key); }

    /**
     * You need to call setLimit before creating any dock widgets of a
     * certain type if you want to allow multiple instances for that type
     */
    void setLimit(Utils::Id type, int limit);

    void addDockWidgetAsTab(DockWidget* w, Utils::Id type);
    void addDockWidget(DockWidget* w, Utils::Id type,
                       KDDockWidgets::Location location,
                       std::optional<DockKey> relativeTo = std::nullopt,
                       const KDDockWidgets::InitialOption& initialOption = {});

   private:
    QHash<DockKey, QPointer<DockWidget>> _docks;
    QHash<Utils::Id, IndexPool> _indexPools;
};
