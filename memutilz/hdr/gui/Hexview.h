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

#include "mem/Memdump.h"

namespace gui {
	class Hexview : public QAbstractScrollArea {
		Q_OBJECT

	public:
		struct DisplayConfig {
			size_t bytesPerLine = 0x08;
			bool bShowAddress = true;
			bool bShowAscii = true;
			bool bShowRegionBoundaries = true;
		};

		explicit Hexview(QWidget* parent = Q_NULLPTR);

		void setMemdump(const mem::Memdump* memdump);
		void setDisplayConfig(DisplayConfig& config);
		void goToAddress(LPCVOID address);
		void clear();

	protected:
		void paintEvent(QPaintEvent* event) override;	// This does the actual printing to the screen
		void resizeEvent(QResizeEvent* event) override; // To figure out how many lines fit in the new size
		void wheelEvent(QWheelEvent* event) override;	// For converting wheel movement to scroll bar movement

	private:
		static constexpr size_t MAX_UNKNOWN_PATTERN_SIZE = 128;
		static constexpr char HEX_DIGITS[] = "0123456789ABCDEF";
		static constexpr bool IS_PRINTABLE[256] = {
			// 0–31: control chars
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			// 32–126: printable ASCII
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			// 127: DEL (non-printable)
			0,
			// 128–255: extended ASCII (usually non-printable in plain ASCII)
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};


		struct DisplayMetrics {
			int lineHeight{ 0 };
			int charHeight{ 0 };
			int charWidth{ 0 };
			int addressWidth{ 0 };
			int hexWidth{ 0 };
			int asciiWidth{ 0 };
			int totalWidth{ 0 };
			int totalLines{ 0 };
		};

		const mem::Memdump* m_memdump{ nullptr };
		bool m_bIs64Bit{ true };

		DisplayConfig m_config;
		DisplayMetrics m_metrics;
		QFont m_font;

		uintptr_t m_topAddress{ 0 }; // address at the top of view
		size_t m_visibleLines{ 0 };

		// Caching
		std::unordered_map<uintptr_t, QString> m_lineCache;
		char m_unknownPattern[MAX_UNKNOWN_PATTERN_SIZE];
		size_t m_unknownPatternLength{ 0 };

		void getMetrics();
		void updateScrollbars();
		void buildUnknownPattern();
		QString formatLine(LPCVOID addr, bool bIsUnknown);
	};
}