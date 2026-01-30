RibbonBar only emits signals when actions are pressed, it's also a good idea to make a handler function instead of a signal for each type of widget (which isn't doesn't scale well).

```cpp
// RibbonBar.h
signals:
    void createWidgetRequested(const QString &widgetType);
```

MainWindow is the heavy lifter, which handles connections. For an action that adds a widget to CentralDockingArea, MainWindow would do something like

```cpp
connect(ribbonBar, &RibbonBar::createWidgetRequested,
        this, &MainWindow::onCreateWidgetRequested);
```

```cpp
void MainWindow::onCreateWidgetRequested(const QString &type)
{
    auto widget = WidgetFactory::create(type, this);
    centralDockingArea->addWidget(widget);
}
```

and delegate the creation logic to a factory, such as the one below

```cpp
class WidgetFactory {
public:
    static QWidget* create(const QString& type, QWidget* parent) {
        if (type == "TextEditor") return new TextEditorWidget(parent);
        if (type == "ImageViewer") return new ImageViewerWidget(parent);
        // ... more types
        return nullptr;
    }
};
```

However this approach is not the best if you want to later support [[Plugins]]

So how do we scale this to something like 100 actions? We'd need to keep a map of action to ID/type similar to the one below:

```cpp
QMap<QAction*, QString> actionToType;
```

Ideally we keep this in a separate action manager class, similar to the one in QT Creator.

---
### Plugins and Signals

Plugins CAN use signals, but only pre-defined ones, and usually ones that work with interfaces. For example, if the app interface looks like the one below

```cpp
class IAppInterface {
public:
    virtual ~IAppInterface() = default;

    virtual QAction* createAction(
        const QString& text,
        const QObject* receiver,
        const char* slot
    ) = 0;

    virtual void addActionToMenu(QAction*, const QString& menu) = 0;

    virtual void openDockWidget(QWidget*) = 0;
};
```

The plugin now can

1. Call host API

```cpp
host->openDockWidget(widget);
```

2. Emit known request signals

```cpp
emit requestOpenWidget("HexViewer");
```

3. Register callbacks

```cpp
host->registerCommand("AnalyzeMemory", []{
    runAnalysis();
});
```

4. Receive host signals

```cpp
connect(host, &IAppInterface::currentDocumentChanged,
        this, &MyPlugin::onDocumentChanged);
```

---
### Other example from AI

You never modify any directly ever, everything happens through an interface. For example, suppose your app manages file editing, and your plugin knows how to handle a specific file type. Instead of "modifying" the "open file" action itself, you do something like this:

Instead of this:

```cpp
connect(openAction, &QAction::triggered,
        this, &MainWindow::openFile);
```

You do this:

```cpp
connect(openAction, &QAction::triggered,
        this, &MainWindow::openFileViaHandlers);
```

And now `openFileViaHandlers()` is just a **dispatcher**.

---

1. Define a handler interface (in main lib)

```cpp
class IFileHandler {
public:
    virtual ~IFileHandler() = default;

    virtual bool canOpen(const QString& path) const = 0;
    virtual bool open(const QString& path) = 0;
};
```

2. Register handlers in main app

```cpp
registerFileHandler(new TextFileHandler);
registerFileHandler(new BinaryFileHandler);
```

3. Plugins register additional handlers

```cpp
class MyFormatPlugin : public IPlugin {
public:
    void initialize(IAppInterface* host) override {
        host->registerFileHandler(new MyCustomFormatHandler);
    }
};
```

4. "Open File" action delegates

```cpp
void MainWindow::openFileViaHandlers() {
    QString path = askUserForFile();

    for (auto* handler : fileHandlers) {
        if (handler->canOpen(path)) {
            if (handler->open(path))
                return;
        }
    }

    showError("No handler for file type");
}
```

---

You can also choose to support priorities & conflicts

```cpp
virtual int priority() const { return 0; }
```

Then sort handlers before use.

This allows:
- Core handlers as fallback
- Plugins to override _by priority_, not by mutation