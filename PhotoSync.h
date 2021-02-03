#pragma once

#include <QMainWindow>
#include "ui_PhotoSync.h"
#include "Settings.h"
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

private :
    void askImportFolder();
    void askExportFolder();
    void run();

private slots:
    void createWarning(QString title, QString message, bool emitAnswer);
    void setProgressBarValue(int value);
    void setProgressBarMaximum(int maximum);
    void appendOutput(QString output);
    void finish();

signals:
    void warningAnswer(bool answer);

private:
    Ui::PhotoSyncClass m_ui;
    QString m_positiveDefaultText;
    Settings *m_settings;
    FileExplorerDialog *m_fileDialog;
    FileManager *m_fileManager;
};
