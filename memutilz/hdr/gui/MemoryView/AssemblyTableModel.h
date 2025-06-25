#ifndef ASSEMBLYTABLEMODEL_H
#define ASSEMBLYTABLEMODEL_H

#include "../common.h"
#include <vector>
#include <QAbstractTableModel>
#include <capstone/capstone.h>

#include "AssemblyView.h"
#include "../../mem/Memdump.h"

namespace gui {
    class AssemblyTableModel : public QAbstractTableModel {
    public:
        explicit AssemblyTableModel(AssemblyView* asmView) : m_asmView{ asmView } {}
        virtual ~AssemblyTableModel() = default;

        void updateVisibleRows(uintptr_t topAddress, int visibleRows);

        bool isJumpInstruction(int row) const;
        bool isCallInstruction(int row) const;
        uintptr_t getJumpTarget(int row) const;

        // Required overrides
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    private:
        csh m_csh = 0;
        cs_insn* m_insn = nullptr;
        size_t m_count = 0;
        cs_arch m_architecture = CS_ARCH_X86;
        cs_mode m_mode = CS_MODE_64;
        bool m_capstoneInitialized = false;

        AssemblyView* m_asmView;

        bool initializeCapstone();
        void cleanupCapstone();

        void disassembleBytes(const mem::MemoryView& mv);
        QString formatAddress(const cs_insn* insn) const;
        QString formatBytes(const cs_insn* insn) const;
        QString formatAssembly(const cs_insn* insn) const;
        QColor getInstructionColor(const cs_insn* insn) const;
    };
}

#endif