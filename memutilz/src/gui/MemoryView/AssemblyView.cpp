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

        m_font = QFont();
        m_font.setFamily("Consolas");
        m_font.setPointSize(10);
        m_font.setStyleHint(QFont::Monospace);
        m_font.setFixedPitch(true);

        QStringList fontFamilies = { "Consolas", "Courier New", "Monaco", "monospace" };
        for (const QString& family : fontFamilies) {
            m_font.setFamily(family);
            QFontInfo info(m_font);
            if (info.fixedPitch()) {
                break;
            }
        }

        setFont(m_font);

        // Connect Signals
        connect(verticalScrollBar(), &QScrollBar::valueChanged,
            this, &AssemblyView::onVerticalScrollChange);
        connect(selectionModel(), &QItemSelectionModel::currentChanged,
            this, &AssemblyView::onSelectionChange);
    }
    void AssemblyView::setDisplayConfig(const DisplayConfig& config) {
        m_config = config;
        if (m_memdump) // only matters if process attached
            updateVisibleRows();
    }
    void AssemblyView::setMemdump(mem::Memdump* memdump) {
        m_memdump = memdump;

        if (m_memdump) {
            m_dasm = std::make_unique<mem::Disassembler>(m_memdump);

            m_meminfo = m_memdump->getMeminfo();
            m_meminfo->is32Bit()
                ? m_maxDisplayAddress = mem::USERSPACE_END_32BIT
                : m_maxDisplayAddress = mem::USERSPACE_END_64BIT;
            m_topAddress = m_meminfo->getProgramBase();
            updateVisibleRows();
            updateScrollbars();
        }
    }
    void AssemblyView::goToAddress(LPCVOID address) {
        if (!address) return;
        uintptr_t addr = reinterpret_cast<uintptr_t>(address);

        m_topAddress = m_dasm->alignToInstrStart(addr);
        updateBoundaries();
        updateVisibleRows();
        updateScrollbars();
    }
    void AssemblyView::detach() {
        m_memdump = nullptr;
        m_meminfo = nullptr;
        
        updateVisibleRows();
    }

    void AssemblyView::showEvent(QShowEvent* event) {
        QTableView::showEvent(event);
        if (!m_initialized) {
            m_initialized = true;
            updateMetrics();
            updateVisibleRows();
            updateScrollbars();
        }
    }
    void AssemblyView::resizeEvent(QResizeEvent* event) {
        QTableView::resizeEvent(event);
        if (m_initialized) {
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
                m_goToAddressAction->trigger();
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
                m_selectedRow = index.row();
            }
        }
    }
    void AssemblyView::contextMenuEvent(QContextMenuEvent* event) {
        // This function updates what options are presented based on selection
        QModelIndex index = indexAt(event->pos());
        if (index.isValid()) {
            setCurrentIndex(index);
            m_selectedRow = index.row();
            
            m_followJumpAction->setEnabled(
                m_model->isJumpInstruction(m_selectedRow) ||
                m_model->isCallInstruction(m_selectedRow)
            );

            m_contextMenu->exec(event->globalPos());
        }
    }

    void AssemblyView::onSelectionChange() {
        QModelIndex index = currentIndex();
        if (index.isValid()) {
            m_selectedRow = index.row();
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

        int instructionDelta = value - m_prevScrollValue;
        if (instructionDelta != 0) {
            updateTopAddress(instructionDelta);
            updateVisibleRows();
            updateScrollbars();
        }
    }
    
    void AssemblyView::updateVisibleRows() {
        // This function gets called on every m_topAddress update
        if (!m_initialized) return;

        m_visibleRows = (viewport()->height() + m_metrics.rowHeight - 1) / m_metrics.rowHeight;

        if (m_topAddress < m_lowerBoundary ||
            (m_topAddress + m_visibleRows * 15) > m_upperBoundary)
            updateBoundaries();

        m_model->updateVisibleRows(m_topAddress, m_visibleRows);
    }

    void AssemblyView::updateTopAddress(int delta) {
        // delta is bytes scrolled, with the disassembly, we can convert this to lines
        // we need a seperate alignAddress() function that does everything below

        if (!m_memdump) { m_topAddress + delta; return; }
        // else { alignTopAddress(); }
    }
    void AssemblyView::updateScrollbars() {
        disconnect(verticalScrollBar(), QScrollBar::valueChanged,
            this, &AssemblyView::onVerticalScrollChange);

        if (m_topAddress < mem::HALF_PAGE_SIZE) {
            int maxScroll = verticalScrollBar()->maximum();
            int value = static_cast<int>(m_topAddress * maxScroll / mem::PAGE_SIZE);
            verticalScrollBar()->setValue(value);
            m_prevScrollValue = value;
        }
        else if (m_topAddress > (m_maxDisplayAddress - mem::HALF_PAGE_SIZE)) {
            int maxScroll = verticalScrollBar()->maximum();
            uintptr_t fromEnd = m_maxDisplayAddress - m_topAddress;
            int value = maxScroll - static_cast<int>(fromEnd * maxScroll / mem::PAGE_SIZE);
            verticalScrollBar()->setValue(value);
            m_prevScrollValue = value;
        }
        else { // reset to middle (99% of cases)
            int middle = verticalScrollBar()->maximum() / 2;
            verticalScrollBar()->setValue(middle);
            m_prevScrollValue = middle;
        }

        connect(verticalScrollBar(), QScrollBar::valueChanged,
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
        // This function update boundaries between readable and unreadable memory regions

        mem::RegionContext ct = m_memdump->getRegionContext(reinterpret_cast<LPCVOID>(m_topAddress));\

        if (ct.curr) {
            if (ct.prev) {
                m_lowerBoundary = reinterpret_cast<uintptr_t>(ct.prev->m_original_addr);
            } else m_lowerBoundary = reinterpret_cast<uintptr_t>(ct.curr->m_original_addr);

            if (ct.next) {
                m_upperBoundary = (reinterpret_cast<uintptr_t>(ct.next->m_original_addr) + ct.next->m_size);
            } else m_upperBoundary = (reinterpret_cast<uintptr_t>(ct.curr->m_original_addr) + ct.curr->m_size);

        } else {
            // We still have to check that curr is not the first region in memory, in which case both curr and prev would be null
            if (ct.prev)
                m_lowerBoundary = (reinterpret_cast<uintptr_t>(ct.prev->m_original_addr) + ct.prev->m_size);
            else m_lowerBoundary = 0;

            if (ct.next) {
                m_upperBoundary = reinterpret_cast<uintptr_t>(ct.next->m_original_addr);
            } else m_upperBoundary = m_maxDisplayAddress;
        }
    }
    void AssemblyView::updateMetrics() {
        m_metrics.rowHeight = fontMetrics().height() + 2; // padding
    }
    void AssemblyView::createContextMenu() {
        m_contextMenu = new QMenu(this);

        m_copyAction = m_contextMenu->addAction("Copy", this, &AssemblyView::copyToClipboard);
        m_copyAction->setShortcut(QKeySequence::Copy);

        m_contextMenu->addSeparator();

        m_followJumpAction = m_contextMenu->addAction("Follow Jump/Call", this, &AssemblyView::followJump);
        m_followJumpAction->setShortcut(Qt::Key_Enter);

        m_goToAddressAction = m_contextMenu->addAction("Go to Address", this, [this]() {
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
        m_goToAddressAction->setShortcut(QKeySequence("Ctrl+G"));
    }
    void AssemblyView::copyToClipboard() {
        if (!m_memdump || m_selectedRow < 0) return;

        QString address = m_model->data(m_model->index(m_selectedRow, 0)).toString();
        QString bytes = m_model->data(m_model->index(m_selectedRow, 1)).toString();
        QString assembly = m_model->data(m_model->index(m_selectedRow, 2)).toString();

        QString text = QString("%1 %2 %3").arg(address, bytes, assembly);
        QApplication::clipboard()->setText(text);
    }
    void AssemblyView::followJump() {
        if (!m_memdump || m_selectedRow < 0) return;

        if (m_model->isJumpInstruction(m_selectedRow) || m_model->isCallInstruction(m_selectedRow)) {
            uintptr_t targetAddr = m_model->getJumpTarget(m_selectedRow);
            if (targetAddr != 0)
                goToAddress(reinterpret_cast<LPCVOID>(targetAddr));
        }
    }
}