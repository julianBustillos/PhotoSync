#pragma once
#include <PortableDeviceApi.h> 

class WPDManager;


class WPDDeviceEventsCallback  : public IPortableDeviceEventCallback
{
public:
    WPDDeviceEventsCallback(WPDManager &manager);
    ~WPDDeviceEventsCallback();

public:
    HRESULT __stdcall QueryInterface(REFIID riid, LPVOID* ppvObj);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall OnEvent(IPortableDeviceValues* eventParameters);

private:
    ULONG m_ref;
    WPDManager &m_manager;
};
