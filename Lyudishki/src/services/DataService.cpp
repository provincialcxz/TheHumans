#include "DataService.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QSet>

namespace {

struct ParsedVCardPerson {
    QString lastName, firstName, patronymic;
    QDate birthDate;
    QVector<QPair<QString, QString>> phones; // (number, label)
    QVector<QPair<QString, QString>> emails; // (address, label)
};

QString unescapeVCardValue(QString value)
{
    value.replace("\\n", "\n").replace("\\N", "\n")
         .replace("\\,", ",").replace("\\;", ";").replace("\\\\", "\\");
    return value;
}

// Splits "NAME;PARAM=VAL:VALUE" into the property name (uppercased),
// its raw parameter string, and the (unescaped) value.
bool parseVCardLine(const QString &line, QString &name, QString &params, QString &value)
{
    int colon = line.indexOf(':');
    if (colon < 0) return false;
    QString left = line.left(colon);
    value = unescapeVCardValue(line.mid(colon + 1));
    int semi = left.indexOf(';');
    if (semi < 0) {
        name = left.toUpper();
        params = QString();
    } else {
        name = left.left(semi).toUpper();
        params = left.mid(semi + 1).toUpper();
    }
    return true;
}

QDate parseVCardDate(const QString &raw)
{
    QString v = raw;
    v.remove('-');
    if (v.size() >= 8)
        return QDate::fromString(v.left(8), "yyyyMMdd");
    return QDate();
}

QString labelFromParams(const QString &params)
{
    if (params.contains("CELL")) return "мобильный";
    if (params.contains("HOME")) return "домашний";
    if (params.contains("WORK")) return "рабочий";
    return QString();
}

QVector<ParsedVCardPerson> parseVCardFile(const QString &content)
{
    QVector<ParsedVCardPerson> result;

    // Unfold RFC 6350 continuation lines (a line starting with a space or
    // tab is a wrap of the previous one) before splitting into logical lines.
    QString normalized = content;
    normalized.replace("\r\n", "\n").replace('\r', '\n');
    QStringList rawLines = normalized.split('\n');
    QStringList lines;
    for (const QString &line : rawLines) {
        if (!line.isEmpty() && (line[0] == ' ' || line[0] == '\t') && !lines.isEmpty())
            lines.last() += line.mid(1);
        else
            lines.append(line);
    }

    bool inCard = false;
    ParsedVCardPerson current;
    QString fullName;

    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) continue;

        if (line.compare("BEGIN:VCARD", Qt::CaseInsensitive) == 0) {
            inCard = true;
            current = ParsedVCardPerson();
            fullName.clear();
            continue;
        }
        if (line.compare("END:VCARD", Qt::CaseInsensitive) == 0) {
            if (inCard) {
                if (current.lastName.isEmpty() && current.firstName.isEmpty() && !fullName.isEmpty()) {
                    // FN has no formal word order, but real-world exporters
                    // (Google/Apple contacts) write it as "GivenName FamilyName" —
                    // the opposite order from N's family-first fields.
                    QStringList parts = fullName.split(' ', Qt::SkipEmptyParts);
                    if (parts.size() >= 1) current.firstName = parts[0];
                    if (parts.size() >= 2) current.lastName = parts[1];
                    if (parts.size() >= 3) current.patronymic = parts.mid(2).join(' ');
                }
                if (!current.lastName.isEmpty() || !current.firstName.isEmpty())
                    result.append(current);
            }
            inCard = false;
            continue;
        }
        if (!inCard) continue;

        QString name, params, value;
        if (!parseVCardLine(line, name, params, value)) continue;

        if (name == "N") {
            QStringList comps = value.split(';');
            if (comps.size() > 0) current.lastName = comps[0];
            if (comps.size() > 1) current.firstName = comps[1];
            if (comps.size() > 2 && !comps[2].isEmpty()) current.patronymic = comps[2];
        } else if (name == "FN") {
            fullName = value;
        } else if (name == "TEL") {
            current.phones.append({value, labelFromParams(params)});
        } else if (name == "EMAIL") {
            current.emails.append({value, labelFromParams(params)});
        } else if (name == "BDAY") {
            current.birthDate = parseVCardDate(value);
        }
    }
    return result;
}

} // namespace

DataService::DataService(std::shared_ptr<IPersonRepository> personRepo,
                         std::shared_ptr<IGroupRepository> groupRepo)
    : m_personRepo(std::move(personRepo))
    , m_groupRepo(std::move(groupRepo))
{
}

