#pragma once

#include "IPersonRepository.h"
#include <QSqlDatabase>

class SqlitePersonRepository : public IPersonRepository {
public:
    explicit SqlitePersonRepository(QSqlDatabase &db);

    QVector<Person> getAll() override;
    QVector<Person> getByGroupId(int groupId) override;
    Person getById(int id) override;
    int add(const Person &person) override;
    bool update(const Person &person) override;
    bool remove(int id) override;

    PersonProfile loadProfile(int personId) override;
    bool saveProfile(const PersonProfile &profile) override;
    QMap<int, QString> getReliabilityMap() override;
    QVector<int> searchPersonIds(const QString &query) override;

    QVector<SocialAccount> getSocialAccounts(int personId) override;
    int addSocialAccount(const SocialAccount &account) override;
    bool removeSocialAccount(int id) override;

    QVector<Email> getEmails(int personId) override;
    int addEmail(const Email &email) override;
    bool removeEmail(int id) override;

    QVector<PhoneNumber> getPhoneNumbers(int personId) override;
    int addPhoneNumber(const PhoneNumber &phone) override;
    bool removePhoneNumber(int id) override;

    QVector<PersonNote> getNotes(int personId) override;
    int addNote(const PersonNote &note) override;
    bool removeNote(int id) override;

    QVector<PersonFile> getFiles(int personId) override;
    PersonFile getFileById(int id) override;
    int addFile(const PersonFile &file) override;
    bool removeFile(int id) override;

    QVector<PersonDocument> getDocuments(int personId) override;
    int addDocument(const PersonDocument &document) override;
    bool removeDocument(int id) override;

    QVector<Tag> getAllTags() override;
    QVector<Tag> getTagsForPerson(int personId) override;
    int addTagToPerson(int personId, const QString &tagName) override;
    bool removeTagFromPerson(int personId, int tagId) override;

    QVector<PersonEvent> getEvents(int personId) override;
    QVector<PersonEvent> getAllEvents() override;
    int addEvent(const PersonEvent &event) override;
    bool updateEvent(const PersonEvent &event) override;
    bool removeEvent(int id) override;

private:
    QSqlDatabase &m_db;
    Person personFromQuery(QSqlQuery &q);
};
