#pragma once

#include <QMainWindow>
#include "ui_PhotoSync.h"
#include "FileManager.h"


class PhotoSync : public QMainWindow
{
    Q_OBJECT
    friend FileManager;

public:
    PhotoSync(QWidget *parent = Q_NULLPTR);

private :
    void askImportFolder();
    void askExportFolder();
    void run();
    void cancel();

private:
    Ui::PhotoSyncClass m_ui;
    FileManager m_fileManager;
    QString m_positiveDefaultText;
};
