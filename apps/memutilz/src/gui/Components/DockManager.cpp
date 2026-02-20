#include "CentralDockingArea.h"

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

CentralDockingArea::CentralDockingArea(QObject* parent,
                                       ads::CDockManager* dockManager)
    : QObject(parent), _dockManager{dockManager} {}
CentralDockingArea::~CentralDockingArea() { removeAll(); }

QPointer<ads::CDockWidget> CentralDockingArea::dockWidget(DockKey key) const {
    if (_docks.contains(key))
        return _docks[key];
    else
        return nullptr;
}
std::optional<DockKey> CentralDockingArea::keyFromDock(
    ads::CDockWidget* dockWidget) const {
    if (_keys.contains(dockWidget))
        return _keys[dockWidget];
    else
        return std::nullopt;
}

void CentralDockingArea::setLimit(Utils::Id type, int limit) {
    if (_indexPools.contains(type)) return;
    _indexPools[type] = IndexPool(limit);
}

// TODO: U-Test
ads::CDockAreaWidget* CentralDockingArea::addDockWidgetOrShow(
    ads::DockWidgetArea area, Utils::Id type, ads::CDockWidget* dockWidget,
    ads::CDockAreaWidget* dockAreaWidget) {
    if (_indexPools.contains(type)) {
        auto& pool = _indexPools[type];  // copy instead of move :/
        auto nextId = pool.acquire();
        if (nextId.has_value()) {
            DockKey key{type, *nextId};
            _docks[key] = dockWidget;
            _keys[dockWidget] = key;

            connect(dockWidget, &QObject::destroyed, this,
                    &CentralDockingArea::onDockDestroyed);
            return _dockManager->addDockWidget(area, dockWidget,
                                               dockAreaWidget);
        } else {
            int limit = pool.limit();
            for (int index = 1; index < limit; ++index) {
                if (!_docks[DockKey{type, index}]->isVisible()) {
                    return showDock(DockKey{type, index}, true);
                    break;
                }
            }
            return showDock(DockKey{type, limit});
        }
    } else {
        if (contains(type)) {
            return showDock(type, true);
        }  // doesn't exist at all
        else {
            DockKey key{type, std::nullopt};
            _docks[key] = dockWidget;
            _keys[dockWidget] = key;

            connect(dockWidget, &QObject::destroyed, this,
                    &CentralDockingArea::onDockDestroyed);
            return _dockManager->addDockWidget(area, dockWidget,
                                               dockAreaWidget);
        }
    }
}

// TODO: U-Test
ads::CDockAreaWidget* CentralDockingArea::addDockWidgetFactoryOrShow(
    ads::DockWidgetArea area, Utils::Id type,
    ads::CDockAreaWidget* dockAreaWidget) {
    if (!_factories.contains(type)) return nullptr;
    auto w = _factories[type]();
    return addDockWidgetOrShow(area, type, w, dockAreaWidget);
}

ads::CDockAreaWidget* CentralDockingArea::showDock(DockKey key,
                                                   bool bringToFront) {
    auto w = dockWidget(key);
    if (!w) return nullptr;

    w->toggleView(true);
    if (bringToFront) {
        w->raise();
        w->setFocus();
    }
    return w->dockAreaWidget();
}
void CentralDockingArea::hideDock(DockKey key) {
    auto w = dockWidget(key);
    if (!w) return;

    w->toggleView(false);
}
void CentralDockingArea::toggleDock(DockKey key) {
    auto w = dockWidget(key);
    if (!w) return;

    w->toggleView(!w->isVisible());
}
void CentralDockingArea::activateDock(DockKey key) {
    auto w = dockWidget(key);
    if (!w) return;

    w->toggleView(true);
    w->raise();
    w->setFocus();
}

bool CentralDockingArea::remove(DockKey key) {
    auto w = dockWidget(key);
    if (!w) return false;

    _dockManager->removeDockWidget(w);

    !w->features().testFlag(ads::CDockWidget::DockWidgetDeleteOnClose)
        ? w->deleteLater()
        : w->closeDockWidget();

    _docks.remove(key);
    _keys.remove(w);

    if (_indexPools.contains(key.type)) {
        auto& pool = _indexPools[key.type];
        pool.release(*key.index);
    }

    return true;
}
void CentralDockingArea::removeAll() {
    auto keys = _docks.keys();
    for (auto key : keys) {
        remove(key);
    }
}

void CentralDockingArea::toggleLockLayout() {
    _layoutLocked = !_layoutLocked;
    _layoutLocked ? _dockManager->lockDockWidgetFeaturesGlobally(
                        ads::CDockWidget::NoDockWidgetFeatures)
                  : _dockManager->lockDockWidgetFeaturesGlobally(
                        ads::CDockWidget::DefaultDockWidgetFeatures);
}

void CentralDockingArea::registerFactory(
    Utils::Id type, std::function<ads::CDockWidget*()> factory) {
    _factories[type] = factory;
}

// ------------------------------------------------------------------

void CentralDockingArea::onDockDestroyed(QObject* obj) {
    auto* destroyedWidget = qobject_cast<ads::CDockWidget*>(obj);
    if (!destroyedWidget) return;

    DockKey key = _keys[destroyedWidget];
    _docks.remove(key);
    _keys.remove(destroyedWidget);

    if (_indexPools.contains(key.type)) {
        auto& pool = _indexPools[key.type];
        pool.release(*key.index);
    }
}
