#include "FileManager.h"
#include <QCryptographicHash>
#include "DateParser.h"


FileManager::FileManager(QObject *parent) :
    QThread(parent), m_importPath(""), m_exportPath(""), m_extensions({ "*.jpg" , "*.mp4" }), m_runCount(0), m_cancelled(false), m_status(false)
{
}

FileManager::~FileManager()
{
    cancel();
    wait();
}

void FileManager::setSettings(const QString & importPath, const QString & exportPath, bool removeFiles)
{
    m_importPath = importPath;
    m_exportPath = exportPath;
    m_removeFiles = removeFiles;
}

bool FileManager::getStatus() const
{
    return m_status;
}

void FileManager::warningAnswer(bool answer)
{
    QMutexLocker locker(&m_mutex);
    m_removeFiles = answer;
    m_condition.wakeAll();
}

void FileManager::cancel()
{
    m_cancelled.storeRelaxed(true);
}

void FileManager::run()
{
    m_cancelled.storeRelaxed(false);
    emit progressBarValue(m_progress = 0);
    emit progressBarMaximum(100);

    m_status = checkDir() && checkRemove();

    if (m_status) {
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

        if (m_runCount++)
            emit output(QString());

        emit output("FROM : " + m_importPath.toQString());
        emit output("TO      : " + m_exportPath.toQString());
        
        m_duplicateCount = 0;
        m_copyCount = 0;
        m_removeCount = 0;

        buildExistingFileData();
        buildImportFileData();
        exportFiles();
        printStats();

        m_importErrors.clear();
        m_exportErrors.clear();
        m_existingFiles.clear();
        m_DirectoriesToCreate.clear();
        m_filesToCopy.clear();

        std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
        printElapsedTime(startTime, endTime);
    }

    m_status &= m_cancelled.loadRelaxed();
}

bool FileManager::checkDir()
{
    if (m_importPath.isEmpty()) {
        emit warning("Path error", "Import path is empty !", false);
        return false;
    }
    if (!EFS::Dir(m_importPath).exists()) {
        emit warning("Path error", "Import path : \"" + m_importPath.toQString() + "\" not found !", false);
        return false;
    }

    if (m_exportPath.isEmpty()) {
        emit warning("Path error", "Export path is empty !", false);
        return false;
    }
    if (!EFS::Dir(m_exportPath).exists()) {
        emit warning("Path error", "Export path : \"" + m_exportPath.toQString() + "\" not found !", false);
        return false;
    }

    return true;
}

bool FileManager::checkRemove()
{
    if (m_removeFiles) {
        QMutexLocker locker(&m_mutex);
        emit warning("Remove warning", "Files are going to be removed after copy. Proceed anyway ?", true);
        m_condition.wait(&m_mutex);
        return m_removeFiles;
    }
    return true;
}

bool FileManager::getDate(const EFS::FileInfo &fileInfo, Date &date)
{
    bool isParsed = false;

    if (fileInfo.suffix().compare("jpg", Qt::CaseInsensitive) == 0) {
        EFS::File file(fileInfo.path());
        if (file.open(QIODevice::ReadOnly)) {
            DateParser::fromJPGBuffer(file.readAll(), date);
            file.close();
            isParsed = true;
        }
    }
    else if (fileInfo.suffix().compare("mp4", Qt::CaseInsensitive) == 0) {
        EFS::File file(fileInfo.path());
        if (file.open(QIODevice::ReadOnly)) {
            DateParser::fromMPRBuffer(file.readAll(), date);
            file.close();
            isParsed = true;
        }
    }

    return isParsed;
}

void FileManager::buildExistingFileData()
{
    EFS::DirIterator it(m_exportPath, m_extensions);
    while (it.hasNext()) {
        if (m_cancelled.loadRelaxed())
            return;

        EFS::FileInfo fileInfo(it.next());
        m_existingFiles[fileInfo.size()].emplace_back(fileInfo.path());
    }
}

