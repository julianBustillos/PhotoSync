#include "MTPFileSystem.h"
#include "PhotoSync.h"


/* MTPFileSystem::DirIterator */
MTPFileSystem::DirIterator::DirIterator(const QString & rootPath, const QStringList & extensions) :
    m_rootLevel(rootPath.count('/') + 1), m_extensions(extensions)
{
    findNext(rootPath + "/");
}

bool MTPFileSystem::DirIterator::hasNext() const
{
    return !m_next.isEmpty();
}

QString MTPFileSystem::DirIterator::next()
{
    if (hasNext()) {
        QString current = m_next;
        findNext(current);
        return current;
    }
    return QString();
}

void MTPFileSystem::DirIterator::findNext(const QString & current)
{
    m_next = QString();
    QStringList splittedPath = current.split('/');
    int level = splittedPath.size();
    bool searching = true;

    while (searching && level > m_rootLevel) {
        QString path = splittedPath[0];
        for (int i = 1; i < level - 1; i++)
            path += "/" + splittedPath[i];
        QVector<WPDManager::Item> content;
        PhotoSync::getWPDInstance().getContent(path, content);

        int id = -1;
        if (!splittedPath[level - 1].isEmpty()) {
            for (id = 0; id < content.size(); id++) {
                if (content[id].m_name == splittedPath[level - 1])
                    break;
            }
        }
        for (++id; id < content.size() && searching; id++) {
            if (content[id].m_type == WPDManager::FILE) {
                int pointId = content[id].m_name.lastIndexOf('.') + 1;
                QStringRef currentExtRef(&content[id].m_name, pointId, content[id].m_name.size() - pointId);

                for (const QString &extension : m_extensions) {
                    QStringRef filterExtRef(&extension, 2, extension.size() - 2);
                    if (currentExtRef.compare(filterExtRef, Qt::CaseInsensitive) == 0) {
                        splittedPath[level - 1] = content[id].m_name;
                        searching = false;
                        break;
                    }
                }
                continue;
            }
            else {
                if (splittedPath.size() == level)
                    splittedPath.append("");
                splittedPath[level - 1] = content[id].m_name;
                splittedPath[level++] = "";
                break;
            }
        }

        if (searching && id == content.size())
            level--;
    }

    if (!searching) {
        m_next = splittedPath[0];
        for (int i = 1; i < level; i++)
            m_next += "/" + splittedPath[i];
    }
}


/* MTPFileSystem::FileInfo */
MTPFileSystem::FileInfo::FileInfo(const QString & path) :
    m_path(path)
{
}

MTPFileSystem::FileInfo::~FileInfo()
{
}

QString MTPFileSystem::FileInfo::path() const
{
    return m_path;
}

QString MTPFileSystem::FileInfo::fileName() const
{
    int index = m_path.lastIndexOf("/");
    return m_path.right(m_path.size() - index - 1);
}

QString MTPFileSystem::FileInfo::suffix() const
{
    int lastSeparator = m_path.lastIndexOf('/');
    int lastPoint = m_path.lastIndexOf('.');
    if (lastSeparator >= lastPoint)
        return QString();
    return m_path.right(m_path.size() - lastPoint - 1);
}

void MTPFileSystem::FileInfo::setFile(const QString &name)
{
    int index = m_path.lastIndexOf("/");
    m_path = m_path.left(index + 1) + name;
}

int MTPFileSystem::FileInfo::size() const
{
    WPDManager::Item item;
    PhotoSync::getWPDInstance().getItem(m_path, item);
    return item.m_size; //TODO: check size MTP vs CLASSIC
}

bool MTPFileSystem::FileInfo::exists() const
{
    return PhotoSync::getWPDInstance().getItem(m_path, WPDManager::Item());
}


/* MTPFileSystem::File */
MTPFileSystem::File::File(const QString & path) :
    m_mode(QIODevice::NotOpen), m_path(path), m_stream(nullptr), m_size(0)
{
}

MTPFileSystem::File::~File()
{
    close();
}

bool MTPFileSystem::File::open(QIODevice::OpenMode mode)
{
    m_mode = mode;
    switch (m_mode) {
    case QIODevice::ReadOnly:
        return PhotoSync::getWPDInstance().getStream(m_path, &m_stream, m_size);
    case QIODevice::WriteOnly:
        //TODO
        break;
    }

    m_mode = QIODevice::NotOpen;
    return false;
}

qint64 MTPFileSystem::File::write(const QByteArray & byteArray)
{
    if (m_mode == QIODevice::WriteOnly) {
        //TODO
    }
    return qint64();
}

QByteArray MTPFileSystem::File::readAll()
{
    if (m_mode == QIODevice::ReadOnly && m_stream && m_size > 0) {
        ULONG readBytes = 0;
        QByteArray byteArray(m_size, '0');
        if ((m_stream->Read(byteArray.data(), m_size, &readBytes) == S_OK) && (readBytes == m_size))
            return byteArray;
    }
    return QByteArray();
}

void MTPFileSystem::File::close()
{
    if (m_stream)
        m_stream->Release();
    m_stream = nullptr;

    m_size = 0;
}

/* MTPFileSystem::Dir */
MTPFileSystem::Dir::Dir(const QString & path) :
    m_path(path)
{
}

MTPFileSystem::Dir::~Dir()
{
}

QString MTPFileSystem::Dir::path(const QString &concatenate) const
{
    return m_path + "/" + QString(concatenate).replace("\\", "/");
}

bool MTPFileSystem::Dir::exists() const
{
    return PhotoSync::getWPDInstance().getItem(m_path, WPDManager::Item());
}

bool MTPFileSystem::Dir::mkpath(const QString & dirPath) const
{
    QString tempPath = dirPath;
    QStringList folders = tempPath.replace("\\", "/").split("/");
    
    bool created = true;
    int index = 0;
    QString existingPath = m_path;
    while (index < folders.size() && created) {
        created = PhotoSync::getWPDInstance().createFolder(existingPath, folders[index]);
        existingPath += "/" + folders[index];
        index++;
    }

    return created;
}