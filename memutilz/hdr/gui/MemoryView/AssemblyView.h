#ifndef ASSEMBLYVIEW_H
#define ASSEMBLYVIEW_H

#include <capstone/capstone.h>

#include <QTableView>

#include "IMemoryView.h"
#include "AssemblyTableModel.h"
#include "Disassembler.h"

/*
-- CLASS ASSEMBLYVIEW --

This class handles all GUI related tasks, if it needs to do
some calculations for example aligning an address to the start
of a valid instruction, it will do so through TableModel's API.

assemblyView is meant to be responsive to changes in memory, this
means it doesn't do large disassembly because it's assumed that
memory could change at any time. It works by getting a hash of the
memory displayed on screen, specifically the actual bytes. If the
hash changes, we re-disassemble the new memory.
*/

namespace gui {
	class AssemblyView : public QTableView, public IMemoryView {
		Q_OBJECT
	public:
		struct DisplayConfig {
			bool m_bShowModuleAddresses{ true };
			cs_opt_value syntax = CS_OPT_SYNTAX_INTEL;
		};

		explicit AssemblyView(QWidget* parent = nullptr);
		void setDisplayConfig(const DisplayConfig& config);
		virtual void setMemdump(std::shared_ptr<mem::Memdump> memdump) override;
		virtual void goToAddress(LPCVOID address) override;
		virtual void detach() override;
		virtual ~AssemblyView() = default;
	protected:
		void showEvent(QShowEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;
		void wheelEvent(QWheelEvent* event) override;
		void keyPressEvent(QKeyEvent* event) override;
		void mousePressEvent(QMouseEvent* event) override;
		void contextMenuEvent(QContextMenuEvent* event) override;
	private slots:
		void onVerticalScrollChange(int value);
		void onSelectionChange();
	private:
		void updateTopAddress(int delta);
		void updateScrollbars();

		void setupTable();
		void createContextMenu();

		void updateBoundaries();
		void updateMetrics();
		void updateVisibleRows();
		void copyToClipboard();
		void followJump();

		const int SCROLL_RANGE{ 4 };
		const int NEUTRAL_SCROLL_POS{ SCROLL_RANGE / 2 };

		friend class AssemblyTableModel;
		std::unique_ptr<AssemblyTableModel> m_model;

		struct MemoryContext {
			std::shared_ptr<mem::Memdump> memdump{ nullptr };
			uintptr_t topAddress{ 0 };
			uintptr_t maxDisplayAddress{ mem::USERSPACE_END_32BIT };
			uintptr_t lowerBoundary{ 0 };
			uintptr_t upperBoundary{ mem::USERSPACE_END_64BIT };
			bool bInReadableMemory{ false };
		};
		struct DisassemblyContext {
			mem::InsnChunk currentDisassembly;
			std::unique_ptr<mem::Disassembler> disassembler{ nullptr };
		};
		struct ViewState {
			int selectedRow{ 0 };
			bool initialized = false;
		};
		struct DisplayMetrics {
			QFont font;
			int rowHeight{ 20 };
			int visibleRows{ 1 };
		};
		struct UIComponents {
			std::unique_ptr<QMenu> contextMenu{ nullptr };
			QAction* copyAction{ nullptr };
			QAction* followJumpAction{ nullptr };
			QAction* goToAddressAction{ nullptr };
		};

		MemoryContext		m_memory;
		DisassemblyContext	m_disasm;

		DisplayConfig		m_config;
		ViewState			m_view;
		DisplayMetrics		m_metrics;

		UIComponents		m_ui;
	};
}

#endif