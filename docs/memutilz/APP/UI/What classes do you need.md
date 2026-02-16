In [[Managing Signals and Slots]] I mention general `createWidgetType`, but if you for example simply set your instance of QMenuBar as the menu using MainWindow's setMenu(), then you don't get any custom signals, so what you need to do is subclass menu, or whatever class for your own signals. So for every widget you'd like to get signals from, you'll need to make a subclass that accounts for plugins.

1. DockManager class
2. ApplicationWidget
3. RibbonBar
	You might want to make 2 different subclasses for `SARibbonBar` and `WindowButtonGroup (systemBar)`. Those can go inside of MainWindowInternal
4. StatusBar

```cpp
class WidgetMenu : public QMenu
{
    Q_OBJECT

public:
    enum class WidgetType {
        Button,
        Label,
        Slider
    };

    explicit WidgetMenu(QWidget* parent = nullptr);

signals:
    void createWidgetType(WidgetType type);
};
```

```cpp
WidgetMenu::WidgetMenu(QWidget* parent)
    : QMenu("Create Widget", parent)
{
    QAction* buttonAction = addAction("Button");
    QAction* labelAction  = addAction("Label");
    QAction* sliderAction = addAction("Slider");

    connect(buttonAction, &QAction::triggered, this, [this]() {
        emit createWidgetType(WidgetType::Button);
    });

    connect(labelAction, &QAction::triggered, this, [this]() {
        emit createWidgetType(WidgetType::Label);
    });

    connect(sliderAction, &QAction::triggered, this, [this]() {
        emit createWidgetType(WidgetType::Slider);
    });
}
```

