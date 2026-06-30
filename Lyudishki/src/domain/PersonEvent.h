#pragma once

#include <QString>
#include <QDate>

struct PersonEvent {
    int id = 0;
    int personId = 0;
    QString title;
    QDate date;
    bool recursYearly = false;
    int remindDaysBefore = 3;
};
