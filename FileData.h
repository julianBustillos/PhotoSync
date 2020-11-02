#pragma once
#include <QString>
#include <QByteArray>


struct Date {
    int m_year;
    int m_month;

    Date() : m_year(0), m_month(0) {};
    QString toQString() const;
};

bool operator<(const Date &lhs, const Date &rhs);

struct ExistingFile {
    QString m_path;
    QByteArray m_checksum;

    ExistingFile(QString &path) : m_path(path) {};
};

struct ExportFile {
    Date m_date;
    QString m_path;

    ExportFile(Date &date, QString &path) : m_date(date), m_path(path) {};
};
