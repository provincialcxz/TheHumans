#pragma once

#include <QString>
#include <QDateTime>

struct PersonFile {
    int id = 0;
    int personId = 0;
    QString fileName;
    QString filePath;
    QDateTime addedAt;
};
