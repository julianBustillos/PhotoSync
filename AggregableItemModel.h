#pragma once
#include <QAbstractItemModel>


class AggregableItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    AggregableItemModel(QObject *parent = nullptr) : QAbstractItemModel(parent) {};
    virtual ~AggregableItemModel() {};

signals:
    void rootPathChanged(const QModelIndex &rootPathIndex);

public:
    //Begin to implement
    virtual void setRootPath(const QString &newPath) = 0;
    virtual QString filePath(const QModelIndex &index) const = 0;

    virtual QModelIndex	index(int row, int column, const QModelIndex &parent = QModelIndex()) const = 0;
    virtual QModelIndex	parent(const QModelIndex &index) const = 0;

    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const = 0;
    virtual bool canFetchMore(const QModelIndex &parent) const = 0;
    virtual void fetchMore(const QModelIndex &parent) = 0;

    virtual int	rowCount(const QModelIndex &parent = QModelIndex()) const = 0;
    virtual int	columnCount(const QModelIndex &parent = QModelIndex()) const = 0;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const = 0;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const = 0;
    //virtual Qt::ItemFlags flags(const QModelIndex &index) const = 0; //TODO REMOVE ?
    //End to implement
};