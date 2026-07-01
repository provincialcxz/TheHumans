#include "SqlitePersonRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>

SqlitePersonRepository::SqlitePersonRepository(QSqlDatabase &db)
    : m_db(db)
{
}

Person SqlitePersonRepository::personFromQuery(QSqlQuery &q)
{
    Person p;
    p.id = q.value("id").toInt();
    p.groupId = q.value("group_id").toInt();
    p.photoPath = q.value("photo_path").toString();
    p.lastName = q.value("last_name").toString();
    p.firstName = q.value("first_name").toString();
    p.patronymic = q.value("patronymic").toString();
    p.birthDate = QDate::fromString(q.value("birth_date").toString(), Qt::ISODate);
    p.phone = q.value("phone").toString();
    p.primaryContactMethod = q.value("primary_contact_method").toString();
    p.address = q.value("address").toString();
    p.habits = q.value("habits").toString();
    p.hobbies = q.value("hobbies").toString();
    p.note = q.value("note").toString();
    p.metInPerson = q.value("met_in_person").toInt() != 0;
    p.lastContactDate = QDate::fromString(q.value("last_contact_date").toString(), Qt::ISODate);
    p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    return p;
}

QVector<Person> SqlitePersonRepository::getAll()
{
    QVector<Person> result;
    QSqlQuery q(m_db);
    if (!q.exec("SELECT * FROM person ORDER BY last_name, first_name"))
        qWarning("SqlitePersonRepository::getAll: %s", qPrintable(q.lastError().text()));
    while (q.next())
        result.append(personFromQuery(q));
    return result;
}

QVector<Person> SqlitePersonRepository::getByGroupId(int groupId)
{
    QVector<Person> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM person WHERE group_id = ? ORDER BY last_name, first_name");
    q.addBindValue(groupId);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getByGroupId: %s", qPrintable(q.lastError().text()));
    while (q.next())
        result.append(personFromQuery(q));
    return result;
}

Person SqlitePersonRepository::getById(int id)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM person WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getById: %s", qPrintable(q.lastError().text()));
    if (q.next())
        return personFromQuery(q);
    return {};
}

int SqlitePersonRepository::add(const Person &person)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO person (group_id, photo_path, last_name, first_name, patronymic, "
              "birth_date, phone, primary_contact_method, address, habits, hobbies, note, met_in_person, last_contact_date) "
              "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    q.addBindValue(person.groupId);
    q.addBindValue(person.photoPath);
    q.addBindValue(person.lastName);
    q.addBindValue(person.firstName);
    q.addBindValue(person.patronymic);
    q.addBindValue(person.birthDate.isValid() ? person.birthDate.toString(Qt::ISODate) : QVariant());
    q.addBindValue(person.phone);
    q.addBindValue(person.primaryContactMethod);
    q.addBindValue(person.address);
    q.addBindValue(person.habits);
    q.addBindValue(person.hobbies);
    q.addBindValue(person.note);
    q.addBindValue(person.metInPerson ? 1 : 0);
    q.addBindValue(person.lastContactDate.isValid() ? person.lastContactDate.toString(Qt::ISODate) : QVariant());
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::add: %s", qPrintable(q.lastError().text()));
        return -1;
    }

    int personId = q.lastInsertId().toInt();
    QSqlQuery pq(m_db);
    pq.prepare("INSERT INTO person_profile (person_id) VALUES (?)");
    pq.addBindValue(personId);
    if (!pq.exec())
        qWarning("SqlitePersonRepository::add (profile row): %s", qPrintable(pq.lastError().text()));
    return personId;
}

