#pragma once
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QStack>
#include <QVector>
#include <QMetaType>
#include "WPDManager.h"
#include "MTPFileNode.h"
#include "NodeContainer.h"


class MTPFileFetcher : public QThread
{
    Q_OBJECT

public:
    MTPFileFetcher(QObject *parent = nullptr);
    ~MTPFileFetcher();

public:
    void updateDevices();
    void addToFetch(MTPFileNode *node);

signals:
    void loadedDevices(const QStringList &devices);
    void loadedContent(const NodeContainer &container);

private:
    void run() override;
    void fetchDevices();
    void fetch(MTPFileNode *node);

private:
    QAtomicInt m_stopped;

    mutable QMutex m_mutex;
    // begin protected by mutex
    QWaitCondition m_condition;
    bool m_fetchDevices;
    QStack<MTPFileNode *> m_toFetch;
    // end protected by mutex

};
