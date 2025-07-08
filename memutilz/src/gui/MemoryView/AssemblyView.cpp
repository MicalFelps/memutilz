#include <QHeaderView>
#include <QWheelEvent>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QInputDialog>

#include "gui/MemoryView/AssemblyView.h"
#include "gui/constants.h"
#include "mem/constants.h"
#include "AssemblyTableModel.h"

namespace gui {
    AssemblyView::AssemblyView(QWidget* parent)
        : QTableView{ parent }
        , m_model{ std::make_unique<AssemblyTableModel>(this) }
    {
        setModel(m_model.get());
        setupTable();
        createContextMenu();

        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setFocusPolicy(Qt::StrongFocus);

        m_metrics.font = QFont();
        m_metrics.font.setFamily("Consolas");
        m_metrics.font.setPointSize(10);
        m_metrics.font.setStyleHint(QFont::Monospace);
        m_metrics.font.setFixedPitch(true);

        setFont(m_metrics.font);

        // Connect Signals
        connect(verticalScrollBar(), &QScrollBar::valueChanged,
            this, &AssemblyView::onVerticalScrollChange);
        connect(selectionModel(), &QItemSelectionModel::currentChanged,
            this, &AssemblyView::onSelectionChange);
    }
    void AssemblyView::setDisplayConfig(const DisplayConfig& config) {
        m_config = config;
        if (m_memory.memdump) // only matters if process attached
            updateVisibleRows();
    }
    void AssemblyView::setMemdump(std::shared_ptr<mem::Memdump> memdump) {
        m_memory.memdump = memdump;

        if (m_memory.memdump) {
            m_disasm.disassembler = std::make_unique<mem::Disassembler>(memdump);

            m_memory.memdump->getMeminfo()->is32Bit()
                ? m_memory.maxDisplayAddress = mem::USERSPACE_END_32BIT
                : m_memory.maxDisplayAddress = mem::USERSPACE_END_64BIT;
            m_memory.topAddress = m_memory.memdump->getMeminfo()->getProgramBase();
            updateVisibleRows();
            updateScrollbars();
        }
    }
    void AssemblyView::goToAddress(LPCVOID address) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(address);

        if (m_disasm.disassembler) {
            m_memory.topAddress = m_disasm.disassembler->alignToInstrStart(addr);
            updateBoundaries();
        } else m_memory.topAddress = addr;

