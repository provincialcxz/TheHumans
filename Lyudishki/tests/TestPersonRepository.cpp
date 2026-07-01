#include <QtTest>
#include "data/db/DatabaseManager.h"
#include "data/db/MigrationManager.h"
#include "data/repositories/SqlitePersonRepository.h"

class TestPersonRepository : public QObject {
    Q_OBJECT
private slots:
    void testAddAndGet()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "Иван";
        p.lastName = "Петров";
        p.patronymic = "Сергеевич";
        p.birthDate = QDate(1990, 5, 15);
        p.phone = "+79001234567";
        p.note = "тест";

        int id = repo.add(p);
        QVERIFY(id > 0);

        auto fetched = repo.getById(id);
        QCOMPARE(fetched.firstName, "Иван");
        QCOMPARE(fetched.lastName, "Петров");
        QCOMPARE(fetched.birthDate, QDate(1990, 5, 15));
    }

    void testProfile()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "Test";
        p.lastName = "User";
        int id = repo.add(p);

        PersonProfile prof;
        prof.personId = id;
        prof.character = "Спокойный";
        prof.reliability = "высокая";
        QVERIFY(repo.saveProfile(prof));

        auto loaded = repo.loadProfile(id);
        QCOMPARE(loaded.character, "Спокойный");
        QCOMPARE(loaded.reliability, "высокая");
    }

    void testSocialAndEmail()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "A";
        p.lastName = "B";
        int pid = repo.add(p);

        SocialAccount sa;
        sa.personId = pid;
        sa.platform = "Telegram";
        sa.handleOrUrl = "@test";
        int sid = repo.addSocialAccount(sa);
        QVERIFY(sid > 0);

        auto socials = repo.getSocialAccounts(pid);
        QCOMPARE(socials.size(), 1);
        QCOMPARE(socials[0].platform, "Telegram");

        Email e;
        e.personId = pid;
        e.address = "test@test.com";
        e.label = "work";
        int eid = repo.addEmail(e);
        QVERIFY(eid > 0);

        auto emails = repo.getEmails(pid);
        QCOMPARE(emails.size(), 1);
        QCOMPARE(emails[0].address, "test@test.com");
    }

    void testEvents()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "E";
        p.lastName = "V";
        int pid = repo.add(p);

        PersonEvent ev;
        ev.personId = pid;
        ev.title = "Годовщина";
        ev.date = QDate(2025, 6, 1);
        ev.recursYearly = true;
        ev.remindDaysBefore = 5;
        int evid = repo.addEvent(ev);
        QVERIFY(evid > 0);

        auto events = repo.getEvents(pid);
        QCOMPARE(events.size(), 1);
        QCOMPARE(events[0].title, "Годовщина");
        QVERIFY(events[0].recursYearly);
    }

    void testDelete()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "Del";
        p.lastName = "Me";
        int id = repo.add(p);
        QVERIFY(repo.remove(id));

        auto fetched = repo.getById(id);
        QCOMPARE(fetched.id, 0);
    }

    void testPhoneNumbers()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "Phone";
        p.lastName = "Test";
        int pid = repo.add(p);

        PhoneNumber ph;
        ph.personId = pid;
        ph.number = "+79991234567";
        ph.label = "рабочий";
        int phId = repo.addPhoneNumber(ph);
        QVERIFY(phId > 0);

        auto phones = repo.getPhoneNumbers(pid);
        QCOMPARE(phones.size(), 1);
        QCOMPARE(phones[0].number, "+79991234567");
        QCOMPARE(phones[0].label, "рабочий");

        QVERIFY(repo.removePhoneNumber(phId));
        QCOMPARE(repo.getPhoneNumbers(pid).size(), 0);
    }

    void testNotes()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "Note";
        p.lastName = "Test";
        int pid = repo.add(p);

        PersonNote n;
        n.personId = pid;
        n.text = "Первая заметка";
        int nid = repo.addNote(n);
        QVERIFY(nid > 0);

        auto notes = repo.getNotes(pid);
        QCOMPARE(notes.size(), 1);
        QCOMPARE(notes[0].text, "Первая заметка");
        QVERIFY(notes[0].createdAt.isValid());

        QVERIFY(repo.removeNote(nid));
        QCOMPARE(repo.getNotes(pid).size(), 0);
    }

    void testFiles()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "File";
        p.lastName = "Test";
        int pid = repo.add(p);

        PersonFile f;
        f.personId = pid;
        f.fileName = "document.pdf";
        f.filePath = "/tmp/document.pdf";
        int fid = repo.addFile(f);
        QVERIFY(fid > 0);

        auto files = repo.getFiles(pid);
        QCOMPARE(files.size(), 1);
        QCOMPARE(files[0].fileName, "document.pdf");

        QVERIFY(repo.removeFile(fid));
        QCOMPARE(repo.getFiles(pid).size(), 0);
    }

    void testExtendedProfileFields()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "Ext";
        p.lastName = "Profile";
        int pid = repo.add(p);

        PersonProfile prof;
        prof.personId = pid;
        prof.gender = "Мужской";
        prof.giftIdeas = "Книги";
        prof.clothingSize = "L";
        prof.favoriteFood = "Пицца";
        QVERIFY(repo.saveProfile(prof));

        auto loaded = repo.loadProfile(pid);
        QCOMPARE(loaded.gender, "Мужской");
        QCOMPARE(loaded.giftIdeas, "Книги");
        QCOMPARE(loaded.clothingSize, "L");
        QCOMPARE(loaded.favoriteFood, "Пицца");
    }

    void testReliabilityMap()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p1;
        p1.groupId = 1;
        p1.firstName = "A";
        p1.lastName = "Reliable";
        int pid1 = repo.add(p1);

        Person p2;
        p2.groupId = 1;
        p2.firstName = "B";
        p2.lastName = "Unreliable";
        int pid2 = repo.add(p2);

        PersonProfile prof1;
        prof1.personId = pid1;
        prof1.reliability = "Высокая";
        QVERIFY(repo.saveProfile(prof1));

        PersonProfile prof2;
        prof2.personId = pid2;
        prof2.reliability = "Низкая";
        QVERIFY(repo.saveProfile(prof2));

        auto map = repo.getReliabilityMap();
        QCOMPARE(map.value(pid1), "Высокая");
        QCOMPARE(map.value(pid2), "Низкая");
    }

    void testPhotoGallery()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "Photo";
        p.lastName = "Test";
        int pid = repo.add(p);

        PersonPhoto ph;
        ph.personId = pid;
        ph.filePath = "/tmp/gallery1.jpg";
        int phId = repo.addPhoto(ph);
        QVERIFY(phId > 0);

        auto photos = repo.getPhotos(pid);
        QCOMPARE(photos.size(), 1);
        QCOMPARE(photos[0].filePath, "/tmp/gallery1.jpg");
        QVERIFY(photos[0].addedAt.isValid());

        auto fetched = repo.getPhotoById(phId);
        QCOMPARE(fetched.personId, pid);

        QVERIFY(repo.removePhoto(phId));
        QCOMPARE(repo.getPhotos(pid).size(), 0);
    }

    void testDocuments()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "Doc";
        p.lastName = "Test";
        int pid = repo.add(p);

        PersonDocument d;
        d.personId = pid;
        d.type = "Паспорт";
        d.value = "1234 567890";
        d.note = "выдан в 2015";
        int did = repo.addDocument(d);
        QVERIFY(did > 0);

        auto docs = repo.getDocuments(pid);
        QCOMPARE(docs.size(), 1);
        QCOMPARE(docs[0].type, "Паспорт");
        QCOMPARE(docs[0].value, "1234 567890");

        QVERIFY(repo.removeDocument(did));
        QCOMPARE(repo.getDocuments(pid).size(), 0);
    }

    void testTags()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p1;
        p1.groupId = 1;
        p1.firstName = "Tag";
        p1.lastName = "One";
        int pid1 = repo.add(p1);

        Person p2;
        p2.groupId = 1;
        p2.firstName = "Tag";
        p2.lastName = "Two";
        int pid2 = repo.add(p2);

        int tagId = repo.addTagToPerson(pid1, "важное");
        QVERIFY(tagId > 0);

        // Same tag name attached to a second person must reuse the existing
        // tag row, not create a duplicate.
        int tagId2 = repo.addTagToPerson(pid2, "важное");
        QCOMPARE(tagId2, tagId);
        QCOMPARE(repo.getAllTags().size(), 1);

        auto tags1 = repo.getTagsForPerson(pid1);
        QCOMPARE(tags1.size(), 1);
        QCOMPARE(tags1[0].name, "важное");

        QVERIFY(repo.removeTagFromPerson(pid1, tagId));
        QCOMPARE(repo.getTagsForPerson(pid1).size(), 0);
        // Removing the tag from one person must not affect the other.
        QCOMPARE(repo.getTagsForPerson(pid2).size(), 1);
    }

    void testMetInPersonAndLastContact()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        SqlitePersonRepository repo(dbm.database());

        Person p;
        p.groupId = 1;
        p.firstName = "Contact";
        p.lastName = "Test";
        p.metInPerson = true;
        p.lastContactDate = QDate(2026, 1, 15);
        int pid = repo.add(p);

        auto fetched = repo.getById(pid);
        QVERIFY(fetched.metInPerson);
        QCOMPARE(fetched.lastContactDate, QDate(2026, 1, 15));

        fetched.metInPerson = false;
        fetched.lastContactDate = QDate();
        QVERIFY(repo.update(fetched));

        auto refetched = repo.getById(pid);
        QVERIFY(!refetched.metInPerson);
        QVERIFY(!refetched.lastContactDate.isValid());
    }
};

QObject* createTestPersonRepository() { return new TestPersonRepository(); }

#include "TestPersonRepository.moc"
