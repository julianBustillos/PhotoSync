#include "MTPFileFetcher.h"
#include "MTPFileModel.h"


MTPFileFetcher::MTPFileFetcher(QObject *parent) :
    QThread(parent), m_paused(false), m_stopped(false)
{
    m_WPDManager = new WPDManager(); //TODO: CHANGE
}

MTPFileFetcher::~MTPFileFetcher()
{
    m_stopped.storeRelaxed(true);
    QMutexLocker locker(&m_mutex);
    m_condition.wakeAll();
    locker.unlock();
    wait();

    if (m_WPDManager) //TODO: CHANGE
        delete m_WPDManager;
    m_WPDManager = nullptr;
}

void MTPFileFetcher::fetchDevices()
{
    QStringList devices;
    if (m_WPDManager && m_WPDManager->getDevices(devices))
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
        while (!m_stopped.loadRelaxed() && (m_paused.loadRelaxed() || m_toFetch.isEmpty()))
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
    if (content && m_WPDManager && m_WPDManager->getContent(node->getPath(), *content))
        emit loadedContent(NodeContainer(node, content));
}
