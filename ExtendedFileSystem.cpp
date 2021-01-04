#include "ExtendedFileSystem.h"


/* ExtendedFileSystem::Path */
ExtendedFileSystem::Path::Path(const QString & path) :
    m_path(path), m_type(INVALID)
{
    if (!m_path.isEmpty()) {
        //TODO MTP
        m_type = SYSTEM;
    }
}

ExtendedFileSystem::Path::~Path()
{
}

const QString & ExtendedFileSystem::Path::toQString() const
{
    return m_path;
}

bool ExtendedFileSystem::Path::isEmpty() const
{
    return m_path.isEmpty();
}

bool ExtendedFileSystem::Path::operator<(const Path & rhs) const
{
    return m_path < rhs.m_path;
}


/* ExtendedFileSystem::Iterator */
ExtendedFileSystem::Iterator::Iterator(const Path & path, const QStringList & extensions)
{
    //TODO MTP
    m_iteratorImpl = new QDirIterator(path.toQString(), extensions, QDir::Files, QDirIterator::Subdirectories);
}

ExtendedFileSystem::Iterator::Iterator()
{
    if (m_iteratorImpl)
        delete m_iteratorImpl;
    m_iteratorImpl = nullptr;

}

bool ExtendedFileSystem::Iterator::hasNext() const
{
    //TODO MTP
    if (m_iteratorImpl)
        return m_iteratorImpl->hasNext();
    return false;
}

ExtendedFileSystem::Path ExtendedFileSystem::Iterator::next()
{
    //TODO MTP
    if (m_iteratorImpl)
        return Path(m_iteratorImpl->next());
    return Path(QString());
}


/* ExtendedFileSystem::Info */
ExtendedFileSystem::Info::Info(const Path & path)
{
    //TODO MTP
    m_infoImpl = new QFileInfo(path.toQString());
}

ExtendedFileSystem::Info::~Info()
{
    if (m_infoImpl)
        delete m_infoImpl;
    m_infoImpl = nullptr;
}

ExtendedFileSystem::Path ExtendedFileSystem::Info::path() const
{
    //TODO MTP
    if (m_infoImpl)
        return Path(m_infoImpl->absoluteFilePath());
    return Path(QString());
}

QString ExtendedFileSystem::Info::fileName() const
{
    //TODO MTP
    if (m_infoImpl)
        return m_infoImpl->fileName();
    return QString();
}

void ExtendedFileSystem::Info::setFile(QString name)
{
    //TODO MTP
    if (m_infoImpl)
        m_infoImpl->setFile(name);
}

QString ExtendedFileSystem::Info::suffix() const
{
    //TODO MTP
    if (m_infoImpl)
        return m_infoImpl->suffix();
    return QString();
}

int ExtendedFileSystem::Info::size() const
{
    //TODO MTP
    if (m_infoImpl)
        return m_infoImpl->size();
    return 0;
}

bool ExtendedFileSystem::Info::exists() const
{
    //TODO MTP
    if (m_infoImpl)
        return m_infoImpl->exists();
    return false;
}


/* ExtendedFileSystem::File */
ExtendedFileSystem::File::File(const Path & path)
//TODO MTP
{
    m_fileImpl = new QFile(path.toQString());
}

ExtendedFileSystem::File::~File()
{
    if (m_fileImpl)
        delete m_fileImpl;
    m_fileImpl = nullptr;
}

bool ExtendedFileSystem::File::open()
{
    //TODO MTP
    if (m_fileImpl)
        return m_fileImpl->open(QIODevice::ReadOnly);
    return false;
}

QByteArray ExtendedFileSystem::File::readAll()
{
    //TODO MTP
    if (m_fileImpl)
        return m_fileImpl->readAll();
    return QByteArray();
}

void ExtendedFileSystem::File::close()
{
    //TODO MTP
    if (m_fileImpl)
        m_fileImpl->close();
}

bool ExtendedFileSystem::File::copy(const Path & sourcePath, const Path & destPath)
{
    //TODO MTP
    return QFile::copy(sourcePath.toQString(), destPath.toQString());
}


/* ExtendedFileSystem::Dir */
ExtendedFileSystem::Dir::Dir(const Path & path)
{
    //TODO MTP
    m_dirImpl = new QDir(path.toQString());
}

ExtendedFileSystem::Dir::~Dir()
{
    if (m_dirImpl)
        delete m_dirImpl;
    m_dirImpl = nullptr;
}

ExtendedFileSystem::Path ExtendedFileSystem::Dir::path(QString concatenate) const
{
    //TODO MTP
    if (m_dirImpl)
        return Path(m_dirImpl->absoluteFilePath(concatenate));
    return Path();
}

bool ExtendedFileSystem::Dir::exists() const
{
    //TODO MTP
    if (m_dirImpl)
        m_dirImpl->exists();
    return false;
}

bool ExtendedFileSystem::Dir::mkpath(const QString & dirPath) const
{
    //TODO MTP
    if (m_dirImpl)
        m_dirImpl->mkpath(dirPath);
    return false;
}
