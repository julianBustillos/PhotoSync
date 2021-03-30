#pragma once
#include <QString>
#include <QIODevice>


namespace ExtendedFileSystem /*ALIAS: EFS*/
{
    class Path
    {
        friend class DirIterator;
        friend class FileInfo;
        friend class File;
        friend class Dir;

    public:
        Path(const QString &path = QString());
        ~Path();

    public:
        const QString &toQString() const;
        bool isEmpty() const;

    public:
        bool operator<(const Path& rhs) const;

    private:
        enum Type {
            SYSTEM,
            MTP,
            INVALID
        };

    private:
        QString m_path;
        Type m_type;
    };


    class DirIterator
    {
    public:
        DirIterator(const Path &path, const QStringList &extensions);
        ~DirIterator();

    public:
        bool hasNext() const;
        Path next();

    private:
        const Path &m_path;
        void *m_iteratorImpl;
    };


    class FileInfo
    {
    public:
        FileInfo(const Path &path);
        ~FileInfo();

    public:
        Path path() const;
        QString fileName() const;
        void setFile(const QString &name);
        QString suffix() const;
        int size() const;
        bool exists() const;

    private:
        const Path &m_path;
        void *m_infoImpl;
    };


    class File
    {
    public:
        File(const Path& path);
        ~File();

    public:
        bool open(QIODevice::OpenMode mode);
        qint64 write(const QByteArray &byteArray);
        QByteArray readAll();
        void close();
        bool remove();

    public:
        static int remove(const QVector<Path> &paths, QVector<bool>& results);

    private:
        const Path &m_path;
        void *m_fileImpl;
    };


    class Dir
    {
    public:
        Dir(const Path &path);
        ~Dir();

    public:
        Path path(const QString &concatenate) const;
        bool exists() const;
        bool mkpath(const QString &dirPath) const;

    private:
        const Path &m_path;
        void *m_dirImpl;
    };
}


namespace EFS = ExtendedFileSystem;
