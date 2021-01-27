#include "WPDManager.h"
#include "WPDEvents.h"
#include <PortableDevice.h> 
#include <ObjIdl.h>
#include <Shlwapi.h>

#define NUM_OBJECTS_TO_REQUEST 10 //TODO: CHANGE ?


WPDManager::WPDManager() :
    m_hr_COM(E_FAIL), m_hr_init(E_FAIL)
{
    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    m_hr_COM = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    m_hr_init = CoCreateInstance(CLSID_PortableDeviceManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_deviceManager));

    createClientInformation();
    createPropertiesToRead();
    fetchDevices();
}

WPDManager::~WPDManager()
{
    m_removedDevices.clear();

    m_deviceIDMap.clear();
    for (auto &deviceIter : m_deviceMap) {
        if (deviceIter.second) {
            unregisterForDeviceEvents(*deviceIter.second);
            delete deviceIter.second;
        }
        deviceIter.second = nullptr;
    }
    m_deviceMap.clear();

    m_propertiesToRead.Reset();
    m_clientInformation.Reset();
    m_deviceManager.Reset();

    if (SUCCEEDED(m_hr_COM))
        CoUninitialize();
}

bool WPDManager::getDevices(QStringList& devices)
{
    if (SUCCEEDED(m_hr_init)) {
        devices.reserve(m_deviceMap.size());

        for (auto &deviceIter : m_deviceMap)
            devices.append(deviceIter.first);
    }

    return SUCCEEDED(m_hr_init);
}

bool WPDManager::getItem(const QString &path, Item &item)
{
    DeviceNode *node = findNode(path);
    if (node) {
        item.m_name = path.right(path.size() - path.lastIndexOf('/') - 1);
        item.m_type = node->m_type;
        item.m_date = node->m_date;
        item.m_size = node->m_size;
    }

    return !!node;
}

bool WPDManager::getContent(const QString &path, QVector<Item> &content)
{
    DeviceNode *node = findNode(path);
    if (node) {
        content.reserve(node->m_children.size());
        for (auto &child : node->m_children)
            content.append(Item(child.first, child.second->m_type, child.second->m_date, child.second->m_size));
    }

    return !!node;
}

bool WPDManager::readData(const QString & path, char * data)
{
    HRESULT hr = E_FAIL;

    if (!path.isEmpty() && data) {
        Microsoft::WRL::ComPtr<IStream> stream;
        DWORD optimalTransferSize = 0;
        int size = 0;

        DeviceData *device = findDevice(path.split('/')[0]);
        DeviceNode *node = findNode(path);
        if (device && node) {
            if (SUCCEEDED(hr))
                hr = device->m_resources->GetStream(node->m_objectID.toStdWString().c_str(), WPD_RESOURCE_DEFAULT, STGM_READ, &optimalTransferSize, &stream);
            size = node->m_size;
        }

        if (SUCCEEDED(hr) && stream &&optimalTransferSize && size) {
            ULONG readBytes = 0;
            int totalReadBytes = 0;
            int remainingBytes = size;

            while (hr == S_OK && remainingBytes > 0) {
                int bytesToRead = (remainingBytes > optimalTransferSize) ? optimalTransferSize : remainingBytes;
                hr = stream->Read(data + totalReadBytes, bytesToRead, &readBytes);
                totalReadBytes += readBytes;
                remainingBytes -= readBytes;
                readBytes = 0;
            }

            if (SUCCEEDED(hr) && remainingBytes != 0)
                hr = E_FAIL;
        }
    }

    return SUCCEEDED(hr);
}

bool WPDManager::createFolder(const QString & path, const QString & folderName)
{
    HRESULT hr = E_FAIL;

    if (!path.isEmpty()) {
        DeviceData *device = findDevice(path.split('/')[0]);
        DeviceNode *node = findNode(path);

        if (device && node && (node->m_type == FOLDER || node->m_type == DRIVE)) {
            Microsoft::WRL::ComPtr<IPortableDeviceValues> objectProperties;

            hr = createBasicObjectProperties(objectProperties, node->m_objectID, folderName, WPD_CONTENT_TYPE_FOLDER);

            if (SUCCEEDED(hr))
                hr = device->m_content->CreateObjectWithPropertiesOnly(objectProperties.Get(), nullptr);
        }
    }

    return SUCCEEDED(hr);
}

