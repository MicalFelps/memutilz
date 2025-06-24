#ifndef ASSEMBLYVIEW_H
#define ASSEMBLYVIEW_H

#include <QTableView>

#include "AbstractMemoryView.h"
#include "AssemblyTableModel.h"

namespace gui {
	class AssemblyView : public QTableView, public AbstractMemoryView {
		Q_OBJECT
	public:
		explicit AssemblyView(QWidget* parent = nullptr);
		virtual void setMemdump(mem::Memdump* memdump) override;
		virtual void goToAddress(LPCVOID address) override;
		virtual void clear() override; // Detach from process
		virtual ~AssemblyView() = default;
	protected:
		void showEvent(QShowEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;
		void wheelEvent(QWheelEvent* event) override;
	private slots:
		void onTableScrolled(int value);
	private:
		void setupTable();
		void updateVisibleRows();

		AssemblyTableModel* m_model;
		bool m_bShowRelativeAddress{ false };
		int m_rowHeight{ 20 };
	};
}

#endif