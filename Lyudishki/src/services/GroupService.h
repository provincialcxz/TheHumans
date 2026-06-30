#pragma once

#include "data/repositories/IGroupRepository.h"
#include <QVector>
#include <memory>

class GroupService {
public:
    explicit GroupService(std::shared_ptr<IGroupRepository> repo);

    QVector<Group> getAllGroups();
    Group getGroup(int id);
    int addGroup(const QString &name);
    bool renameGroup(int id, const QString &newName);
    bool removeGroup(int id);

private:
    std::shared_ptr<IGroupRepository> m_repo;
};
