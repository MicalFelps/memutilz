#include "gui/Hexview.h"

namespace gui {
	Hexview::Hexview(QWidget* parent) 
		: QAbstractScrollArea(parent)
	{
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

		m_font = QFont("Consolas", 10);
		m_font.setStyleHint(QFont::Monospace);
		m_font.setFixedPitch(true);
		
		getMetrics();
		updateScrollbars();
	}

	void Hexview::setMemdump(const mem::Memdump* memdump) {
		m_memdump = memdump;
		m_lineCache.clear();

		updateScrollbars();
		viewport()->update();
	}

	void Hexview::setDisplayConfig(const DisplayConfig& config) {
		m_config = config;
		m_lineCache.clear();
		m_unknownPattern.clear();
		getMetrics();
		updateScrollbars();
		viewport()->update();
	}
	void Hexview::goToAddress(LPCVOID address) {
		uintptr_t alignedAddr = reinterpret_cast<uintptr_t>(address);
		alignedAddr &= ~(m_config.bytesPerLine - 1);
		m_topAddress = alignedAddr;

		updateScrollbars();
		viewport()->update();
	}
	void Hexview::clear() {
		m_memdump = nullptr;
		m_lineCache.clear();
		viewport()->update();
	}

	void Hexview::paintEvent(QPaintEvent* event) {
	}
	void Hexview::resizeEvent(QResizeEvent* event) {
		QAbstractScrollArea::resizeEvent(event);
		getMetrics();
		updateScrollbars();
	}
	void Hexview::wheelEvent(QWheelEvent* event) {
		int degrees = event->angleDelta().y() / 8;
		int steps = degrees / 15;

		verticalScrollBar()->setValue(verticalScrollBar()->value() - steps);
		event->accept();
	}

	void Hexview::getMetrics() {
	}
	void Hexview::updateScrollbars() {
	}
	void Hexview::buildUnknownPattern() {
	}
	QString Hexview::formatLine(LPCVOID addr, bool bIsUnknown)
	{
		return QString();
	}
	std::vector<BYTE> Hexview::readBytesAt(LPCVOID addr, size_t amount) const
	{
		return std::vector<BYTE>();
	}
}