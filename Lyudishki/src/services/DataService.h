#pragma once

#include "data/repositories/IPersonRepository.h"
#include "data/repositories/IGroupRepository.h"
#include <QString>
#include <memory>

class DataService {
public:
    DataService(std::shared_ptr<IPersonRepository> personRepo,
                std::shared_ptr<IGroupRepository> groupRepo);

    bool exportPeopleJson(const QString &filePath);
    int importPeopleJson(const QString &filePath);

    static bool createBackup(const QString &dbPath, const QString &backupDir);
    static bool restoreBackup(const QString &backupFile, const QString &dbPath);

private:
    std::shared_ptr<IPersonRepository> m_personRepo;
    std::shared_ptr<IGroupRepository> m_groupRepo;
};
