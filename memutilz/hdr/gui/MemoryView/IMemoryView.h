#ifndef IMEMORYVIEW_H
#define IMEMORYVIEW_H

#include <QAbstractScrollArea>
#include <QScrollBar>

#include "mem/Memdump.h"
#include "mem/constants.h"

namespace gui {
	class IMemoryView {
	public:
		virtual void setMemdump(std::shared_ptr<mem::Memdump> memdump) = 0;
		virtual void goToAddress(LPCVOID address) = 0;
		virtual void detach() = 0;
	};
}

#endif