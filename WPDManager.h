#pragma once
#include <windows.h>
#include <wrl/client.h>
#include <PortableDeviceApi.h> 
#include <QStringList>
#include <QVector>
#include <map>

#define CLIENT_NAME         L"WPD Manager"
#define CLIENT_MAJOR_VER    1
#define CLIENT_MINOR_VER    0
#define CLIENT_REVISION     0


class WPDManager
{
public:
    enum ItemType {
        DRIVE,
        FOLDER,
        FILE,
        UNKNOWN
    };

    struct Item {
        Item(QString name = QString(), ItemType type = UNKNOWN, QString date = QString(), int size = 0);

        QString m_name;
        ItemType m_type;
        QString m_date;
        int m_size;
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


private:
    struct DeviceNode
    {
        DeviceNode(PCWSTR objectID);
        ~DeviceNode();

        PWSTR m_objectID = nullptr;
        ItemType m_type = UNKNOWN;
        PWSTR m_date = nullptr;
        ULONG m_size = 0;
        std::map<QString, DeviceNode *> m_children;
        bool m_populatedChildren = false;
    };

    struct DeviceData
    {
        DeviceData(PCWSTR deviceID, Microsoft::WRL::ComPtr<IPortableDevice> device, Microsoft::WRL::ComPtr<IPortableDeviceContent> content, 
                   Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties, Microsoft::WRL::ComPtr<IPortableDeviceResources> resources);
        ~DeviceData();

        PWSTR m_deviceID = nullptr;
        Microsoft::WRL::ComPtr<IPortableDevice> m_device;
        Microsoft::WRL::ComPtr<IPortableDeviceContent> m_content;
        Microsoft::WRL::ComPtr<IPortableDeviceProperties> m_properties;
        Microsoft::WRL::ComPtr<IPortableDeviceResources> m_resources;
        DeviceNode m_rootNode;
    };

private:
    void createClientInformation();
    void createPropertiesToRead();
    HRESULT createBasicObjectProperties(Microsoft::WRL::ComPtr<IPortableDeviceValues> &objectProperties, PWSTR &parentID, const QString &name, const GUID &type);
    void fetchDevices();
    bool populate(DeviceNode &node, Microsoft::WRL::ComPtr<IPortableDeviceContent> content, Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties);
    bool fetchData(PWSTR & name, DeviceNode &node, Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties);
    DeviceData *findDevice(const QString &deviceName);
    DeviceNode *findNode(const QString &path);

private:
    HRESULT m_hr_COM;
    HRESULT m_hr_init;
    Microsoft::WRL::ComPtr<IPortableDeviceManager> m_deviceManager;
    Microsoft::WRL::ComPtr<IPortableDeviceValues> m_clientInformation;
    Microsoft::WRL::ComPtr<IPortableDeviceKeyCollection>  m_propertiesToRead;
    std::map<QString, DeviceData> m_deviceMap;
};