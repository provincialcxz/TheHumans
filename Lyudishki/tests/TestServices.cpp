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
};

QObject* createTestServices() { return new TestServices(); }

#include "TestServices.moc"
