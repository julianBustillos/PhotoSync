#include "FileSystemProxyModel.h"
#include <QFileSystemModel>


FileSystemProxyModel::FileSystemProxyModel(QObject *parent) :
    AggregableProxyModel(parent)
{
    m_model = new QFileSystemModel(parent);
    connectSignals(*m_model);
    m_model->setRootPath(QString());
}

FileSystemProxyModel::~FileSystemProxyModel()
{
    if (m_model)
        delete m_model;
    m_model = nullptr;
}

void FileSystemProxyModel::setRootPath(const QString & newPath)
{
    m_model->setRootPath(QString());
    const QModelIndex sourceIndex = m_model->setRootPath(newPath);
    if (sourceIndex.isValid())
        emit rootPathChanged(mapFromSource(sourceIndex));
}

QString FileSystemProxyModel::filePath(const QModelIndex & index) const
{
    return m_model->filePath(mapToSource(index));
}

QAbstractItemModel * FileSystemProxyModel::model() const
{
    return m_model;
}

QModelIndex FileSystemProxyModel::mapFromSource(const QModelIndex & sourceIndex) const
{
    if (!sourceIndex.isValid())
        return QModelIndex();

    return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
}

QModelIndex FileSystemProxyModel::mapToSource(const QModelIndex & proxyIndex) const
{
    if (!proxyIndex.isValid())
        return QModelIndex();

    return static_cast<const ModelIndexHelper *>(m_model)->createIndex(proxyIndex.row(), proxyIndex.column(), proxyIndex.internalPointer());
}