#include "gui/MemoryView/AssemblyTableModel.h"

namespace gui {
	void AssemblyTableModel::updateFromAddress(uintptr_t topAddress, int rows, mem::Memdump* memdump) {
	}

	int AssemblyTableModel::rowCount(const QModelIndex& parent) const {
		return 0;
	}
	int AssemblyTableModel::columnCount(const QModelIndex& parent) const {
		return 0;
	}
	QVariant AssemblyTableModel::data(const QModelIndex& index, int role) const {
		return QVariant();
	}
	QVariant AssemblyTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
		return QVariant();
	}
}