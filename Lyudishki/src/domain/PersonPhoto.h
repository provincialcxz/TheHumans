#pragma once

#include <QString>
#include <QDateTime>

struct PersonPhoto {
    int id = 0;
    int personId = 0;
    QString filePath;
    QDateTime addedAt;
};
