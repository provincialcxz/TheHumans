#include <QtTest>
#include "data/db/DatabaseManager.h"
#include "data/db/MigrationManager.h"
#include "data/repositories/SqlitePersonRepository.h"
#include "data/repositories/SqliteGroupRepository.h"
#include "services/PeopleService.h"
#include "services/GroupService.h"
#include "services/SearchService.h"

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
};

QObject* createTestServices() { return new TestServices(); }

#include "TestServices.moc"
