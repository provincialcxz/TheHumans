#pragma once

#include <QString>
#include <QSqlDatabase>

class DatabaseManager {
public:
    explicit DatabaseManager(const QString &dbPath);
    ~DatabaseManager();

    QSqlDatabase &database();
    bool isOpen() const;

private:
    QSqlDatabase m_db;
    QString m_connectionName;
};
