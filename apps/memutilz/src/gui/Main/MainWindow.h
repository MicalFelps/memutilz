#include "MainWindowInternal.h"

/**
 * @brief The actual MainWindow class that's used to handle connections
 * between widgets.
 */
class MainWindow : public MainWindowInternal {
    Q_OBJECT
public:
    explicit MainWindow(
           const QString& uniqueName, QWidget* parent = nullptr, bool frameless = true,
           KDDockWidgets::MainWindowOptions options = {}, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~MainWindow() = default;
private:
    
};
