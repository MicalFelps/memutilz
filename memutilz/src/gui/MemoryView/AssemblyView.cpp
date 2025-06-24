#include <QHeaderView>

#include "gui/MemoryView/AssemblyView.h"
#include "AssemblyTableModel.h"

namespace gui {
    AssemblyView::AssemblyView(QWidget* parent)
        : QTableView(parent)
    {
        m_model = new AssemblyTableModel(this);
        setModel(m_model);
        setupTable();

        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setFocusPolicy(Qt::StrongFocus);

        m_font = QFont("Consolas", 10);
        if (!m_font.exactMatch()) {
            m_font = QFont("Courier New", 10);
        }

        m_font.setStyleHint(QFont::Monospace);
        m_font.setFixedPitch(true);
        setFont(m_font);
    }

    void AssemblyView::setMemdump(mem::Memdump* memdump) {
        m_memdump = memdump;

        if (m_memdump) {
            m_meminfo = m_memdump->getMeminfo();
            m_meminfo->is32Bit()
                ? m_maxDisplayAddress = mem::USERSPACE_END_32BIT
                : m_maxDisplayAddress = mem::USERSPACE_END_64BIT;
            m_topAddress = m_meminfo->get_program_base();
        }
        updateVisibleRows();
    }
    void AssemblyView::goToAddress(LPCVOID address) {
        /*
        if (!m_memdump) return;

        uintptr_t addr = reinterpret_cast<uintptr_t>(address);
        m_topAddress = addr;
        updateVisibleRows();

        Scroll to show the target address at the top

        verticalScrollBar()->setValue(0);
        */
    }
    void AssemblyView::clear() {
        m_memdump = nullptr;
        m_meminfo = nullptr;
        m_model->clear();
    }

    void AssemblyView::showEvent(QShowEvent* event) {
        QTableView::showEvent(event);
        if (!m_initialized) {
            m_initialized = true;
            m_rowHeight = rowHeight(0);
            if (m_memdump) {
                updateVisibleRows();
            }
        }
    }
    void AssemblyView::resizeEvent(QResizeEvent* event) {
        QTableView::resizeEvent(event);
        if (m_initialized) {
            updateVisibleRows();
        }
    }
    void AssemblyView::wheelEvent(QWheelEvent* event) {
        QTableView::wheelEvent(event);
    }

    void AssemblyView::onTableScrolled(int value) {
        // 1. get the new address and align it to a valid instruction
        // 2. updateVisibleRows
    }

    void AssemblyView::setupTable() {
        setAlternatingRowColors(true);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setShowGrid(false);
        setWordWrap(false);

        // Headers
        horizontalHeader()->setStretchLastSection(true);
        horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        verticalHeader()->setVisible(false);

        // column widths
        setColumnWidth(0, 100); // Address
        setColumnWidth(1, 120); // Bytes
        setColumnWidth(2, 200); // Assembly

        connect(verticalScrollBar(), &QScrollBar::valueChanged,
                this, &AssemblyView::onTableScrolled);
    }
    void AssemblyView::updateVisibleRows() {
        if (!m_initialized) return;

    }
}