bool SqlitePersonRepository::update(const Person &person)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE person SET group_id=?, photo_path=?, last_name=?, first_name=?, "
              "patronymic=?, birth_date=?, phone=?, primary_contact_method=?, "
              "address=?, habits=?, hobbies=?, note=?, met_in_person=?, last_contact_date=? WHERE id=?");
    q.addBindValue(person.groupId);
    q.addBindValue(person.photoPath);
    q.addBindValue(person.lastName);
    q.addBindValue(person.firstName);
    q.addBindValue(person.patronymic);
    q.addBindValue(person.birthDate.isValid() ? person.birthDate.toString(Qt::ISODate) : QVariant());
    q.addBindValue(person.phone);
    q.addBindValue(person.primaryContactMethod);
    q.addBindValue(person.address);
    q.addBindValue(person.habits);
    q.addBindValue(person.hobbies);
    q.addBindValue(person.note);
    q.addBindValue(person.metInPerson ? 1 : 0);
    q.addBindValue(person.lastContactDate.isValid() ? person.lastContactDate.toString(Qt::ISODate) : QVariant());
    q.addBindValue(person.id);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::update: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool SqlitePersonRepository::remove(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM person WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::remove: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return q.numRowsAffected() > 0;
}

// --- Profile ---

PersonProfile SqlitePersonRepository::loadProfile(int personId)
{
    PersonProfile p;
    p.personId = personId;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM person_profile WHERE person_id = ?");
    q.addBindValue(personId);
    if (!q.exec())
        qWarning("SqlitePersonRepository::loadProfile: %s", qPrintable(q.lastError().text()));
    if (q.next()) {
        p.character = q.value("character").toString();
        p.interests = q.value("interests").toString();
        p.humorAndTriggers = q.value("humor_and_triggers").toString();
        p.politicalViews = q.value("political_views").toString();
        p.religiousViews = q.value("religious_views").toString();
        p.languages = q.value("languages").toString();
        p.nicknames = q.value("nicknames").toString();
        p.family = q.value("family").toString();
        p.relationshipStatus = q.value("relationship_status").toString();
        p.mutualAcquaintances = q.value("mutual_acquaintances").toString();
        p.howAndWhenMet = q.value("how_and_when_met").toString();
        p.careerTrack = q.value("career_track").toString();
        p.education = q.value("education").toString();
        p.skills = q.value("skills").toString();
        p.workSchedule = q.value("work_schedule").toString();
        p.workContacts = q.value("work_contacts").toString();
        p.platformAccounts = q.value("platform_accounts").toString();
        p.mediaConsumption = q.value("media_consumption").toString();
        p.property = q.value("property").toString();
        p.favoritePlaces = q.value("favorite_places").toString();
        p.pets = q.value("pets").toString();
        p.healthAndAllergies = q.value("health_and_allergies").toString();
        p.reliability = q.value("reliability").toString();
        p.infoFreshness = q.value("info_freshness").toString();
        p.additionalNotes = q.value("additional_notes").toString();
        p.gender = q.value("gender").toString();
        p.giftIdeas = q.value("gift_ideas").toString();
        p.clothingSize = q.value("clothing_size").toString();
        p.favoriteFood = q.value("favorite_food").toString();
    }
    return p;
}

bool SqlitePersonRepository::saveProfile(const PersonProfile &profile)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT OR REPLACE INTO person_profile "
              "(person_id, character, interests, humor_and_triggers, political_views, "
              "religious_views, languages, nicknames, family, relationship_status, "
              "mutual_acquaintances, how_and_when_met, career_track, education, skills, "
              "work_schedule, work_contacts, platform_accounts, media_consumption, "
              "property, favorite_places, pets, health_and_allergies, reliability, "
              "info_freshness, additional_notes, gender, gift_ideas, clothing_size, favorite_food) "
              "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(profile.personId);
    q.addBindValue(profile.character);
    q.addBindValue(profile.interests);
    q.addBindValue(profile.humorAndTriggers);
    q.addBindValue(profile.politicalViews);
    q.addBindValue(profile.religiousViews);
    q.addBindValue(profile.languages);
    q.addBindValue(profile.nicknames);
    q.addBindValue(profile.family);
    q.addBindValue(profile.relationshipStatus);
    q.addBindValue(profile.mutualAcquaintances);
    q.addBindValue(profile.howAndWhenMet);
    q.addBindValue(profile.careerTrack);
    q.addBindValue(profile.education);
    q.addBindValue(profile.skills);
    q.addBindValue(profile.workSchedule);
    q.addBindValue(profile.workContacts);
    q.addBindValue(profile.platformAccounts);
    q.addBindValue(profile.mediaConsumption);
    q.addBindValue(profile.property);
    q.addBindValue(profile.favoritePlaces);
    q.addBindValue(profile.pets);
    q.addBindValue(profile.healthAndAllergies);
    q.addBindValue(profile.reliability);
    q.addBindValue(profile.infoFreshness);
    q.addBindValue(profile.additionalNotes);
    q.addBindValue(profile.gender);
    q.addBindValue(profile.giftIdeas);
    q.addBindValue(profile.clothingSize);
    q.addBindValue(profile.favoriteFood);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::saveProfile: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return true;
}

