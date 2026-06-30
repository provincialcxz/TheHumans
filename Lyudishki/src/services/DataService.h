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

    // Imports people from a JSON export. Entries whose (lastName, firstName,
    // birthDate) exactly match an already-existing person are skipped rather
    // than added again — re-importing the same export file is a common
    // "restore/merge from another device" scenario and shouldn't silently
    // duplicate every contact. Returns the number actually added;
    // skippedCount, if provided, receives the number of exact-match skips.
    int importPeopleJson(const QString &filePath, int *skippedCount = nullptr);

    static bool createBackup(const QString &dbPath, const QString &backupDir);
    static bool restoreBackup(const QString &backupFile, const QString &dbPath);

private:
    std::shared_ptr<IPersonRepository> m_personRepo;
    std::shared_ptr<IGroupRepository> m_groupRepo;
};
