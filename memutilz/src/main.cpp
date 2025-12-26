#include "MainWindow.h"
#include "Options.h"

#include <qapplication.h>
#include <qcommandlineparser.h>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("Memutilz");
    QCoreApplication::setApplicationVersion("2.0.0");

    QCommandLineParser parser{};
    parser.addHelpOption();
    parser.setApplicationDescription("UI Interface for the memutilz library");

    parser.addOption(Options::verboseOption);
    parser.addOption(Options::pidOption);
    parser.addOption(Options::fileOption);

    parser.process(a);

    a.setStyle("fusion");

    MainWindow w;
    w.show();
    return a.exec();
}