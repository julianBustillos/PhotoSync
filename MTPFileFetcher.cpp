#include "MTPFileFetcher.h"
#include "MTPFileModel.h"
#include "WPDInstance.h"


MTPFileFetcher::MTPFileFetcher(QObject *parent) :
    QThread(parent), m_stopped(false), m_fetchDevices(true)
{
}

MTPFileFetcher::~MTPFileFetcher()
{
    m_stopped.storeRelaxed(true);
    QMutexLocker locker(&m_mutex);
    m_condition.wakeAll();
    locker.unlock();
    wait();
}

void MTPFileFetcher::fetchDevices()
{
    QStringList devices;
    if (WPDInstance::get().getDevices(devices))
        emit loadedDevices(devices);
}

void MTPFileFetcher::updateDevices()
{
    QMutexLocker locker(&m_mutex);
    m_fetchDevices = true;
    m_condition.wakeAll();
}

void MTPFileFetcher::addToFetch(MTPFileNode *node)
{
    QMutexLocker locker(&m_mutex);
    if (!node || node->isFetching())
        return;
    node->setFetching();
    m_toFetch.push(node);
    m_condition.wakeAll();
}

void MTPFileFetcher::run() 
{
    forever{
        QMutexLocker locker(&m_mutex);
        while (!m_stopped.loadRelaxed() && !m_fetchDevices && m_toFetch.isEmpty())
            m_condition.wait(&m_mutex);
        if (m_stopped.loadRelaxed())
            return;

        if (m_fetchDevices) {
            m_fetchDevices = false;
            fetchDevices();
        }
        else if (!m_toFetch.isEmpty()) {
            MTPFileNode *nodeToFetch = m_toFetch.front();
            m_toFetch.pop_front();
            locker.unlock();
            fetch(nodeToFetch);
        }
    }
}

void MTPFileFetcher::fetch(MTPFileNode *node)
{
    QVector<WPDManager::Item> *content = new QVector<WPDManager::Item>();
    if (content && WPDInstance::get().getContent(node->getPath(), *content))
        emit loadedContent(NodeContainer(node, content));
}