bool WPDManager::createFile(const QString & path, const QString & fileName, const char * data, int size)
{
    HRESULT hr = E_FAIL;

    if (!path.isEmpty() && !fileName.isEmpty() && data && size) {
        Microsoft::WRL::ComPtr<IStream> stream;
        DWORD optimalTransferSize = 0;

        DeviceData *device = findDevice(path.split('/')[0]);
        DeviceNode *node = findNode(path);
        if (device && node && (node->m_type == FOLDER || node->m_type == DRIVE)) {
            Microsoft::WRL::ComPtr<IPortableDeviceValues> objectProperties;

            hr = createBasicObjectProperties(objectProperties, node->m_objectID, fileName, WPD_CONTENT_TYPE_UNSPECIFIED);

            if (SUCCEEDED(hr))
               hr = objectProperties->SetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, size);

            if (SUCCEEDED(hr))
                hr = device->m_content->CreateObjectWithPropertiesAndData(objectProperties.Get(), &stream, &optimalTransferSize, nullptr);
        }

        if (SUCCEEDED(hr) && stream && optimalTransferSize) {
            ULONG writtenBytes = 0;
            int totalWrittenBytes = 0;
            int remainingBytes = size;

            while (hr == S_OK && remainingBytes > 0) {
                ULONG bytesToWrite = (remainingBytes > optimalTransferSize) ? optimalTransferSize : remainingBytes;
                hr = stream->Write(data + totalWrittenBytes, bytesToWrite, &writtenBytes);
                totalWrittenBytes += writtenBytes;
                remainingBytes -= writtenBytes;
                writtenBytes = 0;
            }

            if (SUCCEEDED(hr))
                hr = (remainingBytes == 0) ? stream->Commit(STGC_DEFAULT) : E_FAIL;
        }
    }

    return SUCCEEDED(hr);
}

bool WPDManager::registerForEvents(Observer * observer)
{
    auto iter = m_observers.insert(observer);
    return iter.second;
}

bool WPDManager::unregisterForEvents(Observer * observer)
{
    auto iter = m_observers.find(observer);
    if (iter != m_observers.end()) {
        m_observers.erase(iter);
        return true;
    }
    return false;
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

    if (SUCCEEDED(m_hr_init))
        m_hr_init = m_clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ | GENERIC_WRITE);
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

HRESULT WPDManager::createBasicObjectProperties(Microsoft::WRL::ComPtr<IPortableDeviceValues>& objectProperties, const QString & parentID, const QString & name, const GUID & type)
{
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&objectProperties));
    if (SUCCEEDED(hr))
        hr = objectProperties->SetStringValue(WPD_OBJECT_PARENT_ID, parentID.toStdWString().c_str());

    if (SUCCEEDED(hr))
        hr = objectProperties->SetStringValue(WPD_OBJECT_NAME, name.toStdWString().c_str());

    if (SUCCEEDED(hr))
        hr = objectProperties->SetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, name.toStdWString().c_str());

    if (SUCCEEDED(hr))
        hr = objectProperties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, type);

    return hr;
}

