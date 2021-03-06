#pragma once
#include <windows.h>
#include <wrl/client.h>
#include <PortableDeviceApi.h> 
#include <QStringList>
#include <QVector>
#include <QMutex>
#include <QWaitCondition>
#include <unordered_map>
#include <set>

#define CLIENT_NAME         L"WPD Manager"
#define CLIENT_MAJOR_VER    1
#define CLIENT_MINOR_VER    0
#define CLIENT_REVISION     0

class WPDUSBDetector;
class WPDDeviceEventsCallback;

class WPDManager
{
    friend WPDUSBDetector;
    friend WPDDeviceEventsCallback;

public:
    enum class ItemType {
        DRIVE,
        FOLDER,
        FILE,
        UNKNOWN
    };

    struct Item {
        Item(QString name = QString(), ItemType type = ItemType::UNKNOWN, QString date = QString(), int size = 0);

        QString m_name;
        ItemType m_type;
        QString m_date;
        int m_size;
    };

    class Observer {
    public:
        Observer() {};
        virtual ~Observer() = 0 {};

    public:
        virtual void addDevice(const QString &device) = 0;
        virtual void removeDevice(const QString &device) = 0;
        virtual void addItem(const QString &path) = 0;
        virtual void updateItem(const QString &path) = 0;
        virtual void removeItem(const QString &path) = 0;
    };

public:
    WPDManager();
    ~WPDManager();

public:
    bool getDevices(QStringList& devices);
    bool getItem(const QString &path, Item &item);
    bool getContent(const QString &path, QVector<Item>& content);
    bool readData(const QString &path, char *data);
    bool createFolder(const QString &path, const QString &folderName);
    bool createFile(const QString &path, const QString &fileName, const char *data, int size);
    int deleteObjects(const QStringList& paths, QVector<bool>& results);
    bool registerForEvents(Observer *observer);
    bool unregisterForEvents(Observer *observer);

private:
    struct DeviceNode
    {
        DeviceNode(DeviceNode *parent, const QString &objectID);
        ~DeviceNode();

        DeviceNode *m_parent;
        QString m_name;
        QString m_objectID;
        ItemType m_type = ItemType::UNKNOWN;
        QString m_date;
        ULONG m_size = 0;
        std::unordered_map<QString, DeviceNode *> m_children;
        bool m_populatedChildren = false;
    };

    struct DeviceData
    {
        DeviceData(const QString &deviceName, Microsoft::WRL::ComPtr<IPortableDevice> device, Microsoft::WRL::ComPtr<IPortableDeviceContent> content,
        Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties, Microsoft::WRL::ComPtr<IPortableDeviceResources> resources);
        ~DeviceData();

        Microsoft::WRL::ComPtr<IPortableDevice> m_device;
        Microsoft::WRL::ComPtr<IPortableDeviceContent> m_content;
        Microsoft::WRL::ComPtr<IPortableDeviceProperties> m_properties;
        Microsoft::WRL::ComPtr<IPortableDeviceResources> m_resources;
        std::unordered_map<QString, DeviceNode *> m_objectIDMap;
        DeviceNode *m_rootNode;
        QString m_eventCookie;
    };

private:
    void createClientInformation();
    void createPropertiesToRead();
    void createObjectsToDelete();
    HRESULT createBasicObjectProperties(Microsoft::WRL::ComPtr<IPortableDeviceValues> &objectProperties, const QString &parentID, const QString &name, const GUID &type);
    HRESULT createDeviceData(const QString &deviceID);
    HRESULT fetchDevices();
    HRESULT registerForUSBEvents();
    HRESULT unregisterForUSBEvents();
    HRESULT registerForDeviceEvents(DeviceData &device);
    HRESULT unregisterForDeviceEvents(DeviceData &device);
    DeviceNode *createNode(DeviceData &device, DeviceNode &parent, const QString &objectID);
    bool populate(DeviceData &device, DeviceNode &node);
    bool fetchData(const DeviceData &device, DeviceNode &node);
    DeviceData *findDevice(const QString &path);
    DeviceNode *findNode(const QString &path);
    QString findPath(const DeviceNode &node);

private:
    void refreshDevices();
    void removeDevice(const QString &deviceID);
    void addObject(const QString &deviceID, const QString &parentID, const QString& objectID);
    void updateObject(const QString &deviceID, const QString& objectID);
    void removeObject(const QString &deviceID, const QString& objectID);

private:
    void startReading();
    void endReading();
    void startWriting();
    void endWriting();
    bool waitObjects();


private:
    mutable QMutex m_mutex;
    // begin protected by mutex
    QAtomicInt m_readCount;
    QWaitCondition m_readFinished;
    QWaitCondition m_writeFinished;
    int m_objectsToWait;
    // end protected by mutex

private:
    HRESULT m_hr_COM;
    HRESULT m_hr_init;
    Microsoft::WRL::ComPtr<IPortableDeviceManager> m_deviceManager;
    Microsoft::WRL::ComPtr<IPortableDeviceValues> m_clientInformation;
    Microsoft::WRL::ComPtr<IPortableDeviceKeyCollection> m_propertiesToRead;
    Microsoft::WRL::ComPtr<IPortableDevicePropVariantCollection> m_objectsToDelete;
    std::unordered_map<QString, QString> m_deviceIDMap;
    std::unordered_map<QString, DeviceData *> m_deviceMap;
    std::vector<Microsoft::WRL::ComPtr<IPortableDevice>> m_removedDevices;
    WPDUSBDetector *m_USBDetector;
    std::set<Observer *> m_observers;
};