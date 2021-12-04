#pragma once
#include <QByteArray>
#include "FileData.h"


namespace DateParser
{
    bool fromJPGBuffer(const QByteArray &buffer, Date &date);
    bool fromMP4Buffer(const QByteArray &buffer, Date &date);
}