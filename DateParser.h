#pragma once
#include <QByteArray>
#include "FileData.h"


namespace DateParser
{
    bool fromJPGBuffer(const QByteArray &fileData, Date &date);
    bool fromMP4FilePath(const QString &filePath, Date &date);
}