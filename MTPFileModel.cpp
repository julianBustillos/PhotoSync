#include "MTPFileModel.h"


MTPFileModel::MTPFileModel(QObject * parent) :
    QAbstractItemModel(parent)
{
    m_root = new MTPFileNode();
    //DEBUG
    MTPFileNode *node = new MTPFileNode("Juju test DRIVE", m_root);
    node->m_isDir = true;
    node->m_size = 0;
    node->m_type = "Drive";
    node->m_lastModified = "13/01/2020 00:00";
    node->m_icon = m_iconProvider.icon(QFileIconProvider::Drive);

    m_root->m_children.insert(MTPFileNodePathKey("Juju test DRIVE"), node);
    m_root->m_visibleChildren.append("Juju test DRIVE");
    //DEBUG
}

MTPFileModel::~MTPFileModel()
{
    delete m_root;
}

QModelIndex MTPFileModel::index(int row, int column, const QModelIndex & parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    MTPFileNode *parentNode = node(parent);
    if (row >= parentNode->m_visibleChildren.size())
        return QModelIndex();

    const QString &childName = parentNode->m_visibleChildren.at(row);
    MTPFileNode *indexNode = parentNode->m_children.value(childName);

    return createIndex(row, column, indexNode);
}

QModelIndex MTPFileModel::parent(const QModelIndex & child) const
{
    if (!child.isValid())
        return QModelIndex();

    MTPFileNode *indexNode = node(child);
    MTPFileNode *parentNode = indexNode->m_parent;
    if (parentNode == nullptr || parentNode == m_root)
        return QModelIndex();

    MTPFileNode *grandParentNode = parentNode->m_parent;
    int visualRow = grandParentNode->visibleLocation(grandParentNode->m_children.value(parentNode->m_fileName)->m_fileName);
    if (visualRow == -1)
        return QModelIndex();

    return createIndex(visualRow, 0, parentNode);
}

int MTPFileModel::rowCount(const QModelIndex & parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return m_root->m_visibleChildren.count();

    const MTPFileNode *parentNode = node(parent);
    return parentNode->m_visibleChildren.count();
}

int MTPFileModel::columnCount(const QModelIndex & parent) const
{
    return (parent.column() > 0) ? 0 : 4;
}

QVariant MTPFileModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return node(index)->m_fileName;
        case 1: return node(index)->m_isDir ? "" : QString::number(node(index)->m_size);
        case 2: return node(index)->m_type;
        case 3: return node(index)->m_lastModified;
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
            return node(index)->m_icon;
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
