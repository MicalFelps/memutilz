Remember that IDs are primarily semantic identifiers. Viewing them as such, allows you to do something like this:

```cpp
struct DockKey {
    Utils::Id kind; // semantic identity, e.g., Dock.Text
    int index;      // runtime instance, e.g., 3

    bool operator==(const DockKey& other) const {
        return kind == other.kind && index == other.index;
    }
};

// Needed for QHash
inline uint qHash(const DockKey& k, uint seed = 0) {
    return qHash(k.kind, seed) ^ qHash(k.index, seed << 1);
}
```

This approach however requires that you have an index allocator that enforces per-widget-type limits, and creates suitable names for *instances*.

```cpp
class NamePool {
public:
    // Allocate the next available index for a given kind
    // Returns std::nullopt if limit is reached
    std::optional<int> acquire(const Utils::Id& kind) {
        auto& pool = _pools[kind];

        if (!pool.free.isEmpty()) {
            int idx = *pool.free.begin();
            pool.free.remove(idx);
            return idx;
        }

        if (pool.limit != -1 && pool.next > pool.limit)
            return std::nullopt; // limit reached

        return pool.next++;
    }

    // Release an index back to the pool
    void release(const Utils::Id& kind, int index) {
        auto& pool = _pools[kind];
        pool.free.insert(index);
        if (index < pool.next)
            pool.next = index;
    }

    // Set a limit for a kind
    void setLimit(const Utils::Id& kind, int limit) {
        _pools[kind].limit = limit;
    }

private:
    struct Pool {
        int next = 1;       // next available index
        QSet<int> free;     // freed/reusable indexes
        int limit = -1;     // -1 = no limit
    };

    QHash<Utils::Id, Pool> _pools;
};
```

The approach above allows for each Utils::Id to have it's own array of indexes, and since n is basically guaranteed to be less than 10, you have O(1) allocation / release (std::priority_queue would actually be worse here).

```cpp
class CentralDockingArea : public QWidget {
    Q_OBJECT

public:
    std::optional<DockKey> createDock(const Utils::Id& kind, const QString& title) {
        auto idxOpt = _pool.acquire(kind);
        if (!idxOpt)
            return std::nullopt; // limit reached

        int idx = *idxOpt;
        DockKey key{kind, idx};

        auto* dock = new ads::CDockWidget(title, this);
        _docks.insert(key, dock);

        // Release index automatically when destroyed
        connect(dock, &QObject::destroyed, this, [this, kind, idx]() {
            _docks.remove(DockKey{kind, idx});
            _pool.release(kind, idx);
        });

        return key;
    }

    QString dockName(const DockKey& key) const {
        return QString("%1.%2").arg(key.kind.toString()).arg(key.index);
    }

    void setLimit(const Utils::Id& kind, int limit) {
        _pool.setLimit(kind, limit);
    }

private:
    QHash<DockKey, QPointer<ads::CDockWidget>> _docks;
    NamePool _pool;
};
```

### Example Usage

```cpp
CentralDockingArea area;

// Set per-type limits
area.setLimit(Ids::Dock::Text, 5);
area.setLimit(Ids::Dock::Console, 2);

// Create a Text dock
auto keyOpt = area.createDock(Ids::Dock::Text, "Text Window");
if (keyOpt) {
    QString name = area.dockName(*keyOpt); // e.g. "Dock.Text.1"
}

// Create a Console dock
auto cKeyOpt = area.createDock(Ids::Dock::Console, "Console");
```