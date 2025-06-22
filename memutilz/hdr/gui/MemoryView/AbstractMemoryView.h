#pragma once

#include <QAbstractScrollArea>
#include <QScrollBar>

#include "mem/Memdump.h"
#include "mem/constants.h"

namespace gui {
	class AbstractMemoryView : public QAbstractScrollArea {
		Q_OBJECT
	public:
		AbstractMemoryView(QWidget* parent = nullptr);

		void setMemdump(mem::Memdump* memdump);
		virtual void goToAddress(LPCVOID address) = 0;
		void clear() { m_memdump = nullptr; viewport()->update(); } // Detach from process
	protected slots:
		virtual void onVerticalScrollChange(int value) = 0;
	protected:
		virtual void updateAddressWidth() = 0; // Virtual hook in setMemdump

		virtual void getMetrics() = 0;
		virtual void updateScrollbars() = 0;

		virtual QString formatLine(mem::MemoryView lineView, LPCVOID addr, bool bIsUnknown) = 0;
		virtual QString formatHeaderLine() = 0;

		mem::Memdump* m_memdump{ nullptr };
		const mem::Meminfo* m_meminfo{ nullptr };

		uintptr_t m_maxDisplayAddress{ mem::USERSPACE_END_32BIT };
		
		QFont m_font;
		bool initialized = false;

		uintptr_t m_topAddress{ 0 }; // address at the top of view
		int m_visibleLines{ 1 };
	};
}