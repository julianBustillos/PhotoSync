#pragma once
#include "WPDManager.h"


class WPDInstance
{
public:
    static WPDManager &get();
    static void free();

private:
    static WPDManager* s_manager;

private:
    WPDInstance();
    ~WPDInstance();
};