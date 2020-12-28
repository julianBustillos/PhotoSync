#include "FileManager.h"
#include <QDir>
#include <QDirIterator>
#include <QCryptographicHash>
#include "DateParser.h"


FileManager::FileManager(QObject *parent) :
    QThread(parent), m_importPath(""), m_exportPath(""), m_extensions({ "*.jpg" , "*.mp4" }), m_runCount(0), m_cancelled(false)
{
}

FileManager::~FileManager()
{
}

void FileManager::setPaths(const QString & importPath, const QString & exportPath)
{
    m_importPath = importPath;
    m_exportPath = exportPath;
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

    if (checkDir()) {
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

        if (m_runCount++)
            emit output(QString());

        emit output("FROM : " + m_importPath);
        emit output("TO      : " + m_exportPath);
        
        m_duplicateCount = 0;
        m_copyCount = 0;

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
}

bool FileManager::checkDir()
{
    if (m_importPath.isEmpty()) {
        emit warning("Path error", "Import path is empty !");
        return false;
    }
    if (!QDir(m_importPath).exists()) {
        emit warning("Path error", "Import path : \"" + m_importPath + "\" not found !");
        return false;
    }

    if (m_exportPath.isEmpty()) {
        emit warning("Path error", "Export path is empty !");
        return false;
    }
    if (!QDir(m_exportPath).exists()) {
        emit warning("Path error", "Export path : \"" + m_exportPath + "\" not found !");
        return false;
    }

    return true;
}

bool FileManager::getDate(const QFileInfo & fileInfo, Date &date)
{
    bool isParsed = false;

    if (fileInfo.suffix().compare("jpg", Qt::CaseInsensitive) == 0) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly)) {
            DateParser::fromJPGBuffer(file.readAll(), date);
            file.close();
            isParsed = true;
        }
    }
    else if (fileInfo.suffix().compare("mp4", Qt::CaseInsensitive) == 0) {
        DateParser::fromMP4FilePath(fileInfo.absoluteFilePath(), date);
        isParsed = true;
    }

    return isParsed;
}

void FileManager::buildExistingFileData()
{
    QDirIterator it(m_exportPath, m_extensions, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        if (m_cancelled.loadRelaxed())
            return;

        QFileInfo fileInfo(it.next());
        m_existingFiles[fileInfo.size()].emplace_back(fileInfo.filePath());
    }
}

void FileManager::buildImportFileData()
{
    QDirIterator it(m_importPath, m_extensions, QDir::Files, QDirIterator::Subdirectories);
    QStringList importFilePaths;

    while (it.hasNext())
        importFilePaths.append(it.next());

    size_t progressSize = importFilePaths.size();
    if (progressSize == 0)
        emit progressBarValue(m_progress = 100);
    else
        emit progressBarMaximum(progressSize);

    for (QString filePath : importFilePaths) {
        if (m_cancelled.loadRelaxed())
            return;

        bool copyFile = true;

        QFileInfo fileInfo(filePath);
        auto filesIt = m_existingFiles.find(fileInfo.size());
        if (filesIt != m_existingFiles.end()) {
            QCryptographicHash hash(QCryptographicHash::Md5);
            QFile newFile(fileInfo.filePath());
            if (!newFile.open(QIODevice::ReadOnly)) {
                m_importErrors.insert(fileInfo.filePath());
                continue;
            }
            hash.addData(newFile.readAll());
            QByteArray newFileChecksum = hash.result();

            for (auto &fileData : filesIt->second) {
                if (fileData.m_checksum.isEmpty()) {
                    QFile existingFile(fileData.m_path);
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
            newFile.close();
        }

        if (copyFile) {
            Date date;
            if (!getDate(fileInfo, date)) {
                m_importErrors.insert(fileInfo.filePath());
                emit progressBarValue(++m_progress);
                continue;
            }
            m_DirectoriesToCreate.insert(date);
            m_filesToCopy.emplace_back(date, fileInfo.filePath());
        }
    }
}

void FileManager::exportFiles()
{
    QDir exportPath(m_exportPath);

    for (auto &date : m_DirectoriesToCreate) {
        exportPath.mkpath(date.toQString());
    }

    for (auto &file : m_filesToCopy) {
        if (m_cancelled.loadRelaxed())
            return;

        QFileInfo importFileInfo(file.m_path);
        QString fileName = importFileInfo.fileName();
        QFileInfo exportFileInfo(exportPath.absoluteFilePath(file.m_date.toQString() + "\\" + fileName));
        
        int count = 0;
        bool copyResult = false;
        while (exportFileInfo.exists() && count < 100) {
            exportFileInfo.setFile(fileName + "_" + QString::number(++count));
        }

        if (count < 100)
            copyResult = QFile::copy(importFileInfo.filePath(), exportFileInfo.filePath());
        if (copyResult)
            m_copyCount++;
        else
            m_importErrors.insert(importFileInfo.filePath());
        emit progressBarValue(++m_progress);
    }

}

void FileManager::printStats()
{
    if (m_duplicateCount > 0)
        emit output("Found " + QString::number(m_duplicateCount) + (m_duplicateCount > 1 ? " already existing files." : " already existing file."));

    emit output(QString::number(m_copyCount) + (m_copyCount > 1 ? " files copied." : " file copied."));

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
        elapsedTime -= s * 60;
    }

    s = elapsedTime;

    QString timeToPrint = QString::number(h).rightJustified(2, '0') + ":" + QString::number(m).rightJustified(2, '0') + ":" + QString::number(s).rightJustified(2, '0');
    emit output("Elapsed time : " + timeToPrint);
}
