#include <QCommandLineParser>
#include <QApplication>
#include <QStyleFactory>
#include <QFile>

#include "Options.h"
#include "gui/Resources.h"
#include "gui/Main/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("Memutilz");
    QCoreApplication::setApplicationVersion("0.0.2");

    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);

    QCommandLineParser parser{};
    parser.setApplicationDescription("UI Interface for the memutilz library");
    parser.addHelpOption();
    parser.addOption(Options::verboseOption);
    parser.addOption(Options::pidOption);
    parser.addOption(Options::fileOption);

    parser.process(QCoreApplication::arguments());

    a.setStyle(QStyleFactory::create("Fusion"));

    loadFonts();
    loadThemes();

    std::optional<ThemeInfo> defaultTheme =
        ThemeRegistry::getOrRegister("dark");
    a.setPalette(defaultTheme.has_value() ? defaultTheme.value().palette
                                          : QPalette());

    QFile style(":/styles/default.qss");
    style.open(QFile::ReadOnly);
    QString styleSheet = style.readAll();
    a.setStyleSheet(styleSheet);

    MainWindow mainWindow(QStringLiteral("MainWindow"));
    mainWindow.resize(1600, 900);
    mainWindow.show();
    return a.exec();
}
