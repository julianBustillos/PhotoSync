#pragma once

#include <QDialog>
#include "ui_FileExplorerDialog.h"
#include <QFileSystemModel>


class FileExplorerDialog : public QDialog
{
    Q_OBJECT

public:
    FileExplorerDialog(QWidget *parent = Q_NULLPTR);
    ~FileExplorerDialog();

public:
    void setDirectory(QString directory);
    QString getDirectory();

private:
    void chooseDirectory();

private:
    Ui::FileExplorerDialogClass m_ui;
    QString m_currentDirectory;
    QFileSystemModel m_fileSystemModel;
};