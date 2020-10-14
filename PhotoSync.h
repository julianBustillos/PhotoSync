#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PhotoSync.h"
#include "FileManager.h"


class PhotoSync : public QMainWindow
{
    Q_OBJECT

public:
    PhotoSync(QWidget *parent = Q_NULLPTR);

private :
    void askImportFolder();
    void askExportFolder();
    void run();

private:
    Ui::PhotoSyncClass m_ui;
};
