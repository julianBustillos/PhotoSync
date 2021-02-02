#pragma once
#include <PortableDeviceApi.h> 
#include <QThread>

class WPDManager;


class WPDUSBDetector : public QThread
{
public:
    WPDUSBDetector(WPDManager &manager);
    ~WPDUSBDetector();

private:
    void run() override;
    void initialize();
    void finalize();

private:
    static LRESULT connectedUSBCallback(HWND__ *hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    WPDManager &m_manager;
    HWND m_hWnd;
    HDEVNOTIFY m_hDeviceNotify;
    int m_arrivalCount;
};


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
