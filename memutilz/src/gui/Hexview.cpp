#include "gui/Hexview.h"

namespace mem {
	inline constexpr SIZE_T USERSPACE_START_ADDR = 0x0;
	inline constexpr SIZE_T USERSPACE_END_32BIT = 0x7FFFFFFF;
	inline constexpr SIZE_T USERSPACE_END_64BIT = 0x00007FFFFFFFFFFF;
}

namespace gui {
	Hexview::Hexview(QWidget* parent) 
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

		buildUnknownPattern();
		getMetrics();
		updateScrollbars();
	}
	void Hexview::setMemdump(const mem::Memdump* memdump) {
		m_memdump = memdump;

		if (m_memdump) {
			m_bIs64Bit = !m_memdump->getMeminfo()->m_targetProcess->isWoW64();
			m_topAddress = m_memdump->getMeminfo()->m_targetProcess->get_program_base();
		}

		m_lineCache.clear();

		updateScrollbars();
		viewport()->update();
	}
	void Hexview::setDisplayConfig(DisplayConfig& config) {
		config.bytesPerLine = (config.bytesPerLine + 7) & ~0x7;
		m_config = config;
		m_lineCache.clear();
		buildUnknownPattern();
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
		QPainter painter{ viewport() };
		painter.fillRect(event->rect(), palette().base());

		int firstLine = event->rect().top() / m_metrics.lineHeight;
		int lastLine = (event->rect().bottom() + m_metrics.lineHeight - 1) / m_metrics.lineHeight;
		lastLine = (((lastLine) < (static_cast<int>(m_visibleLines) - 1)) ? (lastLine) : (static_cast<int>(m_visibleLines) - 1));
		int hScrollOffset = horizontalScrollBar()->value();

		uintptr_t maxAddress = m_bIs64Bit ? mem::USERSPACE_END_64BIT : mem::USERSPACE_END_32BIT;

		if (m_unknownPatternLength)
			buildUnknownPattern();

		if (!m_memdump) {
			for (int line = firstLine; line <= lastLine; ++line) {
				int y = line * m_metrics.lineHeight + m_metrics.charHeight;
				uintptr_t lineAddress = m_topAddress + (line * m_config.bytesPerLine);

				if (lineAddress >= maxAddress)
					break;

				QString formattedLine;
				formattedLine = formatLine(reinterpret_cast<LPCVOID>(lineAddress), true);

				painter.drawText(-hScrollOffset, y, formattedLine);
			}
		} else {
			const mem::RegionContext ct = m_memdump->getRegionContext(reinterpret_cast<LPCVOID>(m_topAddress));
			uintptr_t boundary{ maxAddress };
			size_t linesBeforeBoundary;
			bool isUnknown{ false };

			if (!ct.curr) { // If we're in unreadable memory
				uintptr_t boundary = reinterpret_cast<uintptr_t>(ct.next->m_original_addr);
				isUnknown = true;
			}
			else
				uintptr_t boundary = reinterpret_cast<uintptr_t>(ct.curr->m_original_addr) + ct.curr->m_size;

			if ((m_topAddress + (m_visibleLines * m_config.bytesPerLine)) > boundary)
				linesBeforeBoundary = ((m_topAddress + (m_visibleLines * m_config.bytesPerLine)) - boundary) / m_config.bytesPerLine;

			for (int line = firstLine; line <= lastLine; ++line) {
				int y = line * m_metrics.lineHeight + m_metrics.charHeight;
				uintptr_t lineAddress = m_topAddress + (line * m_config.bytesPerLine);

				if (lineAddress >= maxAddress)
					break;

				QString formattedLine;
				if (m_lineCache.contains(lineAddress))
					formattedLine = m_lineCache[lineAddress];
				else {
					if (lineAddress >= boundary)
						isUnknown = !isUnknown;
					formattedLine = formatLine(reinterpret_cast<LPCVOID>(lineAddress), isUnknown);
					m_lineCache[lineAddress] = formattedLine;
				}
				painter.drawText(-hScrollOffset, y, formattedLine);
			}
		}

		// Draw Lines
		if (m_config.bShowAddress && m_config.bShowAscii) {
			painter.setPen(QPen(palette().mid().color(), 1));

			// Line after address
			int addressEndX = m_metrics.addressWidth - hScrollOffset;
			if (addressEndX > 0 && addressEndX < viewport()->width()) {
				painter.drawLine(addressEndX - 20, event->rect().top(),
								addressEndX - 20, event->rect().bottom());
			}

			// Line after hex
			int hexEndX = m_metrics.addressWidth + m_metrics.hexWidth - hScrollOffset;
			if (hexEndX > 0 && hexEndX < viewport()->width()) {
				painter.drawLine(hexEndX - 60, event->rect().top(),
								hexEndX - 60, event->rect().bottom());
			}
		}
	}
	void Hexview::resizeEvent(QResizeEvent* event) {
		QAbstractScrollArea::resizeEvent(event);
		m_visibleLines = (viewport()->height() + m_metrics.lineHeight - 1) / m_metrics.lineHeight;
		updateScrollbars();
	}
	void Hexview::wheelEvent(QWheelEvent* event) {
		int degrees = event->angleDelta().y() / 8;
		int steps = degrees / 15;

		verticalScrollBar()->setValue(verticalScrollBar()->value() - steps);
		event->accept();
	}

	void Hexview::getMetrics() {
		QFontMetrics fm{m_font};
		m_metrics.charWidth = fm.horizontalAdvance('0');
		m_metrics.charHeight = fm.height();
		m_metrics.lineHeight = m_metrics.charHeight + 2; // some space in between lines

		if (m_config.bShowAddress) {
			m_metrics.addressWidth = m_bIs64Bit
				? fm.horizontalAdvance("000000000000: ")
				: fm.horizontalAdvance("00000000: ");
		} else m_metrics.addressWidth = 0;

		// Hex Area
		m_metrics.hexWidth = (m_config.bytesPerLine * 3) * m_metrics.charWidth;

		// Ascii Area
		if (m_config.bShowAscii) {
			m_metrics.asciiWidth = ((m_config.bytesPerLine + 1) * m_metrics.charWidth);
		} else m_metrics.asciiWidth = 0;

		m_metrics.totalWidth = m_metrics.addressWidth + m_metrics.hexWidth + m_metrics.asciiWidth;
		m_visibleLines = (viewport()->height() + m_metrics.lineHeight - 1) / m_metrics.lineHeight;

		// Height
		size_t totalBytes = m_bIs64Bit
			? mem::USERSPACE_END_64BIT
			: mem::USERSPACE_END_32BIT;

		m_metrics.totalLines = (totalBytes + m_config.bytesPerLine - 1) / m_config.bytesPerLine;
	}
	void Hexview::updateScrollbars() {
		int maxScroll = m_metrics.totalLines - static_cast<int>(m_visibleLines);
		verticalScrollBar()->setRange(0, maxScroll);
		verticalScrollBar()->setPageStep(m_visibleLines);
		verticalScrollBar()->setSingleStep(1);

		size_t currentLine = verticalScrollBar()->value();
		uintptr_t maxAddress = m_bIs64Bit ? mem::USERSPACE_END_64BIT : mem::USERSPACE_END_32BIT;
		if (currentLine > maxAddress / m_config.bytesPerLine) {
			currentLine = maxAddress / m_config.bytesPerLine;
		}
		m_topAddress = currentLine * m_config.bytesPerLine;

		int viewportWidth = viewport()->width();
		if (m_metrics.totalWidth > viewportWidth) {
			horizontalScrollBar()->setRange(0, m_metrics.totalWidth - viewportWidth);
			horizontalScrollBar()->setPageStep(viewportWidth);
			horizontalScrollBar()->setSingleStep(m_metrics.charWidth);
		} else {
			horizontalScrollBar()->setRange(0, 0);
		}
		viewport()->update();
	}
	void Hexview::buildUnknownPattern() {
		char* ptr = m_unknownPattern;
		char* end = m_unknownPattern + MAX_UNKNOWN_PATTERN_SIZE - 1;

		for(int i = 0; i < m_config.bytesPerLine - 1 && ptr < end - 2; ++i)
			*ptr++ = '?'; *ptr++ = '?'; *ptr++ = ' ';

		if (m_config.bShowAscii && ptr < end - m_config.bytesPerLine) {
			for (int i = 0; i < m_config.bytesPerLine && ptr < end; ++i)
				*ptr++ = '.';
		}
		m_unknownPatternLength = ptr - m_unknownPattern;
	}
	QString Hexview::formatLine(LPCVOID addr, bool bIsUnknown) {
		char buffer[256];
		char* ptr{ buffer };

		uintptr_t address = reinterpret_cast<uintptr_t>(addr);

		if (m_config.bShowAddress) {
			size_t addrDigits = m_bIs64Bit ? 12 : 8;
			*ptr++ = '0'; *ptr++ = 'x';
			int j = 0;
			for (int i = addrDigits - 1; i >= 0; --i, ++j) {
				ptr[i] = HEX_DIGITS[(address >> (4 * j)) & 0x0F];
			}
			ptr += addrDigits;
			*ptr++ = ':'; *ptr++ = ' ';
		}
		if (bIsUnknown) {
			memcpy(ptr, m_unknownPattern, m_unknownPatternLength);
			ptr += m_unknownPatternLength;
			return QString::fromLatin1(buffer, ptr - buffer);
		}
		if (m_memdump) {
			mem::MemoryView mv = m_memdump->readBytesAt(addr, m_config.bytesPerLine);

			// Hex
			for (size_t i = 0; i < m_config.bytesPerLine; ++i) {
				if (i < mv.size) {
					BYTE b = mv.data[i];
					*ptr++ = HEX_DIGITS[b >> 4];
					*ptr++ = HEX_DIGITS[b & 0x0F];
				}
				else {
					*ptr++ = '?'; *ptr++ = '?';
				}
				*ptr++ = ' ';
			}

			if (m_config.bShowAscii) {
				for (size_t i = 0; i < m_config.bytesPerLine; ++i) {
					if (i < mv.size) {
						BYTE b = mv.data[i];
						*ptr++ = IS_PRINTABLE[b] ? static_cast<char>(b) : '.';
					}
					else {
						*ptr++ = '.';
					}
				}
			}
			return QString::fromLatin1(buffer, ptr - buffer);
		}
		return QString::fromLatin1("");
	}
}