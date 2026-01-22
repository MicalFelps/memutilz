#include <QApplication>
#include <QFontDatabase>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <QFile>

#include "gui/Src/Gui/MainWindow.h"
#include "Options.h"

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
    dark.setColor(QPalette::WindowText, QColor("#bbbbbb"));

    dark.setColor(QPalette::Base, QColor("#121212"));
    dark.setColor(QPalette::AlternateBase, QColor("#181818"));  // checked / alternate rows

    dark.setColor(QPalette::ToolTipBase, QColor("#202020"));
    dark.setColor(QPalette::ToolTipText, QColor("#e6e6e6"));

    dark.setColor(QPalette::Text, QColor("#d8d8d8"));
    dark.setColor(QPalette::Button, QColor("#1c1c1c"));
    dark.setColor(QPalette::ButtonText, QColor("#d6d6d6"));
    dark.setColor(QPalette::BrightText, QColor("#ffffff"));  // warnings / emphasis only

    dark.setColor(QPalette::Light, QColor("#2a2a2a"));     // raised edges / highlight
    dark.setColor(QPalette::Midlight, QColor("#202020"));  // hover
    dark.setColor(QPalette::Mid, QColor("#151515"));       // pressed
    dark.setColor(QPalette::Dark, QColor("#0b0b0b"));      // borders
    dark.setColor(QPalette::Shadow, QColor("#050505"));    // depth

    dark.setColor(QPalette::PlaceholderText, QColor("#7a7a7a"));

    a.setPalette(dark);

    QFile style(":/styles/dark.qss");
    style.open(QFile::ReadOnly);
    QString styleSheet = style.readAll();
    a.setStyleSheet(styleSheet);

    MainWindow w;
    w.show();
    return a.exec();
}