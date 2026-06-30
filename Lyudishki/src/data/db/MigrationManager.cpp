#include "MigrationManager.h"
#include <QSqlQuery>
#include <QSqlError>

MigrationManager::MigrationManager(QSqlDatabase &db)
    : m_db(db)
{
}

bool MigrationManager::migrate()
{
    int ver = currentVersion();
    const int targetVersion = 3;
    while (ver < targetVersion) {
        int next = ver + 1;
        qInfo("MigrationManager: running migration v%d", next);
        if (!runMigration(next)) {
            qWarning("MigrationManager: migration v%d failed", next);
            return false;
        }
        setVersion(next);
        ver = next;
    }
    return true;
}

int MigrationManager::currentVersion()
{
    QSqlQuery q(m_db);
    q.exec("PRAGMA user_version");
    if (q.next())
        return q.value(0).toInt();
    return 0;
}

void MigrationManager::setVersion(int version)
{
    QSqlQuery q(m_db);
    q.exec(QString("PRAGMA user_version = %1").arg(version));
}

bool MigrationManager::runMigration(int version)
{
    switch (version) {
    case 1: return migration_v1();
    case 2: return migration_v2();
    case 3: return migration_v3();
    default: return false;
    }
}

bool MigrationManager::migration_v1()
{
    QSqlQuery q(m_db);
    QStringList statements = {
        "CREATE TABLE IF NOT EXISTS groups ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL UNIQUE"
        ")",

        "CREATE TABLE IF NOT EXISTS person ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  group_id INTEGER NOT NULL REFERENCES groups(id),"
        "  photo_path TEXT DEFAULT '',"
        "  last_name TEXT NOT NULL DEFAULT '',"
        "  first_name TEXT NOT NULL DEFAULT '',"
        "  patronymic TEXT DEFAULT '',"
        "  birth_date TEXT,"
        "  phone TEXT DEFAULT '',"
        "  primary_contact_method TEXT DEFAULT '',"
        "  address TEXT DEFAULT '',"
        "  habits TEXT DEFAULT '',"
        "  hobbies TEXT DEFAULT '',"
        "  note TEXT DEFAULT '',"
        "  created_at TEXT NOT NULL DEFAULT (datetime('now'))"
        ")",

        "CREATE TABLE IF NOT EXISTS person_profile ("
        "  person_id INTEGER PRIMARY KEY REFERENCES person(id) ON DELETE CASCADE,"
        "  character TEXT DEFAULT '',"
        "  interests TEXT DEFAULT '',"
        "  humor_and_triggers TEXT DEFAULT '',"
        "  political_views TEXT DEFAULT '',"
        "  religious_views TEXT DEFAULT '',"
        "  languages TEXT DEFAULT '',"
        "  nicknames TEXT DEFAULT '',"
        "  family TEXT DEFAULT '',"
        "  relationship_status TEXT DEFAULT '',"
        "  mutual_acquaintances TEXT DEFAULT '',"
        "  how_and_when_met TEXT DEFAULT '',"
        "  career_track TEXT DEFAULT '',"
        "  education TEXT DEFAULT '',"
        "  skills TEXT DEFAULT '',"
        "  work_schedule TEXT DEFAULT '',"
        "  work_contacts TEXT DEFAULT '',"
        "  platform_accounts TEXT DEFAULT '',"
        "  media_consumption TEXT DEFAULT '',"
        "  property TEXT DEFAULT '',"
        "  favorite_places TEXT DEFAULT '',"
        "  pets TEXT DEFAULT '',"
        "  health_and_allergies TEXT DEFAULT '',"
        "  reliability TEXT DEFAULT '',"
        "  info_freshness TEXT DEFAULT '',"
        "  additional_notes TEXT DEFAULT ''"
        ")",

        "CREATE TABLE IF NOT EXISTS social_accounts ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  person_id INTEGER NOT NULL REFERENCES person(id) ON DELETE CASCADE,"
        "  platform TEXT NOT NULL DEFAULT '',"
        "  handle_or_url TEXT DEFAULT ''"
        ")",

        "CREATE TABLE IF NOT EXISTS emails ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  person_id INTEGER NOT NULL REFERENCES person(id) ON DELETE CASCADE,"
        "  address TEXT NOT NULL DEFAULT '',"
        "  label TEXT DEFAULT ''"
        ")",

        "CREATE TABLE IF NOT EXISTS person_events ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  person_id INTEGER NOT NULL REFERENCES person(id) ON DELETE CASCADE,"
        "  title TEXT NOT NULL DEFAULT '',"
        "  date TEXT,"
        "  recurs_yearly INTEGER DEFAULT 0,"
        "  remind_days_before INTEGER DEFAULT 3"
        ")",

        // Seed default groups
        "INSERT OR IGNORE INTO groups (name) VALUES ('Работа')",
        "INSERT OR IGNORE INTO groups (name) VALUES ('Друзья')",
        "INSERT OR IGNORE INTO groups (name) VALUES ('Случайные')",
        "INSERT OR IGNORE INTO groups (name) VALUES ('Родственники')",
    };

    m_db.transaction();
    for (const auto &sql : statements) {
        if (!q.exec(sql)) {
            qWarning("Migration v1 failed: %s\nSQL: %s",
                     qPrintable(q.lastError().text()),
                     qPrintable(sql));
            m_db.rollback();
            return false;
        }
    }
    m_db.commit();
    return true;
}

