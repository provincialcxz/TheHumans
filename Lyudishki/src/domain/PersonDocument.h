#pragma once

#include <QString>

struct PersonDocument {
    int id = 0;
    int personId = 0;
    QString type;   // паспорт, СНИЛС, ИНН, банковские реквизиты...
    QString value;
    QString note;
};
