#include <QHeaderView>
#include <QWheelEvent>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QInputDialog>

#include "gui/MemoryView/AssemblyView.h"
#include "AssemblyTableModel.h"



namespace gui {
    explicit AssemblyView::AssemblyView(QWidget* parent)
        : QTableView{ parent }
        , m_model{ new AssemblyTableModel(this) }
    {
        setModel(m_model);
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
        if (m_model && m_memdump) {
            m_model->updateVisibleRows(m_topAddress, m_visibleRows); // update data based on new config
            viewport()->update();
        }     
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
        if(m_model)
            m_model->updateVisibleRows(m_topAddress, m_visibleRows);
    }

    void AssemblyView::goToAddress(LPCVOID address) {
    }

    void AssemblyView::detach() {
        m_memdump = nullptr;
        m_meminfo = nullptr;
        
        if (m_model) m_model->updateVisibleRows(m_topAddress, m_visibleRows);
        viewport()->update();
    }
    void AssemblyView::showEvent(QShowEvent* event) {
        QTableView::showEvent(event);
        if (!m_initialized) {
            m_initialized = true;
            updateVisibleRows();
        }
    }
    void AssemblyView::resizeEvent(QResizeEvent* event) {
        QTableView::resizeEvent(event);
        if (m_initialized) {
            updateVisibleRows();
        }
    }

    void AssemblyView::wheelEvent(QWheelEvent* event) {
        int degrees = event->angleDelta().y() / 8;
        int steps = degrees / 15;

        int newScrollValue = verticalScrollBar()->value() - steps;

        // yeah I have no idea how to do this shit

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
        QModelIndex index = indexAt(event->pos());
        if (index.isValid()) {
            m_selectedRow = index.row();
            
            m_followJumpAction->setEnabled(
                m_model->isJumpInstruction(m_selectedRow) ||
                m_model->isCallInstruction(m_selectedRow)
            );

            m_contextMenu->exec(event->globalPos());
        }
    }

    void AssemblyView::onVerticalScrollChange(int value) {
        m_visibleRows = viewport()->height() / rowHeight(0);
        int topRow = value;
        int bottomRow = topRow + m_visibleRows;

        m_model->ensureRowsLoaded(topRow, bottomRow + 10);
        // 1. get the new address and align it to a valid instruction
        // 2. updateVisibleRows
    }


    void AssemblyView::onSelectionChange() {
        QModelIndex current = currentIndex();
        if (current.isValid()) {
            m_selectedRow = current.row();
        }
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

        setContextMenuPolicy(Qt::CustomContextMenu);
    }
    void AssemblyView::updateVisibleRows() {
        int rowHeight = fontMetrics().height();
        if (rowHeight <= 0) rowHeight = 20;

        m_visibleRows = (viewport()->height() / rowHeight);

        if(m_model)
            m_model->updateVisibleRows(m_topAddress, m_visibleRows);
    }
    void AssemblyView::createContextMenu() {
        m_contextMenu = new QMenu(this);

        m_copyAction = m_contextMenu->addAction("Copy", this, &AssemblyView::copyToClipboard);
        m_copyAction->setShortcut(QKeySequence::Copy);

        m_contextMenu->addSeparator();

        m_followJumpAction = m_contextMenu->addAction("Follow Jump/Call", this, &AssemblyView::followJump);
        m_followJumpAction->setShortcut(Qt::Key_Enter);

        m_goToAddressAction = m_contextMenu->addAction("Follow Jump/Call", this, [this]() {
            bool ok;
            QString addressStr = QInputDialog::getText(this, "Go to Address",
                "Fill in the address you want to go to:",
                QLineEdit::Normal, "", &ok);

            if (ok && !addressStr.isEmpty()) {
                bool convertOk;
                uintptr_t address = addressStr.toULongLong(&convertOk, 16);
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