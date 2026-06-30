#pragma once

#include <QString>
#include <QDateTime>

struct PersonNote {
    int id = 0;
    int personId = 0;
    QString text;
    QDateTime createdAt;
};
