#include "FileManager.h"
#include "PhotoSync.h"
#include <QMessageBox>
#include <QDir>
#include <QDirIterator>
#include <QCryptographicHash>
#include "easyexif\exif.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}


FileManager::FileManager(QWidget *parent, Ui::PhotoSyncClass &ui) :
    m_runCount(0), m_parent(parent), m_ui(ui)
{
    m_extensions << "*.jpg" << "*.mp4";
}

FileManager::~FileManager()
{
}

void FileManager::run()
{
    m_ui.progressBar->setValue(0);
    m_ui.progressBar->setMaximum(100);

    if (checkDir()) {
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

        if (m_runCount++)
            m_ui.textEditOutput->append("");

        m_ui.textEditOutput->append("FROM : " + m_ui.importEdit->text());
        m_ui.textEditOutput->append("TO      : " + m_ui.exportEdit->text());
        
        m_duplicateCount = 0;
        m_copyCount = 0;
        m_canceled = false;

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
    if (m_ui.importEdit->text().isEmpty()) {
        QMessageBox::warning(m_parent, "Path error", "Import path is empty !");
        return false;
    }
    if (!QDir(m_ui.importEdit->text()).exists()) {
        QMessageBox::warning(m_parent, "Path error", "Import path : \"" + m_ui.importEdit->text() + "\" not found !");
        return false;
    }

    if (m_ui.exportEdit->text().isEmpty()) {
        QMessageBox::warning(m_parent, "Path error", "Export path is empty !");
        return false;
    }
    if (!QDir(m_ui.exportEdit->text()).exists()) {
        QMessageBox::warning(m_parent, "Path error", "Export path : \"" + m_ui.exportEdit->text() + "\" not found !");
        return false;
    }

    return true;
}

bool FileManager::getDate(const QFileInfo & fileInfo, Date &date)
{
    bool readError = false;

    if (fileInfo.suffix().compare("jpg", Qt::CaseInsensitive) == 0) {
        easyexif::EXIFInfo exifInfo;
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray fileData = file.readAll();
            int parsingResult = exifInfo.parseFrom((const unsigned char *)fileData.constData(), fileData.size());
            if (parsingResult == PARSE_EXIF_SUCCESS) {
                date.m_year = std::stoi(exifInfo.DateTime.substr(0, 4));
                date.m_month = std::stoi(exifInfo.DateTime.substr(5, 2));
            }
            file.close();
        }
        else {
            readError = true;
        }
    }
    else if (fileInfo.suffix().compare("mp4", Qt::CaseInsensitive) == 0) {
        AVFormatContext *fmt_ctx = NULL;
        AVDictionaryEntry *tag = NULL;

        int ret = avformat_open_input(&fmt_ctx, fileInfo.absoluteFilePath().toLocal8Bit().data(), NULL, NULL);
        if (ret == 0) {
            tag = av_dict_get(fmt_ctx->metadata, "creation_time", tag, AV_DICT_IGNORE_SUFFIX);
            if (tag) {
                std::string value = tag->value;
                date.m_year = std::stoi(value.substr(0, 4));
                date.m_month = std::stoi(value.substr(5, 2));
            }
        }
        else {
            readError = true;
        }
        avformat_close_input(&fmt_ctx);
    }
    else {
        date.m_year = -1;
        date.m_month = -1;
    }

    return !readError;
}

void FileManager::cancel()
{
    m_canceled = true;
}

void FileManager::buildExistingFileData()
{
    QDirIterator it(m_ui.exportEdit->text(), m_extensions, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        if (m_canceled)
            return;

        QFileInfo fileInfo(it.next());
        m_existingFiles[fileInfo.size()].emplace_back(fileInfo.filePath());
        if (m_canceled)
            return;
    }
}

void FileManager::buildImportFileData()
{
    QDirIterator it(m_ui.importEdit->text(), m_extensions, QDir::Files, QDirIterator::Subdirectories);
    QStringList importFilePaths;

    while (it.hasNext())
        importFilePaths.append(it.next());

    size_t progressSize = importFilePaths.size();
    if (progressSize == 0)
        m_ui.progressBar->setValue(100);
    else
        m_ui.progressBar->setMaximum(progressSize);

    for (QString filePath : importFilePaths) {
        if (m_canceled)
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
                    m_ui.progressBar->setValue(m_ui.progressBar->value() + 1);
                    break;
                }
            }
            newFile.close();
        }

        if (copyFile) {
            Date date;
            if (!getDate(fileInfo, date)) {
                m_importErrors.insert(fileInfo.filePath());
                continue;
            }
            m_DirectoriesToCreate.insert(date);
            m_filesToCopy.emplace_back(date, fileInfo.filePath());
        }
    }
}

void FileManager::exportFiles()
{
    QDir exportPath(m_ui.exportEdit->text());

    for (auto &date : m_DirectoriesToCreate) {
        exportPath.mkpath(date.toQString());
    }

    for (auto &file : m_filesToCopy) {
        if (m_canceled)
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
        m_ui.progressBar->setValue(m_ui.progressBar->value() + 1);
    }

}

void FileManager::printStats()
{
    if (m_duplicateCount > 0)
        m_ui.textEditOutput->append("Found " + QString::number(m_duplicateCount) + (m_duplicateCount > 1 ? " already existing files." : " already existing file."));

    m_ui.textEditOutput->append(QString::number(m_copyCount) + (m_copyCount > 1 ? " files copied." : " file copied."));

    if (m_exportErrors.size() > 0) {
        m_ui.textEditOutput->append("ERROR : " + QString::number(m_exportErrors.size()) + ((m_exportErrors.size() > 1) ? " files in export directory could not be read !" : " file in export directory could not be read !"));
    }

    if (m_importErrors.size() > 0) {
        m_ui.textEditOutput->append("ERROR : " + QString::number(m_importErrors.size()) + ((m_importErrors.size() > 1) ? " files in import directory could not be read !" : " file in import directory could not be read !"));
    }

    if (m_canceled) {
        m_ui.textEditOutput->append("SYNC CANCELED !");
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
    m_ui.textEditOutput->append("Elapsed time : " + timeToPrint);
}
