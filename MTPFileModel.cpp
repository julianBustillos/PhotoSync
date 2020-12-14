#include "MTPFileModel.h"

MTPFileModel::MTPFileModel(QObject * parent) :
    QAbstractItemModel(parent)
{
}

MTPFileModel::~MTPFileModel()
{
}

QModelIndex MTPFileModel::index(int row, int column, const QModelIndex & parent) const
{
    if (!parent.isValid() && row == 0) {
        //TEST INDEX VALIDITY
        return createIndex(row, column, nullptr);
    }

    if (!hasIndex(row, column, parent)) //TO TEST
        return QModelIndex();

    return QModelIndex();
}

QModelIndex MTPFileModel::parent(const QModelIndex & child) const
{
    //TODO
    return QModelIndex();
}

int MTPFileModel::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid()) //TODO CHANGE
        return 1;
    return 0;
}

int MTPFileModel::columnCount(const QModelIndex & parent) const
{
    return 4;
}

QVariant MTPFileModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (!index.parent().isValid() && index.row() == 0) //TODO CHANGE
    {
        switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            switch (index.column()) {
            case 0: return "JUJU DRIVE";
            case 1: return "";
            case 2: return "Drive";
            case 3: return "13/01/2020 00:00";
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
                return m_iconProvider.icon(QFileIconProvider::Drive);
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
    case 2: returnValue =
        tr("Type", "All other platforms");
        break;
    case 3: returnValue = tr("Date Modified");
        break;
    default: return QVariant();
    }
    return returnValue;
}