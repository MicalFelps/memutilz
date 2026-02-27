#include <kddockwidgets/core/TitleBar.h>

#include "TitleBar.h"
#include "../../Constants/Ui.h"

using namespace Memutilz;

TitleBar::TitleBar(KDDockWidgets::Core::TitleBar* controller,
                   KDDockWidgets::Core::View* parent)
    : KDDockWidgets::QtWidgets::TitleBar(controller, parent),
      _controller(controller) {
    setFixedHeight(Constants::Ui::FixedDockTitleBarHeight);
    setContentsMargins(0, 0, 2, 0);
}

TitleBar::~TitleBar() {}

void TitleBar::init() {
    _controller->setHideDisabledButtons(
        KDDockWidgets::TitleBarButtonType::Close);
    KDDockWidgets::QtWidgets::TitleBar::init();
}
