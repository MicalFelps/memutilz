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

		size_t maxBytes = visibleRows * 16; // assume max 16 bytes / instruction
		mem::MemoryView mv = m_asmView->m_memdump->readBytesAt(
			reinterpret_cast<LPCVOID>(topAddress), maxBytes);
		disassembleBytes(mv);

		endResetModel();
	}

	bool AssemblyTableModel::isJumpInstruction(int row) const {
		if (row < 0 || row >= static_cast<int>(m_count) || !m_insn)
			return false;
		return cs_insn_group(m_csh, &m_insn[row], CS_GRP_JUMP);
	}
	bool AssemblyTableModel::isCallInstruction(int row) const {
		if (row < 0 || row >= static_cast<int>(m_count) || !m_insn)
			return false;
		return cs_insn_group(m_csh, &m_insn[row], CS_GRP_CALL);
	}
	uintptr_t AssemblyTableModel::getJumpTarget(int row) const {
		if (row < 0 || row >= static_cast<int>(m_count) || !m_insn)
			return 0;

		const cs_insn* insn = &m_insn[row];

		if(!cs_insn_group(m_csh, insn, CS_GRP_JUMP) && !cs_insn_group(m_csh, insn, CS_GRP_CALL))
			return 0;

		// check operands
		cs_detail* detail = insn->detail;
		if (!detail) return 0;

		for (int i = 0; i < detail->x86.op_count; ++i) {
			if (detail->x86.operands[i].type & X86_OP_IMM) {
				return static_cast<uintptr_t>(detail->x86.operands[i].imm);
			}
		}
		return 0;
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

	bool AssemblyTableModel::initializeCapstone() {
		if (m_capstoneInitialized) {
			cleanupCapstone();
		}

		if (m_asmView->m_meminfo) {
			m_architecture = CS_ARCH_X86;
			m_mode = m_asmView->m_meminfo->is32Bit() ? CS_MODE_32 : CS_MODE_64;
		}

		cs_err e = cs_open(m_architecture, m_mode, &m_csh);
		if (e != CS_ERR_OK) {
			return false;
		}

		// syntax options (IMPORTANT)
		cs_option(m_csh, CS_OPT_SYNTAX, m_asmView->m_config.syntax);
		cs_option(m_csh, CS_OPT_DETAIL, CS_OPT_ON);

		m_capstoneInitialized = true;
		return true;
	}
	void AssemblyTableModel::cleanupCapstone() {
		if (m_insn) {
			cs_free(m_insn, m_count);
			m_insn = nullptr;
			m_count = 0;
		}

		if (m_capstoneInitialized) {
			cs_close(&m_csh);
			m_csh = 0;
			m_capstoneInitialized = false;
		}
	}

	void AssemblyTableModel::disassembleBytes(const mem::MemoryView& mv) {

	}
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

		if(cs_insn_group(m_csh, insn, CS_GRP_JUMP)) {
			return QColor(0, 128, 255); // blue
		} else if (cs_insn_group(m_csh, insn, CS_GRP_CALL)) {
			return QColor(255, 128, 0); // orange
		} else if (cs_insn_group(m_csh, insn, CS_GRP_RET)) {
			return QColor(255, 0, 128); // pink
		} else if (cs_insn_group(m_csh, insn, CS_GRP_INT)) {
			return QColor(255, 0, 0); // red 4 interrupts
		}

		return QColor(0, 0, 0); // black for everything else
	}
}