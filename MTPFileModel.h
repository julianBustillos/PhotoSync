#pragma once
#include <QAbstractItemModel>
#include <QFileIconProvider>


class MTPFileModel : public QAbstractItemModel
{
public:
    MTPFileModel(QObject *parent = nullptr);
    ~MTPFileModel();

public:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QFileIconProvider m_iconProvider;
};