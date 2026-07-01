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
    const int targetVersion = 9;
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
    case 4: return migration_v4();
    case 5: return migration_v5();
    case 6: return migration_v6();
    case 7: return migration_v7();
    case 8: return migration_v8();
    case 9: return migration_v9();
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

bool MigrationManager::migration_v4()
{
    QSqlQuery q(m_db);

    // Concatenation of every searchable person_profile column, reused by the
    // AFTER INSERT trigger (fires on every save — the app always writes
    // profiles via INSERT OR REPLACE) and by the one-time backfill below.
    static const QString kProfileConcat =
        "COALESCE(character,'') || ' ' || COALESCE(interests,'') || ' ' || COALESCE(humor_and_triggers,'') || ' ' || "
        "COALESCE(political_views,'') || ' ' || COALESCE(religious_views,'') || ' ' || COALESCE(languages,'') || ' ' || COALESCE(nicknames,'') || ' ' || "
        "COALESCE(family,'') || ' ' || COALESCE(relationship_status,'') || ' ' || COALESCE(mutual_acquaintances,'') || ' ' || COALESCE(how_and_when_met,'') || ' ' || "
        "COALESCE(career_track,'') || ' ' || COALESCE(education,'') || ' ' || COALESCE(skills,'') || ' ' || COALESCE(work_schedule,'') || ' ' || COALESCE(work_contacts,'') || ' ' || "
        "COALESCE(platform_accounts,'') || ' ' || COALESCE(media_consumption,'') || ' ' || "
        "COALESCE(property,'') || ' ' || COALESCE(favorite_places,'') || ' ' || COALESCE(pets,'') || ' ' || "
        "COALESCE(health_and_allergies,'') || ' ' || "
        "COALESCE(reliability,'') || ' ' || COALESCE(info_freshness,'') || ' ' || "
        "COALESCE(gender,'') || ' ' || COALESCE(gift_ideas,'') || ' ' || COALESCE(clothing_size,'') || ' ' || COALESCE(favorite_food,'') || ' ' || "
        "COALESCE(additional_notes,'')";

    QStringList statements = {
        "CREATE VIRTUAL TABLE IF NOT EXISTS person_search USING fts5("
        "  person_id UNINDEXED,"
        "  full_name,"
        "  note,"
        "  phone,"
        "  emails_text,"
        "  phones_text,"
        "  notes_text,"
        "  profile_text,"
        "  tokenize = 'unicode61'"
        ")",

        // --- person: owns the row's lifecycle in person_search ---
        // COALESCE every concatenated column: the app frequently leaves
        // optional fields (e.g. patronymic) as a null QString, and SQLite's
        // '||' makes the *whole* expression NULL if any operand is NULL.
        "CREATE TRIGGER IF NOT EXISTS trg_search_person_ai AFTER INSERT ON person BEGIN "
        "  INSERT INTO person_search(person_id, full_name, note, phone, emails_text, phones_text, notes_text, profile_text) "
        "  VALUES (new.id, COALESCE(new.last_name,'') || ' ' || COALESCE(new.first_name,'') || ' ' || COALESCE(new.patronymic,''), COALESCE(new.note,''), COALESCE(new.phone,''), '', '', '', ''); "
        "END",

        "CREATE TRIGGER IF NOT EXISTS trg_search_person_au AFTER UPDATE ON person BEGIN "
        "  UPDATE person_search SET "
        "    full_name = COALESCE(new.last_name,'') || ' ' || COALESCE(new.first_name,'') || ' ' || COALESCE(new.patronymic,''), "
        "    note = COALESCE(new.note,''), "
        "    phone = COALESCE(new.phone,'') "
        "  WHERE person_id = new.id; "
        "END",

        "CREATE TRIGGER IF NOT EXISTS trg_search_person_ad AFTER DELETE ON person BEGIN "
        "  DELETE FROM person_search WHERE person_id = old.id; "
        "END",

        // --- person_profile: saved via INSERT OR REPLACE, so AFTER INSERT covers both create and edit ---
        "CREATE TRIGGER IF NOT EXISTS trg_search_profile_ai AFTER INSERT ON person_profile BEGIN "
        "  UPDATE person_search SET profile_text = ("
        "    SELECT " + kProfileConcat + " FROM person_profile WHERE person_id = new.person_id"
        "  ) WHERE person_id = new.person_id; "
        "END",

        // --- emails / phone_numbers / person_notes: 1-to-many, re-aggregate on every change ---
        "CREATE TRIGGER IF NOT EXISTS trg_search_email_ai AFTER INSERT ON emails BEGIN "
        "  UPDATE person_search SET emails_text = ("
        "    SELECT COALESCE(group_concat(address || ' ' || COALESCE(label,''), ' '), '') FROM emails WHERE person_id = new.person_id"
        "  ) WHERE person_id = new.person_id; "
        "END",

        "CREATE TRIGGER IF NOT EXISTS trg_search_email_ad AFTER DELETE ON emails BEGIN "
        "  UPDATE person_search SET emails_text = ("
        "    SELECT COALESCE(group_concat(address || ' ' || COALESCE(label,''), ' '), '') FROM emails WHERE person_id = old.person_id"
        "  ) WHERE person_id = old.person_id; "
        "END",

        "CREATE TRIGGER IF NOT EXISTS trg_search_phone_ai AFTER INSERT ON phone_numbers BEGIN "
        "  UPDATE person_search SET phones_text = ("
        "    SELECT COALESCE(group_concat(number || ' ' || COALESCE(label,''), ' '), '') FROM phone_numbers WHERE person_id = new.person_id"
        "  ) WHERE person_id = new.person_id; "
        "END",

        "CREATE TRIGGER IF NOT EXISTS trg_search_phone_ad AFTER DELETE ON phone_numbers BEGIN "
        "  UPDATE person_search SET phones_text = ("
        "    SELECT COALESCE(group_concat(number || ' ' || COALESCE(label,''), ' '), '') FROM phone_numbers WHERE person_id = old.person_id"
        "  ) WHERE person_id = old.person_id; "
        "END",

        "CREATE TRIGGER IF NOT EXISTS trg_search_note_ai AFTER INSERT ON person_notes BEGIN "
        "  UPDATE person_search SET notes_text = ("
        "    SELECT COALESCE(group_concat(text, ' '), '') FROM person_notes WHERE person_id = new.person_id"
        "  ) WHERE person_id = new.person_id; "
        "END",

        "CREATE TRIGGER IF NOT EXISTS trg_search_note_ad AFTER DELETE ON person_notes BEGIN "
        "  UPDATE person_search SET notes_text = ("
        "    SELECT COALESCE(group_concat(text, ' '), '') FROM person_notes WHERE person_id = old.person_id"
        "  ) WHERE person_id = old.person_id; "
        "END",

        // --- one-time backfill for people that already existed before this migration ---
        "INSERT INTO person_search(person_id, full_name, note, phone, emails_text, phones_text, notes_text, profile_text) "
        "SELECT p.id, "
        "  COALESCE(p.last_name,'') || ' ' || COALESCE(p.first_name,'') || ' ' || COALESCE(p.patronymic,''), "
        "  COALESCE(p.note,''), COALESCE(p.phone,''), "
        "  COALESCE((SELECT group_concat(address || ' ' || COALESCE(label,''), ' ') FROM emails WHERE person_id = p.id), ''), "
        "  COALESCE((SELECT group_concat(number || ' ' || COALESCE(label,''), ' ') FROM phone_numbers WHERE person_id = p.id), ''), "
        "  COALESCE((SELECT group_concat(text, ' ') FROM person_notes WHERE person_id = p.id), ''), "
        "  COALESCE((SELECT " + kProfileConcat + " FROM person_profile WHERE person_id = p.id), '') "
        "FROM person p "
        "WHERE NOT EXISTS (SELECT 1 FROM person_search ps WHERE ps.person_id = p.id)",
    };

    m_db.transaction();
    for (const auto &sql : statements) {
        if (!q.exec(sql)) {
            qWarning("Migration v4 failed: %s\nSQL: %s",
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

bool MigrationManager::migration_v5()
{
    QSqlQuery q(m_db);
    QStringList statements = {
        "CREATE TABLE IF NOT EXISTS person_documents ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  person_id INTEGER NOT NULL REFERENCES person(id) ON DELETE CASCADE,"
        "  type TEXT NOT NULL DEFAULT '',"
        "  value TEXT NOT NULL DEFAULT '',"
        "  note TEXT DEFAULT ''"
        ")",
    };

    m_db.transaction();
    for (const auto &sql : statements) {
        if (!q.exec(sql)) {
            qWarning("Migration v5 failed: %s\nSQL: %s",
                     qPrintable(q.lastError().text()), qPrintable(sql));
            m_db.rollback();
            return false;
        }
    }
    m_db.commit();
    return true;
}

bool MigrationManager::migration_v6()
{
    QSqlQuery q(m_db);
    QStringList statements = {
        "CREATE TABLE IF NOT EXISTS tags ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL UNIQUE"
        ")",

        "CREATE TABLE IF NOT EXISTS person_tags ("
        "  person_id INTEGER NOT NULL REFERENCES person(id) ON DELETE CASCADE,"
        "  tag_id INTEGER NOT NULL REFERENCES tags(id) ON DELETE CASCADE,"
        "  PRIMARY KEY (person_id, tag_id)"
        ")",
    };

    m_db.transaction();
    for (const auto &sql : statements) {
        if (!q.exec(sql)) {
            qWarning("Migration v6 failed: %s\nSQL: %s",
                     qPrintable(q.lastError().text()), qPrintable(sql));
            m_db.rollback();
            return false;
        }
    }
    m_db.commit();
    return true;
}

bool MigrationManager::migration_v7()
{
    QSqlQuery q(m_db);
    QStringList statements = {
        "ALTER TABLE person ADD COLUMN met_in_person INTEGER DEFAULT 0",
    };

    m_db.transaction();
    for (const auto &sql : statements) {
        if (!q.exec(sql)) {
            qWarning("Migration v7 failed: %s\nSQL: %s",
                     qPrintable(q.lastError().text()), qPrintable(sql));
            m_db.rollback();
            return false;
        }
    }
    m_db.commit();
    return true;
}

bool MigrationManager::migration_v8()
{
    QSqlQuery q(m_db);
    QStringList statements = {
        "ALTER TABLE person ADD COLUMN last_contact_date TEXT",
    };

    m_db.transaction();
    for (const auto &sql : statements) {
        if (!q.exec(sql)) {
            qWarning("Migration v8 failed: %s\nSQL: %s",
                     qPrintable(q.lastError().text()), qPrintable(sql));
            m_db.rollback();
            return false;
        }
    }
    m_db.commit();
    return true;
}

bool MigrationManager::migration_v9()
{
    QSqlQuery q(m_db);
    QStringList statements = {
        "CREATE TABLE IF NOT EXISTS person_photos ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  person_id INTEGER NOT NULL REFERENCES person(id) ON DELETE CASCADE,"
        "  file_path TEXT NOT NULL DEFAULT '',"
        "  added_at TEXT NOT NULL DEFAULT (datetime('now'))"
        ")",
    };

    m_db.transaction();
    for (const auto &sql : statements) {
        if (!q.exec(sql)) {
            qWarning("Migration v9 failed: %s\nSQL: %s",
                     qPrintable(q.lastError().text()), qPrintable(sql));
            m_db.rollback();
            return false;
        }
    }
    m_db.commit();
    return true;
}
