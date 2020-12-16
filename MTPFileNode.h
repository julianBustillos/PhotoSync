#pragma once
#include "MTPFileNodePathKey.h"
#include <QIcon>
#include <QHash>


class MTPFileNode
{
public:
    Q_DISABLE_COPY_MOVE(MTPFileNode)

    explicit MTPFileNode(const QString &filename = QString(), MTPFileNode *parent = nullptr);
    ~MTPFileNode();

public:
    bool m_isDir;
    QString m_fileName;
    qint64 m_size;
    QString m_type;
    QString m_lastModified;
    QIcon m_icon;
    QHash<MTPFileNodePathKey, MTPFileNode *> m_children;
    QList<QString> m_visibleChildren;
    MTPFileNode *m_parent;
    //int dirtyChildrenIndex = -1;
    //bool populatedChildren = false;
    //bool isVisible = false;

public:
    inline int visibleLocation(const QString &childName);
};


inline int MTPFileNode::visibleLocation(const QString &childName) {
    return m_visibleChildren.indexOf(childName);
}