QMap<int, QString> SqlitePersonRepository::getReliabilityMap()
{
    QMap<int, QString> result;
    QSqlQuery q(m_db);
    if (!q.exec("SELECT person_id, reliability FROM person_profile"))
        qWarning("SqlitePersonRepository::getReliabilityMap: %s", qPrintable(q.lastError().text()));
    while (q.next())
        result.insert(q.value(0).toInt(), q.value(1).toString());
    return result;
}

QVector<int> SqlitePersonRepository::searchPersonIds(const QString &query)
{
    QVector<int> result;
    QString trimmed = query.trimmed();
    if (trimmed.isEmpty())
        return result;

    // Build a safe FTS5 MATCH expression: each whitespace-separated token is
    // quoted as a literal string (so stray FTS5 syntax characters like
    // - " * : ( ) or the bareword operators AND/OR/NOT in the user's typed
    // text can't be interpreted as query syntax) with a trailing '*' for
    // prefix matching, ANDed together.
    QStringList tokens = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    QStringList quotedTokens;
    for (const QString &t : tokens) {
        QString escaped = t;
        escaped.replace("\"", "\"\"");
        quotedTokens << QString("\"%1\"*").arg(escaped);
    }
    if (quotedTokens.isEmpty())
        return result;
    QString matchExpr = quotedTokens.join(" AND ");

    QSqlQuery q(m_db);
    q.prepare("SELECT person_id FROM person_search WHERE person_search MATCH ? ORDER BY rank");
    q.addBindValue(matchExpr);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::searchPersonIds: %s", qPrintable(q.lastError().text()));
        return result;
    }
    while (q.next())
        result.append(q.value(0).toInt());
    return result;
}

// --- Social Accounts ---

QVector<SocialAccount> SqlitePersonRepository::getSocialAccounts(int personId)
{
    QVector<SocialAccount> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, person_id, platform, handle_or_url FROM social_accounts WHERE person_id = ?");
    q.addBindValue(personId);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getSocialAccounts: %s", qPrintable(q.lastError().text()));
    while (q.next()) {
        SocialAccount s;
        s.id = q.value(0).toInt();
        s.personId = q.value(1).toInt();
        s.platform = q.value(2).toString();
        s.handleOrUrl = q.value(3).toString();
        result.append(s);
    }
    return result;
}

