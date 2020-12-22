#pragma once
#include "MTPFileNode.h"
#include "WPDManager.h"


struct NodeContainer
{
    NodeContainer(MTPFileNode *node = nullptr, QVector<WPDManager::Item>* content = nullptr) : m_node(node), m_content(content) {};
    NodeContainer(const NodeContainer &container) : m_node(container.m_node), m_content(container.m_content) {};
    ~NodeContainer() {};

    MTPFileNode *m_node = nullptr;
    QVector<WPDManager::Item> *m_content = nullptr;
};


Q_DECLARE_METATYPE(NodeContainer);