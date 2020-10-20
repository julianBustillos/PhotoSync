#include "FileManager.h"
#include <QMessageBox>
#include <QDir>
#include <QDirIterator>
#include <QCryptographicHash>
#include "easyexif\exif.h"


FileManager::FileManager(QWidget *parent, Ui::PhotoSyncClass &ui) :
    m_parent(parent), m_ui(ui)
{
    m_extensions << "*.jpg";
}

FileManager::~FileManager()
{
}

void FileManager::run()
{
    if (checkDir()) {
        m_existingFiles.clear();
        m_exportDirectories.clear();
        m_exportFiles.clear();

        buildExistingFileData();
        buildImportFileData();
        exportFiles();
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

Date FileManager::getDate(const QFileInfo & fileInfo)
{
    if (fileInfo.suffix().compare("jpg", Qt::CaseInsensitive) == 0) {
        easyexif::EXIFInfo exifInfo;
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray fileData = file.readAll();
            int parsingResult = exifInfo.parseFrom((const unsigned char *)fileData.constData(), fileData.size());
            if (parsingResult == PARSE_EXIF_SUCCESS) {
                return Date(std::stoi(exifInfo.DateTime.substr(0, 4)), std::stoi(exifInfo.DateTime.substr(5, 2)));
            }
        }

    }

    return Date();
}

void FileManager::buildExistingFileData()
{
    QDirIterator it(m_ui.exportEdit->text(), m_extensions, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo fileInfo(it.next());
        m_existingFiles[fileInfo.size()].emplace_back(fileInfo.filePath());
    }
}

void FileManager::buildImportFileData()
{
    QDirIterator it(m_ui.importEdit->text(), m_extensions, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo fileInfo(it.next());
        bool copyFile = true;

        auto filesIt = m_existingFiles.find(fileInfo.size());
        if (filesIt != m_existingFiles.end()) {
            QCryptographicHash hash(QCryptographicHash::Md5);
            QFile newFile(fileInfo.filePath());
            if (!newFile.open(QIODevice::ReadOnly)) 
                continue;
            hash.addData(newFile.readAll());
            QByteArray newFileChecksum = hash.result();

            for (auto &fileData : filesIt->second) {
                if (fileData.m_checksum.isEmpty()) {
                    QFile existingFile(fileData.m_path);
                    if (!existingFile.open(QIODevice::ReadOnly))
                        continue;

                    hash.reset();
                    hash.addData(existingFile.readAll());
                    fileData.m_checksum = hash.result();
                }
                
                if (newFileChecksum == fileData.m_checksum) {
                    copyFile = false;
                    break;
                }
            }
        }

        if (copyFile) {
            Date date = getDate(fileInfo);
            m_exportDirectories.insert(date);
            m_exportFiles.emplace_back(date, fileInfo.filePath());
        }
    }
}

void FileManager::exportFiles()
{
    QDir exportPath(m_ui.exportEdit->text());

    for (auto &date : m_exportDirectories) {
        exportPath.mkpath(date.toQString());
    }

    for (auto &file : m_exportFiles) {
        QFileInfo importFileInfo(file.m_path);
        QString fileName = importFileInfo.fileName();
        QFileInfo exportFileInfo(exportPath.absoluteFilePath(file.m_date.toQString() + "\\" + fileName));
        
        int count = 0;
        while (exportFileInfo.exists()) {
            exportFileInfo.setFile(fileName + "_" + QString::number(++count));
        }

        QFile::copy(importFileInfo.filePath(), exportFileInfo.filePath());
    }
}