int SqlitePersonRepository::addSocialAccount(const SocialAccount &account)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO social_accounts (person_id, platform, handle_or_url) VALUES (?,?,?)");
    q.addBindValue(account.personId);
    q.addBindValue(account.platform);
    q.addBindValue(account.handleOrUrl);
    if (!q.exec()) {
        qWarning("addSocialAccount: %s", qPrintable(q.lastError().text()));
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool SqlitePersonRepository::removeSocialAccount(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM social_accounts WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::removeSocialAccount: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return true;
}

// --- Emails ---

QVector<Email> SqlitePersonRepository::getEmails(int personId)
{
    QVector<Email> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, person_id, address, label FROM emails WHERE person_id = ?");
    q.addBindValue(personId);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getEmails: %s", qPrintable(q.lastError().text()));
    while (q.next()) {
        Email e;
        e.id = q.value(0).toInt();
        e.personId = q.value(1).toInt();
        e.address = q.value(2).toString();
        e.label = q.value(3).toString();
        result.append(e);
    }
    return result;
}

int SqlitePersonRepository::addEmail(const Email &email)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO emails (person_id, address, label) VALUES (?,?,?)");
    q.addBindValue(email.personId);
    q.addBindValue(email.address);
    q.addBindValue(email.label);
    if (!q.exec()) {
        qWarning("addEmail: %s", qPrintable(q.lastError().text()));
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool SqlitePersonRepository::removeEmail(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM emails WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::removeEmail: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return true;
}

// --- Photo gallery ---

QVector<PersonPhoto> SqlitePersonRepository::getPhotos(int personId)
{
    QVector<PersonPhoto> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, person_id, file_path, added_at FROM person_photos "
              "WHERE person_id = ? ORDER BY added_at DESC");
    q.addBindValue(personId);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getPhotos: %s", qPrintable(q.lastError().text()));
    while (q.next()) {
        PersonPhoto p;
        p.id = q.value(0).toInt();
        p.personId = q.value(1).toInt();
        p.filePath = q.value(2).toString();
        p.addedAt = QDateTime::fromString(q.value(3).toString(), Qt::ISODate);
        result.append(p);
    }
    return result;
}

PersonPhoto SqlitePersonRepository::getPhotoById(int id)
{
    PersonPhoto p;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, person_id, file_path, added_at FROM person_photos WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getPhotoById: %s", qPrintable(q.lastError().text()));
    if (q.next()) {
        p.id = q.value(0).toInt();
        p.personId = q.value(1).toInt();
        p.filePath = q.value(2).toString();
        p.addedAt = QDateTime::fromString(q.value(3).toString(), Qt::ISODate);
    }
    return p;
}

int SqlitePersonRepository::addPhoto(const PersonPhoto &photo)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO person_photos (person_id, file_path) VALUES (?,?)");
    q.addBindValue(photo.personId);
    q.addBindValue(photo.filePath);
    if (!q.exec()) {
        qWarning("addPhoto: %s", qPrintable(q.lastError().text()));
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool SqlitePersonRepository::removePhoto(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM person_photos WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::removePhoto: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return true;
}

// --- Documents ---

QVector<PersonDocument> SqlitePersonRepository::getDocuments(int personId)
{
    QVector<PersonDocument> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, person_id, type, value, note FROM person_documents WHERE person_id = ?");
    q.addBindValue(personId);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getDocuments: %s", qPrintable(q.lastError().text()));
    while (q.next()) {
        PersonDocument d;
        d.id = q.value(0).toInt();
        d.personId = q.value(1).toInt();
        d.type = q.value(2).toString();
        d.value = q.value(3).toString();
        d.note = q.value(4).toString();
        result.append(d);
    }
    return result;
}

int SqlitePersonRepository::addDocument(const PersonDocument &document)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO person_documents (person_id, type, value, note) VALUES (?,?,?,?)");
    q.addBindValue(document.personId);
    q.addBindValue(document.type);
    q.addBindValue(document.value);
    q.addBindValue(document.note);
    if (!q.exec()) {
        qWarning("addDocument: %s", qPrintable(q.lastError().text()));
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool SqlitePersonRepository::removeDocument(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM person_documents WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::removeDocument: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return true;
}

// --- Tags ---

QVector<Tag> SqlitePersonRepository::getAllTags()
{
    QVector<Tag> result;
    QSqlQuery q(m_db);
    if (!q.exec("SELECT id, name FROM tags ORDER BY name"))
        qWarning("SqlitePersonRepository::getAllTags: %s", qPrintable(q.lastError().text()));
    while (q.next()) {
        Tag t;
        t.id = q.value(0).toInt();
        t.name = q.value(1).toString();
        result.append(t);
    }
    return result;
}

QVector<Tag> SqlitePersonRepository::getTagsForPerson(int personId)
{
    QVector<Tag> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT t.id, t.name FROM tags t "
              "JOIN person_tags pt ON pt.tag_id = t.id "
              "WHERE pt.person_id = ? ORDER BY t.name");
    q.addBindValue(personId);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getTagsForPerson: %s", qPrintable(q.lastError().text()));
    while (q.next()) {
        Tag t;
        t.id = q.value(0).toInt();
        t.name = q.value(1).toString();
        result.append(t);
    }
    return result;
}

int SqlitePersonRepository::addTagToPerson(int personId, const QString &tagName)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT OR IGNORE INTO tags (name) VALUES (?)");
    q.addBindValue(tagName);
    if (!q.exec()) {
        qWarning("addTagToPerson (insert tag): %s", qPrintable(q.lastError().text()));
        return -1;
    }

    q.prepare("SELECT id FROM tags WHERE name = ?");
    q.addBindValue(tagName);
    if (!q.exec() || !q.next()) {
        qWarning("addTagToPerson (lookup tag): %s", qPrintable(q.lastError().text()));
        return -1;
    }
    int tagId = q.value(0).toInt();

    q.prepare("INSERT OR IGNORE INTO person_tags (person_id, tag_id) VALUES (?,?)");
    q.addBindValue(personId);
    q.addBindValue(tagId);
    if (!q.exec()) {
        qWarning("addTagToPerson (link): %s", qPrintable(q.lastError().text()));
        return -1;
    }
    return tagId;
}

bool SqlitePersonRepository::removeTagFromPerson(int personId, int tagId)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM person_tags WHERE person_id = ? AND tag_id = ?");
    q.addBindValue(personId);
    q.addBindValue(tagId);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::removeTagFromPerson: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return true;
}

