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
    virtual int	rowCount(const QModelIndex &parent = QModelIndex()) const = 0;
    virtual int	columnCount(const QModelIndex &parent = QModelIndex()) const = 0;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const = 0;
    //End to implement
};