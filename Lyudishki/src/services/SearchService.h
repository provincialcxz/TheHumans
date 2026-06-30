#pragma once

#include "data/repositories/IPersonRepository.h"
#include <QString>
#include <QVector>
#include <memory>

class SearchService {
public:
    explicit SearchService(std::shared_ptr<IPersonRepository> repo);

    QVector<Person> search(const QString &query);

private:
    std::shared_ptr<IPersonRepository> m_repo;
};
