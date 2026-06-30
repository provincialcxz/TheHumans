#include "SqliteGroupRepository.h"
#include <QSqlQuery>
#include <QSqlError>

SqliteGroupRepository::SqliteGroupRepository(QSqlDatabase &db)
    : m_db(db)
{
}

QVector<Group> SqliteGroupRepository::getAll()
{
    QVector<Group> result;
    QSqlQuery q(m_db);
    q.exec("SELECT id, name, sort_order FROM groups ORDER BY sort_order, id");
    while (q.next()) {
        Group g;
        g.id = q.value(0).toInt();
        g.name = q.value(1).toString();
        g.sortOrder = q.value(2).toInt();
        result.append(g);
    }
    return result;
}

Group SqliteGroupRepository::getById(int id)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT id, name, sort_order FROM groups WHERE id = ?");
    q.addBindValue(id);
    q.exec();
    Group g;
    if (q.next()) {
        g.id = q.value(0).toInt();
        g.name = q.value(1).toString();
        g.sortOrder = q.value(2).toInt();
    }
    return g;
}

int SqliteGroupRepository::add(const Group &group)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO groups (name) VALUES (?)");
    q.addBindValue(group.name);
    if (!q.exec()) {
        qWarning("SqliteGroupRepository::add: %s", qPrintable(q.lastError().text()));
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool SqliteGroupRepository::update(const Group &group)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE groups SET name = ? WHERE id = ?");
    q.addBindValue(group.name);
    q.addBindValue(group.id);
    if (!q.exec()) {
        qWarning("SqliteGroupRepository::update: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool SqliteGroupRepository::remove(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM groups WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning("SqliteGroupRepository::remove: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return q.numRowsAffected() > 0;
}
