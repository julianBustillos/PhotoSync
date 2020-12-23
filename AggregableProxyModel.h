#pragma once
#include "AggregableItemModel.h"

class AggregableProxyModel : public AggregableItemModel
{
public:
    AggregableProxyModel(QObject *parent = nullptr);
    virtual ~AggregableProxyModel() {};

public:
    virtual void setRootPath(const QString &newPath) = 0;
    virtual QString filePath(const QModelIndex &index) const = 0;

    QModelIndex	index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex	parent(const QModelIndex &index) const override;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    int	rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int	columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

protected:
    virtual QAbstractItemModel *model() const = 0;
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const = 0;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const = 0;
    void connectSignals(QAbstractItemModel &model);

private slots:
    void sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
    void sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);

    void sourceRowsAboutToBeInserted(const QModelIndex&, int, int);
    void sourceRowsInserted(const QModelIndex&, int, int);

private:
    QList<QPersistentModelIndex> layoutChangePersistentIndexes;
    QModelIndexList proxyIndexes;
};