// --- Phone Numbers ---

QVector<PhoneNumber> SqlitePersonRepository::getPhoneNumbers(int personId)
{
    QVector<PhoneNumber> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, person_id, number, label FROM phone_numbers WHERE person_id = ?");
    q.addBindValue(personId);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getPhoneNumbers: %s", qPrintable(q.lastError().text()));
    while (q.next()) {
        PhoneNumber p;
        p.id = q.value(0).toInt();
        p.personId = q.value(1).toInt();
        p.number = q.value(2).toString();
        p.label = q.value(3).toString();
        result.append(p);
    }
    return result;
}

int SqlitePersonRepository::addPhoneNumber(const PhoneNumber &phone)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO phone_numbers (person_id, number, label) VALUES (?,?,?)");
    q.addBindValue(phone.personId);
    q.addBindValue(phone.number);
    q.addBindValue(phone.label);
    if (!q.exec()) {
        qWarning("addPhoneNumber: %s", qPrintable(q.lastError().text()));
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool SqlitePersonRepository::removePhoneNumber(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM phone_numbers WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::removePhoneNumber: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return true;
}

// --- Notes ---

QVector<PersonNote> SqlitePersonRepository::getNotes(int personId)
{
    QVector<PersonNote> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, person_id, text, created_at FROM person_notes "
              "WHERE person_id = ? ORDER BY created_at DESC");
    q.addBindValue(personId);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getNotes: %s", qPrintable(q.lastError().text()));
    while (q.next()) {
        PersonNote n;
        n.id = q.value(0).toInt();
        n.personId = q.value(1).toInt();
        n.text = q.value(2).toString();
        n.createdAt = QDateTime::fromString(q.value(3).toString(), Qt::ISODate);
        result.append(n);
    }
    return result;
}

int SqlitePersonRepository::addNote(const PersonNote &note)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO person_notes (person_id, text) VALUES (?,?)");
    q.addBindValue(note.personId);
    q.addBindValue(note.text);
    if (!q.exec()) {
        qWarning("addNote: %s", qPrintable(q.lastError().text()));
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool SqlitePersonRepository::removeNote(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM person_notes WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::removeNote: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return true;
}

// --- File Attachments ---

QVector<PersonFile> SqlitePersonRepository::getFiles(int personId)
{
    QVector<PersonFile> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, person_id, file_name, file_path, added_at FROM person_files "
              "WHERE person_id = ? ORDER BY added_at DESC");
    q.addBindValue(personId);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getFiles: %s", qPrintable(q.lastError().text()));
    while (q.next()) {
        PersonFile f;
        f.id = q.value(0).toInt();
        f.personId = q.value(1).toInt();
        f.fileName = q.value(2).toString();
        f.filePath = q.value(3).toString();
        f.addedAt = QDateTime::fromString(q.value(4).toString(), Qt::ISODate);
        result.append(f);
    }
    return result;
}

