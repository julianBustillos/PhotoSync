#pragma once
#include <QString>
#include <QStringList>
#include <QIODevice>
#include <objidl.h>


namespace MTPFileSystem /*ALIAS: MTPFS*/
{
    class DirIterator
    {
    public:
        DirIterator(const QString &rootPath, const QStringList &extensions);

    public:
        bool hasNext() const;
        QString next();

    private:
        void findNext(const QString &current);

    private:
        const int m_rootLevel;
        const QStringList m_extensions;
        QString m_next;
    };


    class FileInfo
    {
    public:
        FileInfo(const QString &path);
        ~FileInfo();

    public:
        QString path() const;
        QString fileName() const;
        QString suffix() const;
        void setFile(const QString &name);
        int size() const;
        bool exists() const;

    private:
        QString m_path;
    };


    class File
    {
    public:
        File(const QString &path);
        ~File();

    public:
        bool open(QIODevice::OpenMode mode);
        qint64 write(const QByteArray &fileData);
        QByteArray readAll();
        void close();
        bool remove();

    public:
        static int remove(const QStringList &pathList, QVector<bool> &results);

    private:
        QIODevice::OpenMode m_mode;
        const QString m_path;
        int m_size;
    };


    class Dir
    {
    public:
        Dir(const QString &path);
        ~Dir();

    public:
        QString path(const QString &concatenate) const;
        bool exists() const;
        bool mkpath(const QString & dirPath) const;

    private:
        const QString m_path;
    };
}

namespace MTPFS = MTPFileSystem;