#include "WPDEvents.h"
#include "WPDManager.h"
#include <PortableDevice.h> 
#include <QString>


WPDDeviceEventsCallback::WPDDeviceEventsCallback(WPDManager &manager) :
    m_ref(0), m_manager(manager)
{
}

WPDDeviceEventsCallback::~WPDDeviceEventsCallback()
{
}

HRESULT WPDDeviceEventsCallback::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = S_OK;
    if (ppvObj == NULL) {
        hr = E_INVALIDARG;
        return hr;
    }

    if ((riid == IID_IUnknown) || (riid == IID_IPortableDeviceEventCallback)) {
        AddRef();
        *ppvObj = this;
    }
    else {
        hr = E_NOINTERFACE;
    }
    return hr;
}

ULONG WPDDeviceEventsCallback::AddRef()
{
    return InterlockedIncrement(&m_ref);
}

ULONG WPDDeviceEventsCallback::Release()
{
    long ref = InterlockedDecrement(&m_ref);
    if (ref == 0)
    {
        delete this;
        return 0;
    }
    return ref;
}

HRESULT WPDDeviceEventsCallback::OnEvent(IPortableDeviceValues * eventParameters)
{
    if (eventParameters != nullptr) {
        HRESULT hr = E_FAIL;
        QString deviceID, parentID, objectID;
        PWSTR tempStr = nullptr;
        GUID guid = GUID_NULL;

        hr = eventParameters->GetStringValue(WPD_EVENT_PARAMETER_PNP_DEVICE_ID, &tempStr);
        if (SUCCEEDED(hr))
            deviceID = QString::fromWCharArray(tempStr);
        CoTaskMemFree(tempStr);
        tempStr = nullptr;

        hr = eventParameters->GetStringValue(WPD_OBJECT_PARENT_ID, &tempStr);
        if (SUCCEEDED(hr))
            parentID = QString::fromStdWString(tempStr);
        CoTaskMemFree(tempStr);
        tempStr = nullptr;

        hr = eventParameters->GetStringValue(WPD_OBJECT_ID, &tempStr);
        if (SUCCEEDED(hr))
            objectID = QString::fromStdWString(tempStr);
        CoTaskMemFree(tempStr);
        tempStr = nullptr;

        hr = eventParameters->GetGuidValue(WPD_EVENT_PARAMETER_EVENT_ID, &guid);
        if (SUCCEEDED(hr))
        {
            if (guid == WPD_EVENT_DEVICE_REMOVED || guid == WPD_EVENT_DEVICE_RESET) {
                m_manager.removeDevice(deviceID);
            }
            else if (guid == WPD_EVENT_OBJECT_ADDED) {
                m_manager.addObject(deviceID, parentID, objectID);
            }
            else if (guid == WPD_EVENT_OBJECT_UPDATED) {
                m_manager.updateObject(deviceID, objectID);
            }
            else if (guid == WPD_EVENT_OBJECT_REMOVED) {
                m_manager.removeObject(deviceID, objectID);
            }
        }
    }

    return S_OK;
}