PersonFile SqlitePersonRepository::getFileById(int id)
{
    PersonFile f;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, person_id, file_name, file_path, added_at FROM person_files WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getFileById: %s", qPrintable(q.lastError().text()));
    if (q.next()) {
        f.id = q.value(0).toInt();
        f.personId = q.value(1).toInt();
        f.fileName = q.value(2).toString();
        f.filePath = q.value(3).toString();
        f.addedAt = QDateTime::fromString(q.value(4).toString(), Qt::ISODate);
    }
    return f;
}

int SqlitePersonRepository::addFile(const PersonFile &file)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO person_files (person_id, file_name, file_path) VALUES (?,?,?)");
    q.addBindValue(file.personId);
    q.addBindValue(file.fileName);
    q.addBindValue(file.filePath);
    if (!q.exec()) {
        qWarning("addFile: %s", qPrintable(q.lastError().text()));
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool SqlitePersonRepository::removeFile(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM person_files WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::removeFile: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return true;
}

// --- Events ---

QVector<PersonEvent> SqlitePersonRepository::getEvents(int personId)
{
    QVector<PersonEvent> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, person_id, title, date, recurs_yearly, remind_days_before "
              "FROM person_events WHERE person_id = ?");
    q.addBindValue(personId);
    if (!q.exec())
        qWarning("SqlitePersonRepository::getEvents: %s", qPrintable(q.lastError().text()));
    while (q.next()) {
        PersonEvent e;
        e.id = q.value(0).toInt();
        e.personId = q.value(1).toInt();
        e.title = q.value(2).toString();
        e.date = QDate::fromString(q.value(3).toString(), Qt::ISODate);
        e.recursYearly = q.value(4).toBool();
        e.remindDaysBefore = q.value(5).toInt();
        result.append(e);
    }
    return result;
}

QVector<PersonEvent> SqlitePersonRepository::getAllEvents()
{
    QVector<PersonEvent> result;
    QSqlQuery q(m_db);
    if (!q.exec("SELECT id, person_id, title, date, recurs_yearly, remind_days_before FROM person_events"))
        qWarning("SqlitePersonRepository::getAllEvents: %s", qPrintable(q.lastError().text()));
    while (q.next()) {
        PersonEvent e;
        e.id = q.value(0).toInt();
        e.personId = q.value(1).toInt();
        e.title = q.value(2).toString();
        e.date = QDate::fromString(q.value(3).toString(), Qt::ISODate);
        e.recursYearly = q.value(4).toBool();
        e.remindDaysBefore = q.value(5).toInt();
        result.append(e);
    }
    return result;
}

int SqlitePersonRepository::addEvent(const PersonEvent &event)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO person_events (person_id, title, date, recurs_yearly, remind_days_before) "
              "VALUES (?,?,?,?,?)");
    q.addBindValue(event.personId);
    q.addBindValue(event.title);
    q.addBindValue(event.date.isValid() ? event.date.toString(Qt::ISODate) : QVariant());
    q.addBindValue(event.recursYearly ? 1 : 0);
    q.addBindValue(event.remindDaysBefore);
    if (!q.exec()) {
        qWarning("addEvent: %s", qPrintable(q.lastError().text()));
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool SqlitePersonRepository::updateEvent(const PersonEvent &event)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE person_events SET title=?, date=?, recurs_yearly=?, remind_days_before=? WHERE id=?");
    q.addBindValue(event.title);
    q.addBindValue(event.date.isValid() ? event.date.toString(Qt::ISODate) : QVariant());
    q.addBindValue(event.recursYearly ? 1 : 0);
    q.addBindValue(event.remindDaysBefore);
    q.addBindValue(event.id);
    if (!q.exec()) {
        qWarning("updateEvent: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool SqlitePersonRepository::removeEvent(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM person_events WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning("SqlitePersonRepository::removeEvent: %s", qPrintable(q.lastError().text()));
        return false;
    }
    return true;
}
