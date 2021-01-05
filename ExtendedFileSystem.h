#pragma once
#include <QString>



namespace ExtendedFileSystem /*ALIAS: EFS*/
{
    class Path
    {
        friend class Iterator;
        friend class Info;
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


    class Iterator
    {
    public:
        Iterator(const Path &path, const QStringList &extensions);
        ~Iterator();

    public:
        bool hasNext() const;
        Path next();

    private:
        const Path &m_path;
        void *m_iteratorImpl;
    };


    class Info
    {
    public:
        Info(const Path &path);
        ~Info();

    public:
        Path path() const;
        QString fileName() const;
        void setFile(QString name);
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
        bool open();
        QByteArray readAll();
        void close();

    public:
        static bool copy(const Path& sourcePath, const Path &destPath);

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
        Path path(QString concatenate) const;
        bool exists() const;
        bool mkpath(const QString &dirPath) const;

    private:
        const Path &m_path;
        void *m_dirImpl;
    };
}


namespace EFS = ExtendedFileSystem;
