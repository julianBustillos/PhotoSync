#include "MTPFileNode.h"


MTPFileNode::MTPFileNode(const QString & filename, MTPFileNode * parent) :
    m_fileName(filename), m_parent(parent)
{
}

MTPFileNode::~MTPFileNode()
{
    qDeleteAll(m_children);
}


