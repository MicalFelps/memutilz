#ifndef ASSEMBLYTABLEMODEL_H
#define ASSEMBLYTABLEMODEL_H

#include "../common.h"
#include <vector>
#include <QAbstractTableModel>
#include <capstone/capstone.h>

#include "../../mem/Disassembler.h"
#include "../../mem/Memdump.h"

namespace gui {
    class AssemblyView;

    class AssemblyTableModel : public QAbstractTableModel {
    public:
        explicit AssemblyTableModel(AssemblyView* asmView);
        virtual ~AssemblyTableModel() = default;

        void updateVisibleRows(uintptr_t topAddress);

        // Required overrides
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    private:
        struct Row {
            std::optional<const cs_insn*> insn;
            uintptr_t address{ 0 };
            bool isReadable{ false };
        };

        AssemblyView* m_asmView;
        size_t m_rowOffsetToBoundary{0};

        QString formatAddress(const Row& row) const;
        QString formatBytes(const Row& row) const;
        QString formatAssembly(const Row& row) const;
        QColor getInstructionColor(const Row& row) const;
    };
}

#endif