#include "AggregateRootModel.h"


AggregateRootModel::AggregateRootModel(QVector<AggregableItemModel *> &sourceModels, QObject *parent) :
    AggregableItemModel(parent), m_sourceModels(sourceModels)
{
}

AggregateRootModel::~AggregateRootModel()
{
}

void AggregateRootModel::setCurrentPath(const QString & newPath)
{
}

QString AggregateRootModel::filePath(const QModelIndex & index) const
{
    return QString();
}

QModelIndex AggregateRootModel::index(int row, int column, const QModelIndex & parent) const
{
    int rowOffset = 0;
    for (int sourceID = 1; sourceID < m_sourceModels.size(); sourceID++) {
        int rowCount = m_sourceModels[sourceID]->rowCount(QModelIndex());
        if (rowCount > row - rowOffset) {
            return m_sourceModels[sourceID]->index(row - rowOffset, column, QModelIndex());
        }
        rowOffset += rowCount;
    }

    return QModelIndex();
}

QModelIndex AggregateRootModel::parent(const QModelIndex & index) const
{
    return QModelIndex();
}

bool AggregateRootModel::hasChildren(const QModelIndex & parent) const
{
    return true;
}

bool AggregateRootModel::canFetchMore(const QModelIndex & parent) const
{
    for (int sourceID = 1; sourceID < m_sourceModels.size(); sourceID++)
        if (m_sourceModels[sourceID]->canFetchMore(QModelIndex()))
            return true;
    return false;
}

void AggregateRootModel::fetchMore(const QModelIndex & parent)
{
    for (int sourceID = 1; sourceID < m_sourceModels.size(); sourceID++)
        m_sourceModels[sourceID]->fetchMore(QModelIndex());
}

int AggregateRootModel::rowCount(const QModelIndex & parent) const
{
    int rootRows = 0;
    for (int sourceID = 1; sourceID < m_sourceModels.size(); sourceID++)
        rootRows += m_sourceModels[sourceID]->rowCount(QModelIndex());
    return rootRows;
}

int AggregateRootModel::columnCount(const QModelIndex & parent) const
{
    if (m_sourceModels.size() > 1)
        return m_sourceModels[1]->columnCount(QModelIndex());
    return 0;
}

QVariant AggregateRootModel::data(const QModelIndex & index, int role) const
{
    return QVariant();
}

QVariant AggregateRootModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (m_sourceModels.size() > 1)
        return m_sourceModels[1]->headerData(section, orientation, role);
    return QVariant();
}
