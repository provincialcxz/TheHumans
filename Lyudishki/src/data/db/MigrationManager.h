#pragma once

#include <QSqlDatabase>

class MigrationManager {
public:
    explicit MigrationManager(QSqlDatabase &db);
    bool migrate();

private:
    QSqlDatabase &m_db;
    int currentVersion();
    void setVersion(int version);
    bool runMigration(int version);
    bool migration_v1();
    bool migration_v2();
    bool migration_v3();
    bool migration_v4();
    bool migration_v5();
    bool migration_v6();
    bool migration_v7();
    bool migration_v8();
    bool migration_v9();
    bool migration_v10();
    bool migration_v11();
};
