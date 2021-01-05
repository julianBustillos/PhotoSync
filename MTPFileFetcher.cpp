#include "MTPFileFetcher.h"
#include "MTPFileModel.h"
#include "PhotoSync.h"
#include "WPDManager.h"


MTPFileFetcher::MTPFileFetcher(QObject *parent) :
    QThread(parent), m_stopped(false)
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
    if (PhotoSync::getWPDInstance().getDevices(devices))
        emit loadedDevices(devices);
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
    fetchDevices();

    forever{
        QMutexLocker locker(&m_mutex);
        while (!m_stopped.loadRelaxed() && m_toFetch.isEmpty())
            m_condition.wait(&m_mutex);
        if (m_stopped.loadRelaxed())
            return;

        MTPFileNode *nodeToFetch = m_toFetch.front();
        m_toFetch.pop_front();
        locker.unlock();

        fetch(nodeToFetch);
    }
}

void MTPFileFetcher::fetch(MTPFileNode *node)
{
    QVector<WPDManager::Item> *content = new QVector<WPDManager::Item>();
    if (content && PhotoSync::getWPDInstance().getContent(node->getPath(), *content))
        emit loadedContent(NodeContainer(node, content));
}
