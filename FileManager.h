#pragma once
#include <ui_PhotoSync.h>
#include <QFileInfo>
#include <unordered_map>
#include <vector>
#include <set>
#include "FileData.h"


class FileManager
{
public:
    FileManager(QWidget *parent, Ui::PhotoSyncClass &ui);
    ~FileManager();
    void run();

private:
    bool checkDir();
    Date getDate(const QFileInfo &fileInfo);
    void buildExistingFileData();
    void buildImportFileData();
    void exportFiles();

private:
    QWidget *m_parent;
    Ui::PhotoSyncClass &m_ui;
    QStringList m_extensions;
    std::unordered_map<qint64, std::vector<ExistingFile>> m_existingFiles;
    std::set<Date> m_exportDirectories;
    std::vector<ExportFile> m_exportFiles;
};