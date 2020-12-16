#pragma once
#include <QString>


class MTPFileNodePathKey : public QString
{
public:
    MTPFileNodePathKey() {}
    MTPFileNodePathKey(const QString &other) : QString(other) {}
    MTPFileNodePathKey(const MTPFileNodePathKey &other) : QString(other) {}
    bool operator==(const MTPFileNodePathKey &other) const { return !compare(other, Qt::CaseInsensitive); }
};
Q_DECLARE_TYPEINFO(MTPFileNodePathKey, Q_MOVABLE_TYPE);

inline uint qHash(const MTPFileNodePathKey &key) { return qHash(key.toCaseFolded()); }