#pragma once

#include "AbstractMemoryView.h"

namespace gui {
	class AssemblyView : public AbstractMemoryView {
		Q_OBJECT
	public:
		explicit AssemblyView(QWidget* parent);

		virtual void goToAddress(LPCVOID address) override;

		virtual ~AssemblyView() = default;
	protected:
		void paintEvent(QPaintEvent* event) override;
		void showEvent(QShowEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;
		void wheelEvent(QWheelEvent* event) override;

	private slots:
		virtual void onVerticalScrollChange(int value) override;
	private:
		virtual void updateAddressWidth() override;

		virtual void getMetrics() override;
		virtual void updateScrollbars() override;

		virtual QString formatLine(mem::MemoryView lineView, LPCVOID addr, bool bIsUnknown) override;
		QString formatLine(LPCVOID addr, bool bIsUnknown) { return formatLine({}, addr, bIsUnknown); }
		virtual QString formatHeaderLine() override;

		struct DisplayMetrics {
			int lineHeight{ 0 };
			int charHeight{ 0 };
			int charWidth{ 0 };
			int addressWidth{ 0 };
			int hexWidth{ 0 };
			int opcodeWidth{ 0 };
			int commentWidth{ 0 };
			int totalWidth{ 0 };
		} m_metrics;
	};
}