#include "CalendarExportService.h"
#include <QFile>
#include <QTextStream>
#include <QUuid>

CalendarExportService::CalendarExportService(std::shared_ptr<IPersonRepository> repo)
    : m_repo(std::move(repo))
{
}

QString CalendarExportService::escapeIcs(const QString &text)
{
    QString escaped = text;
    escaped.replace("\\", "\\\\");
    escaped.replace(";", "\\;");
    escaped.replace(",", "\\,");
    escaped.replace("\n", "\\n");
    return escaped;
}

bool CalendarExportService::exportToIcs(const QString &filePath, int groupId, bool includeEvents,
                                         int birthdayRemindDays)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << "BEGIN:VCALENDAR\r\n";
    out << "VERSION:2.0\r\n";
    out << "PRODID:-//Lyudishki//RU\r\n";
    out << "CALSCALE:GREGORIAN\r\n";

    auto people = (groupId > 0) ? m_repo->getByGroupId(groupId) : m_repo->getAll();

    for (const auto &p : people) {
        if (!p.birthDate.isValid()) continue;
        QString uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        out << "BEGIN:VEVENT\r\n";
        out << "UID:" << uid << "\r\n";
        out << "DTSTART;VALUE=DATE:" << p.birthDate.toString("yyyyMMdd") << "\r\n";
        out << "SUMMARY:" << escapeIcs("ДР: " + p.lastName + " " + p.firstName) << "\r\n";
        out << "RRULE:FREQ=YEARLY\r\n";
        if (birthdayRemindDays > 0) {
            out << "BEGIN:VALARM\r\n";
            out << "TRIGGER:-P" << birthdayRemindDays << "D\r\n";
            out << "ACTION:DISPLAY\r\n";
            out << "DESCRIPTION:" << escapeIcs("День рождения: " + p.lastName + " " + p.firstName) << "\r\n";
            out << "END:VALARM\r\n";
        }
        out << "END:VEVENT\r\n";
    }

    if (includeEvents) {
        for (const auto &p : people) {
            for (const auto &ev : m_repo->getEvents(p.id)) {
                if (!ev.date.isValid()) continue;
                QString uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
                out << "BEGIN:VEVENT\r\n";
                out << "UID:" << uid << "\r\n";
                out << "DTSTART;VALUE=DATE:" << ev.date.toString("yyyyMMdd") << "\r\n";
                out << "SUMMARY:" << escapeIcs(ev.title + " (" + p.lastName + " " + p.firstName + ")") << "\r\n";
                if (ev.recursYearly)
                    out << "RRULE:FREQ=YEARLY\r\n";
                if (ev.remindDaysBefore > 0) {
                    out << "BEGIN:VALARM\r\n";
                    out << "TRIGGER:-P" << ev.remindDaysBefore << "D\r\n";
                    out << "ACTION:DISPLAY\r\n";
                    out << "DESCRIPTION:" << escapeIcs(ev.title) << "\r\n";
                    out << "END:VALARM\r\n";
                }
                out << "END:VEVENT\r\n";
            }
        }
    }

    out << "END:VCALENDAR\r\n";
    file.close();
    return true;
}
