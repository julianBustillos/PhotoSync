#include "MTPFileModel.h"
#include <QVector>


MTPFileModel::MTPFileModel(QObject * parent) :
    QAbstractItemModel(parent)
{
    m_root = new MTPFileNode();
    m_fetcher = new MTPFileFetcher(this);
    if (m_fetcher) {
        QObject::connect(m_fetcher, &MTPFileFetcher::loadedDevices, this, &MTPFileModel::addDevices);
        QObject::connect(m_fetcher, &MTPFileFetcher::loadedContent, this, &MTPFileModel::populate);
        m_fetcher->start(QThread::LowPriority);
    }
}

MTPFileModel::~MTPFileModel()
{
    if (m_root)
        delete m_root;

    if (m_fetcher) 
        delete m_fetcher;
}

void MTPFileModel::setRootPath(const QString & newPath)
{
    m_rootStack.clear();
    QString copiedPath = newPath;
    QList<QString> splittedPath = copiedPath.replace("\\", "/").split("/");
    for (auto fileName = splittedPath.rbegin(); fileName != splittedPath.rend(); fileName++)
        m_rootStack.push(*fileName);

    MTPFileNode *node = m_root;

    while (node && node->isPopulated() && !m_rootStack.isEmpty()) {
        QString &fileName = m_rootStack.top();
        node = node->getChildren().value(fileName);
        m_rootStack.pop();
    }

    if (!node) {
        m_rootStack.clear();
        emit rootPathChanged(QModelIndex());
        return;
    }

    if (m_fetcher && !m_rootStack.isEmpty()) {
        m_rootStack.push(node->getName());
        m_fetcher->addToFetch(node);
        return;
    }

    if (node && node->getParent()) {
        int row = node->getParent()->visibleLocation(node->getName());
        emit rootPathChanged(createIndex(row, 0, node));
    }
}

QString MTPFileModel::filePath(const QModelIndex & index) const
{
    if (index.isValid()) {
        MTPFileNode *indexNode = node(index);
        if (indexNode)
            return node(index)->getPath();
    }
    return QString();
}

QModelIndex MTPFileModel::index(int row, int column, const QModelIndex & parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    MTPFileNode *parentNode = node(parent);
    if (row >= parentNode->getVisibleChildren().size())
        return QModelIndex();

    const QString &childName = parentNode->getVisibleChildren().at(row);
    MTPFileNode *indexNode = parentNode->getChildren().value(childName);
    m_fetcher->addToFetch(indexNode);
    return createIndex(row, column, indexNode);
}

QModelIndex MTPFileModel::parent(const QModelIndex & child) const
{
    if (!child.isValid())
        return QModelIndex();

    MTPFileNode *indexNode = node(child);
    if (indexNode)
        return parent(*indexNode);
    return QModelIndex();
}

int MTPFileModel::rowCount(const QModelIndex & parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return m_root->getVisibleChildren().count();

    MTPFileNode *parentNode = node(parent);
    return parentNode->getVisibleChildren().count();
}

int MTPFileModel::columnCount(const QModelIndex & parent) const
{
    return (parent.column() > 0) ? 0 : 4;
}

QVariant MTPFileModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    MTPFileNode *indexNode = node(index);
    if (!indexNode)
        return QVariant();

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return node(index)->getName();
        case 1: return node(index)->getSize() ? QString::number(node(index)->getSize()) : "";
        case 2: return node(index)->getType();
        case 3: return node(index)->getLastModified();
        default:
            qWarning("data: invalid display value column %d", index.column());
            break;
        }
        break;
    /*case QFileSystemModel::FilePathRole:
        return "J:";
    case QFileSystemModel::FileNameRole:
        return "J:";*/
    case Qt::DecorationRole:
        if (index.column() == 0)
            return node(index)->getQIcon();
        return QIcon();
    case Qt::TextAlignmentRole:
        if (index.column() == 1)
            return QVariant(Qt::AlignTrailing | Qt::AlignVCenter);
        break;
    case Qt::ForegroundRole:
        return node(index)->isPopulated() ? QColor(Qt::black) : QColor(Qt::gray);
    /*case QFileSystemModel::FilePermissions:
        return QVariant();*/
    default:
        return QVariant();
    }
    return QVariant();
}

