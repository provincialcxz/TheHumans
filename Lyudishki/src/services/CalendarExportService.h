#pragma once

#include "data/repositories/IPersonRepository.h"
#include <QString>
#include <memory>

class CalendarExportService {
public:
    explicit CalendarExportService(std::shared_ptr<IPersonRepository> repo);

    bool exportToIcs(const QString &filePath, int groupId = -1, bool includeEvents = true,
                      int birthdayRemindDays = 3);

private:
    std::shared_ptr<IPersonRepository> m_repo;
    QString escapeIcs(const QString &text);
};
