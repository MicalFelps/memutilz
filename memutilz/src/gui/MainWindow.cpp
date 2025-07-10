#include "gui/MainWindow.h"

#include <DockManager.h>

void MainWindow::printError(const std::wstring& err) {
    QMessageBox::critical(this, "Error", QString::fromStdWString(err));
}

void MainWindow::printError(const std::string& err) {
    QMessageBox::critical(this, "Error", QString::fromStdString(err));
}

struct MainWindowPrivate {
    MainWindow* _this;
    gui::Ui_MainWindow* ui;

    std::unique_ptr<mem::Process> proc;
    std::unique_ptr<mem::Meminfo> meminfo;
    std::shared_ptr<mem::Memdump> memdump;

    std::unique_ptr<ads::CDockManager> DockManager{ nullptr };
    ads::CDockWidget* AssemblyView{ nullptr };
    ads::CDockWidget* HexView{ nullptr };

    gui::AssemblyView* AssemblyWidget = nullptr;
    gui::Hexview* HexWidget = nullptr;

    MainWindowPrivate(MainWindow* _public) : _this{ _public } {
        ui = new gui::Ui_MainWindow(_this);
    };
    ~MainWindowPrivate() {
        delete ui;
    }

    void initialize();
    void createContent();
    ads::CDockWidget* createAssemblyView();
    ads::CDockWidget* createHexView();
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , d(new MainWindowPrivate(this))
{
    using namespace ads;
    d->ui->setupUi(this);
    setWindowTitle(QApplication::instance()->applicationName());

    CDockManager::setConfigFlag(CDockManager::FocusHighlighting, true);
    CDockManager::setAutoHideConfigFlags({ CDockManager::DefaultAutoHideConfig });

    d->DockManager = std::make_unique<CDockManager>(this);
    d->DockManager->setDockWidgetToolBarStyle(Qt::ToolButtonIconOnly, ads::CDockWidget::StateFloating);

    d->createContent();
    resize(1280, 720);
    setGeometry(QStyle::alignedRect(
        Qt::LeftToRight, Qt::AlignCenter, frameSize(),
        QGuiApplication::primaryScreen()->availableGeometry()
    ));
    d->initialize();
}
MainWindow::~MainWindow() {
    delete d;
}

void MainWindowPrivate::initialize() {
    try {
        proc = std::make_unique<mem::Process>(L"ac_client.exe");

        meminfo = std::make_unique<mem::Meminfo>(proc.get());
        meminfo->findPageInfo();

        memdump = std::make_shared<mem::Memdump>(meminfo.get());
        memdump->dump();

        HexWidget->setMemdump(memdump);
        AssemblyWidget->setMemdump(memdump);
    }
    catch (const mem::Exception& e) {
        _this->printError(e.full_msg());
    }
    catch (const std::runtime_error& e) {
        _this->printError(e.what());
    }
    catch (...) {
        _this->printError(L"[!] Unknown Error\n");
    }
}
void MainWindowPrivate::createContent() {
    AssemblyView = createAssemblyView();
    HexView = createHexView();

    DockManager->addDockWidget(ads::DockWidgetArea::LeftDockWidgetArea, AssemblyView);
    DockManager->addDockWidget(ads::DockWidgetArea::RightDockWidgetArea, HexView);

}

ads::CDockWidget* MainWindowPrivate::createAssemblyView() {
    gui::AssemblyView* w = new gui::AssemblyView{};
    AssemblyWidget = w;
    ads::CDockWidget* DockWidget = DockManager->createDockWidget(QString("Disassembly"));
    DockWidget->setWidget(w);
    return DockWidget;
}
ads::CDockWidget* MainWindowPrivate::createHexView() {
    gui::Hexview* w = new gui::Hexview{};
    HexWidget = w;
    ads::CDockWidget* DockWidget = DockManager->createDockWidget(QString("Hexview"));
    DockWidget->setWidget(w);
    return DockWidget;
}