#pragma once
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
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
    void setSettings(const QString &importPath, const QString &exportPath, bool removeFiles);
    bool getStatus() const;

public slots:
    void warningAnswer(bool answer);
    void cancel();

signals:
    void warning(QString title, QString message, bool emitAnswer);
    void progressBarValue(int value);
    void progressBarMaximum(int maximum);
    void output(QString output);

private:
    void run() override;

private:
    bool checkDir();
    bool checkRemove();
    bool getDate(const EFS::FileInfo &fileInfo, Date &date);
    void buildExistingFileData();
    void buildImportFileData();
    void exportFiles();
    void removeFiles();
    void printStats();
    void printElapsedTime(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end);

private:
    QMutex m_mutex;
    QWaitCondition m_condition;
    QAtomicInt m_cancelled;

private:
    EFS::Path m_importPath;
    EFS::Path m_exportPath;
    bool m_removeFiles;
    const QStringList m_extensions;
    int m_runCount;
    int m_progress;
    int m_copyProgress;
    int m_removeProgress;
    std::unordered_map<qint64, std::vector<ExistingFile>> m_existingFiles;
    std::set<Date> m_DirectoriesToCreate;
    std::vector<ExportFile> m_filesToCopy;
    std::vector<EFS::Path> m_filesToRemove;
    std::set<EFS::Path> m_importErrors;
    std::set<EFS::Path> m_exportErrors;
    int m_duplicateCount;
    int m_copyCount;
    int m_removeCount;
    int m_status;
};