#pragma once

#include <QString>

struct PersonRelation {
    int id = 0;
    int personAId = 0;
    int personBId = 0;
    QString relationType; // родственник, друг, коллега, партнёр...
    QString note;
};
