#include "MTPFileModel.h"
#include <vector>


MTPFileModel::MTPFileModel(QObject * parent) :
    QAbstractItemModel(parent)
{
    m_WPDManager = new WPDManager();
    m_root = new MTPFileNode(QString("root"));
    getDevices();
}

MTPFileModel::~MTPFileModel()
{
    if (m_root)
        delete m_root;

    if (m_WPDManager)
        delete m_WPDManager;
}

QModelIndex MTPFileModel::setRootPath(const QString & newPath)
{
    QString copiedPath = newPath;
    QVector<QStringRef> splittedPath = copiedPath.replace("\\", "/").splitRef("/");

    MTPFileNode *node = m_root;
    int index = 0;
    int row = 0;

    while (node && index < splittedPath.size()) {
        populate(*node);
        node = node->getChildren().value(splittedPath[index++].toString());
    }

    if (node && node->getParent()) {
        int row = node->getParent()->visibleLocation(splittedPath.last().toString());
        return createIndex(row, 0, node);
    }
    return QModelIndex();
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
    if (indexNode)
        populate(*indexNode);

    return createIndex(row, column, indexNode);
}

QModelIndex MTPFileModel::parent(const QModelIndex & child) const
{
    if (!child.isValid())
        return QModelIndex();

    MTPFileNode *indexNode = node(child);
    MTPFileNode *parentNode = indexNode->getParent();
    if (parentNode == nullptr || parentNode == m_root)
        return QModelIndex();

    MTPFileNode *grandParentNode = parentNode->getParent();
    int visualRow = grandParentNode->visibleLocation(grandParentNode->getChildren().value(parentNode->getName())->getName());
    if (visualRow == -1)
        return QModelIndex();

    return createIndex(visualRow, 0, parentNode);
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

    populate(*indexNode);
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
    /*case QFileSystemModel::FilePermissions:
        return QVariant();*/
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

MTPFileNode * MTPFileModel::node(const QModelIndex & index) const
{
    if (!index.isValid())
        return m_root;

    MTPFileNode *indexNode = static_cast<MTPFileNode *>(index.internalPointer());
    return indexNode;
}

void MTPFileModel::getDevices()
{
    QStringList deviceList;
    if (m_WPDManager)
        m_WPDManager->getDevices(deviceList);
    for (QString device : deviceList) {
        MTPFileNode *node = new MTPFileNode(device, MTPFileNode::DEVICE, 0, "", m_iconProvider.icon(QFileIconProvider::Computer), m_root);

        if (node) {
            m_root->getChildren().insert(MTPFileNodePathKey(device), node);
            m_root->getVisibleChildren().append(device);
        }

        m_root->setPopulated();
    }
}

void MTPFileModel::populate(MTPFileNode & node) const
{
    if (node.isPopulated())
        return;

    std::vector<WPDManager::Item> content;
    if (m_WPDManager && m_WPDManager->getContent(node.getPath(), content)) {
        for (WPDManager::Item &item : content) {
            MTPFileNode *child = new MTPFileNode(item.m_name, TypeConversion(item.m_type), 0, FormatDate(item.m_date), m_iconProvider.icon(IconConversion(item.m_type)), &node); //TODO SET REAL VALUES FOR DATE AND SIZE
            if (child) {
                node.getChildren().insert(MTPFileNodePathKey(item.m_name), child);
                node.getVisibleChildren().append(item.m_name);
            }
        }
    }

    node.setPopulated();
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
