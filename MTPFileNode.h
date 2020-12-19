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

    explicit MTPFileNode(QString &name, Type type = UNKNOWN, qint64 size = 0, QString lastModified = "", QIcon &icon = QIcon(), MTPFileNode *parent = nullptr);
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
    inline bool isPopulated();
    inline void setPopulated();
    QString getPath() const;

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
    bool m_populatedChildren;
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

inline bool MTPFileNode::isPopulated()
{
    return m_populatedChildren;
}

inline void MTPFileNode::setPopulated()
{
    m_populatedChildren = true;
}
