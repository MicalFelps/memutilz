#include "gui/MemoryView/AbstractMemoryView.h"
#include "gui/MemoryView/Hexview.h"

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

	void Hexview::setMemdump(mem::Memdump* memdump) {
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

	void Hexview::paintEvent(QPaintEvent* event) {
		QPainter painter{ viewport() };
		painter.fillRect(event->rect(), palette().base());

		int firstLine = event->rect().top() / m_metrics.lineHeight;
		int lastLine = (event->rect().bottom() + m_metrics.lineHeight - 1) / m_metrics.lineHeight;
		lastLine = (((lastLine) < (static_cast<int>(m_visibleLines) - 1)) ? (lastLine) : (static_cast<int>(m_visibleLines) - 1));

		int hScrollOffset = horizontalScrollBar() ? horizontalScrollBar()->value() : 0;
		painter.drawText(-hScrollOffset, m_metrics.charHeight, formatHeaderLine());
		painter.setPen(QPen({ 255, 255, 255 }));
		int headerSeperatorOffsetY = 2;
		painter.drawLine(event->rect().left(), m_metrics.lineHeight + headerSeperatorOffsetY,
						m_metrics.totalWidth, m_metrics.lineHeight + headerSeperatorOffsetY);


		if (!m_memdump) {
			for (int line = firstLine; line <= lastLine; ++line) {
				int y = (line + 1) * m_metrics.lineHeight + m_metrics.charHeight;
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
			uintptr_t firstValidAddr{0};
			bool isUnknown{ false };

			if (!ct.curr) { // Unreadable Memory
				if (ct.next) {
					boundary = reinterpret_cast<uintptr_t>(ct.next->m_original_addr);
					firstValidAddr = boundary;
				}
				isUnknown = true;
			} else {  // if we're in readable memory
				if (ct.next) {
					if (ct.next->m_original_addr != reinterpret_cast<LPCVOID>(reinterpret_cast<uintptr_t>(ct.curr->m_original_addr) + ct.curr->m_size))
						boundary = reinterpret_cast<uintptr_t>(ct.next->m_original_addr);
				}
				firstValidAddr = m_topAddress;
			}

			SIZE_T shownBytes = m_visibleLines * m_config.bytesPerLine;
			m_memdump->setLiveMode();
			mem::MemoryView mv = m_memdump->readBytesAt(reinterpret_cast<LPCVOID>(firstValidAddr), shownBytes);

			for (int line = firstLine; line <= lastLine; ++line) {
				int y = (line + 1) * m_metrics.lineHeight + m_metrics.charHeight;
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

				mem::MemoryView lineView;
				SIZE_T byteOffset = (lineAddress - firstValidAddr);

				if (byteOffset <= mv.size) {
					lineView.data = mv.data + byteOffset;
					lineView.size = (((m_config.bytesPerLine) < (mv.size - byteOffset)) ? (m_config.bytesPerLine) : (mv.size - byteOffset));
				}

				QString formattedLine = formatLine(lineView, reinterpret_cast<LPCVOID>(lineAddress), isUnknown);
				painter.drawText(-hScrollOffset, y, formattedLine);
			}
		}

		// Draw Lines
		QColor lineColor{ 255, 0, 0 };
		painter.setPen(QPen(lineColor, 1));
		int numberOfLines = (m_config.bytesPerLine - 1) / 8;
		int lineOffsetX = m_metrics.charWidth / 2;

		for (int i = 1; i <= numberOfLines; ++i) {
			int positionX = m_metrics.addressWidth + ((m_metrics.charWidth * 3) * 8 * i) - hScrollOffset - lineOffsetX;
			if (positionX > 0 && positionX < viewport()->width()) {
				painter.drawLine(positionX, event->rect().top(),
								positionX, event->rect().bottom());
			}
		}
	}
	void Hexview::showEvent(QShowEvent* event) {
		QAbstractScrollArea::showEvent(event);
		if (!m_initialized) {
			getMetrics();
			m_visibleLines = viewport()->height() / m_metrics.lineHeight;
			updateScrollbars();
			m_initialized = true;
		}
	}
	void Hexview::resizeEvent(QResizeEvent* event) {
		QAbstractScrollArea::resizeEvent(event);
		if (m_initialized) {
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

	void Hexview::updateAddressWidth() {
		if (!m_config.bShowAddress) {
			m_metrics.addressWidth = 0;
			m_metrics.totalWidth = m_metrics.addressWidth + m_metrics.hexWidth + m_metrics.asciiWidth;
			return;
		}

		QFontMetrics fm{ m_font };
		if (!m_memdump) {
			m_metrics.addressWidth = fm.horizontalAdvance("0x00000000: ");
			return;
		}
		else {
			m_maxDisplayAddress == mem::USERSPACE_END_32BIT
				? fm.horizontalAdvance("0x00000000: ")
				: fm.horizontalAdvance("0x000000000000: ");
		}

		m_metrics.totalWidth = m_metrics.addressWidth + m_metrics.hexWidth + m_metrics.asciiWidth;
	}

	void Hexview::getMetrics() {
		QFontMetrics fm{m_font};
		m_metrics.charWidth = fm.horizontalAdvance('0');
		m_metrics.charHeight = fm.height();
		m_metrics.lineHeight = m_metrics.charHeight + 2; // some space in between lines

		// Address Area
		updateAddressWidth();
		
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

	QString Hexview::formatLine(mem::MemoryView lineView, LPCVOID addr, bool bIsUnknown) {
		uintptr_t address = reinterpret_cast<uintptr_t>(addr);

		SIZE_T capacity = 0;
		if (m_config.bShowAddress) {
			capacity += m_meminfo->is32Bit() ? 12 : 16;
		}

		capacity += m_config.bytesPerLine * 3;

		if (m_config.bShowAscii)
			capacity += m_config.bytesPerLine;

		QString formattedLine;
		formattedLine.reserve(capacity);

		if (m_config.bShowAddress) {
			m_meminfo->is32Bit()
				? formattedLine.append(QStringLiteral("0x%1: ").arg(static_cast<uint32_t>(address), 8, 16, QLatin1Char('0')))
				: formattedLine.append(QStringLiteral("0x%1: ").arg(address, 12, 16, QLatin1Char('0')));
		}

		if (bIsUnknown) {
			for (SIZE_T i = 0; i < m_config.bytesPerLine; ++i) {
				formattedLine.append("?? ");
			}

			if (m_config.bShowAscii)
				formattedLine.append(QString('.').repeated(m_config.bytesPerLine));

			return formattedLine;
		}
		
		if (!m_memdump)
			return formattedLine;

		for (SIZE_T i = 0; i < m_config.bytesPerLine; ++i) {
			if (i < lineView.size) {
				BYTE b = lineView.data[i];
				formattedLine.append(HEX_DIGITS[b >> 4]);
				formattedLine.append(HEX_DIGITS[b & 0x0F]);
			} else {
				formattedLine.append("??");
			}
			formattedLine.append(" ");
		}

		if (m_config.bShowAscii) {
			for (SIZE_T i = 0; i < m_config.bytesPerLine; ++i) {
				if (i < lineView.size) {
					const BYTE b = lineView.data[i];
					formattedLine.append(IS_PRINTABLE[b] ? static_cast<char>(b) : '.');
				} else {
					formattedLine.append(".");
				} 
			}
		}
		return formattedLine;
	}
	QString Hexview::formatHeaderLine() {
		QString formattedLine;
		if (m_maxDisplayAddress == mem::USERSPACE_END_32BIT)
			formattedLine.append("address     "); // "0x00000000: " length
		else formattedLine.append("address         "); // "0x000000000000: " length

		for (size_t i = 0; i < m_config.bytesPerLine; ++i) {
			formattedLine.append(QStringLiteral("%1 ").arg(i, 2, 16, QLatin1Char('0')).toUpper());
		}

		return formattedLine;
	}
}