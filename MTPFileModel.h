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
    virtual void setRootPath(const QString &newPath);
    virtual QString filePath(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

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