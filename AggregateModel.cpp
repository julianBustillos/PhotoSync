#include "AggregateModel.h"
#include "AggregateRootModel.h"


AggregateModel::AggregateModel(QObject * parent) :
    AggregableProxyModel(parent), m_sourceModelID(new int(0)), m_mapDataToModelID(new QHash<void *, int>())
{
    m_sourceModels.append(new AggregateRootModel(m_sourceModels, this));
}

AggregateModel::~AggregateModel()
{
    for (AggregableItemModel *model : m_sourceModels) {
        delete model;
        model = nullptr;
    }

    delete m_sourceModelID;
    m_sourceModelID = nullptr;

    delete m_mapDataToModelID;
    m_mapDataToModelID = nullptr;
}

void AggregateModel::addModel(AggregableItemModel * model)
{
    if (model) {
        connectSignals(*model);
        connectRootPathChanged(*model);
        m_sourceModels.append(model);
    }
}

void AggregateModel::setRootPath(const QString & newPath)
{
    for (int sourceID = 1; sourceID < m_sourceModels.size(); sourceID++) {
        m_sourceModels[sourceID]->setRootPath(newPath);
    }
}

QString AggregateModel::filePath(const QModelIndex & index) const
{
    QModelIndex sourceIndex = mapToSource(index);
    return m_sourceModels[*m_sourceModelID]->filePath(sourceIndex);
}

QAbstractItemModel * AggregateModel::model() const
{
    if (*m_sourceModelID >= 0)
        return m_sourceModels[*m_sourceModelID];
    return nullptr;
}

QModelIndex AggregateModel::mapFromSource(const QModelIndex & sourceIndex) const
{
    *m_sourceModelID = 0;

    if (!sourceIndex.isValid())
        return QModelIndex();

    for (int sourceID = 1; sourceID < m_sourceModels.size(); sourceID++) {
        if (sourceIndex.model() == m_sourceModels[sourceID]) {
            *m_sourceModelID = sourceID;
            break;
        }
    }

    int rowOffset = sourceIndex.parent().isValid() ? 0 : getRowOffset(sourceIndex.model());
    m_mapDataToModelID->insert(sourceIndex.internalPointer(), *m_sourceModelID);

    return createIndex(sourceIndex.row() + rowOffset, sourceIndex.column(), sourceIndex.internalPointer());
}

QModelIndex AggregateModel::mapToSource(const QModelIndex & proxyIndex) const
{
    *m_sourceModelID = 0;

    if (!proxyIndex.isValid()) 
        return QModelIndex();

    auto iter = m_mapDataToModelID->find(proxyIndex.internalPointer());
    if (iter != m_mapDataToModelID->end()) {
        *m_sourceModelID = iter.value();
        return static_cast<const ModelIndexHelper *>(m_sourceModels[*m_sourceModelID])->createIndex(proxyIndex.row(), proxyIndex.column(), proxyIndex.internalPointer());
    }

    return QModelIndex();
}

int AggregateModel::rowFromSource(const QModelIndex & sourceIndex, int row) const
{
    return sourceIndex.isValid() ? row : row + getRowOffset(sourceIndex.model());
}

void AggregateModel::sourceRootPathChanged(const QModelIndex &rootPathIndex)
{
    emit rootPathChanged(mapFromSource(rootPathIndex));
}

void AggregateModel::connectRootPathChanged(AggregableItemModel &model) const
{
    QObject::connect(&model, &AggregableItemModel::rootPathChanged, this, &AggregateModel::sourceRootPathChanged);
}

int AggregateModel::getRowOffset(const QAbstractItemModel * model) const
{
    int rowOffset = 0;
    for (int sourceID = 1; sourceID < m_sourceModels.size() && model; sourceID++) {
        if (model == m_sourceModels[sourceID]) 
            break;
        rowOffset += m_sourceModels[sourceID]->rowCount(QModelIndex());
    }

    return rowOffset;
}
