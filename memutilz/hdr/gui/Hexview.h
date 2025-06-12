#pragma once

#include "gui/common.h"

#include "mem/Memdump.h"

namespace gui {
	class Hexview : public QAbstractScrollArea {
		Q_OBJECT

	public:
		struct DisplayConfig {
			size_t bytesPerLine = 16;
			bool bShowAddress = true;
			bool bShowAscii = true;
			bool bShowRegionBoundaries = true;
		};

		explicit Hexview(QWidget* parent = Q_NULLPTR);

		void setMemdump(const mem::Memdump* memdump);
		void setDisplayConfig(const DisplayConfig& config);
		void goToAddress(LPCVOID address);
		void clear();

	protected:
		void paintEvent(QPaintEvent* event) override;	// This does the actual printing to the screen
		void resizeEvent(QResizeEvent* event) override; // To figure out how many lines fit in the new size
		void wheelEvent(QWheelEvent* event) override;	// For converting wheel movement to scroll bar movement

	private:
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
		};

		const mem::Memdump* m_memdump{ nullptr };

		DisplayConfig m_config;
		DisplayMetrics m_metrics;
		QFont m_font;

		LPCVOID m_topAddress{ nullptr }; // address at the top of view
		size_t m_visibleLines{ 0 };

		void getMetrics();
		void updateScrollbars();
		QString formatLine(const BYTE* data, size_t length, uintptr_t baseAddress) const;
		BYTE* readBytesAt(LPCVOID addr) const;
		const mem::MemoryRegion* findRegionForAddress(LPCVOID address) const;

	};
}
