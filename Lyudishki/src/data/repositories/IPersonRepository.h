#pragma once

#include <QVector>
#include "domain/Person.h"
#include "domain/PersonProfile.h"
#include "domain/SocialAccount.h"
#include "domain/Email.h"
#include "domain/PersonEvent.h"
#include "domain/PhoneNumber.h"
#include "domain/PersonNote.h"
#include "domain/PersonFile.h"

class IPersonRepository {
public:
    virtual ~IPersonRepository() = default;

    virtual QVector<Person> getAll() = 0;
    virtual QVector<Person> getByGroupId(int groupId) = 0;
    virtual Person getById(int id) = 0;
    virtual int add(const Person &person) = 0;
    virtual bool update(const Person &person) = 0;
    virtual bool remove(int id) = 0;

    // Profile (lazy)
    virtual PersonProfile loadProfile(int personId) = 0;
    virtual bool saveProfile(const PersonProfile &profile) = 0;

    // Social accounts
    virtual QVector<SocialAccount> getSocialAccounts(int personId) = 0;
    virtual int addSocialAccount(const SocialAccount &account) = 0;
    virtual bool removeSocialAccount(int id) = 0;

    // Emails
    virtual QVector<Email> getEmails(int personId) = 0;
    virtual int addEmail(const Email &email) = 0;
    virtual bool removeEmail(int id) = 0;

    // Phone numbers (additional, beyond Person::phone)
    virtual QVector<PhoneNumber> getPhoneNumbers(int personId) = 0;
    virtual int addPhoneNumber(const PhoneNumber &phone) = 0;
    virtual bool removePhoneNumber(int id) = 0;

    // Notes (timestamped, multiple per person)
    virtual QVector<PersonNote> getNotes(int personId) = 0;
    virtual int addNote(const PersonNote &note) = 0;
    virtual bool removeNote(int id) = 0;

    // File attachments
    virtual QVector<PersonFile> getFiles(int personId) = 0;
    virtual int addFile(const PersonFile &file) = 0;
    virtual bool removeFile(int id) = 0;

    // Events
    virtual QVector<PersonEvent> getEvents(int personId) = 0;
    virtual QVector<PersonEvent> getAllEvents() = 0;
    virtual int addEvent(const PersonEvent &event) = 0;
    virtual bool updateEvent(const PersonEvent &event) = 0;
    virtual bool removeEvent(int id) = 0;
};
