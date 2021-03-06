#pragma once
#include "AggregableItemModel.h"

class AggregableProxyModel : public AggregableItemModel
{
public:
    AggregableProxyModel(QObject *parent = nullptr);
    virtual ~AggregableProxyModel() {};

public:
    virtual void setCurrentPath(const QString &newPath) = 0;
    virtual QString filePath(const QModelIndex &index) const = 0;

    virtual QModelIndex	index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex	parent(const QModelIndex &index) const override;

    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool canFetchMore(const QModelIndex &parent) const override;
    virtual void fetchMore(const QModelIndex &parent) override;

    virtual int	rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int	columnCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

protected:
    virtual QAbstractItemModel *model() const = 0;
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const = 0;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const = 0;
    virtual int rowFromSource(const QModelIndex& sourceIndex, int row) const;
    void connectSignals(QAbstractItemModel &model);

private slots:
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

    void sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
    void sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);

    void sourceRowsAboutToBeInserted(const QModelIndex&parent, int start, int end);
    void sourceRowsInserted(const QModelIndex&parent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &parent, int start, int end);
};