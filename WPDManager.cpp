#include "WPDManager.h"
#include <PortableDevice.h> 

#define NUM_OBJECTS_TO_REQUEST 10


WPDManager::WPDManager() :
    m_hr_COM(E_FAIL), m_hr_init(E_FAIL)
{
    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    m_hr_COM = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); //TODO : if thread init switch to COINIT_MULTITHREADED

    if (SUCCEEDED(m_hr_COM))
        m_hr_init = CoCreateInstance(CLSID_PortableDeviceManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_deviceManager));

    createClientInformation();
    createPropertiesToRead();
    fetchDevices();
}

WPDManager::~WPDManager()
{
    if (SUCCEEDED(m_hr_COM))
        CoUninitialize();
}

WPDManager & WPDManager::getInstance()
{
    static WPDManager instance;
    return instance;
}

bool WPDManager::getDevices(QStringList& devices)
{
    if (SUCCEEDED(m_hr_init)) {
        devices.reserve(m_deviceMap.size());

        for (auto &device : m_deviceMap)
            devices.append(device.first);
    }

    return SUCCEEDED(m_hr_init);
}

bool WPDManager::getContent(QString path, QVector<Item>& content)
{
    bool foundPath = false;
    QStringList splittedPath = path.split("/");
    if (!splittedPath.empty()) {
        int index = 0;
        auto deviceIter = m_deviceMap.find(splittedPath[index++]);
        if (deviceIter != m_deviceMap.end()) {
            DeviceNode *node = &deviceIter->second.m_rootNode;
            while (node && index < splittedPath.size()) {
                populate(*node, deviceIter->second.m_content, deviceIter->second.m_properties);
                auto nodeIter = node->m_children.find(splittedPath[index++]);
                node = nodeIter != node->m_children.end() ? nodeIter->second : nullptr;
            }

            if (node) {
                foundPath = true;
                populate(*node, deviceIter->second.m_content, deviceIter->second.m_properties);
                content.reserve(node->m_children.size());
                for (auto &child : node->m_children)
                    content.append(Item(child.first, child.second->m_type, QString::fromWCharArray(child.second->m_date), child.second->m_size));
            }
        }
    }

    return foundPath;
}

void WPDManager::createClientInformation()
{
    if (SUCCEEDED(m_hr_init))
        m_hr_init = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_clientInformation));
    
    if (SUCCEEDED(m_hr_init))
        m_hr_init = m_clientInformation->SetStringValue(WPD_CLIENT_NAME, CLIENT_NAME);

    if (SUCCEEDED(m_hr_init))
        m_hr_init = m_clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, CLIENT_MAJOR_VER);
        
    if (SUCCEEDED(m_hr_init))
        m_hr_init = m_clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, CLIENT_MINOR_VER);
        
    if (SUCCEEDED(m_hr_init))
        m_hr_init = m_clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, CLIENT_REVISION);
        
    if (SUCCEEDED(m_hr_init))
        m_hr_init = m_clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION);
}

void WPDManager::createPropertiesToRead()
{
    if (SUCCEEDED(m_hr_init))
        m_hr_init = CoCreateInstance(CLSID_PortableDeviceKeyCollection, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_propertiesToRead));

    if (SUCCEEDED(m_hr_init))
        m_hr_init = m_propertiesToRead->Add(WPD_OBJECT_CONTENT_TYPE);

    if (SUCCEEDED(m_hr_init))
        m_hr_init = m_propertiesToRead->Add(WPD_FUNCTIONAL_OBJECT_CATEGORY);

    if (SUCCEEDED(m_hr_init))
        m_hr_init = m_propertiesToRead->Add(WPD_OBJECT_NAME);

    if (SUCCEEDED(m_hr_init))
        m_hr_init = m_propertiesToRead->Add(WPD_OBJECT_DATE_MODIFIED);

    if (SUCCEEDED(m_hr_init))
        m_hr_init = m_propertiesToRead->Add(WPD_OBJECT_SIZE);
}

void WPDManager::fetchDevices()
{
    if (SUCCEEDED(m_hr_init)) {
        DWORD pnpDeviceIDCount = 0;
        HRESULT hr = m_deviceManager->GetDevices(nullptr, &pnpDeviceIDCount);

        if (SUCCEEDED(hr) && (pnpDeviceIDCount > 0)) {
            PWSTR* pnpDeviceIDs = new (std::nothrow) PWSTR[pnpDeviceIDCount];
            if (pnpDeviceIDs != nullptr) {
                ZeroMemory(pnpDeviceIDs, pnpDeviceIDCount * sizeof(PWSTR));
                DWORD retrievedDeviceIDCount = pnpDeviceIDCount;
                hr = m_deviceManager->GetDevices(pnpDeviceIDs, &retrievedDeviceIDCount);
                if (retrievedDeviceIDCount <= pnpDeviceIDCount) {
                    for (DWORD index = 0; index < retrievedDeviceIDCount && SUCCEEDED(hr); index++) {
                        QString friendlyName = "ERROR_NAME";
                        Microsoft::WRL::ComPtr<IPortableDevice> device = nullptr;
                        Microsoft::WRL::ComPtr<IPortableDeviceContent> content = nullptr;
                        Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties = nullptr;

                        DWORD friendlyNameLength = 0;
                        hr = m_deviceManager->GetDeviceFriendlyName(pnpDeviceIDs[index], nullptr, &friendlyNameLength);
                        if (SUCCEEDED(hr) && friendlyNameLength > 0) {
                            PWSTR friendlyNameWSTR = new (std::nothrow) WCHAR[friendlyNameLength];
                            if (friendlyNameWSTR != nullptr) {
                                ZeroMemory(friendlyNameWSTR, friendlyNameLength * sizeof(WCHAR));
                                hr = m_deviceManager->GetDeviceFriendlyName(pnpDeviceIDs[index], friendlyNameWSTR, &friendlyNameLength);
                                if (SUCCEEDED(hr))
                                    friendlyName = QString::fromWCharArray(friendlyNameWSTR);

                                delete[] friendlyNameWSTR;
                                friendlyNameWSTR = nullptr;
                            }
                        }

                        hr = CoCreateInstance(CLSID_PortableDeviceFTM, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&device));
                        if (SUCCEEDED(hr))
                            hr = device->Open(pnpDeviceIDs[index], m_clientInformation.Get());

                        if (SUCCEEDED(hr))
                            hr = device->Content(&content);

                        if (SUCCEEDED(hr))
                            hr = content->Properties(&properties);

                        if (SUCCEEDED(hr))
                            m_deviceMap.try_emplace(friendlyName, pnpDeviceIDs[index], device, content, properties, WPD_DEVICE_OBJECT_ID);
                    }
                }

                for (DWORD index = 0; index < pnpDeviceIDCount; index++) {
                    CoTaskMemFree(pnpDeviceIDs[index]);
                    pnpDeviceIDs[index] = nullptr;
                }

                delete[] pnpDeviceIDs;
                pnpDeviceIDs = nullptr;
            }
        }

        m_hr_init = hr;
    }
}

