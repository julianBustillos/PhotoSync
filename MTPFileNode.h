#pragma once
#include "MTPFileNodePathKey.h"
#include <QIcon>
#include <QHash>


class MTPFileNode
{
public:
    enum Type {
        DEVICE,
        DRIVE,
        FOLDER,
        FILE,
        UNKNOWN
    };

public:
    Q_DISABLE_COPY_MOVE(MTPFileNode)

    MTPFileNode(const QString &name = "root", Type type = UNKNOWN, qint64 size = 0, QString lastModified = "", const QIcon &icon = QIcon(), MTPFileNode *parent = nullptr);
    ~MTPFileNode();

public:
    inline int visibleLocation(const QString &childName) const;
    inline const QString &getName();
    inline const QString getType();
    inline const qint64 &getSize();
    inline const QString &getLastModified();
    inline const QIcon &getQIcon();
    inline MTPFileNode *getParent();
    inline QHash<MTPFileNodePathKey, MTPFileNode *> &getChildren();
    inline QList<QString> &getVisibleChildren();
    inline bool isDir();
    inline bool isFetching();
    inline bool isPopulated();
    inline void setFetching();
    inline void setPopulated();
    QString getPath() const;

private:
    enum Step {
        TODO = 0,
        FETCHING = 1,
        POPULATED = 2
    };

private:
    static QString TypeToString(Type type);

private:
    const QString m_name;
    const Type m_type;
    const qint64 m_size;
    const QString m_lastModified;
    const QIcon m_icon;
    MTPFileNode *m_parent;
    QHash<MTPFileNodePathKey, MTPFileNode *> m_children;
    QList<QString> m_visibleChildren;
    Step m_step;
};

inline int MTPFileNode::visibleLocation(const QString &childName) const 
{
    return m_visibleChildren.indexOf(childName);
}

inline const QString & MTPFileNode::getName()
{
    return m_name;
}

inline const QString MTPFileNode::getType()
{
    return TypeToString(m_type);
}

inline const qint64 & MTPFileNode::getSize()
{
    return m_size;
}

inline const QString & MTPFileNode::getLastModified()
{
    return m_lastModified;
}

inline const QIcon & MTPFileNode::getQIcon()
{
    return m_icon;
}

inline MTPFileNode * MTPFileNode::getParent()
{
    return m_parent;
}

inline QHash<MTPFileNodePathKey, MTPFileNode*>& MTPFileNode::getChildren()
{
    return m_children;
}

inline QList<QString>& MTPFileNode::getVisibleChildren()
{
    return m_visibleChildren;
}
inline bool MTPFileNode::isDir()
{
    return m_type == DEVICE || m_type == DRIVE || m_type== FOLDER;
}
inline bool MTPFileNode::isFetching()
{
    return m_step >= FETCHING;
}

inline bool MTPFileNode::isPopulated()
{
    return m_step >= POPULATED;
}

inline void MTPFileNode::setFetching()
{
    if (m_step < FETCHING)
        m_step = FETCHING;
}

inline void MTPFileNode::setPopulated()
{
    if (m_step < POPULATED)
        m_step = POPULATED;
}
