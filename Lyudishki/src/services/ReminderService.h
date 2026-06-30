#pragma once

#include "data/repositories/IPersonRepository.h"
#include <memory>

class ReminderService {
public:
    explicit ReminderService(std::shared_ptr<IPersonRepository> repo);

    struct UpcomingEvent {
        QString personName;
        QString eventTitle;
        QDate eventDate;
        int daysUntil;
    };

    QVector<UpcomingEvent> getUpcomingEvents(int withinDays);

private:
    std::shared_ptr<IPersonRepository> m_repo;
};
