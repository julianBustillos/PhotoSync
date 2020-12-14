#pragma once

#include <QMainWindow>
#include "ui_PhotoSync.h"
#include "FileExplorerDialog.h"
#include "FileManager.h"
#include <QFileDialog>


class PhotoSync : public QMainWindow
{
    Q_OBJECT
    friend FileManager;

public:
    PhotoSync(QWidget *parent = Q_NULLPTR);
    ~PhotoSync();

private :
    void askImportFolder();
    void askExportFolder();
    void run();
    void cancel();

private:
    Ui::PhotoSyncClass m_ui;
    FileExplorerDialog m_dialog; //Todo rename ??
    QFileDialog m_fileDialog;
    FileManager m_fileManager;
    QString m_positiveDefaultText;
};