bool DataService::exportPeopleJson(const QString &filePath)
{
    QJsonArray peopleArr;

    auto groups = m_groupRepo->getAll();
    QMap<int, QString> groupNames;
    for (const auto &g : groups)
        groupNames[g.id] = g.name;

    for (const auto &p : m_personRepo->getAll()) {
        QJsonObject obj;
        obj["lastName"] = p.lastName;
        obj["firstName"] = p.firstName;
        obj["patronymic"] = p.patronymic;
        obj["group"] = groupNames.value(p.groupId, "");
        obj["birthDate"] = p.birthDate.isValid() ? p.birthDate.toString(Qt::ISODate) : "";
        obj["phone"] = p.phone;
        obj["primaryContactMethod"] = p.primaryContactMethod;
        obj["address"] = p.address;
        obj["habits"] = p.habits;
        obj["hobbies"] = p.hobbies;
        obj["note"] = p.note;
        obj["metInPerson"] = p.metInPerson;
        obj["lastContactDate"] = p.lastContactDate.isValid() ? p.lastContactDate.toString(Qt::ISODate) : "";
        obj["photoPath"] = p.photoPath;

        // Social accounts
        QJsonArray socArr;
        for (const auto &s : m_personRepo->getSocialAccounts(p.id)) {
            QJsonObject so;
            so["platform"] = s.platform;
            so["handleOrUrl"] = s.handleOrUrl;
            socArr.append(so);
        }
        obj["socialAccounts"] = socArr;

        // Emails
        QJsonArray emArr;
        for (const auto &e : m_personRepo->getEmails(p.id)) {
            QJsonObject eo;
            eo["address"] = e.address;
            eo["label"] = e.label;
            emArr.append(eo);
        }
        obj["emails"] = emArr;

        // Phone numbers (additional)
        QJsonArray phArr;
        for (const auto &ph : m_personRepo->getPhoneNumbers(p.id)) {
            QJsonObject pho;
            pho["number"] = ph.number;
            pho["label"] = ph.label;
            phArr.append(pho);
        }
        obj["phoneNumbers"] = phArr;

        // Notes
        QJsonArray noteArr;
        for (const auto &n : m_personRepo->getNotes(p.id)) {
            QJsonObject no;
            no["text"] = n.text;
            no["createdAt"] = n.createdAt.toString(Qt::ISODate);
            noteArr.append(no);
        }
        obj["notes"] = noteArr;

        // Events
        QJsonArray evArr;
        for (const auto &ev : m_personRepo->getEvents(p.id)) {
            QJsonObject evo;
            evo["title"] = ev.title;
            evo["date"] = ev.date.isValid() ? ev.date.toString(Qt::ISODate) : "";
            evo["recursYearly"] = ev.recursYearly;
            evo["remindDaysBefore"] = ev.remindDaysBefore;
            evArr.append(evo);
        }
        obj["events"] = evArr;

        // Profile
        auto prof = m_personRepo->loadProfile(p.id);
        QJsonObject profObj;
        profObj["character"] = prof.character;
        profObj["interests"] = prof.interests;
        profObj["humorAndTriggers"] = prof.humorAndTriggers;
        profObj["politicalViews"] = prof.politicalViews;
        profObj["religiousViews"] = prof.religiousViews;
        profObj["languages"] = prof.languages;
        profObj["nicknames"] = prof.nicknames;
        profObj["family"] = prof.family;
        profObj["relationshipStatus"] = prof.relationshipStatus;
        profObj["mutualAcquaintances"] = prof.mutualAcquaintances;
        profObj["howAndWhenMet"] = prof.howAndWhenMet;
        profObj["careerTrack"] = prof.careerTrack;
        profObj["education"] = prof.education;
        profObj["skills"] = prof.skills;
        profObj["workSchedule"] = prof.workSchedule;
        profObj["workContacts"] = prof.workContacts;
        profObj["platformAccounts"] = prof.platformAccounts;
        profObj["mediaConsumption"] = prof.mediaConsumption;
        profObj["property"] = prof.property;
        profObj["favoritePlaces"] = prof.favoritePlaces;
        profObj["pets"] = prof.pets;
        profObj["healthAndAllergies"] = prof.healthAndAllergies;
        profObj["reliability"] = prof.reliability;
        profObj["infoFreshness"] = prof.infoFreshness;
        profObj["additionalNotes"] = prof.additionalNotes;
        profObj["gender"] = prof.gender;
        profObj["giftIdeas"] = prof.giftIdeas;
        profObj["clothingSize"] = prof.clothingSize;
        profObj["favoriteFood"] = prof.favoriteFood;
        obj["profile"] = profObj;

        peopleArr.append(obj);
    }

    QJsonDocument doc(peopleArr);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

int DataService::importPeopleJson(const QString &filePath, int *skippedCount)
{
    if (skippedCount) *skippedCount = 0;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return -1;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isArray())
        return -1;

    auto groups = m_groupRepo->getAll();
    QMap<QString, int> groupByName;
    for (const auto &g : groups)
        groupByName[g.name] = g.id;

    // Identity key for dedup: re-importing the same export file is a normal
    // "restore on another device" flow and shouldn't multiply every contact.
    QSet<QString> existingIdentities;
    for (const auto &existing : m_personRepo->getAll()) {
        existingIdentities.insert(existing.lastName + "\x1f" + existing.firstName + "\x1f" +
                                   existing.birthDate.toString(Qt::ISODate));
    }

    int imported = 0;
    for (const auto &val : doc.array()) {
        QJsonObject obj = val.toObject();

        QString identity = obj["lastName"].toString() + "\x1f" + obj["firstName"].toString() +
                            "\x1f" + obj["birthDate"].toString();
        if (existingIdentities.contains(identity)) {
            if (skippedCount) (*skippedCount)++;
            continue;
        }

        QString groupName = obj["group"].toString();
        int groupId = groupByName.value(groupName, 0);
        if (groupId == 0 && !groupName.isEmpty()) {
            Group ng;
            ng.name = groupName;
            groupId = m_groupRepo->add(ng);
            groupByName[groupName] = groupId;
        }
        if (groupId == 0 && !groups.isEmpty())
            groupId = groups.first().id;

        Person p;
        p.groupId = groupId;
        p.lastName = obj["lastName"].toString();
        p.firstName = obj["firstName"].toString();
        p.patronymic = obj["patronymic"].toString();
        p.birthDate = QDate::fromString(obj["birthDate"].toString(), Qt::ISODate);
        p.phone = obj["phone"].toString();
        p.primaryContactMethod = obj["primaryContactMethod"].toString();
        p.address = obj["address"].toString();
        p.habits = obj["habits"].toString();
        p.hobbies = obj["hobbies"].toString();
        p.note = obj["note"].toString();
        p.metInPerson = obj["metInPerson"].toBool();
        p.lastContactDate = QDate::fromString(obj["lastContactDate"].toString(), Qt::ISODate);
        p.photoPath = obj["photoPath"].toString();

        int pid = m_personRepo->add(p);
        if (pid < 0) continue;

        // Social accounts
        for (const auto &sv : obj["socialAccounts"].toArray()) {
            auto so = sv.toObject();
            SocialAccount sa;
            sa.personId = pid;
            sa.platform = so["platform"].toString();
            sa.handleOrUrl = so["handleOrUrl"].toString();
            m_personRepo->addSocialAccount(sa);
        }

        // Emails
        for (const auto &ev : obj["emails"].toArray()) {
            auto eo = ev.toObject();
            Email em;
            em.personId = pid;
            em.address = eo["address"].toString();
            em.label = eo["label"].toString();
            m_personRepo->addEmail(em);
        }

        // Phone numbers
        for (const auto &pv : obj["phoneNumbers"].toArray()) {
            auto pho = pv.toObject();
            PhoneNumber ph;
            ph.personId = pid;
            ph.number = pho["number"].toString();
            ph.label = pho["label"].toString();
            m_personRepo->addPhoneNumber(ph);
        }

        // Notes
        for (const auto &nv : obj["notes"].toArray()) {
            auto no = nv.toObject();
            PersonNote note;
            note.personId = pid;
            note.text = no["text"].toString();
            m_personRepo->addNote(note);
        }

        // Events
        for (const auto &ev : obj["events"].toArray()) {
            auto evo = ev.toObject();
            PersonEvent pe;
            pe.personId = pid;
            pe.title = evo["title"].toString();
            pe.date = QDate::fromString(evo["date"].toString(), Qt::ISODate);
            pe.recursYearly = evo["recursYearly"].toBool();
            pe.remindDaysBefore = evo["remindDaysBefore"].toInt(3);
            m_personRepo->addEvent(pe);
        }

        // Profile
        if (obj.contains("profile")) {
            auto po = obj["profile"].toObject();
            PersonProfile prof;
            prof.personId = pid;
            prof.character = po["character"].toString();
            prof.interests = po["interests"].toString();
            prof.humorAndTriggers = po["humorAndTriggers"].toString();
            prof.politicalViews = po["politicalViews"].toString();
            prof.religiousViews = po["religiousViews"].toString();
            prof.languages = po["languages"].toString();
            prof.nicknames = po["nicknames"].toString();
            prof.family = po["family"].toString();
            prof.relationshipStatus = po["relationshipStatus"].toString();
            prof.mutualAcquaintances = po["mutualAcquaintances"].toString();
            prof.howAndWhenMet = po["howAndWhenMet"].toString();
            prof.careerTrack = po["careerTrack"].toString();
            prof.education = po["education"].toString();
            prof.skills = po["skills"].toString();
            prof.workSchedule = po["workSchedule"].toString();
            prof.workContacts = po["workContacts"].toString();
            prof.platformAccounts = po["platformAccounts"].toString();
            prof.mediaConsumption = po["mediaConsumption"].toString();
            prof.property = po["property"].toString();
            prof.favoritePlaces = po["favoritePlaces"].toString();
            prof.pets = po["pets"].toString();
            prof.healthAndAllergies = po["healthAndAllergies"].toString();
            prof.reliability = po["reliability"].toString();
            prof.infoFreshness = po["infoFreshness"].toString();
            prof.additionalNotes = po["additionalNotes"].toString();
            prof.gender = po["gender"].toString();
            prof.giftIdeas = po["giftIdeas"].toString();
            prof.clothingSize = po["clothingSize"].toString();
            prof.favoriteFood = po["favoriteFood"].toString();
            m_personRepo->saveProfile(prof);
        }

        existingIdentities.insert(identity); // guard against dupes within this same file
        imported++;
    }
    return imported;
}

