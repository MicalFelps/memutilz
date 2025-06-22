#include "gui/MemoryView/AbstractMemoryView.h"

namespace gui {
	AbstractMemoryView::AbstractMemoryView(QWidget* parent) 
		: QAbstractScrollArea(parent)
	{
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		setFocusPolicy(Qt::StrongFocus);

		m_font = QFont("Consolas", 10);
		if (!m_font.exactMatch()) {
			m_font = QFont("Courier New", 10);
		}

		m_font.setStyleHint(QFont::Monospace);
		m_font.setFixedPitch(true);
		setFont(m_font);

		connect(verticalScrollBar(), &QScrollBar::valueChanged,
			this, &AbstractMemoryView::onVerticalScrollChange);
	}
	void AbstractMemoryView::setMemdump(mem::Memdump* memdump) {
		m_memdump = memdump;

		if (m_memdump) {
			m_meminfo = m_memdump->getMeminfo();
			m_meminfo->is32Bit()
				? m_maxDisplayAddress = mem::USERSPACE_END_32BIT
				: m_maxDisplayAddress = mem::USERSPACE_END_64BIT;
			m_topAddress = m_meminfo->get_program_base();
			updateAddressWidth();
			updateScrollbars();
		}
		viewport()->update();
	}
}