#include "MainWindow.h"
#include "Options.h"

#include <QApplication>
#include <QFontDatabase>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <QFile>

static void loadFonts() {

    QStringList fontFiles = {
        // Inter (UI)
        ":/fonts/Inter_18pt-Regular.ttf",
        ":/fonts/Inter_18pt-Bold.ttf",
        ":/fonts/Inter_18pt-Italic.ttf",
        ":/fonts/Inter_18pt-BoldItalic.ttf",
        ":/fonts/Inter_18pt-SemiBold",
        ":/fonts/Inter_18pt-SemiBoldItalic.ttf",

        // Monospace
        ":/fonts/SourceCodePro-Regular.ttf",
        ":/fonts/SourceCodePro-Bold.ttf",
        ":/fonts/SourceCodePro-Italic.ttf",
        ":/fonts/SourceCodePro-BoldItalic.ttf"
    };


    for (const auto& file : fontFiles) {
        int fontId = QFontDatabase::addApplicationFont(file);
        if (fontId == -1) qWarning() << "Failed to load:" << file;
    }
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    loadFonts();

    QCoreApplication::setApplicationName("Memutilz");
    QCoreApplication::setApplicationVersion("1.0.0");

    QCommandLineParser parser{};
    parser.addHelpOption();
    parser.setApplicationDescription("UI Interface for the memutilz library");
    parser.addOption(Options::verboseOption);
    parser.addOption(Options::pidOption);
    parser.addOption(Options::fileOption);

    parser.process(a);

    a.setStyle(QStyleFactory::create("Fusion"));

    QPalette dark;

    dark.setColor(QPalette::Window, QColor("#0b0b0b"));
    dark.setColor(QPalette::Base, QColor("#101010"));
    dark.setColor(QPalette::AlternateBase, QColor("#181818"));

    dark.setColor(QPalette::Button, QColor("#101010"));
    dark.setColor(QPalette::Mid, QColor("#121212"));

    dark.setColor(QPalette::Text, QColor("#ffffff"));
    dark.setColor(QPalette::ButtonText, QColor("#ffffff"));
    dark.setColor(QPalette::WindowText, QColor("#ffffff"));

    dark.setColor(QPalette::Highlight, QColor("#ffffff"));
    dark.setColor(QPalette::HighlightedText, QColor("#000000"));

    a.setPalette(dark);

    QFile style(":/styles/default.qss");
    style.open(QFile::ReadOnly);
    QString styleSheet = style.readAll();
    a.setStyleSheet(styleSheet);

    MainWindow w;
    w.show();
    return a.exec();
}