#ifndef WIDGETS_PAGES_H
#define WIDGETS_PAGES_H

enum class PageId {
    Explorer,
    Debug,
    Scan,
    Settings,
    None
};

constexpr const char* toString(PageId id) {
    switch (id) {
    case PageId::Explorer: return "Explorer";
    case PageId::Debug:    return "Debug";
    case PageId::Scan:     return "Scan";
    case PageId::Settings: return "Settings";
    case PageId::None:     return "None";
    }
    return "???";
}

#endif // WIDGETS_PAGES_H
