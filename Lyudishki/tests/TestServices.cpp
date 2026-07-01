#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include "data/db/DatabaseManager.h"
#include "data/db/MigrationManager.h"
#include "data/repositories/SqlitePersonRepository.h"
#include "data/repositories/SqliteGroupRepository.h"
#include "services/PeopleService.h"
#include "services/GroupService.h"
#include "services/SearchService.h"
#include "services/CalendarExportService.h"
#include "services/DataService.h"

class TestServices : public QObject {
    Q_OBJECT
private slots:
    void testGroupService()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        auto repo = std::make_shared<SqliteGroupRepository>(dbm.database());
        GroupService svc(repo);

        auto groups = svc.getAllGroups();
        QCOMPARE(groups.size(), 4);

        int id = svc.addGroup("Новая");
        QVERIFY(id > 0);
        QCOMPARE(svc.getAllGroups().size(), 5);

        QVERIFY(svc.renameGroup(id, "Переименованная"));
        QCOMPARE(svc.getGroup(id).name, "Переименованная");

        QVERIFY(svc.removeGroup(id));
        QCOMPARE(svc.getAllGroups().size(), 4);
    }

    void testPeopleService()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        auto repo = std::make_shared<SqlitePersonRepository>(dbm.database());
        PeopleService svc(repo);

        Person p;
        p.groupId = 1;
        p.firstName = "Тест";
        p.lastName = "Человек";
        int id = svc.addPerson(p);
        QVERIFY(id > 0);

        auto person = svc.getPerson(id);
        QCOMPARE(person.firstName, "Тест");

        auto inGroup = svc.getPeopleInGroup(1);
        QCOMPARE(inGroup.size(), 1);

        person.firstName = "Обновлённый";
        QVERIFY(svc.updatePerson(person));
        QCOMPARE(svc.getPerson(id).firstName, "Обновлённый");

        QVERIFY(svc.removePerson(id));
        QCOMPARE(svc.getAllPeople().size(), 0);
    }

    void testMarkContactedNow()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        auto repo = std::make_shared<SqlitePersonRepository>(dbm.database());
        PeopleService svc(repo);

        Person p;
        p.groupId = 1;
        p.firstName = "Давно";
        p.lastName = "НеВидел";
        int id = svc.addPerson(p);
        QVERIFY(!svc.getPerson(id).lastContactDate.isValid());

        QVERIFY(svc.markContactedNow(id));
        QCOMPARE(svc.getPerson(id).lastContactDate, QDate::currentDate());
    }

    void testSetAsAvatar()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        auto repo = std::make_shared<SqlitePersonRepository>(dbm.database());
        PeopleService svc(repo);

        Person p;
        p.groupId = 1;
        p.firstName = "Avatar";
        p.lastName = "Test";
        int id = svc.addPerson(p);

        PersonPhoto ph;
        ph.personId = id;
        ph.filePath = "/tmp/some_photo.jpg";
        int phId = repo->addPhoto(ph);

        QVERIFY(svc.setAsAvatar(id, phId));
        QCOMPARE(svc.getPerson(id).photoPath, "/tmp/some_photo.jpg");
    }

    // Regression test: switching avatars from the gallery used to silently
    // discard whatever photo was set before (it was overwritten with no
    // trace). setAsAvatar must swap instead: the outgoing avatar goes into
    // the gallery, and the incoming one is removed from it.
    void testSetAsAvatarPreservesPreviousAvatar()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        auto repo = std::make_shared<SqlitePersonRepository>(dbm.database());
        PeopleService svc(repo);

        Person p;
        p.groupId = 1;
        p.firstName = "Swap";
        p.lastName = "Test";
        p.photoPath = "/tmp/old_avatar.jpg";
        int id = svc.addPerson(p);

        PersonPhoto ph;
        ph.personId = id;
        ph.filePath = "/tmp/gallery_photo.jpg";
        int phId = repo->addPhoto(ph);

        QVERIFY(svc.setAsAvatar(id, phId));
        QCOMPARE(svc.getPerson(id).photoPath, "/tmp/gallery_photo.jpg");

        auto gallery = svc.getPhotos(id);
        QCOMPARE(gallery.size(), 1);
        QCOMPARE(gallery[0].filePath, "/tmp/old_avatar.jpg");
    }

    void testSearchService()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        auto repo = std::make_shared<SqlitePersonRepository>(dbm.database());
        SearchService svc(repo);

        Person p1;
        p1.groupId = 1;
        p1.firstName = "Алексей";
        p1.lastName = "Смирнов";
        repo->add(p1);

        Person p2;
        p2.groupId = 2;
        p2.firstName = "Ольга";
        p2.lastName = "Иванова";
        repo->add(p2);

        auto results = svc.search("смирнов");
        QCOMPARE(results.size(), 1);
        QCOMPARE(results[0].firstName, "Алексей");

        auto all = svc.search("");
        QCOMPARE(all.size(), 2);
    }

    void testSearchCoversProfileEmailsPhonesAndNotes()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        auto repo = std::make_shared<SqlitePersonRepository>(dbm.database());
        SearchService svc(repo);

        Person target;
        target.groupId = 1;
        target.firstName = "Игорь";
        target.lastName = "Скрытый";
        int targetId = repo->add(target);

        Person decoy;
        decoy.groupId = 1;
        decoy.firstName = "Other";
        decoy.lastName = "Person";
        repo->add(decoy);

        // None of these live on the Person row itself — they're exactly the
        // data the old substring-on-getAll() search couldn't see at all.
        PersonProfile prof;
        prof.personId = targetId;
        prof.careerTrack = "Работает программистом-зоологом в стартапе Жирафхак";
        QVERIFY(repo->saveProfile(prof));

        Email e;
        e.personId = targetId;
        e.address = "skrytyi@example.com";
        repo->addEmail(e);

        PhoneNumber ph;
        ph.personId = targetId;
        ph.number = "+79995554433";
        ph.label = "запасной";
        repo->addPhoneNumber(ph);

        PersonNote note;
        note.personId = targetId;
        note.text = "Любит играть в петанк по выходным";
        repo->addNote(note);

        // Match by an obscure profile word.
        auto byProfile = svc.search("жирафхак");
        QCOMPARE(byProfile.size(), 1);
        QCOMPARE(byProfile[0].id, targetId);

        // Match by email local part.
        auto byEmail = svc.search("skrytyi");
        QCOMPARE(byEmail.size(), 1);
        QCOMPARE(byEmail[0].id, targetId);

        // Match by the secondary phone number.
        auto byPhone = svc.searchPersonIds("79995554433");
        QCOMPARE(byPhone.size(), 1);
        QCOMPARE(byPhone[0], targetId);

        // Match by a note.
        auto byNote = svc.search("петанк");
        QCOMPARE(byNote.size(), 1);
        QCOMPARE(byNote[0].id, targetId);

        // Removing the note should drop it from the index too.
        auto notes = repo->getNotes(targetId);
        QCOMPARE(notes.size(), 1);
        QVERIFY(repo->removeNote(notes[0].id));
        QCOMPARE(svc.search("петанк").size(), 0);
    }

    void testFileCleanupOnRemoval()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        QTemporaryDir attachDir;
        QVERIFY(attachDir.isValid());

        auto repo = std::make_shared<SqlitePersonRepository>(dbm.database());
        PeopleService svc(repo, attachDir.path());

        Person p;
        p.groupId = 1;
        p.firstName = "File";
        p.lastName = "Cleanup";
        int pid = svc.addPerson(p);
        QVERIFY(pid > 0);

        // Attach a file: source lives outside the managed dir, attachFile copies it in.
        QTemporaryDir sourceDir;
        QString sourcePath = sourceDir.path() + "/doc.txt";
        QFile src(sourcePath);
        QVERIFY(src.open(QIODevice::WriteOnly));
        src.write("hello");
        src.close();

        int fileId = svc.attachFile(pid, sourcePath);
        QVERIFY(fileId > 0);
        auto files = svc.getFiles(pid);
        QCOMPARE(files.size(), 1);
        QString managedPath = files[0].filePath;
        QVERIFY(QFile::exists(managedPath));
        QVERIFY(managedPath.startsWith(attachDir.path()));

        // Removing the file should delete the managed copy from disk.
        QVERIFY(svc.removeFile(fileId));
        QVERIFY(!QFile::exists(managedPath));

        // Attach another file, then remove the whole person — its managed
        // storage directory should be gone too.
        int fileId2 = svc.attachFile(pid, sourcePath);
        QVERIFY(fileId2 > 0);
        QString personDir = attachDir.path() + "/person_" + QString::number(pid);
        QVERIFY(QDir(personDir).exists());

        QVERIFY(svc.removePerson(pid));
        QVERIFY(!QDir(personDir).exists());

        // The original source file must never be touched.
        QVERIFY(QFile::exists(sourcePath));
    }

    void testIcsExportUsesCustomBirthdayRemindDays()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        auto repo = std::make_shared<SqlitePersonRepository>(dbm.database());
        CalendarExportService svc(repo);

        Person p;
        p.groupId = 1;
        p.firstName = "Дима";
        p.lastName = "Тестов";
        p.birthDate = QDate(1990, 5, 1);
        repo->add(p);

        QTemporaryDir outDir;
        QString icsPath = outDir.path() + "/out.ics";
        QVERIFY(svc.exportToIcs(icsPath, -1, true, 7));

        QFile f(icsPath);
        QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
        QString contents = f.readAll();
        QVERIFY(contents.contains("TRIGGER:-P7D"));
        QVERIFY(!contents.contains("TRIGGER:-P3D"));
    }

    void testImportSkipsExactDuplicates()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        auto personRepo = std::make_shared<SqlitePersonRepository>(dbm.database());
        auto groupRepo = std::make_shared<SqliteGroupRepository>(dbm.database());
        DataService dataSvc(personRepo, groupRepo);

        Person p;
        p.groupId = 1;
        p.firstName = "Олег";
        p.lastName = "Дубликатов";
        p.birthDate = QDate(1985, 3, 10);
        personRepo->add(p);

        QTemporaryDir dir;
        QString jsonPath = dir.path() + "/export.json";
        QVERIFY(dataSvc.exportPeopleJson(jsonPath));

        int skipped = 0;
        int imported = dataSvc.importPeopleJson(jsonPath, &skipped);
        QCOMPARE(imported, 0);
        QCOMPARE(skipped, 1);
        QCOMPARE(personRepo->getAll().size(), 1); // no duplicate created

        // A genuinely new person in the same file should still import.
        Person p2;
        p2.groupId = 1;
        p2.firstName = "Новый";
        p2.lastName = "Человек";
        personRepo->add(p2);
        QVERIFY(dataSvc.exportPeopleJson(jsonPath)); // re-export now includes both

        skipped = 0;
        imported = dataSvc.importPeopleJson(jsonPath, &skipped);
        QCOMPARE(imported, 0); // both already exist now
        QCOMPARE(skipped, 2);
    }

    void testImportVCard()
    {
        DatabaseManager dbm(":memory:");
        MigrationManager mm(dbm.database());
        mm.migrate();

        auto personRepo = std::make_shared<SqlitePersonRepository>(dbm.database());
        auto groupRepo = std::make_shared<SqliteGroupRepository>(dbm.database());
        DataService dataSvc(personRepo, groupRepo);

        QTemporaryDir dir;
        QString vcfPath = dir.path() + "/contacts.vcf";
        QFile vcf(vcfPath);
        QVERIFY(vcf.open(QIODevice::WriteOnly | QIODevice::Text));
        vcf.write(QByteArray(
            "BEGIN:VCARD\r\n"
            "VERSION:3.0\r\n"
            "N:Петров;Иван;Сергеевич;;\r\n"
            "FN:Иван Петров\r\n"
            "TEL;TYPE=CELL:+79001234567\r\n"
            "TEL;TYPE=HOME:+74951234567\r\n"
            "EMAIL;TYPE=WORK:ivan@example.com\r\n"
            "BDAY:1990-05-15\r\n"
            "END:VCARD\r\n"
            "BEGIN:VCARD\r\n"
            "VERSION:3.0\r\n"
            "FN:Анна Смирнова\r\n"
            "TEL:+79997654321\r\n"
            "END:VCARD\r\n"
        ));
        vcf.close();

        int skipped = 0;
        int imported = dataSvc.importPeopleVCard(vcfPath, &skipped);
        QCOMPARE(imported, 2);
        QCOMPARE(skipped, 0);

        auto people = personRepo->getAll();
        QCOMPARE(people.size(), 2);

        Person ivan;
        for (const auto &p : people)
            if (p.lastName == "Петров") ivan = p;
        QCOMPARE(ivan.firstName, "Иван");
        QCOMPARE(ivan.patronymic, "Сергеевич");
        QCOMPARE(ivan.phone, "+79001234567");
        QCOMPARE(ivan.birthDate, QDate(1990, 5, 15));

        auto extraPhones = personRepo->getPhoneNumbers(ivan.id);
        QCOMPARE(extraPhones.size(), 1);
        QCOMPARE(extraPhones[0].number, "+74951234567");

        auto emails = personRepo->getEmails(ivan.id);
        QCOMPARE(emails.size(), 1);
        QCOMPARE(emails[0].address, "ivan@example.com");

        Person anna;
        for (const auto &p : people)
            if (p.firstName == "Анна") anna = p;
        QCOMPARE(anna.lastName, "Смирнова");
        QCOMPARE(anna.phone, "+79997654321");

        // Both go into a fresh "Импорт_<date>" group, not into an
        // arbitrary pre-existing one (e.g. the seeded "Родственники").
        QVERIFY(ivan.groupId != 0);
        QCOMPARE(anna.groupId, ivan.groupId);
        Group importGroup = groupRepo->getById(ivan.groupId);
        QVERIFY(importGroup.name.startsWith("Импорт_"));
        QVERIFY(importGroup.name != "Родственники");

        // Re-importing the same file must skip both (exact duplicate).
        skipped = 0;
        imported = dataSvc.importPeopleVCard(vcfPath, &skipped);
        QCOMPARE(imported, 0);
        QCOMPARE(skipped, 2);
        QCOMPARE(personRepo->getAll().size(), 2); // no duplicates created

        // A second, genuinely new contact imported the same day reuses the
        // same "Импорт_<date>" group instead of failing on its UNIQUE name.
        QString vcfPath2 = dir.path() + "/more_contacts.vcf";
        QFile vcf2(vcfPath2);
        QVERIFY(vcf2.open(QIODevice::WriteOnly | QIODevice::Text));
        vcf2.write(QByteArray(
            "BEGIN:VCARD\r\n"
            "VERSION:3.0\r\n"
            "FN:Пётр Кузнецов\r\n"
            "TEL:+79111112233\r\n"
            "END:VCARD\r\n"
        ));
        vcf2.close();

        skipped = 0;
        imported = dataSvc.importPeopleVCard(vcfPath2, &skipped);
        QCOMPARE(imported, 1);
        QCOMPARE(groupRepo->getAll().size(), 5); // still just one import group added overall
    }
};

QObject* createTestServices() { return new TestServices(); }

#include "TestServices.moc"
