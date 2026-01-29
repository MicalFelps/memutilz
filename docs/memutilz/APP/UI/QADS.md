### Main Components

1. CDockManager
	Owns everything, maanges layout, docking, floating, usually one per main window, or docking area
2. CDockWidget
	actual dockable content, basically like QDockWidget but better

The minimum you need to do is

```cpp
auto dock = new ads::CDockWidget("Foo");
dock->setWidget(myWidget);
dockManager->addDockWidget(ads::LeftDockWidgetArea, dock);
```

### Additional

Configuration flags, most important thing to set before creating the dockManager, they also exist for dockWidgets.

```cpp
CDockManager::setConfigFlags(CDockManager::DefaultOpaqueConfig);
CDockManager::setConfigFlag(CDockManager::RetainTabSizeWhenCloseButtonHidden, true);
...
d->DockManager = new CDockManager(this);
```

Existing flags are documented [here](https://githubuser0xffff.github.io/Qt-Advanced-Docking-System/doc/user-guide.html#configuration-flags)

---

### DockWidgetArea vs CDockAreaWidget 

DockWidgetArea is an ENUM:

```cpp
enum DockWidgetArea
{
	NoDockWidgetArea = 0x00,
	LeftDockWidgetArea = 0x01,
	RightDockWidgetArea = 0x02,
	TopDockWidgetArea = 0x04,
	BottomDockWidgetArea = 0x08,
	CenterDockWidgetArea = 0x10,
	LeftAutoHideArea = 0x20,
	RightAutoHideArea = 0x40,
	TopAutoHideArea = 0x80,
	BottomAutoHideArea = 0x100,

	InvalidDockWidgetArea = NoDockWidgetArea,
	OuterDockAreas = TopDockWidgetArea | LeftDockWidgetArea | RightDockWidgetArea | BottomDockWidgetArea,
	AutoHideDockAreas = LeftAutoHideArea | RightAutoHideArea | TopAutoHideArea | BottomAutoHideArea,
	AllDockAreas = OuterDockAreas | CenterDockWidgetArea
};
```

which tells dockManager where to place the new dock widget (left side, right side, top, bottom, center-as-tab, etc.)

Meanwhile CDockAreaWidget is an actual container widget that holds one or more tabbed CDockWidgets.

---

### Example documentation

```cpp
/**
 * Central docking area for the main application window.
 *
 * This class owns the ads::CDockManager instance and is responsible
 * for its lifetime.
 *
 * @note Dock widgets should be registered through this class only.
 * @warning This class assumes it lives for the duration of the application.
 */
class CentralDockingArea : public QWidget {

```

```cpp
#pragma once

#include <unordered_map>
#include <memory>
#include <string>

/**
 * @brief Owns and manages uniquely-owned resources indexed by string keys.
 *
 * ResourceCache provides non-owning access to stored resources while
 * retaining sole ownership internally. Resources can be added, queried,
 * and removed by key.
 *
 * @tparam T The resource type being stored.
 */
template <typename T>
class ResourceCache {
public:
    /**
     * @brief Inserts a resource into the cache.
     *
     * Ownership of the resource is transferred to the cache.
     * If a resource with the same key already exists, it will be replaced.
     *
     * @param key Unique identifier for the resource.
     * @param resource Resource to store; must not be null.
     */
    void insert(const std::string& key, std::unique_ptr<T> resource);

    /**
     * @brief Returns a non-owning pointer to a cached resource.
     *
     * The returned pointer is valid as long as the resource remains
     * in the cache and the cache itself is alive.
     *
     * @param key Resource identifier.
     * @return Pointer to the resource, or nullptr if not found.
     */
    T* get(const std::string& key) const;

    /**
     * @brief Removes a resource from the cache.
     *
     * @param key Resource identifier.
     * @return True if a resource was removed, false if it did not exist.
     */
    bool remove(const std::string& key);

    /**
     * @brief Clears all stored resources.
     */
    void clear();

private:
    /// Map storing owned resources by key
    std::unordered_map<std::string, std::unique_ptr<T>> _resources;
};
```