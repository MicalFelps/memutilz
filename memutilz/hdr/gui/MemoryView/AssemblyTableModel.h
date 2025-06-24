#ifndef ASSEMBLYTABLEMODEL_H
#define ASSEMBLYTABLEMODEL_H

#include "../common.h"
#include <QAbstractTableModel>

#include "../../mem/Memdump.h"

namespace gui {
    class AssemblyTableModel : public QAbstractTableModel {
        Q_OBJECT
    public:
        explicit AssemblyTableModel(QWidget* parent) : QAbstractTableModel(parent) {}
        void updateFromAddress(uintptr_t topAddress, int rows, mem::Memdump* memdump);
        void clear() {};

        // Required overrides:
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    };
}

#endif