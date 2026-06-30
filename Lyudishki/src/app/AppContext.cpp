#include "AppContext.h"
#include <QSqlQuery>

AppContext::AppContext(const QString &dbPath, const QString &dataDir)
    : m_dataDir(dataDir)
    , m_dbPath(dbPath)
    , m_dbManager(std::make_unique<DatabaseManager>(dbPath))
{
}

bool AppContext::initialize()
{
    if (!m_dbManager->isOpen())
        return false;

    QSqlDatabase &db = m_dbManager->database();

    QSqlQuery q(db);
    q.exec("PRAGMA foreign_keys = ON");

    MigrationManager migrator(db);
    if (!migrator.migrate())
        return false;

    m_personRepo = std::make_shared<SqlitePersonRepository>(db);
    m_groupRepo = std::make_shared<SqliteGroupRepository>(db);

    m_peopleService = std::make_shared<PeopleService>(m_personRepo, m_dataDir + "/attachments");
    m_groupService = std::make_shared<GroupService>(m_groupRepo);
    m_searchService = std::make_shared<SearchService>(m_personRepo);
    m_reminderService = std::make_shared<ReminderService>(m_personRepo);
    m_calendarService = std::make_shared<CalendarExportService>(m_personRepo);
    m_dataService = std::make_shared<DataService>(m_personRepo, m_groupRepo);

    return true;
}
