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