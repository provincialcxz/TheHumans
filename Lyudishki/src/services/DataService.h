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

    // Imports people from an RFC 6350 vCard (.vcf) file — the standard
    // "export contacts" format from phone address books. Reads name, phone
    // numbers, emails and birthday per vCard; the first TEL becomes
    // Person::phone, any further TEL/EMAIL entries become additional phone
    // numbers/emails. Uses the same (lastName, firstName, birthDate) dedup
    // as importPeopleJson. Returns the number of people added, or -1 if the
    // file couldn't be read.
    int importPeopleVCard(const QString &filePath, int *skippedCount = nullptr);

    static bool createBackup(const QString &dbPath, const QString &backupDir);
    static bool restoreBackup(const QString &backupFile, const QString &dbPath);

private:
    std::shared_ptr<IPersonRepository> m_personRepo;
    std::shared_ptr<IGroupRepository> m_groupRepo;
};
