#include "WPDManager.h"
#include "WPDEvents.h"
#include <PortableDevice.h> 
#include <ObjIdl.h>
#include <Shlwapi.h>
#include <Propvarutil.h>


#define WPD_NUM_OBJECTS_TO_REQUEST 100
#define WPD_WAIT_TIMEOUT 1000


WPDManager::WPDManager() :
    m_hr_COM(E_FAIL), m_hr_init(E_FAIL), m_USBDetector(nullptr), m_objectsToWait(0)
{
    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    m_hr_COM = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (!SUCCEEDED(m_hr_COM)) {
        CoUninitialize();
        m_hr_COM = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    }
    m_hr_init = CoCreateInstance(CLSID_PortableDeviceManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_deviceManager));

    createClientInformation();
    createPropertiesToRead();
    createObjectsToDelete();
    fetchDevices();
    registerForUSBEvents();
}

WPDManager::~WPDManager()
{
    m_removedDevices.clear();
    unregisterForUSBEvents();
    m_deviceIDMap.clear();
    for (auto &deviceIter : m_deviceMap) {
        if (deviceIter.second) {
            unregisterForDeviceEvents(*deviceIter.second);
            delete deviceIter.second;
        }
        deviceIter.second = nullptr;
    }
    m_deviceMap.clear();

    m_objectsToDelete.Reset();
    m_propertiesToRead.Reset();
    m_clientInformation.Reset();
    m_deviceManager.Reset();

    if (SUCCEEDED(m_hr_COM))
        CoUninitialize();
}

bool WPDManager::getDevices(QStringList& devices)
{
    startReading();

    if (SUCCEEDED(m_hr_init)) {
        devices.reserve(m_deviceMap.size());
        
        for (auto &deviceIter : m_deviceMap)
            devices.append(deviceIter.first);
    }

    endReading();
    return SUCCEEDED(m_hr_init);
}

bool WPDManager::getItem(const QString &path, Item &item)
{
    startReading();

    DeviceNode *node = findNode(path);
    if (node) {
        item.m_name = path.right(path.size() - path.lastIndexOf('/') - 1);
        item.m_type = (ItemType)node->m_type;
        item.m_date = node->m_date;
        item.m_size = node->m_size;
    }

    endReading();
    return !!node;
}

bool WPDManager::getContent(const QString &path, QVector<Item> &content)
{
    startReading();

    DeviceNode *node = findNode(path);
    if (node) {
        content.reserve(node->m_children.size());
        for (auto &child : node->m_children)
            content.append(Item(child.first, child.second->m_type, child.second->m_date, child.second->m_size));
    }

    endReading();
    return !!node;
}

bool WPDManager::readData(const QString & path, char * data)
{
    HRESULT hr = E_FAIL;

    if (!path.isEmpty() && data) {
        Microsoft::WRL::ComPtr<IStream> stream;
        DWORD optimalTransferSize = 0;
        int size = 0;

        startReading();
        DeviceData *device = findDevice(path);
        DeviceNode *node = findNode(path);
        if (device && node) {
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
        endReading();
    }

    return SUCCEEDED(hr);
}

bool WPDManager::createFolder(const QString & path, const QString & folderName)
{
    HRESULT hr = E_FAIL;

    if (!path.isEmpty()) {
        startWriting();
        DeviceData *device = findDevice(path);
        DeviceNode *node = findNode(path);

        if (device && node && (node->m_type == ItemType::FOLDER || node->m_type == ItemType::DRIVE)) {
            Microsoft::WRL::ComPtr<IPortableDeviceValues> objectProperties;

            hr = createBasicObjectProperties(objectProperties, node->m_objectID, folderName, WPD_CONTENT_TYPE_FOLDER);

            if (SUCCEEDED(hr)) {
                m_objectsToWait = 1;
                hr = device->m_content->CreateObjectWithPropertiesOnly(objectProperties.Get(), nullptr);
            }

            if (SUCCEEDED(hr))
                waitObjects();
        }
        endWriting();
    }

    return SUCCEEDED(hr);
}

bool WPDManager::createFile(const QString & path, const QString & fileName, const char * data, int size)
{
    HRESULT hr = E_FAIL;

    if (!path.isEmpty() && !fileName.isEmpty() && data && size) {
        Microsoft::WRL::ComPtr<IStream> stream;
        DWORD optimalTransferSize = 0;
        
        startWriting();
        DeviceData *device = findDevice(path);
        DeviceNode *node = findNode(path);
        if (device && node && (node->m_type == ItemType::FOLDER || node->m_type == ItemType::DRIVE)) {
            Microsoft::WRL::ComPtr<IPortableDeviceValues> objectProperties;

            hr = createBasicObjectProperties(objectProperties, node->m_objectID, fileName, WPD_CONTENT_TYPE_UNSPECIFIED);

            if (SUCCEEDED(hr))
               hr = objectProperties->SetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, size);

            if (SUCCEEDED(hr)) {
                m_objectsToWait = 1;
                hr = device->m_content->CreateObjectWithPropertiesAndData(objectProperties.Get(), &stream, &optimalTransferSize, nullptr);
            }
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

            if (SUCCEEDED(hr))
                waitObjects();
        }
        endWriting();
    }

    return SUCCEEDED(hr);
}