        updateVisibleRows();
        updateScrollbars();
    }
    void AssemblyView::detach() {
        m_disasm.disassembler.reset();
        m_memory.memdump.reset();
        
        updateVisibleRows();
    }

    void AssemblyView::showEvent(QShowEvent* event) {
        QTableView::showEvent(event);
        if (!m_view.initialized) {
            m_view.initialized = true;
            updateMetrics();
            updateVisibleRows();
            updateScrollbars();
        }
    }
    void AssemblyView::resizeEvent(QResizeEvent* event) {
        QTableView::resizeEvent(event);
        if (m_view.initialized) {
            updateVisibleRows();
            updateScrollbars();
        }
    }
    void AssemblyView::wheelEvent(QWheelEvent* event) {
        int degrees = event->angleDelta().y() / 8;
        int steps = degrees / 15;

        int newScrollValue = verticalScrollBar()->value() - steps;
        verticalScrollBar()->setValue(newScrollValue);

        event->accept();
    }
    void AssemblyView::keyPressEvent(QKeyEvent* event) {
        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            followJump();
            break;
        case Qt::Key_C:
            if (event->modifiers() & Qt::ControlModifier) {
                copyToClipboard();
            }
            break;
        case Qt::Key_G:
            if (event->modifiers() & Qt::ControlModifier) {
                m_ui.goToAddressAction->trigger();
            }
            break;
        default:
            QTableView::keyPressEvent(event);
            break;
        }
    }
    void AssemblyView::mousePressEvent(QMouseEvent* event) {
        QTableView::mousePressEvent(event);

        if (event->button() == Qt::LeftButton) {
            QModelIndex index = indexAt(event->pos());
            if (index.isValid()) {
                m_view.selectedRow = index.row();
            }
        }
    }
    void AssemblyView::contextMenuEvent(QContextMenuEvent* event) {
        // This function updates what options are presented based on selection
        QModelIndex index = indexAt(event->pos());
        if (index.isValid()) {
            setCurrentIndex(index);
            m_view.selectedRow = index.row();
            
            m_ui.followJumpAction->setEnabled(
                m_disasm.disassembler->isJumpInstruction(&m_disasm.currentDisassembly[m_view.selectedRow]) ||
                m_disasm.disassembler->isCallInstruction(&m_disasm.currentDisassembly[m_view.selectedRow])
            );

            m_ui.contextMenu->exec(event->globalPos());
        }
    }

    void AssemblyView::onSelectionChange() {
        QModelIndex index = currentIndex();
        if (index.isValid()) {
            m_view.selectedRow = index.row();
        }
    }
    void AssemblyView::onVerticalScrollChange(int value) {
        /*
        This function gets called when we directly modify the scrollbar
        in some way, this includes:

        1. holding it down and moving it
        2. keyboard movement
        3. scrollwheel

        Since we want the scrollwheel to basically never move from the middle,
        we then call updateScrollwheel whenever delta isn't 0
        */

        int instructionDelta = value - m_view.prevScrollValue;
        if (instructionDelta != 0) {
            updateTopAddress(instructionDelta);
            updateVisibleRows();
            updateScrollbars();
        }
    }
    
    void AssemblyView::updateVisibleRows() {
        // This function gets called on every m_topAddress update
        // or every resize event
        if (!m_view.initialized) return;
        m_metrics.visibleRows = (viewport()->height() + m_metrics.rowHeight - 1) / m_metrics.rowHeight;
        m_model->updateVisibleRows(m_memory.topAddress);
    }

    void AssemblyView::updateTopAddress(int delta) {
        // delta is bytes scrolled, with the disassembly, we can convert this to lines
        // we need a seperate alignAddress() function that does everything below

        m_memory.topAddress += delta;
        if (!m_memory.memdump) { return; }

        if (m_memory.topAddress < m_memory.lowerBoundary ||
            (m_memory.topAddress + m_metrics.visibleRows * 15) > m_memory.upperBoundary)
            updateBoundaries();

        if (m_memory.bInReadableMemory) {
            m_memory.topAddress = m_disasm.disassembler->alignToInstrStart(m_memory.topAddress);
        } else {
            m_memory.topAddress += delta;
        }
    }
    void AssemblyView::updateScrollbars() {
        disconnect(verticalScrollBar(), &QScrollBar::valueChanged,
            this, &AssemblyView::onVerticalScrollChange);

        if (m_memory.topAddress < mem::HALF_PAGE_SIZE) {
            int maxScroll = verticalScrollBar()->maximum();
            int value = static_cast<int>(m_memory.topAddress * maxScroll / mem::PAGE_SIZE);
            verticalScrollBar()->setValue(value);
            m_view.prevScrollValue = value;
        }
        else if (m_memory.topAddress > (m_memory.maxDisplayAddress - mem::HALF_PAGE_SIZE)) {
            int maxScroll = verticalScrollBar()->maximum();
            uintptr_t fromEnd = m_memory.maxDisplayAddress - m_memory.topAddress;
            int value = maxScroll - static_cast<int>(fromEnd * maxScroll / mem::PAGE_SIZE);
            verticalScrollBar()->setValue(value);
            m_view.prevScrollValue = value;
        }
        else { // reset to middle (99% of cases)
            int middle = verticalScrollBar()->maximum() / 2;
            verticalScrollBar()->setValue(middle);
            m_view.prevScrollValue = middle;
        }

        connect(verticalScrollBar(), &QScrollBar::valueChanged,
            this, &AssemblyView::onVerticalScrollChange);
    }

    void AssemblyView::setupTable() {
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setAlternatingRowColors(true);
        setWordWrap(false);
        setShowGrid(false);

        // Headers
        horizontalHeader()->setStretchLastSection(true);
        horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        verticalHeader()->setDefaultSectionSize(18);
        verticalHeader()->hide();

        // Column widths
        setColumnWidth(0, 120);
        setColumnWidth(1, 200);
        setColumnWidth(2, 300);
    }
    void AssemblyView::updateBoundaries() {
        // This function tracks where in the visible rows readable and unreadable regions end
        mem::RegionContext ct = m_memory.memdump->getRegionContext(reinterpret_cast<LPCVOID>(m_memory.topAddress));

        if (ct.curr) {
            m_memory.bInReadableMemory = true;

            if (ct.prev) {
                m_memory.lowerBoundary = reinterpret_cast<uintptr_t>(ct.prev->m_original_addr);
            } else m_memory.lowerBoundary = reinterpret_cast<uintptr_t>(ct.curr->m_original_addr);

            if (ct.next) {
                m_memory.upperBoundary = (reinterpret_cast<uintptr_t>(ct.next->m_original_addr) + ct.next->m_size);
            } else m_memory.upperBoundary = (reinterpret_cast<uintptr_t>(ct.curr->m_original_addr) + ct.curr->m_size);

        } else {
            m_memory.bInReadableMemory = false;
            // We still have to check that curr is not the first region in memory, in which case both curr and prev would be null
            if (ct.prev)
                m_memory.lowerBoundary = (reinterpret_cast<uintptr_t>(ct.prev->m_original_addr) + ct.prev->m_size);
            else m_memory.lowerBoundary = 0;

            if (ct.next) {
                m_memory.upperBoundary = reinterpret_cast<uintptr_t>(ct.next->m_original_addr);
            } else m_memory.upperBoundary = m_memory.maxDisplayAddress;
        }
    }
    void AssemblyView::updateMetrics() {
        m_metrics.rowHeight = fontMetrics().height() + 2; // padding
    }
    void AssemblyView::createContextMenu() {
        m_ui.contextMenu = std::make_unique<QMenu>(this);

        m_ui.copyAction = m_ui.contextMenu->addAction("Copy", this, &AssemblyView::copyToClipboard);
        m_ui.copyAction->setShortcut(QKeySequence::Copy);

        m_ui.contextMenu->addSeparator();

        m_ui.followJumpAction = m_ui.contextMenu->addAction("Follow Jump/Call", this, &AssemblyView::followJump);
        m_ui.followJumpAction->setShortcut(Qt::Key_Enter);

        m_ui.goToAddressAction = m_ui.contextMenu->addAction("Go to Address", this, [this]() {
            bool ok;
            QString sAddress = QInputDialog::getText(this, "Go to Address",
                "Fill in the address you want to go to:",
                QLineEdit::Normal, "", &ok);

            if (ok && !sAddress.isEmpty()) {
                bool convertOk;
                uintptr_t address = sAddress.toULongLong(&convertOk, 16);
                if (convertOk) {
                    goToAddress(reinterpret_cast<LPCVOID>(address));
                }
            }
        });
        m_ui.goToAddressAction->setShortcut(QKeySequence("Ctrl+G"));
    }
    void AssemblyView::copyToClipboard() {
        if (!m_memory.memdump || m_view.selectedRow < 0) return;

        QString address = m_model->data(m_model->index(m_view.selectedRow, 0)).toString();
        QString bytes = m_model->data(m_model->index(m_view.selectedRow, 1)).toString();
        QString assembly = m_model->data(m_model->index(m_view.selectedRow, 2)).toString();

        QString text = QString("%1 %2 %3").arg(address, bytes, assembly);
        QApplication::clipboard()->setText(text);
    }
    void AssemblyView::followJump() {
        if (!m_memory.memdump || m_view.selectedRow < 0) return;

        if (m_disasm.disassembler->isJumpInstruction(&m_disasm.currentDisassembly[m_view.selectedRow]) || m_disasm.disassembler->isCallInstruction(&m_disasm.currentDisassembly[m_view.selectedRow])) {
            uintptr_t targetAddr = m_disasm.disassembler->getJumpTarget(&m_disasm.currentDisassembly[m_view.selectedRow]);
            if (targetAddr != 0)
                goToAddress(reinterpret_cast<LPCVOID>(targetAddr));
        }
    }
}