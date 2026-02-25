#include "DockManager.h"

class IndexPool {
   public:
    IndexPool() = default;
    explicit IndexPool(int limit) : _limit{limit > 2 ? limit : 2} {
        for (int i = 1; i <= _limit; ++i) {
            _allAvailable.insert(i);
        }
        _nextAvailable = 1;
    }

    QSet<int> allAvailable() const { return _allAvailable; }
    std::optional<int> nextAvailable() const { return _nextAvailable; }
    int limit() const { return _limit; }

    bool isAvailable(int index) const { return _allAvailable.contains(index); }

    std::optional<int> acquire() {
        std::optional<int> res = _nextAvailable;

        if (_nextAvailable.has_value()) {
            _allAvailable.remove(*_nextAvailable);
        }

        if (_allAvailable.isEmpty()) {
            _nextAvailable = std::nullopt;
            return res;
        }

        int next = std::numeric_limits<int>::max();
        for (auto i : _allAvailable) {
            if (next > i) next = i;
        }

        _nextAvailable = next;
        return res;
    }

    void release(int index) {
        if (index <= 0 || index > _limit) return;
        _allAvailable.insert(index);
        if (index < _nextAvailable || !_nextAvailable.has_value())
            _nextAvailable = index;
    }

   private:
    QSet<int> _allAvailable;
    std::optional<int> _nextAvailable;
    int _limit;
};

DockManager::DockManager(KDDockWidgets::QtWidgets::MainWindow* parent)
    : QObject(parent) {}
DockManager::~DockManager() {}

QPointer<KDDockWidgets::QtWidgets::DockWidget> DockManager::dockWidget(
    DockKey key) const {
    return _docks.contains(key) ? _docks[key] : nullptr;
}

void DockManager::setLimit(Utils::Id type, int limit) {
    if (_indexPools.contains(type)) return;
    _indexPools[type] = IndexPool(limit);
}

void DockManager::addDockWidgetAsTab(DockWidget* w, Utils::Id type) {
    if (_indexPools.contains(type)) {
        auto& pool = _indexPools[type];
        auto nextId = pool.acquire();
        if (nextId.has_value()) {
            DockKey key{type};
            _docks[key] = w;

            auto* mw =
                qobject_cast<KDDockWidgets::QtWidgets::MainWindow*>(parent());
            if (mw) {
                mw->addDockWidgetAsTab(w);
            }
        }
    } else {
        if (!_docks.contains(type)) {
            DockKey key{type};
            _docks[key] = w;

            auto* mw =
                qobject_cast<KDDockWidgets::QtWidgets::MainWindow*>(parent());
            if (mw) {
                mw->addDockWidgetAsTab(w);
            }
        }
    }
}
void DockManager::addDockWidget(
    DockWidget* w, Utils::Id type, KDDockWidgets::Location location,
    std::optional<DockKey> relativeTo,
    const KDDockWidgets::InitialOption& initialOption) {
    if (_indexPools.contains(type)) {
        auto& pool = _indexPools[type];
        auto nextId = pool.acquire();
        if (nextId.has_value()) {
            DockKey key{type};
            _docks[key] = w;

            auto* mw =
                qobject_cast<KDDockWidgets::QtWidgets::MainWindow*>(parent());
            if (mw) {
                if (relativeTo.has_value() && _docks.contains(*relativeTo)) {
                    mw->addDockWidget(w, location, _docks[*relativeTo],
                                      initialOption);
                } else {
                    mw->addDockWidget(w, location, nullptr, initialOption);
                }
            }
        }
    } else {
        if (!_docks.contains(type)) {
            DockKey key{type};
            _docks[key] = w;

            auto* mw =
                qobject_cast<KDDockWidgets::QtWidgets::MainWindow*>(parent());
            if (mw) {
                if (relativeTo.has_value() && _docks.contains(*relativeTo)) {
                    mw->addDockWidget(w, location, _docks[*relativeTo],
                                      initialOption);
                } else {
                    mw->addDockWidget(w, location, nullptr, initialOption);
                }
            }
        }
    }
}
