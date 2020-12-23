#include "AggregableProxyModel.h"


AggregableProxyModel::AggregableProxyModel(QObject * parent) :
    AggregableItemModel(parent)
{
}

QModelIndex AggregableProxyModel::index(int row, int column, const QModelIndex & parent) const
{
    const QModelIndex sourceParent = mapToSource(parent);
    const QModelIndex sourceIndex = model()->index(row, column, sourceParent);
    return mapFromSource(sourceIndex);
}

QModelIndex AggregableProxyModel::parent(const QModelIndex & index) const
{
    const QModelIndex sourceIndex = mapToSource(index);
    const QModelIndex sourceParent = sourceIndex.parent();
    return mapFromSource(sourceParent);
}

bool AggregableProxyModel::hasChildren(const QModelIndex & parent) const
{
    return model()->hasChildren(mapToSource(parent));
}

bool AggregableProxyModel::canFetchMore(const QModelIndex & parent) const
{
    return model()->canFetchMore(mapToSource(parent));
}

void AggregableProxyModel::fetchMore(const QModelIndex & parent)
{
    model()->fetchMore(mapToSource(parent));
}

int AggregableProxyModel::rowCount(const QModelIndex & parent) const
{
    return model()->rowCount(mapToSource(parent));
}

int AggregableProxyModel::columnCount(const QModelIndex & parent) const
{
    return model()->columnCount(mapToSource(parent));
}

QVariant AggregableProxyModel::data(const QModelIndex & index, int role) const
{
    return model()->data(mapToSource(index), role);
}

QVariant AggregableProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return model()->headerData(section, orientation, role);
}

void AggregableProxyModel::connectSignals(QAbstractItemModel &model)
{
    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeInserted, this, &AggregableProxyModel::sourceRowsAboutToBeInserted);
    QObject::connect(&model, &QAbstractItemModel::rowsInserted, this, &AggregableProxyModel::sourceRowsInserted);
    QObject::connect(&model, &QAbstractItemModel::layoutAboutToBeChanged, this, &AggregableProxyModel::sourceLayoutAboutToBeChanged);
    QObject::connect(&model, &QAbstractItemModel::layoutChanged, this, &AggregableProxyModel::sourceLayoutChanged);
}

void AggregableProxyModel::sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint)
{
    QList<QPersistentModelIndex> parents;
    parents.reserve(sourceParents.size());
    for (const QPersistentModelIndex &parent : sourceParents) {
        if (!parent.isValid()) {
            parents << QPersistentModelIndex();
            continue;
        }
        const QModelIndex mappedParent = mapFromSource(parent);
        parents << mappedParent;
    }

    emit layoutAboutToBeChanged(parents, hint);

    const auto proxyPersistentIndexes = persistentIndexList();
    for (const QModelIndex &proxyPersistentIndex : proxyPersistentIndexes) {
        proxyIndexes << proxyPersistentIndex;
        const QPersistentModelIndex srcPersistentIndex = mapToSource(proxyPersistentIndex);
        layoutChangePersistentIndexes << srcPersistentIndex;
    }
}

void AggregableProxyModel::sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint)
{
    for (int i = 0; i < proxyIndexes.size(); ++i) {
        changePersistentIndex(proxyIndexes.at(i), mapFromSource(layoutChangePersistentIndexes.at(i)));
    }

    layoutChangePersistentIndexes.clear();
    proxyIndexes.clear();

    QList<QPersistentModelIndex> parents;
    parents.reserve(sourceParents.size());
    for (const QPersistentModelIndex &parent : sourceParents) {
        if (!parent.isValid()) {
            parents << QPersistentModelIndex();
            continue;
        }
        const QModelIndex mappedParent = mapFromSource(parent);
        parents << mappedParent;
    }

    emit layoutChanged(parents, hint);
}

void AggregableProxyModel::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    beginInsertRows(mapFromSource(parent), start, end);
}

void AggregableProxyModel::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    endInsertRows();
}
