#pragma once

#include "data/repositories/IPersonRepository.h"
#include <QString>
#include <QVector>
#include <memory>

class SearchService {
public:
    explicit SearchService(std::shared_ptr<IPersonRepository> repo);

    // Full-text search: name/note/main phone plus profile fields, emails,
    // extra phone numbers, and timestamped notes — backed by the person_search
    // FTS5 index, so it doesn't need to load any of that lazy data itself.
    QVector<Person> search(const QString &query);

    // Same search, but returns just the matching ids — for callers (like the
    // list view's debounced "search everywhere" merge) that only need
    // membership and already have the Person data loaded.
    QVector<int> searchPersonIds(const QString &query);

private:
    std::shared_ptr<IPersonRepository> m_repo;
};
