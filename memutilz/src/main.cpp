#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Memutilz v2.0.0");
    a.setStyle("fusion");

    MainWindow w;
    w.resize(1600, 900);
    w.show();
    return a.exec();
}