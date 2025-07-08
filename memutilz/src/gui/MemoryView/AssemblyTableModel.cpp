   #include "AssemblyView.h"
#include "AssemblyTableModel.h"

namespace gui {
	AssemblyTableModel::AssemblyTableModel(AssemblyView* asmView)
		: m_asmView{ asmView }
	{}
	void AssemblyTableModel::updateVisibleRows(uintptr_t topAddress) {
		/*
		It doesn't make sense to have disassembler account for unreadable memory, if it's given an address pointing to
		unreadable memory, then it returns an emtpy InsnChunk. It also doesn't make sense to create cs_insn blocks for
		unreadable addresses, each structure is 256 bytes, which adds up to a lot. Instead, we'll account for unreadable
		memory here. Here's how we do that, we check

		if(m_memory.bInReadableMemory){
			if we are in readable memory, awesome possum, we pass top address to the disassembler and check upperBoundary
			to fill in unreadable memory with unknown patter.
		}
		else{
		|	we need an 'offset variable' which we can simply get by subtracting topAddress from upperBoundary, we then pass
			upperBoundary to disassembler instead!
		}

		Lastly we need to adjust the format functions to account for unreadable memory, except WE DON'T. This is STUPID because
		the whole point of unknown patterns is that we don't know what's there. So we return nothing.

		This function just handles the underlying data, every time you call endResetModel, it actually updates based ON the data
		you've updated here!
		*/

		beginResetModel();

		if ( m_asmView->m_metrics.visibleRows <= 0) {
			endResetModel();
			return;
		}

		size_t maxBytes = m_asmView->m_metrics.visibleRows * 16; // assume max 16 bytes / instruction
		uintptr_t upperBoundary = m_asmView->m_memory.upperBoundary;

		if (m_asmView->m_memory.bInReadableMemory) {
			m_offsetToFirstInsn = 0;
			m_asmView->m_disasm.currentDisassembly = m_asmView->m_disasm.disassembler->disassemble(topAddress, maxBytes);
		} else {
			m_offsetToFirstInsn = upperBoundary - topAddress;
			if (m_offsetToFirstInsn < m_asmView->m_metrics.visibleRows)
				m_asmView->m_disasm.currentDisassembly = m_asmView->m_disasm.disassembler->disassemble(upperBoundary, maxBytes);
		}

		endResetModel();
	}

	int AssemblyTableModel::rowCount(const QModelIndex& parent) const {
		Q_UNUSED(parent);
		return m_asmView->m_metrics.visibleRows;
	}
	int AssemblyTableModel::columnCount(const QModelIndex& parent) const {
		Q_UNUSED(parent);
		return 3; // Address, Bytes, Assembly
	}
	QVariant AssemblyTableModel::data(const QModelIndex& index, int role) const {
		if (!index.isValid() || index.row() > m_asmView->m_metrics.visibleRows)
			return QVariant();

		Row row{};
		if (m_asmView->m_memory.bInReadableMemory) { // topAddress readable
			if (index.row() <= m_asmView->m_disasm.currentDisassembly.size()) {
				row.insn = &m_asmView->m_disasm.currentDisassembly[index.row()];
				row.isReadable = true;
				row.address = row.insn.value()->address;
			}
			// int offsetToUnreadable{ index.row() - m_asmView->m_disasm.currentDisassembly.size() };
			// row.address = m_asmView->m_memory.upperBoundary + offsetToUnreadable;
		} else {
			if (index.row() < m_offsetToFirstInsn) {
				row.address = m_asmView->m_memory.topAddress + m_offsetToFirstInsn;
			} else {
				int offsetToReadable{ index.row() - static_cast<int>(m_offsetToFirstInsn) };
				row.insn = &m_asmView->m_disasm.currentDisassembly[offsetToReadable];
				row.isReadable = true;
				row.address = row.insn.value()->address;
			}
		}

		switch (role) {
		case Qt::DisplayRole:
			switch (index.column()) {
			case 0: return formatAddress(row);
			case 1: return formatBytes(row);
			case 2: return formatAssembly(row);
			}
			break;

		case Qt::ForegroundRole:
			if (index.column() == 2) {
				return QBrush(getInstructionColor(row));
			}
			break;
		case Qt::FontRole: {
			return m_asmView->m_metrics.font;
		}

		case Qt::TextAlignmentRole:
			if (index.column() == 0) {
				return QVariant::fromValue(Qt::AlignRight | Qt::AlignVCenter);
			}
			return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
		}
		return QVariant();
	}
	QVariant AssemblyTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
			switch (section) {
			case 0: return "Address";
			case 1: return "Bytes";
			case 2: return "Assembly";
			}
		}
		return QVariant();
	}

	// --- PRIVATE ---

	QString AssemblyTableModel::formatAddress(const Row& row) const {
		if (m_asmView->m_config.m_bShowModuleAddresses) {
			// show module+offset format, except we need more info
			return QString("%1").arg(row.address, m_asmView->m_memory.maxDisplayAddress == mem::USERSPACE_END_32BIT ? 8 : 16, 16, QChar('0')).toUpper();
		}
		else {
			return QString("%1").arg(row.address, m_asmView->m_memory.maxDisplayAddress == mem::USERSPACE_END_32BIT ? 8 : 16, 16, QChar('0')).toUpper();
		}
	}
	QString AssemblyTableModel::formatBytes(const Row& row) const {
		QString bytes;
		if (!row.isReadable) bytes.append("??");
		else {
			for (int i = 0; i < row.insn.value()->size; ++i) {
				if (!bytes.isEmpty()) bytes += " ";
				bytes += QString("%1").arg(row.insn.value()->bytes[i], 2, 16, QChar('0')).toUpper();
			}
		}
		return bytes;
	}
	QString AssemblyTableModel::formatAssembly(const Row& row) const {
		if (!row.isReadable) return QString();

		QString mnemonic = QString::fromUtf8(row.insn.value()->mnemonic);
		QString operands = QString::fromUtf8(row.insn.value()->op_str);

		if (operands.isEmpty()) {
			return mnemonic;
		} else {
			return QString("%1 %2").arg(mnemonic, operands);
		}
	}
	QColor AssemblyTableModel::getInstructionColor(const Row& row) const {
		// This assumes we enabled detail mode via
		// cs_option(m_csh, CS_OPT_DETAIL, CS_OPT_ON);

		if (!row.isReadable) return QColor(0, 0, 0);

		if(cs_insn_group(m_asmView->m_disasm.disassembler->getHandle(), row.insn.value(), CS_GRP_JUMP)) {
			return QColor(0, 128, 255); // blue
		} else if (cs_insn_group(m_asmView->m_disasm.disassembler->getHandle(), row.insn.value(), CS_GRP_CALL)) {
			return QColor(255, 128, 0); // orange
		} else if (cs_insn_group(m_asmView->m_disasm.disassembler->getHandle(), row.insn.value(), CS_GRP_RET)) {
			return QColor(255, 0, 128); // pink
		} else if (cs_insn_group(m_asmView->m_disasm.disassembler->getHandle(), row.insn.value(), CS_GRP_INT)) {
			return QColor(255, 0, 0); // red 4 interrupts
		}

		return QColor(255, 255, 255); // black for everything else
	}
}