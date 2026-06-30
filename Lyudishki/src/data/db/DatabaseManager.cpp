#include "DatabaseManager.h"
#include <QSqlError>
#include <QUuid>

DatabaseManager::DatabaseManager(const QString &dbPath)
    : m_connectionName(QUuid::createUuid().toString())
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qWarning("DatabaseManager: failed to open DB: %s",
                 qPrintable(m_db.lastError().text()));
    }
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
        m_db.close();
    QSqlDatabase::removeDatabase(m_connectionName);
}

QSqlDatabase &DatabaseManager::database()
{
    return m_db;
}

bool DatabaseManager::isOpen() const
{
    return m_db.isOpen();
}