HRESULT WPDManager::createDeviceData(const QString & deviceID)
{
    QString friendlyName = "ERROR_NAME";
    DWORD friendlyNameLength = 0;
    Microsoft::WRL::ComPtr<IPortableDevice> device;
    Microsoft::WRL::ComPtr<IPortableDeviceContent> content;
    Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties;
    Microsoft::WRL::ComPtr<IPortableDeviceResources> resources;

    HRESULT hr = m_deviceManager->GetDeviceFriendlyName(deviceID.toStdWString().c_str(), nullptr, &friendlyNameLength);

    if (SUCCEEDED(hr) && friendlyNameLength > 0) {
        PWSTR friendlyNameWSTR = new (std::nothrow) WCHAR[friendlyNameLength];
        if (friendlyNameWSTR != nullptr) {
            ZeroMemory(friendlyNameWSTR, friendlyNameLength * sizeof(WCHAR));
            hr = m_deviceManager->GetDeviceFriendlyName(deviceID.toStdWString().c_str(), friendlyNameWSTR, &friendlyNameLength);
            if (SUCCEEDED(hr))
                friendlyName = QString::fromWCharArray(friendlyNameWSTR);

            delete[] friendlyNameWSTR;
            friendlyNameWSTR = nullptr;
        }
    }

    if (SUCCEEDED(hr))
        hr = CoCreateInstance(CLSID_PortableDeviceFTM, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&device));

    if (SUCCEEDED(hr))
        hr = device->Open(deviceID.toStdWString().c_str(), m_clientInformation.Get());

    if (SUCCEEDED(hr))
        hr = device->Content(&content);

    if (SUCCEEDED(hr))
        hr = content->Properties(&properties);

    if (SUCCEEDED(hr))
        hr = content->Transfer(&resources);

    if (SUCCEEDED(hr)) {
        if (device && content && properties && resources) {
            DeviceData *deviceData = new DeviceData(friendlyName, device, content, properties, resources);
            if (deviceData) {
                auto &deviceIter = m_deviceMap.emplace(friendlyName, deviceData);
                auto &deviceIDIter = m_deviceIDMap.emplace(deviceID, friendlyName);
                if (deviceIter.second && deviceIDIter.second && deviceData->m_rootNode) {
                    auto &mapIter = deviceData->m_objectIDMap.emplace(deviceData->m_rootNode->m_objectID, deviceData->m_rootNode);
                    hr = mapIter.second ? S_OK : E_FAIL;
                }

                if (SUCCEEDED(hr))
                    hr = registerForDeviceEvents(*deviceData);
            }
        }
        else
            hr = E_FAIL;
    }

    return hr;
}

HRESULT WPDManager::registerForDeviceEvents(DeviceData &device)
{
    HRESULT hr = E_FAIL;
    if (device.m_eventCookie.isEmpty()) {
        PWSTR tempEventCookie = nullptr;
        Microsoft::WRL::ComPtr<WPDDeviceEventsCallback> callback = new (std::nothrow) WPDDeviceEventsCallback(*this);
        hr = callback ? S_OK : E_OUTOFMEMORY;

        if (SUCCEEDED(hr))
            hr = device.m_device->Advise(0, callback.Get(), nullptr, &tempEventCookie);

        if (SUCCEEDED(hr))
            device.m_eventCookie = QString::fromWCharArray(tempEventCookie);

        CoTaskMemFree(tempEventCookie);
        tempEventCookie = nullptr;
    }

    return hr;
}

