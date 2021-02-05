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
    void currentPathChanged(const QModelIndex &currentPathIndex);

private:
    void chooseDirectory();

private:
    Ui::FileExplorerDialogClass m_ui;
    QString m_directory;
    AggregableItemModel *m_aggregableModel;
};