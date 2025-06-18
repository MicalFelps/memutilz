#pragma once

#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QPainter>
#include <QFontMetrics>
#include <QWheelEvent>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "gui/common.h"

#include "mem/Meminfo.h"
#include "mem/Memdump.h"
#include "mem/constants.h"

namespace gui {
	class Hexview : public QAbstractScrollArea {
		Q_OBJECT

	public:
		struct DisplayConfig {
			int bytesPerLine = 0x20;
			bool bShowAddress = true;
			bool bShowAscii = true;
			bool bShowRegionBoundaries = false;
		} m_config;

		explicit Hexview(QWidget* parent = Q_NULLPTR);

		void setMemdump(mem::Memdump* memdump);
		void setDisplayConfig(DisplayConfig& config);
		void goToAddress(LPCVOID address);
		void clear(); // detach from process

		virtual ~Hexview() = default;
	protected:
		void paintEvent(QPaintEvent* event) override;	// This does the actual printing to the screen
		void showEvent(QShowEvent* event) override;
		void resizeEvent(QResizeEvent* event) override; // To figure out how many lines fit in the new size
		void wheelEvent(QWheelEvent* event) override;	// For converting wheel movement to scroll bar movement
	private slots:
		void onVerticalScrollChange(int value);
	private:
		void getMetrics();
		void updateScrollbars();
		QString formatLine(mem::MemoryView lineView, LPCVOID addr, bool bIsUnknown);
		QString formatLine(LPCVOID addr, bool bIsUnknown) { return formatLine({}, addr, bIsUnknown); }

		static constexpr char HEX_DIGITS[] = "0123456789ABCDEF";
		static constexpr bool IS_PRINTABLE[256] = {
			// 0-31: control chars
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			// 32-126: printable ASCII
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			// 127: DEL (non-printable)
			0,
			// 128-255: extended ASCII (usually non-printable in plain ASCII)
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		static constexpr int SCROLL_RANGE = 0x100000;
		static constexpr int SCROLL_RANGE_HALF = SCROLL_RANGE / 2;

		struct DisplayMetrics {
			int lineHeight{ 0 };
			int charHeight{ 0 };
			int charWidth{ 0 };
			int addressWidth{ 0 };
			int hexWidth{ 0 };
			int asciiWidth{ 0 };
			int totalWidth{ 0 };
			uintptr_t totalLines{ 1 };
		} m_metrics;

		mem::Memdump* m_memdump{ nullptr };
		const mem::Meminfo* m_meminfo{ nullptr };

		uintptr_t m_maxDisplayAddress{ mem::USERSPACE_END_32BIT };

		QFont m_font;
		bool initialized = false;

		uintptr_t m_topAddress{ 0 }; // address at the top of view
		int m_visibleLines{ 1 };
	};
}