#pragma once
#include "AggregableProxyModel.h"
#include <QFileSystemModel>


class FileSystemProxyModel : public AggregableProxyModel
{
    Q_OBJECT

public:
    FileSystemProxyModel(QObject *parent = nullptr);
    ~FileSystemProxyModel();

public:
    virtual void setRootPath(const QString &newPath) override;
    virtual QString filePath(const QModelIndex &index) const override;

protected:
    virtual QAbstractItemModel *model() const override;
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

private:
    class ModelIndexHelper : public QFileSystemModel
    {
        friend class FileSystemProxyModel;
    };

private:
    QFileSystemModel *m_model;
};