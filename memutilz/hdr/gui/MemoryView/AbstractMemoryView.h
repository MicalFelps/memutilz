#ifndef ABSTRACTMEMORYVIEW_H
#define ABSTRACTMEMORYVIEW_H

#include <QAbstractScrollArea>
#include <QScrollBar>

#include "mem/Memdump.h"
#include "mem/constants.h"

namespace gui {
	class AbstractMemoryView {
	public:
		virtual void setMemdump(mem::Memdump* memdump) = 0;
		virtual void goToAddress(LPCVOID address) = 0;
		virtual void detach() = 0;
	protected:
		mem::Memdump* m_memdump{ nullptr };
		const mem::Meminfo* m_meminfo{ nullptr };
		uintptr_t m_maxDisplayAddress{ mem::USERSPACE_END_32BIT };
		uintptr_t m_topAddress{ 0 }; // address at the top of view
		QFont m_font;
		bool m_initialized = false;
		int m_visibleRows{ 1 };
	};
}

#endif