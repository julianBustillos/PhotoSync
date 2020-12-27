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

private slots:
    void createWarning(QString title, QString message);
    void setProgressBarValue(int value);
    void setProgressBarMaximum(int maximum);
    void appendOutput(QString output);
    void finish();

private:
    Ui::PhotoSyncClass m_ui;
    FileExplorerDialog m_dialog; //TODO: rename ??
    FileManager m_fileManager;
    QString m_positiveDefaultText;
};
