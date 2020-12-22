#pragma once

#include <QDialog>
#include "ui_FileExplorerDialog.h"
#include "AggregableItemModel.h"


class FileExplorerDialog : public QDialog
{
    Q_OBJECT

public:
    FileExplorerDialog(QWidget *parent = Q_NULLPTR);
    ~FileExplorerDialog();

public:
    void setDirectory(QString directory);
    QString getDirectory();

public slots:
    void rootPathChanged(const QModelIndex &rootPathIndex);

private:
    void chooseDirectory();

private:
    Ui::FileExplorerDialogClass m_ui;
    QString m_directory;
    //QFileSystemModel m_fileSystemModel; //TODO: REMOVE
    //MTPFileModel m_MTPFileModel; //TODO: REMOVE
    AggregableItemModel *m_aggregableModel;
};