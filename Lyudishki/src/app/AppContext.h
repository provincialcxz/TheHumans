#pragma once

#include "data/db/DatabaseManager.h"
#include "data/db/MigrationManager.h"
#include "data/repositories/SqlitePersonRepository.h"
#include "data/repositories/SqliteGroupRepository.h"
#include "services/PeopleService.h"
#include "services/GroupService.h"
#include "services/SearchService.h"
#include "services/ReminderService.h"
#include "services/CalendarExportService.h"
#include "services/DataService.h"
#include <memory>
#include <QString>

class AppContext {
public:
    AppContext(const QString &dbPath, const QString &dataDir);

    bool initialize();

    std::shared_ptr<PeopleService> peopleService() const { return m_peopleService; }
    std::shared_ptr<GroupService> groupService() const { return m_groupService; }
    std::shared_ptr<SearchService> searchService() const { return m_searchService; }
    std::shared_ptr<ReminderService> reminderService() const { return m_reminderService; }
    std::shared_ptr<CalendarExportService> calendarExportService() const { return m_calendarService; }
    std::shared_ptr<DataService> dataService() const { return m_dataService; }
    QString dataDir() const { return m_dataDir; }
    QString dbPath() const { return m_dbPath; }

private:
    QString m_dataDir;
    QString m_dbPath;
    std::unique_ptr<DatabaseManager> m_dbManager;
    std::shared_ptr<SqlitePersonRepository> m_personRepo;
    std::shared_ptr<SqliteGroupRepository> m_groupRepo;
    std::shared_ptr<PeopleService> m_peopleService;
    std::shared_ptr<GroupService> m_groupService;
    std::shared_ptr<SearchService> m_searchService;
    std::shared_ptr<ReminderService> m_reminderService;
    std::shared_ptr<CalendarExportService> m_calendarService;
    std::shared_ptr<DataService> m_dataService;
};
