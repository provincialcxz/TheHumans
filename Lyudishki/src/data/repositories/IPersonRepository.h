#pragma once

#include <QVector>
#include <QMap>
#include "domain/Person.h"
#include "domain/PersonProfile.h"
#include "domain/SocialAccount.h"
#include "domain/Email.h"
#include "domain/PersonEvent.h"
#include "domain/PhoneNumber.h"
#include "domain/PersonNote.h"
#include "domain/PersonFile.h"
#include "domain/PersonPhoto.h"
#include "domain/PersonDocument.h"
#include "domain/Tag.h"
#include "domain/RelationshipStatusChange.h"

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

    // History of PersonProfile::relationshipStatus changes over time —
    // recorded automatically by saveProfile() whenever the value differs
    // from what's currently stored, oldest first.
    virtual QVector<RelationshipStatusChange> getRelationshipStatusHistory(int personId) = 0;

    // Lightweight side-load for list sorting: avoids pulling the full
    // (lazy-loaded) profile per person just to sort by reliability.
    virtual QMap<int, QString> getReliabilityMap() = 0;

    // Full-text search across name/note/phone plus the otherwise-lazy data
    // (profile fields, emails, extra phones, notes) via the person_search
    // FTS5 index — without loading any of that into memory per person.
    virtual QVector<int> searchPersonIds(const QString &query) = 0;

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
    virtual PersonFile getFileById(int id) = 0;
    virtual int addFile(const PersonFile &file) = 0;
    virtual bool removeFile(int id) = 0;

    // Photo gallery (additional photos beyond Person::photoPath, the avatar)
    virtual QVector<PersonPhoto> getPhotos(int personId) = 0;
    virtual PersonPhoto getPhotoById(int id) = 0;
    virtual int addPhoto(const PersonPhoto &photo) = 0;
    virtual bool removePhoto(int id) = 0;

    // Documents (passport, SNILS, bank details, etc.)
    virtual QVector<PersonDocument> getDocuments(int personId) = 0;
    virtual int addDocument(const PersonDocument &document) = 0;
    virtual bool removeDocument(int id) = 0;

    // Tags (independent of groups: a person can have any number of tags,
    // shared by name across people)
    virtual QVector<Tag> getAllTags() = 0;
    virtual QVector<Tag> getTagsForPerson(int personId) = 0;
    virtual int addTagToPerson(int personId, const QString &tagName) = 0;
    virtual bool removeTagFromPerson(int personId, int tagId) = 0;

    // Events
    virtual QVector<PersonEvent> getEvents(int personId) = 0;
    virtual QVector<PersonEvent> getAllEvents() = 0;
    virtual int addEvent(const PersonEvent &event) = 0;
    virtual bool updateEvent(const PersonEvent &event) = 0;
    virtual bool removeEvent(int id) = 0;
};
