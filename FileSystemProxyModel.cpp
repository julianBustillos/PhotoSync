#include "FileSystemProxyModel.h"
#include <QFileSystemModel>


FileSystemProxyModel::FileSystemProxyModel(QObject *parent) :
    AggregableProxyModel(parent)
{
    m_model = new QFileSystemModel(parent);
    QObject::connect(m_model, &QAbstractItemModel::layoutChanged, this, &FileSystemProxyModel::processStack);
    connectSignals(*m_model);
    m_model->setRootPath(QString());
}

FileSystemProxyModel::~FileSystemProxyModel()
{
    if (m_model)
        delete m_model;
    m_model = nullptr;
}

void FileSystemProxyModel::setCurrentPath(const QString & newPath)
{
    m_currentPathStack.clear();

    int index = newPath.size();
    QString pathToFetch;
    while (index > -1) {
        pathToFetch = newPath.left(index);
        m_currentPathStack.push(pathToFetch);
        index = newPath.lastIndexOf("/", index - 1);
    }

    processStack();
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

void FileSystemProxyModel::processStack()
{
    if (m_currentPathStack.isEmpty())
        return;

    QString pathToFetch;
    QModelIndex sourceIndex;
    do {
        pathToFetch = m_currentPathStack.top();
        m_currentPathStack.pop();
        sourceIndex = m_model->index(pathToFetch);
    } while (sourceIndex.isValid() && !m_model->canFetchMore(sourceIndex) && !m_currentPathStack.isEmpty());

    if (sourceIndex.isValid()) {
        if (m_currentPathStack.isEmpty())
            emit currentPathChanged(mapFromSource(sourceIndex));
        else
            m_model->fetchMore(sourceIndex);
    }
    else {
        m_currentPathStack.clear();
    }
}