int WPDManager::deleteObjects(const QStringList& paths, QVector<bool>& results)
{
    if (paths.empty())
        return 0;

    int deleteCount = 0;
    HRESULT hr = E_FAIL;

    startWriting();
    DeviceData* mainDevice = findDevice(paths[0]);
    if (mainDevice) {
        hr = S_OK;
        for (int pathID = 0; pathID < paths.size() && SUCCEEDED(hr); pathID++) {
            DeviceData* device = findDevice(paths[pathID]);
            DeviceNode* node = findNode(paths[pathID]);

            if (device == mainDevice && node) {
                hr = node->m_children.empty() ? S_OK : E_FAIL;
                PROPVARIANT pv = { 0 };

                if (SUCCEEDED(hr))
                    hr = InitPropVariantFromString(node->m_objectID.toStdWString().c_str(), &pv);

                if (SUCCEEDED(hr))
                    hr = m_objectsToDelete->Add(&pv);

                PropVariantClear(&pv);
            }
            else
                hr = E_FAIL;
        }

        if (SUCCEEDED(hr)) {
            m_objectsToWait = paths.size();
            Microsoft::WRL::ComPtr<IPortableDevicePropVariantCollection> objectsHR;
            hr = mainDevice->m_content->Delete(PORTABLE_DEVICE_DELETE_NO_RECURSION, m_objectsToDelete.Get(), &objectsHR);

            DWORD dwCount = 0;
            objectsHR->GetCount(&dwCount);
            for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
            {
                PROPVARIANT pv = { 0 };
                PropVariantInit(&pv);
                hr = objectsHR->GetAt(dwIndex, &pv);
                if (SUCCEEDED(hr)) {
                    results[dwIndex] = SUCCEEDED(pv.scode);
                    if (results[dwIndex])
                        deleteCount++;
                }

                PropVariantClear(&pv);
            }
        }

        if (SUCCEEDED(hr))
            waitObjects();

        m_objectsToDelete->Clear();
    }

    endWriting();
    return SUCCEEDED(hr) ? deleteCount : 0;
}

bool WPDManager::registerForEvents(Observer * observer)
{
    startReading();
    auto iter = m_observers.insert(observer);
    bool registered = iter.second;
    endReading();
    return registered;
}

bool WPDManager::unregisterForEvents(Observer * observer)
{
    startReading();
    bool unregistered = false;
    auto iter = m_observers.find(observer);
    if (iter != m_observers.end()) {
        m_observers.erase(iter);
        unregistered = true;
    }
    endReading();
    return unregistered;
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

void WPDManager::createObjectsToDelete()
{
    if (SUCCEEDED(m_hr_init))
        m_hr_init = CoCreateInstance(CLSID_PortableDevicePropVariantCollection, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_objectsToDelete));
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
                startWriting();
                auto deviceIter = m_deviceMap.emplace(friendlyName, deviceData);
                auto deviceIDIter = m_deviceIDMap.emplace(deviceID, friendlyName);
                if (deviceIter.second && deviceIDIter.second && deviceData->m_rootNode) {
                    auto mapIter = deviceData->m_objectIDMap.emplace(deviceData->m_rootNode->m_objectID, deviceData->m_rootNode);
                    hr = mapIter.second ? S_OK : E_FAIL;
                }
                endWriting();

                startReading();
                if (SUCCEEDED(hr))
                    hr = registerForDeviceEvents(*deviceData);

                if (SUCCEEDED(hr)) {
                    for (Observer *observer : m_observers) {
                        if (observer)
                            observer->addDevice(friendlyName);
                    }
                }
                endReading();
            }
        }
        else
            hr = E_FAIL;
    }

    return hr;
}

