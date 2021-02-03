#pragma once
#include "WPDManager.h"


class WPDInstance
{
public:
    static WPDManager &get();

private:
    WPDInstance();
    ~WPDInstance();
};