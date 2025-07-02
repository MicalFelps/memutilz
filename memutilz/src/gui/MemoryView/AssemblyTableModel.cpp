#include "AssemblyView.h"
#include "AssemblyTableModel.h"

namespace gui {
	void AssemblyTableModel::updateVisibleRows(uintptr_t topAddress, int visibleRows) {
		beginResetModel();

		if (m_insn) {
			cs_free(m_insn, m_count);
			m_insn = nullptr;
			m_count = 0;
		}

		if (visibleRows <= 0) {
			endResetModel();
			return;
		}

		if (!initializeCapstone()) {
			endResetModel();
			return;
		}

		// 1. fill unknown memory with unknown pattern

		size_t maxBytes = visibleRows * 16; // assume max 16 bytes / instruction
		disassembleBytes(topAddress, maxBytes);

		endResetModel();
	}

	int AssemblyTableModel::rowCount(const QModelIndex& parent) const {
		Q_UNUSED(parent);
		return static_cast<int>(m_count);
	}
	int AssemblyTableModel::columnCount(const QModelIndex& parent) const {
		Q_UNUSED(parent);
		return 3; // Address, Bytes, Assembly
	}
	QVariant AssemblyTableModel::data(const QModelIndex& index, int role) const {
		if (!index.isValid() || index.row() >= static_cast<int>(m_count) || !m_insn){
			return QVariant();
		}

		const cs_insn* insn = &m_insn[index.row()];

		switch (role) {
		case Qt::DisplayRole:
			switch (index.column()) {
			case 0: return formatAddress(insn);
			case 1: return formatBytes(insn);
			case 2: return formatAssembly(insn);
			}
			break;

		case Qt::ForegroundRole:
			if (index.column() == 2) {
				return QBrush(getInstructionColor(insn));
			}
			break;
		case Qt::FontRole: {
			return m_asmView->m_font;
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

	QString AssemblyTableModel::formatAddress(const cs_insn* insn) const {
		if (m_asmView->m_config.m_bShowModuleAddresses && m_asmView->m_meminfo) {
			// show module+offset format, except we need more info
			return QString("%1").arg(insn->address, m_asmView->m_meminfo->is32Bit() ? 8 : 16, 16, QChar('0')).toUpper();
		}
		else {
			return QString("%1").arg(insn->address, m_asmView->m_meminfo->is32Bit() ? 8 : 16, 16, QChar('0')).toUpper();
		}
	}
	QString AssemblyTableModel::formatBytes(const cs_insn* insn) const {
		QString bytes;
		for (int i = 0; i < insn->size; ++i) {
			if (!bytes.isEmpty()) bytes += " ";
			bytes += QString("%1").arg(insn->bytes[i], 2, 16, QChar('0')).toUpper();
		}
		return bytes;
	}
	QString AssemblyTableModel::formatAssembly(const cs_insn* insn) const {
		QString mnemonic = QString::fromUtf8(insn->mnemonic);
		QString operands = QString::fromUtf8(insn->op_str);

		if (operands.isEmpty()) {
			return mnemonic;
		} else {
			return QString("%1 %2").arg(mnemonic, operands);
		}
	}
	QColor AssemblyTableModel::getInstructionColor(const cs_insn* insn) const {
		// This assumes we enabled detail mode via
		// cs_option(m_csh, CS_OPT_DETAIL, CS_OPT_ON);

		if(cs_insn_group(m_asmView->m_dasm->getHandle(), insn, CS_GRP_JUMP)) {
			return QColor(0, 128, 255); // blue
		} else if (cs_insn_group(m_asmView->m_dasm->getHandle(), insn, CS_GRP_CALL)) {
			return QColor(255, 128, 0); // orange
		} else if (cs_insn_group(m_asmView->m_dasm->getHandle(), insn, CS_GRP_RET)) {
			return QColor(255, 0, 128); // pink
		} else if (cs_insn_group(m_asmView->m_dasm->getHandle(), insn, CS_GRP_INT)) {
			return QColor(255, 0, 0); // red 4 interrupts
		}

		return QColor(0, 0, 0); // black for everything else
	}
}