int DataService::importPeopleVCard(const QString &filePath, int *skippedCount)
{
    if (skippedCount) *skippedCount = 0;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return -1;
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    auto entries = parseVCardFile(content);

    // vCards carry no group info. Rather than dumping everyone into whichever
    // group happens to sort first, file each import under its own
    // "Импорт_<date>" group — created lazily so a run that only hits
    // duplicates doesn't leave behind an empty group.
    int importGroupId = 0;

    QSet<QString> existingIdentities;
    for (const auto &existing : m_personRepo->getAll()) {
        existingIdentities.insert(existing.lastName + "\x1f" + existing.firstName + "\x1f" +
                                   existing.birthDate.toString(Qt::ISODate));
    }

    int imported = 0;
    for (const auto &entry : entries) {
        QString identity = entry.lastName + "\x1f" + entry.firstName + "\x1f" +
                            entry.birthDate.toString(Qt::ISODate);
        if (existingIdentities.contains(identity)) {
            if (skippedCount) (*skippedCount)++;
            continue;
        }

        if (importGroupId == 0) {
            QString groupName = "Импорт_" + QDate::currentDate().toString("yyyy-MM-dd");
            for (const auto &g : m_groupRepo->getAll()) {
                if (g.name == groupName) { importGroupId = g.id; break; }
            }
            if (importGroupId == 0) {
                // A second vCard import on the same day reuses this group
                // instead of failing on the UNIQUE(name) constraint.
                Group g;
                g.name = groupName;
                importGroupId = m_groupRepo->add(g);
            }
        }

        Person p;
        p.groupId = importGroupId;
        p.lastName = entry.lastName;
        p.firstName = entry.firstName;
        p.patronymic = entry.patronymic;
        p.birthDate = entry.birthDate;
        if (!entry.phones.isEmpty())
            p.phone = entry.phones.first().first;

        int pid = m_personRepo->add(p);
        if (pid < 0) continue;

        // Any phone beyond the first is stored as an additional number —
        // Person::phone already holds the primary one.
        for (int i = 1; i < entry.phones.size(); ++i) {
            PhoneNumber ph;
            ph.personId = pid;
            ph.number = entry.phones[i].first;
            ph.label = entry.phones[i].second;
            m_personRepo->addPhoneNumber(ph);
        }

        for (const auto &em : entry.emails) {
            Email e;
            e.personId = pid;
            e.address = em.first;
            e.label = em.second;
            m_personRepo->addEmail(e);
        }

        existingIdentities.insert(identity); // guard against dupes within this same file
        imported++;
    }
    return imported;
}

bool DataService::createBackup(const QString &dbPath, const QString &backupDir)
{
    QDir().mkpath(backupDir);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QString backupPath = backupDir + "/lyudishki_backup_" + timestamp + ".db";
    return QFile::copy(dbPath, backupPath);
}

bool DataService::restoreBackup(const QString &backupFile, const QString &dbPath)
{
    if (!QFile::exists(backupFile))
        return false;

    // Copy to a staging file first and only touch the live DB once the copy
    // is verified — never delete dbPath before we know the replacement is
    // actually on disk (disk full / permissions / corrupt backup must not
    // leave the user with no database at all).
    QString tmpPath = dbPath + ".restoring";
    QFile::remove(tmpPath);
    if (!QFile::copy(backupFile, tmpPath))
        return false;

    QFile::remove(dbPath);
    if (!QFile::rename(tmpPath, dbPath)) {
        // Extremely unlikely after a successful copy, but if rename fails
        // leave the staged copy in place so the user's data isn't stranded.
        return false;
    }
    return true;
}
