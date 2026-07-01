#pragma once

#include <QString>
#include <QDateTime>

struct RelationshipStatusChange {
    int id = 0;
    int personId = 0;
    QString oldValue;
    QString newValue;
    QDateTime changedAt;
};