void FileManager::buildImportFileData()
{
    EFS::DirIterator it(m_importPath, m_extensions);
    QVector<EFS::Path> importFilePaths;
    QVector<EFS::Path> filesToRemove;

    while (it.hasNext())
        importFilePaths.append(it.next());

    size_t progressSize = importFilePaths.size();
    if (progressSize == 0)
        emit progressBarValue(m_progress = 100);
    else
        emit progressBarMaximum(progressSize);

    for (EFS::Path &filePath : importFilePaths) {
        if (m_cancelled.loadRelaxed())
            return;

        bool copyFile = true;

        EFS::FileInfo fileInfo(filePath);
        auto filesIt = m_existingFiles.find(fileInfo.size());
        if (filesIt != m_existingFiles.end()) {
            QCryptographicHash hash(QCryptographicHash::Md5);
            EFS::File newFile(fileInfo.path());
            if (!newFile.open(QIODevice::ReadOnly)) {
                m_importErrors.insert(fileInfo.path());
                continue;
            }
            hash.addData(newFile.readAll());
            QByteArray newFileChecksum = hash.result();
            newFile.close();

            for (auto &fileData : filesIt->second) {
                if (fileData.m_checksum.isEmpty()) {
                    EFS::File existingFile(fileData.m_path.toQString());
                    if (!existingFile.open(QIODevice::ReadOnly)) {
                        m_exportErrors.insert(fileData.m_path);
                        continue;
                    }

                    hash.reset();
                    hash.addData(existingFile.readAll());
                    fileData.m_checksum = hash.result();
                    existingFile.close();
                }
                
                if (newFileChecksum == fileData.m_checksum) {
                    copyFile = false;
                    m_duplicateCount++;
                    emit progressBarValue(++m_progress);
                    break;
                }
            }
        }

        if (copyFile) {
            Date date;
            if (!getDate(fileInfo, date)) {
                m_importErrors.insert(fileInfo.path());
                emit progressBarValue(++m_progress);
                continue;
            }
            m_DirectoriesToCreate.insert(date);
            m_filesToCopy.emplace_back(date, fileInfo.path());
        }
        else if (m_removeFiles) {
            filesToRemove.append(filePath);
        }
    }

    for (EFS::Path &path : filesToRemove) {
        EFS::File file(path);
        if (file.remove())
            m_removeCount++;
        else
            m_importErrors.insert(path);
    }
}

void FileManager::exportFiles()
{
    EFS::Dir exportPath(m_exportPath);

    for (auto &date : m_DirectoriesToCreate) {
        exportPath.mkpath(date.toQString());
    }

    for (auto &file : m_filesToCopy) {
        if (m_cancelled.loadRelaxed())
            return;

        EFS::FileInfo importFileInfo(file.m_path);
        QString fileName = importFileInfo.fileName();
        EFS::FileInfo exportFileInfo(exportPath.path(file.m_date.toQString() + "/" + fileName));
        
        int count = 0;
        bool copy = false;
        bool remove = false;

        if (exportFileInfo.exists()) {
            int index = fileName.lastIndexOf(".");
            QString name = fileName.left(index);
            QString extension = fileName.right(fileName.size() - index);
            while (exportFileInfo.exists() && count < 100) {
                exportFileInfo.setFile(name + "_" + QString::number(++count) + extension);
            }
        }

        if (count < 100) {
            EFS::File importFile(importFileInfo.path());
            EFS::File exportFile(exportFileInfo.path());

            if (importFile.open(QIODevice::ReadOnly) && exportFile.open(QIODevice::WriteOnly))
                copy = (exportFile.write(importFile.readAll()) >= 0);

            importFile.close();
            exportFile.close();

            if (copy && m_removeFiles)
                remove = importFile.remove();
        }

        if (copy)
            m_copyCount++;

        if (remove)
            m_removeCount++;

        if (!copy || (m_removeFiles && !remove))
            m_importErrors.insert(importFileInfo.path());
        emit progressBarValue(++m_progress);
    }

}

void FileManager::printStats()
{
    if (m_duplicateCount > 0)
        emit output("Found " + QString::number(m_duplicateCount) + (m_duplicateCount > 1 ? " already existing files." : " already existing file."));

    emit output(QString::number(m_copyCount) + (m_copyCount > 1 ? " files copied." : " file copied."));

    if (m_removeCount > 0) {
        emit output(QString::number(m_removeCount) + (m_removeCount > 1 ? " files removed." : " file removed."));
    }

    if (m_exportErrors.size() > 0) {
        emit output("ERROR : " + QString::number(m_exportErrors.size()) + ((m_exportErrors.size() > 1) ? " files in export directory could not be read !" : " file in export directory could not be read !"));
    }

    if (m_importErrors.size() > 0) {
        emit output("ERROR : " + QString::number(m_importErrors.size()) + ((m_importErrors.size() > 1) ? " files in import directory could not be read !" : " file in import directory could not be read !"));
    }

    if (m_cancelled.loadRelaxed()) {
        emit output("SYNC CANCELED !");
    }
}

void FileManager::printElapsedTime(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end)
{
    int elapsedTime = (int)std::round(std::chrono::duration_cast<std::chrono::seconds>(end - start).count());
    int h = 0, m = 0, s = 0;

    if (elapsedTime >= 3600) {
        h = elapsedTime / 3600;
        elapsedTime -= h * 3600;
    }

    if (elapsedTime >= 60) {
        m = elapsedTime / 60;
        elapsedTime -= m * 60;
    }

    s = elapsedTime;

    QString timeToPrint = QString::number(h).rightJustified(2, '0') + ":" + QString::number(m).rightJustified(2, '0') + ":" + QString::number(s).rightJustified(2, '0');
    emit output("Elapsed time : " + timeToPrint);
}
