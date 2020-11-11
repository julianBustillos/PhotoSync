#pragma once
#include <ui_PhotoSync.h>
#include <QFileInfo>
#include <unordered_map>
#include <vector>
#include <set>
#include <chrono>
#include "FileData.h"


class FileManager
{
public:
    FileManager(QWidget *parent, Ui::PhotoSyncClass &ui);
    ~FileManager();
    void run();
    void cancel();

private:
    bool checkDir();
    bool getDate(const QFileInfo &fileInfo, Date &date);
    void buildExistingFileData();
    void buildImportFileData();
    void exportFiles();
    void printStats();
    void printElapsedTime(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end);

private:
    int m_runCount;
    QWidget *m_parent;
    Ui::PhotoSyncClass &m_ui;
    QStringList m_extensions;
    std::unordered_map<qint64, std::vector<ExistingFile>> m_existingFiles;
    std::set<Date> m_DirectoriesToCreate;
    std::vector<ExportFile> m_filesToCopy;
    int m_copyCount;
    int m_importErrors;
    int m_exportErrors;
    bool m_canceled;
};