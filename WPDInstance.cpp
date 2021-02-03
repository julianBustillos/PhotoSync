#include "WPDInstance.h"


WPDManager & WPDInstance::get()
{
    static WPDManager manager;
    return manager;
}

WPDInstance::WPDInstance()
{
}

WPDInstance::~WPDInstance()
{
}
