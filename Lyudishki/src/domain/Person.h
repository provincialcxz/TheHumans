#pragma once

#include <QString>
#include <QDate>
#include <QDateTime>

struct Person {
    int id = 0;
    int groupId = 0;
    QString photoPath;
    QString lastName;
    QString firstName;
    QString patronymic;
    QDate birthDate;
    QString phone;
    QString primaryContactMethod;
    QString address;
    QString habits;
    QString hobbies;
    QString note;
    bool metInPerson = false;
    QDate lastContactDate;
    QDateTime createdAt;
};
