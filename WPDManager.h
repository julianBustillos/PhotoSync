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
        Item(QString name, ItemType type, QString date, int size);

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
    bool getContent(QString path, QVector<Item>& content);

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
        DeviceData(PCWSTR deviceID, Microsoft::WRL::ComPtr<IPortableDevice> device, Microsoft::WRL::ComPtr<IPortableDeviceContent> content, Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties, PCWSTR objectID);
        ~DeviceData();

        PWSTR m_deviceID = nullptr;
        Microsoft::WRL::ComPtr<IPortableDevice> m_device;
        Microsoft::WRL::ComPtr<IPortableDeviceContent> m_content;
        Microsoft::WRL::ComPtr<IPortableDeviceProperties> m_properties;
        DeviceNode m_rootNode;
    };

private:
    void createClientInformation();
    void createPropertiesToRead();
    void fetchDevices();
    bool populate(DeviceNode &node, Microsoft::WRL::ComPtr<IPortableDeviceContent> content, Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties);
    bool fetchData(PWSTR & name, DeviceNode &node, Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties);

private:
    HRESULT m_hr_COM;
    HRESULT m_hr_init;
    Microsoft::WRL::ComPtr<IPortableDeviceManager> m_deviceManager;
    Microsoft::WRL::ComPtr<IPortableDeviceValues> m_clientInformation;
    Microsoft::WRL::ComPtr<IPortableDeviceKeyCollection>  m_propertiesToRead;
    std::map<QString, DeviceData> m_deviceMap;
};