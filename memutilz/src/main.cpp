#include "gui/common.h"
#include "gui/MainWindow.h"
#include "priv/privilege.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("Memutilz 1.0");
    app.setStyle("fusion");

    if (!priv::CheckAndRequestAdmin())
        return 1; // Exit if we're not admin or if elevation failed / got declined

    MainWindow w;
    w.show();
    return app.exec();
}