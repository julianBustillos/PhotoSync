#pragma once
#include <QAbstractItemModel>
#include <QFileIconProvider>
#include "MTPFileNode.h"
#include "WPDManager.h"


class MTPFileModel : public QAbstractItemModel
{
public:
    MTPFileModel(QObject *parent = nullptr);
    ~MTPFileModel();

public:
    QModelIndex setRootPath(const QString &newPath);
    QString filePath(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    MTPFileNode *node(const QModelIndex &index) const;
    void getDevices();
    void populate(MTPFileNode &node) const;

    /*
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
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
    MTPFileNode *m_root;
    WPDManager *m_WPDManager; //TODO CHANGE ??
    QFileIconProvider m_iconProvider; //TODO REMOVE ??
};