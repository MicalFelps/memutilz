#ifndef ASSEMBLYVIEW_H
#define ASSEMBLYVIEW_H

#include <capstone/capstone.h>

#include <QTableView>

#include "AbstractMemoryView.h"
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
	class AssemblyView : public QTableView, public AbstractMemoryView {
		Q_OBJECT
	public:
		struct DisplayConfig {
			bool m_bShowModuleAddresses{ true };
			cs_opt_value syntax = CS_OPT_SYNTAX_INTEL;
		};

		explicit AssemblyView(QWidget* parent = nullptr);
		void setDisplayConfig(const DisplayConfig& config);
		virtual void setMemdump(mem::Memdump* memdump) override;
		virtual void goToAddress(LPCVOID address) override;
		virtual void detach() override;
		virtual ~AssemblyView() {if (m_contextMenu) delete m_contextMenu; }
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

		struct DisplayMetrics {
			int rowHeight{ 20 };
		} m_metrics;

		friend class AssemblyTableModel;
		std::unique_ptr<AssemblyTableModel> m_model;
		std::unique_ptr<mem::Disassembler> m_dasm;
		DisplayConfig m_config;
		int m_selectedRow{ 0 };

		int m_prevScrollValue{ 0 };

		uintptr_t m_lowerBoundary{ 0 };
		uintptr_t m_upperBoundary{ 0 };

		QMenu* m_contextMenu{ nullptr };
		QAction* m_copyAction{ nullptr };
		QAction* m_followJumpAction{ nullptr };
		QAction* m_goToAddressAction{ nullptr };
	};
}

#endif