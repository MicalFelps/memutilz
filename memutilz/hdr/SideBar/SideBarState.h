#ifndef SIDEBARSTATE_H
#define SIDEBARSTATE_H

enum class SideBarState {
	Opening = 0,
	Opened,
	Closing,
	Closed
};

const char* to_str(const SideBarState state);

#endif