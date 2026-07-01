#pragma once

#include "data/repositories/IPersonRepository.h"
#include <QVector>
#include <QString>
#include <memory>

class PeopleService {
public:
    explicit PeopleService(std::shared_ptr<IPersonRepository> repo,
                           const QString &attachmentsDir = {});

    QVector<Person> getAllPeople();
    QVector<Person> getPeopleInGroup(int groupId);
    Person getPerson(int id);
    int addPerson(const Person &person);
    bool updatePerson(const Person &person);
    bool removePerson(int id);

    // Copies an externally-chosen photo into managed app storage so it
    // survives the user moving/deleting the original file. Returns the
    // new managed path (or the original path if no attachments dir is set).
    QString importPhoto(int personId, const QString &sourceFilePath);

    PersonProfile loadProfile(int personId);
    bool saveProfile(const PersonProfile &profile);
    QMap<int, QString> getReliabilityMap();

    QVector<SocialAccount> getSocialAccounts(int personId);
    int addSocialAccount(const SocialAccount &account);
    bool removeSocialAccount(int id);

    QVector<Email> getEmails(int personId);
    int addEmail(const Email &email);
    bool removeEmail(int id);

    QVector<PhoneNumber> getPhoneNumbers(int personId);
    int addPhoneNumber(const PhoneNumber &phone);
    bool removePhoneNumber(int id);

    QVector<PersonNote> getNotes(int personId);
    int addNote(int personId, const QString &text);
    bool removeNote(int id);

    QVector<PersonFile> getFiles(int personId);
    int attachFile(int personId, const QString &sourceFilePath);
    bool removeFile(int id);

    QVector<PersonDocument> getDocuments(int personId);
    int addDocument(const PersonDocument &document);
    bool removeDocument(int id);

    QVector<PersonEvent> getEvents(int personId);
    QVector<PersonEvent> getAllEvents();
    int addEvent(const PersonEvent &event);
    bool updateEvent(const PersonEvent &event);
    bool removeEvent(int id);

private:
    std::shared_ptr<IPersonRepository> m_repo;
    QString m_attachmentsDir;
};
