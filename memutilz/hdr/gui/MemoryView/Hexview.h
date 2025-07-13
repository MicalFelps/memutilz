#ifndef HEXVIEW_H
#define HEXVIEW_H

#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QPainter>
#include <QFontMetrics>
#include <QWheelEvent>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "gui/common.h"
#include "gui/MemoryView/IMemoryView.h"
#include "gui/constants.h"

#include "mem/Meminfo.h"
#include "mem/Memdump.h"
#include "mem/constants.h"

namespace gui {
	class Hexview : public QAbstractScrollArea, public IMemoryView {
		Q_OBJECT

	public:
		struct DisplayConfig {
			int bytesPerLine = 0x8;
			bool bShowAddress = true;
			bool bShowAscii = true;
		};

		explicit Hexview(QWidget* parent = Q_NULLPTR);

		virtual void setMemdump(std::shared_ptr<mem::Memdump> memdump) override;
		void updateDisplayConfig();
		virtual void goToAddress(LPCVOID address) override;
		virtual void detach() override { m_memdump = nullptr; viewport()->update(); }

		virtual ~Hexview() = default;
	protected:
		void paintEvent(QPaintEvent* event) override;	// This does the actual printing to the screen
		void showEvent(QShowEvent* event) override;
		void resizeEvent(QResizeEvent* event) override; // To figure out how many lines fit in the new size
		void wheelEvent(QWheelEvent* event) override;	// For converting wheel movement to scroll bar movement
	private slots:
		void onVerticalScrollChange(int value);
	private:
		void updateAddressWidth();
		void updateScrollbars();
		void getMetrics();

		QString formatLine(mem::MemoryView lineView, LPCVOID addr, bool bIsUnknown);
		QString formatLine(LPCVOID addr, bool bIsUnknown) { return formatLine({}, addr, bIsUnknown); }
		QString formatHeaderLine();

		struct DisplayMetrics {
			int lineHeight{ 0 };
			int charHeight{ 0 };
			int charWidth{ 0 };
			int addressWidth{ 0 };
			int hexWidth{ 0 };
			int asciiWidth{ 0 };
			int totalWidth{ 0 };
		} m_metrics;

		DisplayConfig m_config;

		std::shared_ptr<mem::Memdump> m_memdump{ nullptr };
		const mem::Meminfo* m_meminfo{ nullptr };
		uintptr_t m_maxDisplayAddress{ mem::USERSPACE_END_32BIT };
		uintptr_t m_topAddress{ 0 }; // address at the top of view
		QFont m_font;
		bool m_initialized = false;
		int m_visibleRows{ 1 };
	};
}

#endif