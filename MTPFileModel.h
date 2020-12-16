#pragma once
#include <QAbstractItemModel>
#include "MTPFileNode.h"
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
    MTPFileNode *node(const QModelIndex &index) const;

    /*
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void setIconProvider(QFileIconProvider *provider);
    QFileIconProvider *iconProvider() const;
    void removeNode(MTPFileModel *parentNode, const QString &name);
    MTPFileModel* addNode(MTPFileModel *parentNode, const QString &fileName, const QFileInfo &info);
    void addVisibleFiles(MTPFileModel *parentNode, const QStringList &newFiles);
    void removeVisibleFile(MTPFileModel *parentNode, int visibleLocation);
    void sortChildren(int column, const QModelIndex &parent);
    void _q_directoryChanged(const QString &directory, const QStringList &list);
    void _q_fileSystemChanged(const QString &path, const QVector<QPair<QString, QFileInfo> > &);
    */

private:
    MTPFileNode *m_root;
    QFileIconProvider m_iconProvider; //TODO REMOVE ??
};