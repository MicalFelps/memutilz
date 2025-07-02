#ifndef ASSEMBLYTABLEMODEL_H
#define ASSEMBLYTABLEMODEL_H

#include "../common.h"
#include <vector>
#include <QAbstractTableModel>
#include <capstone/capstone.h>

#include "../../mem/Memdump.h"

namespace gui {
    class AssemblyView;

    class AssemblyTableModel : public QAbstractTableModel {
    public:
        explicit AssemblyTableModel(AssemblyView* asmView) : m_asmView{ asmView } {}
        virtual ~AssemblyTableModel() = default;

        void updateVisibleRows(uintptr_t topAddress, int visibleRows);

        // Required overrides
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    private:
        AssemblyView* m_asmView = nullptr;

        QString formatAddress(const cs_insn* insn) const;
        QString formatBytes(const cs_insn* insn) const;
        QString formatAssembly(const cs_insn* insn) const;
        QColor getInstructionColor(const cs_insn* insn) const;
    };
}

#endif