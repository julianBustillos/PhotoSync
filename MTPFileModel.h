#pragma once
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QFileIconProvider>
#include <QStack>
#include "MTPFileFetcher.h"
#include "MTPFileNode.h"
#include "NodeContainer.h"


class MTPFileModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    MTPFileModel(QObject *parent = nullptr);
    ~MTPFileModel();

public:
    void setRootPath(const QString &newPath);
    QString filePath(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

public slots:
    void addDevices(const QStringList &devices);
    void populate(const NodeContainer &container);

signals:
    void rootPathChanged(const QModelIndex &rootPathIndex);

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