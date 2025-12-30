#include "SideBar/SideBarState.h"

const char* to_str(const SideBarState state)
{
	switch (state) {
	case SideBarState::Opening: return "Opening";
	case SideBarState::Opened:  return "Opened";
	case SideBarState::Closing: return "Closing";
	case SideBarState::Closed:  return "Closed";
	default: return "";
	}
}