QVariant MTPFileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::DecorationRole:
        if (section == 0) {
            QImage pixmap(16, 1, QImage::Format_ARGB32_Premultiplied);
            pixmap.fill(Qt::transparent);
            return pixmap;
        }
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft;
    }

    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractItemModel::headerData(section, orientation, role);

    QString returnValue;
    switch (section) {
    case 0: returnValue = tr("Name");
        break;
    case 1: returnValue = tr("Size");
        break;
    case 2: returnValue = tr("Type", "All other platforms");
        break;
    case 3: returnValue = tr("Date Modified");
        break;
    default: return QVariant();
    }
    return returnValue;
}

void MTPFileModel::addDevices(const QStringList &devices)
{
    for (const QString &device : devices) {
        MTPFileNode *node = new MTPFileNode(device, MTPFileNode::DEVICE, 0, "", m_iconProvider.icon(QFileIconProvider::Computer), m_root);

        if (node) {
            m_root->getChildren().insert(MTPFileNodePathKey(device), node);
            m_root->getVisibleChildren().append(device);
        }
    }
    m_root->setPopulated();
}

void MTPFileModel::populate(const NodeContainer &container)
{
    if (!container.m_content->isEmpty()) {
        for (WPDManager::Item &item : *container.m_content) {
            MTPFileNode *child = new MTPFileNode(item.m_name, TypeConversion(item.m_type), 0, FormatDate(item.m_date), m_iconProvider.icon(IconConversion(item.m_type)), container.m_node);
            if (child) {
                container.m_node->getChildren().insert(MTPFileNodePathKey(item.m_name), child);
                container.m_node->getVisibleChildren().append(item.m_name);
            }
        }
    }
    container.m_node->setPopulated();

    int row = container.m_node->getParent()->visibleLocation(container.m_node->getName());
    QModelIndex index = createIndex(row, 0, container.m_node);

    emit dataChanged(index, index, QVector<int>(1, Qt::ForegroundRole));

    delete container.m_content;

    //SetRootPath management
    if (!m_rootStack.isEmpty()) {
        const QString &fileName = m_rootStack.top();
        if (container.m_node->getName() == fileName) {
            m_rootStack.pop();
            if (m_rootStack.isEmpty())
                emit rootPathChanged(index);
            else if (m_fetcher) {
                MTPFileNode *child = container.m_node->getChildren().value(m_rootStack.top());
                if (child)
                    m_fetcher->addToFetch(child);
                else
                    m_rootStack.clear();
            }
        }
    }
}

QModelIndex MTPFileModel::parent(MTPFileNode & child) const
{
    MTPFileNode *parentNode = child.getParent();
    if (parentNode == nullptr || parentNode == m_root)
        return QModelIndex();

    MTPFileNode *grandParentNode = parentNode->getParent();
    int visualRow = grandParentNode->visibleLocation(grandParentNode->getChildren().value(parentNode->getName())->getName());
    if (visualRow == -1)
        return QModelIndex();

    return createIndex(visualRow, 0, parentNode);
}

MTPFileNode * MTPFileModel::node(const QModelIndex & index) const
{
    if (!index.isValid())
        return m_root;

    MTPFileNode *indexNode = static_cast<MTPFileNode *>(index.internalPointer());
    return indexNode;
}

MTPFileNode::Type MTPFileModel::TypeConversion(WPDManager::ItemType type)
{
    switch (type) {
    case WPDManager::DRIVE:
        return MTPFileNode::DRIVE;
    case WPDManager::FOLDER:
        return MTPFileNode::FOLDER;
    case WPDManager::FILE:
        return MTPFileNode::FILE;
    default:
        return MTPFileNode::UNKNOWN;
    }
}

QFileIconProvider::IconType MTPFileModel::IconConversion(WPDManager::ItemType type)
{
    switch (type) {
    case WPDManager::DRIVE:
        return QFileIconProvider::Drive;
    case WPDManager::FOLDER:
        return QFileIconProvider::Folder;
    case WPDManager::FILE:
        return QFileIconProvider::File;
    default:
        return QFileIconProvider::File;
    }
}

QString MTPFileModel::FormatDate(QString date)
{
    QString newDate = "";

    if (date.size() == 23) {
        QVector<QStringRef> YMD = QStringRef(&date, 0, 10).split("/");
        QVector<QStringRef> HM = QStringRef(&date, 11, 5).split(":");

        if (YMD.size() == 3 && HM.size() == 2) {
            newDate += YMD[2] + "/"; //day
            newDate += YMD[1] + "/"; //month
            newDate += YMD[0] + " "; //year

            newDate += HM[0] + ":";  //hour
            newDate += HM[1];        //minute
        }
    }

    return newDate;
}
