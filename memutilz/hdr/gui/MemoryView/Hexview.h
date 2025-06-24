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
#include "gui/MemoryView/AbstractMemoryView.h"
#include "gui/constants.h"

#include "mem/Meminfo.h"
#include "mem/Memdump.h"
#include "mem/constants.h"

namespace gui {
	class Hexview : public QAbstractScrollArea, public AbstractMemoryView {
		Q_OBJECT

	public:
		struct DisplayConfig {
			int bytesPerLine = 0x20;
			bool bShowAddress = true;
			bool bShowAscii = true;
			bool bShowRegionBoundaries = false;
		};

		explicit Hexview(QWidget* parent = Q_NULLPTR);

		virtual void setMemdump(mem::Memdump* memdump) override;
		void setDisplayConfig(DisplayConfig& config);
		virtual void goToAddress(LPCVOID address) override;
		virtual void clear() override { m_memdump = nullptr; viewport()->update(); }

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
	};
}

#endif