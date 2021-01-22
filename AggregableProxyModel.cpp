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
    const QModelIndex sourceParent = model()->parent(sourceIndex);
    return mapFromSource(sourceParent);
}

bool AggregableProxyModel::hasChildren(const QModelIndex & parent) const
{
    const QModelIndex sourceParent = mapToSource(parent);
    return model()->hasChildren(sourceParent);
}

bool AggregableProxyModel::canFetchMore(const QModelIndex & parent) const
{
    const QModelIndex sourceParent = mapToSource(parent);
    return model()->canFetchMore(sourceParent);
}

void AggregableProxyModel::fetchMore(const QModelIndex & parent)
{
    const QModelIndex sourceParent = mapToSource(parent);
    model()->fetchMore(sourceParent);
}

int AggregableProxyModel::rowCount(const QModelIndex & parent) const
{
    const QModelIndex sourceParent = mapToSource(parent);
    return model()->rowCount(sourceParent);
}

int AggregableProxyModel::columnCount(const QModelIndex & parent) const
{
    const QModelIndex sourceParent = mapToSource(parent);
    return model()->columnCount(sourceParent);
}

QVariant AggregableProxyModel::data(const QModelIndex & index, int role) const
{
    const QModelIndex sourceIndex = mapToSource(index);
    return model()->data(sourceIndex, role);
}

QVariant AggregableProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    mapToSource(QModelIndex());
    return model()->headerData(section, orientation, role);
}

void AggregableProxyModel::connectSignals(QAbstractItemModel &model)
{
    QObject::connect(&model, &QAbstractItemModel::dataChanged, this, &AggregableProxyModel::sourceDataChanged);

    QObject::connect(&model, &QAbstractItemModel::layoutAboutToBeChanged, this, &AggregableProxyModel::sourceLayoutAboutToBeChanged);
    QObject::connect(&model, &QAbstractItemModel::layoutChanged, this, &AggregableProxyModel::sourceLayoutChanged);

    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeInserted, this, &AggregableProxyModel::sourceRowsAboutToBeInserted);
    QObject::connect(&model, &QAbstractItemModel::rowsInserted, this, &AggregableProxyModel::sourceRowsInserted);
    QObject::connect(&model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &AggregableProxyModel::sourceRowsAboutToBeRemoved);
    QObject::connect(&model, &QAbstractItemModel::rowsRemoved, this, &AggregableProxyModel::sourceRowsRemoved);
}

void AggregableProxyModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
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

    /*const auto proxyPersistentIndexes = persistentIndexList();
    for (const QModelIndex &proxyPersistentIndex : proxyPersistentIndexes) {
        m_proxyIndexes << proxyPersistentIndex;
        const QPersistentModelIndex srcPersistentIndex = mapToSource(proxyPersistentIndex);
        m_layoutChangePersistentIndexes << srcPersistentIndex;
    }*/
}

void AggregableProxyModel::sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint)
{
    /*for (int i = 0; i < m_proxyIndexes.size(); ++i) {
        changePersistentIndex(m_proxyIndexes.at(i), mapFromSource(m_layoutChangePersistentIndexes.at(i)));
    }

    m_layoutChangePersistentIndexes.clear();
    m_proxyIndexes.clear();*/

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

void AggregableProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
    beginRemoveRows(mapFromSource(parent), start, end);
}

void AggregableProxyModel::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    endRemoveRows();
}
