#include "MTPFileNode.h"


MTPFileNode::MTPFileNode(QString &name, Type type, qint64 size, QString lastModified, QIcon &icon, MTPFileNode *parent) :
    m_name(name), m_type(type), m_size(size), m_lastModified(lastModified), m_icon(icon), m_parent(parent)
{
    m_populatedChildren = (type == FILE);
}

MTPFileNode::~MTPFileNode()
{
    qDeleteAll(m_children);
}

QString MTPFileNode::getPath() const
{
    if (m_parent && m_parent->m_parent)
        return m_parent->getPath() + "/" + m_name;
    return m_name;
}

QString MTPFileNode::TypeToString(Type type)
{
    switch (type) {
    case DEVICE:
        return "Device";
    case DRIVE:
        return "Drive";
    case FOLDER:
        return "Folder";
    case FILE:
        return "File";
    default:
        return "ERROR_TYPE";
    }
}
