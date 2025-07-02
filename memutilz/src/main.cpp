#include "gui/common.h"
#include "gui/MainWindow.h"
#include "priv/privilege.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setStyle("fusion");

    if (!priv::CheckAndRequestAdmin())
        return 1; // Exit if we're not admin or if elevation failed / got declined

    MainWindow w;
    w.setWindowTitle("Memutilz 1.0");
    w.show();
    w.initialize();
    return app.exec();
}