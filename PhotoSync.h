#pragma once

#include <QMainWindow>
#include "ui_PhotoSync.h"
#include "WPDManager.h"
#include "FileExplorerDialog.h"
#include "FileManager.h"

class PhotoSync : public QMainWindow
{
    Q_OBJECT
    friend FileManager;

public:
    PhotoSync(QWidget *parent = Q_NULLPTR);
    ~PhotoSync();

public:
    static WPDManager &getWPDInstance();

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
    QString m_positiveDefaultText;
    FileExplorerDialog *m_dialog; //TODO: rename ??
    FileManager *m_fileManager;

private:
    static WPDManager *WPDInstance;
};
