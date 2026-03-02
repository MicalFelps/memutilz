#include <QToolButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QBoxLayout>
#include <QPainter>

#include "ApplicationWidget.h"
#include "../Main/MainWindow.h"
#include "../Constants/Ui.h"

using namespace Memutilz;

class SidebarToolButton : public QToolButton {
   public:
    explicit SidebarToolButton(QWidget* parent = nullptr)
        : QToolButton(parent) {
        setCheckable(true);
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        setFixedHeight(50);
        setMinimumWidth(200);
    }
    explicit SidebarToolButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr)
        : SidebarToolButton(parent) {
        setText(text);
        if (!icon.isNull())
            setIcon(icon);
    }

    QColor accentColor() const { return _accentColor; }
    void setAccentColor(const QColor& color) {
        _accentColor = color;
        update();
    }
    
   protected:
    virtual void paintEvent(QPaintEvent* event) override
    {
        QToolButton::paintEvent(event);

        if (!isChecked())
            return;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPen pen(_accentColor);
        pen.setWidth(4);
        painter.setPen(pen);

        painter.drawLine(2, 4, 2, height() - 4);
    }

   private:
    QColor _accentColor{ "#6D68CC" };
};

struct ApplicationWidget::Impl {
   private:
    MEMUTILZ_DECLARE_PUBLIC(ApplicationWidget);
   public:
    Impl() = delete;
    Impl(ApplicationWidget* _public) : _this{ _public } { initUI(); }

    void initUI();
    void addButtons();

    QHBoxLayout* layout;
    QWidget* sidebar;
    QVBoxLayout* sidebarLayout;
    QButtonGroup* buttonGroup;
    QStackedWidget* stack;
};

void ApplicationWidget::Impl::initUI() {
    _this->setStyleSheet("background-color: palette(window);");

    layout = new QHBoxLayout(_this);
    sidebar = new QWidget(_this);
    sidebarLayout = new QVBoxLayout(sidebar);
    stack = new QStackedWidget();

    layout->addWidget(sidebar);
    layout->addWidget(stack);

    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    buttonGroup = new QButtonGroup(_this);
    buttonGroup->setExclusive(true);
    
    addButtons();

    sidebarLayout->addStretch();

    // Colors

    sidebar->setStyleSheet("background-color: red;");
    stack->setStyleSheet("background-color: blue;");
}
void ApplicationWidget::Impl::addButtons() {
    struct ButtonInfo {
        QString iconPath;
        QString text;
    };

    ButtonInfo items[] = {
        {":/icons/file.svg", "Window-1"},
        {":/icons/file.svg", "Window-2"},
        {":/icons/file.svg", "Window-3"},
        {":/icons/file.svg", "Window-4"},
        {":/icons/file.svg", "Window-5"}
    };

    for (const auto& item : items) {
        SidebarToolButton* btn = new SidebarToolButton(QIcon(item.iconPath), item.text);
        if (item.text == "Performance")
            btn->setChecked(true);

        buttonGroup->addButton(btn);
        sidebarLayout->addWidget(btn);
    }
}

// ##################################################
// ##### ApplicationWidget
// ##################################################

ApplicationWidget::ApplicationWidget(MainWindow* parent)
    : QFrame(parent), d{ std::make_unique<ApplicationWidget::Impl>(this) } {
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    parent->installEventFilter(this);
    if (parent) {
        if (parent->isFrameless()) {
            int height = parent->height() - Constants::Ui::FixedMainWindowTitleBarHeight;
            setGeometry(0, Constants::Ui::FixedMainWindowTitleBarHeight, parent->width(), height);
        }
        else {
            setGeometry(0, 0, parent->width(), parent->height());
        }
    } else {
        setGeometry(0, 0, 300, 300);
    }
}
ApplicationWidget::~ApplicationWidget() {}

void ApplicationWidget::resizeToParent(const QSize& parentSize) {
    int y = 0;
    if (qobject_cast<MainWindow*>(parentWidget())->isFrameless()) y = Constants::Ui::FixedMainWindowTitleBarHeight;
    setGeometry(0, y, parentSize.width(), parentSize.height());
}

bool ApplicationWidget::eventFilter(QObject* obj, QEvent* ev) {
    if (obj && ev && (obj == parent())) {
        switch (ev->type()) {
            case QEvent::Resize: {
                auto size = static_cast<QResizeEvent*>(ev)->size();
                if (qobject_cast<MainWindow*>(parentWidget())->isFrameless()) {
                    size.setHeight(Constants::Ui::FixedMainWindowTitleBarHeight);
                }
                resizeToParent(static_cast<QResizeEvent*>(ev)->size());
                break;
            }
            default:
                break;
        }
    }
    return QFrame::eventFilter(obj, ev);
}
void ApplicationWidget::showEvent(QShowEvent* event) {
    QWidget* par = parentWidget();
    if (par) {
        resizeToParent(par->size());
    }
    raise();
    setFocus();  // Request focus when the window is shown
    QFrame::showEvent(event);
}
void ApplicationWidget::keyPressEvent(QKeyEvent* ev) {
    if (ev) {
        if (ev->key() == Qt::Key_Escape) {
            hide();
            ev->accept();
        }
    }
    return QFrame::keyPressEvent(ev);
}