#include <QStackedWidget>
#include <QStyle>

#include "MainWindow.h"
#include "Widgets/SideBar.h"
#include "Widgets/IconButton.h"
#include "Widgets/Metrics.h"

/*
#include <DockManager.h>
#include <DockWidget.h>
#include <DockAreaWidget.h>
*/

// ------------------------------------------------------------------

#include <QLabel>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>

static QLabel* makeHeader(const QString& text, const QColor& color)
{
    QLabel* label = new QLabel(text);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QString(
        "font-size: 18px;"
        "font-weight: bold;"
        "padding: 12px;"
        "background-color: %1;"
        "color: white;"
    ).arg(color.name()));
    return label;
}

// ------------------------------------------------------------------

struct MainWindowPrivate {
    MainWindow* _this;
    QWidget* _central{ nullptr };
    QHBoxLayout* _layout{ nullptr };
    SideBar* _sidebar{ nullptr };
    QStackedWidget* _content{ nullptr };
    QHash<PageId, QWidget*> _pages;

    MainWindowPrivate(MainWindow* _public);

    void createSidebar();
    void createContent();

    QWidget* createExplorerPage();
    QWidget* createDebugPage();
    QWidget* createScanPage();
    QWidget* createSettingsPage();
};

MainWindowPrivate::MainWindowPrivate(MainWindow* _public)
    : _this{ _public }
    , _central{ new QWidget(_this) }
    , _layout{ new QHBoxLayout(_central) }
{
}

void MainWindowPrivate::createSidebar() {
    _sidebar = new SideBar(SideBar::ExpandMode::Click);

    QList<IconButton*> buttons{ QList<IconButton*>() };

    IconButton* explorer = new IconButton(QIcon(":/icons/processor.svg"), "Explorer", _sidebar);
    IconButton* debug = new IconButton(QIcon(":/icons/debug.svg"), "Debug", _sidebar);
    IconButton* scan = new IconButton(QIcon(":/icons/search.svg"), "Scan", _sidebar);
    IconButton* settings = new IconButton(QIcon(":/icons/settings.svg"), "Settings", _sidebar);

    _sidebar->addTopButton(explorer, PageId::Explorer);
    _sidebar->addTopButton(debug, PageId::Debug);
    _sidebar->addTopButton(scan, PageId::Scan);
    _sidebar->addBottomButton(settings, PageId::Settings);

    buttons.append(explorer);
    buttons.append(debug);
    buttons.append(scan);
    buttons.append(settings);

    for (auto& b : buttons) {
        b->setCheckable(true);
        b->setIconScalePercent(50);
        b->setIconTextSpacing(20);
        b->setPadding({ (Ui::SideBar::buttonHeight - b->iconPaintRect().width()) / 2, 0, 0, 0 });
        b->setProperty("group", "sidebar");
        b->setFocusPolicy(Qt::NoFocus);
    }

    qDebug() << _sidebar->rect();
}
void MainWindowPrivate::createContent() {
    if(_content == nullptr) _content = new QStackedWidget();

    _pages[PageId::Explorer]    = createExplorerPage();
    _pages[PageId::Debug]       = createDebugPage();
    _pages[PageId::Scan]        = createScanPage();
    _pages[PageId::Settings]    = createSettingsPage();

    _content->addWidget(_pages.value(PageId::Explorer));
    _content->addWidget(_pages.value(PageId::Debug));
    _content->addWidget(_pages.value(PageId::Scan));
    _content->addWidget(_pages.value(PageId::Settings));
}

QWidget* MainWindowPrivate::createExplorerPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);

    layout->addWidget(makeHeader("Explorer", QColor("#3b82f6")));

    QListWidget* list = new QListWidget;
    for (int i = 0; i < 20; ++i)
        list->addItem(QString("Item %1").arg(i + 1));

    QLineEdit* filter = new QLineEdit;
    filter->setPlaceholderText("Filter…");

    layout->addWidget(filter);
    layout->addWidget(list, 1);

    return page;
}
QWidget* MainWindowPrivate::createDebugPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);

    layout->addWidget(makeHeader("Debug", QColor("#ef4444")));

    QCheckBox* verbose = new QCheckBox("Verbose logging");
    QCheckBox* trace = new QCheckBox("Enable tracing");

    QTextEdit* output = new QTextEdit;
    output->setPlaceholderText("Debug output…");
    output->setReadOnly(true);

    layout->addWidget(verbose);
    layout->addWidget(trace);
    layout->addWidget(output, 1);

    return page;
}
QWidget* MainWindowPrivate::createScanPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);

    layout->addWidget(makeHeader("Scan", QColor("#22c55e")));

    QProgressBar* progress = new QProgressBar;
    progress->setRange(0, 100);
    progress->setValue(42);

    QPushButton* start = new QPushButton("Start Scan");
    QPushButton* stop = new QPushButton("Stop");

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addWidget(start);
    buttons->addWidget(stop);
    buttons->addStretch();

    layout->addWidget(progress);
    layout->addLayout(buttons);
    layout->addStretch();

    return page;
}
QWidget* MainWindowPrivate::createSettingsPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);

    layout->addWidget(makeHeader("Settings", QColor("#a855f7")));

    QFormLayout* form = new QFormLayout;

    form->addRow("Username:", new QLineEdit);
    form->addRow("Refresh rate:", new QSpinBox);
    form->addRow("Enable feature X:", new QCheckBox);

    QComboBox* theme = new QComboBox;
    theme->addItems({ "Light", "Dark", "System" });
    form->addRow("Theme:", theme);

    layout->addLayout(form);
    layout->addStretch();

    return page;
}

// ------------------------------------------------------------------

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , d{new MainWindowPrivate(this)} {

    setMinimumSize(250, 50);
    resize(1600, 900);

    setGeometry(QStyle::alignedRect(
        Qt::LeftToRight, Qt::AlignCenter, frameSize(),
        QGuiApplication::primaryScreen()->availableGeometry()
    ));

    d->createSidebar();
    d->createContent();

    connect(d->_sidebar, &SideBar::selectedPageChanged, [this](PageId id) {
        d->_content->setCurrentWidget(d->_pages.value(id));
        });

    setCentralWidget(d->_central);

    d->_layout->setContentsMargins(0, 0, 0, 0);
    d->_layout->setSpacing(0);
    d->_layout->addWidget(d->_sidebar);
    d->_layout->addWidget(d->_content, 1);
}
MainWindow::~MainWindow() { delete d; }

void MainWindow::closeEvent(QCloseEvent* event) {
    QMainWindow::closeEvent(event);
}