HRESULT WPDManager::unregisterForDeviceEvents(DeviceData &device)
{
    HRESULT hr = E_FAIL;
    if (!device.m_eventCookie.isEmpty()) {
        hr = device.m_device->Unadvise(device.m_eventCookie.toStdWString().c_str());
        device.m_eventCookie.clear();
    }

    return hr;
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
                        hr = createDeviceData(QString::fromWCharArray(pnpDeviceIDs[index]));
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

WPDManager::DeviceNode *WPDManager::createNode(DeviceData &device, DeviceNode &parent, const QString &objectID)
{
    DeviceNode *node = new DeviceNode(&parent, objectID);
    if (node) {
        if (fetchData(device, *node)) {
            parent.m_children.emplace(node->m_name, node);
            device.m_objectIDMap.emplace(node->m_objectID, node);
        }
        else {
            delete node;
            node = nullptr;
        }
    }

    return node;
}

bool WPDManager::populate(DeviceData &device, DeviceNode &node)
{
    if (node.m_populatedChildren)
        return true;

    Microsoft::WRL::ComPtr<IEnumPortableDeviceObjectIDs> enumObjectIDs;
    HRESULT hr = device.m_content->EnumObjects(0, node.m_objectID.toStdWString().c_str(), nullptr, &enumObjectIDs);

    while (hr == S_OK)
    {
        DWORD numFetched = 0;
        PWSTR objectIDArray[NUM_OBJECTS_TO_REQUEST] = { 0 };
        hr = enumObjectIDs->Next(NUM_OBJECTS_TO_REQUEST, objectIDArray, &numFetched);
        if (SUCCEEDED(hr))
        {
            for (DWORD index = 0; (index < numFetched) && (objectIDArray[index] != nullptr); index++)
            {
                createNode(device, node, QString::fromWCharArray(objectIDArray[index]));
                CoTaskMemFree(objectIDArray[index]);
                objectIDArray[index] = nullptr;
            }
        }
    }

     node.m_populatedChildren = true;
     return SUCCEEDED(hr);
}

bool WPDManager::fetchData(DeviceData &device, DeviceNode &node)
{
    PWSTR str = nullptr;
    Microsoft::WRL::ComPtr<IPortableDeviceValues> objectProperties;
    HRESULT hr = device.m_properties->GetValues(node.m_objectID.toStdWString().c_str(), m_propertiesToRead.Get(), &objectProperties);

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

    if (SUCCEEDED(hr)) {
        hr = objectProperties->GetStringValue(WPD_OBJECT_NAME, &str);
        if (SUCCEEDED(hr))
            node.m_name = QString::fromStdWString(str);
        CoTaskMemFree(str);
        str = nullptr;
    }

    if (SUCCEEDED(hr) && node.m_type != DRIVE) {

        hr = objectProperties->GetStringValue(WPD_OBJECT_DATE_MODIFIED, &str);
        if (SUCCEEDED(hr))
            node.m_date = QString::fromStdWString(str);
        CoTaskMemFree(str);
        str = nullptr;
    }

    if (SUCCEEDED(hr) && node.m_type == FILE)
        hr = objectProperties->GetUnsignedIntegerValue(WPD_OBJECT_SIZE, &node.m_size);

    return SUCCEEDED(hr);
}

WPDManager::DeviceData * WPDManager::findDevice(const QString & deviceName)
{
    DeviceData *device = nullptr;
    auto &deviceIter = m_deviceMap.find(deviceName);
    if (deviceIter != m_deviceMap.end())
        device = deviceIter->second;

    return device;
}

WPDManager::DeviceNode * WPDManager::findNode(const QString & path)
{
    DeviceNode *node = nullptr;

    QStringList splittedPath = path.split('/');
    if (!splittedPath.empty()) {
        int index = 0;
        DeviceData *device = findDevice(splittedPath[index++]);
        if (device) {
            node = device->m_rootNode;
            while (node && index < splittedPath.size()) {
                populate(*device, *node);
                auto nodeIter = node->m_children.find(splittedPath[index++]);
                node = nodeIter != node->m_children.end() ? nodeIter->second : nullptr;
            }

            if (node)
                populate(*device, *node);
        }
    }

    return node;
}

QString WPDManager::findPath(const DeviceNode & node)
{
    if (node.m_parent)
        return findPath(*node.m_parent) + "/" + node.m_name;
    return node.m_name;
}

void WPDManager::removeDevice(const QString & deviceID)
{
    auto &deviceIDIter = m_deviceIDMap.find(deviceID);
    if (deviceIDIter == m_deviceIDMap.end())
        return;

    QString deviceName = deviceIDIter->second;
    m_deviceIDMap.erase(deviceIDIter);

    auto &deviceIter = m_deviceMap.find(deviceName);
    if (deviceIter != m_deviceMap.end()) {
        if (deviceIter->second) {
            m_removedDevices.push_back(deviceIter->second->m_device);
            unregisterForDeviceEvents(*deviceIter->second);
            delete deviceIter->second;
        }
        deviceIter->second = nullptr;
        m_deviceMap.erase(deviceIter);
        
        for (Observer *observer : m_observers) {
            if (observer)
                observer->removeDevice(deviceName);
        }
    }
}

void WPDManager::addObject(const QString & deviceID, const QString &parentID, const QString& objectID)
{
    auto &deviceIDIter = m_deviceIDMap.find(deviceID);
    if (deviceIDIter == m_deviceIDMap.end())
        return;

    QString deviceName = deviceIDIter->second;
    DeviceData *device = findDevice(deviceName);
    if (device) {
        auto &parentIter = device->m_objectIDMap.find(parentID);
        if (parentIter != device->m_objectIDMap.end()) {
            DeviceNode *parent = parentIter->second;
            if (parent) {
                DeviceNode *node = createNode(*device, *parent, objectID);

                if (node) {
                    for (Observer *observer : m_observers) {
                        if (observer)
                            observer->addItem(findPath(*node));
                    }
                }
            }
        }
    }
}

void WPDManager::updateObject(const QString & deviceID, const QString& objectID)
{
    auto &deviceIDIter = m_deviceIDMap.find(deviceID);
    if (deviceIDIter == m_deviceIDMap.end())
        return;

    QString deviceName = deviceIDIter->second;
    DeviceData *device = findDevice(deviceName);
    if (device) {
        auto &objectIter = device->m_objectIDMap.find(objectID);
        if (objectIter != device->m_objectIDMap.end()) {
            DeviceNode *node = objectIter->second;
            if (node) {
                QString oldPath = findPath(*node);
                DeviceNode *parent = node->m_parent;
                if (parent)
                    parent->m_children.erase(node->m_name);
                fetchData(*device, *node);

                if (parent)
                parent->m_children.emplace(node->m_name, node);

                for (Observer *observer : m_observers) {
                    if (observer)
                        observer->updateItem(oldPath);
                }
            }
        }
    }
}

void WPDManager::removeObject(const QString & deviceID, const QString& objectID)
{
    auto &deviceIDIter = m_deviceIDMap.find(deviceID);
    if (deviceIDIter == m_deviceIDMap.end())
        return;

    QString deviceName = deviceIDIter->second;
    DeviceData *device = findDevice(deviceName);
    if (device) {
        auto &objectIter = device->m_objectIDMap.find(objectID);
        if (objectIter != device->m_objectIDMap.end()) {
            DeviceNode *node = objectIter->second;
            if (node) {
                device->m_objectIDMap.erase(objectIter);
                if (node->m_parent)
                    node->m_parent->m_children.erase(node->m_name);

                QString path = findPath(*node);
                delete node;
                node = nullptr;

                for (Observer *observer : m_observers) {
                    if (observer)
                        observer->removeItem(path);
                }
            }
        }
    }
}

WPDManager::DeviceNode::DeviceNode(DeviceNode *parent, const QString &objectID) :
    m_parent(parent), m_objectID(objectID)
{
}

WPDManager::DeviceNode::~DeviceNode()
{
    for (auto &child : m_children) {
        if (child.second)
            delete child.second;
        child.second = nullptr;
    }
}

WPDManager::DeviceData::DeviceData(const QString &deviceName, Microsoft::WRL::ComPtr<IPortableDevice> device, Microsoft::WRL::ComPtr<IPortableDeviceContent> content,
    Microsoft::WRL::ComPtr<IPortableDeviceProperties> properties, Microsoft::WRL::ComPtr<IPortableDeviceResources> resources)
    : m_device(device), m_content(content), m_properties(properties), m_resources(resources),
    m_rootNode(new DeviceNode(nullptr, QString::fromWCharArray(WPD_DEVICE_OBJECT_ID)))
{
    if (m_rootNode)
        m_rootNode->m_name = deviceName;
}

WPDManager::DeviceData::~DeviceData()
{
    m_objectIDMap.clear();
    if (m_rootNode)
        delete m_rootNode;
    m_rootNode = nullptr;

    m_resources.Reset();
    m_properties.Reset();
    m_content.Reset();
    m_device.Reset();
}

WPDManager::Item::Item(QString name, ItemType type, QString date, int size) :
    m_name(name), m_type(type), m_date(date), m_size(size)
{
}
