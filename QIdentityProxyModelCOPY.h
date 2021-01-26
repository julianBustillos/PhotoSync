#pragma once

#include <QtCore/qabstractproxymodel.h>
#include <QFileSystemModel>
#include "AggregableItemModel.h"


class QIdentityProxyModelCOPY : public AggregableItemModel
{
    Q_OBJECT

public:
    explicit QIdentityProxyModelCOPY(QObject* parent = nullptr);
    ~QIdentityProxyModelCOPY();

    void setRootPath(const QString &newPath) override;
    QString filePath(const QModelIndex &index) const override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    //QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &proxyIndex, int role) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

public:
    //UNUSED
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

    QItemSelection mapSelectionFromSource(const QItemSelection& selection) const;
    QItemSelection mapSelectionToSource(const QItemSelection& selection) const;
    QModelIndexList match(const QModelIndex& start, int role, const QVariant& value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const override;

    bool insertColumns(int column, int count, const QModelIndex& parent = QModelIndex()) override;
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    bool moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count, const QModelIndex &destinationParent, int destinationChild) override;

    //bool submit() override;
    //void revert() override;

    //QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    //Qt::ItemFlags flags(const QModelIndex &index) const override;

    //bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    //bool setItemData(const QModelIndex& index, const QMap<int, QVariant> &roles) override;
    //bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    //QModelIndex buddy(const QModelIndex &index) const override;

    //void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    //QSize span(const QModelIndex &index) const override;

    //QMimeData* mimeData(const QModelIndexList &indexes) const override;
    //bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    //bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    //QStringList mimeTypes() const override;
    //Qt::DropActions supportedDragActions() const override;
    //Qt::DropActions supportedDropActions() const override;
    //UNUSED

private:
    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

private:
    class ModelIndexHelper : public QFileSystemModel
    {
        friend class QIdentityProxyModelCOPY;
    };

private slots:
    void _q_sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
    void _q_sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);

    void _q_sourceRowsAboutToBeInserted(const QModelIndex&, int, int);
    void _q_sourceRowsInserted(const QModelIndex&, int, int);


private slots:
    //UNUSED
    void _q_sourceRowsAboutToBeRemoved(const QModelIndex&, int, int);
    void _q_sourceRowsRemoved(const QModelIndex&, int, int);
    void _q_sourceRowsAboutToBeMoved(const QModelIndex&, int, int, const QModelIndex&, int);
    void _q_sourceRowsMoved(const QModelIndex&, int, int, const QModelIndex&, int);

    void _q_sourceColumnsAboutToBeInserted(const QModelIndex&, int, int);
    void _q_sourceColumnsInserted(const QModelIndex&, int, int);
    void _q_sourceColumnsAboutToBeRemoved(const QModelIndex&, int, int);
    void _q_sourceColumnsRemoved(const QModelIndex&, int, int);
    void _q_sourceColumnsAboutToBeMoved(const QModelIndex&, int, int, const QModelIndex&, int);
    void _q_sourceColumnsMoved(const QModelIndex&, int, int, const QModelIndex&, int);

    void _q_sourceDataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&);
    void _q_sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);


    void _q_sourceModelAboutToBeReset();
    void _q_sourceModelReset();
    //UNUSED

private:
    void connectSignals();

private:
    QList<QPersistentModelIndex> layoutChangePersistentIndexes;
    QModelIndexList proxyIndexes;

private:
    QFileSystemModel m_model;
};

