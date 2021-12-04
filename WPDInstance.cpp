#include "WPDInstance.h"


WPDManager *WPDInstance::s_manager = nullptr;

WPDManager & WPDInstance::get()
{
    if (!s_manager)
        s_manager = new WPDManager();
    return *s_manager;
}

void WPDInstance::free()
{
    if (s_manager)
        delete s_manager;
    s_manager = nullptr;
}

WPDInstance::WPDInstance()
{
}

WPDInstance::~WPDInstance()
{
}
