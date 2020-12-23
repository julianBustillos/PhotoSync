#pragma once
#include <QFileIconProvider>
#include <QStack>
#include "AggregableItemModel.h"
#include "MTPFileFetcher.h"
#include "MTPFileNode.h"
#include "NodeContainer.h"


class MTPFileModel : public AggregableItemModel
{
    Q_OBJECT

public:
    MTPFileModel(QObject *parent = nullptr);
    ~MTPFileModel();

public:
    void setRootPath(const QString &newPath) override;
    QString filePath(const QModelIndex &index) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public slots:
    void addDevices(const QStringList &devices);
    void populate(const NodeContainer &container);

private:
    QModelIndex parent(MTPFileNode &child) const;
    MTPFileNode *node(const QModelIndex &index) const;

    //TODO: ADD THESE METHODS ?
    /*
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void sortChildren(int column, const QModelIndex &parent);
    void _q_directoryChanged(const QString &directory, const QStringList &list);
    void _q_fileSystemChanged(const QString &path, const QVector<QPair<QString, QFileInfo> > &);
    */

private:
    static MTPFileNode::Type TypeConversion(WPDManager::ItemType type);
    static QFileIconProvider::IconType IconConversion(WPDManager::ItemType type);
    static QString FormatDate(QString date);

private:
    MTPFileFetcher *m_fetcher;
    MTPFileNode *m_root;
    QStack<QString> m_rootStack;
    QFileIconProvider m_iconProvider; //TODO REMOVE ??
};