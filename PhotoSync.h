#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PhotoSync.h"

class PhotoSync : public QMainWindow
{
    Q_OBJECT

public:
    PhotoSync(QWidget *parent = Q_NULLPTR);

private:
    Ui::PhotoSyncClass ui;
};
