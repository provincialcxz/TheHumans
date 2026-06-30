#include "SearchService.h"

SearchService::SearchService(std::shared_ptr<IPersonRepository> repo)
    : m_repo(std::move(repo))
{
}

QVector<Person> SearchService::search(const QString &query)
{
    if (query.trimmed().isEmpty())
        return m_repo->getAll();

    QVector<Person> result;
    for (int id : m_repo->searchPersonIds(query))
        result.append(m_repo->getById(id));
    return result;
}

QVector<int> SearchService::searchPersonIds(const QString &query)
{
    return m_repo->searchPersonIds(query);
}
