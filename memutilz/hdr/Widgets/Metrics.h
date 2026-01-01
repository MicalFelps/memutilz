#ifndef UI_METRICS_H
#define UI_METRICS_H

namespace Ui {
    namespace IconButton {
        inline constexpr int baseIconSize = 25;
        inline constexpr int minSize = 5;
        inline constexpr int minScaledIconSize = 10;
        inline constexpr int minWrapWidth = 25;
        
        inline constexpr int arrowRectSize = 8;
    }

    namespace SideBar {
        inline constexpr int buttonHeight = 50;
        inline constexpr int collapsedWidth = buttonHeight;
        inline constexpr int expandedWidth = 250;
        inline constexpr int hoverDelay = 500;
        inline constexpr int animDuration = 500; // milliseconds
    }
}

#endif // UI_METRICS_H