#ifndef SIDEPANELSTATE_H
#define SIDEPANELSTATE_H

enum class SidePanelState {
	Opening = 0,
	Opened,
	Closing,
	Closed
};

const char* to_str(const SidePanelState state);

#endif