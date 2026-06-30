#include "PeopleService.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

PeopleService::PeopleService(std::shared_ptr<IPersonRepository> repo, const QString &attachmentsDir)
    : m_repo(std::move(repo))
    , m_attachmentsDir(attachmentsDir)
{
}

QVector<Person> PeopleService::getAllPeople()
{
    return m_repo->getAll();
}

QVector<Person> PeopleService::getPeopleInGroup(int groupId)
{
    return m_repo->getByGroupId(groupId);
}

Person PeopleService::getPerson(int id)
{
    return m_repo->getById(id);
}

int PeopleService::addPerson(const Person &person)
{
    return m_repo->add(person);
}

bool PeopleService::updatePerson(const Person &person)
{
    return m_repo->update(person);
}

QString PeopleService::importPhoto(int personId, const QString &sourceFilePath)
{
    QFileInfo info(sourceFilePath);
    QString destPath = sourceFilePath;

    if (!m_attachmentsDir.isEmpty()) {
        QString photoDir = m_attachmentsDir + "/photos/person_" + QString::number(personId);
        QDir().mkpath(photoDir);
        QString ext = info.suffix();
        destPath = photoDir + "/photo" + (ext.isEmpty() ? "" : "." + ext);
        if (QFile::exists(destPath))
            QFile::remove(destPath);
        QFile::copy(sourceFilePath, destPath);
    }

    return destPath;
}

bool PeopleService::removePerson(int id)
{
    return m_repo->remove(id);
}

PersonProfile PeopleService::loadProfile(int personId)
{
    return m_repo->loadProfile(personId);
}

bool PeopleService::saveProfile(const PersonProfile &profile)
{
    return m_repo->saveProfile(profile);
}

QVector<SocialAccount> PeopleService::getSocialAccounts(int personId)
{
    return m_repo->getSocialAccounts(personId);
}

int PeopleService::addSocialAccount(const SocialAccount &account)
{
    return m_repo->addSocialAccount(account);
}

bool PeopleService::removeSocialAccount(int id)
{
    return m_repo->removeSocialAccount(id);
}

QVector<Email> PeopleService::getEmails(int personId)
{
    return m_repo->getEmails(personId);
}

int PeopleService::addEmail(const Email &email)
{
    return m_repo->addEmail(email);
}

bool PeopleService::removeEmail(int id)
{
    return m_repo->removeEmail(id);
}

QVector<PhoneNumber> PeopleService::getPhoneNumbers(int personId)
{
    return m_repo->getPhoneNumbers(personId);
}

int PeopleService::addPhoneNumber(const PhoneNumber &phone)
{
    return m_repo->addPhoneNumber(phone);
}

bool PeopleService::removePhoneNumber(int id)
{
    return m_repo->removePhoneNumber(id);
}

QVector<PersonNote> PeopleService::getNotes(int personId)
{
    return m_repo->getNotes(personId);
}

int PeopleService::addNote(int personId, const QString &text)
{
    PersonNote n;
    n.personId = personId;
    n.text = text;
    return m_repo->addNote(n);
}

bool PeopleService::removeNote(int id)
{
    return m_repo->removeNote(id);
}

QVector<PersonFile> PeopleService::getFiles(int personId)
{
    return m_repo->getFiles(personId);
}

int PeopleService::attachFile(int personId, const QString &sourceFilePath)
{
    QFileInfo info(sourceFilePath);
    QString destPath = sourceFilePath;

    if (!m_attachmentsDir.isEmpty()) {
        QString personDir = m_attachmentsDir + "/person_" + QString::number(personId);
        QDir().mkpath(personDir);
        destPath = personDir + "/" + info.fileName();
        if (QFile::exists(destPath) && QFileInfo(destPath).absoluteFilePath() != info.absoluteFilePath()) {
            QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
            destPath = personDir + "/" + ts + "_" + info.fileName();
        }
        if (!QFile::exists(destPath))
            QFile::copy(sourceFilePath, destPath);
    }

    PersonFile pf;
    pf.personId = personId;
    pf.fileName = info.fileName();
    pf.filePath = destPath;
    return m_repo->addFile(pf);
}

bool PeopleService::removeFile(int id)
{
    return m_repo->removeFile(id);
}

QVector<PersonEvent> PeopleService::getEvents(int personId)
{
    return m_repo->getEvents(personId);
}

QVector<PersonEvent> PeopleService::getAllEvents()
{
    return m_repo->getAllEvents();
}

int PeopleService::addEvent(const PersonEvent &event)
{
    return m_repo->addEvent(event);
}

bool PeopleService::updateEvent(const PersonEvent &event)
{
    return m_repo->updateEvent(event);
}

bool PeopleService::removeEvent(int id)
{
    return m_repo->removeEvent(id);
}