bool MigrationManager::migration_v2()
{
    QSqlQuery q(m_db);
    QStringList statements = {
        "ALTER TABLE groups ADD COLUMN sort_order INTEGER DEFAULT 100",
        "UPDATE groups SET sort_order = 1 WHERE name = 'Родственники'",
        "UPDATE groups SET sort_order = 2 WHERE name = 'Друзья'",
        "UPDATE groups SET sort_order = 3 WHERE name = 'Работа'",
        "UPDATE groups SET sort_order = 4 WHERE name = 'Случайные'",
    };

    m_db.transaction();
    for (const auto &sql : statements) {
        if (!q.exec(sql)) {
            qWarning("Migration v2 failed: %s\nSQL: %s",
                     qPrintable(q.lastError().text()), qPrintable(sql));
            m_db.rollback();
            return false;
        }
    }
    m_db.commit();
    return true;
}

bool MigrationManager::migration_v3()
{
    QSqlQuery q(m_db);
    QStringList statements = {
        "CREATE TABLE IF NOT EXISTS phone_numbers ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  person_id INTEGER NOT NULL REFERENCES person(id) ON DELETE CASCADE,"
        "  number TEXT NOT NULL DEFAULT '',"
        "  label TEXT DEFAULT ''"
        ")",

        "CREATE TABLE IF NOT EXISTS person_notes ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  person_id INTEGER NOT NULL REFERENCES person(id) ON DELETE CASCADE,"
        "  text TEXT NOT NULL DEFAULT '',"
        "  created_at TEXT NOT NULL DEFAULT (datetime('now'))"
        ")",

        "CREATE TABLE IF NOT EXISTS person_files ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  person_id INTEGER NOT NULL REFERENCES person(id) ON DELETE CASCADE,"
        "  file_name TEXT NOT NULL DEFAULT '',"
        "  file_path TEXT NOT NULL DEFAULT '',"
        "  added_at TEXT NOT NULL DEFAULT (datetime('now'))"
        ")",

        "ALTER TABLE person_profile ADD COLUMN gender TEXT DEFAULT ''",
        "ALTER TABLE person_profile ADD COLUMN gift_ideas TEXT DEFAULT ''",
        "ALTER TABLE person_profile ADD COLUMN clothing_size TEXT DEFAULT ''",
        "ALTER TABLE person_profile ADD COLUMN favorite_food TEXT DEFAULT ''",
    };

    m_db.transaction();
    for (const auto &sql : statements) {
        if (!q.exec(sql)) {
            qWarning("Migration v3 failed: %s\nSQL: %s",
                     qPrintable(q.lastError().text()), qPrintable(sql));
            m_db.rollback();
            return false;
        }
    }
    m_db.commit();
    return true;
}
