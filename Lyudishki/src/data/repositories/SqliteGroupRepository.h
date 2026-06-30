#pragma once

#include "IGroupRepository.h"
#include <QSqlDatabase>

class SqliteGroupRepository : public IGroupRepository {
public:
    explicit SqliteGroupRepository(QSqlDatabase &db);

    QVector<Group> getAll() override;
    Group getById(int id) override;
    int add(const Group &group) override;
    bool update(const Group &group) override;
    bool remove(int id) override;

private:
    QSqlDatabase &m_db;
};
