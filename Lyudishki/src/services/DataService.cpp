#include "DataService.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

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

int DataService::importPeopleJson(const QString &filePath)
{
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

    int imported = 0;
    for (const auto &val : doc.array()) {
        QJsonObject obj = val.toObject();

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
    QFile::remove(dbPath);
    return QFile::copy(backupFile, dbPath);
}
