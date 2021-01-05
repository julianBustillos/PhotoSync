#pragma once
#include <QString>
#include <QStringList>


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
}

namespace MTPFS = MTPFileSystem;