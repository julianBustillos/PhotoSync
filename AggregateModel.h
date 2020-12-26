#pragma once
#include "AggregableProxyModel.h"


class AggregateModel : public AggregableProxyModel
{
public:
    AggregateModel(QObject *parent = nullptr);
    ~AggregateModel();

public:
    void addModel(AggregableItemModel *model);

    void setRootPath(const QString &newPath);
    QString filePath(const QModelIndex &index) const;

    //QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    //QModelIndex parent(const QModelIndex &index) const override;

    //bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    //canFetchMore(const QModelIndex &parent) const override;
    //void fetchMore(const QModelIndex &parent) override;

    //int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    //int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    //QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    //QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

protected:
    QAbstractItemModel *model() const;
    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

private:
    class ModelIndexHelper : public AggregableItemModel
    {
        friend class AggregateModel;
    };

private slots:
void sourceRootPathChanged(const QModelIndex &rootPathIndex);

private:
    void connectRootPathChanged(AggregableItemModel &model) const;
    void changeSourceModelID(const QAbstractItemModel *sourceModel) const;

private:
    QVector<AggregableItemModel *> m_sourceModels;
    int *m_sourceModelID;
    QHash<void *, int> *m_mapDataToModelID;
};