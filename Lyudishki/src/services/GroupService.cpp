#include "GroupService.h"

GroupService::GroupService(std::shared_ptr<IGroupRepository> repo)
    : m_repo(std::move(repo))
{
}

QVector<Group> GroupService::getAllGroups()
{
    return m_repo->getAll();
}

Group GroupService::getGroup(int id)
{
    return m_repo->getById(id);
}

int GroupService::addGroup(const QString &name)
{
    Group g;
    g.name = name;
    return m_repo->add(g);
}

bool GroupService::renameGroup(int id, const QString &newName)
{
    Group g;
    g.id = id;
    g.name = newName;
    return m_repo->update(g);
}

bool GroupService::removeGroup(int id)
{
    return m_repo->remove(id);
}
