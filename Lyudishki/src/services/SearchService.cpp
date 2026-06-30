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
    const QString lower = query.toLower();
    for (const auto &p : m_repo->getAll()) {
        QString fullName = p.lastName + " " + p.firstName + " " + p.patronymic;
        if (fullName.toLower().contains(lower) ||
            p.note.toLower().contains(lower) ||
            p.phone.contains(lower)) {
            result.append(p);
        }
    }
    return result;
}
