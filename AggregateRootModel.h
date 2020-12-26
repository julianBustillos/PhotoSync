#pragma once
#include "AggregableItemModel.h"


class AggregateRootModel : public AggregableItemModel
{
    Q_OBJECT

public:
    AggregateRootModel(QVector<AggregableItemModel *> &sourceModels, QObject *parent = nullptr);
    virtual ~AggregateRootModel();

public:
    void setRootPath(const QString &newPath) override;
    QString filePath(const QModelIndex &index) const override;

    QModelIndex	index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex	parent(const QModelIndex &index) const override;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    int	rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int	columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QVector<AggregableItemModel *> &m_sourceModels;
};