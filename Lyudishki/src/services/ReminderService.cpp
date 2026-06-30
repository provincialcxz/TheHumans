#include "ReminderService.h"
#include <QDate>

ReminderService::ReminderService(std::shared_ptr<IPersonRepository> repo)
    : m_repo(std::move(repo))
{
}

QVector<ReminderService::UpcomingEvent> ReminderService::getUpcomingEvents(int withinDays)
{
    QVector<UpcomingEvent> result;
    const QDate today = QDate::currentDate();

    for (const auto &p : m_repo->getAll()) {
        if (p.birthDate.isValid()) {
            QDate nextBD(today.year(), p.birthDate.month(), p.birthDate.day());
            if (nextBD < today)
                nextBD = nextBD.addYears(1);
            int days = static_cast<int>(today.daysTo(nextBD));
            if (days <= withinDays) {
                UpcomingEvent ue;
                ue.personName = p.lastName + " " + p.firstName;
                ue.eventTitle = "День рождения";
                ue.eventDate = nextBD;
                ue.daysUntil = days;
                result.append(ue);
            }
        }
    }

    for (const auto &ev : m_repo->getAllEvents()) {
        if (!ev.date.isValid()) continue;
        int remind = ev.remindDaysBefore;
        QDate eventDate = ev.date;
        if (ev.recursYearly) {
            eventDate = QDate(today.year(), ev.date.month(), ev.date.day());
            if (eventDate < today)
                eventDate = eventDate.addYears(1);
        }
        int days = static_cast<int>(today.daysTo(eventDate));
        if (days >= 0 && days <= remind) {
            Person person = m_repo->getById(ev.personId);
            UpcomingEvent ue;
            ue.personName = person.lastName + " " + person.firstName;
            ue.eventTitle = ev.title;
            ue.eventDate = eventDate;
            ue.daysUntil = days;
            result.append(ue);
        }
    }

    return result;
}
