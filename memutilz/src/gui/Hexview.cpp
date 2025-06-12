#include "gui/Hexview.h"

namespace gui {
	Hexview::Hexview(QWidget* parent) 
		: QAbstractScrollArea(parent)
	{

	}

	void Hexview::setMemdump(const mem::Memdump* memdump) {
	}

	void Hexview::setDisplayConfig(const DisplayConfig& config) {
	}

	void Hexview::goToAddress(LPCVOID address) {
	}

	void Hexview::clear() {
	}

	void Hexview::paintEvent(QPaintEvent* event) {
	}

	void Hexview::resizeEvent(QResizeEvent* event) {
	}

	void Hexview::wheelEvent(QWheelEvent* event) {
	}

	void Hexview::getMetrics() {
	}

	void Hexview::updateScrollbars() {
	}

	QString Hexview::formatLine(const BYTE* data, size_t length, uintptr_t baseAddress) const {
		return QString();
	}

	BYTE* Hexview::readBytesAt(LPCVOID addr) const {
		return nullptr;
	}

	const mem::MemoryRegion* Hexview::findRegionForAddress(LPCVOID address) const {
		return nullptr;
	}
}