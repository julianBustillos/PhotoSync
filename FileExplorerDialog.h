#pragma once

#include <QDialog>
#include "ui_FileExplorerDialog.h"
#include <QFileSystemModel>
#include "MTPFileModel.h"


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
    QFileSystemModel m_fileSystemModel;
    MTPFileModel m_MTPFileModel;
};