bool WPDManager::populate(DeviceNode & node, Microsoft::WRL::ComPtr<IPortableDeviceContent> content, Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties)
{
    if (node.m_populatedChildren)
        return true;

    Microsoft::WRL::ComPtr<IEnumPortableDeviceObjectIDs> enumObjectIDs;
    HRESULT hr = content->EnumObjects(0, node.m_objectID, nullptr, &enumObjectIDs);

    while (hr == S_OK)
    {
        DWORD numFetched = 0;
        PWSTR objectIDArray[NUM_OBJECTS_TO_REQUEST] = { 0 };
        hr = enumObjectIDs->Next(NUM_OBJECTS_TO_REQUEST, objectIDArray, &numFetched);
        if (SUCCEEDED(hr))
        {
            for (DWORD index = 0; (index < numFetched) && (objectIDArray[index] != nullptr); index++)
            {
                DeviceNode *child = new DeviceNode(objectIDArray[index]);
                if (child) {
                    PWSTR name = nullptr;
                    if (fetchData(name, *child, properties))
                        node.m_children.emplace(QString::fromWCharArray(name), child);
                }
                CoTaskMemFree(objectIDArray[index]);
                objectIDArray[index] = nullptr;
            }
        }
    }

     node.m_populatedChildren = true;
     return SUCCEEDED(hr);
}

bool WPDManager::fetchData(PWSTR & name, DeviceNode & node, Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties)
{
    Microsoft::WRL::ComPtr<IPortableDeviceValues> objectProperties;
    HRESULT hr = properties->GetValues(node.m_objectID, m_propertiesToRead.Get(), &objectProperties);

    if (SUCCEEDED(hr)) {
        GUID typeGUID = GUID_NULL;
        hr = objectProperties->GetGuidValue(WPD_OBJECT_CONTENT_TYPE, &typeGUID);

        if (typeGUID == WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT) {
            GUID categoryGUID = GUID_NULL;
            if (SUCCEEDED(hr))
                hr = objectProperties->GetGuidValue(WPD_FUNCTIONAL_OBJECT_CATEGORY, &categoryGUID);
            if (categoryGUID != WPD_FUNCTIONAL_CATEGORY_STORAGE)
                return false;

            node.m_type = DRIVE;
        }
        else if (typeGUID == WPD_CONTENT_TYPE_FOLDER) {
            node.m_type = FOLDER;
        }
        else {
            node.m_type = FILE;
        }
    }

    if (SUCCEEDED(hr))
        hr = objectProperties->GetStringValue(WPD_OBJECT_NAME, &name);

    if (SUCCEEDED(hr) && node.m_type != DRIVE)
        hr = objectProperties->GetStringValue(WPD_OBJECT_DATE_MODIFIED, &node.m_date);

    if (SUCCEEDED(hr) && node.m_type == FILE)
        hr = objectProperties->GetUnsignedIntegerValue(WPD_OBJECT_SIZE, &node.m_size);

    return SUCCEEDED(hr);
}

WPDManager::DeviceNode::DeviceNode(PCWSTR objectID)
{
    size_t size = wcslen(objectID);
    m_objectID = new wchar_t[size + 1];
    wcscpy(m_objectID, objectID);
}

WPDManager::DeviceNode::~DeviceNode()
{
    if (m_objectID)
        delete[] m_objectID;
    m_objectID = nullptr;

    for (auto &child : m_children) {
        if (child.second)
            delete child.second;
        child.second = nullptr;
    }
}

WPDManager::DeviceData::DeviceData(PCWSTR deviceID, Microsoft::WRL::ComPtr<IPortableDevice> device, Microsoft::WRL::ComPtr<IPortableDeviceContent> content, Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties, PCWSTR objectID) :
    m_device(device), m_content(content), m_properties(properties), m_rootNode(objectID)
{
    size_t size = wcslen(deviceID);
    m_deviceID = new wchar_t[size + 1];
    wcscpy(m_deviceID, deviceID);
}

WPDManager::DeviceData::~DeviceData()
{
    if (m_deviceID)
        delete[] m_deviceID;
    m_deviceID = nullptr;
}

WPDManager::Item::Item(QString name, ItemType type, QString date, int size) :
    m_name(name), m_type(type), m_date(date), m_size(size)
{
}
