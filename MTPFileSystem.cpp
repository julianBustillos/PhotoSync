#include "MTPFileSystem.h"
#include "PhotoSync.h"


/* MTPFileSystem::MTPDirIterator */
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
    QStringList splittedPath = current.split("/");
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
