#pragma once

#include <QString>

struct SocialAccount {
    int id = 0;
    int personId = 0;
    QString platform;
    QString handleOrUrl;
};
