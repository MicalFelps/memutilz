#include "gui/Hexview.h"

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

		connect(verticalScrollBar(), &QScrollBar::valueChanged,
				this, &Hexview::onVerticalScrollChange);
	}
	void Hexview::setMemdump(const mem::Memdump* memdump) {
		m_memdump = memdump;

		if (m_memdump) {
			m_bIs64Bit = (!m_memdump->getMeminfo()->is32Bit());
			(m_bIs64Bit)
				? m_maxDisplayAddress = mem::USERSPACE_END_64BIT
				: m_maxDisplayAddress = mem::USERSPACE_END_32BIT;
			m_metrics.totalLines = (m_maxDisplayAddress + m_config.bytesPerLine - 1) / m_config.bytesPerLine;
			m_topAddress = m_memdump->getMeminfo()->get_program_base();
			updateScrollbars();
		}

		viewport()->update();
	}
	void Hexview::setDisplayConfig(DisplayConfig& config) {
		config.bytesPerLine = (config.bytesPerLine + 7) & ~0x7;
		m_config = config;
		getMetrics();
		updateScrollbars();
		viewport()->update();
	}
	void Hexview::goToAddress(LPCVOID address) {
		uintptr_t alignedAddr = reinterpret_cast<uintptr_t>(address);

		if (alignedAddr > m_maxDisplayAddress)
			alignedAddr = m_maxDisplayAddress;

		alignedAddr &= ~(m_config.bytesPerLine - 1);

		m_topAddress = alignedAddr;

		updateScrollbars();
		viewport()->update();
	}
	void Hexview::clear() {
		m_memdump = nullptr;
		viewport()->update();
	}

	void Hexview::paintEvent(QPaintEvent* event) {
		QPainter painter{ viewport() };
		painter.fillRect(event->rect(), palette().base());

		int firstLine = event->rect().top() / m_metrics.lineHeight;
		int lastLine = (event->rect().bottom() + m_metrics.lineHeight - 1) / m_metrics.lineHeight;
		lastLine = (((lastLine) < (static_cast<int>(m_visibleLines) - 1)) ? (lastLine) : (static_cast<int>(m_visibleLines) - 1));

		int hScrollOffset = horizontalScrollBar() ? horizontalScrollBar()->value() : 0;

		if (!m_memdump) {
			for (int line = firstLine; line <= lastLine; ++line) {
				int y = line * m_metrics.lineHeight + m_metrics.charHeight;
				uintptr_t lineAddress = m_topAddress + (line * m_config.bytesPerLine);

				if (lineAddress >= m_maxDisplayAddress)
					break;

				QString formattedLine = formatLine(reinterpret_cast<LPCVOID>(lineAddress), true);
				painter.drawText(-hScrollOffset, y, formattedLine);
			}
		} 
		else { // If process attached
			mem::RegionContext ct = m_memdump->getRegionContext(reinterpret_cast<LPCVOID>(m_topAddress));
			uintptr_t boundary{ m_maxDisplayAddress };
			bool isUnknown{ false };

			if (!ct.curr) { // If we're in unreadable memory
				if (ct.next) {
					boundary = reinterpret_cast<uintptr_t>(ct.next->m_original_addr);
				}
				else {
					boundary = m_maxDisplayAddress;
				}
				isUnknown = true;
			}
			else if (ct.next) { // if we're in readable memory
				if(ct.next->m_original_addr != reinterpret_cast<LPCVOID>(reinterpret_cast<uintptr_t>(ct.curr->m_original_addr) + ct.curr->m_size))
					boundary = reinterpret_cast<uintptr_t>(ct.next->m_original_addr);
			}

			for (int line = firstLine; line <= lastLine; ++line) {
				int y = line * m_metrics.lineHeight + m_metrics.charHeight;
				uintptr_t lineAddress = m_topAddress + (line * m_config.bytesPerLine);

				if (lineAddress >= m_maxDisplayAddress)
					break;

				// Recalc new boundary
				if (lineAddress >= boundary) {
					ct = m_memdump->getRegionContext(reinterpret_cast<LPCVOID>(lineAddress));
					isUnknown = !isUnknown;

					if (!ct.curr) { // Now in unreadable memory
						if (ct.next)
							boundary = reinterpret_cast<uintptr_t>(ct.next->m_original_addr);
						else
							boundary = m_maxDisplayAddress;
					}
					else { // Now in readable memory
						boundary = reinterpret_cast<uintptr_t>(ct.curr->m_original_addr) + ct.curr->m_size;
					}
				}

				QString formattedLine = formatLine(reinterpret_cast<LPCVOID>(lineAddress), isUnknown);
				painter.drawText(-hScrollOffset, y, formattedLine);
			}
		}
		// Draw Lines
		if (m_config.bShowAddress && m_config.bShowAscii) {
			painter.setPen(QPen(palette().mid().color(), 1));
			const int lineOffsetX = 2 * m_metrics.charWidth;

			// Line after address
			int addressEndX = m_metrics.addressWidth - hScrollOffset;
			if (addressEndX > 0 && addressEndX < viewport()->width()) {
				painter.drawLine(addressEndX - lineOffsetX, event->rect().top(),
								addressEndX - lineOffsetX, event->rect().bottom());
			}

			// Line after hex
			int hexEndX = m_metrics.addressWidth + m_metrics.hexWidth - hScrollOffset;
			if (hexEndX > 0 && hexEndX < viewport()->width()) {
				painter.drawLine(hexEndX - lineOffsetX, event->rect().top(),
								hexEndX - lineOffsetX, event->rect().bottom());
			}
		}
	}
	void Hexview::showEvent(QShowEvent* event) {
		QAbstractScrollArea::showEvent(event);
		if (!initialized) {
			getMetrics();
			m_visibleLines = viewport()->height() / m_metrics.lineHeight;
			updateScrollbars();
			initialized = true;
		}
	}
	void Hexview::resizeEvent(QResizeEvent* event) {
		QAbstractScrollArea::resizeEvent(event);
		if (initialized) {
			m_visibleLines = (viewport()->height() + m_metrics.lineHeight - 1) / m_metrics.lineHeight;
			updateScrollbars();
		}
	}
	void Hexview::wheelEvent(QWheelEvent* event) {
		int degrees = event->angleDelta().y() / 8;
		int steps = degrees / 15;

		int newScrollValue = verticalScrollBar()->value() - steps;
		verticalScrollBar()->setValue(newScrollValue);

		m_topAddress = static_cast<uintptr_t>(newScrollValue) * m_config.bytesPerLine;

		viewport()->update();
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
	}
	void Hexview::updateScrollbars() {
		disconnect(verticalScrollBar(), &QScrollBar::valueChanged,
			this, &Hexview::onVerticalScrollChange);

		int scrollRangeLines = SCROLL_RANGE / m_config.bytesPerLine;
		int maxLines = m_maxDisplayAddress / m_config.bytesPerLine;
		int currentLine = m_topAddress / m_config.bytesPerLine;
		int minScrollLine, maxScrollLine;

		if (currentLine < scrollRangeLines / 2) {
			minScrollLine = 0;
			maxScrollLine = scrollRangeLines;
		}
		else if (currentLine + scrollRangeLines / 2 > maxLines) {
			maxScrollLine = maxLines;
			minScrollLine = maxLines - scrollRangeLines;
		}
		else {
			minScrollLine = currentLine - scrollRangeLines / 2;
			maxScrollLine = currentLine + scrollRangeLines / 2;
		}

		// size_t maxScroll = m_metrics.totalLines - static_cast<int>(m_visibleLines);
		verticalScrollBar()->setRange(minScrollLine, maxScrollLine);
		verticalScrollBar()->setValue(currentLine);
		verticalScrollBar()->setPageStep(m_visibleLines);
		verticalScrollBar()->setSingleStep(1);

		connect(verticalScrollBar(), &QScrollBar::valueChanged,
			this, &Hexview::onVerticalScrollChange);

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
	void Hexview::onVerticalScrollChange(int value) {
		m_topAddress = static_cast<uintptr_t>(value) * m_config.bytesPerLine;

		// Check if we need to recalculate the window
		int minScrollLine = verticalScrollBar()->minimum();
		int maxScrollLine = verticalScrollBar()->maximum();
		int scrollRangeLines = SCROLL_RANGE / m_config.bytesPerLine;

		// If we're getting close to the edges, recalculate the window
		int edgeThreshold = scrollRangeLines / 4; // Recalculate when within 1/4 of edge

		bool needsRecalc = false;
		if (value - minScrollLine < edgeThreshold) {
			// Near the bottom edge, need to expand downward
			needsRecalc = true;
		}
		else if (maxScrollLine - value < edgeThreshold) {
			// Near the top edge, need to expand upward  
			needsRecalc = true;
		}

		if (needsRecalc) {
			// Temporarily disconnect to prevent recursion
			disconnect(verticalScrollBar(), &QScrollBar::valueChanged,
				this, &Hexview::onVerticalScrollChange);

			updateScrollbars(); // This will recalculate the window around new position

			// Reconnect
			connect(verticalScrollBar(), &QScrollBar::valueChanged,
				this, &Hexview::onVerticalScrollChange);
		}

		viewport()->update();
	}
	QString Hexview::formatLine(LPCVOID addr, bool bIsUnknown) {
		uintptr_t address = reinterpret_cast<uintptr_t>(addr);

		size_t capacity = 0;
		if (m_config.bShowAddress) {
			capacity += m_bIs64Bit ? 16 : 12;
		}

		capacity += m_config.bytesPerLine * 3;

		if (m_config.bShowAscii)
			capacity += m_config.bytesPerLine;

		QString formattedLine;
		formattedLine.reserve(capacity);

		if (m_config.bShowAddress) {
			m_bIs64Bit 
				? formattedLine.append(QStringLiteral("0x%1: ").arg(address, 12, 16, QLatin1Char('0')))
				: formattedLine.append(QStringLiteral("0x%1: ").arg(static_cast<uint32_t>(address), 8, 16, QLatin1Char('0')));
		}

		if (bIsUnknown) {
			for (size_t i = 0; i < m_config.bytesPerLine; ++i) {
				formattedLine.append("?? ");
			}

			if (m_config.bShowAscii)
				formattedLine.append(QString('.').repeated(m_config.bytesPerLine));

			return formattedLine;
		}
		
		if (!m_memdump)
			return formattedLine;

		mem::MemoryView mv = m_memdump->readBytesAt(addr, m_config.bytesPerLine);

		for (size_t i = 0; i < m_config.bytesPerLine; ++i) {
			if (i < mv.size) {
				BYTE b = mv.data[i];
				formattedLine.append(HEX_DIGITS[b >> 4]);
				formattedLine.append(HEX_DIGITS[b & 0x0F]);
			} else {
				formattedLine.append("??");
			}
			formattedLine.append(" ");
		}

		if (m_config.bShowAscii) {
			for (size_t i = 0; i < m_config.bytesPerLine; ++i) {
				if (i < mv.size) {
					const BYTE b = mv.data[i];
					formattedLine.append(IS_PRINTABLE[b] ? static_cast<char>(b) : '.');
				} else {
					formattedLine.append(".");
				} 
			}
		}
		return formattedLine;
	}
}