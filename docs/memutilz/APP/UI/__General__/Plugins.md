### WidgetFactory for Plugins

Instead of a massive case statement

```cpp
QWidget* WidgetFactory::create(WidgetType type) {
    if (type == WidgetType::TextEditor) return new TextEditorWidget();
    if (type == WidgetType::ImageViewer) return new ImageViewerWidget();
    // ...
}
```

We use a map of factory functions:

```cpp
#include <functional>
#include <QMap>
#include <QString>

class WidgetFactory {
public:
    using FactoryFunc = std::function<QWidget*()>;

    static void registerWidget(const QString& type, FactoryFunc func) {
        getRegistry()[type] = func;
    }

    static QWidget* create(const QString& type) {
        auto it = getRegistry().find(type);
        if (it != getRegistry().end())
            return it.value()();
        return nullptr;
    }

private:
    static QMap<QString, FactoryFunc>& getRegistry() {
        static QMap<QString, FactoryFunc> registry;
        return registry;
    }
};
```

Imagine your plugin provides a new widget type ChartWidget, it would register that widget like so:

```cpp
extern "C" void registerPluginWidgets() {
    WidgetFactory::registerWidget("Chart", [](){ return new ChartWidget(); });
}
```

Your main program then loads the plugin via `QLibrary` or `QPluginLoader`, calls `registerPluginWidgets()` and finally `WidgetFactory::create("Chart")`.

Also, you might want to consider a base plugin interface class similar to the one below if you plan on making your plugin support large scale

```cpp
class IWidgetPlugin {
public:
    virtual ~IWidgetPlugin() {}
    virtual void registerWidgets() = 0;
};
```

Each plugin implements this interface, the main app loads plugin via `QPluginLoader`, calls `registerWidgets()`, and each plugin registers the widgets it provides.

```cpp
IWidgetPlugin* plugin = qobject_cast<IWidgetPlugin*>(loader.instance());
plugin->registerWidgets();
```

---
### Plugins Support

For plugins to integrate with your app, they need to be aware of certain UI elements, such as `Utils::Id` and `IWidgetPlugin`, `WidgetFactory` so you need to separate your main app logic into app + library. You can just call the main library "memutilz".

Plugins are just shared libraries, the main app scans a directory at runtime, loads each library dynamically, queries it for known interfaces (i.e. `IWidgetPlugin*`) and calls the plugin's registration or init methods.

QT facilitates this with `QPluginLoader`:

```cpp
QPluginLoader loader("path/to/plugin.dll");
QObject* pluginInstance = loader.instance();
if (pluginInstance) {
    auto widgetPlugin = qobject_cast<IWidgetPlugin*>(pluginInstance);
    if (widgetPlugin) {
        widgetPlugin->registerWidgets();
    }
}
```

The plugin library must implement a Qt interface with `Q_INTERFACES(IWidgetPlugin)`, and use `Q_PLUGIN_METADATA` to give it a unique identifier.

The way they interact with the main program itself is through an interface, everything happens through an interface.

```cpp
class IAppInterface {
public:
    virtual ~IAppInterface() {}
    virtual void addAction(QAction* action, const QString& menuName) = 0;
    virtual QWidget* centralDockingArea() = 0;
};
```

When the plugin initializes, the host passes a pointer:

```cpp
widgetPlugin->initialize(appInterfacePointer);
```

##### Example: Adding a menu action

In main lib:
```cpp
class IWidgetPlugin {
public:
    virtual ~IWidgetPlugin() {}
    virtual void initialize(IAppInterface* host) = 0;
};
```

Plugin implementation:
```cpp
void MyPlugin::initialize(IAppInterface* host) {
    QAction* action = new QAction("Do Something", host->mainWindow());
    QObject::connect(action, &QAction::triggered, [](){
        qDebug() << "Plugin action triggered!";
    });
    host->addAction(action, "Tools"); // host adds it to menu
}
```

Main app:
```cpp
void MyApp::addAction(QAction* action, const QString& menuName) {
    QMenu* menu = menuBar()->findChild<QMenu*>(menuName);
    if (!menu) {
        menu = menuBar()->addMenu(menuName);
        menu->setObjectName(menuName);
    }
    menu->addAction(action);
}
```

The plugin never directly manipulates `QMenuBar` except through the interface.

#### Plugins in C / Rust

The only thing that changes is that now you have 2 libraries for a single plugin, the QT C++ Interface, and the backend C / Rust library (that has the main logic). The QT Library links against C / Rust library, and is what the main app scans and loads during runtime.

Rust lib:
```cpp
#[no_mangle]
pub extern "C" fn create_widget() -> *mut std::os::raw::c_void {
    // allocate widget logic, return opaque pointer
    std::ptr::null_mut() // placeholder
}
```

or alternatively C lib:
```cpp
#include <stdlib.h>

void* create_widget() {
    return NULL; // replace with real allocation
}
```

C++ wrapper lib:
 ```cpp
 class RustPlugin : public IWidgetPlugin {
public:
    void initialize(IAppInterface* host) override {
        QWidget* w = reinterpret_cast<QWidget*>(create_widget());
        host->centralDockingArea()->addWidget(w);
    }
};
 ```

The main thing you need to worry about is ABI compatibility, anything that you share across languages should always be C code.