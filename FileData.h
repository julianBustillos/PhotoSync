#pragma once
#include <QString>
#include <QByteArray>
#include "ExtendedFileSystem.h"


struct Date {
    int m_year;
    int m_month;

    Date() : m_year(0), m_month(0) {};
    QString toQString() const;
};

bool operator<(const Date &lhs, const Date &rhs);

struct ExistingFile {
    const EFS::Path m_path;
    QByteArray m_checksum;

    ExistingFile(const EFS::Path &path) : m_path(path) {};
};

struct ExportFile {
    const Date m_date;
    const EFS::Path m_path;

    ExportFile(const Date &date, const EFS::Path &path) : m_date(date), m_path(path) {};
};
