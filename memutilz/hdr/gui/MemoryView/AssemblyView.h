#ifndef ASSEMBLYVIEW_H
#define ASSEMBLYVIEW_H

#include <capstone/capstone.h>

#include <QTableView>

#include "AbstractMemoryView.h"
#include "AssemblyTableModel.h"

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
		virtual ~AssemblyView() { delete m_model; }
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
		void setupTable();
		void createContextMenu();
		void updateVisibleRows();
		void copyToClipboard();
		void followJump();

		friend class AssemblyTableModel;
		AssemblyTableModel* m_model;
		DisplayConfig m_config;
		int m_selectedRow{ 0 };

		QMenu* m_contextMenu{ nullptr };
		QAction* m_copyAction{ nullptr };
		QAction* m_followJumpAction{ nullptr };
		QAction* m_goToAddressAction{ nullptr };
	};
}

#endif