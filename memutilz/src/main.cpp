#include "MainWindow.h"
#include "Options.h"

#include <QApplication>
#include <QFontDatabase>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <QFile>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    int fontId = QFontDatabase::addApplicationFont(":/fonts/JetBrainsMono-Regular.ttf");
    if (fontId < 0) {
        qWarning() << "Failed to load JetBrains Mono!";
    } else {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        a.setFont(QFont(family));
    }

    QCoreApplication::setApplicationName("Memutilz");
    QCoreApplication::setApplicationVersion("2.0.0");

    QCommandLineParser parser{};
    parser.addHelpOption();
    parser.setApplicationDescription("UI Interface for the memutilz library");
    parser.addOption(Options::verboseOption);
    parser.addOption(Options::pidOption);
    parser.addOption(Options::fileOption);

    parser.process(a);

    a.setStyle(QStyleFactory::create("Fusion"));

    QFile style(":/styles/dark.qss");
    style.open(QFile::ReadOnly);
    QString styleSheet = style.readAll();
    a.setStyleSheet(styleSheet);

    MainWindow w;
    w.show();
    return a.exec();
}