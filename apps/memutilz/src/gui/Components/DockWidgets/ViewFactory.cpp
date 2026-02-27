#include "ViewFactory.h"
#include "TitleBar.h"
#include "Group.h"

QIcon DockComponentFactory::iconForButtonType(
    KDDockWidgets::TitleBarButtonType type, qreal dpr) const {
    Q_UNUSED(dpr)

    QString iconName;
    switch (type) {
        case KDDockWidgets::TitleBarButtonType::AutoHide:
            iconName = QStringLiteral("pin-small");
            break;
        case KDDockWidgets::TitleBarButtonType::UnautoHide:
            iconName = QStringLiteral("pin-filled-small");
            break;
        case KDDockWidgets::TitleBarButtonType::Close:
            iconName = QStringLiteral("window-close-small");
            break;
        case KDDockWidgets::TitleBarButtonType::Minimize:
            iconName = QStringLiteral("window-minimize-small");
            break;
        case KDDockWidgets::TitleBarButtonType::Maximize:
            iconName = QStringLiteral("window-maximize-small");
            break;
        case KDDockWidgets::TitleBarButtonType::Normal:
            iconName = QStringLiteral("window-restore-small");
            break;
        case KDDockWidgets::TitleBarButtonType::Float:
            iconName = QStringLiteral("window-restore-small");
            break;
        case KDDockWidgets::TitleBarButtonType::AllTitleBarButtonTypes:
            break;
    }

    if (iconName.isEmpty()) return {};
    QIcon icon(QStringLiteral(":/icons/%1.svg").arg(iconName));
    return icon;
}

KDDockWidgets::Core::View* DockComponentFactory::createGroup(KDDockWidgets::Core::Group* controller, KDDockWidgets::Core::View* parent) const {
    return new Group(controller, KDDockWidgets::QtCommon::View_qt::asQWidget(parent));
}
KDDockWidgets::Core::View* DockComponentFactory::createTitleBar(
    KDDockWidgets::Core::TitleBar* controller,
    KDDockWidgets::Core::View* parent) const {
    return new TitleBar(controller, parent);
}
