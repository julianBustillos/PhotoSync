#pragma once
#include <QThread>
#include <unordered_map>
#include <vector>
#include <set>
#include <chrono>
#include "FileData.h"
#include "ExtendedFileSystem.h"


class FileManager : public QThread
{
    Q_OBJECT

public:
    FileManager(QObject *parent = nullptr);
    ~FileManager();

public:
    void setPaths(const QString &importPath, const QString &exportPath);

public slots:
    void cancel();

signals:
    void warning(QString title, QString message);
    void progressBarValue(int value);
    void progressBarMaximum(int maximum);
    void output(QString output);

private:
    void run() override;

private:
    bool checkDir();
    bool getDate(const EFS::Info &fileInfo, Date &date);
    void buildExistingFileData();
    void buildImportFileData();
    void exportFiles();
    void printStats();
    void printElapsedTime(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end);

private:
    QAtomicInt m_cancelled;
    EFS::Path m_importPath;
    EFS::Path m_exportPath;
    const QStringList m_extensions;
    int m_runCount;
    int m_progress;
    std::unordered_map<qint64, std::vector<ExistingFile>> m_existingFiles;
    std::set<Date> m_DirectoriesToCreate;
    std::vector<ExportFile> m_filesToCopy;
    std::set<EFS::Path> m_importErrors;
    std::set<EFS::Path> m_exportErrors;
    int m_duplicateCount;
    int m_copyCount;
};