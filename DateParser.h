#pragma once
#include <QByteArray>
#include "FileData.h"


namespace DateParser
{
    bool fromJPGBuffer(const QByteArray &buffer, Date &date);
    bool fromMPRBuffer(const QByteArray &buffer, Date &date);
}