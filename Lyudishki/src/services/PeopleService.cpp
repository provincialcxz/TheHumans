#include "PeopleService.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QImage>

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

        // Downscale to the largest size the UI ever displays (150x150 detail
        // photo) with headroom, so a multi-MB phone photo doesn't get carried
        // around (and re-decoded on every list repaint) at full resolution.
        const int kMaxDimension = 640;
        QImage img(sourceFilePath);
        if (!img.isNull()) {
            if (img.width() > kMaxDimension || img.height() > kMaxDimension)
                img = img.scaled(kMaxDimension, kMaxDimension, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (!img.save(destPath, nullptr, 90))
                QFile::copy(sourceFilePath, destPath); // fallback if save fails
        } else {
            QFile::copy(sourceFilePath, destPath); // unrecognized format: copy as-is
        }
    }

    return destPath;
}

bool PeopleService::removePerson(int id)
{
    // Drop the managed on-disk copies (attached files + photo) before the
    // DB row disappears — otherwise a "deleted" person's photos/documents
    // stay on disk forever with no UI to find or clear them. Only ever
    // touches paths under our own managed storage, never the user's
    // original files (those are only referenced directly when no
    // attachments dir is configured at all).
    if (!m_attachmentsDir.isEmpty()) {
        QDir(m_attachmentsDir + "/person_" + QString::number(id)).removeRecursively();
        QDir(m_attachmentsDir + "/photos/person_" + QString::number(id)).removeRecursively();
    }
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

QMap<int, QString> PeopleService::getReliabilityMap()
{
    return m_repo->getReliabilityMap();
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

QVector<PersonDocument> PeopleService::getDocuments(int personId)
{
    return m_repo->getDocuments(personId);
}

int PeopleService::addDocument(const PersonDocument &document)
{
    return m_repo->addDocument(document);
}

bool PeopleService::removeDocument(int id)
{
    return m_repo->removeDocument(id);
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
    PersonFile f = m_repo->getFileById(id);
    if (!f.filePath.isEmpty() && !m_attachmentsDir.isEmpty() && f.filePath.startsWith(m_attachmentsDir))
        QFile::remove(f.filePath);
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
