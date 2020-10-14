#pragma once
#include <ui_PhotoSync.h>


class FileManager
{
public:
    FileManager(Ui::PhotoSyncClass &ui);
    ~FileManager();
    void run();

private:
    Ui::PhotoSyncClass &m_ui;
};