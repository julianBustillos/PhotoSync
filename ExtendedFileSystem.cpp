#include "ExtendedFileSystem.h"
#include "MTPFileSystem.h"
#include "PhotoSync.h"
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QStringRef>


/* ExtendedFileSystem::Path */
ExtendedFileSystem::Path::Path(const QString & path) :
    m_path(path), m_type(INVALID)
{
    if (!m_path.isEmpty()) {
        m_path.replace("\\", "/");
        if (m_path.back() == "/")
            m_path = m_path.left(m_path.size() - 1);
        QStringList splittedPath = m_path.split("/");
        if (QDir(splittedPath[0]).exists())
            m_type = SYSTEM;
        else {
            QStringList devices;
            PhotoSync::getWPDInstance().getDevices(devices);
            for (const QString &device : devices) {
                if (device == splittedPath[0]) {
                    m_type = MTP;
                    break;
                }
            }
        }
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
ExtendedFileSystem::Iterator::Iterator(const Path & path, const QStringList & extensions) :
    m_path(path), m_iteratorImpl(nullptr)
{
    if (m_path.m_type == Path::SYSTEM) {
        m_iteratorImpl = new QDirIterator(path.toQString(), extensions, QDir::Files, QDirIterator::Subdirectories);
    }
    else if (m_path.m_type == Path::MTP) {
        m_iteratorImpl = new MTPFS::DirIterator(path.toQString(), extensions);
    }
}

ExtendedFileSystem::Iterator::~Iterator()
{
    if (m_iteratorImpl)
        delete m_iteratorImpl;
    m_iteratorImpl = nullptr;

}

bool ExtendedFileSystem::Iterator::hasNext() const
{
    if (m_iteratorImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            return static_cast<QDirIterator *>(m_iteratorImpl)->hasNext();
        }
        else if (m_path.m_type == Path::MTP) {
            return static_cast<MTPFS::DirIterator *>(m_iteratorImpl)->hasNext();
        }
    }
    return false;
}

ExtendedFileSystem::Path ExtendedFileSystem::Iterator::next()
{
    if (m_iteratorImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            return Path(static_cast<QDirIterator *>(m_iteratorImpl)->next());
        }
        else if (m_path.m_type == Path::MTP) {
            return Path(static_cast<MTPFS::DirIterator *>(m_iteratorImpl)->next());
        }
    }
    return Path(QString());
}


/* ExtendedFileSystem::Info */
ExtendedFileSystem::Info::Info(const Path & path) :
    m_path(path), m_infoImpl(nullptr)
{
    if (m_path.m_type == Path::SYSTEM) {
        m_infoImpl = new QFileInfo(path.toQString());
    }
    else if (m_path.m_type == Path::MTP) {
        //TODO MTP
    }
}

ExtendedFileSystem::Info::~Info()
{
    if (m_infoImpl)
        delete m_infoImpl;
    m_infoImpl = nullptr;
}

ExtendedFileSystem::Path ExtendedFileSystem::Info::path() const
{
    if (m_infoImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            return Path(static_cast<QFileInfo *>(m_infoImpl)->absoluteFilePath());
        }
        else if (m_path.m_type == Path::MTP) {
            //TODO MTP
        }
    }
    return Path(QString());
}

QString ExtendedFileSystem::Info::fileName() const
{
    if (m_infoImpl) {
        if (m_infoImpl) {
            if (m_path.m_type == Path::SYSTEM) {
                return static_cast<QFileInfo *>(m_infoImpl)->fileName();
            }
            else if (m_path.m_type == Path::MTP) {
                //TODO MTP
            }
        }
    }
    return QString();
}

void ExtendedFileSystem::Info::setFile(QString name)
{
    if (m_infoImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            static_cast<QFileInfo *>(m_infoImpl)->setFile(name);
        }
        else if (m_path.m_type == Path::MTP) {
            //TODO MTP
        }
    }
}

QString ExtendedFileSystem::Info::suffix() const
{
    if (m_infoImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            return static_cast<QFileInfo *>(m_infoImpl)->suffix();
        }
        else if (m_path.m_type == Path::MTP) {
            //TODO MTP
        }
    }
    return QString();
}

int ExtendedFileSystem::Info::size() const
{
    if (m_infoImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            return static_cast<QFileInfo *>(m_infoImpl)->size();
        }
        else if (m_path.m_type == Path::MTP) {
            //TODO MTP
        }
    }
    return 0;
}

bool ExtendedFileSystem::Info::exists() const
{
    if (m_infoImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            return static_cast<QFileInfo *>(m_infoImpl)->exists();
        }
        else if (m_path.m_type == Path::MTP) {
            //TODO MTP
        }
    }
    return false;
}


/* ExtendedFileSystem::File */
ExtendedFileSystem::File::File(const Path & path) :
    m_path(path), m_fileImpl(nullptr)
{
    if (m_path.m_type == Path::SYSTEM) {
        m_fileImpl = new QFile(path.toQString());
    }
    else if (m_path.m_type == Path::MTP) {
        //TODO MTP
    }
}

ExtendedFileSystem::File::~File()
{
    if (m_fileImpl)
        delete m_fileImpl;
    m_fileImpl = nullptr;
}

bool ExtendedFileSystem::File::open()
{
    if (m_fileImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            return static_cast<QFile *>(m_fileImpl)->open(QIODevice::ReadOnly);
        }
        else if (m_path.m_type == Path::MTP) {
            //TODO MTP
        }
    }
    return false;
}

QByteArray ExtendedFileSystem::File::readAll()
{
    if (m_fileImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            return static_cast<QFile *>(m_fileImpl)->readAll();
        }
        else if (m_path.m_type == Path::MTP) {
            //TODO MTP
        }
    }
    return QByteArray();
}

void ExtendedFileSystem::File::close()
{
    if (m_fileImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            static_cast<QFile *>(m_fileImpl)->close();
        }
        else if (m_path.m_type == Path::MTP) {
            //TODO MTP
        }
    }
}

bool ExtendedFileSystem::File::copy(const Path & sourcePath, const Path & destPath)
{
    //TODO MTP
    return QFile::copy(sourcePath.toQString(), destPath.toQString());
}


/* ExtendedFileSystem::Dir */
ExtendedFileSystem::Dir::Dir(const Path & path) :
    m_path(path), m_dirImpl(nullptr)
{
    if (m_path.m_type == Path::SYSTEM) {
        m_dirImpl = new QDir(path.toQString());
    }
    else if (m_path.m_type == Path::MTP) {
        //TODO MTP
    }
}

ExtendedFileSystem::Dir::~Dir()
{
    if (m_dirImpl)
        delete m_dirImpl;
    m_dirImpl = nullptr;
}

ExtendedFileSystem::Path ExtendedFileSystem::Dir::path(QString concatenate) const
{
    if (m_dirImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            return static_cast<QDir *>(m_dirImpl)->absoluteFilePath(concatenate);
        }
        else if (m_path.m_type == Path::MTP) {
            //TODO MTP
        }
    }
    return Path();
}

bool ExtendedFileSystem::Dir::exists() const
{
    if (m_dirImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            return static_cast<QDir *>(m_dirImpl)->exists();
        }
        else if (m_path.m_type == Path::MTP) {
            //TODO MTP
        }
    }
    return false;
}

bool ExtendedFileSystem::Dir::mkpath(const QString & dirPath) const
{
    if (m_dirImpl) {
        if (m_path.m_type == Path::SYSTEM) {
            return static_cast<QDir *>(m_dirImpl)->mkpath(dirPath);
        }
        else if (m_path.m_type == Path::MTP) {
            //TODO MTP
        }
    }
    return false;
}