HRESULT WPDManager::fetchDevices()
{
    HRESULT hr = E_FAIL;
    if (SUCCEEDED(m_hr_init)) {
        DWORD pnpDeviceIDCount = 0;
        hr = m_deviceManager->GetDevices(nullptr, &pnpDeviceIDCount);

        if (SUCCEEDED(hr) && (pnpDeviceIDCount > 0)) {
            PWSTR* pnpDeviceIDs = new (std::nothrow) PWSTR[pnpDeviceIDCount];
            if (pnpDeviceIDs != nullptr) {
                ZeroMemory(pnpDeviceIDs, pnpDeviceIDCount * sizeof(PWSTR));
                DWORD retrievedDeviceIDCount = pnpDeviceIDCount;
                hr = m_deviceManager->GetDevices(pnpDeviceIDs, &retrievedDeviceIDCount);
                if (retrievedDeviceIDCount <= pnpDeviceIDCount) {
                    for (DWORD index = 0; index < retrievedDeviceIDCount && SUCCEEDED(hr); index++) {
                        QString deviceID = QString::fromWCharArray(pnpDeviceIDs[index]);
                        if (m_deviceIDMap.find(deviceID) == m_deviceIDMap.end())
                            hr = createDeviceData(deviceID);
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
    }

    return hr;
}

HRESULT WPDManager::registerForUSBEvents()
{
    HRESULT hr = E_FAIL;
    if (!m_USBDetector) {
        m_USBDetector = new WPDUSBDetector(*this);
        if (m_USBDetector) {
            m_USBDetector->start(QThread::LowPriority);
            hr = S_OK;
        }
    }
    return hr;
}

HRESULT WPDManager::unregisterForUSBEvents()
{
    HRESULT hr = E_FAIL;
    if (m_USBDetector) {
        delete m_USBDetector;
        m_USBDetector = nullptr;
        hr = S_OK;
    }
    return hr;
}

HRESULT WPDManager::registerForDeviceEvents(DeviceData &device)
{
    HRESULT hr = E_FAIL;
    startReading();
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
    endReading();

    return hr;
}

HRESULT WPDManager::unregisterForDeviceEvents(DeviceData &device)
{
    HRESULT hr = E_FAIL;
    startReading();
    if (!device.m_eventCookie.isEmpty()) {
        hr = device.m_device->Unadvise(device.m_eventCookie.toStdWString().c_str());
        device.m_eventCookie.clear();
    }
    endReading();

    return hr;
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
        PWSTR objectIDArray[WPD_NUM_OBJECTS_TO_REQUEST] = { 0 };
        hr = enumObjectIDs->Next(WPD_NUM_OBJECTS_TO_REQUEST, objectIDArray, &numFetched);
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

bool WPDManager::fetchData(const DeviceData &device, DeviceNode &node)
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

            node.m_type = ItemType::DRIVE;
        }
        else if (typeGUID == WPD_CONTENT_TYPE_FOLDER) {
            node.m_type = ItemType::FOLDER;
        }
        else {
            node.m_type = ItemType::FILE;
        }
    }

    if (SUCCEEDED(hr)) {
        hr = objectProperties->GetStringValue(WPD_OBJECT_NAME, &str);
        if (SUCCEEDED(hr))
            node.m_name = QString::fromStdWString(str);
        CoTaskMemFree(str);
        str = nullptr;
    }

    if (SUCCEEDED(hr) && node.m_type == ItemType::FILE)
        hr = objectProperties->GetUnsignedIntegerValue(WPD_OBJECT_SIZE, &node.m_size);

    if (SUCCEEDED(hr) && node.m_type != ItemType::DRIVE) {

        HRESULT hrOptional = objectProperties->GetStringValue(WPD_OBJECT_DATE_MODIFIED, &str);
        if (SUCCEEDED(hrOptional))
            node.m_date = QString::fromStdWString(str);
        CoTaskMemFree(str);
        str = nullptr;
    }

    return SUCCEEDED(hr);
}

WPDManager::DeviceData * WPDManager::findDevice(const QString & path)
{
    DeviceData *device = nullptr;
    auto deviceIter = m_deviceMap.find(path.split('/')[0]);
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

void WPDManager::refreshDevices()
{
    m_removedDevices.clear();
    m_deviceManager->RefreshDeviceList();
    fetchDevices();
}

void WPDManager::removeDevice(const QString & deviceID)
{
    startWriting();
    auto deviceIDIter = m_deviceIDMap.find(deviceID);
    if (deviceIDIter == m_deviceIDMap.end())
        return;

    QString deviceName = deviceIDIter->second;
    m_deviceIDMap.erase(deviceIDIter);

    auto deviceIter = m_deviceMap.find(deviceName);
    if (deviceIter != m_deviceMap.end()) {
        DeviceData *deviceToRemove = nullptr;

        if (deviceIter->second) {
            m_removedDevices.push_back(deviceIter->second->m_device);
            deviceToRemove = deviceIter->second;
        }
        deviceIter->second = nullptr;
        m_deviceMap.erase(deviceIter);
        endWriting();

        if (deviceToRemove) {
            unregisterForDeviceEvents(*deviceToRemove);
            delete deviceToRemove;
            deviceToRemove = nullptr;
        }
        
        startReading();
        for (Observer *observer : m_observers) {
            if (observer)
                observer->removeDevice(deviceName);
        }
        endReading();
    }
    else {
        endWriting();
    }
}

void WPDManager::addObject(const QString & deviceID, const QString &parentID, const QString& objectID)
{
    QString path;

    startWriting();
    auto deviceIDIter = m_deviceIDMap.find(deviceID);
    if (deviceIDIter == m_deviceIDMap.end())
        return;

    QString deviceName = deviceIDIter->second;
    DeviceData *device = findDevice(deviceName);
    if (device) {
        auto parentIter = device->m_objectIDMap.find(parentID);
        if (parentIter != device->m_objectIDMap.end()) {
            DeviceNode *parent = parentIter->second;
            if (parent) {
                DeviceNode *node = createNode(*device, *parent, objectID);
                if (m_objectsToWait > 0)
                    m_objectsToWait--;
            }
        }
    }
    endWriting();

    if (!path.isEmpty()) {
        startReading();
        for (Observer *observer : m_observers) {
            if (observer)
                observer->addItem(path);
        }
        endReading();
    }
}

void WPDManager::updateObject(const QString & deviceID, const QString& objectID)
{
    QString oldPath;

    startWriting();
    auto deviceIDIter = m_deviceIDMap.find(deviceID);
    if (deviceIDIter == m_deviceIDMap.end()) {
        endWriting();
        return;
    }

    QString deviceName = deviceIDIter->second;
    DeviceData *device = findDevice(deviceName);
    if (device) {
        auto objectIter = device->m_objectIDMap.find(objectID);
        if (objectIter != device->m_objectIDMap.end()) {
            DeviceNode *node = objectIter->second;
            if (node) {
                oldPath = findPath(*node);
                DeviceNode *parent = node->m_parent;
                if (parent)
                    parent->m_children.erase(node->m_name);
                fetchData(*device, *node);

                if (parent)
                    parent->m_children.emplace(node->m_name, node);
            }
        }
    }
    endWriting();

    if (!oldPath.isEmpty()) {
        startReading();
        for (Observer *observer : m_observers) {
            if (observer)
                observer->updateItem(oldPath);
        }
        endReading();
    }
}

void WPDManager::removeObject(const QString & deviceID, const QString& objectID)
{
    QString path;

    startWriting();
    auto deviceIDIter = m_deviceIDMap.find(deviceID);
    if (deviceIDIter == m_deviceIDMap.end()) {
        endWriting();
        return;
    }

    QString deviceName = deviceIDIter->second;
    DeviceData *device = findDevice(deviceName);
    if (device) {
        auto objectIter = device->m_objectIDMap.find(objectID);
        if (objectIter != device->m_objectIDMap.end()) {
            DeviceNode *node = objectIter->second;
            if (node) {
                device->m_objectIDMap.erase(objectIter);
                if (node->m_parent)
                    node->m_parent->m_children.erase(node->m_name);
                if (m_objectsToWait > 0)
                    m_objectsToWait--;
                delete node;
                node = nullptr;
            }
        }
    }
    endWriting();

    if (!path.isEmpty()) {
        startReading();
        for (Observer *observer : m_observers) {
            if (observer)
                observer->removeItem(path);
        }
        endReading();
    }
}

void WPDManager::startReading()
{
    QMutexLocker locker(&m_mutex);
    m_readCount.fetchAndAddRelaxed(1);
}

void WPDManager::endReading()
{
    QMutexLocker locker(&m_mutex);
    m_readCount.fetchAndSubRelaxed(1);
    m_readFinished.wakeAll();
}

void WPDManager::startWriting()
{
    m_mutex.lock();
    while (m_readCount.loadRelaxed() > 0) {
        m_readFinished.wait(&m_mutex, WPD_WAIT_TIMEOUT);
    }
}

void WPDManager::endWriting()
{
    m_writeFinished.wakeAll();
    m_mutex.unlock();
}

bool WPDManager::waitObjects()
{
    bool timeout = false;
    while (!timeout) {
        if (m_objectsToWait == 0)
            break;
        timeout = !m_writeFinished.wait(&m_mutex, WPD_WAIT_TIMEOUT);
    }

    return !timeout;
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
